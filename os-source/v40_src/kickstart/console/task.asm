**
**	$Id: task.asm,v 36.29 93/03/05 16:59:49 darren Exp $
**
**      console task
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/execbase.i"
	INCLUDE	"devices/timer.i"
	INCLUDE "internal/librarytags.i"

	INCLUDE	"debug.i"

**	Exports

	XDEF	CDTaskStart
	XDEF	GetEvents		;exported (see write.asm)
					;Semaphore access required



**	Imports

	XLVO	GetMsg			; Exec
	XLVO	ObtainSemaphoreShared	;
	XLVO	ObtainSemaphore		;
	XLVO	OpenDevice		;
	XLVO	ReleaseSemaphore	;
	XLVO	Wait			;

	XREF	LockDRPort		;able.asm
	XREF	UnLockRPort		;able.asm
	XREF	RefreshDamage		;refresh.asm
	XREF	ReSizeUnit
	XREF	SelectClip

	XREF	IfNewSize		;refresh.asm
	XREF	IfGhostCursor		;cursor.asm

	XREF	CursRender

*---------------------------------------------------------------------
		CNOP	0,4
CDTaskStart:
		;--	grab the device base
		move.l	4(sp),a5

		;--	allocate hard coded signals
		move.l	cd_ExecLib(a5),a6	; set up a6 == SysBase
		move.l	ThisTask(a6),a1
		or.l	#CDSIG_HARDCODED,TC_SIGALLOC(a1)

		;--	initialize timer port
		lea	cd_TPort(a5),a0
		move.l	a0,cd_TIOR+MN_REPLYPORT(a5)
		move.b	#NT_MSGPORT,cd_TPort+LN_TYPE(a5)
		move.w	#CDSIGB_TICK,cd_TPort+MP_FLAGS(a5) ; FLAGS=0, SIGBIT=b
		move.l	a1,cd_TPort+MP_SIGTASK(a5)
		lea	cd_TPort+MP_MSGLIST(a5),a0
		NEWLIST	a0

		suba.l	a0,a0			;ODTAG_TIMER
	;;	lea	TDName(pc),a0
		moveq	#UNIT_VBLANK,d0
		lea	cd_TIOR(a5),a1
		moveq	#0,d1
		CALLLVO	OpenDevice

		lea	cd_USemaphore(a5),a0
		CALLLVO	ObtainSemaphoreShared

		move.l	a5,a6			; set up a6 == ConsoleDevice

taskPrimeTick:
		move.b	cd_SelectFlags(a6),d0
		move.b	d0,d1
		and.b	#CDSF_CIRCLING!CDSF_DRAGGING!CDSF_SELECTING,d1
		beq.s	taskWait
		btst	#CDSB_SELECTDOWN,d0
		beq.s	taskWait
		bset	#CDSB_TICKING,d0
		bne.s	taskWait
		move.b	d0,cd_SelectFlags(a6)

		move.l	cd_TIOR+IO_DEVICE(a6),d0
		beq.s	taskWait
		lea	cd_TIOR(a6),a1
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		clr.b	IO_FLAGS(a1)
		moveq	#CD_TEVENTSECS,d1
		move.l	d1,IOTV_TIME+TV_SECS(a1)
		move.l	#CD_TEVENTMICRO,IOTV_TIME+TV_MICRO(a1)
		move.l	a6,-(a7)
		move.l	d0,a6
		jsr	DEV_BEGINIO(a6)
		move.l	(a7)+,a6

	;-- wait for signal
taskWait:
		lea	cd_USemaphore(a6),a0
		LINKEXE	ReleaseSemaphore

		move.l	#CDSIG_HARDCODED,d0
		LINKEXE	Wait
		move.l	d0,d7

		lea	cd_USemaphore(a6),a0
		LINKEXE	ObtainSemaphoreShared


		;safety feature - if I were to awake without any
		;windows, just ignore this stuff.

		tst.l	cd_WindowCount(a6)
		beq	taskPrimeTick

		;-- EVENT LIST

		move.l	d7,d0
		andi.l	#CDSIGF_NEWACTIVE!CDSIGF_REFRESH,d0
		beq.s	checkSelect

		;-- loops through list of console units looking
		;-- for new events (tail != head)
		;
		;-- Refresh, and Newsize are handled by the same
		;-- routine.  If newsize, a full redraw is done
		;-- throwing away the damage.
		;
		;-- The active bit in cu_States reflects the window's
		;-- activation state at the time the event occured.
		;-- This is necessary for proper cursor drawing,
		;-- and restoration.

		move.l	d7,-(sp)		;preserve D7

		lea	cd_EVSemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		lea	cd_UHead(a6),a2		;loop through units
get_events:
		bsr.s	GetEvents		;handle pending events

next_unit:	move.l	(a2),a2
                tst.l	(a2)
		bne.s	get_events


		lea	cd_EVSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore

		move.l	(sp)+,d7		;restore D7

checkSelect:
		btst	#CDSIGB_SELECT,d7
		beq.s	checkTick

		bsr	SelectClip

checkTick:
		btst	#CDSIGB_TICK,d7
		beq	taskPrimeTick
		lea	cd_TPort(a6),a0
		LINKEXE	GetMsg

		bclr	#CDSB_TICKING,cd_SelectFlags(a6)
		beq	taskPrimeTick

		bsr	SelectClip

		bra	taskPrimeTick

***************************************************************************
* Removes events from list of events maintained in each window structure.
* 
* This routine only updates the tail index - the inputhandler which
* adds tokens only updates the head of the list.  Tokens pulled off
* after updating tail.
*
* Function is exported so that it can be used before writing text to update
* any pending things.  Protected by a semaphore such that only one task
* at a time can remove tokens.
*
* The active state bit held for each window is updated within this routine
* to reflect the proper active state needed to properly draw, and undraw
* the cursor at the time the event happened.
*
* Continguous refresh, and newsize events are accumulated, and are added
* as only one event by the inputhandler.
*
* register usage:
*
* IN - a2 points to unit structure
*
* USED - d1 index into list
*        d0 holds token removed from list
*        

GetEvents:

		lea	cu_IList(a2),a0
		moveq	#00,d1
		move.b	cu_ITail(a2),d1
		cmp.b	cu_IHead(a2),d1
		beq.s	GotEvents


		; update tail (1 instruction) - remove token after
		; bumping index.

		addq.b	#1,cu_ITail(a2)
		move.b	0(a0,d1),d0

		btst	#IPB_MAKEACTIVE,d0
		beq.s	ifinactive

		bset	#CUB_ISACTIVE,cu_States(a2)
fix_cursor:
		;ignore cursor updating if this is a SIMPLE
		;refresh, non-character mapped console

		bsr	IfGhostCursor
		beq.s	GetEvents

		;
		;ignore cursor updating if resize pending
		;fixes cursor border trashing if activate followed
		;by newsize before these can be handled.
		;
		;case - activate window by clicking on sizing
		;gadget, and drag smaller such that a window
		;border clips the cursor.  Do NOT release mouse
		;button before dragging - The call to LockDRPort
		;is put on hold until the mouse button is released.
		;

		bsr	LockDRPort

		bsr	IfNewSize
		bne.s	skipAhead

		bsr	CursRender
skipAhead:
		bsr	UnLockRPort
		bra.s	GetEvents

ifinactive:	btst	#IPB_MAKEINACTIVE,d0
		beq.s	ifdamaged

		bclr	#CUB_ISACTIVE,cu_States(a2)
		bra.s	fix_cursor

ifdamaged:
		bclr	#CUB_ISACTIVE,cu_States(a2)
		btst	#IPB_ISACTIVE,d0
		beq.s	notactive
		bset	#CUB_ISACTIVE,cu_States(a2)
notactive:
		bsr	RefreshDamage
		bra.s	GetEvents

GotEvents:	rts


	END
