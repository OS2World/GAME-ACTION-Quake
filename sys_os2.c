// sys_null.h -- null system driver to aid porting efforts

#include "quakedef.h"
#include "errno.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#define INCL_DOS
#include <os2.h>

/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_FileOpenRead (char *path, int *handle)
{
	int	h;
	struct stat	fileinfo;
    
	h = open (path, O_RDONLY|O_BINARY, 0666);
	*handle = h;
	if (h == -1)
		return -1;
	
	if (fstat (h,&fileinfo) == -1)
		Sys_Error ("Error fstating %s", path);

	return fileinfo.st_size;
}

int Sys_FileOpenWrite (char *path)
{
	int     handle;

	umask (0);
	
	handle = open(path,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	, 0666);

	if (handle == -1)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));

	return handle;
}

void Sys_FileClose (int handle)
{
	close (handle);
}

void Sys_FileSeek (int handle, int position)
{
	lseek (handle, position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
   return read (handle, dest, count);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return write (handle, data, count);
}

int	Sys_FileTime (char *path)
{
	struct	stat	buf;
	
	if (stat (path,&buf) == -1)
		return -1;
	
	return buf.st_mtime;
}

void Sys_mkdir (char *path)
{
	mkdir (path, 0777);
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

	Host_Shutdown ();

	printf ("I_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
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
	Host_Shutdown ();
	exit (0);
}

double Sys_FloatTime (void)
{
    ULONG ms;
    
    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &ms, 4); 
    return ms/1000.0;
}

static qboolean		isDedicated;

char *Sys_ConsoleInput(void)
{
	static char	text[256];
	static int	len = 0;
	char		ch;
	int		key;

	if (!isDedicated)
		return NULL;

	key = _read_kbd(1,0,0);
	if (key == -1)
		return NULL;

	ch = key;
	if (!ch)
	{
		_read_kbd(1,0,0);
		return NULL;
	}

	switch (ch)
	{
		case '\r':
			putchar('\n');
			if (len)
			{
				text[len] = 0;
				len = 0;
				return text;
			}
			break;

		case '\b':
			putchar(' ');
			if (len)
			{
				len--;
				putchar('\b');
			}
			break;

		default:
			text[len] = ch;
			len = (len + 1) & 0xff;
			break;
	}

	return NULL;
}


void Sys_Sleep (void)
{
}

#if !id386

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

#endif

void floating_point_exception_handler(int whatever)
{
        fprintf(stderr,"floating point exception\n");
	fflush(stderr);
	signal(SIGFPE, floating_point_exception_handler);
}

void Sys_Init(void)
{
#if id386
	Sys_SetFPCW();
#endif
}

int be_sleepy = 0;

//=============================================================================

void main (int argc, char **argv)
{
	double			time, oldtime, newtime;
	quakeparms_t	parms;
	int j, tsize;


	COM_InitArgv (argc, argv);
	j = COM_CheckParm("-mem");
	if (j)
	{
		parms.memsize = (int) (Q_atof(com_argv[j+1]) * 1024 * 1024);
	}
	else
		parms.memsize = 12 * 1024 * 1024;

	parms.membase = malloc (parms.memsize);
	parms.basedir = ".";
	parms.cachedir = 0;

	parms.argc = com_argc;
	parms.argv = com_argv;

	printf ("Host_Init\n");
	Host_Init (&parms);

	MaskExceptions ();

	isDedicated = (COM_CheckParm ("-dedicated") != 0);
	be_sleepy = (COM_CheckParm ("-bnice") != 0);

	Sys_Init();

	signal(SIGFPE, SIG_IGN);
	signal(SIGSEGV, SIG_IGN);

	oldtime = Sys_FloatTime ();
	while (1)
	{
		newtime = Sys_FloatTime ();
		time = newtime - oldtime;

		if ((be_sleepy || cls.state == ca_dedicated) && (time<sys_ticrate.value))
		{
			ULONG ms;
			ms = (ULONG)((sys_ticrate.value - time) * 1000);
			if (ms > 30)
				ms = 30;
			DosSleep(ms);
			continue;
		}

		Host_Frame (time);

		oldtime = newtime;
	}
}


