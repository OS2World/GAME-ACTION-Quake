// vid_null.h -- null video driver to aid porting efforts

#include "winquake.h"

// disable data conversion warnings

#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
  

#include <gl\gl.h>
#include <gl\glu.h>

HGLRC	baseRC;
HWND	mainwindow;

HWND WINAPI InitializeWindow (HINSTANCE hInstance, int nCmdShow);

#include "d_local.h"



viddef_t	vid;				// global video state

#define	BASEWIDTH	320
#define	BASEHEIGHT	200

byte	vid_buffer[BASEWIDTH*BASEHEIGHT];
short	zbuffer[BASEWIDTH*BASEHEIGHT];
byte	surfcache[600*1024];
unsigned	vid_32[BASEWIDTH*BASEHEIGHT];
unsigned	*vid_glbitmap = vid_32;

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

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
	
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
//		v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
//		v = (255<<0) + (r<<8) + (g<<16) + (b<<24);
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette (palette);
}

void	VID_Init (unsigned char *palette)
{
    if (!(mainwindow = InitializeWindow (global_hInstance, global_nCmdShow)))
        Sys_Error ("Failed to init window");

	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	
	d_pzbuffer = zbuffer;
	D_InitCaches (surfcache, sizeof(surfcache));
	VID_SetPalette (palette);
}


void	VID_Shutdown (void)
{
}

void	VID_Update (vrect_t *rects)
{
	int		x,y;
	byte	*src;
	unsigned	*dest;
	RECT		rect;

	src = vid.buffer;
	dest = vid_32 + (vid.height-1) * vid.width;
	for (y=0 ; y<vid.height ; y++)
	{
		for (x=0 ; x<vid.width ; x++)
			dest[x] = d_8to24table[src[x]];
		src += vid.width;
		dest -= vid.width;
	}

	rect.left = 0;
	rect.right = vid.width;
	rect.bottom = 0;
	rect.top = vid.height;
	InvalidateRect (mainwindow, &rect, false);
    UpdateWindow (mainwindow);
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



//==========================================================================


BOOL bSetupPixelFormat(HDC hDC)
{
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,				// version number
	PFD_DRAW_TO_WINDOW 		// support window
	|  PFD_SUPPORT_OPENGL 	// support OpenGL
	|  PFD_DOUBLEBUFFER ,	// double buffered
	PFD_TYPE_RGBA,			// RGBA type
	24,				// 24-bit color depth
	0, 0, 0, 0, 0, 0,		// color bits ignored
	0,				// no alpha buffer
	0,				// shift bit ignored
	0,				// no accumulation buffer
	0, 0, 0, 0, 			// accum bits ignored
	32,				// 32-bit z-buffer	
	0,				// no stencil buffer
	0,				// no auxiliary buffer
	PFD_MAIN_PLANE,			// main layer
	0,				// reserved
	0, 0, 0				// layer masks ignored
    };
    int pixelformat;

    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
    {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    if (SetPixelFormat(hDC, pixelformat, &pfd) == FALSE)
    {
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    return TRUE;
}



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
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 

byte        shiftscantokey[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '!',    '@',    '#',    '$',    '%',    '^', 
	'&',    '*',    '(',    ')',    '_',    '+',    K_BACKSPACE, 9, // 0 
	'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I', 
	'O',    'P',    '{',    '}',    13 ,    K_CTRL,'A',  'S',      // 1 
	'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':', 
	'"' ,    '~',    K_SHIFT,'|',  'Z',    'X',    'C',    'V',      // 2 
	'B',    'N',    'M',    '<',    '>',    '?',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10,0  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'_',K_LEFTARROW,'%',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 


/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int MapKey (int key)
{
	key = (key>>16)&255;
	if (key > 127)
		return 0;
	return scantokey[key];
}

/*
===================================================================

MAIN WINDOW

===================================================================
*/

/* main window procedure */
LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG    lRet = 1;
	int		fwKeys, xPos, yPos;
    RECT	rect;

    GetClientRect(hWnd, &rect);

    switch (uMsg)
    {
	case WM_CREATE:
		{
			HDC	  hDC;

            hDC = GetDC(hWnd);
	    	bSetupPixelFormat(hDC);

            baseRC = wglCreateContext( hDC );
			if (!baseRC)
				Sys_Error ("wglCreateContect failed");
            if (!wglMakeCurrent( hDC, baseRC ))
				Sys_Error ("wglMakeCurrent failed");
		}
		break;

    	case WM_PAINT:
        { 
		    HDC		hDC;
		    PAINTSTRUCT	ps;

		    hDC = BeginPaint(hWnd, &ps);
            if (!wglMakeCurrent( hDC, baseRC ))
				Sys_Error ("wglMakeCurrent failed");
glScissor(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
glViewport(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
glClearColor (1, 0.5, 0.3, 0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// glRasterPos2i(100, 100);
glDisable (GL_DEPTH_TEST);
glDrawPixels(320, 200, GL_RGBA, GL_UNSIGNED_BYTE, vid_glbitmap);

		    hDC = wglGetCurrentDC();
		    SwapBuffers(hDC);
		    EndPaint(hWnd, &ps);
        }
		break;

		case WM_KEYDOWN:
			Key_Event (MapKey(lParam), true);
			break;
			
		case WM_KEYUP:
			Key_Event (MapKey(lParam), false);
			break;

    	case WM_SIZE:
//			View_Reshape (rect.right, rect.bottom);
//			PostMessage (hWnd, WM_PAINT, 0, 0L);
            break;

        // The WM_QUERYNEWPALETTE message informs a window that it is about to
        // receive input focus. In response, the window receiving focus should
        // realize its palette as a foreground palette and update its client
        // area. If the window realizes its palette, it should return TRUE;
        // otherwise, it should return FALSE.


   	    case WM_CLOSE:
        {
    	    HGLRC hRC;
    	    HDC	  hDC;

                /* release and free the device context and rendering context */
    	    hRC = wglGetCurrentContext();
    	    hDC = wglGetCurrentDC();

    	    wglMakeCurrent(NULL, NULL);

    	    if (hRC)
    	    	wglDeleteContext(hRC);
    	    if (hDC)
    	        ReleaseDC(hWnd, hDC);

            /* call destroy window to cleanup and go away */
            DestroyWindow (hWnd);
        }
        break;

   	    case WM_DESTROY:
        {
    	    HGLRC hRC;
    	    HDC	  hDC;

                /* release and free the device context and rendering context */
    	    hRC = wglGetCurrentContext();
    	    hDC = wglGetCurrentDC();

    	    wglMakeCurrent(NULL, NULL);

    	    if (hRC)
    	    	wglDeleteContext(hRC);
    	    if (hDC)
    	        ReleaseDC(hWnd, hDC);

            PostQuitMessage (0);
        }
        break;

    	default:
            /* pass all unhandled messages to DefWindowProc */
            lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
        break;
    }

    /* return 1 if handled message, 0 if not */
    return lRet;
}


/*
================
InitializeWindow
================
*/
HWND WINAPI InitializeWindow (HINSTANCE hInstance, int nCmdShow)
    {
    WNDCLASS   wc;
    HWND       hWnd;

    /* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = 0;
    wc.lpszClassName = "WinQuake";

    if (!RegisterClass (&wc) )
        return FALSE;

    hWnd = CreateWindow ("WinQuake",
         "WinQuake",
	     WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
         0, 0, 328, 226,
         NULL,
         NULL,
         hInstance,
         NULL);
    if (!hWnd)
		Sys_Error ("Couldn't create main window");

    ShowWindow (hWnd, SW_SHOWDEFAULT);
    UpdateWindow (hWnd);

    return hWnd;
}
