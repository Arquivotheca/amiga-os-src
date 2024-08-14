/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * ramsound.c
 * This shows how to play a sound of type DTST_RAM.
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>
#include <stdio.h>

#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/datatypes_pragmas.h>

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;

/*****************************************************************************/

struct Library *DataTypesBase;

/*****************************************************************************/

void main (void)
{
    ULONG i, samplelength = 2;
    struct dtTrigger dtt;
    LONG clock = 3579545;
    LONG frequency = 440;
    LONG samplecycle = 1;
    LONG duration = 30;
    BYTE *sample;
    Object *o;

    if (DataTypesBase = OpenLibrary ("datatypes.library", 39))
    {
	samplelength = 2;
	if (sample = AllocVec (samplelength, MEMF_CHIP))
	{
	    sample[0] = 127;
	    sample[1] = -127;

	    if (o = NewDTObject ((APTR) "sound.test",
				 DTA_SourceType,	DTST_RAM,
				 DTA_GroupID,		GID_SOUND,
				 SDTA_Sample,		sample,
				 SDTA_SampleLength,	samplelength,
				 SDTA_Volume,		0,
				 SDTA_Period,		(clock*samplecycle/(samplelength*frequency)),
				 SDTA_Cycles,		(frequency*duration/samplecycle),
				 TAG_DONE))
	    {
		/* Play the sound */
		dtt.MethodID     = DTM_TRIGGER;
		dtt.dtt_GInfo    = NULL;
		dtt.dtt_Function = STM_PLAY;
		dtt.dtt_Data     = NULL;
		DoMethodA (o, &dtt);

		for (i = 0; i < 64; i++)
		{
		    SetDTAttrs (o, NULL, NULL, SDTA_Volume, i, TAG_DONE);
		    Delay (2);
		}

		for (i = 64; i > 0; i--)
		{
		    SetDTAttrs (o, NULL, NULL, SDTA_Volume, i, TAG_DONE);
		    Delay (2);
		}

		/* Get rid of the object */
		DisposeDTObject (o);
	    }
	    else
	    {
		printf ("couldn't create sound object\n");
		FreeVec (sample);
	    }
	}
	CloseLibrary (DataTypesBase);
    }
    else
	printf ("couldn't open datatypes.library V39\n");
}
