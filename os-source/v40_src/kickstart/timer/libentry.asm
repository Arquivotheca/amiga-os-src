*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* libentry.asm
*
* Source Control
* ------ -------
* 
* $Id: libentry.asm,v 36.19 91/11/05 18:40:53 darren Exp $
*
* $Locker:  $
*
* $Log:	libentry.asm,v $
* Revision 36.19  91/11/05  18:40:53  darren
* Autodoc changes
* 
* Revision 36.19  91/11/05  18:00:36  darren
* Remove spurious "()" from autodoc lines.
* 
* Revision 36.18  91/04/09  13:46:52  darren
* Major changes for better MIDI performance (decrease DISABLE time)
* 
* Revision 36.17  91/01/25  15:46:14  rsbx
* Change to V37
* 
* Revision 36.16  90/04/25  21:52:21  rsbx
* Fixed CmpTime autodoc.
* 
* Revision 36.15  90/04/01  19:12:54  rsbx
* RCS version change.
* 
* Revision 36.14  90/02/16  11:56:18  rsbx
* 
* 
* Revision 36.13  89/11/23  16:49:27  rsbx
* Cleaned up some graphics kludges. Removed references to TR_WAITUNTIL and
* WaitUnitl() from to docs.
* 
* Revision 36.12  89/09/15  18:55:14  rsbx
* 
* 
* Revision 36.2  89/08/09  19:14:04  rsbx
* changed some comments for autodoc
* 
* Revision 36.1  89/08/09  18:09:41  rsbx
* *** empty log message ***
* 
* Revision 1.1  89/08/09  14:07:47  rsbx
* Initial revision
*
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
	XREF	ReadEClockTime
	XREF	ReadESClockTime
	XREF	Mult64
	XREF	Open
	XREF	Close
	XREF	ReadTOD
	XREF	Tics2Time

	INT_ABLES

*------ Functions ----------------------------------------------------


*------ System Library Functions -------------------------------------

	XSYS	AllocSignal
	XSYS	FreeSignal
	XSYS	DoIO
	XSYS	FindTask

*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF	TimSub
	XDEF	TimAdd
	XDEF	TimCmp
	XDEF	ReadETime
	XDEF	GetSysTime
	XDEF	GetSTTOD
	XDEF	GetSTEClock
	XDEF	STTOD2Req
	XDEF	STEClock2Req

*------ Data ---------------------------------------------------------

*------ Local Definitions --------------------------------------------






******* timer.device/GetSysTime **************************************
*
*   NAME
*	GetSysTime -- Get the system time. (V36)
*
*   SYNOPSIS
*	GetSysTime( Dest )
*	            A0
*
*	void GetSysTime( struct timeval * );
*
*   FUNCTION
*	Ask the system what time it is.  The system time starts off at
*	zero at power on, but may be initialized via the TR_SETSYSTIME
*	timer.device command.
*
*	System time is monotonocally increasing and guarenteed to be
*	unique (except when the system time is set back).
*
*	A0 will be left unchanged.
*
*	This function is less expensive to use than the TR_GETSYSTIME
*	IORequest.
*
*   INPUTS
*	Dest -- pointer to a timeval structure to hold the system time.
*
*   RESULTS
*	Dest -- the timeval structure will contain the system time.
*
*   NOTES
*	This function may be called from interrupts.
*
*   SEE ALSO
*	timer.device/TR_GETSYSTIME,
*	timer.device/TR_SETSYSTIME,
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*


GetSysTime:
*$* debug
*		ERRMSG	"GetSysTime"
*$* debug
		jsr	STfmt2Req(a6)
		rts


**********************************************************************
*
* mode specific systime to timerequest conversion
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*

STEClock2Req:
		bsr	GetSTEClock		; get internal systime

		move.l	TV_MICRO(a0),d0
		move.l	TD_ECLOCKCONST2(a6),d1
		bsr	Mult64			; etics to usecs
		move.w	d0,d1
		swap	d1			; micros

		move.l	d1,TV_MICRO(a0)
		rts


**********************************************************************
*
* mode specific systime to timerequest conversion
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*

STTOD2Req:
		TD_DISABLE	a1,TD_SYSLIB(a6)

		move.l	TD_SYSTIME+TV_MICRO(a6),TV_MICRO(a0)
		move.l	TD_SYSTIME+TV_SECS(a6),TV_SECS(a0)
		bsr	ReadTOD		; get num tic since last update
		move.l	TD_NUMREQS(a6),-(sp)

		TD_ENABLE	a1,TD_SYSLIB(a6)

		bsr	Tics2Time		; adjust

		move.l	TV_MICRO(a0),d1
		mulu.w	TD_TIMEPERTOD(a6),d1	; adjust
		add.l	(sp)+,d1		; make unique
		move.l	d1,TV_MICRO(a0)
		rts



**********************************************************************
*
* return mode specific systime
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*

GetSTTOD:
		TD_DISABLE	a1,TD_SYSLIB(a6)
		move.l	TD_SYSTIME+TV_MICRO(a6),TV_MICRO(a0)
		move.l	TD_SYSTIME+TV_SECS(a6),TV_SECS(a0)
		bsr	ReadTOD		; get num tic since last update
		TD_ENABLE	a1,TD_SYSLIB(a6)
		bsr	Tics2Time	; adjust
		rts


**********************************************************************
*
* return mode specific systime
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A1 -- munged
*	A0 -- time field of request (given)
*

GetSTEClock:
		subq.l	#TV_SIZE,sp
		move.l	sp,a1

	;
	; ReadESClockTime() also grabs TD_SYSTIME, and puts it wherever
	; A1 pts too.  TD_SYSTIME is grabbed along with the EClock time
	; within disable.
	;
	; TB Interrupt state at ReadEClockTime() is returned in A1
	;

		bsr	ReadESClockTime
				
		move.l	TV_SECS(sp),TV_SECS(a0)
		move.l	TV_MICRO(sp),d0

		addq.l	#TV_SIZE,sp

	;
	; Test if SetICR() returned TRUE at ReadEClockTime() - if so,
	; then TD_SYSTIME is off by $10000.  See EIncSTEClock().
	;
	; Typically this test will be FALSE, and will branch to
	; gsc_noint
	;

		move.w	a1,d1
		bne.s	gsc_noint	;1 if no TB interrupt at read time


		addi.l	#$10000,d0
		cmp.l	TD_ECLOCKHERTZ(a6),d0
		bcs.s	gsc_noint
		addq.l	#1,TV_SECS(a0)
		sub.l	TD_ECLOCKHERTZ(a6),d0

gsc_noint:					
		move.l	EV_LO(a0),d1	; same offset as TV_MICRO - still valid

		swap	d1
		move.w	#0,d1
		swap	d1
		add.l	d1,d0

		cmp.l	TD_ECLOCKHERTZ(a6),d0
		bcs.s	NoAdjust
		addq.l	#1,TV_SECS(a0)
		sub.l	TD_ECLOCKHERTZ(a6),d0
NoAdjust:
		move.l	d0,TV_MICRO(a0)
		rts


******* timer.device/ReadEClock **************************************
*
*   NAME
*	ReadEClock -- Get the current value of the E-Clock. (V36)
*
*   SYNOPSIS
*	E_Freq = ReadEClock( Dest )
*	D0                   A0
*
*	ULONG ReadEClock ( struct EClockVal * );
*
*   FUNCTION
*	This routine calculates the current 64 bit value of the E-Clock
*	and stores it in the destination EClockVal structure. The count
*	rate of the E-Clock is also returned.
*
*	A0 will be left unchanged
*
*	This is a low overhead function designed so that very short
*	intervals may be timed.
*
*   INPUTS
*	Dest -- pointer to an EClockVal structure.
*
*   RETURNS
*	Dest -- the EClockVal structure will contain the E-Clock time
*	E_Freq -- The count rate of the E-Clock (tics/sec).
*
*   NOTES
*	This function may be called from interrupts.
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
*	A6 -- TDDev (given)
*	A1 -- munged
*	A0 -- time field of request (given)
*

ReadETime:
*$* debug
*		ERRMSG	"ReadETime"
*$* debug
		bsr	ReadEClockTime
		move.l	TD_ECLOCKHERTZ(a6),d0

		RTS


******* timer.device/SubTime *****************************************
*
*   NAME
*	SubTime -- Subtract one time request from another.
*
*   SYNOPSIS
*	SubTime( Dest, Source )
*	         A0    A1
*
*	void SubTime( struct timeval *, struct timeval *);
*
*   FUNCTION
*	This routine subtracts one timeval structure from another.  The
*	results are stored in the destination (Dest - Source -> Dest)
*
*	A0 and A1 will be left unchanged
*
*   INPUTS
*	Dest, Source -- pointers to timeval structures.
*
*   NOTES
*	This function may be called from interrupts.
*
*   SEE ALSO
*	timer.device/AddTime(),
*	timer.device/CmpTime()
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

TimSub:
*$* debug
*		ERRMSG	"TimSub"
*$* debug
		MOVEM.L	(A0),D0/D1		; TV_SECS == 0

		SUB.L	(A1),D0			; TV_SECS == 0
		SUB.L	TV_MICRO(A1),D1


TimSub_Overflow:
		BGE.S	TimSub_Assign

		SUBQ.L	#1,D0
		ADD.L	#MAXMICRO,D1
		BRA.S	TimSub_Overflow

TimSub_Assign:
		MOVEM.L	D0/D1,(A0)		; TV_SECS == 0

		RTS


******* timer.device/AddTime *****************************************
*
*   NAME
*	AddTime -- Add one time request to another.
*
*   SYNOPSIS
*	AddTime( Dest, Source )
*	         A0    A1
*
*	void AddTime( struct timeval *, struct timeval *);
*
*   FUNCTION
*	This routine adds one timeval structure to another.  The
*	results are stored in the destination (Dest + Source -> Dest)
*
*	A0 and A1 will be left unchanged
*
*   INPUTS
*	Dest, Source -- pointers to timeval structures.
*
*   NOTES
*	This function may be called from interrupts.
*
*   SEE ALSO
*	timer.device/CmpTime(),
*	timer.device/SubTime()
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

TimAdd:
*$* debug
*		ERRMSG	"TimAdd"
*$* debug

		MOVE.L	D2,-(SP)
    
		MOVE.L	#MAXMICRO,D2

		MOVEM.L	(A0),D0/D1		; TV_SECS == 0

		ADD.L	(A1),D0			; TV_SECS == 0
		ADD.L	TV_MICRO(A1),D1

TimAdd_OverFlow:
		CMP.L	D1,D2
		BGT.S	TimAdd_Assign

		ADDQ.L	#1,D0
		SUB.L	D2,D1
		BRA.S	TimAdd_OverFlow

TimAdd_Assign:
		MOVEM.L	D0/D1,(A0)		; TV_SECS == 0

		MOVE.L	(SP)+,D2
		RTS


******* timer.device/CmpTime *****************************************
*
*   NAME
*	CmpTime -- Compare two timeval structures.
*
*   SYNOPSIS
*	result = CmpTime( Dest, Source )
*	D0                A0    A1
*
*	LONG CmpTime( struct timeval *, struct timeval *);
*
*   FUNCTION
*	This routine compares timeval structures
*
*	A0 and A1 will be left unchanged
*
*   INPUTS
*	Dest, Source -- pointers to timeval structures.
*
*   RESULTS
*	result will be   0 if Dest has same time as source
*	                -1 if Dest has more time than source
*	                +1 if Dest has less time than source
*
*   NOTES
*	This function may be called from interrupts.
*
*   SEE ALSO
*	timer.device/AddTime(),
*	timer.device/SubTime()
*
*   BUGS
*	Older version of this document had the sense of the return
*	codes wrong; the code hasn't changed but the document has.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

TimCmp:
*$* debug
*		ERRMSG	"TimCmp"
*$* debug

		CLEAR	D0

		TIMCMP	A0,A1,D1
		BEQ.S	TimCmp_End

		BHI.S	TimCmp_hi

		MOVEQ	#-1,D0

TimCmp_End:
		RTS
TimCmp_hi:
		MOVEQ	#1,D0
		rts


	END

