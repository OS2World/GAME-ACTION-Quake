// vid_x.c -- general x video driver

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/XShm.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "quakedef.h"
#include "vid_256.h"

typedef struct
{
	int input;
	int output;
} keymap_t;

viddef_t vid; // global video state

int		num_shades=32;

static qboolean			doShm;
static Display			*x_disp;
static Colormap			x_cmap;
static Window			x_win;
static GC				x_gc;
static Visual			*x_vis;
static XVisualInfo		*x_visinfo=0;
//static XImage			*x_image;
static int				x_screen;

static int				x_shmeventtype;
//static XShmSegmentInfo	x_shminfo;

static qboolean			oktodraw = false;

static unsigned char	*framebuffer=0;

static int verbose=0;

static short *jcspace=0;

static qboolean			doGLX;
static int glX_errorBase, glX_eventBase;
static GLXContext glX_context;

static qboolean			glX_scaled=0;
static GLfloat			glX_scale_width;
static GLfloat			glX_scale_height;

static int config_notify=0;
static int config_notify_width;
static int config_notify_height;

int	d_con_indirect = 0;

void Scaled_f(void)
{
	config_notify = 1;
	glX_scaled = 1;
}

void UnScaled_f(void)
{
	config_notify = 1;
	glX_scaled = 0;
}

// ========================================================================
// Tragic death handler
// ========================================================================

void TragicDeath(int signal_num)
{
	XAutoRepeatOn(x_disp);
	XCloseDisplay(x_disp);
	I_Error("This death brought to you by the number %d\n", signal_num);
}

// ========================================================================
// makes a null cursor
// ========================================================================

static Cursor CreateNullCursor(Display *display, Window root)
{
    Pixmap cursormask; 
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask,
          &dummycolour,&dummycolour, 0,0);
    XFreePixmap(display,cursormask);
    XFreeGC(display,gc);
    return cursor;
}

void ResetFrameBuffer(void)
{

	int mem;
	int pwidth;

	if (!glX_scaled)
	{

		if (framebuffer)
			Z_Free(framebuffer);

		pwidth = 4;
		mem = vid.width*4 * vid.height;

		framebuffer = Z_Malloc(mem);

		vid.rowbytes = 2 * vid.width;
		if (jcspace) Z_Free(jcspace);
		jcspace = Z_Malloc(vid.rowbytes * vid.height);
		vid.buffer = (pixel_t *) jcspace;

		glViewport(0, 0, vid.width, vid.height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, vid.width, 0, vid.height);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glPixelZoom(1.0, -1.0);
		glRasterPos2i(0, vid.height);

	}
	else
	{
		glViewport(0, 0, config_notify_width, config_notify_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, config_notify_width, 0, config_notify_height);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glX_scale_width = (float) config_notify_width / vid.width;
		glX_scale_height = (float) config_notify_height / vid.height;
		glPixelZoom(glX_scale_width, -glX_scale_height);

		glRasterPos2i(0, config_notify_height);

	}

}

int *lookup24;

void InitLookup24(unsigned char *pal)
{

	int intensity;
	int color;
	int r, g, b;

	lookup24 = Z_Malloc(VID_GRADES * 256 * 4);

// intensities go from 0 (bright) to VID_GRADES - 1 (dark)

	for (intensity = 0 ; intensity < VID_GRADES ; intensity++)
	{
		for (color = 0 ; color < 256 ; color++)
		{
			r = pal[color*3] * (VID_GRADES - intensity) / VID_GRADES;
			g = pal[color*3+1] * (VID_GRADES - intensity) / VID_GRADES;
			b = pal[color*3+2] * (VID_GRADES - intensity) / VID_GRADES;
			r = (r<<24) + (r<<16) + (r<<8) + r;
			g = (g<<24) + (g<<16) + (g<<8) + g;
			b = (b<<24) + (b<<16) + (b<<8) + b;
			r = r & 0xff000000;
			g = g & 0x00ff0000;
			b = b & 0x0000ff00;
			/*
			r = r & x_visinfo->red_mask;
			g = g & x_visinfo->green_mask;
			b = b & x_visinfo->blue_mask;
			*/
			lookup24[intensity * 256 + color] = r + g + b;
		}
	}

}

void JCSpaceTo24(void)
{

	int rowcount;
	int height;
	short *from;
	int *to;

	from = jcspace;
	to = (int*) framebuffer;

	for (height = vid.height ; height ; height--)
	{
		for (rowcount = vid.width ; rowcount ; rowcount--)
		{
			*to = lookup24[*from];
			from++;
			to++;
		}
	}

}

void PickVisual(void)
{

	int attribList1[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 8,
		GLX_BLUE_SIZE, 8, GLX_GREEN_SIZE, 8, None };
	int attribList2[] = { GLX_RGBA, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8, None };

	doGLX = glXQueryExtension(x_disp, &glX_errorBase, &glX_eventBase);
	if (!doGLX) I_Error("OpenGL not supported\n");

	x_visinfo = glXChooseVisual(x_disp, x_screen, attribList1);
	if (!x_visinfo)
		x_visinfo = glXChooseVisual(x_disp, x_screen, attribList2);
	else
		printf("double-buffered\n");
	if (!x_visinfo)
		I_Error("Could not chose an RGB visual.  Use X version.\n");
	else
		printf("single-buffered\n");

//	if (verbose)
	{
		printf("Using visualid %d:\n", (int)(x_visinfo->visualid));
		printf("	screen %d\n", x_visinfo->screen);
		printf("	red_mask 0x%x\n", (int)(x_visinfo->red_mask));
		printf("	green_mask 0x%x\n", (int)(x_visinfo->green_mask));
		printf("	blue_mask 0x%x\n", (int)(x_visinfo->blue_mask));
		printf("	colormap_size %d\n", x_visinfo->colormap_size);
		printf("	bits_per_rgb %d\n", x_visinfo->bits_per_rgb);
	}

	x_vis = x_visinfo->visual;

}

void CreateWindow(void)
{

// setup attributes for main window
	int attribmask = CWEventMask  | CWColormap | CWBorderPixel;
	XSetWindowAttributes attribs;
	Colormap tmpcmap;

	tmpcmap = XCreateColormap(x_disp, XRootWindow(x_disp,
		x_visinfo->screen), x_vis, AllocNone);

	attribs.event_mask = StructureNotifyMask | KeyPressMask
		| KeyReleaseMask | ExposureMask;
	attribs.border_pixel = 0;
	attribs.colormap = tmpcmap;

// create the main window
	x_win = XCreateWindow(	x_disp,
		XRootWindow(x_disp, x_visinfo->screen),
		0, 0,	// x, y
		vid.width, vid.height,
		0, // borderwidth
		x_visinfo->depth,
		InputOutput,
		x_vis,
		attribmask,
		&attribs );

	XFreeColormap(x_disp, tmpcmap);

}

// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void	VID_Init (unsigned char *palette)
{

	int pnum, i;
	int num_visuals;

	Cmd_AddCommand("glx_scaled", Scaled_f);
	Cmd_AddCommand("glx_unscaled", UnScaled_f);

	vid.width = 320;
	vid.height = 200;
	vid.aspect = 1.0;
	vid.numpages = 1;

	srandom(getpid());

	verbose=COM_CheckParm("-verbose");

// open the display
	x_disp = XOpenDisplay(0);
	if (!x_disp)
	{
		if (getenv("DISPLAY"))
			I_Error("VID: Could not open display [%s]\n",
				getenv("DISPLAY"));
		else
			I_Error("VID: Could not open local display\n");
	}

	x_screen = DefaultScreen(x_disp);

// catch signals so i can turn on auto-repeat

	{
		struct sigaction sa;
		sigaction(SIGINT, 0, &sa);
		sa.sa_handler = TragicDeath;
		sigaction(SIGINT, &sa, 0);
		sigaction(SIGTERM, &sa, 0);
	}

	XAutoRepeatOff(x_disp);

// for debugging only
	XSynchronize(x_disp, True);

// check for command-line window size
	if ((pnum=COM_CheckParm("-winsize")))
	{
		if (pnum >= com_argc-2)
			I_Error("VID: -winsize <width> <height>\n");
		vid.width = Q_atoi(com_argv[pnum+1]);
		vid.height = Q_atoi(com_argv[pnum+2]);
		if (!vid.width || !vid.height)
			I_Error("VID: Bad window width/height\n");
	}

	PickVisual();

	CreateWindow();

	glX_context = glXCreateContext(x_disp, x_visinfo, 0, True);
	glXMakeCurrent(x_disp, x_win, glX_context);
	glPixelZoom(1.0, -1.0);

// create the GC
	{
		XGCValues xgcvalues;
		int valuemask = GCGraphicsExposures;
		xgcvalues.graphics_exposures = False;
		x_gc = XCreateGC(x_disp, x_win, valuemask, &xgcvalues );
	}

	InitLookup24(palette);

// inviso cursor
	XDefineCursor(x_disp, x_win, CreateNullCursor(x_disp, x_win));

// map the window
	XMapWindow(x_disp, x_win);

// wait for first exposure event
	{
		XEvent event;
		do
		{
			XNextEvent(x_disp, &event);
			if (event.type == Expose && !event.xexpose.count)
				oktodraw = true;
		} while (!oktodraw);
	}
// now safe to draw

	ResetFrameBuffer();

	XSynchronize(x_disp, False);

}

void VID_SetPalette(unsigned char *palette)
{

	if (lookup24)
		Z_Free(lookup24);
	InitLookup24(palette);

}

// Called at shutdown

void	VID_Shutdown (void)
{
	I_Printf("VID_Shutdown\n");
	XAutoRepeatOn(x_disp);
	XCloseDisplay(x_disp);
}

int XLateKey(XKeyEvent *ev)
{

	int key;
	char buf[64];
	KeySym keysym;

	XLookupString(ev, buf, sizeof buf, &keysym, 0);

	switch(keysym)
	{
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
		case XK_Control_L: 
		case XK_Control_R:	key = K_CTRL;		 break;
		case XK_Alt_L:	
		case XK_Meta_L: 
		case XK_Alt_R:	
		case XK_Meta_R: key = K_ALT;			break;
		default:
			key = *buf;
			break;
	} 

	return key;

}

void GetEvent(void)
{

	XEvent x_event;

	XNextEvent(x_disp, &x_event);
	switch(x_event.type)
	{
		case KeyPress:
			Key_Event(XLateKey(&x_event.xkey), true);
			break;
		case KeyRelease:
			Key_Event(XLateKey(&x_event.xkey), false);
			break;
		case ConfigureNotify:
//			printf("config notify\n");
			config_notify_width = x_event.xconfigure.width;
			config_notify_height = x_event.xconfigure.height;
			config_notify = 1;
			break;
	}

}

// flushes the given rectangles from the view buffer to the screen

void	VID_Update (vrect_t *rects)
{

// if the window changes dimension, skip this frame

	if (config_notify)
	{
		printf("config notify\n");
		config_notify = 0;
		if (!glX_scaled)
		{
			vid.width = config_notify_width & ~3;
			vid.height = config_notify_height;
		}
		ResetFrameBuffer();
		return;
	}

// slam that baby

	JCSpaceTo24();
	glDrawPixels(vid.width, vid.height, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);
	glFlush();
	glXSwapBuffers(x_disp, x_win);

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

int I_OpenWindow(void)
{
	return 0;
}

void I_EraseWindow(int window)
{
}

void I_DrawCircle(int window, int x, int y, int r)
{
}

void I_DisplayWindow(int window)
{
}

void Sys_SendKeyEvents(void)
{
// get events from x server
	while (XPending(x_disp)) GetEvent();
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

