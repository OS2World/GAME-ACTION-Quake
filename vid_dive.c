// vid_null.h -- null video driver to aid porting efforts


#define WS_DesktopDive    0x00000000L   // Desktop dive window style
#define WS_MaxDesktopDive 0x00000001L   // Maximized desktop dive window style
#define WS_FullScreenDive 0x00000002L   // Full-screen 320x200x256 dive style

#define WM_GetVideoModeTable  0x04A2
#define WM_SetVideoMode       0x04A0
#define WM_NotifyVideoModeChange 0x04A1

#include "quakedef.h"
#include "d_local.h"

#define INCL_DOS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define  _MEERROR_H_
#include <mmioos2.h>                   /* It is from MMPM toolkit           */
#include <dive.h>
#include <fourcc.h>
#include "show.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <process.h>

HAB hab;
HMQ hmq;
PWINDATA pwinData = 0;
FOURCC    fccFormats[100] = {0};        /* Color format code                 */
DIVE_CAPS DiveCaps = {0};
int da_ready = 0;
ULONG ulImage = 0;
ULONG     flCreate;             /* Window creation control flags        */
QMSG qmsg;
ULONG pm_thread_id;
int quit_req = 0;

int fSwitching = 0;
int FullScreen = 0;

struct
{
	int key;
	int down;
} keyq[64];
int keyq_head=0;
int keyq_tail=0;

byte        scantokey[128] = 
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

MRESULT EXPENTRY MyWindowProc ( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY MyDlgProc ( HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2 );


viddef_t	vid;				// global video state

#define	BASEWIDTH	320
#define	BASEHEIGHT	200

byte	vid_buffer[BASEWIDTH*BASEHEIGHT];
short	zbuffer[BASEWIDTH*BASEHEIGHT];
byte	surfcache[512*1024];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

int InitMainWindow(void);

extern int swap_rgb;

unsigned char *fs_buffer = 0;

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	int		r,g,b,v;
	int		i;
	unsigned	*table;
	
	
//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	
	if (swap_rgb)
	{
		for (i=0 ; i<256 ; i++)
		{
			r = pal[0];
			g = pal[1];
			b = pal[2];
			pal += 3;	
			v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
			*table++ = v;
		}
	}
	else
	{
		for (i=0 ; i<256 ; i++)
		{
			r = pal[2];
			g = pal[1];
			b = pal[0];
			pal += 3;	
			v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
			*table++ = v;
		}
	}
	if (pwinData)
	         DiveSetSourcePalette ( pwinData->hDive, 0,
                                  256,
                                  d_8to24table);
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

void	VID_Init (unsigned char *palette)
{
	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	
	d_pzbuffer = zbuffer;

	if (InitMainWindow())
	{
		printf("Failed to create window\n");
		exit(1);
	}

	D_InitCaches (surfcache, sizeof(surfcache));
	VID_SetPalette (palette);
}

void	VID_Shutdown (void)
{
}

void	VID_Update (vrect_t *rects)
{
	while (fSwitching)
	{
		DosSleep(50);
	}
	if (pwinData &&  !pwinData->fDataInProcess)
	{
            DiveBlitImage ( pwinData->hDive,
       	                    ulImage,  
              	            DIVE_BUFFER_SCREEN );
	}
	else
	{
		DosSleep(20);
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

UCHAR     szErrorBuf[256];          // GameSrvr
HMODULE   hmodGameSrvr;             // GameSrvr
PFN       pfnInitGameFrameProc;     // GameSrvr


void pm_thread(void *bleah)
{
   hab = WinInitialize ( 0 );
   hmq = WinCreateMsgQueue ( hab, 0 );

   if (!hab && !hmq)
   {
	da_ready = 1;
	return;
   }

   /* Allocate a buffer for the window data
   */
   pwinData = (PWINDATA) malloc (sizeof(WINDATA));

   bzero(pwinData, sizeof(WINDATA));

   pwinData->ulHeight = BASEHEIGHT;
   pwinData->ulWidth = BASEWIDTH;

   DiveCaps.pFormatData = fccFormats;
   DiveCaps.ulFormatLength = 120;
   DiveCaps.ulStructLen = sizeof(DIVE_CAPS);

   if ( DiveQueryCaps ( &DiveCaps, DIVE_BUFFER_SCREEN ))
      {
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
          (PSZ)"usage: The sample program can not run on this system environment.",
          (PSZ)"SHOW.EXE - DIVE Sample", 0, MB_OK | MB_INFORMATION );
      free ( pwinData );
      WinDestroyMsgQueue ( hmq );
      WinTerminate ( hab );
      da_ready = 1;
      return;
      }

   if ( DiveCaps.ulDepth < 8 )
      {
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
          (PSZ)"usage: The sample program can not run on this system environment.",
          (PSZ)"SHOW.EXE - DIVE Sample", 0, MB_OK | MB_INFORMATION );
      free ( pwinData );
      WinDestroyMsgQueue ( hmq );
      WinTerminate ( hab );
      da_ready = 1;
      return;
      }

   /* Calculate number of bytes per pell
   */
   pwinData->ulColorSizeBytes = DiveCaps.ulScanLineBytes/
                                DiveCaps.ulHorizontalResolution;

   /* Get an instance of DIVE APIs.
   */
   if ( DiveOpen ( &(pwinData->hDive), FALSE, 0 ) )
      {

      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                       (PSZ)"usage: The sample program can not run on this system environment.",
                       (PSZ)"SHOW.EXE - DIVE Sample", 0, MB_OK | MB_INFORMATION );
      free ( pwinData );
      WinDestroyMsgQueue ( hmq );
      WinTerminate ( hab );
      da_ready = 1;
      return;
      }


   /* Register a window class, and create a standard window.
   */
   WinRegisterClass ( hab, "quakewindow", MyWindowProc, 0, sizeof(ULONG) );
   flCreate = FCF_TASKLIST | FCF_SYSMENU  | FCF_TITLEBAR | FCF_ICON |
                  FCF_SIZEBORDER | FCF_MINMAX | FCF_MENU | FCF_SHELLPOSITION;
   pwinData->hwndFrame = WinCreateStdWindow ( HWND_DESKTOP,
                                              WS_VISIBLE, &flCreate,
                                              "quakewindow",
                                              "quake",
                                              WS_SYNCPAINT | WS_VISIBLE,
                                              0, ID_MAINWND,
                                              &(pwinData->hwndClient));

    if ( 0 == DosLoadModule( szErrorBuf, 256, "GAMESRVR", &hmodGameSrvr ) )
    {
        if ( 0 == DosQueryProcAddr( hmodGameSrvr, 1, 0, &pfnInitGameFrameProc ) )
        {
            ( pfnInitGameFrameProc )( pwinData->hwndFrame, 0 );
        }
        {
            ULONG pvmi;
            ULONG ul;

            WinSendMsg( pwinData->hwndFrame, WM_GetVideoModeTable, (MPARAM)&pvmi, (MPARAM)&ul );
        }
    }
    else
    {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                (PSZ)"usage: FSDIVE failed to load GAMESRVR.DLL.",
                (PSZ)"Error!", 0, MB_OK | MB_MOVEABLE );
    }


   WinSetWindowULong (pwinData->hwndClient, 0, (ULONG)pwinData);

   pwinData->cxWidthBorder = (LONG) WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
   pwinData->cyWidthBorder = (LONG) WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
   pwinData->cyTitleBar    = (LONG) WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
   pwinData->cyMenu        = (LONG) WinQuerySysValue(HWND_DESKTOP, SV_CYMENU);

   pwinData->cxWidthWindow  = pwinData->ulWidth * 2 + pwinData->cxWidthBorder * 2;
   pwinData->cyHeightWindow = pwinData->ulHeight * 2 + (pwinData->cyWidthBorder * 2 +
                                       pwinData->cyTitleBar + pwinData->cyMenu );

   pwinData->cxWindowPos   = ( (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN)
                       - pwinData->cxWidthWindow ) / 2;
   pwinData->cyWindowPos   = ( (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN)
                       - pwinData->cyHeightWindow ) / 2;

   pwinData->fccColorFormat = FOURCC_LUT8;

   pwinData->ulSrcLineSizeBytes = ((( BASEWIDTH * ( 8 >> 3 )) + 3 ) / 4 ) * 4;

   if ( DiveAllocImageBuffer (pwinData->hDive,
                              &ulImage,
                              pwinData->fccColorFormat,
                              pwinData->ulWidth,
                              pwinData->ulHeight,
                              pwinData->ulSrcLineSizeBytes,
                              vid.buffer))
	{
		WinDestroyMsgQueue ( hmq );
		WinTerminate ( hab );
                da_ready = 1;
                return;	
	}

   WinSetWindowPos (pwinData->hwndFrame,
                      HWND_TOP,
                      pwinData->cxWindowPos, pwinData->cyWindowPos,
                      pwinData->cxWidthWindow, pwinData->cyHeightWindow,
                      SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ACTIVATE);

   WinSetVisibleRegionNotify ( pwinData->hwndClient, TRUE );

   /* set the flag for the first time simulation of palette of bitmap data
   */
   pwinData->fChgSrcPalette = FALSE;
   pwinData->fStartBlit = FALSE;
   pwinData->fDataInProcess = FALSE;
   pwinData->fDirect = FALSE;

   WinPostMsg ( pwinData->hwndFrame, WM_VRNENABLED, 0L, 0L );

   da_ready = 2;

   while ( WinGetMsg ( hab, &qmsg, 0, 0, 0 ) )
      WinDispatchMsg ( hab, &qmsg );

   DiveClose ( pwinData->hDive );
   return;
}

void fs_exit_func(void)
{
	if (FullScreen)
	{
		WinPostMsg(pwinData->hwndFrame, WM_SetVideoMode, 0, 0);
		while(FullScreen || fSwitching)
			DosSleep(50);
	}
}

int InitMainWindow(void)
{
	printf("InitMainWindow() - creating thread\n");
	fflush(stdout);
	DosCreateThread(&pm_thread_id, (PFNTHREAD) pm_thread, 0, 0, 163840);
	while (!da_ready)
		sleep(1);
	printf("InitMainWindow() - thread created, da_ready = %d\n", da_ready);
	fflush(stdout);
	if (da_ready == 1)
	{
		printf("exiting\n");
		fflush(stdout);
		exit(1);
	}
	atexit(fs_exit_func);
	return (0);
}

MRESULT EXPENTRY MyWindowProc ( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
   {
   POINTL    pointl;                /* Point to offset from Desktop         */
   HRGN      hrgn;                  /* Region handle                        */
   HPS       hps;                   /* Presentation Space handle            */
   RECTL     rcl;                   /* Current widow rectangle              */
   ULONG     ulTmpImage;            /* For tmp image number                 */
   RGNRECT   rgnCtl;                /* Processing control structure         */
   SETUP_BLITTER SetupBlitter;      /* structure for DiveSetupBlitter       */
   ULONG     ulScanLineBytes;       /* Size of scan line for current window */
   ULONG     ulScanLines;           /* Number of scan lines in window       */
   ULONG     ulRemainder;           /* Used to check direct mode support    */
   ULONG     i, rc;

   /* Get the pointer to window data
   */
   if ( pwinData )
      {
      switch( msg )
         {
         case WM_COMMAND:
            switch ( SHORT1FROMMP ( mp1 ) )
               {
               case ID_SNAP:
                  {
                  /* Use the initial width and height of the window such that
                  ** the actual video area equals the source width and height.
                  */

                  /* Set the new size of the window, but don't move it.
                  */
                  WinSetWindowPos ( pwinData->hwndFrame, HWND_TOP,
                                      100, 100,
                                      pwinData->cxWidthWindow,
                                      pwinData->cyHeightWindow,
                                      SWP_SIZE | SWP_ACTIVATE | SWP_SHOW );
                  }
                  break;

               case ID_QUERY:

                  /* Display screen capablilities in the dialog box
                  */
                  WinDlgBox ( HWND_DESKTOP, pwinData->hwndClient,
                               MyDlgProc, (HMODULE)0,
                               ID_DIALOG, (PVOID)pwinData );

                  break;

               case ID_EXIT:
                  /* Post to quit the dispatch message loop.
                  */
		  quit_req = 1;
                  break;

               case ID_NEWTEXT:
                  /* Write new text string to the title bar
                  */
                  WinSetWindowText ( pwinData->hwndFrame, (PSZ)mp2 );
                  break;

               case ID_DIRECT:

                  pwinData->fDataInProcess = TRUE;

		  WinPostMsg ( hwnd, WM_SetVideoMode, (MPARAM)2, 0);
                  break;

               case ID_USEDIVE:
                  pwinData->fDataInProcess = TRUE;

                  /* Set flag for dive blit mode
                  */
                  pwinData->fDirect = FALSE;

                  /* Post invalidation message.
                  */
                  WinPostMsg ( hwnd, WM_VRNENABLED, 0L, 0L );
                  break;

               default:
                  /* Let PM handle this message.
                  */
                  return WinDefWindowProc ( hwnd, msg, mp1, mp2 );
               }
            break;

         case WM_NotifyVideoModeChange:
		if( (ULONG)mp1 == 0 )
      		{
	         	fSwitching = TRUE;
	                pwinData->fDataInProcess = TRUE;
		}
	      	else
      		{
         // Determine the mode we are now in
         //
         		if( ( (ULONG)mp2 == 0 ) || ( (ULONG)mp2 == 1 ) )
         		{
            			FullScreen = FALSE;
		                DiveSetSourcePalette ( pwinData->hDive, 0, 256, d_8to24table);
         		}
         		else
         		{
            			FullScreen = TRUE;
		                DiveSetSourcePalette ( pwinData->hDive, 0, 256, d_8to24table);
         		}
	         // Indicate that we are done switching modes
	         //
			fSwitching = FALSE;
		        WinPostMsg( hwnd, WM_VRNENABLED, 0L, 0L );
      		}
		break;

         case WM_VRNDISABLED:

            pwinData->fDataInProcess = TRUE;
            DiveSetupBlitter ( pwinData->hDive, 0 );
            pwinData->fVrnDisabled = TRUE;
            break;

         case WM_VRNENABLED:

	    if( fSwitching ) 
		break;
            pwinData->fDataInProcess = TRUE;

	    if (FullScreen)
	    {
		pwinData->swp.cx = 320;
		pwinData->swp.cy = 200;
		pwinData->swp.x = 0;
		pwinData->swp.y = 0;
		pointl.x = 0;
		pointl.y = 0;
                pwinData->cxWindowPos = pointl.x;
       	        pwinData->cyWindowPos = pointl.y;
		pwinData->ulNumRcls = 1;
		pwinData->rcls[0].xLeft = 0;
		pwinData->rcls[0].xRight = 320;
		pwinData->rcls[0].yBottom = 0;
		pwinData->rcls[0].yTop = 200;
		WinSetCapture(HWND_DESKTOP, hwnd);
		WinSetPointerPos(HWND_DESKTOP, 100, 100);
            }
	    else
	    {
	            WinSetCapture(HWND_DESKTOP, 0);
	            hps = WinGetPS ( hwnd );
        	    if ( !hps )
	               break;
        	    hrgn = GpiCreateRegion ( hps, 0L, NULL );
	            if ( hrgn )
        	       {
	               /* NOTE: If mp1 is zero, then this was just a move message.
        	       ** Illustrate the visible region on a WM_VRNENABLE.
	               */
	               WinQueryVisibleRegion ( hwnd, hrgn );
        	       rgnCtl.ircStart     = 0;
	               rgnCtl.crc          = 50;
        	       rgnCtl.ulDirection  = 1;

	               /* Get the all ORed rectangles
        	       */
	               if ( GpiQueryRegionRects ( hps, hrgn, NULL,
        	                                  &rgnCtl, pwinData->rcls) )
                	  {
	                  pwinData->ulNumRcls = rgnCtl.crcReturned;
	
        	          /* Now find the window position and size, relative to parent.
                	  */
	                  WinQueryWindowPos ( pwinData->hwndClient, &pwinData->swp );
	
        	          rcl.xLeft   = 0;
	                  rcl.yBottom = 0;

	                  /* Convert the point to offset from desktop lower left.
        	          */
	                  pointl.x = pwinData->swp.x;
        	          pointl.y = pwinData->swp.y;

	                  WinMapWindowPoints ( pwinData->hwndFrame,
        	                               HWND_DESKTOP, &pointl, 1 );
	
	                  pwinData->cxWindowPos = pointl.x;
        	          pwinData->cyWindowPos = pointl.y;

	                  /* Calculate size of scan line bounded by visible window
        	          ** and round up to 4 pell increments.
                	  */
	                  i = pwinData->swp.cx * pwinData->ulColorSizeBytes;
        	          pwinData->ulLineSizeBytes = i/4*4;
                	  if (i - pwinData->ulLineSizeBytes)
	                     pwinData->ulLineSizeBytes += 4;
        	          }
	               else
        	          {
	                  DiveSetupBlitter ( pwinData->hDive, 0 );
        	          GpiDestroyRegion ( hps, hrgn );
                	  break;
	                  }
	               GpiDestroyRegion( hps, hrgn );
        	       }
            	WinReleasePS( hps );
	    }

            /* Tell DIVE about the new settings.
            */
            SetupBlitter.ulStructLen = sizeof ( SETUP_BLITTER );
            SetupBlitter.fccSrcColorFormat = FOURCC_LUT8;
            SetupBlitter.ulSrcWidth = pwinData->ulWidth;
            SetupBlitter.ulSrcHeight = pwinData->ulHeight;
            SetupBlitter.ulSrcPosX = 0;
            SetupBlitter.ulSrcPosY = 0;
            SetupBlitter.ulDitherType = 1;
            SetupBlitter.fInvert = TRUE;

            SetupBlitter.lDstPosX = 0;
            SetupBlitter.lDstPosY = 0;
            SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
            SetupBlitter.lScreenPosX = 0;
            SetupBlitter.lScreenPosY = 0;
            SetupBlitter.ulNumDstRects = 1;

            rcl.xRight = pwinData->ulWidth;
            rcl.yTop   = pwinData->ulHeight;
            SetupBlitter.ulDstWidth = pwinData->ulWidth;
            SetupBlitter.ulDstHeight = pwinData->ulHeight;

            SetupBlitter.lScreenPosX = pointl.x;
            SetupBlitter.lScreenPosY = pointl.y;
            SetupBlitter.fInvert = FALSE;
            SetupBlitter.ulDstWidth = pwinData->swp.cx;
            SetupBlitter.ulDstHeight = pwinData->swp.cy;
            SetupBlitter.ulNumDstRects = pwinData->ulNumRcls;
            SetupBlitter.pVisDstRects = pwinData->rcls;

            DiveSetupBlitter ( pwinData->hDive, &SetupBlitter );

            pwinData->fDataInProcess = FALSE;
            pwinData->fVrnDisabled = FALSE;

            break;

         case WM_REALIZEPALETTE:
            /* This tells DIVE that the physical palette may have changed.
            */
            DiveSetDestinationPalette ( pwinData->hDive, 0, 0, 0 );

            break;

         case WM_CLOSE:
            /* Post to quit the dispatch message loop.
            */
	    quit_req = 1;
            break;

         case WM_CHAR:
	{
		int down;
		CHRMSG *chr = CHARMSG(&msg);
		if (chr->fs & KC_SCANCODE && chr->scancode < 128)
		{
			if (chr->fs & KC_KEYUP)
				down = 0;
			else
				down = 1;
#if 0
			fprintf(stderr, "scancode = %d %s\n", chr->scancode, down ? "down" : "up");
			fflush(stderr);
#endif
			keyq[keyq_head].key = scantokey[chr->scancode];
			keyq[keyq_head].down = down;
			keyq_head = (keyq_head + 1) & 63;
		}
	}
	break;

         default:
            /* Let PM handle this message.
            */
            return WinDefWindowProc ( hwnd, msg, mp1, mp2 );
         }
      }
   else
      /* Let PM handle this message.
      */
      return WinDefWindowProc ( hwnd, msg, mp1, mp2 );

   return ( FALSE );
   }

MRESULT EXPENTRY MyDlgProc ( HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2 )
   {
   CHAR      string[10];
   CHAR      *pString;

   switch( msg )
      {
      case WM_INITDLG:

         WinSetFocus ( HWND_DESKTOP, hwndDlg );

         if ( !DiveCaps.fScreenDirect )
            WinSetDlgItemText ( hwndDlg, ID_EF_11, "NO" );
         else
            WinSetDlgItemText ( hwndDlg, ID_EF_11, "YES" );

         if ( !DiveCaps.fBankSwitched )
            WinSetDlgItemText ( hwndDlg, ID_EF_12, "NO" );
         else
            WinSetDlgItemText ( hwndDlg, ID_EF_12, "YES" );

         pString = _ultoa ( DiveCaps.ulDepth, string, 10 );
         WinSetDlgItemText ( hwndDlg, ID_EF_13, pString );

         pString = _ultoa ( DiveCaps.ulHorizontalResolution,
                             string, 10 );
         WinSetDlgItemText ( hwndDlg, ID_EF_14, pString );

         pString = _ultoa ( DiveCaps.ulVerticalResolution, string, 10 );
         WinSetDlgItemText ( hwndDlg, ID_EF_15, pString );

         pString = _ultoa ( DiveCaps.ulScanLineBytes, string, 10 );
         WinSetDlgItemText ( hwndDlg, ID_EF_16, pString );

         switch (DiveCaps.fccColorEncoding)
            {
            case FOURCC_LUT8:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "256" );
               break;
            case FOURCC_R565:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "64K" );
               break;
            case FOURCC_R555:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "32K" );
               break;
            case FOURCC_R664:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "64K" );
               break;
            case FOURCC_RGB3:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "16M" );
               break;
            case FOURCC_BGR3:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "16M" );
               break;
            case FOURCC_RGB4:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "16M" );
               break;
            case FOURCC_BGR4:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "16M" );
               break;
            default:
               WinSetDlgItemText ( hwndDlg, ID_EF_17, "???" );
            } /* endswitch */

         pString = _ultoa ( DiveCaps.ulApertureSize, string, 10 );
         WinSetDlgItemText ( hwndDlg, ID_EF_18, pString );

         pString = _ultoa ( DiveCaps.ulInputFormats, string, 10 );
         WinSetDlgItemText ( hwndDlg, ID_EF_19, pString );

         pString = _ultoa ( DiveCaps.ulOutputFormats, string, 10 );
         WinSetDlgItemText ( hwndDlg, ID_EF_20, pString );

         break;

      case WM_COMMAND:
         switch ( SHORT1FROMMP ( mp1 ) )
            {
            case DID_OK:

               WinDismissDlg ( hwndDlg, TRUE );
               break;
            }

      default:
         return ( WinDefDlgProc (hwndDlg, msg, mp1, mp2) );

      }

   return( 0 );

   }

void Sys_SendKeyEvents(void)
{
	while (keyq_head != keyq_tail)
	{
		Key_Event(keyq[keyq_tail].key, keyq[keyq_tail].down);
		keyq_tail = (keyq_tail + 1) & 63;
	}
}

