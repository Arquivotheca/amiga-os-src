/* readwrite.c - code to handle the prep main screen */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/errors.h>
#include <devices/trackdisk.h>
#include <devices/scsidisk.h>
#include <devices/hardblocks.h>
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
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "scsi.h"

#include "FindNotFormatted_protos.h"

#define SAME 0

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

extern struct Library *SysBase;

void
main (argc,argv)
{
	FindUnformattedDrive("scsi.device");
	exit(0);
}

/* get all drives in the system */

int
FindUnformattedDrive (name)
	char *name;			/* of device */
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port = NULL;
	int res = FALSE,done = FALSE;
	unsigned short addr,lun,board;
	char str[80];
	LONG error;
	int numaddr = 8, numlun = 8;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	/* handle steve's wierd board multiple handlers */
	for (board = 0; board < 8 && !done; board++)
	{
	    /* for each addr/lun pair */
	    for (addr = 0; addr < numaddr && !done; addr++)
	    {
		for (lun = 0; lun < numlun && !done; lun++)
		{
//			printf("Checking %s%s address %d unit %d...\n",
//				devnames[board],name,addr,lun);
			sprintf(str,"%s%s",devnames[board],name);
			if (!(error = OpenDevice(str,addr + lun * 10,
				         (struct IORequest *) ior, 0L)))
			{
				if (!HasRDB(ior) &&
				    IsDrive(ior))
				{
					done = 1;
					printf("%ld %s\n",addr + lun*10,
						str);
				}

				CloseDevice((struct IORequest *) ior);
			} else {
				/* if device doesn't exist, stop */
				if (error == IOERR_OPENFAIL)
					done = TRUE;
			} /* opendevice */
		} /* for lun */
	    } /* for addr */
	} /* for board */

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
int
IsDrive (ior)
	struct IOStdReq *ior;
{
	char inquiry[6],data[36];

	inquiry[0] = S_INQUIRY;
	inquiry[1] =
	inquiry[2] =
	inquiry[3] = 0;
	inquiry[4] = 36;
	inquiry[5] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,6,(UWORD *) data,36,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		if (data[0] == 0x7f)
			return FALSE;
		switch (data[0] & 0x1f) {
		case 0:
		case 7:
			return TRUE;
		default:
			return FALSE;
		}
	}
	return FALSE;
}


int
HasRDB (ior)
	struct IOStdReq *ior;
{
	ULONG size = 512;	/* default and only for st506 */
	struct RigidDiskBlock *rdb;
	ULONG block;
	short i;

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

	if (!(rdb = AllocMem(size,MEMF_PUBLIC|MEMF_CLEAR)))
 		return NULL;

	/* first find rigid disk block, if it exists */
	for (block = 0; block < RDB_LOCATION_LIMIT; block++)
	{
		/* if read fails, abort immediately */
		if (!ReadBlock(ior,(APTR) rdb,size,block))
		{
			FreeMem(rdb, size);
			return NULL;
		}
		/* make sure it's a rigiddiskblock */
		if (rdb->rdb_ID == IDNAME_RIGIDDISK &&
		    CheckCheckSum((struct RigidDiskBlock *) rdb))
		{
			FreeMem(rdb, size);
			return TRUE;
		}
	}
	FreeMem(rdb, size);
	return FALSE;
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

	SendIO((struct IORequest *) ior);
}



