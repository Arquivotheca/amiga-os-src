*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: eclock.asm,v 39.0 92/01/20 13:10:20 mks Exp $
*
* $Log:	eclock.asm,v $
* Revision 39.0  92/01/20  13:10:20  mks
* Timer device now calls battclock to read the base system time.
* Also made some branches short...
* 
*************************************************************************

	SECTION	timer

*------ Included Files -----------------------------------------------

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

*------ Imported Names -----------------------------------------------

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	_ciaa
	XREF	_ciab
	XREF	_AbsExecBase

	INT_ABLES

*------ Functions ----------------------------------------------------

	XREF	TermIO
	XREF	IntCommon
	XREF	IntEnd
	XREF	InsertReq
	XREF	ReadEClockTime
	XREF	AbortMicroTimer
	XREF	RethinkMicroTimer

*------ System Library Functions -------------------------------------

	XSYS	Insert
	XSYS	Alert
	XSYS	ReplyMsg
	XSYS	SetICR
	XSYS	AbleICR

*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF	AddReqUnit2
	XDEF	RemReqUnit2
	XDEF	AddReqUnit4
	XDEF	RemReqUnit4

*------ Data ---------------------------------------------------------

*------ Local Definitions --------------------------------------------



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

AddReqUnit2:
*$* debug
*		ERRMSG	"AddReqUnit2"
*$* debug

		bsr	AbortMicroTimer
		bsr.s	CalcUnit2
		bsr	InsertReq
		bra	RethinkMicroTimer
		;rft


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

AddReqUnit4:
*$* debug
*		ERRMSG	"AddReqUnit4"
*$* debug

		bsr	AbortMicroTimer
		bsr	InsertReq
		bra	RethinkMicroTimer
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

RemReqUnit2:
RemReqUnit4:
*$* debug
*		ERRMSG	"RemReqUnit2,4"
*$* debug
		REMOVE				; node a1
		bsr	AbortMicroTimer
		bsr	RethinkMicroTimer
		TD_ENABLE	a0,TD_SYSLIB(a6)

		rts


**********************************************************************
*
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- unit list (given)
*	A2 -- munged
*	A1 -- request (given,munged)
*	A0 -- munged

CalcUnit2:
		subq.l	#EV_SIZE,sp
		move.l	sp,a0
		move.l	a1,-(sp)
		bsr	ReadEClockTime
		move.l	(sp),a1
		lea	IOTV_TIME(a1),a1
		andi.b	#0,CCR
		addq.l	#EV_SIZE,a0
		addq.l	#EV_SIZE,a1
		addx.l	-(a0),-(a1)
		addx.l	-(a0),-(a1)
		move.l	(sp)+,a1
		addq.l	#EV_SIZE,sp
		rts


	END
