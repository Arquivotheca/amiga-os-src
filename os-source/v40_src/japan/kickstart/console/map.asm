**
**	$Id: map.asm,v 36.9 91/04/12 17:28:10 darren Exp $
**
**      console.device character map manipulation commands
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"
	INCLUDE	"debug.i"

	INCLUDE	"exec/ables.i"

**	Exports

	XDEF	CDObtainMap
	XDEF	CDReleaseMap

	XDEF	CDCBObtainMap
	XDEF	CDCBReleaseMap

	XDEF	BeginMapSemaphore
	XDEF	InitMapSemaphore
	XDEF	ObtainMapSemaphore
	XDEF	ReleaseMapSemaphore

**	Imports

	XLVO	GetMsg			; Exec
	XLVO	Permit			;
	XLVO	PutMsg			;
	XLVO	ReplyMsg		;
	XLVO	Signal			;
	XLVO	WaitPort		;

	XREF	EndCommand

*****i* console.device/CD_OBTAINMAP **********************************
*
*   NAME
*	CD_OBTAINMAP -- Acquire locked character map.
*
*   FUNCTION
*	This command gets a pointer to the locked ConsoleMap
*	associated with a character mapped console unit, i.e. one
*	opened with a unit with CDUNITF_MAPPED set.  The map contents
*	may be inspected and/or modified, but the map dimensions must
*	not be changed.  Updates to all consoles can be inhibited
*	while an application owns this map, so CD_RELEASEMAP should
*	be performed as quickly as possible.
*
*   IO REQUEST INPUT
*	io_Message -	with valid mn_ReplyPort
*	io_Device -	preset by the call to OpenDevice
*	io_Unit -	preset by the call to OpenDevice
*	io_Command -	CD_OBTAINMAP
*
*   IO REQUEST RESULT
*	io_Error -	can be one of:
*	    CDERR_NOUNIT -  not a real unit (i.e. OpenDevice w/ -1 unit)
*	    CDERR_NOMAP	-   no character map is available for this unit
*	io_Actual -	sizeof(ConsoleMap)
*	io_Length -	sizeof(ConsoleMap)
*	io_Data -	struct ConsoleMap *, the character map
*
**********************************************************************
CDCBObtainMap	EQU	-1

CDObtainMap:
		movem.l a2-a3,-(a7)
		move.l	a1,a3			; save I/O Request
		move.l	IO_UNIT(a3),a2
		move.l	a2,d0			; cmp.l	#-1,a2
		addq.l	#1,d0
		beq.s	mNoUnit

		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	mNoMap

		clr.b	IO_ERROR(a3)
		moveq	#ConsoleMap_SIZEOF,d0
		move.l	d0,IO_ACTUAL(a3)
		move.l	d0,IO_LENGTH(a3)
		lea	cu_CM(a2),a0
		move.l	a0,IO_DATA(a3)
		bsr	BeginMapSemaphore
		tst.l	d0
		bne.s	endOK

		bclr	#IOB_QUICK,IO_FLAGS(a3)
		movem.l	(a7)+,a2-a3
		rts

endOK:
		moveq	#0,d0
endCommand:
		move.l	a3,a1
		movem.l	(a7)+,a2-a3
		bra	EndCommand

mNoUnit:
		moveq	#CDERR_NOUNIT,d0
		bra.s	endCommand
		
mNoMap:
		moveq	#CDERR_NOMAP,d0
		bra.s	endCommand

mBadMap:
		moveq	#CDERR_BADMAP,d0
		bra.s	endCommand


*****i* console.device/CD_RELEASEMAP *********************************
*
*   NAME
*	CD_RELEASEMAP -- Release acquired character map.
*
*   FUNCTION
*	This command releases the ConsoleMap locked via CD_OBTAINMAP,
*	and optionally refreshes the display from the map.  The
*	application must set io_Offset to indicate a refresh when it
*	has made changes to the map.
*
*   IO REQUEST INPUT
*	io_Message -	with valid mn_ReplyPort
*	io_Device -	preset by the call to OpenDevice
*	io_Unit -	preset by the call to OpenDevice
*	io_Command -	CD_RELEASEMAP
*	io_Length -	preset by the CD_OBTAINMAP command
*	io_Data -	preset by the CD_OBTAINMAP command
*	io_Offset -	0 if no map changes made, 1 if refresh needed
*
*   IO REQUEST RESULT
*	io_Error -	can be one of:
*	    CDERR_NOUNIT -  not a real unit (i.e. OpenDevice w/ -1 unit)
*	    CDERR_BADMAP -  map not acquired via CD_OBTAINMAP
*
**********************************************************************
CDCBReleaseMap	EQU	-1

CDReleaseMap:
		movem.l a2-a3,-(a7)
		move.l	a1,a3			; save I/O Request
		move.l	IO_UNIT(a3),a2
		move.l	a2,d0			; cmp.l	#-1,a2
		addq.l	#1,d0
		beq.s	mNoUnit

		lea	cu_CM(a2),a0
		cmp.l	IO_DATA(a3),a0
		bne.s	mBadMap

		bsr	ReleaseMapSemaphore

		tst.l	IO_OFFSET(a3)
		beq.s	endOK

		bset	#CUB_REFRESH,cu_Flags(a2)
		move.l	#CDSIGF_REFRESH,d0
		lea	cd_TC(a6),a1
		LINKEXE	Signal
		bra.s	endOK


*****i* InitMapSemaphore *********************************************
*
*   a2	console unit
*
InitMapSemaphore:
		move.b	#PA_IGNORE,cu_MapSemaphore+MP_FLAGS(a2)
		move.w	#-1,cu_MapSemaphore+SM_BIDS(a2)
		rts


*****i* ObtainMapSemaphore *******************************************
*
*   a2	console unit
*
ObtainMapSemaphore:
		;-- use BeginMapSemaphore w/ message on the stack
		lea	-MN_SIZE-MP_SIZE(a7),a7	; message & message port
		move.w	#SIGB_SINGLE,MP_FLAGS(a7)	; clr FLAGS, set SIGBIT
		move.l	cd_ExecLib(a6),a0
		move.l	ThisTask(a0),MP_SIGTASK(a7)
		lea	MP_MSGLIST(a7),a1
		NEWLIST	a1
		lea	MP_SIZE(a7),a1
		move.l	a7,MN_REPLYPORT(a1)
		bsr.s	BeginMapSemaphore
		tst.l	d0
		bne.s	omsProcured

	;--@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	;--@@@ BROKEN - BUT NEVER EXECUTED!

		lea	MP_SIZE(a7),a1
		LINKEXE	WaitPort

omsProcured:
		lea	MN_SIZE+MP_SIZE(a7),a7
		rts


*****i* BeginMapSemaphore ********************************************
*
*   a1	semaphore message
*   a2	console unit
*
BeginMapSemaphore:
		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH
		addq.w	#1,cu_MapSemaphore+SM_BIDS(a2)
		bne.s	bmsBid
		move.l	a1,cu_MapSemaphore+SM_LOCKMSG(a2)
		moveq	#1,d0
bmsExit:
		LINKEXE	Permit
		rts

bmsBid:
		lea	cu_MapSemaphore(a2),a0
		LINKEXE PutMsg
		moveq	#0,d0
		bra.s	bmsExit


*****i* ReleaseMapSemaphore ******************************************
*
*   a1	semaphore message, or zero for current locker
*   a2	console unit
*
ReleaseMapSemaphore:
		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH
		move.l	a1,d0
		beq.s	rmsCurrent

		;--	release specific locker
		cmp.l	cu_MapSemaphore+SM_LOCKMSG(a2),a1
		beq.s	rmsCurrent

		;--	look for lock request in pending list
rmsLookLoop:
		move.l	cu_MapSemaphore+MP_MSGLIST+LH_HEAD(a2),a0
		move.l	(a0),d0
		beq.s	rmsDone

		cmp.l	a0,a1
		beq.s	rmsInList

		move.l	d0,a0
		bra.s	rmsLookLoop

rmsInList:
		REMOVE
		subq.w	#1,cu_MapSemaphore+SM_BIDS(a2)
		moveq	#1,d0
		bra.s	rmsDone

rmsCurrent:
		clr.l	cu_MapSemaphore+SM_LOCKMSG(a2)
		subq.w	#1,cu_MapSemaphore+SM_BIDS(a2)
		bge.s	rmsNext

rmsDone:
		LINKEXE	Permit
		rts

rmsNext:
		lea	cu_MapSemaphore(a2),a0
		LINKEXE GetMsg
		move.l	d0,cu_MapSemaphore+SM_LOCKMSG(a2)
		move.l	d0,a1
		LINKEXE	ReplyMsg
		bra.s	rmsDone

	END
