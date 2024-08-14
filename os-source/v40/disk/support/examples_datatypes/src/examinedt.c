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
 * examinedt.c
 * Shows how to examine a file using datatypes.library
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <libraries/iffparse.h>
#include <string.h>
#include <stdio.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;

/*****************************************************************************/

struct Library *DataTypesBase, *IFFParseBase;

/*****************************************************************************/

#define	TEMPLATE	"NAMES/M/A"
#define	OPT_NAME	0
#define	OPT_MAX		1

/*****************************************************************************/

int main (void)
{
    struct DataTypeHeader *dth;
    struct DataType *dtn;
    UBYTE buffer[6];
    BPTR lock;

    ULONG options[OPT_MAX];
    struct RDArgs *rdargs;
    STRPTR *names;

    if (SysBase->lib_Version < 39)
    {
	printf ("requires 3.x or beyond to run\n");
    }
    else
    {
	/* Clear the option array */
	memset (options, 0, sizeof (options));

	/* Parse the arguments */
	if (rdargs = ReadArgs (TEMPLATE, options, NULL))
	{
	    /* Open the libraries */
	    if (DataTypesBase = OpenLibrary ("datatypes.library", 39))
	    {
		if (IFFParseBase = OpenLibrary ("iffparse.library", 39))
		{
		    /* Get a pointer to the name array */
		    names = (STRPTR *) options[OPT_NAME];

		    /* Step through the name array */
		    while (*names)
		    {
			/* Lock the current name */
			if (lock = Lock (*names, ACCESS_READ))
			{
			    /* Determine the DataType of the file */
			    if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
			    {
				dth = dtn->dtn_Header;

				printf ("information on: %s\n", *names);
				printf ("   Description: %s\n", dth->dth_Name);
				printf ("     Base Name: %s\n", dth->dth_BaseName);
				printf ("          Type: %s\n", GetDTString ((dth->dth_Flags & DTF_TYPE_MASK) + DTMSG_TYPE_OFFSET));
				printf ("         Group: %s\n", GetDTString (dth->dth_GroupID));
				printf ("            ID: %s\n\n", IDtoStr (dth->dth_ID, buffer));

				/* Release the DataType */
				ReleaseDataType (dtn);
			    }
			    else
				PrintFault (IoErr (), *names);
			    UnLock (lock);
			}
			else
			    PrintFault (IoErr (), *names);

			/* Get the next name */
			names++;
		    }

		    CloseLibrary (IFFParseBase);
		}
		else
		    printf ("couldn't open iffparse.library V39\n");

		CloseLibrary (DataTypesBase);
	    }
	    else
		printf ("couldn't open datatypes.library V39\n");

	    /* Free the allocated memory after ReadArgs */
	    FreeArgs (rdargs);
	}
	else
	    PrintFault (IoErr (), NULL);
    }
}
