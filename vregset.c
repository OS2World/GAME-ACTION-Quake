//
// vregset.c: video register-setting interpreter
//

#include "quakedef.h"
#include "vregset.h"

#define _outp8	outport8
#define _inp8   inport8

void outport8(unsigned short, byte);
byte inport8(unsigned short);

/*
================
VideoRegisterSet
================
*/
void VideoRegisterSet (int *pregset)
{
	int		port, temp0, temp1, temp2;

	for ( ;; )
	{
		switch (*pregset++)
		{
			case VRS_END:
				return;

			case VRS_BYTE_OUT:
				port = *pregset++;
				_outp8 (port, *pregset++);
				break;

			case VRS_BYTE_RMW:
				port = *pregset++;
				temp0 = *pregset++;
				temp1 = *pregset++;
				temp2 = _inp8 (port);
				temp2 &= temp0;
				temp2 |= temp1;
				_outp8 (port, temp2);
				break;

			case VRS_WORD_OUT:
				port = *pregset++;
				_outp8 (port, *pregset & 0xFF);
				_outp8 (port+1, *pregset >> 8);
				pregset++;
				break;

			default:
				Sys_Error ("VideoRegisterSet: Invalid command\n");
		}
	}
}

