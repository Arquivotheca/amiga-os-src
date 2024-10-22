/* verify.c - code for doing verification of data on the drive */

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
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include <clib/alib_protos.h>

#include "refresh.h"
#include "protos.h"

#include "global.h"

#include "scsidisk.h"
#include "scsi.h"

#define SAME	0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

/* verify the data on the selected drive, via the SCSI verify command. */

ULONG MaxBlock;	   /* Block to start with next */
ULONG TotalBlocks; /* # of blocks on the drive */
ULONG VerifySize;  /* # blocks to verify */
char  *XTBuffer;   /* memory buffer for XT */
ULONG NumErrors;   /* number of bad blocks found */
struct BadBlockBlock *bad;
ULONG LastBlock = 17*4*2;

#define Notify(w,x,y)  ErrNotify(x,y)

extern struct SysBase *SysBase;
extern struct DOSBase *DOSBase;

extern char *device;
extern int  unit;

int
VerifyRDSK ()
{
	int error = 0;
	char msg[80];
	struct RigidDiskBlock *rdb;
	LONG i;

	rdb = (struct RigidDiskBlock *)
		AllocMem(512,MEMF_CLEAR|MEMF_PUBLIC);
	if (!rdb)
		return 20;

	ReadRDSK((CPTR) rdb,512,0);
	if (rdb->rdb_ID != IDNAME_RIGIDDISK)
	{
		Notify(w,"No rigid disk block on drive!",0);
		FreeMem((char *) rdb,512);
		return 20;
	}

	NumErrors   = 0;
	LastBlock   = rdb->rdb_RDBBlocksHi + 1;

	TotalBlocks = ReadCapacity(0,0);

/*printf("Drive has %ld blocks\n",TotalBlocks);*/
	if (TotalBlocks == -1)
	{
		Notify(w,"Can't find size of drive!",0);
		return 20;
	}

	bad = (struct BadBlockBlock *) AllocMem(512,MEMF_CLEAR);
	if (!bad)
	{
		FreeMem((char *) rdb,512);
		return 20;
	}

	if ((LONG) rdb->rdb_BadBlockList != -1)
	{
		ReadRDSK((CPTR) bad,512,512 * (LONG) rdb->rdb_BadBlockList);
		if ((LONG) bad->bbb_Next != -1)
		{
			Notify(w,"Too many bad blocks - use HDToolkit",0);
			FreeMem((char *) rdb,512);
			FreeMem((char *) bad,512);
		}
		/* find lowest replacement */
		for (i = 0; i < (((bad->bbb_SummedLongs-6) * 4)/
				 sizeof(struct BadBlockEntry));
		     i++)
		{
			if (bad->bbb_BlockPairs[i].bbe_GoodBlock <
			    LastBlock)
			{
			    LastBlock = bad->bbb_BlockPairs[i].bbe_GoodBlock;
			}
		}
	} else {
		bad->bbb_ID          = IDNAME_BADBLOCK;
		bad->bbb_SummedLongs = 6;
		bad->bbb_HostID	     = 7; /* FIX! */
		bad->bbb_Next	     = (struct BadBlockBlock *) -1;
	}

	FreeMem((char *) rdb,512);

	if (strcmp(device,"xt.device") == 0)
	{
		return DoXTVerify();
	}

	/* Handle more than 65535 blocks */
	MaxBlock = 0;
	while (error == 0 && MaxBlock < TotalBlocks)
	{
		VerifySize = min(TotalBlocks - MaxBlock,0xffff);

		sprintf(msg,"Verifying blocks %ld to %ld, please wait",
			MaxBlock,MaxBlock+VerifySize-1);
		error = DoLongIO(msg,
				 VerifySendIO,
				 VerifyError);

		MaxBlock += VerifySize;
	}

	if (error)
	{
		if (error != 255 && error != 45) /* already reported... */
			Notify(w,"Had an I/O error during verify (%ld)",error);
		Notify(w,"Verify aborted",0);
	} else if (NumErrors == 0)
		Notify(w,"Drive reported no errors from verify command",0);
	else
		Notify(w,"Drive reported %ld bad blocks from verify command",
		       NumErrors);
	if (bad->bbb_SummedLongs != 6)
	{
		rdb = (struct RigidDiskBlock *)
			AllocMem(512,MEMF_CLEAR|MEMF_PUBLIC);
		if (!rdb)
			return 20;
		ReadRDSK((CPTR) rdb,512,0);
		if ((LONG) rdb->rdb_BadBlockList == -1)
			rdb->rdb_BadBlockList = (struct BadBlockBlock *)
						++(rdb->rdb_HighRDSKBlock);
		Notify(w,"Writing bad block table at block %ld",
		       (LONG) rdb->rdb_BadBlockList);

		CheckSumBlock(rdb);
		WriteRDSK((CPTR) rdb,512,0);
		CheckSumBlock((struct RigidDiskBlock *) bad);
		WriteRDSK((CPTR) bad,512,((LONG) rdb->rdb_BadBlockList)*512);
		FreeMem((char *) rdb,512);
	}
	FreeMem((char *) bad, 512);
			
	if (error)
		return 20;

	return 0;
}

int
DoXTVerify ()
{
	struct RigidDiskBlock *rdb;
	int error = 0;
	char msg[80];

	XTBuffer = AllocMem(17 * 512,MEMF_PUBLIC);
	if (!XTBuffer)
	{
		Notify(w,"Can't get enough memory!",NULL);
		return 20;
	}

	MaxBlock = 0;
	VerifySize = 17 * 4 * 10;
	while (error == 0 && MaxBlock < TotalBlocks)
	{
		sprintf(msg,"Verifying blocks %ld to %ld, please wait",
			     MaxBlock,min(MaxBlock+VerifySize-1,TotalBlocks-1));

		Notify(w,msg,0);
		error = DoLongIO(msg,
				 VerifyXTSendIO,
				 VerifyError);

		MaxBlock += VerifySize;
	}
	FreeMem(XTBuffer,17 * 512);

	if (error)
	{
		if (error != 255 && error != 45) /* already reported... */
			Notify(w,"Had an I/O error during verify (%ld)",error);
		Notify(w,"Verify aborted",0);
	} else if (NumErrors == 0)
		Notify(w,"Drive reported no errors from verify command",0);
	else
		Notify(w,"Drive reported %ld bad blocks from verify command",
		       NumErrors);

	if (bad->bbb_SummedLongs != 6)
	{
		rdb = (struct RigidDiskBlock *)
			AllocMem(512,MEMF_CLEAR|MEMF_PUBLIC);
		if (!rdb) {
			FreeMem((char *) bad,512);
			return 20;
		}
		ReadRDSK((CPTR) rdb,512,0);
		if ((LONG) rdb->rdb_BadBlockList == -1)
			rdb->rdb_BadBlockList = (struct BadBlockBlock *)
						++(rdb->rdb_HighRDSKBlock);
		Notify(w,"Writing bad block table at block %ld",
		       (LONG) rdb->rdb_BadBlockList);

		CheckSumBlock(rdb);
		WriteRDSK((CPTR) rdb,512,0);
		CheckSumBlock((struct RigidDiskBlock *) bad);
		WriteRDSK((CPTR) bad,512,((LONG)rdb->rdb_BadBlockList)*512);
		FreeMem((char *) rdb,512);
	}
	FreeMem((char *) bad, 512);
	if (error)
		return 20;

	return 0;
}

int
VerifySendIO (ior)
	struct IOStdReq *ior;
{
	static char verify[10];

/*printf("Verifying from block %ld for %ld blocks\n",MaxBlock,VerifySize);*/
	verify[0] = S_VERIFY;
	verify[1] = 0;	/* no data */
	verify[2] = (MaxBlock & 0xff000000) >> 24;
	verify[3] = (MaxBlock & 0x00ff0000) >> 16;
	verify[4] = (MaxBlock & 0x0000ff00) >> 8;
	verify[5] = (MaxBlock & 0x000000ff);
	verify[6] = 0;
	verify[7] = (VerifySize & 0x0000ff00) >> 8;
	verify[8] = (VerifySize & 0x000000ff);
	verify[9] = 0;

	SendSCSI(ior,(UWORD *) verify, 10, NULL, 0,SCSIF_WRITE|SCSIF_AUTOSENSE);

	return TRUE;
}

/* slow, but it will work */

int
VerifyXTSendIO (ior)
	register struct IOStdReq *ior;
{
	static char read[6];
	register LONG i,block = MaxBlock;

	i = 0;
	while (block < TotalBlocks && i < VerifySize) {
		read[0] = S_READ;
		read[1] = (block & 0x001f0000) >> 16;	/* note 1f */
		read[2] = (block & 0x0000ff00) >> 8;
		read[3] = (block & 0x000000ff);
		read[4] = 17;
		read[5] = 0;

		DoSCSI (ior,(UWORD *) read, 6, (UWORD *) XTBuffer,
			17 * 512,
			SCSIF_READ|SCSIF_AUTOSENSE);

		if (ior->io_Error)
			break;

		block += 17;
		i += 17;
	}

	if (ior->io_Error)
	{
		/* read it again to have DoLongIO pass the error on */
		/* hopefully, we'll get the same error, if we got one */
		extern int slowdown;

		if (slowdown)
			Delay(20);

		read[0] = S_READ;
		read[1] = (block & 0x001f0000) >> 16;	/* note 1f */
		read[2] = (block & 0x0000ff00) >> 8;
		read[3] = (block & 0x000000ff);
		read[4] = 17;
		read[5] = 0;

		SendSCSI (ior,(UWORD *) read, 6, (UWORD *) XTBuffer,
			17 * 512,
			SCSIF_READ|SCSIF_AUTOSENSE);

		return TRUE;
	}

	return FALSE;
}

/* may be called from VerifyError() */

int
VerifyXTError (ior,block)
	struct IOStdReq *ior;
	LONG block;
{
	register LONG cyl,head,sec,i,j;

	NumErrors++;

	/* first get sector on cylinder */
	cyl = block / (17*4);
	sec = block % (17*4);

	/* now make that sector on head */
	head = sec / (17);
	sec  = sec % (17);

	Notify(w,"Mapping out Block %ld",block);
	Notify(w,"Cylinder %ld",cyl);
	Notify(w,"head %ld",head);
	Notify(w,"sector %ld",sec);

	/* map it out */

	/* first try SCSI mapping */
	if (stricmp("scsi.device",device) == 0)
	{
		if (ReassignBlock(ior,block))
		{
			/* increase the number listed */
			Notify(w,"Mapped out using ReassignBlock...",0);

			ior->io_Error = 0;
			return 0;
		} /* else use our mapping */
	}

	/* use driver mapping */
	i = ((bad->bbb_SummedLongs - 6) * 4) /
	    sizeof(struct BadBlockEntry);
	if (i >= 61)
	{
		Notify(w,"Too many bad blocks!",0);
		return 255;
	}
	for (j = 0; j < i; j++)
	{
		if (bad->bbb_BlockPairs[j].bbe_BadBlock == block)
		{
			Notify(w,"Block already mapped to block %ld",
			       bad->bbb_BlockPairs[i].bbe_GoodBlock);
			ior->io_Error = 0;
			return 0;
		}
	}

	bad->bbb_BlockPairs[i].bbe_BadBlock  = block;
	bad->bbb_BlockPairs[i].bbe_GoodBlock = --LastBlock;
	bad->bbb_SummedLongs += sizeof(struct BadBlockEntry) >> 2;
	Notify(w,"Replaced with block %ld",LastBlock);

	ior->io_Error = 0;
	return 0;
}


int
VerifyError (ior)
	struct IOStdReq *ior;
{
	LONG block = -1;
	int  code  = -1;
	int  key;
	int  len;
	extern struct SCSICmd cmdblk;
	extern UBYTE sensedata[];

/*("In error routine\n");*/

	if (cmdblk.scsi_Status == 0)
		return 0;
/*Notify(w,"SCSI return status %d\n",cmdblk.scsi_Status);*/

	len = cmdblk.scsi_SenseActual;

/*
Notify(w,"len = %d\n",len);
{
char x[80];
for (key = 0; key < len; key++)
sprintf(x[key*5],"0x%2x ",sensedata[key]);
Notify(w,x,0);
}
*/
	if (len >= 3 && (sensedata[0] & 0x7E) == 0x70)	/* 0x70 or 0x71 */
	{
		/* extended sense data exists */
		if (len >= 7 && sensedata[0] & 0x80)
		{
			/* block is valid */
			block = (sensedata[3] << 24) |
				(sensedata[4] << 16) |
				(sensedata[5] << 8)  |
				(sensedata[6]);
		}

		key = sensedata[2] & 0x0F;

		if (len >= 13)
			code = sensedata[12];

		if (key == MEDIUM_ERROR)
		{
			if (block != -1)
			{
				if (VerifyXTError(ior,block) == 255)/* stop */
					return 255;
			} else {
				NumErrors++;
				Notify(w,"Medium error at unknown location!",0);
				Notify(w,
			 "You should back up the drive and low-level format",0);
			}
		} else if (key == HARDWARE_ERROR) {

			Notify(w,"Hardware error %ld!",code);

		} else if (key == RECOVERED_ERROR) {

			Notify(w,"Recovered read error at block %ld", block);
			if (block != -1)
			{
				if (VerifyXTError(ior,block) == 255)/* stop */
					return 255;
			} else {
				NumErrors++;
				Notify(w,
				   "Recoverable error at unknown location!",0);
				Notify(w,
		 "You should consider backing up the drive and reformatting",0);
			}
		} else if (key == ILLEGAL_REQUEST) {

			Notify(w,
			       "Drive returned an Illegal Request Error (%ld)",
			       code);

		} else if (code && code != -1) {

			Notify(w,"Unknown error: sense key %ld (more)", key);
			Notify(w,"Device returned sense code %ld (more)", code);
			Notify(w,"Consult drive documentation", 0);
		}

		/* continue with verify.... */
		/* note that DoVerify increments maxblock by verifysize... */
		if (block != -1 && ior->io_Error == 0)
		{
			Notify(w,"Continuing verify at block %d...",block+1);
			MaxBlock = (block+1) - VerifySize;
		}
		return 0;
	}

	/* non-extended sense */
	if (len == 4)
	{
		Notify(w,"Sense returned 0x%lx",*((LONG *)sensedata));
		if (sensedata[0] & 0x80)
		{
			/* block is valid */
			block = ((sensedata[1] & 0x1f) << 16)	|
				(sensedata[2] << 8)		|
				(sensedata[3]);
		}

		key = sensedata[0] & 0x7F;

		switch (key) {
		case 0x15:	/* seek error */
		case 02:	/* incomplete seek */
			Notify(w,"Seek error!",0);
			break;
		case 01:	/* no index */
		case 03:	/* write fault */
		case 0x11:	/* uncorrectable error */
		case 0x12:	/* missing ID address mark */
		case 0x14:	/* record not found */
		case 0x19:	/* ecc error during verify */
			/* all R/W errors - map out */
			if (block != -1)
			{
				if (VerifyXTError(ior,block) == 255)/* stop */
					return 255;
			} else {
				NumErrors++;
				Notify(w,"Medium error at unknown location!",0);
				Notify(w,
			 "You should back up the drive and low-level format",0);
			}
			break;
	
		case 0x18:	/* recovered error */
			Notify(w,"Recovered error at block %ld", block);
			if (block != -1)
			{
				if (VerifyXTError(ior,block) == 255)/* stop */
					return 255;
			} else {
				NumErrors++;
				Notify(w,
				   "Recoverable error at unknown location!",0);
				Notify(w,
		 "You should consider backing up the drive and reformatting",0);
			}
			break;

		case 0x20:	/* illegal command */
			Notify(w,"Illegal command!",0);
			break;

		case 0x21:	/* illegal address */
			Notify(w,"Illegal address %ld!",block);
			break;

		case 0x24:	/* bad arguement */
			Notify(w,"Bad arguement!",0);
			break;

		case 0x28:	/* cartridge changed */
			Notify(w,"Cartridge changed",0);
			break;

		case 0x25:	/* invalid LUN */
			Notify(w,"Invalid LUN addressed!",0);
			break;

		case 0x1c:
			Notify(w,"Unformatted drive!",0);
			break;
	
		case 0:		/* no error */
			ior->io_Error = 0;
			break;

		default:
			Notify(w,"Non-extended error is 0x%x",key);
		}
		/* continue with verify.... */
		/* note that DoVerify increments maxblock by verifysize... */
		if (block != -1 && ior->io_Error == 0)
		{
			Notify(w,"Continuing verify at block %d...",block+1);
			MaxBlock = (block+1) - VerifySize;
		}
		return 0;
	}

	Notify(w,"Device didn't return good sense data!",0);
	return 255;
}

/* sendio return true if ok, false if should abort */
/* if ok, assume ior has been SendIO()ed, and wait */
/* errrtn is called to handle errors, and is	   */
/* passed the ior.  errrtn may be NULL.		   */
/* returns ior->io_Error or -5			   */

int
DoLongIO (waittext,sendio,errrtn)
	char *waittext;
	int (*sendio)(struct IOStdReq *);
	int (*errrtn)(struct IOStdReq *);
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port;
	int  opened = FALSE;
	int  i;
	int  error = -5;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	/* open the unit */
	if (i = OpenDevice(device,unit,
			   (struct IORequest *) ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}
	opened = TRUE;


	error = 0;

	if ((*sendio)(ior) == FALSE)
		goto cleanup;

	/* asynch allows us to handle refresh, put up requesters */
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

	/* call the error handling routine - important for SCSI */
	if (errrtn)
		error = (*errrtn)(ior);

/* FIX! HFERR_BadStatus! */
	if (error && error != 45 && error != 255)
		Notify(w,"Driver returned I/O error code %ld",(LONG) error);

	/* make sure we return error if io_error */
	if (!error && ior->io_Error && error != 255)
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

/* returns -1 if error */

LONG
ReadCapacity (pmi,block)
	int pmi;
	ULONG block;
{
	register struct IOStdReq *ior  = NULL;
	struct MsgPort  	 *port;
	unsigned char inquiry[10];
	unsigned char data[8];
	ULONG blocks = -1;
	int  opened  = FALSE;
	int i;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(device,unit,
			   (struct IORequest *) ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}
	opened = TRUE;

	/* now try to find out what size the drive is */
	inquiry[0] = S_READ_CAPACITY;
	inquiry[1] = 0;
	inquiry[2] = (block & 0xff000000) >> 24;
	inquiry[3] = (block & 0x00ff0000) >> 16;
	inquiry[4] = (block & 0x0000ff00) >> 8;
	inquiry[5] = (block & 0x000000ff);
	inquiry[6] =
	inquiry[7] = 0;
	inquiry[8] = pmi;
	inquiry[9] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,10,(UWORD *) data,8,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		blocks = ((data[0] << 24) | (data[1] << 16) |
			  (data[2] <<  8) | (data[3])) + 1;
	}

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	return (long)blocks;
}

void
ErrNotify (str,i)
	char *str;
	long i;
{
	printf(str,i);
	printf("\n");
}

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
	for (i = 0; i < rdb->rdb_SummedLongs; i++)
	{
		checksum += larray[i];
	}

	rdb->rdb_ChkSum = 0 - checksum;	  /* I prefer EOR checksums, or CRC */
}

int
ReassignBlock (ior,block)
	struct IOStdReq *ior;
	ULONG block;
{
	struct IOStdReq *myior;
	unsigned char reassign[6];
	unsigned char data[8];

	if (!(myior = CreateStdIO(ior->io_Message.mn_ReplyPort)))
		return FALSE;

	/* copy the old ior - should be safe */
	*myior = *ior;

	/* now try a reassign block command */
	reassign[0] = S_REASSIGN_BLOCKS;
	reassign[1] = 0;
	reassign[2] = 0;
	reassign[3] = 0;
	reassign[4] = 0;
	reassign[5] = 0;

	data[0] = 0;
	data[1] = 0;
	data[2] = 0; /* high byte of defect length */
	data[3] = 4; /* low byte */
	data[4] = (block & 0xff000000) >> 24;
	data[5] = (block & 0x00ff0000) >> 16;
	data[6] = (block & 0x0000ff00) >> 8;
	data[7] = (block & 0x000000ff);

	if (!DoSCSI(myior,(UWORD *) reassign,6,(UWORD *) data,8,
		    SCSIF_WRITE|SCSIF_AUTOSENSE))
	{
		DeleteStdIO(myior);
		return TRUE;
	}

	/* ran out of space, or doesn't support it */
	DeleteStdIO(myior);
	return FALSE;
}
