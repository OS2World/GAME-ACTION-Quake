// sys_null.h -- null system driver to aid porting efforts

#include "winquake.h"
#include "errno.h"

int	starttime;

/*
===============================================================================

FILE IO

===============================================================================
*/

#define	MAX_HANDLES		10
FILE	*sys_handles[MAX_HANDLES];

int		findhandle (void)
{
	int		i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE	*f;
	int		i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE	*f;
	int		i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], SEEK_SET, position);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int	Sys_FileTime (char *path)
{
	FILE	*f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_DebugLog(char *file, char *fmt, ...)
{
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

    MessageBox(NULL, text, "Error", 0 /* MB_OK */ );

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{
	exit (0);
}

double Sys_FloatTime (void)
{
	double t;

	t = (clock()-starttime)*1.0/1024;
	
	return t;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
}


void Sys_SendKeyEvents (void)
{
    MSG        msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/


/*
==================
WinMain

==================
*/
char	*hackargv[] = {"winquake", "-game", "test_jc"};
HINSTANCE	global_hInstance;
int			global_nCmdShow;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG        msg;
	quakeparms_t	parms;
	int			time, oldtime;

    /* previous instances do not exist in Win32 */
    if (hPrevInstance)
        return 0;

	global_hInstance = hInstance;
	global_nCmdShow = nCmdShow;

	parms.memsize = 16384*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = "f:\\quake";

	parms.argc = sizeof(hackargv)/sizeof(hackargv[0]);
	parms.argv = hackargv;

	COM_InitArgv (parms.argc, parms.argv);

	printf ("Host_Init\n");
	Host_Init (&parms);

	oldtime = starttime = clock();

    /* main window message loop */
	while (1)
	{
			while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
			{
			    if (!GetMessage (&msg, NULL, 0, 0))
					break;
      			TranslateMessage (&msg);
      			DispatchMessage (&msg);
			}
	
			time = clock();
			Host_Frame ( 1.0/1024*(time - oldtime) );
			oldtime = time;
	}

    /* return success of application */
    return TRUE;
}

