#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include "/usr/local/include/vga.h"
#include "/usr/local/include/vgakeyboard.h"
#include "/usr/local/include/vgamouse.h"

#include "quakedef.h"
#include "vid_256.h"

#define stringify(m) { #m, m }

viddef_t vid;

int num_modes;
vga_modeinfo *modes;
int current_mode;

int num_shades=32;

struct
{
	char *name;
	int num;
} mice[] =
{
	stringify(MOUSE_MICROSOFT),
	stringify(MOUSE_MOUSESYSTEMS),
	stringify(MOUSE_MMSERIES),
	stringify(MOUSE_LOGITECH),
	stringify(MOUSE_BUSMOUSE),
	stringify(MOUSE_PS2),
};

static unsigned char scantokey[128];

int num_mice = sizeof (mice) / sizeof(mice[0]);

int	d_con_indirect = 0;

int		svgalib_inited=0;
int		UseMouse = 1;
int		UseDisplay = 1;
int		UseKeyboard = 1;

int		mouserate = MOUSE_DEFAULTSAMPLERATE;

char	*framebuffer_ptr;

cvar_t  mouse_button_commands[3] =
{
    {"mouse1","+attack"},
    {"mouse2","+strafe"},
    {"mouse3","+forward"},
};
cvar_t  mouse_sensitivity = {"sensitivity","5"};

int     mouse_buttons;
int     mouse_oldbuttonstate;
int     mouse_x, mouse_y;

/*
=================
VID_Gamma_f

Keybinding command
=================
*/
void VID_Gamma_f (void)
{
	float	gamma, f, inf;
	unsigned char	palette[768];
	int		i;

	if (Cmd_Argc () == 2)
	{
		gamma = Q_atof (Cmd_Argv(1));

		for (i=0 ; i<768 ; i++)
		{
			f = pow ( (host_basepal[i]+1)/256.0 , gamma );
			inf = f*255 + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			palette[i] = inf;
		}

		VID_SetPalette (palette);

		vid.recalc_refdef = 1;				// force a surface cache flush
	}
}


void VID_InitModes(void)
{

	int i;

// get complete information on all modes

	num_modes = vga_lastmodenumber()+1;
	I_Printf("num_modes = %d\n", num_modes);
	modes = Z_Malloc(num_modes * sizeof(vga_modeinfo));
	for (i=0 ; i<num_modes ; i++)
	{
		if (vga_hasmode(i))
			Q_memcpy(&modes[i], vga_getmodeinfo(i), sizeof (vga_modeinfo));
		else
			modes[i].width = 0; // means not available
	}

// filter for modes i don't support

	for (i=0 ; i<num_modes ; i++)
	{
		if (modes[i].bytesperpixel != 1) modes[i].width = 0;
	}

}

int get_mode(char *name, int width, int height, int depth)
{

	int i;
	int ok, match;

	match = (!!width) + (!!height)*2 + (!!depth)*4;

	if (name)
	{
		i = vga_getmodenumber(name);
		if (!modes[i].width)
		{
			I_Printf("Mode [%s] not supported\n", name);
			i = G320x200x256;
		}
	}
	else
	{
		for (i=0 ; i<num_modes ; i++)
			if (modes[i].width)
			{
				ok = (modes[i].width == width)
					+ (modes[i].height == height)*2
					+ (modes[i].bytesperpixel == depth/8)*4;
				if ((ok & match) == ok)
					break;
			}
		if (i==num_modes)
		{
			I_Printf("Mode %dx%d (%d bits) not supported\n",
				width, height, depth);
			i = G320x200x256;
		}
	}

	return i;

}

int matchmouse(int mouse, char *name)
{
	int i;
	for (i=0 ; i<num_mice ; i++)
		if (!strcmp(mice[i].name, name))
			return i;
	return mouse;
}

#if 0

void vtswitch(int newconsole)
{

	int fd;
	struct vt_stat x;

// switch consoles and wait until reactivated
	fd = open("/dev/console", O_RDONLY);
	ioctl(fd, VT_GETSTATE, &x);
	ioctl(fd, VT_ACTIVATE, newconsole);
	ioctl(fd, VT_WAITACTIVE, x.v_active);
	close(fd);

}

#endif

void keyhandler(int scancode, int state)
{
	
	int sc;

	sc = scancode & 0x7f;
//	Con_Printf("scancode=%x (%d%s)\n", scancode, sc, scancode&0x80?"+128":"");
	Key_Event(scantokey[sc], state == KEY_EVENTPRESS);

}

void VID_Shutdown(void)
{

	if (!svgalib_inited) return;

//	printf("shutdown graphics called\n");
	if (UseKeyboard)
		keyboard_close();
	if (UseDisplay)
		vga_setmode(TEXT);
//	printf("shutdown graphics finished\n");

	svgalib_inited = 0;

}

void VID_SetPalette(byte *palette)
{

	static int tmppal[256*3];
	int *tp;
	int i;

	if (!svgalib_inited)
		return;

	if (vga_getcolors() == 256)
	{

		tp = tmppal;
		for (i=256*3 ; i ; i--)
			*(tp++) = *(palette++) >> 2;

		if (UseDisplay)
			vga_setpalvec(0, 256, tmppal);

	}

}

void VID_Init(unsigned char *palette)
{

	int i;
	int w, h, d;

	if (svgalib_inited)
		return;

	Cmd_AddCommand ("gamma", VID_Gamma_f);

/*
    signal(SIGHUP, (void (*)(int)) I_Quit);
    signal(SIGINT, (void (*)(int)) I_Quit);
    signal(SIGKILL, (void (*)(int)) I_Quit);
    signal(SIGTERM, (void (*)(int)) I_Quit);
    signal(SIGSTOP, (void (*)(int)) I_Quit);
    signal(SIGQUIT, (void (*)(int)) I_Quit);
*/

	if (UseDisplay)
	{
		vga_init();

		VID_InitModes();

	// interpret command-line params

		w = h = d = 0;
		if (getenv("GSVGAMODE"))
			current_mode = get_mode(getenv("GSVGAMODE"), w, h, d);
		else if (COM_CheckParm("-mode"))
			current_mode = get_mode(com_argv[COM_CheckParm("-mode")+1], w, h, d);
		else if (COM_CheckParm("-w") || COM_CheckParm("-h")
			|| COM_CheckParm("-d"))
		{
			if (COM_CheckParm("-w"))
				w = Q_atoi(com_argv[COM_CheckParm("-w")+1]);
			if (COM_CheckParm("-h"))
				h = Q_atoi(com_argv[COM_CheckParm("-h")+1]);
			if (COM_CheckParm("-d"))
				d = Q_atoi(com_argv[COM_CheckParm("-d")+1]);
			current_mode = get_mode(0, w, h, d);
		}
		else
			current_mode = G320x200x256;

	// set vid parameters

		vid.width = modes[current_mode].width;
		vid.rowbytes = modes[current_mode].linewidth;
		vid.height = modes[current_mode].height;
		vid.aspect = 1.0;
		vid.colormap = (pixel_t *) colormap256;
		vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
		vid.buffer = (pixel_t *) Hunk_HighAlloc(vid.rowbytes * vid.height);

	// get goin'

		vga_setmode(current_mode);
		framebuffer_ptr = (char *) vga_getgraphmem();
//		if (vga_setlinearaddressing()>0)
//			framebuffer_ptr = (char *) vga_getgraphmem();
		if (!framebuffer_ptr)
			I_Error("This mode isn't hapnin'\n");

		vga_setpage(0);

		svgalib_inited=1;

		VID_SetPalette(palette);

	}

	if (COM_CheckParm("-nokybd")) UseKeyboard = 0;

	if (UseKeyboard)
	{
		for (i=0 ; i<128 ; i++)
			scantokey[i] = ' ';

		scantokey[42] = K_SHIFT;
		scantokey[54] = K_SHIFT;
		scantokey[72] = K_UPARROW;
		scantokey[103] = K_UPARROW;
		scantokey[80] = K_DOWNARROW;
		scantokey[108] = K_DOWNARROW;
		scantokey[75] = K_LEFTARROW;
		scantokey[105] = K_LEFTARROW;
		scantokey[77] = K_RIGHTARROW;
		scantokey[106] = K_RIGHTARROW;
		scantokey[29] = K_CTRL;
		scantokey[97] = K_CTRL;
		scantokey[56] = K_ALT;
		scantokey[100] = K_ALT;
//		scantokey[58] = JK_CAPS;
//		scantokey[69] = JK_NUM_LOCK;
//		scantokey[71] = JK_HOME;
//		scantokey[79] = JK_END;
//		scantokey[83] = JK_DEL;
		scantokey[1 ] = K_ESCAPE;
		scantokey[28] = K_ENTER;
		scantokey[15] = K_TAB;
		scantokey[14] = K_BACKSPACE;
		scantokey[119] = K_PAUSE;
    	scantokey[57] = ' ';

		scantokey[2] = '1';
		scantokey[3] = '2';
		scantokey[4] = '3';
		scantokey[5] = '4';
		scantokey[6] = '5';
		scantokey[7] = '6';
		scantokey[8] = '7';
		scantokey[9] = '8';
		scantokey[10] = '9';
		scantokey[11] = '0';
		scantokey[12] = '-';
		scantokey[13] = '=';
		scantokey[41] = '`';
		scantokey[26] = '[';
		scantokey[27] = ']';
		scantokey[39] = ';';
		scantokey[40] = '\'';
		scantokey[51] = ',';
		scantokey[52] = '.';
		scantokey[53] = '/';
		scantokey[43] = '\\';

		scantokey[59] = K_F1;
		scantokey[60] = K_F2;
		scantokey[61] = K_F3;
		scantokey[62] = K_F4;
		scantokey[63] = K_F5;
		scantokey[64] = K_F6;
		scantokey[65] = K_F7;
		scantokey[66] = K_F8;
		scantokey[67] = K_F9;
		scantokey[68] = K_F10;
		scantokey[87] = K_F11;
		scantokey[88] = K_F12;
		scantokey[30] = 'a';
		scantokey[48] = 'b';
		scantokey[46] = 'c';
        scantokey[32] = 'd';       
        scantokey[18] = 'e';       
        scantokey[33] = 'f';       
        scantokey[34] = 'g';       
        scantokey[35] = 'h';       
        scantokey[23] = 'i';       
        scantokey[36] = 'j';       
        scantokey[37] = 'k';       
        scantokey[38] = 'l';       
        scantokey[50] = 'm';       
        scantokey[49] = 'n';       
        scantokey[24] = 'o';       
        scantokey[25] = 'p';       
        scantokey[16] = 'q';       
        scantokey[19] = 'r';       
        scantokey[31] = 's';       
        scantokey[20] = 't';       
        scantokey[22] = 'u';       
        scantokey[47] = 'v';       
        scantokey[17] = 'w';       
        scantokey[45] = 'x';       
        scantokey[21] = 'y';       
        scantokey[44] = 'z';       

		if (keyboard_init())
			I_Error("keyboard_init() failed");
		keyboard_seteventhandler(keyhandler);
	}

}

void VID_Update(vrect_t *rects)
{

	int ycount;
	int offset;

	if (!svgalib_inited)
		return;

	while (rects)
	{
		ycount = rects->height;
		offset = rects->y * vid.rowbytes + rects->x;
		while (ycount--)
		{
			memcpy(framebuffer_ptr + offset, vid.buffer + offset, rects->width);
			offset += vid.rowbytes;
		}

		rects = rects->pnext;
	}

}

static int dither;

void VID_DitherOn(void)
{
    if (dither == 0)
    {
        R_ViewChanged ();
        dither = 1;
    }
}

void VID_DitherOff(void)
{
    if (dither)
    {
        R_ViewChanged ();
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
	if (!svgalib_inited)
		return;

	if (UseKeyboard)
		while (keyboard_update());
}

char *Sys_ConsoleInput (void)
{
	return 0;
}

void mousehandler(int mouse_buttonstate, int dx, int dy)
{

	int i;

    for (i=0 ; i<mouse_buttons ; i++)
    {
        if ( (mouse_buttonstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) )
            Cbuf_AddText (mouse_button_commands[i].string);

        if ( !(mouse_buttonstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) )
        {
            if (mouse_button_commands[i].string[0] == '+')
            {
                mouse_button_commands[i].string[0] = '-';
                Cbuf_AddText (mouse_button_commands[i].string);
                mouse_button_commands[i].string[0] = '+';
            }
        }
    }

	mouse_oldbuttonstate = mouse_buttonstate;

	mouse_x = dx*3;
	mouse_y = dy*3;

}

void IN_Init(void)
{

	int mtype;
	char *mousedev;
	int mouserate;

	if (UseMouse)
	{

		Cvar_RegisterVariable (&mouse_button_commands[0]);
		Cvar_RegisterVariable (&mouse_button_commands[1]);
		Cvar_RegisterVariable (&mouse_button_commands[2]);

		Cvar_RegisterVariable (&mouse_sensitivity);

		mouse_buttons = 3;

		mtype = vga_getmousetype();

		mousedev = "/dev/mouse";
		if (getenv("MOUSEDEV")) mousedev = getenv("MOUSEDEV");
		if (COM_CheckParm("-mdev"))
			mousedev = com_argv[COM_CheckParm("-mdev")+1];

		mouserate = 1200;
		if (getenv("MOUSERATE")) mouserate = atoi(getenv("MOUSERATE"));
		if (COM_CheckParm("-mrate"))
			mouserate = atoi(com_argv[COM_CheckParm("-mrate")+1]);

		printf("Mouse: dev=%s,type=%s,speed=%d\n",
			mousedev, mice[mtype].name, mouserate);
		if (mouse_init(mousedev, mtype, mouserate))
		{
			Con_Printf("No mouse found\n");
			UseMouse = 0;
		}
		else
			mouse_seteventhandler(mousehandler);

	}

}

void IN_Shutdown(void)
{
	if (UseMouse)
		mouse_close();
}

void IN_Commands (void)
{
	if (UseMouse)
		while (mouse_update());
}

int CL_KeyState (int *key);
extern int in_strafe;
void IN_Move (usercmd_t *cmd)
{

	if (!UseMouse)
		return;

// add mouse X/Y movement to cmd
	if (in_strafe)
	{
		cmd->sidemove += 0.8*mouse_sensitivity.value * mouse_x;
		cmd->forwardmove -= 1.0*mouse_sensitivity.value * mouse_y;
	}
	else
	{
		cl_viewangles[YAW] -= 4.0/180*mouse_sensitivity.value * mouse_x;
		cl_viewangles[PITCH] -= 4.0/180*mouse_sensitivity.value * mouse_y;
	}

	if (cl_viewangles[PITCH] > 80)
		cl_viewangles[PITCH] = 80;
	if (cl_viewangles[PITCH] < -70)
		cl_viewangles[PITCH] = -70;

	if (cl_viewangles[ROLL] > 50)
		cl_viewangles[ROLL] = 50;
	if (cl_viewangles[ROLL] < -50)
		cl_viewangles[ROLL] = -50;

/*
    if (!UseMouse)
        return;

// add mouse X/Y movement to cmd
    if (CL_KeyState (&in_strafe))
    {
        cmd->sidemove += 0.8*mouse_sensitivity.value * mouse_x;
        cmd->forwardmove -= 1.0*mouse_sensitivity.value * mouse_y;
    }
    else
    {
        cmd->rightturn += 4.0/180*mouse_sensitivity.value * mouse_x;
        cmd->upturn -= 4.0/180*mouse_sensitivity.value * mouse_y;
    }

*/

}



/*
================
VID_NumModes
================
*/
int VID_NumModes ()
{
	return (1);
}


/*
================
VID_ModeInfo
================
*/
char *VID_ModeInfo (int modenum)
{
	static char	*badmodestr = "Bad mode number";
	static char modestr[40];

	if (modenum == 0)
	{
		sprintf (modestr, "%d x %d, %d bpp",
				 vid.width, vid.height, modes[current_mode].bytesperpixel*8);
		return (modestr);
	}
	else
	{
		return (badmodestr);
	}
}
