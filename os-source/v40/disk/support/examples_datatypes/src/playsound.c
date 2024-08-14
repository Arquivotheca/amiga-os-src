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
 * playsound.c
 * Simple example showing how to play a sound file
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>
#include <workbench/workbench.h>
#include <string.h>
#include <stdio.h>

#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/datatypes_pragmas.h>

/*****************************************************************************/

extern struct ExecBase *SysBase;
extern struct Library *DOSBase;
struct Library *DataTypesBase;

/*****************************************************************************/

#define	TEMPLATE	"NAME/A"
#define	OPT_NAME	0
#define	NUM_OPTS	1

/*****************************************************************************/

void main (int argc, char **argv)
{
    /* Argument parsing variables */
    ULONG options[NUM_OPTS];
    struct RDArgs *rdargs;

    /* Object variables */
    struct dtTrigger dtt;
    STRPTR name;
    Object *o;

    memset (options, 0, sizeof (options));
    if (rdargs = ReadArgs (TEMPLATE, options, NULL))
    {
	/* Open DataTypes */
	if (DataTypesBase = OpenLibrary ("datatypes.library", 39))
	{
	    /* Open the sound object */
	    if (o = NewDTObject ((APTR) options[OPT_NAME],

				/* Say that the source is a file */
				 DTA_SourceType,	DTST_FILE,

				/* We will only accept sound DataTypes */
				 DTA_GroupID,		GID_SOUND,

				/* Set to maximum volume */
				 SDTA_Volume,		64,

				/* Only play the sound once */
				 SDTA_Cycles,		1,

				/* We want to be notified when the sound stops playing, so
				 * we provide a signal task and a signal */
				 SDTA_SignalTask,	(ULONG) FindTask (NULL),
				 SDTA_SignalBit,	(ULONG) SIGBREAKB_CTRL_F,

				/* No more attributes */
				 TAG_DONE))
	    {
		/* Get information about the object */
		if (GetDTAttrs (o, DTA_ObjName, (ULONG) &name, TAG_DONE))
		{
		    /* Display any information we obtained */
		    if (name)
			printf ("     Name: %s\n", name);
		}

		/* Fill out the method message */
		dtt.MethodID     = DTM_TRIGGER;
		dtt.dtt_GInfo    = NULL;
		dtt.dtt_Function = STM_PLAY;
		dtt.dtt_Data     = NULL;

		/* Play the sound */
		printf ("\nplay sound...");
		DoMethodA (o, &dtt);

		Wait (SIGBREAKB_CTRL_F);
		printf ("done\n");

		/* Get rid of the object */
		DisposeDTObject (o);
	    }
	    else
	    {
		LONG errnum = IoErr ();
		UBYTE errbuff[80];

		if (errnum >= DTERROR_UNKNOWN_DATATYPE)
		    sprintf (errbuff, GetDTString (errnum), options[OPT_NAME]);
		else
		    Fault (errnum, NULL, errbuff, sizeof (errbuff));
		printf ("%s\nerror #%ld\n", errbuff, errnum);
	    }

	    CloseLibrary (DataTypesBase);
	}
	else
	    printf ("couldn't open datatypes.library V39\n");

	/* Free the allocated memory after ReadArgs */
	FreeArgs (rdargs);
    }
    else
    {
	/* Show the failure message */
	PrintFault (IoErr (), NULL);
    }
}
