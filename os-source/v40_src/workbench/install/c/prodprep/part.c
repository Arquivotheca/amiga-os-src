/* part.c - code to handle the partitioning screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>

#include <dos/dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <ctype.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <clib/alib_protos.h>

#include "protos.h"
#include "global.h"
#include "refresh.h"

extern struct SysBase *SysBase;
extern struct DOSBase *DOSBase;

#define SAME	0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

/* create after CurrentPart */

int
MakeNewPart (lowcyl,highcyl)
{
	register struct PartitionBlock *newp;

	if (newp = AllocNew(PartitionBlock))
	{
		InitPartition(SelectedDrive,newp,lowcyl,highcyl);
		if (CurrentPart)
		{
			newp->pb_Next  = CurrentPart->pb_Next;
			CurrentPart->pb_Next = newp;
		} else {
			newp->pb_Next  = 
				SelectedDrive->rdb->rdb_PartitionList;
			SelectedDrive->rdb->rdb_PartitionList = newp;
		}

		CurrentPart = newp;
		if (!newp->pb_Next)
			LastPart = newp;

		return TRUE;
	}

	return FALSE;
}

void
InitPartition (d,p,low,hi)
	register struct Drive *d;
	register struct PartitionBlock *p;
	LONG low,hi;
{
	p->pb_ID	    = IDNAME_PARTITION;
	p->pb_SummedLongs   = sizeof(*p) >> 2;
	p->pb_HostID	    = 7;
	p->pb_Next	    = NULL;
	p->pb_Flags	    = 0;	/* default is mounted, not bootable */
	p->pb_DevFlags	    = 0;
	strcpy(&p->pb_DriveName[0],"CHANGE_ME");

	/* handle cylblocks != heads * sectors */

	if (d->rdb->rdb_Heads * d->rdb->rdb_Sectors ==
	    d->rdb->rdb_CylBlocks)
	{
		p->pb_Environment[DE_NUMHEADS]     = d->rdb->rdb_Heads;
		p->pb_Environment[DE_BLKSPERTRACK] = d->rdb->rdb_Sectors;
	} else {
		p->pb_Environment[DE_NUMHEADS]     = 1;
		p->pb_Environment[DE_BLKSPERTRACK] = d->rdb->rdb_CylBlocks;
	}
	p->pb_Environment[DE_LOWCYL]	   = low;
	p->pb_Environment[DE_UPPERCYL]	   = hi;
	p->pb_Environment[DE_BOOTPRI]	   = 0;

	/* Should we InitSFS for the first partition of each drive??? */
	InitFFS(p);	/* sets up most of the rest */
}

/* init a partition to be FFS */

void
InitFFS (p)
	register struct PartitionBlock *p;
{
	p->pb_Environment[DE_TABLESIZE]    = 16;
	p->pb_Environment[DE_DOSTYPE]      = 0x444f5301;
	p->pb_Environment[DE_MEMBUFTYPE]   = 0;
	p->pb_Environment[DE_NUMBUFFERS]   = 30;
	p->pb_Environment[DE_RESERVEDBLKS] = 2;
	p->pb_Environment[DE_PREFAC] 	   = 0;

	/* we want to have the driver deal with dma-ableness */
	/* for example, the A3000 can DMA anywhere. */
	/* A590/A2091 had versions of <= 6.xx.  A3000 has Version 36. */
	/* FIX! */
	if (scsiversion >= 36)
	{
		p->pb_Environment[DE_MASK] 	   = 0xfffffffe;
	} else {
		p->pb_Environment[DE_MASK] 	   = 0x00fffffe;
	}
	/* FIX! should change this too */
	p->pb_Environment[DE_MAXTRANSFER]  = 0x00ffffff;
	p->pb_Environment[DE_SIZEBLOCK]    = 128;  /* 512 bytes */
	p->pb_Environment[DE_SECSPERBLK]   = 1;    /* 512 bytes */
	p->pb_Environment[DE_INTERLEAVE]   = 0;
	p->pb_Environment[DE_BOOTBLOCKS]   = 0;
}

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
	LONG size;

	if (fh = Open(name,MODE_OLDFILE))
	{
		size = Read(fh,(char *) &temp,4);
		if (size != 4)
			goto read_error;
		if (temp == 0x3F3)			/* hunk_header */
		{
			if (Seek(fh,0,OFFSET_BEGINNING) == -1)
				goto read_error;
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
read_error:				if (w)
					  Notify(w,
					   "Error %d while reading filesystem!",
					   IoErr());

					if (fs)
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

