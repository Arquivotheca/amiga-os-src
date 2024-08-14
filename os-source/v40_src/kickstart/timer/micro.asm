*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: micro.asm,v 39.0 92/01/20 13:10:34 mks Exp $
*
* $Log:	micro.asm,v $
* Revision 39.0  92/01/20  13:10:34  mks
* Timer device now calls battclock to read the base system time.
* Also made some branches short...
* 
*************************************************************************

	SECTION	timer

***#*** Included Files ***********************************************

	INCLUDE	'hardware/intbits.i'
	INCLUDE	'hardware/cia.i'
	INCLUDE	'resources/cia.i'

	INCLUDE	'exec/types.i'
	INCLUDE	'exec/ables.i'
	INCLUDE 'exec/interrupts.i'

	INCLUDE	'asmsupp.i'
	INCLUDE	'timer.i'
	INCLUDE 'macros.i'
	INCLUDE 'constants.i'
	INCLUDE 'internal.i'
	INCLUDE 'V:src/kickstart/cia/internal.i'
	INCLUDE 'debug.i'

***#*** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	_ciaa
	XREF	_ciab
	XREF	_intreq
	XREF	_intena
	XREF	_AbsExecBase

	INT_ABLES

*------ Functions ----------------------------------------------------

	XREF	InsertReq
	XREF	IntCommon
	XREF	IntEnd

*------ System Library Functions -------------------------------------

	XSYS	Insert
	XSYS	Alert
	XSYS	ReplyMsg
	XSYS	SetICR
	XSYS	AbleICR
	XSYS	AddICRVector
	XSYS	Cause

***#*** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	Tick0
	XDEF	AddReqUnit0
	XDEF	RemReqUnit0
	XDEF	EClock_Int
	XDEF	ReadEClockTime
	XDEF	ReadESClockTime
	XDEF	CalcDelay0
	XDEF	Mult64
	XDEF	AbortMicroTimer
	XDEF	RethinkMicroTimer
	XDEF	EIncSTTOD
	XDEF	EIncSTEClock
	XDEF	SwapCIA
	XDEF	Tick2

*------ Assumptions -------------------------------------------

	IFNE	CIAICRB_TA
	FAIL	"CIAICRB_TA not 0 - recode"
	ENDC

	IFNE	CIAICRB_TB-1
	FAIL	"CIAICRB_TB not 1 - recode"
	ENDC

*------ Data ---------------------------------------------------------

***#*** Local Definitions ********************************************

ASTOP_AND equ	CIACRBF_ALARM+CIACRAF_PBON+CIACRAF_OUTMODE+CIACRAF_RUNMODE+CIACRAF_SPMODE

	; STOP -
	;	START bit 0 == 0 (STOP IMMEDIATELY)
	;	PBON  bit 1 == same
	;	OUT   bit 2 == same
	;       RUN   bit 3 == same
	;       LOAD  bit 4 == 0 (NO FORCE LOAD)
	;       IN    bit 5 == 0 (counts 02 pulses)
	;       SP    bit 6 == same
	;       ALARM bit 7 == same (unused on ciacra, used by graphics TOD)


ASTART_OR equ	CIACRAF_RUNMODE

	; START -
	;	START bit 0 == same (STARTED BY LOAD OF HI BYTE)
	;	PBON  bit 1 == same
	;	OUT   bit 2 == same
	;       RUN   bit 3 == 1    (ONE SHOT MODE)
	;       LOAD  bit 4 == same
	;       IN    bit 5 == same
	;       SP    bit 6 == same
	;       ALARM bit 7 == same



ARESET_AND	EQU	CIACRBF_ALARM+CIACRAF_PBON+CIACRAF_OUTMODE+CIACRAF_SPMODE

	; RESET -
	;	START bit 0 == 0 (STOP IMMEDIATELY)
	;	PBON  bit 1 == same
	;	OUT   bit 2 == same
	;       RUN   bit 3 == 0 (TURN RUNMODE OFF - POWER-UP DEFAULT)
	;       LOAD  bit 4 == 0 (NO FORCE LOAD)
	;       IN    bit 5 == 0 (counts 02 pulses)
	;       SP    bit 6 == same
	;       ALARM bit 7 == same (unused on ciacra, used by graphics TOD)



INTERRUPTS_OFF	EQU	00


**********************************************************************
*
* Call back routine for cia swapping code (called within DISABLE)
*
* A2 -- CIABASE of CIA to AddICRVector() from
* A3 -- Ptr to other CIA BASE
* D2 -- TD bit to ask for
* D3 -- index # (3..0)
*
SwapCIA:

		movea.l	CR_SharedData(a2),a6
		move.l	cs_TimerBase(a6),a6

	;-- stop timer immediately
	;
	;-- turn off RUNMODE bit - fix for games like
	;-- BLUE ANGELS from Accolade - control register
	;-- dependency

		move.l	TD_JUMPYCIACRX(a6),a0
		andi.b	#ARESET_AND,(a0)

	;-- recalculate:
	;--
	;   1.) Ptr to low byte of interval timer (high byte is
	;       known to be $100 higher)
	;
	;   2.) Ptr to control register.
	;
	;

		lsl.b	#3,d3			;index (3..0) * 8
		lea	ciatable(pc,d3.w),a0
		move.l	(a0)+,TD_JUMPYCIATXLO(a6)
		move.l	(a0),TD_JUMPYCIACRX(a6)

	;-- recalculate:
	;--
	;   3.) Ptr to interrupt code to call when interval timer
	;	counts down.
	;
	;   4.) Data for interrupt code.

		lea	Tick2(pc),a0

		tst.b	CR_HWADDR+3(a2)		;00 for b, 01 for a
		beq.s	sw_install

		lea	Tick0(pc),a0
		move.l	a6,a3

	;-- install new ICR Vector
	;
	;-- Note that we are still in Disable(), and that AddICRVector()
	;-- is going to succeed because we already know that the timer we
	;-- are asking for is free.

sw_install:

		lea	TD_CIATJUMP(a6),a1
		move.l	a0,IS_CODE(a1)		;Tick0     or Tick2
		move.l	a3,IS_DATA(a1)		;timerbase or ciaabase
		move.l	d2,d0

	;
	;-- Clear INMODE1 bit on control register B, CIAB - if timer
	;-- will be using CIAB Timer B.  For 02 pulse count-down mode.
	;
		beq.s	sw_addicrv
		and.b	#(~CIACRBF_INMODE1),_ciab+ciacrb
sw_addicrv:
		exg	a6,a2
		CALLSYS	AddICRVector

	;-- restart micro timer

		move.l	a2,a6

	;-- A3 MUST be set up before calling RethinkMicroTimer

		lea	TD_ECLOCKLIST(a6),a3
		bsr	AbortMicroTimer		;STOP micro timer
						;and set 1 shot mode
		bra	RethinkMicroTimer
		;rts

	;-- cia table (preferential order worst-best choice)

ciatable:


		dc.l	_ciab+ciatalo		;CIAB timer A
		dc.l	_ciab+ciacra		;index 0

		dc.l	_ciaa+ciatalo		;CIAA timer A
		dc.l	_ciaa+ciacra		;index 1

		dc.l	_ciab+ciatblo		;CIAB timer B
		dc.l	_ciab+ciacrb		;index 2

		dc.l	_ciaa+ciatalo		;CIAA timer A
		dc.l	_ciaa+ciacra		;index 3

***#*** Timer/Tick2 **************************************************
*
*   NAME
*	Tick2 -- Generate a level 2 INT
*
*   SYNOPSIS
*	Tick2( ptr to CIAA Base )
*	       A1
*
*   FUNCTION
*	This is the interrupt routine for the microtic interval timer
*	if running off of one of the CIAB interval timers.  It generates
*	a level 2 hardware interrupt, and sets a bit in ciaa.resource
*	which will be caught by the cia.resource interrupt routine.
*
*	This is done to lower level 6 interrupts to level 2, thereby
*	allowing more important level 3-6 interrupts to be processed
*	first.
*
*   INPUTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
Tick2:
		bset	#TIMERINT,CR_TimerINT(a1)
		move.w	#INTF_SETCLR!INTF_PORTS,_intreq
		rts

**********************************************************************
*
* Unit specific insertion of a time request
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- unit list (given)
*	A2 -- munged
*	A1 -- request (given, munged)
*	A0 -- munged


AddReqUnit0:
*$* debug
*		ERRMSG	"AddReqUnit0"
*$* debug
		bsr	AbortMicroTimer
		bsr	CalcDelay0
		bsr	InsertReq
		bra	RethinkMicroTimer
		;rft

**********************************************************************
*
* Unit specific removal of a time request
*
*	this routine runs 'DISABLED' and returns 'ENABLED'
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- unit list (given)
*	A2 -- munged
*	A1 -- request (given, munged)
*	A0 -- munged

RemReqUnit0:
*$* debug
*		ERRMSG	"RemReqUnit0"
*$* debug
		REMOVE
		bsr	AbortMicroTimer
		bsr	RethinkMicroTimer
		TD_ENABLE	A0,TD_SYSLIB(a6)
		rts

***#*** System/Drivers/Timer/TickInt **********************************
*
*   NAME
*	TickInt - Handle and interrupt tick
*
*   SYNOPSIS
*	TickInt( Unit )
*	         A1
*
*   FUNCTION
*	This is the interrupt routine for the microtic interval timer
*
*   INPUTS
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
*   REGISTER USAGE
*
*	A6 -- Device Data
*	A3 -- unit list
*	A1 -- munged
*	A0 -- munged
*

Tick0:
		MOVEM.L	A3,-(SP)
		MOVE.L	A1,A6
		lea	TD_ECLOCKLIST(a6),a3

	;-- AbortMicroTimer() inline for speed

		move.l	TD_JUMPYCIACRX(a6),a0
		andi.b	#ASTOP_AND,(a0)
		ori.b	#ASTART_OR,(a0)	;set 1 shot mode

		subq.l	#8,sp
		move.l	sp,a0
		bsr.s	ReadEClockTime
		bsr	IntCommon		; check for expired reqs
		addq.l	#8,sp

		bsr	RethinkMicroTimer

		bra	IntEnd
		; rft


**********************************************************************
*
* Returns the time, in E-clock tics, since timer.device was initialized.
*	EV_HI is the hi order 32 bits, EV_LO is the lo order 32 bits.
*
* Exits DISABLED ... minimize time spent in DISABLE for MIDI performance.
*                    called by RethinkMicroTimer() only
*
*   REGISTER USAGE
*	A6 -- Device Data (given)
*	A1 -- munged
*	A0 -- eclockval for "e-clock time" (given)
*	D1 -- munged
*	D0 -- munged
*
ReadEMClockTime:

		movem.l	d2-d5/a0/a2,-(sp)

		moveq	#00,d0		; argument for SetICR()
		moveq	#00,d2
		moveq	#01,d5		; Flag results of SetICR()

		lea	_ciaa,a1

		move.l	TD_COUNTERRESOURCE(a6),a2
		exg	a6,a2		; a6=ciaabase a2=timerbase

	; SPECIAL DISABLE here - add two (2) to IDNestCnt so that
	; ReadEClockTime() exits DISABLED

		move.l	TD_SYSLIB(a2),a0
		move.w	#$04000,_intena
		addq.b	#2,IDNestCnt(a0)
		bra.s	readeclock

**********************************************************************
*
* Returns the time, in E-clock tics, since timer.device was initialized.
*	EV_HI is the hi order 32 bits, EV_LO is the lo order 32 bits.
*
* And grabs current TD_SYSTIME SECS, and MICROS within disable
*
*   REGISTER USAGE
*	A6 -- Device Data (given)
*	A1 -- Ptr to TIVEVAL structure (given)
*	A0 -- eclockval for "e-clock time" (given)
*	D1 -- munged
*	D0 -- munged
*
ReadESClockTime:

		movem.l	d2-d5/a0/a2,-(sp)

		moveq	#00,d0		; argument for SetICR()
		moveq	#00,d2
		moveq	#01,d5		; Flag results of SetICR()

		move.l	TD_COUNTERRESOURCE(a6),a2
		exg	a6,a2		; a6=ciaabase a2=timerbase

		TD_DISABLE	A0,TD_SYSLIB(a2)

		move.l	TD_SYSTIME+TV_SECS(a2),TV_SECS(a1)
		move.l	TD_SYSTIME+TV_MICRO(a2),TV_MICRO(a1)
		lea	_ciaa,a1
		bra.s	readeclock

**********************************************************************
*
* Returns the time, in E-clock tics, since timer.device was initialized.
*	EV_HI is the hi order 32 bits, EV_LO is the lo order 32 bits.
*
*
*   REGISTER USAGE
*	A6 -- Device Data (given)
*	A1 -- munged (FLAG return - if eclock interrupt at read time)
*	A0 -- eclockval for "e-clock time" (given)
*	D1 -- munged
*	D0 -- munged
*
ReadEClockTime:
		movem.l	d2-d5/a0/a2,-(sp)

	; set-up as much as possible before disable

		moveq	#00,d0		; argument for SetICR()
		moveq	#00,d2
		moveq	#01,d5		; Flag results of SetICR()

		move.l	TD_COUNTERRESOURCE(a6),a2
		exg	a6,a2		; a6=ciaabase a2=timerbase
		lea	_ciaa,a1

	;
	; Read hardware values, and check for an unprocessed
	; interrupt, which means that the hardware counters
	; rolled-over from $0000->$xxxx either before we read
	; the hardware counters, or during.
	;
	; This code needs to be -fast-, and get out of DISABLE
	; ASAP for the sake of MIDI performance.
	;
	; Testing reveals that we will get an unprocessed
	; interrupt once every 1-2 seconds while reading the hardware
	; values (average case - VBLANK interrupt calls ReadEClockTime
	; a lot, as does much of the rest of the timer.device code).
	;

		TD_DISABLE	A0,TD_SYSLIB(a2)

readeclock:
		move.b	ciatbhi(a1),d3
		move.b	ciatblo(a1),d4
		move.b	ciatbhi(a1),d2

		CALLSYS	SetICR		; sample interrupts

		btst	#CIAICRB_TB,d0	; check if TB interrupt is pending
		beq.s	CountersValid

		lea	_ciaa,a1	; if so, assume previous sample
					; is invalid, and re-read hardware
		move.b	ciatbhi(a1),d3
		move.b	ciatblo(a1),d4
		move.b	ciatbhi(a1),d2
					;
		moveq	#00,d5          ; Flag interrupt happened

CountersValid:
		movem.l	TD_ECLOCKTIME(a2),d0/d1

		TD_ENABLE	A0,TD_SYSLIB(a2)

		move.l	a2,a6		; restore a6

	;
	; check for hi-byte roll-over while reading hardware values
	;
		cmp.b	d3,d2
		beq.s	GotCounterParts
		moveq.l	#-1,d4
GotCounterParts:

	;
	; form an incrementing number from the decrementing number
	;
		lsl.w	#8,d2		; hibyte << 8
		move.b	d4,d2		; OR lowbyte
		addq.w	#1,d2		; +1, and form 2's complement
		neg.w	d2		; tic count in d2

	;
	; check to see if the counters rolled over from 0000->XXXX
	;
	; by checking to see if an EClock interrupt was left
	; unprocessed when we read the software, and hardware
	; counters
	;

		tst.b	d5
		bne.s	AddCounterParts

	;
	; roll-over, so add $10000 to EClock time in registers
	;
	; software values in TD_ECLOCKTIME will be incremented
	; by the EClock_Int routine
	;
		add.l	#$10000,d1
		addx.l	d5,d0		; d5 == 0L

AddCounterParts:

		move.w	d2,d1		; low word should always be 0
		move.w	d5,a1		; return SetICR() flag in A1

		movem.l	(sp)+,d2-d5/a0/a2
		movem.l	d0/d1,(a0)
		rts

**********************************************************************
*
* abort the micro interval timer (if active)
*
*	this must run 'DISABLED'
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A2 -- munged
*	A1 -- must be restored if changed
*	A0 -- munged

AbortMicroTimer:
; stop counter A and set 1 shot mode


		move.l	TD_JUMPYCIACRX(a6),a0
		andi.b	#ASTOP_AND,(a0)
		ori.b	#ASTART_OR,(a0)	;set 1 shot mode
		rts

**********************************************************************
*
* set the micro interval timer
*
*	this must run DISABLED'
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- unit list (given)
*	A1 -- munged
*	A0 -- munged
*	D1 -- munged
*	D0 -- munged


RethinkMicroTimer:

; if the request queue is not empty, program counter A to interrupt us
;	when the first request should be returned. If the desired count
;	is greater than 64k-1, set count to 60k. If desired count is less
;	than 50 or negative, special handling should be looked into.



	; quick check for empty list outside of disable

		cmp.l	LH_TAIL+LN_PRED(a3),a3
		beq.s	RMTEnd

		subq.l	#8,sp
		move.l	sp,a0

		bsr	ReadEMClockTime

	; we are now disabled, recheck to make sure list isnt empty

		cmp.l	LH_TAIL+LN_PRED(a3),a3
		beq.s	RMTExit

		move.l	(a3),a1
		move.l	IOTV_TIME+TV_MICRO(a1),d0
		sub.l	EV_LO(a0),d0
		move.l	d0,EV_LO(a0)
		move.l	IOTV_TIME+TV_SECS(a1),d0
		move.l	(a0),d1		;EV_HI - assembler doesnt optimize
		subx.l	d1,d0
		bcs.s	NegCount
		tst.l	d0
		bne.s	BigCount

		move.l	EV_LO(a0),d0
		cmpi.l	#LONGCOUNT,d0
		bcc.s	BigCount
		moveq	#SHORTCOUNT,d1
		cmp.l	d1,d0		;faster than CMPI.L
		bcs.s	NegCount

; here the count in d0 is known to be in an acceptable range

GotGoodCount:

		move.l	TD_JUMPYCIATXLO(a6),a0
		move.b	d0,(a0)
		lsr.w	#8,d0
		move.b	d0,$100(a0)	;ugly, but we know it to be true
					;starts timer (in 1 SHOT MODE)
RMTExit:

		TD_ENABLE	A0,TD_SYSLIB(a6)
		addq.l	#8,sp

RMTEnd:
		rts

BigCount:
		move.l	#LONGCOUNT,d0
		bra.s	GotGoodCount

NegCount:
; set the interrupt, then return
*$* debug
*		ERRMSG	"NegCount"
*$* debug

; this is much like calling SetICR() to generate an interrupt, however
; we can't afford the overhead of SetICR() here due to MIDI performance.
; With the addition of all other OS optimizations, we are still seeing
; overruns when RethinkMicroTimer() falls through this section of code.


		move.l	TD_COUNTERRESOURCE(a6),a0	;CIA-A!
		bset	#TIMERINT,CR_TimerINT(a0)	;set interrupt
		move.w	#INTF_SETCLR!INTF_PORTS,_intreq	;and return

		TD_ENABLE	A0,TD_SYSLIB(a6)
		addq.l	#8,sp

		rts

**********************************************************************
*
* Calculates the point in "EClock time" that the request should be
*	returned
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A2 -- munged
*	A1 -- request (given)
*	A0 -- munged

CalcDelay0:
*$* debug
*		ERRMSG	"CalcDelay0"
*$* debug
		subq.l	#8,sp
		move.l	sp,a0

		move.l	a1,-(sp)
		bsr	ReadEClockTime
		move.l	(sp)+,a1

		move.l	IOTV_TIME+TV_SECS(a1),d0
		move.l	TD_ECLOCKHERTZ(a6),d1
		bsr	Mult64

		move.l	IOTV_TIME+TV_MICRO(a1),-(sp)

		add.l	EV_LO(a0),d1
		move.l	d1,IOTV_TIME+TV_MICRO(a1)
		move.l	EV_HI(a0),d1
		addx.l	d1,d0
		move.l	d0,IOTV_TIME+TV_SECS(a1)

		move.l	(sp)+,d0
		move.l	TD_ECLOCKCONST1(a6),d1
		bsr.s	Mult64

		move.w	d0,d1
		swap.w	d1
		clr.w	d0
		swap.w	d0

		add.l	d1,IOTV_TIME+TV_MICRO(a1)
		move.l	IOTV_TIME+TV_SECS(a1),d1
		addx.l	d1,d0
		move.l	d0,IOTV_TIME+TV_SECS(a1)

		addq.l	#8,sp
		rts

**********************************************************************
*
* counter B roll-over interrupt routine
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A1 -- munged
*	A0 -- munged
*	D1 -- munged
*	D0 -- munged
*

EClock_Int:

	; A0 will be used to point to ciaa base ... we are going to
	; pull a dirty trick here.
	;
	; This code was identified as the biggest cause of MIDI
	; overruns, so it, and ReadEClockTime() had to be rewritten.
	; ReadEClockTime() notes that there is a pending interrupt,
	; and adds $10000 to its calculated value, but the
	; EClock_Int() routine is actually responsible for updating
	; the software values (TD_ECLOCKTIME) 11x per second.
	;
	; However once the software values have been updated, we
	; want to make sure that SetICR() returns FALSE, so...
	;
	; This code clears a bit in ciabase which would normally
	; be cleared after all interrupts have been processed.
	; The proper way to do this is to call SetICR(), but we
	; can't afford the overhead because of the impact on MIDI
	; performance.
	;


		move.l	TD_COUNTERRESOURCE(a1),a0	;ciaa base

		TD_DISABLE	a6,TD_SYSLIB(a1)	;sys lib in a6


		addi.l	#$10000,TD_ECLOCKTIME+EV_LO(a1)
		moveq	#00,d0
		move.l	TD_ECLOCKTIME+EV_HI(a1),d1
		addx.l	d0,d1
		move.l	d1,TD_ECLOCKTIME+EV_HI(a1)

	;
	; Clear processing bit in ciaa base - now SetICR() will
	; NOT return true for this interrupt again until the next
	; real interrupt.
	;

		bclr	#CIAICRB_TB,CR_IProcessing(a0)

		jsr	EIncSTfmt(a1)	; do systime processing

NoTBInt:
		ENABLE		a6,NOFETCH		;faster than timer
		rts					;TD_ENABLE macro


	;
	; If the ciaa TOD counter isn't running off the power-line,
	; the EClock_Int() code is used to update TD_SYSTIME values.
	;


EIncSTEClock:
		addi.l	#$10000,TD_SYSTIME+TV_MICRO(a1)
		move.l	TD_SYSTIME+TV_MICRO(a1),d0
		cmp.l	TD_ECLOCKHERTZ(a1),d0
		bcs.s	EIncSTEClockE
		addq.l	#1,TD_SYSTIME+TV_SECS(a1)
		sub.l	TD_ECLOCKHERTZ(a1),d0
		move.l	d0,TD_SYSTIME+TV_MICRO(a1)
EIncSTEClockE:

	;
	; jsr EIncSTfmt(An) gets us here if we are using the TOD
	; interrupt to update TD_SYSTIME values
	;

EIncSTTOD:
		rts


**********************************************************************
*
* multiply two unsigned 32 bit integers and return the 64 bit result
*
*   REGISTER USAGE
*	D4 -- scratch (restored)
*	D3 -- scratch (restored)
*	D2 -- scratch (restored)
*	D1 -- arg 1 (given), result 32:63
*	D0 -- arg 0 (given), result 0:31
*

Mult64:
*$* debug
*		ERRMSG	"Mult64"
*$* debug
		movem.l	d2-d4,-(sp)

		move.l	d1,d3
		mulu.w	d0,d3		; 24
		move.l	d1,d2
		swap.w	d2
		swap.w	d0
		mulu.w	d0,d2		; 13

		swap.w	d3
		bsr.s	PartMult
		swap.w	d0
		swap.w	d1
		bsr.s	PartMult
		swap.w	d3

		move.l	d2,d0
		move.l	d3,d1

		movem.l	(sp)+,d2-d4
		rts

PartMult:
		move.l	d1,d4
		mulu.w	d0,d4		; 14 (23)
		add.w	d4,d3
		clr.w	d4
		swap.w	d4
		addx.l	d4,d2
		rts


**********************************************************************




	END
