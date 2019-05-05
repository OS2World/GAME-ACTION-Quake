//
// vid_vga.c: VGA-specific DOS video stuff
//

// TODO: proper handling of page-swap failure

#include "quakedef.h"
#include "d_local.h"
#include "dosisms.h"
#include "vid_dos.h"

#include "vgaports.h"

#define INCL_DOS
#define INCL_VIO

#include <os2.h>

extern regs_t regs;

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes;
byte	*VGA_pagebase = 0;
vmode_t	*VGA_pcurmode;

static int		VGA_planar;
static int		VGA_numpages;

void	*vid_surfcache;
void	*dabuffer;
int		vid_surfcachesize;

int		VGA_highhunkmark;

extern vmode_t		*pcurrentmode;

int internal_set_mode(vmode_t *pcurrentmode);

extern int background;

struct {
	ULONG dst;
	ULONG src;
} iopl;

ULONG iopl_addr;

extern void lock_us();

extern void unlock_us();

#include "vgamodes.h"

#define NUMVIDMODES		(sizeof(vgavidmodes) / sizeof(vgavidmodes[0]))

void VGA_UpdatePlanarScreen (void *srcbuffer);

HFILE ScreenHandle = 0;
int vesa_inited = 0;

VOID _THUNK_FUNCTION (I) ();
VOID _THUNK_FUNCTION (O) ();
VOID _THUNK_FUNCTION (W) ();

#define _outp8	outport8
#define _inp8   inport8

void outport8 (USHORT port, byte val)
{
	      (_THUNK_PROLOG(4);
	       _THUNK_SHORT(port);
	       _THUNK_SHORT(val);		
	       _THUNK_CALL(O));
}


byte inport8 (USHORT port)
{
	      return ((byte)
		(_THUNK_PROLOG(2);
	       _THUNK_SHORT(port);
	       _THUNK_CALL(I)));
}

void wait_retrace(void)
{
	      (_THUNK_PROLOG(0);
	       _THUNK_CALL(W));	
}

int InitializeVesa(void)
{
  ULONG action;

  return(DosOpen("SCREEN$", &ScreenHandle, &action, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS, OPEN_FLAGS_NOINHERIT |
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL) == 0);
}


void block_us()
{
	BYTE not_locked;
	VioScrLock (LOCKIO_WAIT, &not_locked, 0);
}

void unblock_us(void)
{
		VioScrUnLock (0);
}

ULONG saverestore_thread_id;
ULONG modewait_thread_id;

void SetTextMode(void)
{
	  VIOMODEINFO *ModeInfo;

	do
		ModeInfo = alloca(sizeof(*ModeInfo));
	while (!_THUNK_PTR_STRUCT_OK(ModeInfo));

	  ModeInfo->cb     = 12;
	  ModeInfo->fbType = 0x01;
	  ModeInfo->color  = 4;
	  ModeInfo->col    = 640 / 8;
	  ModeInfo->row    = 400 / 16;
	  ModeInfo->hres   = 640;
	  ModeInfo->vres   = 400;
	  VioSetMode(ModeInfo, 0);
}

extern char vid_current_palette[];

void saverestore_thread(void)
{
	USHORT moo;
	while (1)
	{
		if (VioSavRedrawWait(0, &moo, 0) != 0)
			continue;
		if (moo)
		{
			if (pcurrentmode)
			{
				internal_set_mode(pcurrentmode);
				VGA_SetPalette(0, 0, vid_current_palette);
				VGA_ClearVideoMem(pcurrentmode->planar);
			}
		}
		else
		{
			SetTextMode();
		}
	}
}


void modewait_thread(void)
{
	USHORT moo;
	while (1)
	{
		if (VioModeWait(0, &moo, 0) != 0)
			continue;
		if (pcurrentmode)
		{
			internal_set_mode(pcurrentmode);
			VGA_SetPalette(0, 0, vid_current_palette);
			VGA_ClearVideoMem(pcurrentmode->planar);
		}
	}
}

ULONG pal16 = 0;
char *pal32;

/*
================
VGA_Init
================
*/
void VGA_Init (void)
{
	int		i;

// link together all the VGA modes
	for (i=0 ; i<(NUMVIDMODES - 1) ; i++)
	{
		vgavidmodes[i].pnext = &vgavidmodes[i+1];
	}

// add the VGA modes at the start of the mode list
	vgavidmodes[NUMVIDMODES-1].pnext = pvidmodes;
	pvidmodes = &vgavidmodes[0];

	numvidmodes += NUMVIDMODES;

	do
		pal32 = malloc(768);
	while (!_THUNK_PTR_SIZE_OK(pal32, 768));

	pal16 = _emx_32to16(pal32);

	DosCreateThread(&saverestore_thread_id, (PFNTHREAD) saverestore_thread, 0, 0, 16384);
	DosCreateThread(&modewait_thread_id, (PFNTHREAD) modewait_thread, 0, 0, 16384);
	if (InitializeVesa())
		vesa_inited = 1;
	else
		vesa_inited = 0;
}

void set_bank(ULONG Address)
{
  struct
    {
      ULONG  length;
      USHORT bank;
      USHORT modetype;
      USHORT bankmode;
    } parameter;
  ULONG datalen, parmlen;

  datalen = 0;
  parmlen = sizeof(parameter);
  parameter.length   = sizeof(parameter);
  parameter.bank     = Address;
  parameter.modetype = 2;
  parameter.bankmode = 1;
  if (DosDevIOCtl(ScreenHandle, 0x80, 1,
                  &parameter, parmlen, &parmlen, NULL, 0, &datalen))
    DosBeep(880, 500);
}

/*
================
VGA_ClearVideoMem
================
*/
void VGA_ClearVideoMem (int planar)
{
	if (!VGA_pagebase)
		return;
	if (VGA_pcurmode->width == 640)
	{
		set_bank(0);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(1);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(2);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(3);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(4);
		memset(VGA_pagebase, 0, 45056);
		return;
	}
	if (VGA_pcurmode->width == 800)
	{
		set_bank(0);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(1);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(2);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(3);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(4);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(5);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(6);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(7);
		memset(VGA_pagebase, 0, 21248);
		return;
	}
	if (VGA_pcurmode->width == 1024)
	{
		set_bank(0);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(1);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(2);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(3);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(4);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(5);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(6);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(7);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(8);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(9);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(10);
		memset(VGA_pagebase, 0, 256 * 256);
		set_bank(11);
		memset(VGA_pagebase, 0, 256 * 256);
		return;
	}
	if (planar)
	{
	// enable all planes for writing
		_outp8 (SC_INDEX, MAP_MASK);
		_outp8 (SC_DATA, 0x0F);
	}
	memset (VGA_pagebase, 0, VGA_rowbytes * VGA_height);
}

/*
================
VGA_FreeAndAllocVidbuffer
================
*/
qboolean VGA_FreeAndAllocVidbuffer (viddef_t *lvid, int allocnewbuffer)
{
	int		tsize;

	tsize = D_SurfaceCacheForRes (lvid->width, lvid->height);

	vid_surfcachesize = tsize;

	if (d_pzbuffer)
	{
		D_FlushCaches ();
		DosFreeMem(d_pzbuffer);
		DosFreeMem(vid_surfcache);
		DosFreeMem(dabuffer);
		d_pzbuffer = NULL;
	}

	DosAllocMem((PPVOID)&d_pzbuffer, 
		lvid->width * lvid->height * sizeof (*d_pzbuffer),
		PAG_READ | PAG_WRITE | PAG_COMMIT);

	DosAllocMem((PPVOID)&vid_surfcache, tsize, 
		PAG_READ | PAG_WRITE | PAG_COMMIT);
	
	DosAllocMem((PPVOID)&dabuffer, 
		lvid->rowbytes * (lvid->height + 1),
		PAG_READ | PAG_WRITE | PAG_COMMIT);

	lvid->buffer = dabuffer;
	lvid->conbuffer = lvid->buffer;

	return true;
}

#define CRT_C   24      /* 24 CRT Controller Registers    */
#define CRT     0       /* CRT Controller Registers start */
#define SEQ     24      /* Sequencer Register 4 index     */
#define MIS     25      /* Misc.Output Register index     */

static char moderegs[3][60] =
    { /* Register: Index  0..23: CRT Controller Registers 0..23  */
      /*           Index     24: Sequencer Controller Register 4 */
      /*           Index     25: Misc Output Register            */
      /* non-BIOS mode - 320x240x256 */
      { 0x5F,0x4F,0x50,0x82,0x54,0x80,0x0D,0x3E,0x00,0x41,0x00,0x00,
        0x00,0x00,0x00,0x00,0xEA,0xAC,0xDF,0x28,0x00,0xE7,0x06,0xE3,
        0x06,
        0xE3 },
      /* non-BIOS mode - 320x400x256 */
      { 0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x40,0x00,0x00,
        0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0xE3,
        0x06,
        0x63 },
      /* non-BIOS mode - 360x480x256 */
      { 0x6B,0x59,0x5A,0x8E,0x5E,0x8A,0x0D,0x3E,0x00,0x40,0x00,0x00,
        0x00,0x00,0x00,0x00,0xEA,0xAC,0xDF,0x30,0x00,0xE7,0x06,0xE3,
        0x06,
        0xE7 }
    };


VOID *GetPhysBuf(void)
{
  VIOMODEINFO ModeInfo;
  VIOPHYSBUF  PhysBuf;

  ModeInfo.cb  = sizeof(ModeInfo);
  VioGetMode(&ModeInfo, 0);
  PhysBuf.pBuf = (PBYTE) ModeInfo.buf_addr;
  PhysBuf.cb   = 0x10000;
  if (VioGetPhysBuf(&PhysBuf, 0) != 0)
    return(NULL);
  else
    return(MAKEP(PhysBuf.asel[0], 0));
}

int internal_set_mode(vmode_t *pcurrentmode)
{
	VIOMODEINFO *ModeInfo;
	vextra_t		*pextra;
	int mode;

	do
		ModeInfo = alloca(sizeof(*ModeInfo));
	while (!_THUNK_PTR_STRUCT_OK(ModeInfo));

	pextra = pcurrentmode->pextradata;

	if (pcurrentmode->width == 640)
	{
	  if (!vesa_inited)
		return 0;
	  ModeInfo->cb     = 12;
	  ModeInfo->fbType = 0x0b;
	  ModeInfo->color  = 8;
	  ModeInfo->col   = 640 / 8;
	  ModeInfo->row    = 480 / 16;
	  ModeInfo->hres   = 640;
	  ModeInfo->vres   = 480;
	  if (VioSetMode(ModeInfo, 0) != 0)
		return 0;
	  else
		return 1;
	}

	if (pcurrentmode->width == 800)
	{
	  if (!vesa_inited)
		return 0;
	  ModeInfo->cb     = 12;
	  ModeInfo->fbType = 0x0b;
	  ModeInfo->color  = 8;
	  ModeInfo->col   =  800 / 8;
	  ModeInfo->row    = 600 / 16;
	  ModeInfo->hres   = 800;
	  ModeInfo->vres   = 600;
	  if (VioSetMode(ModeInfo, 0) != 0)
		return 0;
	  else
		return 1;
	}
	if (pcurrentmode->width == 1024)
	{
	  if (!vesa_inited)
		return 0;
	  ModeInfo->cb     = 12;
	  ModeInfo->fbType = 0x0b;
	  ModeInfo->color  = 8;
	  ModeInfo->col   =  1024 / 8;
	  ModeInfo->row    = 768 / 16;
	  ModeInfo->hres   = 1024;
	  ModeInfo->vres   = 768;
	  if (VioSetMode(ModeInfo, 0) != 0)
		return 0;
	  else
		return 1;
	}

	ModeInfo->cb     = 12;
	ModeInfo->fbType = 0x03;
	ModeInfo->color  = 8;
	ModeInfo->col    = 320 / 8;
	ModeInfo->row    = 200 / 8;
	ModeInfo->hres   = 320;
	ModeInfo->vres   = 200;

	if (VioSetMode(ModeInfo, 0) != 0)
		return 0;

#define lvid pcurrentmode

	if (lvid->width == 320 && (lvid->height == 240 || lvid->height == 480))
		mode = 0;
	else if (lvid->width == 320 && lvid->height == 400)
		mode = 1;
	else if (lvid->width == 360 && lvid->height == 480)
		mode = 2;
	else
		mode = -1;

#undef lvid

	if (pextra)
	{
//	  _wait01 (IS1_R, 0x08);
	  wait_retrace();

	   if (mode != -1)
	   {
		  /* disable video */
		  _inp8(IS1_R);
		  _outp8(ATT_IW, 0x00);

		  /* update misc output register */
		  _outp8(MIS_W, moderegs[mode][MIS]);

		  /* write sequencer registers 4 */
		  _outp8(SEQ_I, 0x04);
		  _outp8(SEQ_D, moderegs[mode][SEQ]);

		  /* deprotect CRT registers 0-7 */
		  _outp8(CRT_I, 0x11);
		  _outp8(CRT_D, _inp8(CRT_D)&0x7F);

		VideoRegisterSet (pextra->pregset);

		  /* enable video */
		  _inp8(IS1_R);
		  _outp8(ATT_IW, 0x20);
	       }
	       else
			VideoRegisterSet (pextra->pregset);        
	}
	return 1;
}

/*
================
VGA_InitMode
================
*/
int VGA_InitMode (viddef_t *lvid, vmode_t *pcurrentmode)
{
	if (!VGA_FreeAndAllocVidbuffer (lvid, 1))
		return -1;	// memory alloc failed

	if (VGA_pcurmode)
		VGA_ClearVideoMem (VGA_pcurmode->planar);

	if (!internal_set_mode(pcurrentmode))
		return 0;

	VGA_pagebase = GetPhysBuf();

	if (!VGA_pagebase)
		return 0;

	iopl.src = _emx_32to16(dabuffer);
	iopl.dst = _emx_32to16(VGA_pagebase);
	iopl_addr = _emx_32to16(&iopl);

	lvid->direct = (pixel_t *)VGA_pagebase;

	VGA_numpages = 1;
	lvid->numpages = VGA_numpages;

	VGA_width = (lvid->width + 0x1F) & ~0x1F;
	VGA_height = lvid->height;
	VGA_planar = pcurrentmode->planar;
	if (VGA_planar)
		VGA_rowbytes = lvid->rowbytes / 4;
	else
		VGA_rowbytes = lvid->rowbytes;
	VGA_bufferrowbytes = lvid->rowbytes;
	lvid->colormap = host_colormap;
	lvid->fullbright = 256 - LittleLong (*((int *)lvid->colormap + 2048));

	lvid->maxwarpwidth = WARP_WIDTH;
	lvid->maxwarpheight = WARP_HEIGHT;

	lvid->conbuffer = lvid->buffer;
	lvid->conrowbytes = lvid->rowbytes;
	lvid->conwidth = lvid->width;
	lvid->conheight = lvid->height;

	VGA_pcurmode = pcurrentmode;

	VGA_ClearVideoMem (pcurrentmode->planar);

	if (_vid_wait_override.value)
	{
		Cvar_SetValue ("vid_wait", (float)VID_WAIT_VSYNC);
	}
	else
	{
		Cvar_SetValue ("vid_wait", (float)VID_WAIT_NONE);
	}

	D_InitCaches (vid_surfcache, vid_surfcachesize);

	return 1;
}

VOID _THUNK_FUNCTION (P) ();

/*
================
VGA_SetPalette
================
*/
void VGA_SetPalette(viddef_t *lvid, vmode_t *pcurrentmode, unsigned char *pal)
{
	UNUSED(lvid);
	UNUSED(pcurrentmode);

	memcpy(pal32, pal, 768);

       (_THUNK_PROLOG(4);
        _THUNK_LONG(pal16);
        _THUNK_CALL(P));	
}


VOID _THUNK_FUNCTION (3648) ();
VOID _THUNK_FUNCTION (3240) ();
VOID _THUNK_FUNCTION (3224) ();
VOID _THUNK_FUNCTION (3620) ();
VOID _THUNK_FUNCTION (3624) ();
VOID _THUNK_FUNCTION (3235) ();
VOID _THUNK_FUNCTION (3635) ();
VOID _THUNK_FUNCTION (3640) ();
VOID _THUNK_FUNCTION (3248) ();

void VGA_320x200linear_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
		memcpy(VGA_pagebase, dabuffer, 320 * 200);
	unlock_us();
}

void VGA_360x480planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3648));
	}
	unlock_us();
}

void VGA_360x200planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3620));
	}
	unlock_us();
}


void VGA_360x240planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3624));
	}
	unlock_us();
}

void VGA_320x400planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3240));
	}
	unlock_us();
}

void VGA_320x240planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3224));
	}
	unlock_us();
}


void VGA_320x350planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3235));
	}
	unlock_us();
}

void VGA_360x350planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3635));
	}
	unlock_us();
}

void VGA_360x400planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3640));
	}
	unlock_us();
}

void VGA_320x480planar_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
	      (_THUNK_PROLOG(4);
	       _THUNK_LONG(iopl_addr);
	       _THUNK_CALL(3248));
	}
	unlock_us();
}


void VGA_640x480linear_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
		set_bank(0);
		memcpy(VGA_pagebase, dabuffer, 256 * 256);
		set_bank(1);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 1, 256 * 256);
		set_bank(2);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 2, 256 * 256);
		set_bank(3);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 3, 256 * 256);
		set_bank(4);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 4, 45056);
	}
	unlock_us();
}


void VGA_800x600linear_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
		set_bank(0);
		memcpy(VGA_pagebase, dabuffer, 256 * 256);
		set_bank(1);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 1, 256 * 256);
		set_bank(2);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 2, 256 * 256);
		set_bank(3);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 3, 256 * 256);
		set_bank(4);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 4, 256 * 256);
		set_bank(5);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 5, 256 * 256);
		set_bank(6);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 6, 256 * 256);
		set_bank(7);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 7, 21248);
	}
	unlock_us();
}

void VGA_1024x768linear_blast (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	lock_us();
	if (!background)
	{
		set_bank(0);
		memcpy(VGA_pagebase, dabuffer, 256 * 256);
		set_bank(1);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 1, 256 * 256);
		set_bank(2);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 2, 256 * 256);
		set_bank(3);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 3, 256 * 256);
		set_bank(4);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 4, 256 * 256);
		set_bank(5);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 5, 256 * 256);
		set_bank(6);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 6, 256 * 256);
		set_bank(7);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 7, 256 * 256);
		set_bank(8);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 8, 256 * 256);
		set_bank(9);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 9, 256 * 256);
		set_bank(10);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 10, 256 * 256);
		set_bank(11);
		memcpy(VGA_pagebase, dabuffer + 256 * 256 * 11, 256 * 256);
	}
	unlock_us();
}


/*
================
VGA_SwapBuffers
================
*/
void VGA_SwapBuffers (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	UNUSED(lvid);

	if (vid_wait.value == VID_WAIT_VSYNC)
//		_wait01 (IS1_R, 0x08);
		 wait_retrace();
}

void VGA_BeginDirectRect (viddef_t *vid, struct vmode_s *pcurrentmode, int x,
	int y, byte *pbitmap, int width, int height)
{
}

void VGA_EndDirectRect (viddef_t *vid, struct vmode_s *pcurrentmode, int x,
	int y, int width, int height)
{
}

