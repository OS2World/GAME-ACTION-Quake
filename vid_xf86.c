// vid_x.c -- general x video driver

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/xf86dga.h>

#include "quakedef.h"
#include "d_local.h"

cvar_t	m_filter = {"m_filter","1"};

qboolean	mouse_avail;
int		mouse_buttons=3;
int		mouse_oldbuttonstate;
int		mouse_buttonstate;
float	mouse_x, mouse_y;
float	old_mouse_x, old_mouse_y;

typedef struct
{
	int input;
	int output;
} keymap_t;

viddef_t vid; // global video state
unsigned short       d_8to16table[256];

int		num_shades=32;
int	d_con_indirect = 0;
int		vid_buffersize;
int		pixbytes;

static Display			*x_disp;
static Colormap			x_cmap;
static Window			x_win;
static GC				x_gc;
static Visual			*x_vis;
static XVisualInfo		*x_visinfo;
//static XImage			*x_image;

static int vid_inited;
static int direct_video_inited;

int work_buffer;
static int verbose=0;

char *hw_fb_addr;
int hw_fb_width;
int hw_fb_banksize;
int hw_fb_ram;
int hw_fb_dotclock;
XF86VidModeModeLine hw_fb_modeline;

static long X11_highhunkmark;
static long X11_buffersize;
int vid_surfcachesize;
void *vid_surfcache;

static int flipy;
static char *flipaddr;

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);


// ========================================================================
// Tragic death handler
// ========================================================================

void TragicDeath(int signal_num)
{
	VID_Shutdown();
	Sys_Error("This death brought to you by the number %d\n", signal_num);
}

// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void	VID_Init (unsigned char *palette)
{

	int devmem;
	int i;
	XVisualInfo template;
	XSetWindowAttributes xswa;
	int MajorVersion, MinorVersion;
	int EventBase, ErrorBase;
	int MINMINOR=4;
	int MINMAJOR=0;
	struct sigaction sa;
	XGCValues xgcvalues;
	int valuemask = GCGraphicsExposures;
	char *dispname;
	Status xrc;
	XSizeHints *x_hints;

	vid_inited = 0;
	direct_video_inited = 0;

	devmem=open("/dev/mem", O_RDWR);
	if (devmem<0)
		fprintf(stderr, "You must run this as root or \"chmod 666 /dev/mem\"\n");
	else
		close(devmem);

	srandom(getpid());

	verbose=COM_CheckParm("-verbose");

// open the display
	dispname = getenv("DISPLAY");
	if (!dispname) dispname = ":0.0";
	x_disp = XOpenDisplay(dispname);
	if (!x_disp)
	{
		if (getenv("DISPLAY"))
			Sys_Error("VID: Could not open display [%s]\n",
				getenv("DISPLAY"));
		else
			Sys_Error("VID: Could not open local display\n");
	}
	XSynchronize(x_disp, True);

// use the root window for everything

//	x_win = RootWindow(x_disp, DefaultScreen(x_disp));
	xswa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

	x_win = XCreateWindow(x_disp, DefaultRootWindow(x_disp), 0, 0,
		WidthOfScreen(ScreenOfDisplay(x_disp, 0)),
		HeightOfScreen(ScreenOfDisplay(x_disp, 0)), 0,
		CopyFromParent, InputOutput, CopyFromParent,
		CWEventMask, &xswa);
	XSetTransientForHint(x_disp, x_win, x_win);
	x_hints = XAllocSizeHints();
	x_hints->flags = USPosition;
	XSetWMNormalHints(x_disp, x_win, x_hints);
//	X_Free(x_hints);
	XMapWindow(x_disp, x_win);
	XRaiseWindow(x_disp, x_win);

// check that vidmode extension is avail

	if (!XF86VidModeQueryVersion(x_disp, &MajorVersion, &MinorVersion))
		Sys_Error("Unable to query VidMode version");
	if (!XF86VidModeQueryExtension(x_disp, &EventBase, &ErrorBase))
		Sys_Error("Unable to query VidMode extension");
	if (MajorVersion < MINMAJOR || MinorVersion < MINMINOR)
		Sys_Error("Minimum required VidMode extension version is %d.%d\n",
			MINMAJOR, MINMINOR);

// catch signals so i can die well

	sigaction(SIGINT, 0, &sa);
	sa.sa_handler = TragicDeath;
	sigaction(SIGINT, &sa, 0);
	sigaction(SIGTERM, &sa, 0);

// set up the mode and get info on the fb

	xrc = XF86DGAGetVideo(x_disp, DefaultScreen(x_disp), &hw_fb_addr,
		&hw_fb_width, &hw_fb_banksize, &hw_fb_ram);
	xrc = XF86DGADirectVideo(x_disp, DefaultScreen(x_disp),
		XF86DGADirectGraphics|XF86DGADirectMouse|XF86DGADirectKeyb);
	if (xrc != True)
		Sys_Error("DirectVideo could not be setup\n");
	direct_video_inited = 1;
	setuid(getuid());
	xrc = XF86DGASetViewPort(x_disp, DefaultScreen(x_disp), 0, 0);
	xrc = XF86DGASetVidPage(x_disp, DefaultScreen(x_disp), 0);
	xrc = XF86VidModeGetModeLine(x_disp, DefaultScreen(x_disp),
		&hw_fb_dotclock, &hw_fb_modeline);

	vid.width = hw_fb_modeline.hdisplay;
	vid.height = hw_fb_modeline.vdisplay;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 2;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

// ritual for getting x_visinfo which basically just has the depth in it

	x_vis = DefaultVisual(x_disp, DefaultScreen(x_disp));
	template.visualid = XVisualIDFromVisual(x_vis);
	x_visinfo = XGetVisualInfo(x_disp, VisualIDMask, &template, &i);
	pixbytes = x_visinfo->depth/8;

// now know everything we need to know about the buffer

	work_buffer = 1;
	vid.rowbytes = hw_fb_width * pixbytes;
	flipaddr = hw_fb_addr + vid.rowbytes * vid.height;
	flipy = vid.height;
	vid.buffer = flipaddr;
	vid.direct = hw_fb_addr;
	vid.conbuffer = flipaddr;
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;

	Sys_Printf("VID: bank size = %d bytes\n", hw_fb_banksize);
	Sys_Printf("VID: ram = %dkb\n", hw_fb_ram);

	if (hw_fb_banksize < vid.rowbytes * vid.height * 2)
		Sys_Error("Video card bank size (%d bytes) too small for this res.", hw_fb_banksize);

	if (vid.rowbytes * vid.height * 2 > hw_fb_ram * 1024)
		Sys_Error("Not enough video memory for this res");

// create the GC

	xgcvalues.graphics_exposures = False;
	x_gc = XCreateGC(x_disp, x_win, valuemask, &xgcvalues );

// make input rawer

	XAutoRepeatOff(x_disp);
	XGrabKeyboard(x_disp, x_win, True, GrabModeAsync,
		GrabModeAsync, CurrentTime);
	XGrabPointer(x_disp, x_win, True, 
		ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime);

// setup colormap if necessary

	if (x_visinfo->depth == 8)
	{

		if (x_visinfo->class == PseudoColor)
		{
			x_cmap = XCreateColormap(x_disp, x_win, x_vis, AllocAll);
			VID_SetPalette(palette);
			XInstallColormap(x_disp, x_cmap);
			XSetWindowColormap(x_disp, x_win, x_cmap);
			XSetWMColormapWindows(x_disp, x_win, &x_win, 1);
		}

	}

	X11_highhunkmark = Hunk_HighMark ();
	X11_buffersize = vid.width * vid.height * sizeof (*d_pzbuffer);
	vid_surfcachesize = D_SurfaceCacheForRes (vid.width, vid.height);
	X11_buffersize += vid_surfcachesize;
	d_pzbuffer = Hunk_HighAllocName (X11_buffersize, "video");
	if (d_pzbuffer == NULL)
        Sys_Error ("Not enough memory for video mode\n");
	vid_surfcache = (byte *) d_pzbuffer
		+ vid.width * vid.height * sizeof (*d_pzbuffer);
	D_InitCaches(vid_surfcache, vid_surfcachesize);

	signal(SIGFPE, SIG_IGN);

	vid_inited = 1;

}

void VID_ShiftPalette(unsigned char *p)
{
	if (!vid_inited) return;
	VID_SetPalette(p);
}

void VID_SetPalette(unsigned char *palette)
{

	int i;
	XColor colors[256];

	if (!vid_inited) return;
	if (x_visinfo->class == PseudoColor && x_visinfo->depth == 8)
	{
		for (i=0 ; i<256 ; i++)
		{
			colors[i].pixel = i;
			colors[i].flags = DoRed|DoGreen|DoBlue;
			colors[i].red = palette[i*3] * 257;
			colors[i].green = palette[i*3+1] * 257;
			colors[i].blue = palette[i*3+2] * 257;
		}
		XStoreColors(x_disp, x_cmap, colors, 256);
	}

}

// Called at shutdown

void	VID_Shutdown (void)
{
	Con_Printf("VID_Shutdown\n");
	if (x_disp)
	{
		XUngrabPointer(x_disp, CurrentTime);
		XUngrabKeyboard(x_disp, CurrentTime);
		XAutoRepeatOn(x_disp);
		if (direct_video_inited)
			XF86DGADirectVideo(x_disp, DefaultScreen(x_disp), 0);
		direct_video_inited = 0;
		XCloseDisplay(x_disp);
		x_disp = 0;
	}
	vid_inited = 0;
}

int XLateKey(XKeyEvent *ev)
{

	int key;
	char buf[64];
	KeySym keysym;

	key = 0;

	XLookupString(ev, buf, sizeof buf, &keysym, 0);

	switch(keysym)
	{
		case XK_Page_Up:	 key = K_PGUP; break;
		case XK_Page_Down:	 key = K_PGDN; break;
		case XK_Home:	 key = K_HOME; break;
		case XK_End:	 key = K_END; break;
		case XK_Left:	 key = K_LEFTARROW; break;
		case XK_Right:	key = K_RIGHTARROW;		break;
		case XK_Down:	 key = K_DOWNARROW; break;
		case XK_Up:		 key = K_UPARROW;	 break;
		case XK_Escape: key = K_ESCAPE;		break;
		case XK_Return: key = K_ENTER;		 break;
		case XK_Tab:		key = K_TAB;			 break;
		case XK_F1:		 key = K_F1;				break;
		case XK_F2:		 key = K_F2;				break;
		case XK_F3:		 key = K_F3;				break;
		case XK_F4:		 key = K_F4;				break;
		case XK_F5:		 key = K_F5;				break;
		case XK_F6:		 key = K_F6;				break;
		case XK_F7:		 key = K_F7;				break;
		case XK_F8:		 key = K_F8;				break;
		case XK_F9:		 key = K_F9;				break;
		case XK_F10:		key = K_F10;			 break;
		case XK_F11:		key = K_F11;			 break;
		case XK_F12:		key = K_F12;			 break;
		case XK_BackSpace:
		case XK_Delete: key = K_BACKSPACE; break;
		case XK_Pause:	key = K_PAUSE;		 break;
		case XK_Shift_L:
		case XK_Shift_R:		key = K_SHIFT;		break;
		case XK_Execute: 
		case XK_Control_L: 
		case XK_Control_R:	key = K_CTRL;		 break;
		case XK_Alt_L:	
		case XK_Meta_L: 
		case XK_Alt_R:	
		case XK_Meta_R: key = K_ALT;			break;

		case 0x07e: key = '`';break;/* [~] */
		case 0x021: key = '1';break;/* [!] */
		case 0x040: key = '2';break;/* [@] */
		case 0x023: key = '3';break;/* [#] */
		case 0x024: key = '4';break;/* [$] */
		case 0x025: key = '5';break;/* [%] */
		case 0x05e: key = '6';break;/* [^] */
		case 0x026: key = '7';break;/* [&] */
		case 0x02a: key = '8';break;/* [*] */
		case 0x028: key = '9';;break;/* [(] */
		case 0x029: key = '0';break;/* [)] */
		case 0x05f: key = '-';break;/* [_] */
		case 0x02b: key = '=';break;/* [+] */
		case 0x07c: key = '\'';break;/* [|] */
		case 0x07d: key = '[';break;/* [}] */
		case 0x07b: key = ']';break;/* [{] */
		case 0x022: key = '\'';break;/* ["] */
		case 0x03a: key = ';';break;/* [:] */
		case 0x03f: key = '/';break;/* [?] */
		case 0x03e: key = '.';break;/* [>] */
		case 0x03c: key = ',';break;/* [<] */

		default:
			key = *(unsigned char*)buf;
			if (key >= 'A' && key <= 'Z')
				key = key - 'A' + 'a';
//			fprintf(stderr, "case 0x0%x: key = ___;break;/* [%c] */\n", keysym);
			break;
	} 

	return key;

}

struct
{
    int key;
    int down;
} keyq[64];
int keyq_head=0;
int keyq_tail=0;

void GetEvent(void)
{

	XEvent x_event;
	int b;

	XNextEvent(x_disp, &x_event);
	switch(x_event.type)
	{
		case KeyPress:
            keyq[keyq_head].key = XLateKey(&x_event.xkey);
            keyq[keyq_head].down = true;
            keyq_head = (keyq_head + 1) & 63;
			break;
		case KeyRelease:
            keyq[keyq_head].key = XLateKey(&x_event.xkey);
            keyq[keyq_head].down = false;
            keyq_head = (keyq_head + 1) & 63;
			break;
		case MotionNotify:
			mouse_x += (float) x_event.xmotion.x;
			mouse_y += (float) x_event.xmotion.y;
			break;
		case ButtonPress:
			b=-1;
			if (x_event.xbutton.button == Button1)
				b = 0;
			else if (x_event.xbutton.button == Button2)
				b = 2;
			else if (x_event.xbutton.button == Button3)
				b = 1;
			if (b>=0)
				mouse_buttonstate |= 1<<b;
			break;
		case ButtonRelease:
			b=-1;
			if (x_event.xbutton.button == Button1)
				b = 0;
			else if (x_event.xbutton.button == Button2)
				b = 2;
			else if (x_event.xbutton.button == Button3)
				b = 1;
			if (b>=0)
				mouse_buttonstate &= ~(1<<b);
			break;
	}

}

// flushes the given rectangles from the view buffer to the screen

void	VID_Update (vrect_t *rects)
{

	if (!vid_inited) return;

// flip pages

	XF86DGASetViewPort(x_disp, DefaultScreen(x_disp), 0,
		flipy * work_buffer);

	work_buffer = !work_buffer;
	if (work_buffer)
	{
		vid.buffer = flipaddr;
		vid.conbuffer = flipaddr;
		vid.direct = hw_fb_addr;
	}
	else
	{
		vid.buffer = hw_fb_addr;
		vid.conbuffer = hw_fb_addr;
		vid.direct = flipaddr;
	}

	XSync(x_disp, False);

}

static int dither;

void VID_DitherOn(void)
{
    if (dither == 0)
    {
		vid.recalc_refdef = 1;
        dither = 1;
    }
}

void VID_DitherOff(void)
{
    if (dither)
    {
		vid.recalc_refdef = 1;
        dither = 0;
    }
}

int Sys_OpenWindow(void)
{
	return 0;
}

void Sys_EraseWindow(int window)
{
}

void Sys_DrawCircle(int window, int x, int y, int r)
{
}

void Sys_DisplayWindow(int window)
{
}

void Sys_SendKeyEvents(void)
{
	if (!vid_inited) return;
// get events from x server
    if (x_disp)
    {
        while (XPending(x_disp)) GetEvent();
        while (keyq_head != keyq_tail)
        {
            Key_Event(keyq[keyq_tail].key, keyq[keyq_tail].down);
            keyq_tail = (keyq_tail + 1) & 63;
        }
    }
}

char *Sys_ConsoleInput (void)
{

	static char	text[256];
	int		len;
	fd_set  readfds;
	int		ready;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	ready = select(1, &readfds, 0, 0, &timeout);

	if (ready>0)
	{
		len = read (0, text, sizeof(text));
		if (len >= 1)
		{
			text[len-1] = 0;	// rip off the /n and terminate
			return text;
		}
	}

	return 0;
	
}


static byte	backingbuf[48*24];

void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	int		i, j, reps, repshift;

	if (!vid_inited) return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	for (i=0 ; i<(height << repshift) ; i += reps)
	{
		for (j=0 ; j<reps ; j++)
		{
			memcpy (&backingbuf[(i + j) * 24],
					vid.direct + x + ((y << repshift) + i + j) *
					 vid.rowbytes, width);
			memcpy (vid.direct + x + ((y << repshift) + i + j) *
					 vid.rowbytes, &pbitmap[(i >> repshift) * width], width);
		}
	}

}

void D_EndDirectRect (int x, int y, int width, int height)
{
	int		i, j, reps, repshift;

	if (!vid_inited) return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	for (i=0 ; i<(height << repshift) ; i += reps)
	{
		for (j=0 ; j<reps ; j++)
		{
			memcpy (vid.direct + x + ((y << repshift) + i + j) *
					 vid.rowbytes, &backingbuf[(i + j) * 24], width);
		}
	}

}

/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	Cvar_RegisterVariable (&m_filter);
	if ( COM_CheckParm ("-nomouse") ) 
		return; 
	mouse_x = mouse_y = 0.0;
	mouse_avail = 1;
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{
	mouse_avail = 0;
}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int		i;

	if (!vid_inited || !mouse_avail) return;

// perform button actions
	for (i=0 ; i<mouse_buttons ; i++)
	{
		if ( (mouse_buttonstate & (1<<i)) &&
		!(mouse_oldbuttonstate & (1<<i)) )
		{
			Key_Event (K_MOUSE1 + i, true);
		}
		if ( !(mouse_buttonstate & (1<<i)) &&
		(mouse_oldbuttonstate & (1<<i)) )
		{
			Key_Event (K_MOUSE1 + i, false);
		}
	}	
	
	mouse_oldbuttonstate = mouse_buttonstate;

}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{

	if (!vid_inited || !mouse_avail)
		return;
	
	if (m_filter.value)
	{
		mouse_x = (mouse_x + old_mouse_x) * 0.5;
		mouse_y = (mouse_y + old_mouse_y) * 0.5;
	}
	old_mouse_x = mouse_x;
	old_mouse_y = mouse_y;

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
		
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}

	mouse_x = mouse_y = 0.0;

}

