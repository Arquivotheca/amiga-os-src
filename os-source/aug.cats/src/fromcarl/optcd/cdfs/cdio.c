/***********************************************************************
****************                                        ****************
****************        -=  CDTV FILE SYSTEM  =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"
#include "fse.h"	/* ewhac/KMY (2707) */

IMPORT	struct Task *FindTask();
IMPORT	APTR FetchA4();

/***********************************************************************
***
***  DiskChanged
***
***	Low-level disk change interrupt server.
***	If there are no other disk change interrupt servers in the
***	list, then no one else is interested in disk inserts, and
***	we will reset the machine!
***
***	May want to delay if SCSI or TrackDisk is active, or if
***	we are not the Boot device.
***
***********************************************************************/
extern	VOID DiskChanged();
#asm
	XREF	ResetAmiga
	XDEF	_DiskChanged
_DiskChanged:
		movem.l	d2/a2/a4/a6,-(sp)
		move.l	10(a1),a4	; data segment
		jsr	_ChangeFunc
		tst.l	d0		; reset? CES V22.3
		bne.s	ResetAmiga	; V22.4
		movem.l	(sp)+,d2/a2/a4/a6
		rts

#endasm

/***********************************************************************
***
***  ChangeFunc
***
***	An interrupt level function that handles disk change events.
***
***********************************************************************/
ChangeFunc()
{
	Debug1("\tC\n");

	/* Reset the machine if no other change int is found... V22.3 */
	if (NoReset > 0 && InhibitCount == 0 &&   /* V34.2 */
		(ULONG)(DevChgReq.io_Message.mn_Node.ln_Pred)+4 == 
		(ULONG)(DevChgReq.io_Message.mn_Node.ln_Succ))
	{
		return TRUE;
	}

	InsertFlag = TRUE;
	Signal(FSProc, 1 << (FSPort->mp_SigBit));
	return FALSE;  /* no reset */
}

/***********************************************************************
***
***  InitBootIO
***
***	Setup a temporary IO connection for checking out the boot disk.
***
***********************************************************************/
InitBootIO()
{
	BootPort.mp_Flags = 0;
	BootPort.mp_SigBit = AllocSignal(-1);
	BootPort.mp_SigTask = FindTask(NULL);
	NewList(&BootPort.mp_MsgList);

	DevIOReq.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	DevIOReq.io_Message.mn_ReplyPort = &BootPort;	
	if (OpenDevice(&DeviceName[0],0,&DevIOReq,0)) return FALSE;

	return TRUE;
}

QuitBootIO()
{
	CloseDevice(&DevIOReq);
	FreeSignal(BootPort.mp_SigBit);
}

/***********************************************************************
***
***  InitCDIO
***
***********************************************************************/
InitCDIO()
{
	extern Reset();

	if (OpenDevice("input.device",0,&InpReq,0)) return FALSE;

	/* Ewhac/KMY (2414) (timer) */
	/*  Init and open timer.  */
	if (OpenDevice ("timer.device", UNIT_MICROHZ, &TimeReq, 0))
		return (FALSE);

	TimeReq.tr_node.io_Message.mn_ReplyPort = FSPort;
	TimeReq.tr_node.io_Command		= TR_ADDREQUEST;

	/* Init and open our device: */
	DevIOReq.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	DevIOReq.io_Message.mn_ReplyPort = FSPort;
	if (OpenDevice(DeviceName,UnitNumber,&DevIOReq,OpenFlags))
		{
		Debug0( "%s failed to open!\n", DeviceName );	/* KMY 2417 */
		return FALSE;
		}

	DevChgReq = DevIOReq;		/* copy struct */

	/* Ewhac/KMY (2414) (UsingCDTV) */
	if ( UsingCDTV )
		{
		Debug2 ("CDTV!\n");		/*  ewhac  */
		DoDevIO(CD_START,0,0,0);	/* start HW ints again V22.2 */
		DoDevIO(CD_OPTIONS,0,DebugLevel>>16,0);	/* V34.4 */
		}

	/* Setup disk change interrupt and its request: */
	DiskChgInt.is_Node.ln_Type = NT_INTERRUPT;
	DiskChgInt.is_Code = &DiskChanged;
	DiskChgInt.is_Data = (APTR)&DevChgReq;

	/* Ewhac/KMY (2414) (length/TRUE) */
	DevChgReq.io_Message.mn_Node.ln_Name = (char *)FetchA4();	
	DevChgReq.io_Command = CD_ADDCHANGEINT;	
	DevChgReq.io_Data = (APTR)&DiskChgInt;	
	DevChgReq.io_Length = sizeof (struct Interrupt);
	SendIO(&DevChgReq);

	return( TRUE );
}

/***********************************************************************
***
***  MakeCache
***
***********************************************************************/
MakeCache()
{
	MUST(Cache = MakeVector(CacheSize << BlockShift));
	MUST(DirCache = MakeVector(DirCacheSize << BlockShift));
	MUST(DirCacheIndex = MakeVector(DirCacheSize * sizeof(ULONG)));
	return TRUE;
}

/***********************************************************************
***
***  MoreCache
***
***	Add or subtract some number of blocks from the disk cache.
***	(Implicit flush in this code).
***
***********************************************************************/
MoreCache(new)
	REG LONG new;
{
	if (!new) return CacheSize;

	new += CacheSize;
	if (new <= 2) new = 2;

	Forbid();	/* Don't want to risk losing my memory! */
	FreeVector(Cache);
	Cache = MakeVector(new << BlockShift);
	while (!Cache) /* Too big... come as close as we can. */
	{
		Cache = MakeVector((--new) << BlockShift);
	}
	Permit();

	CacheBlock = 0;		/* Flush */
	CacheSize = new;

	return new;
}

/***********************************************************************
***
***  FlushCache
***
***********************************************************************/
FlushCache()
{
	REG int i;

	for (i = 0; i < DirCacheSize; i++) DirCacheIndex[i] = 0;
	DirCacheBlock = 0;
	CacheBlock = 0;
}

/***********************************************************************
***
***  ReadBlock
***
***	V34.2 - changes to stat gathering
***
***********************************************************************/
UBYTE *ReadBlock(lbn)
	ULONG	lbn;
{
	Debug3("\tb%lx\n",lbn);	

	BlockCount++;

	if (CacheBlock && lbn >= CacheBlock && lbn < CacheBlock+CacheSize)
	{
		/* KMY (2713/2721) */
		FSE( FSEF_BLOCK_CACHE, lbn );

		HitCount++;
		return &Cache[(lbn-CacheBlock) << BlockShift];
	}

	/* KMY (2713/2721) */
	FSE( FSEF_BLOCK_NOCACHE, lbn );

	ReadCount += CacheSize;

	ReadCD(Cache,lbn,CacheSize);
	CacheBlock = lbn;
	return Cache;
}

/***********************************************************************
***
***  GetDirBlock
***
***********************************************************************/
DIREC *GetDirBlock(lbn)
	ULONG lbn;
{
	DIREC *dr;
	REG int i;

	for (i = 0; i < DirCacheSize; i++)
	{
		if (lbn == DirCacheIndex[i])
		{
			/* KMY (2713/2721) */
			FSE( FSEF_DIR_CACHE, lbn );

			return (DIREC *)(&DirCache[i<<BlockShift]);
		}
	}

	/* KMY (2713/2721) */
	FSE( FSEF_DIR_NOCACHE, lbn );

	do
	{
		dr = (DIREC *)ReadBlock(lbn);
	}
	while (!dr);		/* V24.4 - Must read it! */
	/*	if (!dr) return NULL;*/

	CopyMem(dr,&DirCache[DirCacheBlock<<BlockShift],BlockSize);

	DirCacheIndex[DirCacheBlock++] = lbn;

	if (DirCacheBlock >= DirCacheSize) DirCacheBlock = 0; /* wrap */

	return dr;
}

/***********************************************************************
***
***  ReadCD
***
***********************************************************************/
ReadCD(buf,blk,len)
	char *buf;
	ULONG blk;
	ULONG len;
{
	return ReadBytes(buf, blk << BlockShift, len << BlockShift);
}

/***********************************************************************
***
***  ReadBytes
***
***	Ewhac/KMY (2414) (speed simulation/UsingCDTV)
***
***********************************************************************/
ReadBytes(buf,offset,size)
	char *buf;
	ULONG offset;
	ULONG size;
{
	REG LONG err;
	REG LONG i;

	if ( UsingCDTV )
		size += 8;	/* V23.1 - compensate for DMA H/W bug! */

	if ( SimuSpeed )
		{
		i = ( ( 1000000 * size ) / SimuSpeed );
		if ( err = ( i / 1000000 ) )
			i -= ( 1000000 * err );

		TimeReq.tr_time.tv_micro = i;
		TimeReq.tr_time.tv_secs	 = err;
		DoIO( &TimeReq );
		}

	for (i = 0; i < Retry; i++)
		{
		/* ewhac/KMY (2707/2721/2805) (also Debug4()) */
		FSE( FSEF_LOW_READ, ( offset >> BlockShift ), ( ( size + 2047 ) >> BlockShift ) );

		Debug4( "BLK: Off, siz: %ld, %ld",
			( offset + ZeroPoint ), size );

		err = DoDevIO( CD_READ, ( offset + ZeroPoint ), size, buf );

		Debug4 ("\r\n");

		if (err == 0) return 0;
		ReadErrs++;
		Debug1("\tE%lx.%lx.%lx.%lx\n",err,offset,size,buf);
		if (err != CDERR_DMAFAILED) break;
	}
	return err;
}

/***********************************************************************
***
***  ReMount
***
***	Called when a disk change interrupt has been detected in 
***	the main dispatch loop.
***
***	Re-initializes data structures each time, but this routine
***	happens so rarely that this is not a concern.
***
***********************************************************************/
ReMount()
{
	/* V34.2 moved inhibit check to main.c */
	InsertFlag = FALSE;

	if (InputPends) return FALSE;	/* it's busy, ignore it */
	InputPends = TRUE;

	DoDevIO(CD_CHANGESTATE,0,0,0);
	if (DevIOReq.io_Actual)		/* true when no disk */
	{
		InpEvent.ie_Class = IECLASS_DISKREMOVED;
		UnMount();
	}
	else
	{
		InpEvent.ie_Class = IECLASS_DISKINSERTED;
		Mount(FALSE);
	}
	InpEvent.ie_NextEvent = NULL;
	/* No time stamp. */

	InpPacket.dp_Link = (struct Message *) &InpReq;
	InpPacket.dp_Port = FSPort; 
	InpPacket.dp_Type = ACTION_DISK_CHANGE;

	InpReq.io_Message.mn_Node.ln_Name = (char *) &InpPacket;
	InpReq.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	InpReq.io_Message.mn_ReplyPort = FSPort;
	InpReq.io_Command = IND_WRITEEVENT;	
	InpReq.io_Data = (APTR) &InpEvent;
	InpReq.io_Length = sizeof(InpEvent);

	SendIO(&InpReq);

	return TRUE;
}

