	TTL	'$Id: inputtask.asm,v 40.0 93/03/12 16:03:50 darren Exp $'
**********************************************************************
*
*			------------
*   inputtask.asm	INPUT DEVICE		input task
*			------------
*
*   Copyright 1985, 1987 Commodore-Amiga Inc.
*
*   Source Control	$Locker:  $
*
*   $Log:	inputtask.asm,v $
*   Revision 40.0  93/03/12  16:03:50  darren
*   Use XREF'ed device name - save ROM space
*   
*   Revision 36.11  91/02/08  22:09:37  darren
*   No changes
*   
*   Revision 36.10  91/01/21  12:17:51  darren
*   Key repeat for 101 keys, but not for CDTV keys since the CDTV group
*   currently wants to use 1.3, but we may hose them later if we repeat
*   these under 2.0.  The could not have been repeated under 1.3 since
*   the CDTV keycodes are greater than $60.
*   
*   Revision 36.9  90/05/30  10:20:52  kodiak
*   recode, save 12 bytes
*   
*   Revision 36.8  90/05/22  12:10:28  kodiak
*   sets ie_TimeStamp for any IND_WRITEEVENT command event
*   
*   Revision 36.7  90/05/17  17:30:57  kodiak
*   sets ie_TimeStamp correctly in coalesced mouse events
*   
*   Revision 36.6  90/05/10  18:48:23  kodiak
*   use GetSysTime function instead of GETSYSTIME command
*   fix contents of id_RepeatNumeric to include numeric bit, not exclude it
*   
*   Revision 36.5  90/04/13  12:44:36  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.4  90/04/02  16:30:14  kodiak
*   back to using dd_ExecBase, not SYSBASE (4)
*   
*   Revision 36.3  90/04/02  12:58:50  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.2  90/01/22  16:29:04  kodiak
*   don't coalesce movement onto first occurance of button transition
*   
*   Revision 36.1  90/01/19  18:06:20  kodiak
*   make SetMXxxx commands synchronous
*   
*   Revision 35.7  89/09/20  20:51:12  kodiak
*   first cut at fixing initialization race condition (message port
*   initialization of command queue in task)
*   
*   Revision 35.6  89/08/08  09:54:42  kodiak
*   fix repeat strategy to allow corruption of repeat input event on food chain
*   
*   Revision 35.5  89/07/12  13:28:25  kodiak
*   have input task ReadEvent multiple events from gameport
*   (and have it work)
*   
*   Revision 35.4  89/07/11  16:57:24  kodiak
*   do not affect repeat in progress with qualifier key transitions
*   
*   Revision 35.3  89/07/07  14:08:44  kodiak
*   double buffer requests to keyboard device
*   
*   Revision 35.2  88/12/31  17:53:19  kodiak
*   set qualifier word on tick event
*   
*   Revision 35.1  88/08/02  12:27:32  kodiak
*   use SYSBASE (4) vs. dd_ExecBase
*   (a poor decision, in hindsight)
*   
*   Revision 35.0  87/10/26  11:31:40  kodiak
*   initial from V34, but w/ stripped log
*   
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/interrupts.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"devices/timer.i"

	INCLUDE		"inputevent.i"
	INCLUDE		"gameport.i"
	INCLUDE		"keyboard.i"
	INCLUDE		"input.i"

	INCLUDE		"macros.i"
	INCLUDE		"stddevice.i"
	INCLUDE		"iddata.i"

*------ Imported Names -----------------------------------------------

	XREF_EXE	AllocSignal
	XREF_EXE	CloseDevice
	XREF_EXE	GetMsg
	XREF_EXE	OpenDevice
	XREF_EXE	SetSignal
	XREF_EXE	Signal
	XREF_EXE	Wait
	XREF_EXE	WaitIO

	XREF		_LVOGetSysTime	; timer device

	XREF		InitMsgPort

	XREF		IDAddHandler
	XREF		IDRemHandler
	XREF		IDWriteEvent
	XREF		IDSetMPort
	XREF		IDSetMType
	XREF		IDSetMTrig

	XREF		GDName

*------ Exported Functions -------------------------------------------

	XDEF		IDTaskStart

	XDEF		DispatchEvents

	XDEF		ReopenMouse
	XDEF		AbortMouse
	XDEF		TypeMouse
	XDEF		TrigMouse
	XDEF		ReadMouse

	IFNE		TV_SIZE-8
	FAIL		"recode MOVEM statements that move time values"
	ENDC

*------ constants ----------------------------------------------------

HIGH_KEYCODE    EQU 	$71		;Max for 101 keyboard
					;See also kbdata.i

*------ input.device task --------------------------------------------
*
*   Handle the following signals:
*	PORT		A command has been posted to the message port
*	IEVENT		An I/O read has been satisfied
*
*---------------------------------------------------------------------
IDTaskStart:
	;------ Grab the arguments
		move.l	4(sp),a5		; input device
		move.l	dd_ExecBase(a5),a6

		lea	id_Unit(a5),a0
		bsr	InitMsgPort
		moveq	#0,d7
		bset	d0,d7			; save for signal mask
		move.l	d7,d6
		lea	id_IEPort(a5),a0
		move.l	a0,id_TIOR+MN_REPLYPORT(a5)
		move.l	a0,id_MIOR+MN_REPLYPORT(a5)
		move.l	a0,id_K1IOR+MN_REPLYPORT(a5)
		move.l	a0,id_K2IOR+MN_REPLYPORT(a5)
		move.l	a0,id_RIOR+MN_REPLYPORT(a5)
		bsr	InitMsgPort
		move.w	#TIMERCODE,id_TIOR+MN_LENGTH(a5)
		move.w	#MOUSECODE,id_MIOR+MN_LENGTH(a5)
		move.w	#KEYBOARDCODE,id_K1IOR+MN_LENGTH(a5)
		move.w	#KEYBOARDCODE,id_K2IOR+MN_LENGTH(a5)
		move.w	#REPEATCODE,id_RIOR+MN_LENGTH(a5)
		bset	d0,d7			; save for signal mask

	;------	indicate input.task ready to go
		move.l	id_Stk(a5),a1
		moveq	#SIGF_SINGLE,d0
		CALLEXE	Signal

	;------ Wait for someone to use input.task before setting up reads
		move.l	d7,d0
		CALLEXE Wait

		move.l	d0,d2			    ; save signals
		bsr.s	openGameport

*	    ;------ Fire up reads to the input devices
		bsr	readTimer
		bsr	readKeyboard
		bsr	readKeyboard
		bra	checkPort	; go read the port that started this

;---------------------------------------------------------------------

AbortMouse:
		tst.l	id_MIOR+IO_DEVICE(a5)
		beq.s	agRts

		lea	id_MIOR(a5),a1
		move.l	IO_DEVICE(a1),a6
		jsr	DEV_ABORTIO(a6)
		move.l	dd_ExecBase(a5),a6
		lea	id_MIOR(a5),a1
		CALLEXE	WaitIO
agRts:
		rts


ReopenMouse:
		tst.l	id_MIOR+IO_DEVICE(a5)
		beq.s	openGameport

		bsr.s	AbortMouse

		lea	id_MIOR(a5),a1
		CALLEXE	CloseDevice

openGameport:
	    ;------ open the GamePort device
		lea	GDName(pc),a0
		moveq	#0,d0
		move.b	id_MPort(a5),d0		; get game port
		moveq	#0,d1
		lea	id_MIOR(a5),a1
		CALLEXE OpenDevice
		tst.l	d0
		beq.s	ogSetAll

		clr.l	id_MIOR+IO_DEVICE(a5)

ogSetAll:
		bsr.s	TypeMouse
		bsr.s	TrigMouse
		bra	ReadMouse


TypeMouse:
		tst.l	id_MIOR+IO_DEVICE(a5)
		beq.s	tgRts
	    ;------ Select the Game Port controller type
		lea	id_MIOR(a5),a1
		move.w	#GPD_SETCTYPE,IO_COMMAND(a1)
		move.l	#1,IO_LENGTH(a1)
		lea	id_MType(a5),a0
		bra.s	doMouseIO
		rts

TrigMouse:
		tst.l	id_MIOR+IO_DEVICE(a5)
		beq.s	tgRts
	    ;------ Set the Game Port output trigger
		lea	id_MIOR(a5),a1
		move.w	#GPD_SETTRIGGER,IO_COMMAND(a1)
		move.l	#8,IO_LENGTH(a1)
		lea	id_MTrig(a5),a0

doMouseIO:
		move.l	a0,IO_DATA(a1)
		move.b	#IOF_QUICK,IO_FLAGS(a1)
		bsr	sendIO
		lea	id_MIOR(a5),a1
		CALLEXE	WaitIO

tgRts:
		rts


;---------------------------------------------------------------------

taskWait:
*	    ;------ Wait for something to do
		btst	#DUB_STOPPED,id_Unit+du_Flags(a5)
		beq.s	regularWait
	    ;------ Stopped: wait for start signal
		move.l	d6,d0
		CALLEXE Wait
		bra.s	continue

regularWait:
		move.l	d7,d0
		CALLEXE Wait

		btst	#DUB_STOPPED,id_Unit+du_Flags(a5)
		beq.s	continue
	    ;------ Stopped: ensure wakeup is for start signal only
		move.l	d0,d2		; save start signal
		and.l	d6,d2

		move.l	d7,d1		; set IE signal again if set
		eor.l	d6,d1
		move.l	d1,d3
		and.l	d0,d3
		beq.s	checkIE
		CALLEXE	SetSignal
		bra.s	checkIE

continue:
		move.l	d0,d2			    ; save signals

checkIE:
		move.b	id_IEPort+MP_SIGBIT(a5),d0
		ext.w	d0
		btst	d0,d2
		beq	checkPort
*	    ;------ See who it is
readIEPort:
		lea	id_IEPort(a5),a0
		CALLEXE	GetMsg
		tst.l	d0
		beq	checkPort		; no more messages
		move.l	d0,a0
		move.w	MN_LENGTH(a0),d0	; get message id
		jmp	dispatchMsg(pc,d0.w)


		;-- dispatch offset base
dispatchMsg:


*	    ;------ Dispatch Timer
gotTimer:
		lea	id_TData(a5),a3
		move.l	timerEventTemplate(pc),ie_Class(a3)
		clr.l	ie_X(a3)
		move.w	id_Qualifier(a5),ie_Qualifier(a3)
		bsr	DispatchEvents
		bsr	readTimer
		bra.s	readIEPort

*	    ;------ Dispatch Mouse
gotMouse:
		lea	id_MData(a5),a3
		move.w	id_MIOR+IO_ACTUAL+2(a5),d5

nextMouse:
		move.w	ie_Qualifier(a3),d0
		move.w	id_Qualifier(a5),d1
		move.w	d1,d3			; save old qualifiers
		andi.w	#ID_MOUSEMASK,d0
		andi.w	#(~ID_MOUSEMASK)&$FFFF,d1
		or.w	d0,d1
		move.w	d1,ie_Qualifier(a3)
		move.w	d1,id_Qualifier(a5)

		;------	avoid coalesce of first qualifier transition
		sub.w	#ie_SIZEOF,d5
		ble.s	dispatchMouse
		andi.w	#ID_MOUSEMASK,d3
		cmp.w	d0,d3
		bne.s	dispatchMouse

		;------ coalesce subsequent mouse events with the same qualifier
coalesceMouse:
		move.w	ie_Qualifier+ie_SIZEOF(a3),d1
		andi.w	#ID_MOUSEMASK,d1
		cmp.w	d0,d1
		bne.s	dispatchMouse

		add.w	#ie_SIZEOF,a3
		move.w	ie_X-ie_SIZEOF(a3),d3
		add.w	d3,ie_X(a3)
		move.w	ie_Y-ie_SIZEOF(a3),d3
		add.w	d3,ie_Y(a3)
		move.w	ie_Code-ie_SIZEOF(a3),ie_Code(a3)
		move.w	ie_Qualifier-ie_SIZEOF(a3),ie_Qualifier(a3)
		sub.w	#ie_SIZEOF,d5
		bgt.s	coalesceMouse

dispatchMouse:
		bsr	DispatchEvents
		add.w	#ie_SIZEOF,a3
		tst.w	d5
		bgt.s	nextMouse


*		;------ requeue mouse read
		bsr	ReadMouse
		bra	readIEPort

*	    ;------ Dispatch Key
gotKeyboard:
		lea	id_K1Data-id_K1IOR(a0),a3
		move.w	ie_Qualifier(a3),d0
		move.w	id_Qualifier(a5),d1
		andi.w	#ID_KEYMASK,d0
		andi.w	#~ID_KEYMASK,d1
		or.w	d0,d1
		move.w	d1,ie_Qualifier(a3)
		move.w	d1,id_Qualifier(a5)

		move.w	ie_Code(a3),d0
		bclr	#IECODEB_UP_PREFIX,d0
		bne.s	upKey

		;------	qualifier keys don't affect PrevKey or repeatability
		;
		;------ new 101 keys >= $6b
		;------ 0-5f  >> repeat
		;------ 60-6a >> no repeat
		;------ 6b-71 >> repeat
		;------ 72-7f >> no repeat (101 keys won't repeat since
                ;                they don't repeat under 1.3.


		cmp.w	#START_QUAL,d0		;if < $60; repeat
		blt.s	doPrevKey
		cmp.w	#HIGH_KEYCODE,d0	;if > $71; no repeat
		bhi.s	dispatchKey
		cmp.w	#END_QUAL,d0		;if <= $6a; no repeat
		bls.s	dispatchKey

doPrevKey:
		;------ save this down key in PrevKey variable
		move.l	id_Prev1Down(a5),ie_EventAddress(a3)
		move.w	id_Prev1Down(a5),id_Prev2Down(a5)
		move.b	d0,id_Prev1DownCode(a5)
		move.b	d1,id_Prev1DownQual(a5)

		;------ check for previous repeat key candidate
		tst.w	id_RepeatCode(a5)
		bmi.s	cacheRepeat
		bsr	abortRepeat
cacheRepeat:
		;------	post this key for repeating
		move.w	ie_Code(a3),id_RepeatCode(a5)
		move.w	ie_Qualifier(a3),d0
		and.w	#IEQUALIFIER_NUMERICPAD,d0
		or.w	#IEQUALIFIER_REPEAT,d0
		move.w	d0,id_RepeatNumeric(a5)
		bsr	postThresh
		bra.s	dispatchKey

upKey:
		cmp.w	id_RepeatCode(a5),d0
		bne.s	dispatchKey

		;------	up report for repeat code, abort the repeat
		move.w	#-1,id_RepeatCode(a5)
		bsr	abortRepeat

dispatchKey:
		bsr	DispatchEvents
*		;------ requeue keyboard read
		bsr	readKeyboard
		bra	readIEPort


	;--- Dispatch Repeat
gotRepeat:
		move.w	id_RepeatCode(a5),d0
		bmi	readIEPort	; no repeatable key pending
		lea	id_RepeatKey(a5),a3
		move.w	#(IECLASS_RAWKEY<<8),ie_Class(a3)	; & ie_SubClass
		move.w	d0,ie_Code(a3)
		move.w	id_Qualifier(a5),d0
		and.w	#~IEQUALIFIER_NUMERICPAD,d0
		or.w	id_RepeatNumeric(a5),d0
		move.w	d0,ie_Qualifier(a3)
		move.l	id_Prev1Down(a5),ie_EventAddress(a3)
		bsr	DispatchEvents
*		;------ requeue repeat key
		bsr	postPeriod
		bra	readIEPort


checkPort:
	    ;------ check if a command is at the command port
		move.l	id_Unit+MP_MSGLIST(a5),a1
		tst.l	(a1)
		beq	taskWait

		move.w	IO_COMMAND(a1),d0
		bclr    #IOB_QUICK,IO_FLAGS(a1)	; jimm: 8/1/86
		sub.w	#IND_ADDHANDLER,d0
		lsl.w	#2,d0
		lea	cmdVectors(pc),a0
		move.l	0(a0,d0.w),a0
		jsr	(a0)
		bra.s	checkPort


cmdVectors:
		dc.l	IDAddHandler
		dc.l	IDRemHandler
		dc.l	IDWriteEvent
		dc.l	0
		dc.l	0
		dc.l	IDSetMPort
		dc.l	IDSetMType
		dc.l	IDSetMTrig

timerEventTemplate:
		dc.b	IECLASS_TIMER
		dc.b	0
		dc.w	0


*------ readTimer -----------------------------------------------------
readTimer:
		lea	id_TIOR(a5),a1
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		clr.b	IO_FLAGS(a1)
		move.l	#ID_TEVENTSECS,IOTV_TIME+TV_SECS(a1)
		move.l	#ID_TEVENTMICRO,IOTV_TIME+TV_MICRO(a1)
		bra.s	sendIO


*------ ReadMouse -----------------------------------------------------
ReadMouse:
		tst.l	id_MIOR+IO_DEVICE(a5)
		beq.s	sRts
		lea	id_MIOR(a5),a1
		move.w	#GPD_READEVENT,IO_COMMAND(a1)
		clr.b	IO_FLAGS(a1)
		move.l	#ie_SIZEOF*MOUSEAHEAD,IO_LENGTH(a1)
		lea	id_MData(a5),a0
		move.l	a0,IO_DATA(a1)
		bra.s	sendIO


*------ readKeyboard --------------------------------------------------
readKeyboard:
		lea	id_K1IOR(a5),a1
		moveq	#id_K2IOR-id_K1IOR,d0
		and.b	id_KRActiveMask(a5),d0
		not.b	id_KRActiveMask(a5)
		add.w	d0,a1
		move.w	#KBD_READEVENT,IO_COMMAND(a1)
		clr.b	IO_FLAGS(a1)
		move.l	#ie_SIZEOF,IO_LENGTH(a1)
		lea	id_K1Data-id_K1IOR(a1),a0
		move.l	a0,IO_DATA(a1)
sendIO:
		move.l	IO_DEVICE(a1),a6
		jsr	DEV_BEGINIO(a6)
		move.l	dd_ExecBase(a5),a6
sRts:
		rts


*------ postThresh ----------------------------------------------------
postThresh:
		lea	id_RIOR(a5),a1
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		clr.b	IO_FLAGS(a1)
		move.l	id_Thresh+TV_SECS(a5),IOTV_TIME+TV_SECS(a1)
		move.l	id_Thresh+TV_MICRO(a5),IOTV_TIME+TV_MICRO(a1)
		bra.s	sendIO


*------ postPeriod ----------------------------------------------------
postPeriod:
		lea	id_RIOR(a5),a1
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		clr.b	IO_FLAGS(a1)
		move.l	id_Period+TV_SECS(a5),IOTV_TIME+TV_SECS(a1)
		move.l	id_Period+TV_MICRO(a5),IOTV_TIME+TV_MICRO(a1)
		bra.s	sendIO


*------ abortRepeat ---------------------------------------------------
abortRepeat:
		lea	id_RIOR(a5),a1
		move.l	IO_DEVICE(a1),a6
		jsr	DEV_ABORTIO(a6)
		move.l	dd_ExecBase(a5),a6
		lea	id_RIOR(a5),a1
		CALLEXE	WaitIO
		rts


*------ DispatchEvents ------------------------------------------------
DispatchEvents:
		move.l	a6,-(a7)
		lea	ie_TimeStamp(a3),a0
		move.l	id_RIOR+IO_DEVICE(a5),a6
		jsr	_LVOGetSysTime(a6)
		move.l	(a7)+,a6
		move.l	a3,a0
		clr.l	ie_NextEvent(a3)
		move.l	id_HandlerList(a5),a4
handlerLoop:
		tst.l	(a4)
		beq.s	dRts
		move.l	IS_CODE(a4),a2
		move.l	IS_DATA(a4),a1
		move.l	(a4),a4
		jsr	(a2)
		tst.l	d0		; no sense propagating nothing
		beq.s	dRts
		move.l	d0,a0
		bra.s	handlerLoop
dRts:
		rts

TIMERCODE	EQU	gotTimer-dispatchMsg
MOUSECODE	EQU	gotMouse-dispatchMsg
KEYBOARDCODE	EQU	gotKeyboard-dispatchMsg
REPEATCODE	EQU	gotRepeat-dispatchMsg

	END
