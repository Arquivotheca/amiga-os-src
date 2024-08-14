/* verify.c - code for doing verification of data on the drive */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/filehandler.h>

#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "refresh.h"
#include "protos.h"
#include <clib/alib_protos.h>

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"
#include "scsidisk.h"
#include "scsi.h"

char *TakesLongTime[] = {
	"This operation can take a long time.",
	"Are you certain you wish to continue?",
	" ",
	"(No data on the drive will be changed)",
	NULL,
};

/* verify the data on the selected drive, via the SCSI verify command. */

ULONG MaxBlock;	   /* Block to start with next */
ULONG TotalBlocks; /* # of blocks on the drive */
ULONG VerifySize;  /* # blocks to verify */
char  *XTBuffer;   /* memory buffer for XT */
struct Drive *global_d;
ULONG NumErrors;   /* number of bad blocks found */


void
DoVerify (w)
	struct Window *w;
{
	int error = 0;
	char msg[80];

	if (!AskSure(w,&TakesLongTime[0]))
		return;

	NumErrors   = 0;
	TotalBlocks = ReadCapacity(w,SelectedDrive,0,0);
	if (TotalBlocks == -1)
	{
		Notify(w,"Can't find size of drive!",0);
		return;
	}

	/* for error routine */
	global_d = SelectedDrive;

	if (SelectedDrive->Flags & ST506)
	{
		DoXTVerify(w,SelectedDrive);
		return;
	}

	/* Handle more than 65535 blocks */
	MaxBlock = 0;
	while (error == 0 && MaxBlock < TotalBlocks)
	{
		VerifySize = min(TotalBlocks - MaxBlock,0xffff);

		sprintf(msg,"Verifying blocks %ld to %ld, please wait",
			MaxBlock,MaxBlock+VerifySize-1);
		error = DoLongIO(w,SelectedDrive,msg,
//				 VerifySendIO,&IntuiTextList1,prep_draw,
				 VerifySendIO,NULL,redraw_drivelist,		//
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
}

void
DoXTVerify (w,d)
	struct Window *w;
	struct Drive  *d;
{
	int error = 0;
	char msg[80];

	if (!d->rdb->rdb_BlockBytes)
	{
		/* ICK! */
		return;
	}

	XTBuffer = AllocMem(17 * d->rdb->rdb_BlockBytes,MEMF_PUBLIC);
	if (!XTBuffer)
	{
		Notify(w,"Can't get enough memory!",NULL);
		return;
	}

	MaxBlock = 0;
	VerifySize = d->rdb->rdb_CylBlocks * 10;
	while (error == 0 && MaxBlock < TotalBlocks)
	{
		sprintf(msg,"Verifying blocks %ld to %ld, please wait",
			     MaxBlock,min(MaxBlock+VerifySize-1,TotalBlocks-1));

		error = DoLongIO(w,SelectedDrive,msg,
//				 VerifyXTSendIO,&IntuiTextList1,prep_draw,
				 VerifyXTSendIO,NULL,redraw_drivelist,		//
				 VerifyError);

		MaxBlock += VerifySize;
	}
	FreeMem(XTBuffer,17 * d->rdb->rdb_BlockBytes);

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
}

int
VerifySendIO (w,d,ior)
	struct Window *w;
	register struct Drive *d;
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
VerifyXTSendIO (w,d,ior)
	struct Window *w;
	register struct Drive *d;
	register struct IOStdReq *ior;
{
	static char read[6];
	register struct IntuiMessage *msg;
	register LONG i,block = MaxBlock;


	read[0] = S_READ;
	read[4] = 17;
	read[5] = 0;

	Request(&PleaseWait_Requester,w);

	i = 0;
	while (block < TotalBlocks && i < VerifySize) {
		read[1] = (block & 0x001f0000) >> 16;	/* note 1f */
		read[2] = (block & 0x0000ff00) >> 8;
		read[3] = (block & 0x000000ff);

		DoSCSI (ior,(UWORD *) read, 6, (UWORD *) XTBuffer,
			17 * global_d->rdb->rdb_BlockBytes,
			SCSIF_READ|SCSIF_AUTOSENSE);

		if (ior->io_Error)
			break;

		block += 17;
		i += 17;

		/* handle refresh */
		while (msg = (struct IntuiMessage *) GetMsg(w->UserPort))
		{
			/* eat only messsages already piled up */
			if (msg->Class == REFRESHWINDOW)
			{
				BeginRefresh(w);
//				do_refresh(w,&IntuiTextList1,prep_draw);
				do_refresh(w,NULL,redraw_drivelist);	//
				EndRefresh(w,TRUE);
			}
			ReplyMsg((struct Message *) msg);
		}

	}

	EndRequest(&PleaseWait_Requester,w);

	/* Eat the REQCLEAR message */
	while (1) {
		WaitPort(w->UserPort);
		msg = (struct IntuiMessage *) GetMsg(w->UserPort);
		if (msg->Class == REQCLEAR)
		{
			ReplyMsg((struct Message *) msg);
			break;
		}
		ReplyMsg((struct Message *) msg);
	}

	if (ior->io_Error)
	{
		/* read it again to have DoLongIO pass the error on */
		/* hopefully, we'll get the same error, if we got one */

		read[1] = (block & 0x001f0000) >> 16;	/* note 1f */
		read[2] = (block & 0x0000ff00) >> 8;
		read[3] = (block & 0x000000ff);

		SendSCSI (ior,(UWORD *) read, 6, (UWORD *) XTBuffer,
			17 * global_d->rdb->rdb_BlockBytes,
			SCSIF_READ|SCSIF_AUTOSENSE);

		return TRUE;
	}

	return FALSE;
}

int map_out;

int
MapOutHandler (w,req,msg)
	struct Window *w;
	struct Requester *req;
	struct IntuiMessage *msg;
{
	int done = 0;

	if (msg->Class == GADGETUP)
		if (msg->IAddress == (APTR)&MapOut)
                        {
			map_out = -1;
			done = HANDLE_DONE;
		} else if (msg->IAddress == (APTR) &MapIgnore) {
			map_out = 1;
			done = HANDLE_CANCEL;
		} else if (msg->IAddress == (APTR) &MapStop) {
			map_out = 0;
			done = HANDLE_CANCEL;
		}
			; /* Huh? */

	return done;
}

/* may be called from VerifyError() */

int
VerifyXTError (w,ior,block)
	struct Window *w;
	struct IOStdReq *ior;
	LONG block;
{
	char read[6];
	char *data;
	register struct Bad *b;
	register LONG cyl,head,sec;

	NumErrors++;

	if (isbad(global_d,block))
	{
		/* block already mapped as bad - notify user */
		Notify(w,"Old bad block found, already mapped out (block %ld)",
		       block);

		/* ignore the error */
		ior->io_Error = 0;
		return 0;
	}

	/* first get sector on cylinder */
	cyl = block / global_d->rdb->rdb_CylBlocks;
	sec = block % global_d->rdb->rdb_CylBlocks;

	/* now make that sector on head */
	/* this use of sectors is OK    */
	head = sec / global_d->rdb->rdb_Sectors;
	sec  = sec % global_d->rdb->rdb_Sectors;

	/* GODDAMN XT drives! (well, maybe a SCSI might have interleave too) */
	/* handle interleaves before displaying - translate to interleave 1  */
	if (global_d->rdb->rdb_Interleave > 1)
	{
		ULONG *map;
		LONG temp;

		/* this use of sectors is OK */
		map = BuildInterleaveMap(global_d->rdb->rdb_Interleave,
					 global_d->rdb->rdb_Sectors);
		if (!map)
		{
			/* out of mem, ugh */
			ior->io_Error = 0;
			return 0;
		}

		temp  = sec;
		sec   = FindSector(map,sec);
		block = block - temp + sec;	/* -temp for beginning of trk */

		FreeMem((char *) map, global_d->rdb->rdb_Sectors * 4);
	}

	sprintf(VerifyCyl.IText,"%ld",cyl);
	sprintf(VerifyHead.IText,"%ld",head);
	sprintf(VerifySec.IText,"%ld",sec);
	sprintf(VerifyBlock.IText,"%ld",block);

	map_out = 0;

	if (block <= global_d->rdb->rdb_RDBBlocksHi)
	{
		Notify(w,"Error in rigid disk block area (block %ld)!!!",
		       block);
		Notify(w,"Try low-level formatting the disk!",NULL);
		return 255;
	}

//	Make border for 1.3 type Requester
	MakePrettyRequestBorder(MapOutRequester.Width,MapOutRequester.Height);

	HandleRequest(w,&MapOutRequester,MapOutHandler);

	if (map_out == 1)	/* ignore */
	{
		ior->io_Error = 0;
		return 0;

	} else if (map_out == -1) {
		/* map it out */

		/* first try SCSI mapping */
		if (global_d->Flags & SCSI)
		{
			if (ReassignBlock(ior,block))
			{
				/* increase the number listed */
				global_d->SCSIBadBlocks++;

				ior->io_Error = 0;
				return 0;
			} /* else use our mapping */
		}

		/* use our driver's mapping */
		if (BadSetup(w))	/* works on SelectedDrive! */
		{
			b = AllocNew(Bad);
			if (b)
			{
				b->Next  = NULL;
				b->flags = BADSECTOR;
				b->block = block;
				b->cyl   = cyl;
				b->head  = head;
				b->sector = sec;

				/* FirstBad set by BadSetup */
				b->replacement = GetReplacement(global_d->rdb,
								FirstBad);
				if (!b->replacement)
				{
					FreeMem((char *) b,sizeof(*b));
				} else
					InsertBad(NULL,b);
			}
			FreeBadBlockList(global_d->rdb->rdb_BadBlockList);
			global_d->rdb->rdb_BadBlockList =
						     MakeBadBlockList(FirstBad);
			if (global_d->bad)
				FreeBad(global_d->bad);
			global_d->bad    = FirstBad;
			global_d->Flags |= UPDATE;

			/* now try to recover data */
			data = AllocMem(global_d->rdb->rdb_BlockBytes,
					MEMF_CLEAR|MEMF_PUBLIC);
			if (data)
			{
				/* ignore error returns */
				read[0] = S_READ;
				read[1] = (block & 0x001f0000) >> 16;
				read[2] = (block & 0x0000ff00) >> 8;
				read[3] = (block & 0x000000ff);
				read[4] = 1;
				read[5] = 0;

				DoSCSI (ior,(UWORD *) read, 6, (UWORD *) data,
					global_d->rdb->rdb_BlockBytes,
					SCSIF_READ|SCSIF_AUTOSENSE);

				read[0] = S_WRITE;
				read[1] = (b->replacement & 0x001f0000) >> 16;
				read[2] = (b->replacement & 0x0000ff00) >> 8;
				read[3] = (b->replacement & 0x000000ff);
				DoSCSI (ior,(UWORD *) read, 6, (UWORD *) data,
					global_d->rdb->rdb_BlockBytes,
					SCSIF_WRITE|SCSIF_AUTOSENSE);

				FreeMem(data,global_d->rdb->rdb_BlockBytes);
			}
		}
		ior->io_Error = 0;
		return 0;
	}

	/* stop */
	return 255;
}

int
VerifyError (w,ior)
	struct Window *w;
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
				if (VerifyXTError(w,ior,block) == 255)/* stop */
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
				if (VerifyXTError(w,ior,block) == 255)/* stop */
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
				if (VerifyXTError(w,ior,block) == 255)/* stop */
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
				if (VerifyXTError(w,ior,block) == 255)/* stop */
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
DoLongIO (w,d,waittext,sendio,itext,draw,errrtn)
	struct Window *w;
	struct Drive *d;
	char *waittext;
	int (*sendio)(struct Window *,struct Drive *,struct IOStdReq *);
	struct IntuiText *itext;
	void (*draw)(struct RastPort *);
	int (*errrtn)(struct Window *,struct IOStdReq *);
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
	if (i = OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
			   (struct IORequest *) ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}
	opened = TRUE;

	/* ask them to wait - center the requester */
	PleaseWait_Requester.ReqText->IText    = waittext;
	PleaseWait_Requester.ReqText->LeftEdge = 20;

	PleaseWait_Requester.Width =
			IntuiTextLength(PleaseWait_Requester.ReqText) + 40;
	PleaseWait_Requester.LeftEdge =
			max(0,320 - PleaseWait_Requester.Width/2);

//	Make border for 1.3 type Requester
	MakePrettyRequestBorder(PleaseWait_Requester.Width,
					PleaseWait_Requester.Height);
	error = 0;

	if ((*sendio)(w,d,ior) == FALSE)
		goto cleanup;

	Request(&PleaseWait_Requester,w);

	/* asynch allows us to handle refresh, put up requesters */
	for (;;)
	{
/*printf("Top of request loop\n");*/
		/* wait for reply, handle messages */
		Wait ((1L << w->UserPort->mp_SigBit) |
		      (1L << port->mp_SigBit));

		if (CheckIO((struct IORequest *) ior))
			break;

		while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
		{
			if (msg->Class == REFRESHWINDOW)
			{
				BeginRefresh(w);

				do_refresh(w,itext,draw);

				EndRefresh(w,TRUE);
			}
			ReplyMsg((struct Message *) msg);
		}
	}
	WaitIO((struct IORequest *) ior);
/*printf("IO done\n");*/

	/* kill the requester */
	EndRequest(&PleaseWait_Requester,w);

	/* Wait for and eat the REQCLEAR message */
	while (1) {
		WaitPort(w->UserPort);
		msg = (struct IntuiMessage *) GetMsg(w->UserPort);
		if (msg->Class == REQCLEAR)
		{
			ReplyMsg((struct Message *) msg);
			break;
		}
		ReplyMsg((struct Message *) msg);
	}

	/* more funky magic: to make simple refresh windows look nice,      */
	/* search for a REFRESHWINDOW message and eat it before continuing  */

	while (msg = (struct IntuiMessage *) GetMsg(w->UserPort))
	{
		/* eat only messsages already piled up */
		if (msg->Class == REFRESHWINDOW)
		{
			BeginRefresh(w);
			do_refresh(w,itext,draw);
			EndRefresh(w,TRUE);
			ReplyMsg((struct Message *) msg);
			break;	/* got it - exit */
		}
		ReplyMsg((struct Message *) msg);
	}

	error = ior->io_Error;

	/* call the error handling routine - important for SCSI */
	if (errrtn)
		error = (*errrtn)(w,ior);

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
ReadCapacity (w,d,pmi,block)
	struct Window *w;
	struct Drive *d;
	int pmi;
	ULONG block;
{
	register struct IOStdReq *ior  = NULL;
	struct MsgPort  	 *port = NULL;
	unsigned char inquiry[10];
	unsigned char data[8];
	ULONG blocks = -1;
	int  opened  = FALSE;
	int i;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
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

/* 39.16 */
LONG
ReadSectorSize (w,d)
	struct Window *w;
	struct Drive *d;
{
	register struct IOStdReq *ior  = NULL;
	struct MsgPort  	 *port = NULL;
	unsigned char inquiry[10];
	unsigned char data[8];
	LONG sectorsize = -1;
	int  opened  = FALSE;
	int i;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(d->DeviceName,d->Addr + d->Lun * 10,
			   (struct IORequest *) ior,0L))
	{
		Notify(w,"Error %d on device open!",i);
		goto cleanup;
	}
	opened = TRUE;

	/* now try to find out what size the drive is */
	inquiry[0] = S_READ_CAPACITY;
	inquiry[1] =
	inquiry[2] =
	inquiry[3] =
	inquiry[4] =
	inquiry[5] =
	inquiry[6] =
	inquiry[7] =
	inquiry[8] = 		/* pmi = 0 */
	inquiry[9] = 0;

	if (!DoSCSI(ior,(UWORD *) inquiry,10,(UWORD *) data,8,
		    SCSIF_READ|SCSIF_AUTOSENSE))
	{
		sectorsize = ((data[4] << 24) | (data[4] << 16) |
			      (data[6] <<  8) | (data[7]));
	}

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);

	return sectorsize;
}

int ReassignBlock(struct IOStdReq *ior, ULONG block)
{
	struct IOStdReq *myior;
	unsigned char cmd[10];
	unsigned char data[512];
	UBYTE *old_data;
	int cmdlen = (block <= 0x1fffff ? 6 : 10);

	if (!(myior = CreateStdIO(ior->io_Message.mn_ReplyPort))) return FALSE;

	if (!(old_data = AllocMem(SelectedDrive->rdb->rdb_BlockBytes,MEMF_CLEAR)))
	{
		DeleteStdIO(myior);
		return FALSE;
	}

	/* copy the old ior - should be safe */
	*myior = *ior;

	/* first read the old data, if possible.  May be garbage or give an error. */
	if (block <= 0x1fffff)
	{
		cmd[0] = S_READ;
		cmd[1] = (block & 0x001f0000) >> 16;
		cmd[2] = (block & 0x0000ff00) >> 8;
		cmd[3] = (block & 0x000000ff);
		cmd[4] = 1;
		cmd[5] = 0;
	} else {
		cmd[0] = S_READ10;
		cmd[1] = 0;
		cmd[2] = (block & 0xff000000) >> 24;
		cmd[3] = (block & 0x00ff0000) >> 16;
		cmd[4] = (block & 0x0000ff00) >> 8;
		cmd[5] = (block & 0x000000ff);
		cmd[6] = 0;
		cmd[7] = 0;
		cmd[8] = 1;
		cmd[9] = 0;
	}

	/* we don't care if this fails... */
	DoSCSI(myior, (UWORD *)cmd, cmdlen, (UWORD *)old_data,
		global_d->rdb->rdb_BlockBytes, SCSIF_READ|SCSIF_AUTOSENSE);

	/* now try a reassign block command */
	cmd[0] = S_REASSIGN_BLOCKS;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;
	cmd[4] = 0;
	cmd[5] = 0;

	data[0] = 0;
	data[1] = 0;
	data[2] = 0; /* high byte of defect length */
	data[3] = 4; /* low byte */
	data[4] = (block & 0xff000000) >> 24;
	data[5] = (block & 0x00ff0000) >> 16;
	data[6] = (block & 0x0000ff00) >> 8;
	data[7] = (block & 0x000000ff);

	if (!DoSCSI(myior, (UWORD *)cmd, 6, (UWORD *)data,
		8, SCSIF_WRITE|SCSIF_AUTOSENSE))
	{
		if (block <= 0x1fffff)
		{
			cmd[0] = S_WRITE;
			cmd[1] = (block & 0x001f0000) >> 16;
			cmd[2] = (block & 0x0000ff00) >> 8;
			cmd[3] = (block & 0x000000ff);
			cmd[4] = 1;
			cmd[5] = 0;
		} else {
			cmd[0] = S_WRITE10;
			cmd[1] = 0;
			cmd[2] = (block & 0xff000000) >> 24;
			cmd[3] = (block & 0x00ff0000) >> 16;
			cmd[4] = (block & 0x0000ff00) >> 8;
			cmd[5] = (block & 0x000000ff);
			cmd[6] = 0;
			cmd[7] = 0;
			cmd[8] = 1;
			cmd[9] = 0;
		}

		DoSCSI(myior, (UWORD *)cmd, cmdlen, (UWORD *)old_data,
			global_d->rdb->rdb_BlockBytes, SCSIF_WRITE|SCSIF_AUTOSENSE);

		DeleteStdIO(myior);
		FreeMem(old_data,SelectedDrive->rdb->rdb_BlockBytes);
		return TRUE;
	}

	DeleteStdIO(myior);
	FreeMem(old_data,SelectedDrive->rdb->rdb_BlockBytes);
	return FALSE;
}

