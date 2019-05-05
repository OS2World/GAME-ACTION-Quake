#define INCL_DOS
#include <os2.h>

#define INCL_MCIOS2
#define INCL_MACHDR

#include <os2me.h>

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>

#include "quakedef.h"

dma_t theshm;
int snd_inited = 0;


#define MAX_BUFFERS 16
MCI_MIX_BUFFER       MixBuffers[MAX_BUFFERS+32];   /* Device buffers          */
USHORT               usDeviceID;                /* Amp Mixer device id     */
MCI_MIXSETUP_PARMS   MixSetupParms;             /* Mixer parameters        */
MCI_BUFFER_PARMS     BufferParms;               /* Device buffer parms     */
MCI_STATUS_PARMS get;
ULONG oops = 0;

extern int da_buffers;

int last_written_buffer = 0;
int total_written_buffers = 0;

char *dart_buffer0 = 0, *dart_buffer1 = 0;

char *dart_buffers[17];

volatile extern int last_painted_buffer;

LONG APIENTRY MyEvent (ULONG ulStatus,
                        PMCI_MIX_BUFFER  pBuffer,
                        ULONG  ulFlags  )

{
#if 1
	 if (last_painted_buffer != last_written_buffer)
	 {
		  MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                	               &MixBuffers[last_written_buffer],
                        	       1);
		last_written_buffer = (last_written_buffer + 1) & 15;
	}
	else
	{
		  MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                	               &MixBuffers[16],
                        	       1);
	}
	total_written_buffers++;
//	da_buffers++;
#endif
   return( TRUE );
} /* end MyEvent */


void dart_clear_buffer(char c)
{
	memset(MixBuffers[last_painted_buffer].pBuffer, c, 1024);
}

qboolean SNDDMA_Init(void)
{
	MCI_AMP_OPEN_PARMS   AmpOpenParms;
	ULONG rc;
	int i;

	shm = &theshm;

	shm->channels = 2;
	shm->samplebits = 8;
	shm->speed = 11025;

	memset ( &AmpOpenParms, '\0', sizeof ( MCI_AMP_OPEN_PARMS ) );
	AmpOpenParms.usDeviceID = ( USHORT ) 0;
	AmpOpenParms.pszDeviceType = ( PSZ ) MCI_DEVTYPE_AUDIO_AMPMIX;

	if (COM_CheckParm("-sharesound"))
		rc = mciSendCommand( 0,
        	               MCI_OPEN,
                	       MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE,
	                       ( PVOID ) &AmpOpenParms,
        	               0 );
	else
		rc = mciSendCommand( 0,
        	               MCI_OPEN,
                	       MCI_WAIT | MCI_OPEN_TYPE_ID,
	                       ( PVOID ) &AmpOpenParms,
        	               0 );

	if (rc)
		return 0;

	usDeviceID = AmpOpenParms.usDeviceID;

	memset( &MixSetupParms, '\0', sizeof( MCI_MIXSETUP_PARMS ) );
	MixSetupParms.ulFormatTag = MCI_WAVE_FORMAT_PCM;
	MixSetupParms.ulFormatMode = MCI_PLAY;
	MixSetupParms.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
	MixSetupParms.pmixEvent    = MyEvent;

	MixSetupParms.ulBitsPerSample = shm->samplebits;   
	MixSetupParms.ulSamplesPerSec = shm->speed;
	MixSetupParms.ulChannels = shm->channels;

	rc = mciSendCommand( usDeviceID,
                        MCI_MIXSETUP,
                        MCI_WAIT | MCI_MIXSETUP_INIT,
                        ( PVOID ) &MixSetupParms,
                        0 );

	if (rc)
	{
		fprintf(stderr, "MCI_MIXSETUP _not_ successful\n");
		fflush(stderr);
		return 0;
	}

	BufferParms.ulNumBuffers = 17;
//	BufferParms.ulBufferSize = MixSetupParms.ulBufferSize;
	BufferParms.ulBufferSize = 1024;
	BufferParms.pBufList = MixBuffers;

	rc = mciSendCommand( usDeviceID,
                        MCI_BUFFER,
                        MCI_WAIT | MCI_ALLOCATE_MEMORY,
                        ( PVOID ) &BufferParms,
                        0 );

	if (rc)
	{
		fprintf(stderr, "MCI_BUFFER _not_ successful\n");
		fflush(stderr);
		return 0;
	}

//	MixBuffers[0].ulFlags = 0;

	shm->soundalive = true;
	shm->splitbuffer = false;
//	shm->samples = MixSetupParms.ulBufferSize / (shm->samplebits/8);
	shm->samples = 1024 * 16;
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = malloc(1024 * 32);
	snd_inited = 1;
	get.ulItem = MCI_STATUS_POSITION;
	mciSendCommand( usDeviceID, MCI_STATUS,
		MCI_WAIT | MCI_STATUS_ITEM, &get, 0);
	oops = get.ulReturn;
	dart_buffer0 = MixBuffers[0].pBuffer;
	dart_buffer1 = MixBuffers[1].pBuffer;
	for (i=0; i < 17; i++)
	{
		dart_buffers[i] = MixBuffers[i].pBuffer;
		bzero(MixBuffers[i].pBuffer, 1024);
	}
	memset(MixBuffers[16].pBuffer, 0x80, 1024);
	MixBuffers[16].ulBufferLength = 512;
#if 1
        MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                               &MixBuffers[0],
                               1 );
        MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                               &MixBuffers[1],
                               1 );
        MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                               &MixBuffers[2],
                               1 );
        MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                               &MixBuffers[3],
                               1 );
        MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                               &MixBuffers[4],
                               1 );
	total_written_buffers = 0;
	last_written_buffer = 0;
#endif
	return 1;
}

void dart_write_buf0(void)
{
         MixSetupParms.pmixWrite( MixSetupParms.ulMixHandle,
                               &MixBuffers[0],
                               1);
	total_written_buffers++;
}

int SNDDMA_GetDMAPos(void)
{
	int new;
	static int cnt = 0;
	if (!snd_inited)
		return 0;
	return total_written_buffers * 1024;
}

void SNDDMA_Shutdown(void)
{
}


