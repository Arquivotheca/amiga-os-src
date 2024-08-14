**
**	$Id: inputhandler.asm,v 36.36 92/12/15 08:31:10 darren Exp $
**
**      console device input handler
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/ables.i"
	INCLUDE	"debug.i"
**	Exports

	XDEF	CDInputHandler


**	Imports

	XLVO	ObtainSemaphoreShared	; Exec
	XLVO	Permit			;
	XLVO	RawDoFmt		;
	XLVO	ReleaseSemaphore	;
	XLVO	Signal			;

	XLVO	MapRawKey		; Keymap

	XREF	PutReadData
	XREF	PutReadSnip

	XREF	ReSizePost

	XREF	CDRead


**	Assumptions

	IFNE	IEQUALIFIERB_LEFTBUTTON/8-1
	FAIL	"IEQUALIFIERB_LEFTBUTTON not in high byte, recode"
	ENDC
	IFNE	IEQUALIFIERB_MULTIBROADCAST/8-1
	FAIL	"IEQUALIFIERB_MULTIBROADCAST not in high byte, recode"
	ENDC
	IFNE	IEQUALIFIERB_REPEAT/8-1
	FAIL	"IEQUALIFIERB_REPEAT not in high byte, recode"
	ENDC
	IFNE	IEQUALIFIERB_RCOMMAND/8
	FAIL	"IEQUALIFIERB_RCOMMAND not in low byte, recode"
	ENDC


******* console.device/CDInputHandler ********************************
*
*    NAME
*	CDInputHandler -- handle an input event for the console device
*
*    SYNOPSIS
*	events = CDInputHandler(events, consoleDevice)
*		       a0      a1
*
*    FUNCTION
*	Accept input events from the producer, which is usually the
*	rom input.task.
*
*    INPUTS
*	events - a pointer to a list of input events.
*	consoleDevice - a pointer to the library base address of the
*	    console device.  This has the same value as ConsoleDevice
*	    described below.
*
*    RESULTS
*	events - a pointer to a list of input events not used by this
*	    handler.
*
*    NOTES
*	This function is available for historical reasons.  It is
*	preferred that input events be fed to the system via the
*	WriteEvent command of the input.device.
*
*	This function is different from standard device commands in
*	that it is a function in the console device library vectors.
*	In order to obtain a valid library base pointer for the 
*	console device (a.k.a. ConsoleDevice) call
*	OpenDevice("console.device", -1, IOStdReq, 0),
*	and then grab the io_Device pointer field out of the IOStdReq
*	and use as ConsoleDevice.
*
*    BUGS
*
*    SEE ALSO
*	input.device
*
********************************************************************************

*****************************************************
* Add an event to an input list (circular)
*
* Only adds new events - no duplicates.
* Refresh, and resize are cumulative, and the
* same except a special bit is set in the unit
* structure which means resize happened.  One refresh,
* or full redraw suffices for multiple refresh/resize events.
* Resize can also be caught before the event happens because
* the window height/width values are tracked, and checked.
*
* The main use of the token system is activation/inactivation
* tracking.  This is because an event such as refresh followed
* by activation requires that the console be refreshed in
* the inactive state (to get the cursor right), and then
* made active by redrawing the cursor.
*
* IN: a2 = console unit addr
*     d4 = event token (byte)
*
* USED: a1 = ptr to event in list
*       d3 = offset (head)
*
* All registers must be preserved by the caller - this is
* intended for speedy insertion of event tokens.  See cddata.i
* for bit definitions.
*

AddEvent:
		lea	cu_IList(a2),a1

		moveq	#00,d3
		move.b	cu_IHead(a2),d3
		cmp.b	cu_ITail(a2),d3	;load in register
		beq.s	addevent

		;-- compare with last event
		;-- refresh/resize are cumulative

		subq.b	#1,d3
		cmp.b	0(a1,d3),d4	;compare tokens
		beq.s	sameevent
		addq.b	#1,d3

addevent:
		move.b	d4,0(a1,d3)	;flag info

		;update head last so task can't jump ahead
		;though due to Exec's task scheduling, the
		;task can't be running while we are in this
		;code.
		addq.b	#1,d3		;wraps around	
		move.b	d3,cu_IHead(a2)					
		
sameevent:
                rts

FindEventConsole:
		move.l	cd_UHead(a6),d0		; get unit list
		move.l	ie_EventAddress(a3),d1	; get event window

fcLoop:
		move.l	d0,a0
		move.l	(a0),d0
		beq.s	fcNotFound
		cmp.l	cu_Window(a0),d1
		bne.s	fcLoop

		move.l	a0,d0
fcNotFound:
		rts

		cnop	0,4

CDInputHandler:
		movem.l	d2-d4/a2-a4/a6,-(a7)
		link	a5,#-((CD_CONVBUFSIZE+1)&$fffe)	; ensure even

		move.l	a1,a6		; get console device base
		move.l	a0,d0
		beq	cdihRts
		move.l	a0,a3
		moveq	#0,d2


		lea	cd_USemaphore(a6),a0
		LINKEXE	ObtainSemaphoreShared


loopEvents:
		move.b	ie_Class(a3),d0
		cmpi.b	#IECLASS_EVENT,d0
		bne	checkMultibroadcast

		;-- check for IECLASS_EVENT IECODE_NEWACTIVE
		move.w	ie_Code(a3),d0
		subq.w	#IECODE_NEWACTIVE,d0
		bne.s	checkNewsizeEvent

		;-- a new intuition window is active
		;--	indicate old active unit is no longer active
		move.l	cd_Active(a6),a2
		move.l	a2,d0
		beq.s	findNewActive

		;-- ADD MAKEINACTIVE EVENT
		moveq	#IPF_ISACTIVE!IPF_MAKEINACTIVE,d4 ;was Active before
		bsr	AddEvent


findNewActive:
		;--	find new active unit
		clr.l	cd_Active(a6)		; guess not console unit

		;--	    save new active window
		move.l	ie_EventAddress(a3),cd_ActiveWindow(a6)
		bsr.s	FindEventConsole
		beq.s	checkOldA

		move.l	d0,a2
		move.l	a2,cd_Active(a6)

		and.b	#CDS_SELECTMASK,cd_SelectFlags(a6)

		;-- ADD EVENT

		moveq	#IPF_MAKEACTIVE,d4	;was not active before
		bsr	AddEvent

checkOldA:
		move.l	#CDSIGF_NEWACTIVE,d0
		move.l	a2,d1			; something to signal about?
		bne.s	signalTC
		bra	dispatchActive


		;-- check for IECLASS_EVENT IECODE_NEWSIZE
checkNewsizeEvent:
		subq.w	#IECODE_NEWSIZE-IECODE_NEWACTIVE,d0
		bne.s	checkRefreshEvent
		bsr	FindEventConsole
		beq	chkNextEvent

		move.l	d0,a2
		bsr	ReSizePost

		;-- ADD EVENT

		moveq	#IPF_REDRAW,d4
		cmp.l	cd_Active(a6),a2
		bne.s	setNewSize
		bset	#IPB_ISACTIVE,d4
setNewSize:
		bsr	AddEvent

* Old code - seems wrong as it won't signal in the event of
* no character map when a resize happens (yet it should)
* because I may have to force the cursor into visible range,
* and clean up clipped bits of text around the borders.
*		bra.s	dispatchA2
*
		bra.s	sigRefresh


		;-- check for IECLASS_EVENT IECODE_REFRESH
checkRefreshEvent:
		subq.w	#IECODE_REFRESH-IECODE_NEWSIZE,d0
		bne.s	dispatchWindow

		;-- check for a refresh on a buffered console
		bsr	FindEventConsole
		beq.s	chkNextEvent

		move.l	d0,a2
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	dispatchA2

		bset	#CUB_REFRESH,cu_Flags(a2)

		;-- ADD EVENT

		moveq	#IPF_REDRAW,d4
		cmpa.l	cd_Active(a6),a2
		bne.s	setDamaged
		bset	#IPB_ISACTIVE,d4
setDamaged:
		bsr	AddEvent

sigRefresh:
		move.l	#CDSIGF_REFRESH,d0

		;-- signal task that something happened
signalTC:
		lea	cd_TC(a6),a1
		LINKEXE	Signal
		bra.s	dispatchWindow


checkMultibroadcast:
		btst	#IEQUALIFIERB_MULTIBROADCAST&7,ie_Qualifier(a3)
		beq.s	notMultibroadcast

		;-- dispatch input for all console windows
		move.l	cd_UHead(a6),a2		; get first unit
nextDUnit:
		tst.l	(a2)
		beq.s	chkNextEvent	; no more units
		bsr	dispatchEvent
		move.l	(a2),a2		; next unit
		bra.s	nextDUnit


notMultibroadcast:
		;-- see if the event specifies the window

		;-- As of 36.4130 of intuition.library, SIZEWINDOW,
		;-- and CHANGEWINDOW are broadcast with an event
		;-- address.  The old assumption worked only when the
		;-- user had manually activated a console.device window,
		;-- and moved, or resized it.  If a window was moved/sized
		;-- under program control (when it was not active), these
		;-- two classes would be sent to the active console window
		;-- even though they had nothing whatsoever to do with
		;-- that window.  If there was no active console window,
		;-- they would be lost.

IEWINDOWCLASS1	EQU	(1<<IECLASS_REQUESTER)!(1<<IECLASS_REFRESHWINDOW)
IEWINDOWCLASS2	EQU	(1<<IECLASS_ACTIVEWINDOW)!(1<<IECLASS_INACTIVEWINDOW)
IEWINDOWCLASS3	EQU	(1<<IECLASS_SIZEWINDOW)!(1<<IECLASS_CHANGEWINDOW)
IEWINDOWCLASS	EQU	IEWINDOWCLASS1!IEWINDOWCLASS2!IEWINDOWCLASS3

		move.l	#IEWINDOWCLASS,d1
		btst	d0,d1
		beq.s	dispatchActive


dispatchWindow:
		;-- dispatch input for window in event
		bsr	FindEventConsole
		beq.s	chkNextEvent
		move.l	d0,a2
		bra.s	dispatchA2

dispatchActive:
		;-- dispatch input for active console
		move.l	cd_Active(a6),d0
		beq.s	chkNextEvent
		move.l	d0,a2

dispatchA2:
		;-- dispatch input for console unit
		bsr.s	dispatchEvent

chkNextEvent:
		move.l	(a3),d0
		beq.s	endEvents
		move.l	d0,a3
		bra	loopEvents

endEvents:
		tst	d2
		beq.s	releaseUSem

		;-- satisfy any pending read with the new data
		move.l	cd_UHead(a6),a2		; get first unit
loopReadUnit:
		tst.l	(a2)
		beq.s	releaseUSem		; no more units
		bclr	#DUB_DISPATCH,du_Flags(a2)
		beq.s	nextReadUnit

		;--	satisfy data posted to this unit
		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH

		move.l	MP_MSGLIST(a2),a1
		tst.l	(a1)
		beq.s	ruPermit

		bsr	CDRead
ruPermit:
		move.l	cd_ExecLib(a6),a0
		PERMIT	a0,NOFETCH

nextReadUnit:
		move.l	(a2),a2		; next unit
		bra.s	loopReadUnit

releaseUSem:
		lea	cd_USemaphore(a6),a0
		LINKEXE	ReleaseSemaphore

cdihRts:
		moveq	#0,d0
		unlk	a5
		movem.l	(a7)+,d2-d4/a2-a4/a6
		rts


;***
;***	dispatchEvent - pass the event to unit a2
;***
dispatchEvent:
		moveq	#0,d0
		move.b	ie_Class(a3),d0

		cmpi.b	#IECLASS_RAWMOUSE,d0
		bne.s	deNoSelect

		;-- cache most current qualifier bits

		move.w	ie_Qualifier(a3),cd_MouseQual(a6)

		btst	#1,cu_DevUnit+3(a2)	; check if highlight enabled
		beq.s	deNoSelect
		tst.l	cu_CM+cm_AllocSize(a2)	; check for character map
		beq.s	deNoSelect

		move.w	ie_Code(a3),d1
		cmpi.w	#IECODE_LBUTTON,d1
		bne.s	deNoSelect

		and.b	#CDS_SELECTMASK,cd_SelectFlags(a6)
		move.w	ie_Qualifier(a3),d1
		and.w	#IEQUALIFIER_LSHIFT!IEQUALIFIER_RSHIFT,d1
		beq.s	deStdSelect
		;--	extended selection
		or.b	#CDSF_SELECTDOWN!CDSF_EXTENDED,cd_SelectFlags(a6)
		bra.s	deSelectSignal
		;--	normal selection
deStdSelect:
		or.b	#CDSF_SELECTDOWN,cd_SelectFlags(a6)

deSelectSignal:
		move.l	d0,-(a7)
		move.l	#CDSIGF_SELECT,d0
		lea	cd_TC(a6),a1

		LINKEXE	Signal
		move.l	(a7)+,d0

deNoSelect:
		cmpi.b	#IECLASS_MAX,d0
		bhi	checkRepeatFlush
		move.w	d0,d1
		andi.w	#07,d0
		lsr.w	#3,d1
		lea	cu_RawEvents(a2),a0
		btst	d0,0(a0,d1.w)
		beq	checkRepeatFlush
	    ;------ build the input event control sequence
		move.l	ie_TimeStamp+TV_MICRO(a3),d0
		bclr	#31,d0			; ensure not negative
		move.l	d0,-(a7)
		move.l	ie_TimeStamp+TV_SECS(a3),d0
		bclr	#31,d0			; ensure not negative
		move.l	d0,-(a7)
		moveq	#0,d0
		move.w	ie_Y(a3),d0
		move.l	d0,-(a7)
		move.w	ie_X(a3),d0
		move.l	d0,-(a7)
		move.w	ie_Qualifier(a3),d0
		move.l	d0,-(a7)
		move.w	ie_Code(a3),d0
		move.l	d0,-(a7)
		moveq	#0,d0
		move.b	ie_SubClass(a3),d0
		move.w	d0,-(a7)
		move.b	ie_Class(a3),d0
		move.w	d0,-(a7)
		move.l	a7,a1
		movem.l	a2/a3,-(a7)
		lea	ieFormat(pc),a0
		lea	-CD_CONVBUFSIZE(a5),a3
		lea	rawPutConvBuff(pc),a2
		LINKEXE	RawDoFmt
		movem.l	(a7)+,a2/a3
		lea	28(a7),a7

		;--	determine size of input event control sequence
		lea	-CD_CONVBUFSIZE(a5),a0
		moveq	#-1,d0
peSizeLoop:
		tst.b	(a0)+
		dbeq	d0,peSizeLoop
		not.w	d0
		bra	normalKey	;was .s!

ieFormat:
		dc.b	$9B
		dc.b	'%d;%d;%ld;%ld;%ld;%ld;%ld;%ld|'
		dc.b	0
		ds.w	0


rawPutConvBuff:
		move.b	d0,(a3)+
		rts

		;-- check for repeat-ahead flush
checkRepeatFlush:
		btst.b	#IEQUALIFIERB_REPEAT&7,ie_Qualifier(a3)
		beq.s	convertKey

		lea	MP_MSGLIST(a2),a1
		cmp.l	LH_TAIL+LN_PRED(a1),a1
		beq	ckRts		; no pending reader -- flush repeat

		;-- convert keycode into buffer
convertKey:
		move.l	a2,-(a7)	; save the unit
		move.l	a3,a0
		lea	-CD_CONVBUFSIZE(a5),a1
		move.l	#CD_CONVBUFSIZE,d1
		lea	cu_KeyMapStruct(a2),a2
		LINKKEY	MapRawKey
		move.l	(a7)+,a2	; restore the unit

		tst.w	d0		; get buffer actual
		beq.s	ckRts		;   nothing there

		;-- check for paste here
		cmp.w	#1,d0
		bne.s	normalKey
		btst.b	#IEQUALIFIERB_RCOMMAND,ie_Qualifier+1(a3)
		beq.s	normalKey
		move.b	-CD_CONVBUFSIZE(a5),d1
		and.b	#$df,d1
		cmp.b	#'V',d1
		bne.s	ifCopy

		;--	paste last selection
		bsr	PutReadSnip
		bra.s	nkDispatchDone

		;--	copy selection
ifCopy:		cmp.b	#'C',d1
		bne.s	normalKey
		
		bset	#CDIB_COPY,cd_InputFlags(a6)

		;--  signal task

		move.l	d0,-(sp)
		move.l	#CDSIGF_SELECT,d0
		lea	cd_TC(a6),a1
		LINKEXE	Signal
		move.l	(sp)+,d0		
		bra.s	nkDispatchDone

normalKey:
		lea	-CD_CONVBUFSIZE(a5),a0
		bsr	PutReadData

nkDispatchDone:
		tst.l	d0
		beq.s	ckRts

		bset	#DUB_DISPATCH,du_Flags(a2)
		moveq	#1,d2

ckRts:
		rts


	END
