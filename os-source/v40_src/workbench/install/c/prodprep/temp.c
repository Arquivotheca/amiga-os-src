/* readrdsk.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/filehandler.h>

#include <dos/dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include "global.h"
#include "/refresh.h"
#include "protos.h"

#include "/scsi.h"
#include "/scsidisk.h"

#include "dummy.h"

#define SAME 0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

void
main (argc,argv)
	int argc;
	char **argv;
{
	int ret = 20;
	char *mem;
	BPTR fh = NULL,fh2;

	mem = AllocMem(17*4*512,MEMF_CLEAR);
	if (!mem)
		exit(20);
	fh = Open("rdsk",MODE_OLDFILE);
	if (fh)
	{
		Read(fh,mem, 17*4*512);
		fh2 = Open("rdsk2",MODE_NEWFILE);
		if (fh2)
		{
			Write(fh2,mem, 17*4*512);
			Close(fh2);
		}
		Close(fh);
	}
	ret = 0;	/* success */


	FreeMem(mem,17*4*512);

	exit(ret);
}
