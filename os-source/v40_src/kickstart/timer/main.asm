*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: main.asm,v 39.2 92/07/29 10:08:10 darren Exp $
*
* $Log:	main.asm,v $
* Revision 39.2  92/07/29  10:08:10  darren
* Use REALLY_PALn bit to test for PAL/NTSC machine -- new for
* V39 such that the PALn bit is set if selected via BootMenu, but
* the REALLY_PALn bit tells me the truth for eclock speed, even
* though it may also be incorrect if the machine is jumpered funny.
* Nothing we can do about that
* 
* Revision 39.1  92/04/20  09:37:14  mks
* Changed to use TaggedOpenLibrary()
* 
* Revision 39.0  92/01/20  13:09:08  mks
* Timer device now calls battclock to read the base system time.
* Also made some branches short...
*
*************************************************************************

	SECTION	timer

*------ Included Files ***********************************************

	INCLUDE	'hardware/intbits.i'
	INCLUDE	'hardware/cia.i'
	INCLUDE	'resources/cia.i'

	INCLUDE	'exec/types.i'
	INCLUDE	'exec/ables.i'
	INCLUDE	'exec/initializers.i'
	INCLUDE	'exec/errors.i'
	INCLUDE	'exec/alerts.i'
	INCLUDE	'graphics/gfxbase.i'

	INCLUDE	'internal/librarytags.i'

	INCLUDE	'asmsupp.i'
	INCLUDE	'timer.i'
	INCLUDE 'macros.i'
	INCLUDE 'constants.i'
	INCLUDE 'internal.i'
	INCLUDE	'V:src/kickstart/cia/internal.i'

	INCLUDE 'debug.i'

*------ Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	VERNUM
	XREF	REVNUM
	XREF	VERSTRING
	XREF	timerName
	XREF	ciaName
	XREF	BattClockName

	XREF	AddReqUnit0
	XREF	AddReqUnit1
	XREF	AddReqUnit2
	XREF	AddReqUnit3
	XREF	AddReqUnit4
	XREF	RemReqUnit0
	XREF	RemReqUnit1
	XREF	RemReqUnit2
	XREF	RemReqUnit3
	XREF	RemReqUnit4
	XREF	Tick0
	XREF	Tick1
	XREF	Tick2
	XREF	NewTimer
	XREF	RemTimer
	XREF	SetSysTimeIO
	XREF	GetSysTimeIO
	XREF	TimAdd
	XREF	TimSub
	XREF	TimCmp
	XREF	TOD_Int
	XREF	EClock_Int
	XREF	ReadETime
	XREF	GetSysTime

	XREF	Req2STTOD
	XREF	STTOD2Req
	XREF	GetSTTOD
	XREF	EIncSTTOD
	XREF	TODIncSTTOD
	XREF	AdjSTTOD

	XREF	Req2STEClock
	XREF	STEClock2Req
	XREF	GetSTEClock
	XREF	EIncSTEClock
	XREF	TODIncSTEClock
	XREF	AdjSTEClock

	XREF	_ciaa
	XREF	_ciab
	XREF	_intena

	XREF	SwapCIA

	XREF	KPutStr

	INT_ABLES

*------ Functions ----------------------------------------------------

	XDEF	Init
	XDEF	initStruct
	XDEF	Null
	XDEF	NoIO
	XDEF	BeginIO
	XDEF	TermIO
	XDEF	TermIOC
	XDEF	Open
	XDEF	Close

*------ System Library Functions -------------------------------------

	XSYS	TaggedOpenLibrary
	XSYS	CloseLibrary
	XSYS	CacheClearU
	XSYS	OpenResource
	XSYS	MakeLibrary
	XSYS	AddIntServer
	XSYS	ReplyMsg
	XSYS	OpenDevice
	XSYS	SendIO
	XSYS	AddDevice
	XSYS	Alert
	XSYS	Debug

	XSYS	AddICRVector
	XSYS	RemICRVector
	XSYS	SetICR

	XSYS	ReadBattClock

***#*** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

*------ Data ---------------------------------------------------------

***#*** Local Definitions ********************************************

	DS.W	0

V_DEF	MACRO
	dc.W	\1+(*-devFuncInit)
	ENDM

devFuncInit:
	dc.w	-1
	V_DEF	Open		; - 6	Open
	V_DEF	Close		; - C	Close
	V_DEF	Null		; -12	Expunge
	V_DEF	Null		; -18	reserved
	V_DEF	BeginIO		; -1E	begin
	V_DEF	RemTimer	; -24   abort
	V_DEF	TimAdd		; -2A
	V_DEF	TimSub		; -30
	V_DEF	TimCmp		; -36
	V_DEF	ReadETime	; -3C
	V_DEF	GetSysTime	; -42
	dc.w	-1		; end of table

******* timer.device/--background-- **********************************
*
*   TIMER REQUEST
*	A time request is a non standard IO Request.  It has an IORequest
*	followed by a timeval structure or an eclockval structure.
*
*   TIMEVAL
*	A timeval structure consists of two longwords.  The first is
*	the number of seconds, the latter is the fractional number
*	of microseconds.  The microseconds must always be "normalized"
*	e.g. the longword must be between 0 and one million.
*
*   ECLOCKVAL
*	A eclockval structure consists of two longwords.  The first is
*	the high order 32 bits of a 64 bit number and the second is the
*	the low order 32 bits.  The 64 bit number is a count of "E" clock
*	ticks.  The "E" clock frequency is related to the master clock
*	frequency of the machine and can be determined by calling the
*	ReadEClock() library like call.
*
*   UNITS
*	The timer contains five units -- two designed to accuratly measure
*	short intervals, one that has little system overhead and is very
*	stable over time, and two that work like an alarm clock.
*
*   UNIT_MICROHZ
*	This unit uses the programmable timers in the 8520s to keep track
*	of its time.  It has precision down to about 2 microseconds, but
*	will drift as system load increases.  The accuracy of this unit
*	is the same as that of the master clock of the machine.  This unit
*	uses a timeval in its timerequest.
*
*   UNIT_VBLANK
*	This unit uses a strobe from the power supply to keep track of its
*	time or the "E" clock on machines without power supply strobes.
*	It is very stable over time, but only has a resolution of that of
*	the vertical blank interrupt.  This unit is very cheap to use, and
*	should be used by those who are waiting for long periods of time
*	(typically 1/2 second or more).  This unit uses a timeval in its
*	timerequest.
*
*   UNIT_ECLOCK
*	This unit is exacly the same as UNIT_MICROHZ except that it uses
*	an eclockval instead of a timeval in its timerequest.
*
*   UNIT_WAITUNTIL
*	This unit waits until the systime is greater than or equal to the
*	time in the timeval in the timerequest.  This unit has the same
*	resolution and accuracy as that of UNIT_VBLANK.
*
*   UNIT_WAITECLOCK
*	This unit waits until the E-Clock value as returned by ReadEClock()
*	is greater than or equal to the eclockval in the timerequest. This
*	unit has the same resolution and accuracy as that of UNIT_ECLOCK.
*
*   LIBRARY
*	In addition to the normal device calls, the timer also supports
*	several direct, library like calls.
*
*   BUGS
*	In the V1.2/V1.3 release, the timer device has problems with
*	very short time requests.  When one of these is made, other
*	timer requests may be finished inaccurately.  A side effect
*	is that AmigaDOS requests such as "Delay(0);" or
*	"WaitForChar(x,0);" are unreliable.
*
**********************************************************************



*****i* timer.device/internal/Init ***********************************
*
*   NAME
*	Init -- Initialize the timer from nothing.
*
*   SYNOPSIS
*	Init()
*
*	void Init( );
*
*   FUNCTION
*	This routine initializes the timer from nothing.
*
*   INPUTS
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	OpenResource() must not fail for either ciaa or ciab
*
*   REGISTER USAGE
*
*	A2 -- timer device pointer
*

Init:
*$* debug
*		movem.l	d0-d7/a0-a6,-(sp)
*		CALLSYS	Debug
*		movem.l	(sp)+,d0-d7/a0-a6
*$* debug
		MOVEM.L	A2/A3/D2,-(SP)

		LEA	devFuncInit,A0
		LEA	initStruct,A1
		SUBA.L	A2,A2
		MOVE.L	#TD_SIZE,D0
		CALLSYS	MakeLibrary		; make us a playpen

		TST.L	D0
		BNE.S	Init_MakeLibOK

		move.l	#(AN_TimerDev!AG_MakeLib),d0
		bsr	T_Alert
		BRA	Init_End

Init_MakeLibOK:
		;------ nothing can go wrong from here on...
		MOVE.L	D0,A2
		MOVE.L	A6,TD_SYSLIB(A2)

; OpenResource will NOT fail at boot time

		LEA	ciaName,A1
		CALLSYS	OpenResource

		move.l	d0,TD_COUNTERRESOURCE(a2)
		move.l	d0,TD_TODRESOURCE(a2)

***********************************************************************
*
* Install call back hook for CIA Alloc/Rem interrupt vector.  Note that
* cia.resource has to be init before the timer.device anyway, and that
* the call back hook is FASTER, and SMALLER than OpenDevice() of the
* the timer.device.  Plus it completely hides the call-back routine.
*
* We also have a call-back hook which is a ptr to timer.device's
* microhz interrupt code.
*
		movea.l	d0,a3			;cache in A3
		move.l	CR_SharedData(a3),a0

		lea	SwapCIA(pc),a1
		move.l	a1,cs_TimerCall(a0)	;cache ptr to hook
		move.l	a2,cs_TimerBase(a0)	;and timerbase

		lea	Tick0(pc),a1
		move.l	a1,cs_TimerCode(a0)	; ptr to interrupt code

		;------ initialize units 0,2 & 4 delay lists
		LEA	TD_ECLOCKLIST(A2),A0
		move.l	a0,TD_UNIT0+TU_UNITLIST(a2)
		move.l	a0,TD_UNIT2+TU_UNITLIST(a2)
		move.l	a0,TD_UNIT4+TU_UNITLIST(a2)
		NEWLIST	A0

		;------ initialize unit 1 delay list
		LEA	TD_VBLANKLIST(A2),A0
		move.l	a0,TD_UNIT1+TU_UNITLIST(a2)
		NEWLIST	A0

		;------ initialize unit 3 wait list
		LEA	TD_SYSTIMELIST(A2),A0
		move.l	a0,TD_UNIT3+TU_UNITLIST(a2)
		NEWLIST	A0

		;------ initializer the TermIO queue
		LEA	TD_TERMIOQ(A2),A0
		NEWLIST	A0

		DISABLE

		;------ set up VBlank interrupt
		LEA	TD_VBLANKINT(A2),A1
		MOVE.L	A2,IS_DATA(A1)
		MOVEQ	#INTB_VERTB,D0
		CALLSYS	AddIntServer

; AddICRVector can NOT fail at boot time

		LEA	TD_TODINT(A2),A1
		MOVE.L	A2,IS_DATA(A1)
		MOVE.B	#CIAICRB_ALRM,D0

		LINKLIB	_LVOAddICRVector,a3	;faster via A3
**		LINKLIB	_LVOAddICRVector,TD_TODRESOURCE(A2)

		LEA	TD_CIATJUMP(A2),A1
		MOVE.L	A2,IS_DATA(A1)
		MOVE.B	#CIAICRB_TA,D0

		LINKLIB	_LVOAddICRVector,a3	;faster via A3
**		LINKLIB	_LVOAddICRVector,TD_COUNTERRESOURCE(A2)

		LEA	TD_CIATBINT(A2),A1
		MOVE.L	A2,IS_DATA(A1)
		MOVE.B	#CIAICRB_TB,D0

		LINKLIB	_LVOAddICRVector,a3	;faster via A3
**		LINKLIB	_LVOAddICRVector,TD_COUNTERRESOURCE(A2)

		bsr	SetTODFrequency

; need to check for graphics magic before proceeding
		moveq.l	#OLTAG_GRAPHICS,d0	; Tag for graphics.library
		CALLSYS	TaggedOpenLibrary

*$* not needed in production
*$*		tst.l	d0
*$*		beq	ERROR
*$* not needed in production

		move.l	d0,a1
		moveq.l	#0,d2
		move.w	gb_DisplayFlags(a1),d2
		CALLSYS	CloseLibrary

	;; test prior to define in V39 gfxbase.i

	IFND	REALLY_PALn
REALLY_PALn	EQU	4
	ENDC

		btst.l	#REALLY_PALn,d2
		beq.s	NTSCxtal	; NTSC xtal constants default
		move.l	#709379,TD_ECLOCKHERTZ(a2)
		move.l	#46490,TD_ECLOCKCONST1(a2)
		move.l	#92385,TD_ECLOCKCONST2(a2)
NTSCxtal:

		; initialize mode specific systime vectors
		moveq.l	#5,d0			; do body 6 times
		lea	Req2STfmt(a2),a0	; dest

		btst.l	#TODA_SAFEn,d2
		bne.s	SetTOD

SetEClock:
		lea	AReq2STEClock(a2),a1
		bra.s	STfmtLoop

SetTOD:
		lea	AReq2STTOD(a2),a1

STfmtLoop:
		addq.l	#2,a0
		move.l	(a1)+,(a0)+
		dbf	d0,STfmtLoop


Init_Finished:

		move.l	TD_ECLOCKHERTZ(a2),ex_EClockFrequency(a6)
		move.b	1+TD_TODHERTZ(A2),PowerSupplyFrequency(A6)

; start the counters

;	TOD
		lea	_ciaa,a0

		bset.b	#CIACRBB_ALARM,ciacrb(a0)
		move.b	#0,ciatodhi(a0)
		move.b	#$10,ciatodmid(a0)
		move.b	#0,ciatodlow(a0)
		bclr.b	#CIACRBB_ALARM,ciacrb(a0)
		move.b	#0,ciatodhi(a0)
		move.b	#0,ciatodmid(a0)
		move.b	#0,ciatodlow(a0)

;	EClock

***		lea	_ciaa,a0 	;redundant code

		andi.b	#CIACRBF_PBON+CIACRBF_OUTMODE,ciacrb(a0)
		move.b	#$ff,ciatblo(a0)
		move.b	#$ff,ciatbhi(a0)
		ori.b	#CIACRBF_LOAD+CIACRBF_START,ciacrb(a0)

; ------ Clear the cache (we just made a jump table)
; ------ clear before enabling interrupts

		CALLSYS	CacheClearU


; ------ Now, read the current time from BattClock (if it is there)
		lea	BattClockName,a1	; find the the clock
		CALLSYS	OpenResource
		tst.l	d0
		beq.s	Init_NoTime	; No battclock...
		move.l	d0,a3		; battclockLib base
		exg	a3,a6		; Get it into a6 (a3 is now exec)
		CALLSYS	ReadBattClock	; Get the time
*
* Build fake timeval...
*
		clr.l	-(sp)		; Put a 0 micros onto the stack...
		move.l	d0,-(sp)	; Put the seconds onto the stack...
		move.l	sp,a0		; Pointer to the timeval...
*
		move.l	a2,a6		; Get my device base...
		jsr	Req2STfmt(a6)	; convert format	(no trash a0)
		jsr	AdjSTfmt(a6)	; adjust for type of clock
		move.l	a3,a6		; Get back original execbase...
*
* Take results and set the time in our timerbase structure...
*
		move.l	(sp)+,TD_SYSTIME+TV_SECS(a2)
		move.l	(sp)+,TD_SYSTIME+TV_MICRO(a2)
*
Init_NoTime:

; ------  Now, finally, enable...

		ENABLE

; ------ tell the world we are here
		MOVE.L	A2,A1
		CALLSYS	AddDevice

Init_End:
		MOVEM.L	(SP)+,A2/A3/D2
		RTS


T_Alert:
*$* debug
*		ERRMSG	"T_Alert"
*$* debug
		movem.l	d7/a5/a6,-(sp)
		move.l	d0,d7
		move.l	4,a6
		jsr	_LVOAlert(a6)
		movem.l	(sp)+,d7/a5/a6
		rts


SetTODFrequency:
*$* debug
*		ERRMSG	"SetTODFrequency"
*$* debug
		lea	_ciaa,a0
		lea	_ciaa,a1

		bclr.b	#CIACRBB_ALARM,ciacrb(a1)
		move.b	#0,ciatodhi(a1)
		move.b	#0,ciatodmid(a1)
		move.b	#0,ciatodlow(a1)

		;------ now check for the power supply frequency
		MOVE.B	#CIACRBF_RUNMODE,ciacrb(A0)
		MOVE.B	#$FF,ciatblo(A0)
		MOVE.B	#$FF,ciatbhi(A0)

		;------ here is how we are going to tell the time in
		;------ a processor independent fashion: we will count
		;------ the time between two ticks of the tod register


		move.b	ciatodlow(a1),d0
waitTODl1:
		tst.b	ciatbhi(a0)
		beq.s	NoTOD
		cmp.b	ciatodlow(a1),d0
		beq.s	waitTODl1
waitTODe1:

		MOVE.B	#CIACRBF_RUNMODE!CIACRBF_START!CIACRBF_LOAD,ciacrb(A0)

		move.b	ciatodlow(a1),d0
waitTODl2:
		tst.b	ciatbhi(a0)
		beq.s	NoTOD
		cmp.b	ciatodlow(a1),d0
		beq.s	waitTODl2
waitTODe2:

		MOVE.B	#CIACRBF_RUNMODE,ciacrb(A0)

		;------ look at the count value and make a guess
		;------ at the power supply value
		CLEAR	D0
		MOVE.B	ciatbhi(A0),D0
		LSL.L	#8,D0
		MOVE.B	ciatblo(A0),D0

		;------	Hertz	NTSC	PAL	count	test
		;------ 60	11932	11923	13060	$ccfc
		;------ 50	14318	14188	18982	$b5da
		;------ 30	23864	23646	26120	$99f8
		;------ 25	28636	28375


Try60Hz:
		cmp.w	#$ccfc,d0
		bcs.s	Try50Hz
		move.w	#60,d0
		move.w	#16667,d1
		bra.s	StuffTOD
Try50Hz:
		cmp.w	#$b5da,d0
		bcs.s	Try30Hz
		move.w	#50,d0
		move.w	#20000,d1
		bra.s	StuffTOD
Try30Hz:
		cmp.w	#$99f8,d0
		bcs.s	Try25Hz
		move.w	#30,d0
		move.w	#33333,d1
		bra.s	StuffTOD
Try25Hz:
		move.w	#25,d0
		move.w	#40000,d1

StuffTOD:
		move.w	d0,TD_TODHERTZ(a2)
		move.w	d1,TD_TIMEPERTOD(a2)
		rts


NoTOD:		bra	T_Alert	; Tail recursion


initStruct:
		INITBYTE	LN_TYPE,NT_DEVICE
		INITLONG	LN_NAME,timerName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERNUM
		INITWORD	LIB_REVISION,REVNUM
		INITLONG	LIB_IDSTRING,VERSTRING


		INITLONG	TD_UNIT0+TU_ADDREQ,AddReqUnit0
		INITLONG	TD_UNIT0+TU_REMREQ,RemReqUnit0

		INITLONG	TD_UNIT1+TU_ADDREQ,AddReqUnit1
		INITLONG	TD_UNIT1+TU_REMREQ,RemReqUnit1

		INITLONG	TD_UNIT2+TU_ADDREQ,AddReqUnit2
		INITLONG	TD_UNIT2+TU_REMREQ,RemReqUnit2

		INITLONG	TD_UNIT3+TU_ADDREQ,AddReqUnit3
		INITLONG	TD_UNIT3+TU_REMREQ,RemReqUnit3

		INITLONG	TD_UNIT4+TU_ADDREQ,AddReqUnit4
		INITLONG	TD_UNIT4+TU_REMREQ,RemReqUnit4


		INITWORD	Req2STfmt+TJ_OPCODE,$4ef9
		INITWORD	STfmt2Req+TJ_OPCODE,$4ef9
		INITWORD	GetSTfmt+TJ_OPCODE,$4ef9
		INITWORD	EIncSTfmt+TJ_OPCODE,$4ef9
		INITWORD	TODIncSTfmt+TJ_OPCODE,$4ef9
		INITWORD	AdjSTfmt+TJ_OPCODE,$4ef9

		INITLONG	AReq2STTOD,Req2STTOD
		INITLONG	ASTTOD2Req,STTOD2Req
		INITLONG	AGetSTTOD,GetSTTOD
		INITLONG	AEIncSTTOD,EIncSTTOD
		INITLONG	ATODIncSTTOD,TODIncSTTOD
		INITLONG	AAdjSTTOD,AdjSTTOD

		INITLONG	AReq2STEClock,Req2STEClock
		INITLONG	ASTEClock2Req,STEClock2Req
		INITLONG	AGetSTEClock,GetSTEClock
		INITLONG	AEIncSTEClock,EIncSTEClock
		INITLONG	ATODIncSTEClock,TODIncSTEClock
		INITLONG	AAdjSTEClock,AdjSTEClock


* constants for NTSC systems
		INITLONG	TD_ECLOCKHERTZ,715909
		INITLONG	TD_ECLOCKCONST1,46918	; (Ehz*64K)/10^6
		INITLONG	TD_ECLOCKCONST2,91542	; (64K*10^6)/Ehz


* constants for PAL systems
*		INITLONG	TD_ECLOCKHERTZ,709379
*		INITLONG	TD_ECLOCKCONST1,46490	; (Ehz*64K)/10^6
*		INITLONG	TD_ECLOCKCONST2,92385	; (64K*10^6)/Ehz

* constants for 60Hz systems
*		INITWORD	TD_TODHERTZ,60
*		INITWORD	TD_TIMEPERTOD,16667

* constants for 50Hz systems
*		INITWORD	TD_TODHERTZ,50
*		INITWORD	TD_TIMEPERTOD,20000

* constants for 30Hz systems
*		INITWORD	TD_TODHERTZ,30
*		INITWORD	TD_TIMEPERTOD,33333

* constants for 25Hz systems
*		INITWORD	TD_TODHERTZ,25
*		INITWORD	TD_TIMEPERTOD,40000


		INITBYTE	TD_TODINT+LN_TYPE,NT_INTERRUPT
		INITLONG	TD_TODINT+LN_NAME,timerName
		INITLONG	TD_TODINT+IS_CODE,TOD_Int

		INITBYTE	TD_VBLANKINT+LN_TYPE,NT_INTERRUPT
		INITLONG	TD_VBLANKINT+LN_NAME,timerName
		INITLONG	TD_VBLANKINT+IS_CODE,Tick1

		INITBYTE	TD_CIATBINT+LN_TYPE,NT_INTERRUPT
		INITLONG	TD_CIATBINT+LN_NAME,timerName
		INITLONG	TD_CIATBINT+IS_CODE,EClock_Int

		INITBYTE	TD_CIATJUMP+LN_TYPE,NT_INTERRUPT
		INITLONG	TD_CIATJUMP+LN_NAME,timerName
		INITLONG	TD_CIATJUMP+IS_CODE,Tick0

		INITLONG	TD_JUMPYCIACRX,(_ciaa+ciacra)
		INITLONG	TD_JUMPYCIATXLO,(_ciaa+ciatalo)

		DC.L		0

*****i* timer.device/function/BeginIO ********************************
*
*   NAME
*	BeginIO -- Perform a device command.
*
*   SYNOPSIS
*	BeginIO( IORequest )
*	         A1
*
*	void BeginIO( struct timerequest * );
*
*   FUNCTION
*	This routine is the external entry point for device commands.
*
*   INPUTS
*	IORequest -- the timer request block.
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

BeginIO:
*$* debug
*		ERRMSG	"BeginIO"
*$* debug
		move.b	#0,IO_ERROR(a1)		; clear error

		cmp.b	#NT_MESSAGE,LN_TYPE(a1)
		bne.s	NotAgain
		move.l	a1,-(sp)
		bsr	RemTimer
		move.l	(sp)+,a1
		tst.l	d0
		bne.s	NotAgain
		rts

NotAgain
		move.w	IO_COMMAND(a1),d0

		subi.w	#TR_ADDREQUEST,d0	; validate command
		bmi.s	NoIO
		cmp.w	#(TR_SETSYSTIME-TR_ADDREQUEST),d0
		bhi.s	NoIO

		;------ the following line is needed for WaitIO
		move.b	#NT_MESSAGE,LN_TYPE(a1)

		;------ get the function address
		add.w	d0,d0
		add.w	d0,d0	; d0*4
		jmp	cmdTable(pc,d0.w)

cmdTable:
	bra.w	NewTimer	; TR_ADDTIMER
	bra.w	GetSysTimeIO	; TR_GETSYSTIME
	bra.w	SetSysTimeIO	; TR_SETSYSTIME
cmdTableEnd:


*****i* timer.device/function/Open ***********************************
*
*   NAME
*	Open -- Open the timer for use.
*
*   SYNOPSIS
*	Open( Unit, Flags, IORequest )
*	      D0    D1     A1
*
*	void Open( LONG, LONG, struct timerequest * );
*
*   FUNCTION
*	This routine opens the timer up for use and initializes the
*	IORequest with the correct unit information.
*
*   INPUTS
*	Unit -- the unit number -- either UNIT_VBLANK or UNIT_MICROHZ
*	Flags -- ignored
*	IORequest -- a pointer to the users timerequest block.
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

Open:
		TST.L	D0
		BNE.S	Open_Not0
*$* debug
*		ERRMSG	"Open0"
*$* debug
		LEA	TD_UNIT0(A6),A0
		BRA.S	Open_OK

Open_Not0:
		SUBQ.L	#1,D0
		BNE.S	Open_Not1
*$* debug
*		ERRMSG	"Open1"
*$* debug
		LEA	TD_UNIT1(A6),A0
		BRA.S	Open_OK

Open_Not1:
		SUBQ.L	#1,D0
		BNE.S	Open_Not2
*$* debug
*		ERRMSG	"Open2"
*$* debug
		LEA	TD_UNIT2(A6),A0
		BRA.S	Open_OK

Open_Not2:
		SUBQ.L	#1,D0
		BNE.S	Open_Not3
*$* debug
*		ERRMSG	"Open3"
*$* debug
		LEA	TD_UNIT3(A6),A0
		BRA.S	Open_OK

Open_Not3:
		SUBQ.L	#1,D0
		BNE.S	Open_Err
*$* debug
*		ERRMSG	"Open4"
*$* debug
		LEA	TD_UNIT4(A6),A0

Open_OK:
		MOVE.L	A0,IO_UNIT(A1)
		ADDQ.W	#1,LIB_OPENCNT(A6)
		RTS

Open_Err:
		MOVE.B	#IOERR_OPENFAIL,IO_ERROR(A1)
		bra.s	CloseOut
		; RFT

*****i* timer.device/function/Close **********************************
*
*   NAME
*	Close - Close the device.
*
*   SYNOPSIS
*	Close( IORequest )
*	       A1
*
*	void Close( struct timerequest * );
*
*   FUNCTION
*	This routine closes the device.
*
*   INPUTS
*	IORequest -- an timerequest block for this unit.
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

Close:
*$* debug
*		ERRMSG	"Close"
*$* debug
		SUBQ.W	#1,LIB_OPENCNT(A6)
CloseOut:
*$* debug
*		ERRMSG	"CloseOut"
*$* debug
		MOVEQ	#-1,D0		; invalidate the request
		MOVE.L	D0,IO_DEVICE(A1)
		MOVE.L	D0,IO_UNIT(A1)

		; fall into Null
		; RFT


*****i* timer.device/function/Null ***********************************
*
*   NAME
*	Null -- Do nothing
*
*   SYNOPSIS
*	null = Null()
*	D0
*
*	LONG Null( );
*
*   FUNCTION
*	This routine returns null.
*
*   INPUTS
*
*   RESULTS
*	null -- always zero.
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

Null:
*$* debug
*		ERRMSG	"Null"
*$* debug
		CLEAR	D0
		RTS


*****i* timer.device/internal/NoIO ***********************************
*
*   NAME
*	NoIO -- Return an IO request error.
*
*   SYNOPSIS
*	NoIO( IORequest )
*	      A1
*
*	void NoIO( struct timerequest * );
*
*   FUNCTION
*	This routine is a dummy entry for IO request blocks.  It always
*	returns an error.
*
*   INPUTS
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

NoIO:
*$* debug
*		ERRMSG	"NoIO"
*$* debug
		MOVE.B	#IOERR_NOCMD,IO_ERROR(A1)

		; fall through
		; RFT


*****i* timer.device/internal/TermIO *********************************
*
*   NAME
*	TermIO - Reply an IO Request block
*
*   SYNOPSIS
*	TermIO( IORequest )
*	        A1
*
*	void TermIO( struct timerequest * );
*	void TermIOC( struct timerequest * );
*
*   FUNCTION
*
*   INPUTS
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

TermIOC:
*$* debug
*		ERRMSG	"TermIOC"
*$* debug
		moveq.l	#0,d0
		move.l	d0,IOTV_TIME+TV_SECS(a1)	; don't leave
		move.l	d0,IOTV_TIME+TV_MICRO(a1)	; anything

TermIO:
*$* debug
*		ERRMSG	"TermIO"
*$* debug
		BTST	#IOB_QUICK,IO_FLAGS(A1)
		BNE.S	TermIO_End

		LINKSYS	ReplyMsg
TermIO_End:
		RTS



	END
