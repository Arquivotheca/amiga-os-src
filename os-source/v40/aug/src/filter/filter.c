/* filter.c
 * Written by David N. Junod
 * This example shows how to use the ASL file requester to only
 * show files that are of a certain data type---in this example
 * it only shows pictures.
 *
 */

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <string.h>
#include <stdio.h>

#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/datatypes_pragmas.h>

/*****************************************************************************/

#define ASM           __asm __saveds
#define REG(x)	      register __ ## x

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;
struct Library *DataTypesBase, *AslBase;

/*****************************************************************************/

ULONG ASM Filter (REG (a0) struct Hook *h, REG (a2) struct FileRequester *fr, REG (a1) struct AnchorPath *ap)
{
    struct DataType *dtn;
    ULONG use = FALSE;
    UBYTE buffer[300];
    BPTR lock;

    strncpy (buffer, fr->fr_Drawer, sizeof (buffer));
    AddPart (buffer, ap->ap_Info.fib_FileName, sizeof (buffer));
    if (lock = Lock (buffer, ACCESS_READ))
    {
	if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
	{
	    if (dtn->dtn_Header->dth_GroupID == GID_PICTURE)
		use = TRUE;

	    ReleaseDataType (dtn);
	}
	UnLock (lock);
    }
    return (use);
}

/*****************************************************************************/

int main (int argc, char **argv)
{
    struct FileRequester *fr;
    struct Hook filter;

    if (DataTypesBase = OpenLibrary ("datatypes.library", 39))
    {
	if (AslBase = OpenLibrary ("asl.library", 38))
	{
	    filter.h_Entry = Filter;
	    if (fr = AllocAslRequestTags (ASL_FileRequest,
					  ASLFR_TitleText,	"Select Picture to Open",
					  ASLFR_PositiveText,	"Open",
					  ASLFR_RejectIcons,	TRUE,
					  ASLFR_FilterFunc,	&filter,
					  TAG_DONE))
	    {
		AslRequestTags (fr, TAG_DONE);
		FreeAslRequest (fr);
	    }

	    CloseLibrary (AslBase);
	}

	CloseLibrary (DataTypesBase);
    }
}