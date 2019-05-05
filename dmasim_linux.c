//
//
// deleted current dmasim_linux.  need to fix this!  Jun 20
//
//

// =======================================================================
// Sound client does nothing but walk a circular sound buffer
// =======================================================================

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <stdio.h>

#include "quakedef.h"

extern int	samplewidth;
extern int	numfragments;
extern int	fragmentsize;

extern volatile dma_t	*shm;

char *debugdma;
FILE *debug_file=0;

void I_Error (char *error, ...);
void I_Warn (char *warning, ...);
int Q_log2(int val);

// =======================================================================
// System-specific data
// =======================================================================

int	audio_fd=-1;

// =======================================================================
// Gets available sound parameters
// =======================================================================

static int tryrates[] = { 44100, 22050, 11025, 8000 };

void I_GetSoundParams(void)
{

	int fmt;
	int i;
	char *s;

	shm->splitbuffer = 0;
	s = getenv("QUAKE_SOUND_SAMPLEBITS");
	if (s) shm->samplebits = atoi(s);
	else
	{
		ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &fmt);
		if (fmt & AFMT_S16_LE) shm->samplebits = 16;
		else if (fmt & AFMT_U8) shm->samplebits = 8;
	}

	s = getenv("QUAKE_SOUND_SPEED");
	if (s) shm->speed = atoi(s);
	else
	{
		for (i=0 ; i<sizeof(tryrates)/4 ; i++)
			if (!ioctl(audio_fd, SNDCTL_DSP_SPEED, &tryrates[i])) break;
		shm->speed = tryrates[i];
	}

	s = getenv("QUAKE_SOUND_CHANNELS");
	if (s) shm->channels = atoi(s);
	else shm->channels = 2;

	shm->samples = (1<<16) / (shm->samplebits / 8);

}

// =======================================================================
// Initializes DMA-like sound device
// =======================================================================

void I_InitDMASound(void)
{

	int rc;
	int formats;
	struct
	{
		short log2buffersize;
		short numbuffers;
	} bufferinfo;

	audio_fd = open("/dev/dsp", O_WRONLY);
	if (audio_fd < 0)
		I_Error("Could not open /dev/dsp");

	I_GetSoundParams();

	fragmentsize = (1<<Q_log2(shm->speed/80)) * shm->channels
		* shm->samplebits/8;
//	fragmentsize = 1024;
	shm->submission_chunk = fragmentsize / (shm->channels*shm->samplebits/8);

//	bufferinfo.numbuffers = numfragments;
	bufferinfo.numbuffers = 2;
	bufferinfo.log2buffersize = Q_log2(fragmentsize);

	fprintf(stderr, "bufferinfo = %d : %d (0x%x)\n", bufferinfo.numbuffers,
		bufferinfo.log2buffersize, *(int*)&bufferinfo);

	rc = ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &bufferinfo);
	if (rc < 0) I_Warn("SETFRAGMENT failed.");

	rc = ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
	if (rc < 0) I_Error("Could not reset /dev/dsp");

	rc = ioctl(audio_fd, SNDCTL_DSP_SPEED, &shm->speed);
	if (rc < 0) I_Error("Could not set /dev/dsp speed to %d", shm->speed);

	rc = ioctl(audio_fd, SNDCTL_DSP_STEREO, &shm->channels);
	if (rc < 0) I_Error("Could not set /dev/dsp to stereo");

	if (shm->samplebits == 16)
	{
		rc = AFMT_S16_LE;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
			I_Error("Could not support 16-bit data.  Try 8-bit.");
	}
	else if (shm->samplebits == 8)
	{
		rc = AFMT_U8;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
			I_Error("Could not support 8-bit data.");
	}
	else
		I_Error("%d-bit sound not supported.", shm->samplebits);

	debugdma = getenv("QUAKE_DEBUG_DMA");
	if (debugdma)
	{
		if (*debugdma == '|')
			debug_file = popen(debugdma+1, "w");
		else
			debug_file = fopen(debugdma, "w");
	}

}

// =======================================================================
// Submits data to DMA-like sound device
// =======================================================================

void I_SubmitDMABuffer(void *buffer, int size)
{
	if (size != fragmentsize)
		I_Error("size != fragmentsize (%d != %d)", size, fragmentsize);
	write(audio_fd, buffer, size*shm->channels);
	if (debugdma)
	{
		fwrite(buffer, size*shm->channels, 1, debug_file);
		fflush(debug_file);
	}
}


// =======================================================================
// Shuts down DMA-like sound device
// =======================================================================

void I_ShutdownDMASound(void)
{
	close(audio_fd);
	if (debugdma)
	{
		if (*debugdma == '|')
			pclose(debug_file);
		else
			close(debug_file);
	}
}

