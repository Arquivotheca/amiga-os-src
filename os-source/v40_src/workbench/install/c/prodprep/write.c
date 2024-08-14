/* write.c */

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

#include "scsi.h"
#include "scsidisk.h"

#define SAME 0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

#define GetBlock(n) (&array[((n) - d->Block) * rdb->rdb_BlockBytes])

extern struct SysBase *SysBase;
extern struct DOSBase *DOSBase;

extern char *device;
extern int  unit;

extern int makeflags;

/* TRUE == success, FALSE == failure */

int
WriteRDSK (rdsk,size,offset)
	CPTR rdsk;
	LONG size;
	LONG offset;
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port;
	int  result = FALSE, opened = FALSE,i;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(device,unit,
			   (struct IORequest *) ior,0L))
	{
		printf("Error %d on device open!\n",i);
		goto cleanup;
	}
	opened = TRUE;


	ior->io_Data 	= (APTR) rdsk;
	ior->io_Length	= size;
	ior->io_Offset	= offset;
	ior->io_Command	= CMD_WRITE;

	/* now write out array starting at block d->Block */
	DoIO((struct IORequest *) ior);

	result = (ior->io_Error == 0);

	if (ior->io_Error)
		printf("Error %d on write!\n",(LONG) (ior->io_Error));

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	return result;
}

int
ReadRDSK (rdsk,size,offset)
	CPTR rdsk;
	LONG size;
	LONG offset;
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port;
	int  result = FALSE, opened = FALSE,i;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(device,unit,
			   (struct IORequest *) ior,0L))
	{
		printf("Error %d on device open!\n",i);
		goto cleanup;
	}
	opened = TRUE;


	ior->io_Data 	= (APTR) rdsk;
	ior->io_Length	= size;
	ior->io_Offset	= offset;
	ior->io_Command	= CMD_READ;

	/* now write out array starting at block d->Block */
	DoIO((struct IORequest *) ior);

	result = (ior->io_Error == 0);

	if (ior->io_Error)
		printf("Error %d on read!\n",(LONG) (ior->io_Error));

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	return result;
}

/* do any SCSI command using the supplied iorequest, ret 0=success or error */
/* data must be word aligned and dmaable */
/* ditto for command */

UBYTE  sensedata[255];
struct SCSICmd cmdblk;

int
DoSCSI (ior,command,clen,data,dlen,flags)
	register struct IOStdReq *ior;
	UWORD *command;
	ULONG clen;
	UWORD *data;
	ULONG dlen;
	ULONG flags; /* only a ubyte used for actual xfer, rest for this rtn */
{

	SendSCSI(ior,command,clen,data,dlen,flags);
	WaitIO((struct IORequest *) ior);

#ifdef TEST_SCSI
printf("direct scsi return error %d, status %d\n",ior->io_Error,cmdblk.scsi_Status);
if (cmdblk.scsi_SenseActual)
{
int i;
printf("Sense data (length %d) = 0x",cmdblk.scsi_SenseActual);
for(i = 0; i < cmdblk.scsi_SenseActual; i++)
printf("%02.2x",sensedata[i]);
printf("\n");
}
#endif
	if (cmdblk.scsi_Status)
		return (int) cmdblk.scsi_Status;

	return (int) ior->io_Error;	/* see scsidisk.h for errors */
}

void
SendSCSI (ior,command,clen,data,dlen,flags)
	register struct IOStdReq *ior;
	UWORD *command;
	ULONG clen;
	UWORD *data;
	ULONG dlen;
	ULONG flags; /* only a ubyte used for actual xfer, rest for this rtn */
{
	/* first set up ior */
	ior->io_Data    = (APTR) &cmdblk;
	ior->io_Length  = sizeof(cmdblk);
	ior->io_Actual  = ior->io_Offset = 0;
	ior->io_Command = HD_SCSICMD;

	/* now set up cmdblk */
	cmdblk.scsi_Data      = data;
	cmdblk.scsi_Length    = dlen;
	cmdblk.scsi_Actual    = 0;
	cmdblk.scsi_Command   = (UBYTE *) command;
	cmdblk.scsi_CmdLength = clen;
	cmdblk.scsi_CmdActual = 0;
	cmdblk.scsi_Flags     = flags & 0xff; /* probably not needed */
	cmdblk.scsi_Status    = 0;
	cmdblk.scsi_SenseData = sensedata;
	cmdblk.scsi_SenseLength = sizeof(sensedata);
	cmdblk.scsi_SenseActual = 0;
	cmdblk.scsi_NextLinked  = NULL;

	SendIO((struct IORequest *) ior);
}

/* TRUE == success, FALSE == failure */

int
CommitChanges (w,informat)
	struct Window *w;
	int informat;		/* TRUE means don't tell of errors */
{
	register struct Drive *d = SelectedDrive;
	struct RigidDiskBlock *rdb;
	struct PartitionBlock *p,*lastp,*newp;
	struct FileSysHeaderBlock *fs,*lastfs,*newfs;
	struct LoadSegBlock *ls,*lastls,*newls;
	struct BadBlockBlock *bb,*lastbb,*newbb;
	struct IOStdReq *ior = NULL;
	struct MsgPort *port;
	int  result = FALSE, opened = FALSE;
	LONG maxblock,blocks,i,flags,low_badblock;
	char *array = NULL;

	if (!d || !d->rdb)		/* sanity */
		return FALSE;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	/* now write out array starting at block d->Block */
	if (i = OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
			   (struct IORequest *) ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}
	opened = TRUE;


	/* reboot WILL be needed! */
	ForceReboot = TRUE;

	/* ok, the user wants us to write it out */

	/* first, get the memory */
	blocks = CountBlocks(d->rdb);

	if (!(array = AllocMem(blocks * d->rdb->rdb_BlockBytes + 20,
			       MEMF_PUBLIC | MEMF_CLEAR)))	/* ^ safety*/
		goto cleanup;

	ior->io_Data 	= (APTR) array;
	ior->io_Length	= blocks * d->rdb->rdb_BlockBytes;
	ior->io_Offset	= d->Block * d->rdb->rdb_BlockBytes;
	ior->io_Command	= CMD_WRITE;

	/* set the LASTLUN, LASTTID, or LAST flags in the RDB if needed */
	/* since the driver only handles one board, LASTTID|LASTLUN == LAST */

        if (makeflags) {        /* if we read a valid RDSK, don't assume we know
                                   where we are in the SCSI chain */

          flags = CalcFlags(d);

	  if (flags != (d->rdb->rdb_Flags &
		      (RDBFF_LAST|RDBFF_LASTLUN|RDBFF_LASTTID)))
	  {
		  d->Flags |= UPDATE;
		  d->rdb->rdb_Flags &= ~(RDBFF_LAST|RDBFF_LASTLUN|RDBFF_LASTTID);
		  d->rdb->rdb_Flags |= flags;
          }
	}

	rdb  = (struct RigidDiskBlock *) array;
	*rdb = *(d->rdb);	/* structure copy */
	rdb->rdb_HostID = HOST_ID;

	/* set all the null pointers to 0xffffffff */
	rdb->rdb_BadBlockList	   = (struct BadBlockBlock *)      0xffffffff;
	rdb->rdb_PartitionList     = (struct PartitionBlock *)     0xffffffff;
	rdb->rdb_FileSysHeaderList = (struct FileSysHeaderBlock *) 0xffffffff;
	rdb->rdb_DriveInit 	   = (struct LoadSegBlock *)	   0xffffffff;
	for (i = 0; i < 6; i++)
		rdb->rdb_Reserved1[i] = (ULONG) 		   0xffffffff;

	maxblock = d->Block;	/* start at the old rigid disk block, or 0 */

	/* Partitions next */
	for (lastp = NULL, p = d->rdb->rdb_PartitionList;
	     p;
	     lastp = newp, p = p->pb_Next)
	{
		maxblock++;
		if (lastp)
			lastp->pb_Next = (struct PartitionBlock *) maxblock;
		else
			rdb->rdb_PartitionList = (struct PartitionBlock *)
						 maxblock;

		newp = (struct PartitionBlock *) GetBlock(maxblock);
		*newp = *p; /* structure copy */
		CtoBStr(&(newp->pb_DriveName[0]));

		newp->pb_HostID = HOST_ID;
		newp->pb_Next   = (struct PartitionBlock *) 0xffffffff;
	}

	/* now file systems */
	for (lastfs = NULL, fs = d->rdb->rdb_FileSysHeaderList;
	     fs;
	     lastfs = newfs, fs = fs->fhb_Next)
	{
		maxblock++;
		if (lastfs)
			lastfs->fhb_Next =
					(struct FileSysHeaderBlock *) maxblock;
		else
			rdb->rdb_FileSysHeaderList = 
					(struct FileSysHeaderBlock *) maxblock;

		newfs = (struct FileSysHeaderBlock *) GetBlock(maxblock);
		*newfs = *fs; /* structure copy */

		newfs->fhb_HostID = HOST_ID;

		newfs->fhb_SegListBlocks = (struct LoadSegBlock *) 0xffffffff;
		newfs->fhb_Next = (struct FileSysHeaderBlock *) 0xffffffff;

		/* now loadsegblocks */
		for (lastls = NULL, ls = fs->fhb_SegListBlocks;
		     ls;
		     lastls = newls, ls = ls->lsb_Next)
		{
			maxblock++;
			if (lastls)
				lastls->lsb_Next = (struct LoadSegBlock *)
						   maxblock;
			else
				newfs->fhb_SegListBlocks =
					       (struct LoadSegBlock *) maxblock;

			newls = (struct LoadSegBlock *) GetBlock(maxblock);

			/* to, from, size in bytes */
			memcpy((char *) newls, (char *) ls,
			       ls->lsb_SummedLongs << 2);

			newls->lsb_HostID = HOST_ID;
			newls->lsb_Next   = (struct LoadSegBlock *) 0xffffffff;
		}
	}

	/* now bad blocks */
	low_badblock = d->rdb->rdb_RDBBlocksHi + 1;

	for (lastbb = NULL, bb = d->rdb->rdb_BadBlockList;
	     bb;
	     lastbb = newbb, bb = bb->bbb_Next)
	{
		maxblock++;
		if (lastbb)
			lastbb->bbb_Next = (struct BadBlockBlock *) maxblock;
		else
			rdb->rdb_BadBlockList = (struct BadBlockBlock *)
						maxblock;

		newbb = (struct BadBlockBlock *) GetBlock(maxblock);

		/* to, from, size in bytes */
		memcpy((char *) newbb,(char *) bb,bb->bbb_SummedLongs << 2);

		{
			short i;
			LONG  good;
			ULONG *map;

			if (d->rdb->rdb_Interleave > 1)
			{
				/* this usage of sectors is correct */
				map = BuildInterleaveMap(d->rdb->rdb_Interleave,
							 d->rdb->rdb_Sectors);
				if (!map)
					goto cleanup;
			}

			/* 2 longs per entry */
			for (i = 0; i < (newbb->bbb_SummedLongs - 6)/2; i++)
			{
			    /* first, maintain low_badblock */
			    good = newbb->bbb_BlockPairs[i].bbe_GoodBlock;

			    /* first, maintain low_badblock */
			    if (good > 0 && good < low_badblock)
			    {
				low_badblock = good;
			    }

#ifdef USEINTERLEAVE
			    /* second, translate sector numbers re: interleave*/
			    /* needed for switching interleaves.	      */
			    /* map already built			      */
			    /* all sectors internally are interleave 1        */
			    if (d->rdb->rdb_Interleave > 1)
			    {
				LONG sec;

				/* first get sector on cylinder */
				sec = newbb->bbb_BlockPairs[i].bbe_BadBlock %
				      d->rdb->rdb_CylBlocks;

				/* I _think_ this use of sector is ok */
				/* now make that sector on head */
				sec = sec % d->rdb->rdb_Sectors;

				/* '- sec' to give start of head */
				newbb->bbb_BlockPairs[i].bbe_BadBlock +=
					map[sec] - sec;
			    }
#endif
			}

			if (d->rdb->rdb_Interleave > 1)
				FreeMem((char *) map,d->rdb->rdb_Sectors * 4);
		}

		newbb->bbb_HostID = HOST_ID;
		newbb->bbb_Next   = (struct BadBlockBlock *) 0xffffffff;
	}

	/* drive init code */
	for (lastls = NULL, ls = d->rdb->rdb_DriveInit;
	     ls;
	     lastls = newls, ls = ls->lsb_Next)
	{
		maxblock++;
		if (lastls)
			lastls->lsb_Next = (struct LoadSegBlock *)
					   maxblock;
		else
			rdb->rdb_DriveInit = (struct LoadSegBlock *) maxblock;

		newls = (struct LoadSegBlock *) GetBlock(maxblock);

		/* to, from, size in bytes */
		memcpy((char *) newls, (char *) ls,
		       ls->lsb_SummedLongs << 2);

		newls->lsb_HostID = HOST_ID;
		newls->lsb_Next   = (struct LoadSegBlock *) 0xffffffff;
	}

	rdb->rdb_HighRDSKBlock = maxblock;

	if (maxblock >= low_badblock)
	{
		Notify(w,"Save failed, not enough space for configuration data",
		       0);
/* FIX! - add rdsk cylinder gadget */
		Notify(w,
	       "Try a low-level format, or removing file systems or partitions",
		       0);
		goto cleanup;
	}

	/* checksum all the blocks */
	for (i = d->Block; i <= maxblock; i++)
		CheckSumBlock((struct RigidDiskBlock *) GetBlock(i));


/*
{ BPTR fh;
fh = Open("ram:rdsk",MODE_NEWFILE);
Write(fh,array,blocks * rdb->rdb_BlockBytes);
Close(fh);
printf("sending to disk\n");
}
*/

	/* now write out array starting at block d->Block */
	DoIO((struct IORequest *) ior);

#ifdef DEBUG
{ BPTR fh;
fh = Open("ram:rdsk_after_doio",MODE_NEWFILE);
Write(fh,array,blocks * rdb->rdb_BlockBytes);
Close(fh);
printf("sending to disk\n");
}
#endif

	if (ior->io_Error)
	{
		if (!informat)
		{
			Notify(w,"Error %d on write!",(LONG) (ior->io_Error));
			goto cleanup;
		} else {
			/* don't care if it failed for format */
			return TRUE;
		}
	}

	/* note that this drive had it's RDSK written out */
	d->Flags |= WRITTEN;

	result = TRUE;

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);
	if (array)
		FreeMem(array,blocks * d->rdb->rdb_BlockBytes + 20);
								/* ^ safety */
	return result;
}

int
CountBlocks (rdb)
	struct RigidDiskBlock *rdb;
{
	register struct PartitionBlock *p;
	register struct FileSysHeaderBlock *fsb;
	struct LoadSegBlock *lsb;
	struct BadBlockBlock *bb;
	register int num;

	num = 1;

	for (p = rdb->rdb_PartitionList; p; p = p->pb_Next)
		num++;

	for (fsb = rdb->rdb_FileSysHeaderList; fsb; fsb = fsb->fhb_Next)
	{
		num++;
		for (lsb = fsb->fhb_SegListBlocks; lsb; lsb = lsb->lsb_Next)
			num++;
	}

	for (bb = rdb->rdb_BadBlockList; bb; bb = bb->bbb_Next)
		num++;

	for (lsb = rdb->rdb_DriveInit; lsb; lsb = lsb->lsb_Next)
		num++;

	return num;
}

/* return the LASTLUN, LASTTID, or LAST flags */
/* if the driver handles one board, LASTTID|LASTLUN == LAST */

LONG
CalcFlags (d)
	register struct Drive *d;
{
	register struct Drive *d2;
	register LONG flags = 0;

	if (d->NextDrive == NULL)
	{
		flags = RDBFF_LAST | RDBFF_LASTLUN | RDBFF_LASTTID;

	} else if (strcmp(&(d->NextDrive->DeviceName[0]),&(d->DeviceName[0])) !=
		   SAME) {

		/* Last of this device driver - set all flags */
		flags = RDBFF_LAST | RDBFF_LASTLUN | RDBFF_LASTTID;

	} else if (d->NextDrive->Addr != d->Addr) {

		/* last at this address */
		flags = RDBFF_LASTLUN;
		if (d->NextDrive->Addr / 100 != d->Addr / 100)
		{
			/* next one is on another board */
			flags |= RDBFF_LASTTID;
		}
	} else {

		/* are all the rest on the same address and driver? */
		flags |= RDBFF_LASTTID;				/* assume yes */
		for (d2 = d->NextDrive; d2; d2 = d2->NextDrive)
			if (d2->Addr != d->Addr ||
			    strcmp(&(d2->DeviceName[0]),
				   &(d->DeviceName[0])) != SAME)
			{
				/* no, there's one at a diff addr or driver */
				flags &= ~RDBFF_LASTTID;
				break;
			}
	}
	return flags;
}

/* build an interleave translation table - essentially a hash table */

ULONG *
BuildInterleaveMap (interleave,sectors)
	ULONG interleave,sectors;
{
	ULONG *ptr,sec,lastsec;

	if (ptr = (ULONG *) AllocMem(sectors * sizeof(ULONG),MEMF_CLEAR))
	{
		lastsec = 0;
		for (sec = 1; sec < sectors; sec++)
		{
			lastsec = (lastsec + interleave) % sectors;
			while (1)
			{
				/* make sure it isn't used already */
				if (!lastsec || ptr[lastsec])
					lastsec++;
				else
					break;
			}
			ptr[lastsec] = sec;
		}
	}

	return ptr;
}

/* searches a map for a sector */
ULONG
FindSector (map,sector)
	ULONG *map,sector;
{
	ULONG i = 0;

	/* assumes match will be found! */
	while (map[i] != sector)
		i++;

	return i;
}

int
GetRDB (ior,d)
	struct IOStdReq *ior;
	struct Drive *d;
{
	ULONG size = 512;	/* default and only for st506 */
	register struct RigidDiskBlock *rdb;
	register struct RigidDiskBlock *newrdb;
	ULONG block;
	short i;

#ifdef BIGBLOCKS
	/* we use SCSI read commands for st506/xt drives as well because  */
	/* the driver will interpret them for us.			  */

	/* Lattice will always word-align these */
	unsigned char modesense[6];
	unsigned char modes[12];  /* inc descriptors, not inc parameters */

	/* read the sector size from the drive */
	modesense[0] = S_MODE_SENSE;
	modesense[4] = 12;
	modesense[1] = 			/* driver will fill in lun */
	modesense[2] = 
	modesense[3] = 
	modesense[5] =
	modesense[6] = 0;

	if (!DoSCSI(ior,(UWORD *) modesense,6,(UWORD *) modes,12,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		size = modes[9] * 0x10000 + modes[10] * 0x100 + modes[11];
	} else {
		/* can't get modesense back, assume 512 */
		size = 512;
	}
#endif

	if (!(rdb = (struct RigidDiskBlock *)
		    AllocMem(size,MEMF_PUBLIC|MEMF_CLEAR)))
		return NULL;

	/* first find rigid disk block, if it exists */
	for (block = 0; block < RDB_LOCATION_LIMIT; block++)
	{
		/* if read fails, abort immediately */
		if (!ReadBlock(ior,(APTR) rdb,size,block))
		{
			FreeMem((char *) rdb, size);
			return NULL;
		}
		/* make sure it's a rigiddiskblock */
		if (rdb->rdb_ID == IDNAME_RIGIDDISK &&
		    CheckCheckSum(rdb))
			break;
	}
	if (block == RDB_LOCATION_LIMIT)	/* i.e. didn't find it */
	{
		FreeMem((char *) rdb, size);
		return NULL;	/* didn't find it, but no errors */
	}

	if (!(newrdb = AllocNew(RigidDiskBlock)))
	{
		FreeMem((char *) rdb, size);
		return NULL;
	}

	*newrdb = *rdb;		/* copy the rigid disk portion of the sector */

	/* clear unused chains for easy error handling */
	for (i = 0; i < 6; i++)
		newrdb->rdb_Reserved1[i] = NULL;

	/* we have a valid rigid disk block! */
	if (!ReadBadBlockList(ior,newrdb,size,(struct BadBlockBlock *) rdb))
		goto bbb_error;
	if (!ReadPartitionList(ior,newrdb,size,(struct PartitionBlock *) rdb))
		goto pb_error;
	if (!ReadFileSysList(ior,newrdb,size,(struct FileSysHeaderBlock *) rdb))
		goto fsb_error;
	if (!ReadDriveInitList(ior,newrdb,size,(struct LoadSegBlock *) rdb))
		goto dvi_error;

	d->rdb    = newrdb;
	d->oldrdb = CopyRDB(newrdb);	/* save until rewritten to disk */
	d->Block  = block;
	d->Flags |= PREPED;

	if (memcmp(newrdb->rdb_ControllerVendor,"Adaptec ",8) == SAME)
	{
		d->Flags |= ADAPTEC;
	}

	FreeMem((char *) rdb, size);

	return TRUE;

/* now the error handling code - easier to do this way so I don't free	   */
/* random memory pointed to by block numbers. All reserved lists are null. */

bbb_error:	newrdb->rdb_BadBlockList	= NULL;
pb_error:	newrdb->rdb_PartitionList	= NULL;
fsb_error:	newrdb->rdb_FileSysHeaderList	= NULL;
dvi_error:	newrdb->rdb_DriveInit		= NULL;

/*printf("Error reading sublists!\n");*/
	/* One of them had an error, dump the rigid disk block */
	FreeMem((char *) rdb, size);	/* the temp buffer 	 */
	FreeRDB(newrdb);		/* all the stuff read in */

	return FALSE;
}

int
ReadBadBlockList (ior,rdb,size,sec)
	struct IOStdReq *ior;
	struct RigidDiskBlock *rdb;
	long size;
	struct BadBlockBlock *sec;	/* size of sector */
{
	register struct BadBlockBlock *b,*lastb;
	register ULONG nextblock;
#ifdef USEINTERLEAVE
	register LONG sector;
	register short i;
	ULONG *map;
#endif

	nextblock = (ULONG) rdb->rdb_BadBlockList;
	rdb->rdb_BadBlockList = NULL;
	if (nextblock != 0xffffffff)
	{
		lastb = NULL;
		while (nextblock != 0xffffffff) {
			if (!(b = (struct BadBlockBlock *)
				  AllocMem(size,MEMF_PUBLIC)))
				return FALSE;

			if (!ReadBlockCheck(ior,(APTR) sec,size,nextblock,
					    IDNAME_BADBLOCK))
			{
				FreeMem((char *) b, size);
				return FALSE;
			}

			/* memcpy is to,from,size */
			memcpy((char *) b,(char *) sec,size);/* size may vary */

#ifdef USEINTERLEAVE
			if (rdb->rdb_Interleave > 1)
			{
				/* this use of sectors is ok */
				map = BuildInterleaveMap(rdb->rdb_Interleave,
							 rdb->rdb_Sectors);
				if (!map)
				{
					FreeMem((char *) b, size);
					return FALSE;
				}
			}

			/* 2 longs per entry */
			/* the bbe struct is 8 bytes long */
			for (i = 0; i < (b->bbb_SummedLongs - 6)/2; i++)
			{
			    /* translate sector numbers re: interleave        */
			    /* needed for switching interleaves.	      */
			    /* all sectors internally are interleave 1        */
			    if (rdb->rdb_Interleave > 1)
			    {
				/* first get sector on cylinder */
				sector = b->bbb_BlockPairs[i].bbe_BadBlock %
				         rdb->rdb_CylBlocks;

				/* I _think_ this use of sectors is ok */
				/* now make that sector on head */
				sector = sector % rdb->rdb_Sectors;

				/* translate to interleave 1           */
				/* '- sector' to give start of head    */
				/* FindSector searches table for match */

				b->bbb_BlockPairs[i].bbe_BadBlock +=
					FindSector(map,sector) - sector;
			    }
			}
			if (rdb->rdb_Interleave > 1)
				FreeMem((char *) map,rdb->rdb_Sectors * 4);
#endif

			if (lastb == NULL)
				rdb->rdb_BadBlockList = b;
			else
				lastb->bbb_Next = b;

/*printf("Got BadBlock block %d\n",nextblock);*/

			nextblock = (LONG) b->bbb_Next;
			b->bbb_Next = NULL;

			lastb = b;
		}
	}
	return TRUE;
}

int
ReadPartitionList (ior,rdb,size,sec)
	struct IOStdReq *ior;
	struct RigidDiskBlock *rdb;
	long size;
	struct PartitionBlock *sec;	/* size of sector */
{
	register struct PartitionBlock *p,*lastp;
	register LONG nextblock;

	nextblock = (LONG) rdb->rdb_PartitionList;
	rdb->rdb_PartitionList = NULL;
	if (nextblock != 0xffffffff)
	{
		lastp = NULL;
		while (nextblock != 0xffffffff) {
			if (!(p = AllocNew(PartitionBlock)))
				return FALSE;
			if (!ReadBlockCheck(ior,(APTR) sec,size,nextblock,
					    IDNAME_PARTITION))
			{
				FreeMem((char *) p, sizeof(*p));
				return FALSE;
			}
			*p = *sec;

			if (lastp == NULL)
				rdb->rdb_PartitionList = p;
			else
				lastp->pb_Next = p;

			BtoCStr(p->pb_DriveName);

/*printf("Got partition %s at block %d\n",p->pb_DriveName,nextblock);*/

			nextblock = (LONG) p->pb_Next;
			p->pb_Next = NULL;

			lastp = p;
		}
	}
	return TRUE;
}

int
ReadFileSysList (ior,rdb,size,sec)
	struct IOStdReq *ior;
	struct RigidDiskBlock *rdb;
	long size;
	struct FileSysHeaderBlock *sec;	/* size of sector */
{
	register struct FileSysHeaderBlock *f,*lastf;
	register LONG nextblock;

	nextblock = (LONG) rdb->rdb_FileSysHeaderList;
	rdb->rdb_FileSysHeaderList = NULL;
	if (nextblock != 0xffffffff)
	{
		lastf = NULL;
		while (nextblock != 0xffffffff) {
			if (!(f = AllocNew(FileSysHeaderBlock)))
				return FALSE;
			if (!ReadBlockCheck(ior,(APTR) sec,size,nextblock,
					    IDNAME_FILESYSHEADER))
			{
				FreeMem((char *) f, sizeof(*f));
				return FALSE;
			}
			*f = *sec;	/* structure copy */
			if (lastf == NULL)
				rdb->rdb_FileSysHeaderList = f;
			else
				lastf->fhb_Next = f;

/*printf("Got FileSystem 0x%lx at block %d\n",f->fhb_DosType,nextblock);*/

			nextblock = (LONG) f->fhb_Next;
			f->fhb_Next = NULL;

			/* now read in the file system */
			if (!ReadLoadSegList(ior,f,size,
					     (struct LoadSegBlock *) sec))
			{
				FreeMem((char *) f, sizeof(*f));
				if (lastf)
					lastf->fhb_Next = NULL;
				else
					rdb->rdb_FileSysHeaderList = NULL;

				return FALSE;
			}

			lastf = f;
		}
	}
	return TRUE;
}

int
ReadDriveInitList (ior,rdb,size,sec)
	struct IOStdReq *ior;
	struct RigidDiskBlock *rdb;
	long size;
	struct LoadSegBlock *sec;	/* size of sector */
{
	register struct LoadSegBlock *l,*lastl;
	register LONG nextblock;

	nextblock = (LONG) rdb->rdb_DriveInit;
	rdb->rdb_DriveInit = NULL;
	if (nextblock != 0xffffffff)
	{
		lastl = NULL;
		while (nextblock != 0xffffffff) {
			if (!(l = (struct LoadSegBlock *)
				  AllocMem(size,MEMF_PUBLIC)))
				return FALSE;
			if (!ReadBlockCheck(ior,(APTR) sec,size,nextblock,
					    IDNAME_LOADSEG))
			{
				FreeMem((char *) l, size);
				return FALSE;
			}

			/* memcpy is to,from,size */
			memcpy((char *) l,(char *) sec,size);

			if (lastl == NULL)
				rdb->rdb_DriveInit = l;
			else
				lastl->lsb_Next = l;

/*printf("Got LoadSegBlock block at %d\n",nextblock);*/

			nextblock = (LONG) l->lsb_Next;
			l->lsb_Next = NULL;

			lastl = l;
		}
	}
	return TRUE;
}

int
ReadLoadSegList (ior,f,size,sec)
	struct IOStdReq *ior;
	struct FileSysHeaderBlock *f;
	long size;
	struct LoadSegBlock *sec;	/* size of sector */
{
	register struct LoadSegBlock *l,*lastl;
	register LONG nextblock;

	nextblock = (LONG) f->fhb_SegListBlocks;
	f->fhb_SegListBlocks = NULL;
	if (nextblock != 0xffffffff)
	{
		lastl = NULL;
		while (nextblock != 0xffffffff) {
			if (!(l = (struct LoadSegBlock *)
				  AllocMem(size,MEMF_PUBLIC)))
				return FALSE;
			if (!ReadBlockCheck(ior,(APTR) sec,size,nextblock,
					    IDNAME_LOADSEG))
			{
				FreeMem((char *) l, size);
				return FALSE;
			}

			/* memcpy is to,from,size */
			memcpy((char *) l,(char *) sec,size);

			if (lastl == NULL)
				f->fhb_SegListBlocks = l;
			else
				lastl->lsb_Next = l;

/*printf("Got LoadSegBlock block at %d\n",nextblock);*/

			nextblock = (LONG) l->lsb_Next;
			l->lsb_Next = NULL;

			lastl = l;
		}
	}
	return TRUE;
}

/* read a block off the drive.  ior is already opened */

int
ReadBlock (ior,dest,size,block)
	register struct IOStdReq *ior;
	APTR dest;
	LONG size;	/* size of blocks */
	LONG block;	/* which block # to read */
{
	ior->io_Data   = dest;
	ior->io_Offset = size * block;
	ior->io_Length = size;
	ior->io_Command = CMD_READ;

	DoIO((struct IORequest *) ior);
	return (ior->io_Error == 0);
}

/* Read a block, verify ID and checksum */

int
ReadBlockCheck (ior,dest,size,block,id)
	struct IOStdReq *ior;
	APTR dest;
	LONG size;	/* size of blocks */
	LONG block;	/* which block # to read */
	LONG id;
{
	if (ReadBlock(ior,dest,size,block) &&
	    *((LONG *) dest) == id &&
	    CheckCheckSum((struct RigidDiskBlock *) dest))
		return TRUE;

	return FALSE;
}

/* convert BCPL string to C string */

void
BtoCStr (str)
	register char *str;
{
	register int len,i;

	len = str[0];
	for (i = 0; i < len; i++)	/* note: overlapping copy */
		str[i] = str[i+1];

	str[len] = '\0';
}

/* check a checksum */

int
CheckCheckSum (rdb)
	struct RigidDiskBlock *rdb;
{
	register LONG *array;
	register LONG checksum;
	register unsigned short i;

	if (rdb->rdb_SummedLongs <= 0 ||
	    rdb->rdb_SummedLongs >= 65536)
		return FALSE;		/* sanity check */

	checksum = 0;
	array = (LONG *) rdb;

	for (i = 0; i < rdb->rdb_SummedLongs; i++)
		checksum += array[i];

	return (checksum == 0);
}

