*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: vblank.asm,v 39.0 92/01/20 13:10:48 mks Exp $
*
* $Log:	vblank.asm,v $
* Revision 39.0  92/01/20  13:10:48  mks
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

	INCLUDE	'asmsupp.i'
	INCLUDE	'timer.i'
	INCLUDE 'macros.i'
	INCLUDE 'constants.i'
	INCLUDE 'internal.i'
	INCLUDE 'debug.i'

***#*** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	_ciaa
	XREF	_ciab
	XREF	_intena
	XREF	_AbsExecBase

	INT_ABLES

*------ Functions ----------------------------------------------------

	XREF	TermIO
	XREF	IntCommon
	XREF	IntEnd
	XREF	InsertReq
	XREF	ReadEClockTime
	XREF	CalcDelay0

*------ System Library Functions -------------------------------------

	XSYS	Insert
	XSYS	Alert
	XSYS	ReplyMsg
	XSYS	SetICR
	XSYS	AbleICR

***#*** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	Tick1
	XDEF	AddReqUnit1
	XDEF	RemReqUnit1
	XDEF	TOD_Int
	XDEF	ReadTOD
	XDEF	Tics2Time
	XDEF	TODIncSTTOD
	XDEF	TODIncSTEClock

*------ Data ---------------------------------------------------------

***#*** Local Definitions ********************************************



**********************************************************************
*
* Unit specific insertion of a time request
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- unit list (given)
*	A2 -- munged
*	A1 -- request (given,munged)
*	A0 -- munged

AddReqUnit1:
*$* debug
*		ERRMSG	"AddReqUnit1"
*$* debug

		bsr	CalcDelay0
		bra	InsertReq
		;rft


**********************************************************************
*
* Unit specific removal of a time request
*
*	This code is run 'DISABLED' and returns 'ENABLED'
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- unit list (given)
*	A2 -- munged
*	A1 -- request (given, munged)
*	A0 -- munged

RemReqUnit1:
*$* debug
*		ERRMSG	"RemReqUnit1"
*$* debug
		REMOVE				; node a1

		TD_ENABLE	a0,TD_SYSLIB(a6)

		rts


***#** System/Drivers/Timer/TickInt **********************************
*
*   NAME
*	TickInt - Handle VBlank interrupt tick
*
*   SYNOPSIS
*	TickInt( Unit )
*	         A1
*
*   FUNCTION
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
*	A6 -- TDDev
*	A3 -- unit data
*	A1 -- munged
*	A0 -- munged
*

*
* process exec scheduling quantum stuff
*

Tick1:
		move.l	TD_SYSLIB(a1),a0

		;------ check the scheduling flags
		SUBQ.W	#1,Elapsed(A0)
		BNE.S	NoQuantumKick

		;------ kick the scheduler
		MOVE.W	Quantum(A0),Elapsed(A0)
		OR.W	#SF_SAR!SF_TQE,SysFlags(A0)
NoQuantumKick:
		MOVEM.L	A3,-(SP)
		MOVE.L	A1,A6

		sub.l	#EV_SIZE,sp

		move.l	sp,a0
		bsr	ReadEClockTime
		lea	TD_VBLANKLIST(A6),A3
		bsr	IntCommon		; check for expired reqs

		move.l	sp,a0
		jsr	GetSTfmt(a6)
		lea	TD_SYSTIMELIST(a6),a3
		bsr	IntCommon		; check for expired reqs

		addq.l	#EV_SIZE,sp
		bra	IntEnd
		; rft


**********************************************************************
*
* TOD register alarm interupt routine
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A1 -- munged
*	A0 -- munged
*	D0 -- munged
*

TOD_Int:
		move.l	a1,a6
		jsr	TODIncSTfmt(a6)
		rts


**********************************************************************
*
* TOD register alarm interupt routine
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A1 -- munged
*	A0 -- munged
*	D0 -- munged
*

; assumes that the alarm register doesn't get cleared

TODIncSTTOD:
		TD_DISABLE	a1,TD_SYSLIB(a6)

		bsr.s	ReadTOD

	; Clear TOD registers
	;
	; assumes A1 is _ciaa (see ReadTOD)

		moveq	#00,d1

		move.b	d1,ciatodhi(A1)
		move.b	d1,ciatodmid(A1)
		move.b	d1,ciatodlow(A1)

	;
	; Reset TD_LASTTOD below ... let TD_NUMREQS continue
	; to rise until the next TOD tick
	;
	; This avoids race conditions when GetSysTime() gets
	; in within DISABLE, reads TOD when its met its ALARM
	; condition, or when a higher priority interrupt is
	; is calling GetSysTime(), blocking this level 2 interrupt.
	;

		move.l	d1,TD_LASTTOD(a6)


	; adjust TD_SYSTIME - add whats in D0 to it

		lea	TD_SYSTIME(a6),a0
		bsr.s	Tics2Time

		TD_ENABLE	a1,TD_SYSLIB(a6)

TODIncSTEClock:
		rts


**********************************************************************
*
* Returns the contents of the TOD register
*
*   REGISTER USAGE
*
*	A1 -- _ciaa (return _ciaa for speed!)
*	D0 -- TOD count
*
* MUST be called within DISABLE
* MUST preserve A0
*


ReadTOD:
		moveq	#0,d0

		lea	_ciaa,a1

		MOVE.B	ciatodhi(A1),D0
		LSL.W	#8,D0
		MOVE.B	ciatodmid(A1),D0
		LSL.L	#8,D0
		MOVE.B	ciatodlow(A1),D0

	;
	; maintain a unique, and incrementing number for the period
	; of time during which ReadTOD() returns the same number.
	;

		cmp.l	TD_LASTTOD(a6),d0
		bne.s	newTOD
		addq.l	#1,TD_NUMREQS(a6)
		rts

newTOD:
		clr.l	TD_NUMREQS(a6)		;start new form
		move.l	d0,TD_LASTTOD(a6)
		rts




**********************************************************************
*
* Add a timeval, in secs and TOD tics, to a TOD tic count, and normalize
*	the result to secs and TOD tics.
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- timeval w/ offset (given)
*	D1 -- munged
*	D0 -- TOD count (given)
*

Tics2Time:
		add.l	TV_MICRO(a0),d0
		divu.w	TD_TODHERTZ(a6),d0
		move.l	d0,d1
		ext.l	d0
		add.l	d0,TV_SECS(a0)
		swap	d1
		ext.l	d1
		move.l	d1,TV_MICRO(a0)
		rts


	END

*=====================================================================*
*
* NOTES -
*
* About the ReadTOD() function
* ----------------------------
*
* ReadTOD() checks to see if the TOD counters have changed since
* they were last read.  If NOT, then a positively incrementing "noise"
* factor is incremented (TD_NUMREQS).
*
* This is necessary because system time is guaranteed to be monotomically
* increasing, and the TOD counters are 'crude' (roughly 1 tick per VBLANK).
*
* In theory, the noise factor can become so great that it ceases to be
* noise anymore - in reality this isn't a problem, nor is it a new
* problem.
*
* If the TOD counters have changed since last read, reset our noise
* factor.
*
* About the TOD Interrupt
* -----------------------
*
* The TOD interrupt rolls back the TOD counters at ALARM time.
* Currently this is every #$1000 TOD ticks.  The TD_SYSTIME values
* are also updated at this time.
*
* There is a possible race condition here which is prevented by
* allowing TD_NUMREQS to continue to rise until the next TOD tick.
*
* The scenerio goes like so -
*
*	GetSysTime()
*		DISABLE
*					<-TOD Interrupt NOW
*		ReadTOD()
*					TOD counter changed
*					(e.g., from 4095 to 4096)
*					TD_NUMREQS == 0
*		Calculate time
*
*					TD_SYSTIME + TOD + TD_NUMREQS
*		ENABLE
*
*	TOD Interrupt
*		DISABLE
*
*		ReadTOD()		Counter is still 4096 or greater
*					TD_NUMREQS + 1
*
*		ClearTOD		TOD == 000000
*
*		Calculate TD_SYSTIME
*					4096 TOD ticks, plus previous
*					TD_SYSTIME
*		ENABLE
*
*	GetSysTime()
*		DISABLE
*
*		ReadTOD()
*
*					Aha!  I read 000000, and I
*					thought the last number was
*					4096!  So reset TD_NUMREQS!
*		Calculate time
*
*			And...
*
*			Here is where we get into a problem.  ReadTOD()
*			comes up with 000000, and that is added to
*			TD_SYSTIME which happens to be the same
*			value calculated when we called GetSysTime()
*			above!
*
*			ReadTOD also notes that the TOD counters changed
*			since the last call to ReadTOD(), and sets
*			TD_NUMREQS == 0.
*
*			The net result is the SAME as the previous call
*			to GetSysTime().
*
* Same problem can happen if a high priority interrupt (above level 2)
* is doing a GetSysTime().
*
* We solve this problem by allowing TD_NUMREQS to continue to rise until
* the next TOD tic (e.g., from 0 -> 1).  This is why we have the line
* of code which clears TD_LASTTOD after clearing the TOD hardware counters.
*
*
*=====================================================================*

