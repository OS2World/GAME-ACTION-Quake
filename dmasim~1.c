// =======================================================================
// Sound client does nothing but walk a circular sound buffer
// =======================================================================

//#include <fcntl.h>
#include <sys/types.h>
//#include <sys/ioctl.h>
#include <stdio.h>
#include <audio.h>

#include "quakedef.h"

extern int	samplewidth;
extern int	numfragments;
extern int	fragmentsize;

extern dma_t	*shm;

void I_Error(char *, ...);
void I_Warn(char *, ...);
int log2(int);

// =======================================================================
// System-specific data
// =======================================================================

static	ALconfig	al_config;
static	ALport		al_outport;

// =======================================================================
// Initializes DMA-like sound device
// =======================================================================

void I_InitDMASound(void)
{

	long paramset[2];

	// open & setup audio device

	ALseterrorhandler(0);	// turn off audio library error handler
	al_config = ALnewconfig();

	ALsetchannels(al_config, AL_STEREO);
	ALsetsampfmt(al_config, AL_SAMPFMT_TWOSCOMP);
	ALsetwidth(al_config, samplewidth);
	ALsetqueuesize(al_config, fragmentsize * 2);

	al_outport = ALopenport("iddigout2", "w", al_config);

	if (!al_outport)
		I_Error("Could not open audio port");

	paramset[0] = AL_OUTPUT_RATE;
	paramset[1] = shm->speed;
	ALsetparams(AL_DEFAULT_DEVICE, paramset, 2);

}

// =======================================================================
// Submits data to DMA-like sound device
// =======================================================================

void I_SubmitDMABuffer(void *buffer, int size)
{
	ALwritesamps(al_outport, buffer, size);
}

// =======================================================================
// Shuts down DMA-like sound device
// =======================================================================

void I_ShutdownDMASound(void)
{
	ALfreeconfig(al_config);
	ALcloseport(al_outport);
}

