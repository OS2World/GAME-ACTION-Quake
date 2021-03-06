//
// vgamodes.h: VGA mode set tables
//

#include "vregset.h"

int		VGA_InitMode (viddef_t *vid, vmode_t *pcurrentmode);
void	VGA_SwapBuffers (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void	VGA_SetPalette (viddef_t *vid, vmode_t *pcurrentmode,
						unsigned char *pal);

void VGA_320x200linear_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_320x240planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_320x400planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_360x200planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_360x240planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_360x480planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_320x350planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_360x350planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_360x400planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_320x480planar_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_640x480linear_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_800x600linear_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);
void VGA_1024x768linear_blast (viddef_t *vid, vmode_t *pcurrentmode, vrect_t *rects);

///////////////////////////////////////////////////////////////////////////
// the following base mode descriptors plus extra data together provide all
// the data needed to do VGA mode sets
///////////////////////////////////////////////////////////////////////////

typedef struct {
	int		vidbuffer;
	int		*pregset;
} vextra_t;

int	vrsnull[] = {
	VRS_END,
};

int vrs320x200x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_BYTE_OUT, SC_INDEX, MEMORY_MODE,
	VRS_BYTE_RMW, SC_DATA, ~0x08, 0x04,
	VRS_BYTE_OUT, GC_INDEX, GRAPHICS_MODE,
	VRS_BYTE_RMW, GC_DATA, ~0x13, 0x00,
	VRS_BYTE_OUT, GC_INDEX, MISCELLANOUS,
	VRS_BYTE_RMW, GC_DATA, ~0x02, 0x00,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// change the CRTC from doubleword to byte mode
//
	VRS_BYTE_OUT, CRTC_INDEX, UNDERLINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x40, 0x00,
	VRS_BYTE_OUT, CRTC_INDEX, MODE_CONTROL,
	VRS_BYTE_RMW, CRTC_DATA, ~0x00, 0x40,

	VRS_END,
};

int vrs360x200x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_WORD_OUT, SC_INDEX, 0x0604,
	VRS_BYTE_OUT, MISC_OUTPUT, 0x67,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// change the CRTC from doubleword to byte mode
//
	VRS_BYTE_OUT, CRTC_INDEX, UNDERLINE,
	VRS_BYTE_RMW, CRTC_DATA,  ~0x40, 0x00,
	VRS_BYTE_OUT, CRTC_INDEX, MODE_CONTROL,
	VRS_BYTE_RMW, CRTC_DATA,  ~0x00, 0x40,

//
// set up the CRT Controller
//
	VRS_WORD_OUT, CRTC_INDEX, 0x6B00,
	VRS_WORD_OUT, CRTC_INDEX, 0x5901,
	VRS_WORD_OUT, CRTC_INDEX, 0x5A02,
	VRS_WORD_OUT, CRTC_INDEX, 0x8E03,
	VRS_WORD_OUT, CRTC_INDEX, 0x5E04,
	VRS_WORD_OUT, CRTC_INDEX, 0x8A05,
	VRS_WORD_OUT, CRTC_INDEX, 0x3013,

	VRS_END,
};

int vrs320x240x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_BYTE_OUT, SC_INDEX, MEMORY_MODE,
	VRS_BYTE_RMW, SC_DATA, ~0x08, 0x04,
	VRS_BYTE_OUT, GC_INDEX, GRAPHICS_MODE,
	VRS_BYTE_RMW, GC_DATA, ~0x13, 0x00,
	VRS_BYTE_OUT, GC_INDEX, MISCELLANOUS,
	VRS_BYTE_RMW, GC_DATA, ~0x02, 0x00,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// set up the CRT Controller
//
	VRS_WORD_OUT, CRTC_INDEX, 0x0D06,
	VRS_WORD_OUT, CRTC_INDEX, 0x3E07,
	VRS_WORD_OUT, CRTC_INDEX, 0x4109,
	VRS_WORD_OUT, CRTC_INDEX, 0xEA10,
	VRS_WORD_OUT, CRTC_INDEX, 0xAC11,
	VRS_WORD_OUT, CRTC_INDEX, 0xDF12,
	VRS_WORD_OUT, CRTC_INDEX, 0x0014,
	VRS_WORD_OUT, CRTC_INDEX, 0xE715,
	VRS_WORD_OUT, CRTC_INDEX, 0x0616,
	VRS_WORD_OUT, CRTC_INDEX, 0xE317,

	VRS_END,
};

int vrs360x240x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_WORD_OUT, SC_INDEX, 0x0604,
	VRS_BYTE_OUT, MISC_OUTPUT, 0xE7,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// set up the CRT Controller
//
	VRS_WORD_OUT, CRTC_INDEX, 0x6B00,
	VRS_WORD_OUT, CRTC_INDEX, 0x5901,
	VRS_WORD_OUT, CRTC_INDEX, 0x5A02,
	VRS_WORD_OUT, CRTC_INDEX, 0x8E03,
	VRS_WORD_OUT, CRTC_INDEX, 0x5E04,
	VRS_WORD_OUT, CRTC_INDEX, 0x8A05,
	VRS_WORD_OUT, CRTC_INDEX, 0x0D06,
	VRS_WORD_OUT, CRTC_INDEX, 0x3E07,
	VRS_WORD_OUT, CRTC_INDEX, 0x4109,
	VRS_WORD_OUT, CRTC_INDEX, 0xEA10,
	VRS_WORD_OUT, CRTC_INDEX, 0xAC11,
	VRS_WORD_OUT, CRTC_INDEX, 0xDF12,
	VRS_WORD_OUT, CRTC_INDEX, 0x3013,
	VRS_WORD_OUT, CRTC_INDEX, 0x0014,
	VRS_WORD_OUT, CRTC_INDEX, 0xE715,
	VRS_WORD_OUT, CRTC_INDEX, 0x0616,
	VRS_WORD_OUT, CRTC_INDEX, 0xE317,

	VRS_END,
};

int vrs320x350x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_BYTE_OUT, SC_INDEX, MEMORY_MODE,
	VRS_BYTE_RMW, SC_DATA, ~0x08, 0x04,
	VRS_BYTE_OUT, GC_INDEX, GRAPHICS_MODE,
	VRS_BYTE_RMW, GC_DATA, ~0x10, 0x00,
	VRS_BYTE_OUT, GC_INDEX, MISCELLANOUS,
	VRS_BYTE_RMW, GC_DATA, ~0x02, 0x00,
	VRS_BYTE_OUT, MISC_OUTPUT, 0xA3,	// 350-scan-line scan rate

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// stop scanning each line twice
//
	VRS_BYTE_OUT, CRTC_INDEX, MAX_SCAN_LINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x1F, 0x00,

//
// change the CRTC from doubleword to byte mode
//
	VRS_BYTE_OUT, CRTC_INDEX, UNDERLINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x40, 0x00,
	VRS_BYTE_OUT, CRTC_INDEX, MODE_CONTROL,
	VRS_BYTE_RMW, CRTC_DATA, ~0x00, 0x40,

//
// set the vertical counts for 350-scan-line mode
//
	VRS_WORD_OUT, CRTC_INDEX, 0xBF06,
	VRS_WORD_OUT, CRTC_INDEX, 0x1F07,
	VRS_WORD_OUT, CRTC_INDEX, 0x8310,
	VRS_WORD_OUT, CRTC_INDEX, 0x8511,
	VRS_WORD_OUT, CRTC_INDEX, 0x5D12,
	VRS_WORD_OUT, CRTC_INDEX, 0x6315,
	VRS_WORD_OUT, CRTC_INDEX, 0xBA16,

	VRS_END,
};

int vrs360x350x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_WORD_OUT, SC_INDEX, 0x0604,
	VRS_BYTE_OUT, MISC_OUTPUT, 0xA7,	// 350-scan-line scan rate

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// stop scanning each line twice
//
	VRS_BYTE_OUT, CRTC_INDEX, MAX_SCAN_LINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x1F, 0x00,

//
// change the CRTC from doubleword to byte mode
//
	VRS_BYTE_OUT, CRTC_INDEX, UNDERLINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x40, 0x00,
	VRS_BYTE_OUT, CRTC_INDEX, MODE_CONTROL,
	VRS_BYTE_RMW, CRTC_DATA, ~0x00, 0x40,

//
// set the vertical counts for 350-scan-line mode and 360 pixels across
//
	VRS_WORD_OUT, CRTC_INDEX, 0x6B00,
	VRS_WORD_OUT, CRTC_INDEX, 0x5901,
	VRS_WORD_OUT, CRTC_INDEX, 0x5A02,
	VRS_WORD_OUT, CRTC_INDEX, 0x8E03,
	VRS_WORD_OUT, CRTC_INDEX, 0x5E04,
	VRS_WORD_OUT, CRTC_INDEX, 0x8A05,
	VRS_WORD_OUT, CRTC_INDEX, 0xBF06,
	VRS_WORD_OUT, CRTC_INDEX, 0x1F07,
	VRS_WORD_OUT, CRTC_INDEX, 0x8310,
	VRS_WORD_OUT, CRTC_INDEX, 0x8511,
	VRS_WORD_OUT, CRTC_INDEX, 0x5D12,
	VRS_WORD_OUT, CRTC_INDEX, 0x3013,
	VRS_WORD_OUT, CRTC_INDEX, 0x6315,
	VRS_WORD_OUT, CRTC_INDEX, 0xBA16,

	VRS_END,
};

int vrs320x400x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,


	VRS_BYTE_OUT, SC_INDEX, MEMORY_MODE,
	VRS_BYTE_RMW, SC_DATA, ~0x08, 0x04,
	VRS_BYTE_OUT, GC_INDEX, GRAPHICS_MODE,
	VRS_BYTE_RMW, GC_DATA, ~0x10, 0x00,
	VRS_BYTE_OUT, GC_INDEX, MISCELLANOUS,
	VRS_BYTE_RMW, GC_DATA, ~0x02, 0x00,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// stop scanning each line twice
//
	VRS_BYTE_OUT, CRTC_INDEX, MAX_SCAN_LINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x1F, 0x00,

//
// change the CRTC from doubleword to byte mode
//
	VRS_BYTE_OUT, CRTC_INDEX, UNDERLINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x40, 0x00,
	VRS_BYTE_OUT, CRTC_INDEX, MODE_CONTROL,
	VRS_BYTE_RMW, CRTC_DATA, ~0x00, 0x40,

	VRS_END,
};

int vrs360x400x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_WORD_OUT, SC_INDEX, 0x0604,
	VRS_BYTE_OUT, MISC_OUTPUT, 0x67,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// stop scanning each line twice
//
	VRS_BYTE_OUT, CRTC_INDEX, MAX_SCAN_LINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x1F, 0x00,

//
// change the CRTC from doubleword to byte mode
//
	VRS_BYTE_OUT, CRTC_INDEX, UNDERLINE,
	VRS_BYTE_RMW, CRTC_DATA,  ~0x40, 0x00,
	VRS_BYTE_OUT, CRTC_INDEX, MODE_CONTROL,
	VRS_BYTE_RMW, CRTC_DATA,  ~0x00, 0x40,

//
// set up the CRT Controller
//
	VRS_WORD_OUT, CRTC_INDEX, 0x6B00,
	VRS_WORD_OUT, CRTC_INDEX, 0x5901,
	VRS_WORD_OUT, CRTC_INDEX, 0x5A02,
	VRS_WORD_OUT, CRTC_INDEX, 0x8E03,
	VRS_WORD_OUT, CRTC_INDEX, 0x5E04,
	VRS_WORD_OUT, CRTC_INDEX, 0x8A05,
	VRS_WORD_OUT, CRTC_INDEX, 0x3013,

	VRS_END,
};

int vrs320x480x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_BYTE_OUT, SC_INDEX, MEMORY_MODE,
	VRS_BYTE_RMW, SC_DATA, ~0x08, 0x04,
	VRS_BYTE_OUT, GC_INDEX, GRAPHICS_MODE,
	VRS_BYTE_RMW, GC_DATA, ~0x10, 0x00,
	VRS_BYTE_OUT, GC_INDEX, MISCELLANOUS,
	VRS_BYTE_RMW, GC_DATA, ~0x02, 0x00,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// stop scanning each line twice
//
	VRS_BYTE_OUT, CRTC_INDEX, MAX_SCAN_LINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x1F, 0x00,

//
// change the CRTC from doubleword to byte mode
//
	VRS_BYTE_OUT, CRTC_INDEX, UNDERLINE,
	VRS_BYTE_RMW, CRTC_DATA, ~0x40, 0x00,
	VRS_BYTE_OUT, CRTC_INDEX, MODE_CONTROL,
	VRS_BYTE_RMW, CRTC_DATA, ~0x00, 0x40,

//
// set up the CRT Controller
//
	VRS_WORD_OUT, CRTC_INDEX, 0x0D06,
	VRS_WORD_OUT, CRTC_INDEX, 0x3E07,
	VRS_WORD_OUT, CRTC_INDEX, 0xEA10,
	VRS_WORD_OUT, CRTC_INDEX, 0xAC11,
	VRS_WORD_OUT, CRTC_INDEX, 0xDF12,
	VRS_WORD_OUT, CRTC_INDEX, 0xE715,
	VRS_WORD_OUT, CRTC_INDEX, 0x0616,

	VRS_END,
};

int vrs360x480x256planar[] = {
//
// switch to linear, non-chain4 mode
//
	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  1,

	VRS_WORD_OUT, SC_INDEX, 0x0604,
	VRS_BYTE_OUT, MISC_OUTPUT, 0xE7,

	VRS_BYTE_OUT, SC_INDEX, SYNC_RESET,
	VRS_BYTE_OUT, SC_DATA,  3,

//
// unprotect CRTC0 through CRTC0
//
	VRS_BYTE_OUT, CRTC_INDEX, 0x11,
	VRS_BYTE_RMW, CRTC_DATA, ~0x80, 0x00,

//
// set up the CRT Controller
//
	VRS_WORD_OUT, CRTC_INDEX, 0x6B00,
	VRS_WORD_OUT, CRTC_INDEX, 0x5901,
	VRS_WORD_OUT, CRTC_INDEX, 0x5A02,
	VRS_WORD_OUT, CRTC_INDEX, 0x8E03,
	VRS_WORD_OUT, CRTC_INDEX, 0x5E04,
	VRS_WORD_OUT, CRTC_INDEX, 0x8A05,
	VRS_WORD_OUT, CRTC_INDEX, 0x0D06,
	VRS_WORD_OUT, CRTC_INDEX, 0x3E07,
	VRS_WORD_OUT, CRTC_INDEX, 0x4009,
	VRS_WORD_OUT, CRTC_INDEX, 0xEA10,
	VRS_WORD_OUT, CRTC_INDEX, 0xAC11,
	VRS_WORD_OUT, CRTC_INDEX, 0xDF12,
	VRS_WORD_OUT, CRTC_INDEX, 0x3013,
	VRS_WORD_OUT, CRTC_INDEX, 0x0014,
	VRS_WORD_OUT, CRTC_INDEX, 0xE715,
	VRS_WORD_OUT, CRTC_INDEX, 0x0616,
	VRS_WORD_OUT, CRTC_INDEX, 0xE317,

	VRS_END,
};

//
// extra VGA-specific data for vgavidmodes
//
vextra_t	extra320x200x256linear = {
	1, vrsnull
};
vextra_t	extra320x200x256planar = {
	1, vrs320x200x256planar
};
vextra_t	extra360x200x256planar = {
	1, vrs360x200x256planar
};
vextra_t	extra320x240x256planar = {
	1, vrs320x240x256planar
};
vextra_t	extra360x240x256planar = {
	1, vrs360x240x256planar
};
vextra_t	extra320x350x256planar = {
	1, vrs320x350x256planar
};
vextra_t	extra360x350x256planar = {
	1, vrs360x350x256planar
};
vextra_t	extra320x400x256planar = {
	1, vrs320x400x256planar
};
vextra_t	extra360x400x256planar = {
	1, vrs360x400x256planar
};
vextra_t	extra320x480x256planar = {
	1, vrs320x480x256planar
};
vextra_t	extra360x480x256planar = {
	1, vrs360x480x256planar
};

//
// base mode descriptors, in ascending order of number of pixels
//

vmode_t	vgavidmodes[] = {
{
	NULL,
	"320x200", "    ***** standard VGA modes *****    ",
	320, 200, (200.0/320.0)*(320.0/240.0), 320, 0, 1, NULL,
	VGA_InitMode, VGA_320x200linear_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"360x200", NULL, 360, 200, (200.0/360.0)*(320.0/240.0),
	384, 1, 1, &extra360x200x256planar, VGA_InitMode,
	VGA_360x200planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"320x240", NULL, 320, 240, (240.0/320.0)*(320.0/240.0),
	320, 1, 1, &extra320x240x256planar, VGA_InitMode,
	VGA_320x240planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"360x240", NULL, 360, 240, (240.0/360.0)*(320.0/240.0),
	384, 1, 1, &extra360x240x256planar,
	VGA_InitMode, VGA_360x240planar_blast, VGA_SetPalette,
	VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"320x350", NULL, 320, 350, (350.0/320.0)*(320.0/240.0),
	320, 1, 1, &extra320x350x256planar, VGA_InitMode,
	VGA_320x350planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"360x350", NULL, 360, 350, (350.0/360.0)*(320.0/240.0),
	384, 1, 1, &extra360x350x256planar, VGA_InitMode,
	VGA_360x350planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"320x400", NULL, 320, 400, (400.0/320.0)*(320.0/240.0), 320,
	1, 1, &extra320x400x256planar, VGA_InitMode,
	VGA_320x400planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"360x400", NULL, 360, 400, (400.0/360.0)*(320.0/240.0),
	384, 1, 1, &extra360x400x256planar, VGA_InitMode,
	VGA_360x400planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"320x480", NULL, 320, 480, (480.0/320.0)*(320.0/240.0),
	320, 1, 1, &extra320x480x256planar, VGA_InitMode,
	VGA_320x480planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"360x480", NULL, 360, 480, (480.0/360.0)*(320.0/240.0),
	384, 1, 1, &extra360x480x256planar, VGA_InitMode,
	VGA_360x480planar_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"640x480", NULL, 640, 480, (480.0/640.0)*(320.0/240.0),
	640, 0, 1, NULL, VGA_InitMode,
	VGA_640x480linear_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"800x600", NULL, 800, 600, (600.0/800.0)*(320.0/240.0),
	800, 0, 1, NULL, VGA_InitMode,
	VGA_800x600linear_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
{
	NULL,
	"1024x768", NULL, 1024, 768, (768.0/1024.0)*(320.0/240.0),
	1024, 0, 1, NULL, VGA_InitMode,
	VGA_1024x768linear_blast, 
	VGA_SetPalette, VGA_BeginDirectRect, VGA_EndDirectRect
},
};

