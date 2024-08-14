/* format.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>

#include <dos/dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include <clib/alib_protos.h>

#include "global.h"
#include "refresh.h"
#include "protos.h"

#include "scsidisk.h"
#include "scsi.h"

#define SAME 0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

LONG interleave;

extern struct SysBase *SysBase;
extern struct DOSBase *DOSBase;

extern char *device;
extern int  unit;

int
DoLowLevel ()
{
	int  error;

	interleave = 1;

	error = DoLongIO("Formatting, please wait",LowLevelSendIO,
			 NULL);

	if (error)
	{
		printf("Failed Low-level Format, error %d !\n",
			(LONG) error);
	}
	/* else successful format of drive */
	return error;
}

/* routine called by DoLongIO to send the IO request */
/* return TRUE if ok, FALSE if error		     */

int
LowLevelSendIO (ior)
	struct IOStdReq *ior;
{
	/* now do the format unit */
	ior->io_Command = TD_FORMAT;
	ior->io_Offset  = 0;
	ior->io_Data	= NULL; 	/* dangerous! */
	ior->io_Length	= 512 * 17 * 4;

	SendIO((struct IORequest *) ior);

	return TRUE;
}

/* open drives that were written out to - use flags = 1 - this causes the */
/* driver to read the rigid disk block. */

void
ReopenDrives ()
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (!OpenDevice(device,unit,(struct IORequest *) ior,1L))
		CloseDevice((struct IORequest *) ior);

cleanup:
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);
}
