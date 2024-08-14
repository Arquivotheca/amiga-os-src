/* examinedt.c
 * Copyright © 1992 Commodore Services International, Co.
 * Written by David N. Junod
 *
 * This simple little example shows how to examine a file.
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <libraries/iffparse.h>
#include <stdio.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

extern struct Library *SysBase, *DOSBase;
struct Library *DataTypesBase, *IFFParseBase;

int main (int argc, char **argv)
{
    struct DataTypeHeader *dth;
    struct DataType *dtn;
    UBYTE buffer[6];
    BPTR lock;
    WORD i;

    if (DataTypesBase = OpenLibrary ("datatypes.library", 39))
    {
	if (IFFParseBase = OpenLibrary ("iffparse.library", 39))
	{
	    for (i = 1; i < argc; i++)
	    {
		if (lock = Lock (argv[i], ACCESS_READ))
		{
		    if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
		    {
			dth = dtn->dtn_Header;

			printf ("information on: %s\n\n", argv[i]);

			printf ("   Description: %s\n", dth->dth_Name);
			printf ("     Base Name: %s\n", dth->dth_BaseName);
			printf ("          Type: %s\n", GetDTString ((dth->dth_Flags & DTF_TYPE_MASK) + DTMSG_TYPE_OFFSET));
			printf ("         Group: %s\n", GetDTString (dth->dth_GroupID));
			printf ("            ID: %s\n", IDtoStr (dth->dth_ID, buffer));

			ReleaseDataType (dtn);
		    }
		    else
		    {
			printf ("error\n");
		    }
		    UnLock (lock);
		}
	    }
	    CloseLibrary (IFFParseBase);
	}
	else
	{
	    printf ("couldn't open iffparse.library V39\n");
	}
	CloseLibrary (DataTypesBase);
    }
    else
    {
	printf ("couldn't open datatypes.library V39\n");
    }
}
