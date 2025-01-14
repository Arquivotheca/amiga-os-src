/* longio.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>

#include <dos/dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/dos.h>

#include "global.h"
#include "/refresh.h"
#include "protos.h"

#include "/scsidisk.h"
#include "/scsi.h"

#define SAME	0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

/* sendio return true if ok, false if should abort */
/* if ok, assume ior has been SendIO()ed, and wait */
/* errrtn is called to handle errors, and is	   */
/* passed the ior.  errrtn may be NULL.		   */
/* returns ior->io_Error or -5			   */

int
DoLongIO (waittext,sendio,itext,draw,errrtn)
	char *waittext;
	int (*sendio)(struct IOStdReq *);
	struct IntuiText *itext;
	void (*draw)(struct RastPort *);
	int (*errrtn)(struct IOStdReq *);
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port = NULL;
	struct IntuiMessage *msg;
	int  opened = FALSE;
	int  i;
	int  error = -5;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	/* open the unit */
	if (i = OpenDevice("xt.device",0,
			   (struct IORequest *) ior,0L))
	{
		printf("Error %d on device open!\n",i);
		goto cleanup;
	}
	opened = TRUE;

	if ((*sendio)(ior) == FALSE)
		goto cleanup;

	/* asynch allows us to handle refresh, put up reguesters */
	for (;;)
	{
/*printf("Top of request loop\n");*/
		/* wait for reply, handle messages */
		Wait ((1L << port->mp_SigBit));

		if (CheckIO((struct IORequest *) ior))
			break;
	}
	WaitIO((struct IORequest *) ior);
/*printf("IO done\n");*/


	error = ior->io_Error;

/* FIX! HFERR_BadStatus! */
	if (error && error != 45)
		printf("Driver returned I/O error code %ld\n",(LONG) error);

	/* call the error handling routine - important for SCSI */
	if (errrtn)
		error = (*errrtn)(ior);

	/* make sure we return error if io_error */
	if (!error && ior->io_Error)
		error = ior->io_Error;

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	return error;
}

