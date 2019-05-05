#include "winquake.h"

dma_t theshm;


  
/* 
 * Global variables. Must be visible to window-procedure function 
 *  so it can unlock and free the data block after it has been played. 
 */ 

HANDLE		hData;
HPSTR		lpData;

HGLOBAL		hWaveHdr;
LPWAVEHDR	lpWaveHdr;

HWAVEOUT    hWaveOut; 

WAVEOUTCAPS	wavecaps;

MMTIME		mmstarttime;



void FreeSound (void)
{
	int		i;

	if (!hWaveOut)
		return;


	waveOutUnprepareHeader (hWaveOut, lpWaveHdr,sizeof(WAVEHDR));
	GlobalUnlock(hWaveHdr); 
	GlobalFree(lpWaveHdr);

	GlobalUnlock(hData); 
	GlobalFree(hData);

	waveOutReset (hWaveOut);
	waveOutClose (hWaveOut);
}

/*
==================
SNDDM_Init

Try to find a sound device to mix for.
Returns false if nothing is found.
==================
*/
int SNDDMA_Init(void)
{
    HMMIO       hmmio; 
    UINT        wResult; 
    HANDLE      hFormat; 
    WAVEFORMATEX  format; 
    DWORD       dwDataSize; 
	int			i;


	shm = &theshm;

	shm->channels = 2;
	shm->samplebits = 8;
	shm->speed = 11025;

	format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = shm->channels;
    format.wBitsPerSample = shm->samplebits;
    format.nSamplesPerSec = shm->speed;
    format.nBlockAlign = format.nChannels
		*format.wBitsPerSample / 16;
    format.cbSize = 0;
    format.nAvgBytesPerSec = format.nSamplesPerSec
		*format.nBlockAlign; 
		
	/* Open a waveform device for output using window callback. */ 
    if (waveOutOpen((LPHWAVEOUT)&hWaveOut, WAVE_MAPPER, 
                    &format, 
                    0, 0L, CALLBACK_NULL))
	{
		Con_Printf ("waveOutOpen failed\n");
        return 0; 
    } 

	/* 
	 * Allocate and lock memory for the waveform data. The memory 
	 * for waveform data must be globally allocated with 
	 * GMEM_MOVEABLE and GMEM_SHARE flags. 
	 */ 
	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, 4096 ); 
	if (!hData) 
	{ 
		Con_Printf ("Sound: Out of memory.\n");
		return 0; 
	}
	lpData = GlobalLock(hData);
	if (!lpData)
	{ 
		Con_Printf ("Sound: Failed to lock.\n");
		FreeSound ();
		return 0; 
	} 
	memset (lpData, 0, 4096);

	/* 
	 * Allocate and lock memory for the header. This memory must 
	 * also be globally allocated with GMEM_MOVEABLE and 
	 * GMEM_SHARE flags. 
	 */ 
	hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, 
		(DWORD) sizeof(WAVEHDR)); 
	if (hWaveHdr == NULL)
	{ 
		Con_Printf ("Sound: Failed to Alloc header.\n");
		FreeSound ();
		return 0; 
	} 

	lpWaveHdr = (LPWAVEHDR) GlobalLock(hWaveHdr); 
	if (lpWaveHdr == NULL)
	{ 
		Con_Printf ("Sound: Failed to lock header.\n");
		FreeSound ();
		return 0; 
	} 

	/* After allocation, set up and prepare header. */ 
	lpWaveHdr->lpData = lpData;
	lpWaveHdr->dwBufferLength = 4096; 
	lpWaveHdr->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP; 
	lpWaveHdr->dwLoops = 0x7fffffffL; 
	waveOutPrepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR)); 

	//
	// grab the current sample out time now
	//
	mmstarttime.wType = TIME_SAMPLES;
	waveOutGetPosition (hWaveOut, &mmstarttime, sizeof(mmstarttime)); 


	/* 
	 * Now the data block can be sent to the output device. The 
	 * waveOutWrite function returns immediately and waveform 
	 * data is sent to the output device in the background. 
	 */ 
	wResult = waveOutWrite(hWaveOut, lpWaveHdr, sizeof(WAVEHDR)); 
	if (wResult != 0)
	{ 
		Con_Printf ("Failed to write block to device\n");
		FreeSound ();
		return 0; 
	} 
 

	shm->soundalive = true;
	shm->splitbuffer = false;
	shm->samples = 4096/(shm->samplebits/8);
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = (unsigned char *) lpData;

	return 1;
}

/*
==============
SNDDMA_GetDMAPos

return the current sample position (in mono samples read)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int SNDDMA_GetDMAPos(void)
{
	MMTIME	mmtime;
	int		s;

	mmtime.wType = TIME_SAMPLES;
	waveOutGetPosition (hWaveOut, &mmtime, sizeof(mmtime)); 

	s = mmtime.u.sample - mmstarttime.u.sample;

	if (shm->channels == 2)
		s <<= 1;

	s &= (shm->samples-1);

//	Con_Printf ("%i s %i c\n", s, clock());
	return s;
}

/*
==============
SNDDMA_Shutdown

Reset the sound device for exiting
===============
*/
void SNDDMA_Shutdown(void)
{
	FreeSound ();
}

