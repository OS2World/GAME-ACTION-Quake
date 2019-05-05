// vid_null.h -- null video driver to aid porting efforts

#include "quakedef.h"
#include "d_local.h"
#include "vid_dos.h"

#include <graph.h>

#define INCL_DOS
#define INCL_MOU
#define INCL_KBD
#define INCL_VIO
#define INCL_DOSMONITORS

#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <process.h>
#include <os2thunk.h>
#include <alloca.h>

int			vid_modenum;
vmode_t		*pcurrentmode = NULL;
int			vid_testingmode, vid_realmode;
double		vid_testendtime;

cvar_t		vid_mode = {"vid_mode","0", false};
cvar_t		vid_wait = {"vid_wait","0"};
cvar_t		vid_nopageflip = {"vid_nopageflip","0", true};
cvar_t		_vid_wait_override = {"_vid_wait_override", "0", true};
cvar_t		_vid_default_mode = {"_vid_default_mode","0", true};

int	d_con_indirect = 0;

int		numvidmodes;
vmode_t	*pvidmodes;

static int	firstupdate = 1;

void VID_TestMode_f (void);
void VID_NumModes_f (void);
void VID_DescribeCurrentMode_f (void);
void VID_DescribeMode_f (void);
void VID_DescribeModes_f (void);
void block_us();
void unblock_us();

byte	vid_current_palette[768];	// save for mode changes

cvar_t	m_filter = {"m_filter","1"};

static qboolean	nomodecheck = false;
qboolean	mouse_avail;
int		mouse_buttons=0;
int		mouse_oldbuttonstate = 0;
int		mouse_buttonstate = 0;
float	mouse_x, mouse_y;
float	old_mouse_x, old_mouse_y;

int background = 0;

viddef_t	vid;				// global video state

int in_g_mode = 0;

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

unsigned char replace_table[256 * 3];

typedef struct
{
      USHORT          usMonFlags;
      KBDKEYINFO      KeyInfo;
      USHORT          usDrvFlags;
} KBDMONPACKET;

HKBD hKeyboard = 0;
USHORT our_session;
ULONG monitor_thread_id, mouse_thread_id;
HMOU hMouse = 0;

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);
void VID_MenuDraw (void);
void VID_MenuKey (int key);

struct
{
	int key;
	int down;
} keyq[64];
int keyq_head=0;
int keyq_tail=0;

int background_execution = 1;

void lock_us(void)
{
	BYTE not_locked;
	if (!background_execution)
	{
		VioScrLock (LOCKIO_WAIT, &not_locked, 0);
	}
	else
	{
		VioScrLock (LOCKIO_NOWAIT, &not_locked, 0);
		if (not_locked && !background)
		{
			background = 1;
		}	
		else if (!not_locked && background)
		{
//			g_vgapal (replace_table, 0, 256, 0);	
			background = 0;
		}
	}
}

void unlock_us(void)
{
	if (!background_execution)
	{
		VioScrUnLock (0);
	} 
	else if (!background)
	{
		VioScrUnLock (0);
	}
}

void InitVioKbd(void);
void open_mouse(void);
void close_mouse(void);

extern void SetTextMode(void);

void	VID_Shutdown (void)
{
	SetTextMode();
}

void	VID_Init (unsigned char *palette)
{
	open_mouse();
	atexit(VID_Shutdown);
	if (COM_CheckParm("-nobg"))
		background_execution = 0;

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&vid_wait);
	Cvar_RegisterVariable (&vid_nopageflip);
	Cvar_RegisterVariable (&_vid_wait_override);
	Cvar_RegisterVariable (&_vid_default_mode);

	Cmd_AddCommand ("vid_testmode", VID_TestMode_f);
	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemode", VID_DescribeMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);

// set up the mode list; note that later inits link in their modes ahead of
// earlier ones, so the standard VGA modes are always first in the list. This
// is important because mode 0 must always be VGA mode 0x13
//	if (!COM_CheckParm ("-stdvid"))
//		VID_InitExtra ();
	VGA_Init ();

	vid_testingmode = 0;

	vid_modenum = vid_mode.value;

	VID_SetMode (vid_modenum, palette);

	vid_realmode = vid_modenum;
	
	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;

	InitVioKbd();
}


/*
=================
VID_GetModePtr
=================
*/
vmode_t *VID_GetModePtr (int modenum)
{
	vmode_t	*pv;

	pv = pvidmodes;
	if (!pv)
		Sys_Error ("VID_GetModePtr: empty vid mode list");

	while (modenum--)
	{
		pv = pv->pnext;
		if (!pv)
			Sys_Error ("VID_GetModePtr: corrupt vid mode list");
	}

	return pv;
}

/*
================
VID_ModeInfo
================
*/
char *VID_ModeInfo (int modenum, char **ppheader)
{
	static char	*badmodestr = "Bad mode number";
	vmode_t		*pv;

	pv = VID_GetModePtr (modenum);

	if (!pv)
	{
		if (ppheader)
			*ppheader = NULL;
		return badmodestr;
	}
	else
	{
		if (ppheader)
			*ppheader = pv->header;
		return pv->name;
	}
}


/*
================
VID_NumModes
================
*/
int VID_NumModes ()
{
	return (numvidmodes);
}


/*
================
VID_SetMode 
================
*/
int VID_SetMode (int modenum, unsigned char *palette)
{
	int		stat;
	vmode_t	*pnewmode, *poldmode;

	lock_us();

	if (background)
	{
		unlock_us();
		Con_Printf ("SetMode: setting mode while in background is not supported\n", modenum);
		return 0;
	}

	if ((modenum >= numvidmodes) || (modenum < 0))
	{
		Cvar_SetValue ("vid_mode", (float)vid_modenum);

		nomodecheck = true;
		Con_Printf ("No such video mode: %d\n", modenum);
		nomodecheck = false;

		if (pcurrentmode == NULL)
		{
			modenum = 0;	// mode hasn't been set yet, so initialize to base							//  mode since they gave us an invalid initial mode
		}
		else
		{
			unlock_us();
			return 0;
		}
	}

	pnewmode = VID_GetModePtr (modenum);

	if (pnewmode == pcurrentmode)
	{
		unlock_us();
		return 1;	// already in the desired mode
	}

// initialize the new mode
	poldmode = pcurrentmode;
	pcurrentmode = pnewmode;

	vid.width = pcurrentmode->width;
	vid.height = pcurrentmode->height;
	vid.aspect = pcurrentmode->aspect;
	vid.rowbytes = pcurrentmode->rowbytes;

	stat = (*pcurrentmode->setmode) (&vid, pcurrentmode);

	if (stat < 1)
	{
		unlock_us();
		if (stat == 0)
		{
		// real, hard failure that requires resetting the mode
			if (!VID_SetMode (vid_modenum, palette))	// restore prior mode
				Sys_Error ("VID_SetMode: Unable to set any mode, probably "
						   "because there's not enough memory available");
			Con_Printf ("Failed to set mode %d\n", modenum);
			return 0;
		}
		else if (stat == -1)
		{
		// not enough memory; just put things back the way they were
			pcurrentmode = poldmode;
			vid.width = pcurrentmode->width;
			vid.height = pcurrentmode->height;
			vid.aspect = pcurrentmode->aspect;
			vid.rowbytes = pcurrentmode->rowbytes;
			return 0;
		}
		else
		{
			Sys_Error ("VID_SetMode: invalid setmode return code %d");
		}
	}

	(*pcurrentmode->setpalette) (&vid, pcurrentmode, palette);
	unlock_us();

	vid_modenum = modenum;
	Cvar_SetValue ("vid_mode", (float)vid_modenum);

	nomodecheck = true;
	Con_Printf ("%s\n", VID_ModeInfo (vid_modenum, NULL));
	nomodecheck = false;

	vid.recalc_refdef = 1;

	return 1;
}

void    VID_SetPalette (unsigned char *palette)
{
	if (palette != vid_current_palette)
		memcpy(vid_current_palette, palette, 768);
	lock_us();
	if (!background)
	{
		(*pcurrentmode->setpalette)(&vid, pcurrentmode, vid_current_palette);
	}
	unlock_us();
}


/*
================
VID_ShiftPalette
================
*/
void    VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette (palette);
}


void    VID_Update (vrect_t *rects)
{
	if (firstupdate && _vid_default_mode.value)
	{
		firstupdate = 0;
		Cvar_SetValue ("vid_mode", _vid_default_mode.value);
	}

	(*pcurrentmode->swapbuffers)(&vid, pcurrentmode, rects);

	if (!nomodecheck)
	{
		if (vid_testingmode)
		{
			if (realtime >= vid_testendtime)
			{
				VID_SetMode (vid_realmode, vid_current_palette);
				vid_testingmode = 0;
			}
		}
		else
		{
			if (vid_mode.value != vid_realmode)
			{
				VID_SetMode ((int)vid_mode.value, vid_current_palette);
				Cvar_SetValue ("vid_mode", (float)vid_modenum);
									// so if mode set fails, we don't keep on
									//  trying to set that mode
				vid_realmode = vid_modenum;
			}
		}
	}
}


/*
=================
VID_NumModes_f
=================
*/
void VID_NumModes_f (void)
{
	int		nummodes;

	nummodes = VID_NumModes ();
	if (nummodes == 1)
		Con_Printf ("%d video mode is available\n", VID_NumModes ());
	else
		Con_Printf ("%d video modes are available\n", VID_NumModes ());
}


/*
=================
VID_DescribeCurrentMode_f
=================
*/
void VID_DescribeCurrentMode_f (void)
{
	Con_Printf ("%s\n", VID_ModeInfo (vid_modenum, NULL));
}


/*
=================
VID_DescribeMode_f
=================
*/
void VID_DescribeMode_f (void)
{
	int		modenum;
	
	modenum = Q_atoi (Cmd_Argv(1));

	Con_Printf ("%s\n", VID_ModeInfo (modenum, NULL));
}


/*
=================
VID_DescribeModes_f
=================
*/
void VID_DescribeModes_f (void)
{
	int			i, nummodes;
	char		*pinfo, *pheader;
	vmode_t		*pv;
	qboolean	na;

	na = false;

	nummodes = VID_NumModes ();
	for (i=0 ; i<nummodes ; i++)
	{
		pv = VID_GetModePtr (i);
		pinfo = VID_ModeInfo (i, &pheader);
		if (pheader)
			Con_Printf ("\n%s\n", pheader);
		{
			Con_Printf ("%2d: %s\n", i, pinfo);
		}
	}

	if (na)
	{
		Con_Printf ("\n[**: not enough system RAM for mode]\n");
	}
}


/*
=================
VID_GetModeDescription
=================
*/
char *VID_GetModeDescription (int mode)
{
	char		*pinfo, *pheader;
	vmode_t		*pv;

	pv = VID_GetModePtr (mode);
	pinfo = VID_ModeInfo (mode, &pheader);
	return pinfo;
}


/*
=================
VID_TestMode_f
=================
*/
void VID_TestMode_f (void)
{
	int		modenum;
	double	testduration;

	if (!vid_testingmode)
	{
		modenum = Q_atoi (Cmd_Argv(1));

		if (VID_SetMode (modenum, vid_current_palette))
		{
			vid_testingmode = 1;
			testduration = Q_atof (Cmd_Argv(2));
			if (testduration == 0)
				testduration = 5.0;
			vid_testendtime = realtime + testduration;
		}
	}
}


/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}

/* ======================================================================= */

extern void M_Menu_Options_f (void);
extern void M_Print (int cx, int cy, char *str);
extern void M_PrintWhite (int cx, int cy, char *str);
extern void M_DrawCharacter (int cx, int line, int num);
extern qpic_t	*M_CachePic (char *path);
extern void M_DrawTransPic (int x, int y, qpic_t *pic);
extern void M_DrawPic (int x, int y, qpic_t *pic);

static int	vid_line, vid_wmodes, vid_column_size;

typedef struct
{
	int		modenum;
	char	*desc;
	int		iscur;
} modedesc_t;

#define MAX_COLUMN_SIZE	11

#define MAX_MODEDESCS	(MAX_COLUMN_SIZE*3)

static modedesc_t	modedescs[MAX_MODEDESCS];

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	qpic_t		*p;
	char		*ptr;
	int			nummodes, i, j, column, row, dup;
	char		temp[100];

	vid_wmodes = 0;
	nummodes = VID_NumModes ();
	
	p = M_CachePic ("gfx/vidmodes.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	for (i=0 ; i<nummodes ; i++)
	{
		if (vid_wmodes < MAX_MODEDESCS)
		{
//			if (i != 1)
			{
				ptr = VID_GetModeDescription (i);

				if (ptr)
				{
					dup = 0;

					for (j=0 ; j<vid_wmodes ; j++)
					{
						if (!strcmp (modedescs[j].desc, ptr))
						{
							if (modedescs[j].modenum != 0)
							{
								modedescs[j].modenum = i;
								dup = 1;

								if (i == vid_modenum)
									modedescs[j].iscur = 1;
							}
							else
							{
								dup = 1;
							}

							break;
						}
					}

					if (!dup)
					{
						modedescs[vid_wmodes].modenum = i;
						modedescs[vid_wmodes].desc = ptr;
						modedescs[vid_wmodes].iscur = 0;

						if (i == vid_modenum)
							modedescs[vid_wmodes].iscur = 1;

						vid_wmodes++;
					}
				}
			}
		}
	}

	vid_column_size = (vid_wmodes + 2) / 3;

	column = 16;
	row = 36;

	for (i=0 ; i<vid_wmodes ; i++)
	{
		if (modedescs[i].iscur)
			M_PrintWhite (column, row, modedescs[i].desc);
		else
			M_Print (column, row, modedescs[i].desc);

		row += 8;

		if ((i % vid_column_size) == (vid_column_size - 1))
		{
			column += 13*8;
			row = 36;
		}
	}

// line cursor
	if (vid_testingmode)
	{
		sprintf (temp, "TESTING %s",
				modedescs[vid_line].desc);
		M_Print (13*8, 36 + MAX_COLUMN_SIZE * 8 + 8*4, temp);
		M_Print (9*8, 36 + MAX_COLUMN_SIZE * 8 + 8*6,
				"Please wait 5 seconds...");
	}
	else
	{
		M_Print (9*8, 36 + MAX_COLUMN_SIZE * 8 + 8,
				"Press Enter to set mode");
		M_Print (6*8, 36 + MAX_COLUMN_SIZE * 8 + 8*3,
				"T to test mode for 5 seconds");
		ptr = VID_GetModeDescription (vid_modenum);
		sprintf (temp, "D to make %s the default", ptr);
		M_Print (6*8, 36 + MAX_COLUMN_SIZE * 8 + 8*5, temp);
		ptr = VID_GetModeDescription ((int)_vid_default_mode.value);

		if (ptr)
		{
			sprintf (temp, "Current default is %s", ptr);
			M_Print (7*8, 36 + MAX_COLUMN_SIZE * 8 + 8*6, temp);
		}

		M_Print (15*8, 36 + MAX_COLUMN_SIZE * 8 + 8*8,
				"Esc to exit");

		row = 36 + (vid_line % vid_column_size) * 8;
		column = 8 + (vid_line / vid_column_size) * 13*8;

		M_DrawCharacter (column, row, 12+((int)(realtime*4)&1));
	}
}


/*
================
VID_MenuKey
================
*/
void VID_MenuKey (int key)
{
	if (vid_testingmode)
		return;

	switch (key)
	{
	case K_ESCAPE:
		S_LocalSound ("misc/menu1.wav");
		M_Menu_Options_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line--;

		if (vid_line < 0)
			vid_line = vid_wmodes - 1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line++;

		if (vid_line >= vid_wmodes)
			vid_line = 0;
		break;

	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line -= vid_column_size;

		if (vid_line < 0)
		{
			vid_line += ((vid_wmodes + (vid_column_size - 1)) /
					vid_column_size) * vid_column_size;

			while (vid_line >= vid_wmodes)
				vid_line -= vid_column_size;
		}
		break;

	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		vid_line += vid_column_size;

		if (vid_line >= vid_wmodes)
		{
			vid_line -= ((vid_wmodes + (vid_column_size - 1)) /
					vid_column_size) * vid_column_size;

			while (vid_line < 0)
				vid_line += vid_column_size;
		}
		break;

	case K_ENTER:
		S_LocalSound ("misc/menu1.wav");
		VID_SetMode (modedescs[vid_line].modenum, vid_current_palette);
		break;

	case 'T':
	case 't':
		S_LocalSound ("misc/menu1.wav");
		if (VID_SetMode (modedescs[vid_line].modenum, vid_current_palette))
		{
			vid_testingmode = 1;
			vid_testendtime = realtime + 5.0;
		}
		break;

	case 'D':
	case 'd':
		S_LocalSound ("misc/menu1.wav");
		firstupdate = 0;
		Cvar_SetValue ("_vid_default_mode", vid_modenum);
		break;

	default:
		break;
	}
}

/* ======================================================================= */

byte        scan2key[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'' ,    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10,0  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5 
	K_HOME,   K_UPARROW ,    K_PGUP  ,  K_LEFTARROW,  K_RIGHTARROW  ,   K_END  ,    K_DOWNARROW  ,    K_PGDN, 
/* 104 */
	K_INS  ,    K_DEL  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 
#define BOTH_SHIFT	(KBDSTF_RIGHTSHIFT | KBDSTF_LEFTSHIFT)
#define BOTH_CTRL	(KBDSTF_CONTROL | KBDSTF_LEFTCONTROL | KBDSTF_RIGHTCONTROL)
#define BOTH_ALT	(KBDSTF_ALT | KBDSTF_LEFTALT | KBDSTF_RIGHTALT)

void monitor_thread(void)
{
	MONIN *InBuf;
	MONOUT *OutBuf;
	KBDMONPACKET *key;
	USHORT Count;          /* number of chars in monitor buffer */
	USHORT scan;

	do
		InBuf = alloca(sizeof(*InBuf));
	while (!_THUNK_PTR_STRUCT_OK(InBuf));

	do
		OutBuf = alloca(sizeof(*OutBuf));
	while (!_THUNK_PTR_STRUCT_OK(OutBuf));

	do
		key = alloca(sizeof(*key));
	while (!_THUNK_PTR_STRUCT_OK(key));

    	InBuf->cb = sizeof (*InBuf);
    	OutBuf->cb = sizeof (*OutBuf);

	if (DosMonReg (hKeyboard, (PBYTE)InBuf, (PBYTE)OutBuf, 1, our_session))
        	DosExit(EXIT_THREAD, 0);

	DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, 0x1f, monitor_thread_id);

	while (1)
	{
	        Count = sizeof(*key);

        	if(DosMonRead( InBuf, IO_WAIT, key, &Count))
	            DosExit(EXIT_THREAD, 0);

		if (key->KeyInfo.fbStatus & KBDTRF_EXTENDED_CODE)
			scan = (key->usMonFlags >> 8) & 0x7f;
		else
			scan = key->KeyInfo.chScan;
		if (scan && scan2key[scan])
		{	if (!(scan == 1 && (key->KeyInfo.fsState & (BOTH_CTRL | BOTH_ALT))))
			{
				keyq[keyq_head].key = scan2key[scan];
				keyq[keyq_head].down = (key->usDrvFlags & 0x40) == 0;
				keyq_head = (keyq_head + 1) & 63;
			}
		}
		else
		{
			static int alt_down = 0;
			static int ctrl_down = 0;
			static int shift_down = 0;
			int new_alt_down, new_ctrl_down, new_shift_down;
			if (key->KeyInfo.fbStatus & KBDTRF_SHIFT_KEY_IN)
			{
				new_alt_down = (key->KeyInfo.fsState & BOTH_ALT) != 0;
				new_ctrl_down = (key->KeyInfo.fsState & BOTH_CTRL) != 0;
				new_shift_down = (key->KeyInfo.fsState & BOTH_SHIFT) != 0;
				if (alt_down != new_alt_down)
				{
					keyq[keyq_head].key = K_ALT;
					keyq[keyq_head].down = (key->usDrvFlags & 0x40) == 0;
					keyq_head = (keyq_head + 1) & 63;
				}
				if (ctrl_down != new_ctrl_down)
				{
					keyq[keyq_head].key = K_CTRL;
					keyq[keyq_head].down = (key->usDrvFlags & 0x40) == 0;
					keyq_head = (keyq_head + 1) & 63;
				}
				if (shift_down != new_shift_down)
				{
					keyq[keyq_head].key = K_SHIFT;
					keyq[keyq_head].down = (key->usDrvFlags & 0x40) == 0;
					keyq_head = (keyq_head + 1) & 63;
				}
				alt_down = new_alt_down;
				ctrl_down = new_ctrl_down;
				shift_down = new_shift_down;
			}
		}
	}
}

#define MOUSE_MOVED (MOUSE_MOTION | MOUSE_MOTION_WITH_BN1_DOWN | MOUSE_MOTION_WITH_BN2_DOWN | MOUSE_MOTION_WITH_BN3_DOWN)


void Sys_SendKeyEvents(void)
{
	while (keyq_head != keyq_tail)
	{
		Key_Event(keyq[keyq_tail].key, keyq[keyq_tail].down);
		keyq_tail = (keyq_tail + 1) & 63;
	}
}

void mouse_thread(void)
{
	MOUEVENTINFO *mouse_event;
	USHORT 	mouse_wait = MOU_WAIT;
	DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, 0x1f, mouse_thread_id);
	do
		mouse_event = alloca(sizeof(*mouse_event));
	while (!_THUNK_PTR_STRUCT_OK(mouse_event));
	mouse_event->fs = 0;
	while (1)
	{
		MouReadEventQue(mouse_event, &mouse_wait, hMouse);
		if (mouse_event->fs & MOUSE_MOVED)
		{
			mouse_x += (float) mouse_event->col;
			mouse_y += (float) mouse_event->row;
		}

		if (mouse_event->fs & (MOUSE_MOTION_WITH_BN1_DOWN | MOUSE_BN1_DOWN))
			mouse_buttonstate |= 1;
		else
			mouse_buttonstate &= ~(1);
		if (mouse_event->fs & (MOUSE_MOTION_WITH_BN2_DOWN | MOUSE_BN2_DOWN))
			mouse_buttonstate |= 2;
		else
			mouse_buttonstate &= ~(2);
		if (mouse_event->fs & (MOUSE_MOTION_WITH_BN3_DOWN | MOUSE_BN3_DOWN))
			mouse_buttonstate |= 4;
		else
			mouse_buttonstate &= ~(4);
	}	
}

void close_monitor(void)
{
	if (hKeyboard)
		DosMonClose(hKeyboard);
}

void InitVioKbd(void)
{
	KBDINFO *info;
	ULONG cur_ses;

	do
		info = alloca(sizeof(*info));
	while (!_THUNK_PTR_STRUCT_OK(info));
	bzero(info, sizeof(*info));
	info->cb = sizeof(*info);
	KbdGetStatus(info, 0);
	info->fsMask |= KEYBOARD_BINARY_MODE | KEYBOARD_ECHO_OFF | KEYBOARD_SHIFT_REPORT;
	info->fsMask &= ~(KEYBOARD_ECHO_ON | KEYBOARD_ASCII_MODE);
	KbdSetStatus(info, 0);

	cur_ses = 0;
	DosQuerySysInfo(QSV_FOREGROUND_FS_SESSION, QSV_FOREGROUND_FS_SESSION, 
		&cur_ses, 4);

	cur_ses &= 255;

	if (DosMonOpen ( "KBD$", &hKeyboard ))
	{
		printf("DosMonOpen() failed for keyboard\n");
		exit(1);
	}
	our_session = cur_ses;
	DosCreateThread(&monitor_thread_id, (PFNTHREAD) monitor_thread, 0, 0, 16384);
	DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, 0x1f, monitor_thread_id);
	atexit(close_monitor);
}

void open_mouse(void)
{
	ULONG rc;
	USHORT mouse_status;

	hMouse = 0;
	rc = MouOpen(0, &hMouse);
	if (rc)
	{
		hMouse = 0;
		return;
	}
	if (hMouse != 0)
	{
		rc = MouGetNumButtons(&mouse_buttons, hMouse);
		if (rc)
		{
			MouClose(hMouse);
			hMouse = 0;
			return;
		}
		mouse_status = MOUSE_DISABLED | MOUSE_MICKEYS;
		rc = MouSetDevStatus(&mouse_status, hMouse);
		if (rc)
		{
			MouClose(hMouse);
			hMouse = 0;
			return;
		}
		mouse_status = 0xFFFF;		
		rc = MouSetEventMask(&mouse_status, hMouse);
		if (rc)
		{
			MouClose(hMouse);
			hMouse = 0;
			return;
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
	{
		close_mouse();
		return; 
	}
	if (hMouse)
	{
		mouse_x = mouse_y = 0.0;
		mouse_avail = 1;
		Con_Printf("%d-button mouse available\n", mouse_buttons);
		DosCreateThread(&mouse_thread_id, (PFNTHREAD) mouse_thread, 0, 0, 16384);
		DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, 0x1f, monitor_thread_id);	
	}
}


void close_mouse(void)
{
	if (hMouse)
	{
		MouClose(hMouse);
		hMouse = 0;
	}
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

	if (!mouse_avail) return;

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

	if (!mouse_avail)
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

