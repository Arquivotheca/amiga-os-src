/* makefilesys.c */

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


#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

struct FileSysHeaderBlock *
MakeFileSys (w,rdb,name,dostype,version)
	struct Window *w;
	struct RigidDiskBlock *rdb;
	char *name;
	LONG dostype,version;
{
	struct FileSysHeaderBlock *fs = NULL;
	register struct LoadSegBlock *ls,*lastls;
	BPTR fh;
	register LONG len,newlen;
	ULONG temp = 0, done;

	if (fh = Open(name,MODE_OLDFILE))
	{
		Read(fh,(char *) &temp,4);
		if (temp == 0x3F3)			/* hunk_header */
		{
			Seek(fh,0,OFFSET_BEGINNING);
			if (fs = AllocNew(FileSysHeaderBlock))
			{
			    *fs = defaultfs;	/* structure copy */
			    fs->fhb_DosType = dostype;
			    fs->fhb_Version = version;

			    lastls = NULL;
			    done = FALSE;
			    while (!done) {
				ls = (struct LoadSegBlock *)
				    AllocMem(rdb->rdb_BlockBytes,MEMF_CLEAR);

				ls->lsb_ID = IDNAME_LOADSEG;
				ls->lsb_SummedLongs = 
					rdb->rdb_BlockBytes >> 2;
				ls->lsb_HostID = 7;

				if (lastls)
					lastls->lsb_Next = ls;
				else
					fs->fhb_SegListBlocks = ls;

				newlen = (ls->lsb_SummedLongs - 5) * 4;
				len = Read(fh,(char *)&ls->lsb_LoadData[0],
					   newlen);
				if (len < 0)
				{
					if (w)
					  Notify(w,
					   "Error %d while reading filesystem!",
					   IoErr());

					FreeFileSys(fs);
					fs = NULL;
					done = TRUE;

				} else if (len == 0) {

					/* true EOF - drop last block */
					lastls->lsb_Next = NULL;
					FreeMem((char *)ls,
						ls->lsb_SummedLongs*4);
					done = TRUE;

				} else if (len != newlen) {

					/* EOF is assumed here */
					ls->lsb_SummedLongs = (len >> 2) + 5;
					done = TRUE;

				} /* else not done yet */
				lastls = ls;

			    } /* while */
			}
		} else {
			if (w)
				Notify(w,"File %s is not a filesystem!",
				       (LONG) name);
		}
		Close(fh);
	} else {
		if (w)
			Notify(w,"Can't open filesystem %s!",
			       (LONG) name);
	}

	return fs;
}

