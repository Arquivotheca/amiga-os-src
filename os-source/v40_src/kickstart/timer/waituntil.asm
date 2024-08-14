*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: waituntil.asm,v 39.0 92/01/20 13:10:26 mks Exp $
*
* $Log:	waituntil.asm,v $
* Revision 39.0  92/01/20  13:10:26  mks
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
	XREF	Mult64

*------ System Library Functions -------------------------------------

	XSYS	Insert
	XSYS	Alert
	XSYS	ReplyMsg
	XSYS	SetICR
	XSYS	AbleICR

*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF	AddReqUnit3
	XDEF	RemReqUnit3

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

AddReqUnit3:
*$* debug
*		ERRMSG	"AddReqUnit3"
*$* debug
		bsr.s	CalcWait3
		bra	InsertReq	; Tail recursion


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

RemReqUnit3:
*$* debug
*		ERRMSG	"RemReqUnit3"
*$* debug
		REMOVE				; node a1
		TD_ENABLE	a0,TD_SYSLIB(a6)

		rts


**********************************************************************
*
* Convert secs/usecs to whatever units the systime is in.
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A2 -- munged
*	A1 -- request (given)
*	A0 -- munged

CalcWait3:
*$* debug
*		ERRMSG	"CalcWait3"
*$* debug
		lea	IOTV_TIME(a1),a0
		jmp	Req2STfmt(a6)		; Tail recursion


	END
