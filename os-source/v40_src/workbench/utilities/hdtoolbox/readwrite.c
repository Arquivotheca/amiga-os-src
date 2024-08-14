/* readwrite.c - code to handle the prep main screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/errors.h>
#include <devices/trackdisk.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <dos/filehandler.h>

#include <dos.h>
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

#include "refresh.h"
#include "protos.h"
#include <clib/alib_protos.h>

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"

#include "scsi.h"
#include "scsidisk.h"

#define SAME 0

#define GetBlock(n) (&array[((n) - d->Block) * rdb->rdb_BlockBytes])

#define USEINTERLEAVE	/* for XT drives */

extern struct TextFont *tfont;	/* 40.3 */
extern UWORD XtraTop;	/* 40.3 */

/* TRUE == success, FALSE == failure */
/* TRUE means don't tell of errors */

int CommitChanges(struct Window *w,int informat)
{
	register struct Drive *d = SelectedDrive;
	struct RigidDiskBlock *rdb;
	struct PartitionBlock *p,*lastp,*newp;
	struct FileSysHeaderBlock *fs,*lastfs,*newfs;
	struct LoadSegBlock *ls,*lastls,*newls;
	struct BadBlockBlock *bb,*lastbb,*newbb;
	struct IOStdReq *ior = NULL;
	struct MsgPort *port = NULL;
	int  result = FALSE,opened = FALSE;
	LONG maxblock,blocks,i,flags,low_badblock;
	char *array = NULL;

	if (!d || !d->rdb) return FALSE;

	if (!(port = CreatePort(0L,0L))) goto cleanup;
	if (!(ior = CreateStdIO(port))) goto cleanup;

	/* now write out array starting at block d->Block */
	if (i = OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
		(struct IORequest *)ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}

	opened = TRUE;

	if (d->oldrdb && AnalyzePartitions(d->rdb,d->oldrdb))
	{
		if (!AskSure(w,CommitText))
			goto cleanup;
		else
			ForceReboot = TRUE;     /* reboot WILL be needed! */
	} /* don't ask if nothing will be lost or wasn't preped before */

	/* ok, the user wants us to write it out */

	/* try to reassign any bad blocks the user entered */
	ReassignAll(d,ior);

	/* first, get the memory */
	blocks = CountBlocks(d->rdb);

	if (!(array = AllocMem(blocks * d->rdb->rdb_BlockBytes + 20,
		MEMF_PUBLIC | MEMF_CLEAR))) goto cleanup;

	ior->io_Data    = (APTR) array;
	ior->io_Length  = blocks * d->rdb->rdb_BlockBytes;
	ior->io_Offset  = d->Block * d->rdb->rdb_BlockBytes;
	ior->io_Command = CMD_WRITE;

	/* set the LASTLUN, LASTTID, or LAST flags in the RDB if needed */
	/* since the driver only handles one board, LASTTID|LASTLUN == LAST */
  	flags = CalcFlags(d);
	if (flags != (d->rdb->rdb_Flags & (RDBFF_LAST | RDBFF_LASTLUN | RDBFF_LASTTID)))
	{
		/* the flags aren't right: fix em */
		d->Flags |= UPDATE;
		d->rdb->rdb_Flags &= ~(RDBFF_LAST|RDBFF_LASTLUN|RDBFF_LASTTID);
		d->rdb->rdb_Flags |= flags;
	}

	rdb  = (struct RigidDiskBlock *) array;
	*rdb = *(d->rdb);     /* structure copy */
	rdb->rdb_HostID = HOST_ID;

	/* set all the null pointers to 0xffffffff */
	rdb->rdb_BadBlockList      = (struct BadBlockBlock *)0xffffffff;
	rdb->rdb_PartitionList     = (struct PartitionBlock *)0xffffffff;
	rdb->rdb_FileSysHeaderList = (struct FileSysHeaderBlock *)0xffffffff;
	rdb->rdb_DriveInit         = (struct LoadSegBlock *)0xffffffff;

	for (i = 0; i < 6; i++)
		rdb->rdb_Reserved1[i] = (ULONG)0xffffffff;

	maxblock = d->Block;	/* start at the old rigid disk block, or 0 */

	/* Partitions next */
	for (lastp = NULL,p = d->rdb->rdb_PartitionList;
		p;
		lastp = newp,p = p->pb_Next)
	{
		maxblock++;
		if (lastp)
			lastp->pb_Next = (struct PartitionBlock *)maxblock;
		else
			rdb->rdb_PartitionList = (struct PartitionBlock *)maxblock;

		newp = (struct PartitionBlock *) GetBlock(maxblock);
		*newp = *p; /* structure copy */
		CtoBStr(&(newp->pb_DriveName[0]));

		/* 39.13 - Now partition HostID field is set by users */
		/* newp->pb_HostID = HOST_ID; */
		newp->pb_Next   = (struct PartitionBlock *) 0xffffffff;
	}

	/* now file systems */
	for (lastfs = NULL,fs = d->rdb->rdb_FileSysHeaderList;
		fs;
		lastfs = newfs,fs = fs->fhb_Next)
	{
		maxblock++;
		if (lastfs)
			lastfs->fhb_Next = (struct FileSysHeaderBlock *) maxblock;
		else
			rdb->rdb_FileSysHeaderList = (struct FileSysHeaderBlock *)maxblock;
		newfs = (struct FileSysHeaderBlock *) GetBlock(maxblock);
		*newfs = *fs; /* structure copy */

		newfs->fhb_HostID = HOST_ID;

		newfs->fhb_SegListBlocks = (struct LoadSegBlock *) 0xffffffff;
		newfs->fhb_Next = (struct FileSysHeaderBlock *) 0xffffffff;

		/* now loadsegblocks */
		for (lastls = NULL,ls = fs->fhb_SegListBlocks;
			ls;
			lastls = newls,ls = ls->lsb_Next)
		{
			maxblock++;
			if (lastls)
				lastls->lsb_Next = (struct LoadSegBlock *)maxblock;
			else
				newfs->fhb_SegListBlocks = (struct LoadSegBlock *)maxblock;

			newls = (struct LoadSegBlock *) GetBlock(maxblock);

			/* to, from, size in bytes */
			memcpy((char *) newls,(char *) ls,ls->lsb_SummedLongs << 2);

			newls->lsb_HostID = HOST_ID;
			newls->lsb_Next   = (struct LoadSegBlock *) 0xffffffff;
		}
	}

	/* now bad blocks */
	low_badblock = d->rdb->rdb_RDBBlocksHi + 1;

	for (lastbb = NULL,bb = d->rdb->rdb_BadBlockList;
		bb;
		lastbb = newbb,bb = bb->bbb_Next)
	{
		maxblock++;
		if (lastbb)
			lastbb->bbb_Next = (struct BadBlockBlock *)maxblock;
		else
			rdb->rdb_BadBlockList = (struct BadBlockBlock *)maxblock;

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
				/* second, translate sector numbers                          */
				/* re: interleave needed for switching interleaves.          */
				/* map already built all sectors internally are interleave 1 */
				if (d->rdb->rdb_Interleave > 1)
				{
					LONG sec;

					/* first get sector on cylinder */
					sec = newbb->bbb_BlockPairs[i].bbe_BadBlock % d->rdb->rdb_CylBlocks;

					/* I _think_ this use of sector is ok */
					/* now make that sector on head */
					sec = sec % d->rdb->rdb_Sectors;

					/* '- sec' to give start of head */
					newbb->bbb_BlockPairs[i].bbe_BadBlock += map[sec] - sec;
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
	for (lastls = NULL,ls = d->rdb->rdb_DriveInit;
		ls;
		lastls = newls,ls = ls->lsb_Next)
	{
		maxblock++;
		if (lastls)
			lastls->lsb_Next = (struct LoadSegBlock *)maxblock;
		else
			rdb->rdb_DriveInit = (struct LoadSegBlock *)maxblock;

		newls = (struct LoadSegBlock *) GetBlock(maxblock);

		/* to, from, size in bytes */
		memcpy((char *)newls,(char *)ls,ls->lsb_SummedLongs << 2);

		newls->lsb_HostID = HOST_ID;
		newls->lsb_Next   = (struct LoadSegBlock *) 0xffffffff;
	}

	rdb->rdb_HighRDSKBlock = maxblock;
	if (maxblock >= low_badblock)
	{
		Notify(w, "Save failed, not enough space for configuration data", 0);
		/* FIX! - add rdsk cylinder gadget */
		Notify(w, "Try a low-level format, or removing file systems or partitions", 0);
		goto cleanup;
	}

	/* checksum all the blocks */
	for (i = d->Block;i <= maxblock;i++)
		CheckSumBlock((struct RigidDiskBlock *) GetBlock(i));

	/* now write out array starting at block d->Block */
	DoIO((struct IORequest *) ior);

	if (ior->io_Error)
	{
		if (!informat)
		{
			Notify(w,"Error %d on write!",(LONG) (ior->io_Error));
			goto cleanup;
		}
		else
		{
			/* don't care if it failed for format */
			return TRUE;
		}
	}

	/* note that this drive had it's RDSK written out */
	d->Flags |= WRITTEN;

	result = TRUE;

cleanup:
	if (opened) CloseDevice((struct IORequest *) ior);
	if (ior)    DeleteStdIO(ior);
	if (port)   DeletePort(port);
	if (array)  FreeMem(array,blocks * d->rdb->rdb_BlockBytes + 20);

	return result;
}


int CountBlocks (rdb)
	struct RigidDiskBlock *rdb;
{
	register struct PartitionBlock *p;
	register struct FileSysHeaderBlock *fsb;
	struct LoadSegBlock *lsb;
	struct BadBlockBlock *bb;
	register int num;

	num = 1;

	for (p = rdb->rdb_PartitionList;p;p = p->pb_Next)
		num++;

	for (fsb = rdb->rdb_FileSysHeaderList;fsb;fsb = fsb->fhb_Next)
	{
		num++;
		for (lsb = fsb->fhb_SegListBlocks;lsb;lsb = lsb->lsb_Next)
			num++;
	}

	for (bb = rdb->rdb_BadBlockList;bb;bb = bb->bbb_Next)
		num++;

	for (lsb = rdb->rdb_DriveInit;lsb;lsb = lsb->lsb_Next)
		num++;

	return num;
}

/* compute add-to-zero checksum for a block.  summed_longs must be set up */

void
CheckSumBlock (rdb)
	struct RigidDiskBlock *rdb;	/* may be anything actually */
{
	register LONG *larray;
	register LONG checksum;
	register short i;

	rdb->rdb_ChkSum = 0;

	larray = (LONG *) rdb;
	checksum = 0;
	for (i = 0;i < rdb->rdb_SummedLongs;i++)
	{
		checksum += larray[i];
	}

	rdb->rdb_ChkSum = 0 - checksum;	  /* I prefer EOR checksums, or CRC */
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

	for (i = 0;i < rdb->rdb_SummedLongs;i++)
		checksum += array[i];

	return (checksum == 0);
}

static char *devnames[] = {
	"",
	"2nd.",
	"3rd.",
	"4th.",
	"5th.",
	"6th.",
	"7th.",
	"8th.",
};

/* get all drives in the system */

int
GetDrives (rp,name,flags,numaddr,numlun)
	struct RastPort *rp;
	char *name;			/* of device */
	LONG flags;			/* SCSI or ST506 (aka XT) */
	LONG numaddr,numlun;
{
	register struct Drive *d,*lastd,*firstd;
	struct IOStdReq *ior = NULL;
	struct MsgPort *port = NULL;
	int res = FALSE,done = FALSE;
	unsigned short addr,lun,board;
	char str[80];
	LONG error;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	/* point lastd at last drive in system */
	for (lastd = FirstDrive; lastd && lastd->NextDrive;
	     lastd = lastd->NextDrive)
		; /* NULL */
	firstd = lastd;

	/* handle steve's wierd board multiple handlers */
	for (board = 0; board < 8 && !done; board++)
	{
	    /* for each addr/lun pair */
	    for (addr = 0; addr < numaddr && !done; addr++)
	    {
		for (lun = 0; lun < numlun && !done; lun++)
		{
			sprintf(str,
				"Checking %s%s address %d unit %d...          ",
				devnames[board],name,addr,lun);
			SetFont(rp,tfont);	/* 40.3 */
			Move(rp,140,100 + XtraTop);
			Text(rp,str,strlen(str));
			sprintf(str,"%s%s",devnames[board],name);
			if (!(error = OpenDevice(str,addr + lun * 10,
					         (struct IORequest *) ior, 0L)))
			{
				/* a device exists! */
				found_driver = TRUE;

				/* check the driver version */
				/* so we know if we can use mask=0xffffffff */
				scsiversion =
					ior->io_Device->dd_Library.lib_Version;

				if (!(d = AllocNew(Drive)))
				{
					CloseDevice((struct IORequest *) ior);
					goto cleanup;
				}
				d->NextDrive = NULL;
				if (!FirstDrive)
					FirstDrive = d;
				else
					lastd->NextDrive = d;
				NumDrives++;

				d->Flags = flags;
				d->Addr  = addr;
				d->Lun   = lun;
				d->Block = 0;
				d->SCSIBadBlocks = 0;

				strcpy(d->DeviceName,str);

				if (!GetRDB(ior,d)) GetDriveType(ior,d);

				/* get number of bad blocks */
				if (flags == SCSI)
				  d->SCSIBadBlocks = ReadDefects(ior);

				lastd = d;
				if (!firstd) firstd = d;

				CloseDevice((struct IORequest *) ior);
			} else {
				/* if device doesn't exist, stop */
				if (error == IOERR_OPENFAIL)
					done = TRUE;
				else
					/* we know the driver exists */
					found_driver = TRUE;
			} /* opendevice */
		} /* for lun */
	    } /* for addr */
	} /* for board */

	/* make sure all the lastXXX flags are correct */
	for (d = firstd; d; d = d->NextDrive)
	{
		if (d->rdb)
		{
			flags = CalcFlags(d);

			if (flags != (d->rdb->rdb_Flags &
				      (RDBFF_LAST|RDBFF_LASTLUN|RDBFF_LASTTID)))
			{
				/* the flags aren't right: fix em */
				d->Flags |= UPDATE;
				d->rdb->rdb_Flags &=
				      ~(RDBFF_LAST|RDBFF_LASTLUN|RDBFF_LASTTID);
				d->rdb->rdb_Flags |= flags;
			}
		}
	}

	res = TRUE;
	goto dealloc;

cleanup:
	res = FALSE;
dealloc:
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	return res;
}








/* try to read via inquiry the type of drive and match against drives */
void
GetDriveType (ior,d)
	struct IOStdReq *ior;
	register struct Drive *d;
{
	register struct DriveDef *def;
	char inquiry[6],data[36];
	LONG version = 0,revision = 0;	// 39.9

	inquiry[0] = S_INQUIRY;
	inquiry[1] =
	inquiry[2] =
	inquiry[3] = 0;
	inquiry[4] = 36;
	inquiry[5] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,36,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		if (data[0] != 0 || (data[3] & 0x0F) != 1)
			return;
		if (def = FindDrive(d->Flags & SCSI ? SCSI_Defs : St506_Defs,
				    &data[8],&data[16],&data[32]))
		{
			d->oldrdb = NULL;
			d->bad    = NULL;
			d->rdb    = CopyRDB(def->Initial_RDB);
			d->Flags  = def->ControllerType;
			d->Block  = 0;

			if (!DoQuickSetup(d))
			{
				FreeRDB(d->rdb);
				return;
			}

			GetFSVersion("L:FastFileSystem",&version,&revision);

			/* add FFS to the drive */
			d->rdb->rdb_FileSysHeaderList =
			      MakeFileSys(NULL,d->rdb,"L:FastFileSystem",
					  'DOS\x01',version,revision);

			d->Flags |= UPDATE;
		}
	}
}


#define BIGBLOCKS	// 39.10


int
GetRDB (ior,d)
	struct IOStdReq *ior;
	struct Drive *d;
{
	ULONG size = 512;	/* default and only for st506 */
	register struct RigidDiskBlock *rdb = NULL;
	register struct RigidDiskBlock *newrdb;
	ULONG block;
	short i;

#ifdef BIGBLOCKS
	/* we use SCSI read commands for st506/xt drives as well because  */
	/* the driver will interpret them for us.			  */

	/* This is very critical part, but we need it at least for CD-ROM drive. */
	unsigned char inquiry[10];
	unsigned char data[36];
	ULONG lblock = 0;		/* logical block address */

	/* now try to find out what size the drive is */
	inquiry[0] = S_READ_CAPACITY;
	inquiry[1] = 0;
	inquiry[2] = (lblock & 0xff000000) >> 24;
	inquiry[3] = (lblock & 0x00ff0000) >> 16;
	inquiry[4] = (lblock & 0x0000ff00) >> 8;
	inquiry[5] = (lblock & 0x000000ff);
	inquiry[6] =
	inquiry[7] = 0;
	inquiry[8] = 0;		/* pmi (partical medium indicater) */
	inquiry[9] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,10,(UWORD *) data,8,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		size = ((data[4] << 24) | (data[5] << 16) |
			(data[6] <<  8) | (data[7]));
		if (size == 0)		/* should not happen...., but... */
			goto CD_CHECK;
	} else {
CD_CHECK:
{
		/* FIX for CD-ROM device.
		   At first access to CD, READ CAPACITY command fails.
		   Ask device type and if it's CD-ROM device,
		   we set size 2048 bytes. It prevents lock up drive.  */

		/* ask drive type */
		inquiry[0] = S_INQUIRY;
		inquiry[1] =
		inquiry[2] =
		inquiry[3] = 0;
		inquiry[4] = 36;
		inquiry[5] = 0;

//		kprintf("Failed READ CAPACITY... Send INQUIRY.\n");

		if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,36,
			    SCSIF_READ|SCSIF_AUTOSENSE))
		{
			if (data[0] == 0x05)	/* device type is CD-ROM */
				size = 2048;
			else			/* assume 512 */
				size = 512;
		}
		/* can't get inquiry back, assume 512 */
		else size = 512;
}
	}
//	kprintf("block size = %d\n",size);
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
	d->oldrdb = CopyRDB(newrdb);
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


void AddPartitionList(struct RigidDiskBlock *rdb, struct PartitionBlock *p) {

struct PartitionBlock **sp, *tp;

  if (rdb->rdb_PartitionList == NULL) {
    rdb->rdb_PartitionList = p;
    p->pb_Next = NULL;
    }

  else {
    sp = &rdb->rdb_PartitionList;

    while (*sp) {

      if (p->pb_Environment[DE_LOWCYL] < (*sp)->pb_Environment[DE_LOWCYL]) {
        tp = *sp;
        *sp = p;
        p->pb_Next = tp;
        return;
        }

      sp = &((*sp)->pb_Next);
      }

    *sp = p;
    p->pb_Next = NULL;
    }
  }



int
ReadPartitionList (ior,rdb,size,sec)
	struct IOStdReq *ior;
	struct RigidDiskBlock *rdb;
	long size;
	struct PartitionBlock *sec;	/* size of sector */
{
	register struct PartitionBlock *p;
	register LONG nextblock;

	nextblock = (LONG) rdb->rdb_PartitionList;
	rdb->rdb_PartitionList = NULL;
	if (nextblock != 0xffffffff)
	{
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

/*printf("Got partition %s at block %d\n",p->pb_DriveName,nextblock);*/

			nextblock = (LONG) p->pb_Next;

                        AddPartitionList(rdb, p);

			BtoCStr(p->pb_DriveName);
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

/*printf("Got FileSystem 0x%lx %d at block %d\n",f->fhb_DosType,f->fhb_Version,nextblock);*/

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

/* convert C string to BCPL string */

void
CtoBStr (str)
	register char *str;
{
	register int len,i;

	len = strlen(str);
	for (i = len; i > 0; i--)
		str[i] = str[i-1];

	str[0] = (char) len;
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

/* read all defs in a given directory, perhaps multiple per file */
/* if "dir" is really a file, only read what's in that file.	 */
/* if two defs for same drive, only keep the most recent 	 */

struct DriveDef *
GetDefs (dir,type)
	char *dir;
	int type;	/* SCSI | ST506 */
{
	struct DriveDef *d = NULL;
	char pathname[256];
	register BPTR dirlock;
	register struct FileInfoBlock *fib;	/* must be long-aligned */
	register int len;

	if (!(fib = (struct FileInfoBlock *)
		    AllocMem(sizeof(struct FileInfoBlock),
			     MEMF_PUBLIC|MEMF_CLEAR)))
		return NULL;

	/* build pathname */
	strcpy(pathname,dir);
	len = strlen(pathname);

	if (dirlock = Lock(dir,ACCESS_READ))
	{
		if (Examine(dirlock,fib))
		{
			if (fib->fib_DirEntryType < 0)	/* file */
			{
				d = ReadDefs(pathname,d,type);

			} else {
				/* set up pathname for additions */
				if (len && pathname[len-1] != ':' &&
				    pathname[len-1] != '/')
				{
					pathname[len] = '/';
					pathname[++len] = '\0';
				}

				/* examine everything in directory */
				while (ExNext(dirlock,fib)) {
				   if (fib->fib_DirEntryType < 0) /* file */
				   {
				      strcpy(&pathname[len],fib->fib_FileName);
				      d = ReadDefs(pathname,d,type);
				   }
				}
			}
		}
		UnLock(dirlock);
	}

	FreeMem((char *) fib, sizeof(*fib));
	return d;
}

/* read in all the defs in a file, if it is a drive definition file */

struct DriveDef *
ReadDefs (file,defs,type)
	char *file;
	struct DriveDef *defs;
	int type;
{
	register struct DiskDrive *dd;
	register struct DriveDef  *d;
	BPTR fh;
	register int done = FALSE;
	ULONG offset;

	if (fh = Open(file,MODE_OLDFILE))
	{
		while (!done) {
		  if (dd = AllocNew(DiskDrive))
		  {
		    offset = Seek(fh,0L,OFFSET_CURRENT);
		    if (sizeof(*dd) == Read(fh,(char *) dd,sizeof(*dd)))
		    {
		      if (dd->Ident == IDNAME_DRIVE)
		      {
			if (dd->ControllerType & type)
			{
			  if (d = AllocNew(DriveDef))
			  {
			    if (d->Initial_RDB = AllocNew(RigidDiskBlock))
			    {
			      /* finally! we have it */
			      *(d->Initial_RDB) = dd->rdb;
			      d->ds 		= dd->ds; /* struct copy */
			      d->Offset		= offset;
			      d->ControllerType = dd->ControllerType;
			      d->Reserved1	= dd->Reserved1;
			      d->Reserved2	= dd->Reserved2;
			      d->Succ = d->Prev = NULL;
			      strncpy(d->Filename,file,128);

			      CheckDefDates(&defs,d); /* also links into list */

			    } else {
			      done = TRUE;
			      FreeMem((char *) d, sizeof(*d));
			    }
			  } else
			    done = TRUE;
			} /* no else - if wrong type, continue */
		      } else
		 	done = TRUE;
		    } else
		      done = TRUE;

		    FreeMem((char *) dd, sizeof(*dd));
		  } else
		    done = TRUE;
		}
		Close(fh);
	}

	return defs;
}

/* check for duplicates, keep one with newest date */
/* also links in d to defs, can modify *defs       */

void
CheckDefDates (defs,d)
	struct DriveDef **defs;
	register struct DriveDef *d;
{
	register struct DriveDef *tmp;
	int done = FALSE;

	if (*defs)
	{
		for (tmp = *defs; tmp; tmp = tmp->Succ)
		{
			/* if there are duplicates, check datestamps */

			if (SameDrive(tmp->Initial_RDB,d->Initial_RDB))
			{
				if (d->ds.ds_Days   > tmp->ds.ds_Days   ||
				    d->ds.ds_Minute > tmp->ds.ds_Minute ||
				    d->ds.ds_Tick   > tmp->ds.ds_Tick)
				{
					/* d is newer, replace */
					d->Succ = tmp->Succ;
					d->Prev = tmp->Prev;
					if (d->Prev)
						d->Prev->Succ = d;
					if (d->Succ)
						d->Succ->Prev = d;
					if (*defs == tmp)
						*defs = d;

					FreeMem((char *) tmp->Initial_RDB,
						sizeof(struct RigidDiskBlock));
					FreeMem((char *) tmp, sizeof (*tmp));
				} else {
					/* tmp is newer, drop d */
					FreeMem((char *) d->Initial_RDB,
						sizeof(struct RigidDiskBlock));
					FreeMem((char *) d, sizeof (*d));
				}
				done = TRUE;
				break;
			}
		}
		if (!done)
		{
			/* no duplicates, so just link in at head */
			d->Prev = NULL;
			d->Succ = *defs;
			(*defs)->Prev = d;
			*defs = d;
		}
	} else
		*defs = d;	/* no other entries */
}

/* Find drive with same name, etc, or return NULL */

struct DriveDef *
FindDrive (list,man,prod,rev)
	struct DriveDef *list;
	char *man,*prod,*rev;
{
	register struct DriveDef *dd;

	for (dd = list; dd; dd = dd->Succ)
	{
		if (memcmp(man,dd->Initial_RDB->rdb_DiskVendor,8) == SAME &&
		    memcmp(prod,dd->Initial_RDB->rdb_DiskProduct,16) == SAME &&
		    memcmp(rev,dd->Initial_RDB->rdb_DiskRevision,4) == SAME)

			return(dd);
	}
	return NULL;
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
		return (int)cmdblk.scsi_Status;

	return (int)ior->io_Error;	/* see scsidisk.h for errors */
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







/* return the LASTLUN, LASTTID, or LAST flags */
/* if the driver handles one board, LASTTID | LASTLUN == LAST */

LONG CalcFlags(register struct Drive *d) {

register struct Drive *d2;
register LONG flags = 0;

  if (d->NextDrive == NULL) flags = RDBFF_LAST | RDBFF_LASTLUN | RDBFF_LASTTID;
  else if (strcmp(&(d->NextDrive->DeviceName[0]),&(d->DeviceName[0])) != SAME) {
    /* Last of this device driver - set all flags */
    flags = RDBFF_LAST | RDBFF_LASTLUN | RDBFF_LASTTID;
    }

  else if (d->NextDrive->Addr != d->Addr) {

    /* last at this address */
    flags = RDBFF_LASTLUN;
    if (d->NextDrive->Addr/100 != d->Addr/100) {
      /* next one is on another board */
      flags |= RDBFF_LASTTID;
      }
    }

  else {

    /* are all the rest on the same address and driver? */
    flags |= RDBFF_LASTTID;                             /* assume yes */
    for (d2=d->NextDrive; d2; d2=d2->NextDrive) {
      if (d2->Addr != d->Addr
        || strcmp(&(d2->DeviceName[0]), &(d->DeviceName[0])) != SAME) {
        /* no, there's one at a diff addr or driver */
        flags &= ~RDBFF_LASTTID;
        break;
        }
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

