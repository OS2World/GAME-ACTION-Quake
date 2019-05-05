// =======================================================================
// Sound client does nothing but walk a circular sound buffer
// =======================================================================

//#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdarg.h>
//#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#include "quakedef.h"

// =======================================================================
// Sound data & structures
// =======================================================================

int samplewidth;
int	numfragments;
int	fragmentsize;
int dma_buffer_size;
char *dma_buffer;

volatile dma_t *shm = 0;

void I_Error (char *error, ...)
{ 
    va_list     argptr;
    char        string[1024];
   
	if ((int)shm>0)
	{
		shm->soundalive = 0;
		shmdt((void*)shm);
	}

    va_start (argptr,error);
    vsprintf (string,error,argptr);
    va_end (argptr);
	fprintf(stderr, "dmasim error: %s\n", string);
	exit(-1);
} 

void I_Warn (char *warning, ...)
{ 
    va_list     argptr;
    char        string[1024];
    
    va_start (argptr,warning);
    vsprintf (string,warning,argptr);
    va_end (argptr);
	fprintf(stderr, "dmasim: %s\n", string);
} 

// =======================================================================
// log base-2
// =======================================================================

int Q_log2(int val)
{
	int answer;
	for (answer = 0 ; val>>=1 ; answer++);
	return answer;
}

void I_InitDMASound(void);
void I_ShutdownDMASound(void);
void I_SubmitDMABuffer(void *buffer, int size);

// =======================================================================
// Portable main control
// =======================================================================

void main(int c, char **v)
{

	int shmid;
	int	samplepos;
	int pagesize = getpagesize();
	int shmsize;
	int parent_pid;
	struct shmid_ds shmbuf;

	if (c < 2 || !atoi(v[1]))
		I_Error("Usage: dmasim <Size of DMA buffer in bytes>");

	parent_pid = getppid();

	// attach to the shared memory segment

	shmsize = atoi(v[1]);
	dma_buffer_size = 1<<Q_log2(shmsize);
	fprintf(stderr, "snd started\n");
	fprintf(stderr, "snd DMA buffer size = %d\n", dma_buffer_size);

	shmid = shmget(ftok(".", 0), shmsize, 0666);
	shm = (void *) shmat(shmid, 0, 0);
	if ((int)shm<=0)
		I_Error("Usage: Could not attach to shared memory");
	dma_buffer = (char *) (shm+1);

// initialize sound

	I_InitDMASound();

	samplewidth = shm->samplebits / 8;

	I_Warn("fragmentsize = %d\n", fragmentsize);

// play sound until told to stop

	shm->soundalive = 1;
	samplepos = 0;
	while (1)
	{
		I_SubmitDMABuffer(&dma_buffer[samplepos*samplewidth], fragmentsize);
		samplepos =	(samplepos + fragmentsize)
					& (shm->samples - 1);
		shm->samplepos = samplepos;
		shmctl(shmid, IPC_STAT, &shmbuf);
		if (!shm->gamealive || shmbuf.shm_nattch == 1)
		{
			fprintf(stderr, "Quake appears to have left me.\n");
			break;  // FIXME: not a cool solution
		}
	}

	// get rid of shared memory and close audio device

	shm->soundalive = 0;
	shmdt((void *) shm);

	I_ShutdownDMASound();

	exit(0);

}

