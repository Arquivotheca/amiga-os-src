*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* device.asm
*
* Source Control
* ------ -------
* 
* $Id: device.asm,v 36.24 91/11/05 18:40:41 darren Exp $
*
* $Locker:  $
*
* $Log:	device.asm,v $
* Revision 36.24  91/11/05  18:40:41  darren
* Autodoc changes
* 
* Revision 36.24  91/11/05  18:00:08  darren
* Remove spurious "()" from autodoc lines.
* 
* Revision 36.23  91/03/14  17:24:47  darren
* *** empty log message ***
* 
* Revision 36.22  91/01/25  15:45:56  rsbx
* Change to V37
* 
* Revision 36.21  90/05/30  14:02:55  rsbx
* Added kludge to zero the timeval of the timerequest before returning it,
* since most of the developers that used the timer.device can't read.
* 
* Revision 36.20  90/04/01  19:12:34  rsbx
* RCS version change.
* 
* Revision 36.19  90/02/16  11:55:51  rsbx
* Changes to improve MIDI performance.
* 
* Revision 36.18  89/11/23  16:49:43  rsbx
* Cleaned up some graphics kludges. Removed references to TR_WAITUNTIL and
* WaitUnitl() from to docs.
* 
* Revision 36.17  89/09/15  18:55:01  rsbx
* 
* 
* Revision 36.9  89/08/13  18:37:26  rsbx
* Fixed up comment blocks for autodoc.
* 
* Revision 36.8  89/08/09  19:13:08  rsbx
* changedsome comments for the benefit of autodoc
* 
* Revision 36.7  89/08/09  18:09:24  rsbx
* *** empty log message ***
* 
* Revision 36.5  89/08/09  14:05:50  rsbx
* Rewritten for the new timer.device
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
	INCLUDE	'exec/errors.i'

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
*$* graphics kludge
	XREF	_custom

	INT_ABLES

*------ Functions ----------------------------------------------------

	XREF	TermIO
	XREF	TermIOC
	XREF	Mult64
	XREF	ReadEClockTime
	XREF	GetSysTime
	XREF	TOD_Int

*------ System Library Functions -------------------------------------

	XSYS	Insert
	XSYS	Alert
	XSYS	ReplyMsg
	XSYS	SetICR
	XSYS	AbleICR
	XSYS	AddHead

*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF	NewTimer
	XDEF	RemTimer
	XDEF	GetSysTimeIO
	XDEF	SetSysTimeIO
	XDEF	InsertReq
	XDEF	IntEnd
	XDEF	IntCommon
	XDEF	AdjSTTOD
	XDEF	AdjSTEClock
	XDEF	Req2STTOD
	XDEF	Req2STEClock

*------ Data ---------------------------------------------------------

*------ Local Definitions --------------------------------------------



******* timer.device/TR_ADDREQUEST ***********************************
*
*   NAME
*	TR_ADDREQUEST -- Submit a request to wait a period of time.
*
*   FUNCTION
*	Ask the timer to wait a specified amount of time before
*	replying the timerequest.
*
*	The message may be forced to finish early with an
*	AbortIO()/WaitIO() pair.
*
*   TIMER REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       preset by timer in OpenDevice
*	io_Unit         preset by timer in OpenDevice
*	io_Command      TR_ADDREQUEST
*	io_Flags        IOF_QUICK permitted (but ignored)
*	tr_time         a timeval structure specifying how long the 
*	                    device will wait before replying
*
*   RESULTS
*	tr_time         will be zeroed
*
*   NOTES
*	This function may be called from interrupts.
*
*	Previous to 2.0, the tr_time field was documented as containing
*	junk when the timerequest was returned.
*
*   SEE ALSO
*	timer.device/AbortIO(),
*	timer.device/TimeDelay(),
*
*   BUGS
*
**********************************************************************
*****i* timer.device/internal/NewTimer *******************************
*
*   NAME
*	NewTimer -- Add a new timer request.
*
*   SYNOPSIS
*	NewTimer( IOBlock )
*	          A1
*
*	void NewTimer( struct timerequest * );
*
*   FUNCTION
*	This routine adds an IO request to the timer.  It runs in
*	the context of the requester.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*
*   RESULTS
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
*	A3 -- unit list (found/restored)
*	A2 -- munged (restored)
*	A1 -- request (given)
*	A0 -- munged

NewTimer:
*$* debug
*		ERRMSG	"NewTimer"
*$* debug
		bclr	#IOB_QUICK,IO_FLAGS(a1)

		movem.l	a2-a3,-(sp)

		move.l	IO_UNIT(a1),a3		; do unit specific addreqest
		move.l	TU_ADDREQ(a3),a0	; processing
		move.l	TU_UNITLIST(a3),a3	; get list
		jsr	(a0)

		movem.l	(sp)+,a2-a3
		rts


**********************************************************************
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- list (given)
*	A2 -- munged
*	A1 -- request (given)
*	A0 -- munged
*


InsertReq:
*$* debug
*		ERRMSG	"InsertReq"
*$* debug
		TD_DISABLE	a0,TD_SYSLIB(a6)

		move.l	a3,a0		; setup previous for loop
		move.l	(a3),d0		; setup current for loop
ILoop:
		move.l	a0,a2		; set previous
		move.l	d0,a0		; set current
		move.l	LN_SUCC(a0),d0	; get next
		beq.s	DoInsert	; end of list, insert it
		LTIMCMP	a1,a0,d1,IOTV_TIME,IOTV_TIME	; (a0)-(a1)
		bls.s	ILoop		; keep goin' 'till current > req
DoInsert:
		move.l	a3,a0
		move.l	a1,-(sp)
		LINKSYS	Insert		; list=a0, node=a1, previous=a2
		move.l	(sp)+,a1

		TD_ENABLE	a0,TD_SYSLIB(a6)

		rts



******* timer.device/AbortIO *****************************************
*
*   NAME
*	AbortIO -- Remove an existing timer request.
*
*   SYNOPSIS
*	error = AbortIO( timerequest )
*	D0               A1
*
*	LONG AbortIO( struct timerequest * );
*
*   FUNCTION
*	This is an exec.library call.
*
*	This routine removes a timerquest from the timer.  It runs in
*	the context of the caller.
*
*   INPUTS
*	timerequest - the timer request to be aborted
*
*   RETURNS
*	0  if the request was aborted, io_Error will also be set to
*	    IOERR_ABORTED.
*	-1 otherwise
*
*   NOTES
*	This function may be called from interrupts.
*
*   SEE ALSO
*	exec.library/AbortIO()
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
*	A3 -- unit list
*	A2 -- munged (restored)
*	A1 -- request (given)
*	A0 -- munged

RemTimer:
*$* debug
*		ERRMSG	"RemTimer"
*$* debug
		TD_DISABLE	A0,TD_SYSLIB(a6)

		;------ initialization step
		move.l	IO_UNIT(a1),a0
		move.l	TU_UNITLIST(a0),a0	; get unit list
		move.l	(a0),d0			; get first entry

RemTimer_Loop:
		move.l	d0,a0			; new current
		move.l	(a0),d0			; get next
		beq.s	RemTimer_NotFound	; list end
		cmpa.l	a0,a1			; same?
		bne.s	RemTimer_Loop		; nope, look again

RemTimer_Found:
*$* debug
*		ERRMSG	"RemTimer found"
*$* debug
		movem.l	a2/a3,-(sp)

		move.l	IO_UNIT(a1),a3		; do unit specific remreqest
		move.l	TU_REMREQ(a3),a0	; processing
		move.l	TU_UNITLIST(a3),a3	; get unit list (again)
		move.l	a1,-(sp)		; save request
		jsr	(a0)		; returns ENABLED
		move.l	(sp)+,a1

; reply aborted request
		move.b	#IOERR_ABORTED,IO_ERROR(A1)
		movem.l	(sp)+,a2/a3
		bsr	TermIOC			; reply the request

		clr.l	d0			; set abort result
		rts

RemTimer_NotFound:
*$* debug
*		ERRMSG	"RemTimer not found"
*$* debug
		TD_ENABLE	A0,TD_SYSLIB(a6)
		moveq.l	#-1,D0			; set abort result

		rts


**********************************************************************
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A3 -- list (given)
*	A1 -- munged
*	A0 -- unit specific version of current time (munged)
*

IntCommon:
		movem.l	a2,-(sp)
		move.l	a0,a2		; save time value

		TD_DISABLE	a0,TD_SYSLIB(a6)
ICLoop:
		cmpa.l	LH_TAIL+LN_PRED(a3),a3	; are we done?
		beq.s	ICDone

		move.l	(a3),a1		; get first list entry
		LTIMCMP	a1,a2,d1,IOTV_TIME,0	; (a2)-(a1)
		bcs.s	ICDone		; not expired, bye

		move.l	a3,a0
		REMHEAD			; a0=list, a1=node
		move.l	d0,a1		; get request (again)
		lea	TD_TERMIOQ(a6),a0
		ADDTAIL			; add request to reply Q
		bra.s	ICLoop		; check for more

ICDone:
		TD_ENABLE	A0,TD_SYSLIB(a6)

		movem.l	(sp)+,a2
		rts


**********************************************************************
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A1 -- munged
*	A0 -- munged
*

IntEnd:
		movem.l	(sp)+,a3

;  Process the TermIO queue created IntCommon
PIOLoop:
		lea	TD_TERMIOQ(a6),a0	; find reply Q
		TD_DISABLE	a1,TD_SYSLIB(a6)
		REMHEAD			; get first entry off Q
		TD_ENABLE	a1,TD_SYSLIB(a6)
		tst.l	d0		; test enpty Q
		beq.s	PIODone		; empty, we're done
		move.l	d0,a1
		bsr	TermIOC		; reply the request
		bra.s	PIOLoop		; look for more

PIODone:

*$* graphics kludge
		lea	_custom,a0


		;------ leave the condition codes set correctly
*		moveq.l	#0,d0

		rts


**********************************************************************


******* timer.device/TR_SETSYSTIME ***************************
*
*   NAME
*	TR_SETSYSTIME -- Set the system time.
*
*   FUNCTION
*	Set the system idea of what time it is.  The system starts out
*	at time "zero" so it is safe to set it forward to the real
*	time.  However, care should be taken when setting the time
*	backwards.  System time is generally expected to monotonically
*	increasing.
*
*   TIMER REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       preset by timer in OpenDevice
*	io_Unit         preset by timer in OpenDevice
*	io_Command      TR_GETSYSTIME
*	io_Flags        IOF_QUICK permitted
*	tr_time         a timeval structure with the current system
*	                    time
*
*   RESULTS
*	tr_time         will contain junk
*
*   NOTES
*	This function may be called from interrupts.
*
*   SEE ALSO
*	timer.device/TR_GETSYSTIME,
*	timer.device/GetSysTime(),
*
*   BUGS
*
**********************************************************************
*****i* timer.device/internal/SetSysTimeIO ***************************
*
*   NAME
*	SetSysTimeIO -- Set the system time.
*
*   SYNOPSIS
*	SetSysTimeIO( IORequest )
*	              A1
*
*	void SetSysTimeIO( struct timerequest * );
*
*   FUNCTION
*	This routine sets the system time.  Be careful of setting the
*	time backwards!
*
*   INPUTS
*	IOTimeRequest -- an IOTV request block for the timer
*
*   RESULTS
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
*	A1 -- request (given)
*	A0 -- time field of request
*

SetSysTimeIO:
*$* debug
*		ERRMSG	"SetSysTimeIO"
*$* debug
		movem.l	a1-a2,-(sp)
		lea	IOTV_TIME(a1),a0	; find time field
		jsr	Req2STfmt(a6)		; convert
		jsr	AdjSTfmt(a6)		; adjust

		TD_DISABLE	a2,TD_SYSLIB(a6)
		move.l	TV_SECS(a0),TD_SYSTIME+TV_SECS(a6)
		move.l	TV_MICRO(a0),TD_SYSTIME+TV_MICRO(a6)
		TD_ENABLE	a2,TD_SYSLIB(a6)

		movem.l	(sp)+,a1-a2

		bra	TermIOC			; reply
		; RFT


**********************************************************************
*
* mode specific systime field adjust
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*

AdjSTEClock:
		movem.l	a0,-(sp)
		subq.l	#EV_SIZE,sp
		move.l	sp,a0
		bsr	ReadEClockTime		; get E time
		move.l	EV_LO(a0),d0		; lo nibble
		addq.l	#EV_SIZE,sp
		movem.l	(sp)+,a0

		swap	d0			; zero hi nibble
		move.w	#0,d0
		swap	d0

		move.l	TV_MICRO(a0),d1		; adjust systime base
		move.l	#0,TV_MICRO(a0)		;  against E time
		sub.l	d0,d1
		bcc.s	NoAdj

		move.l	TV_SECS(a0),d0
		subq.l	#1,d0
		bcs.s	Unset			; setting time to 0

		move.l	d0,TV_SECS(a0)
		add.l	TD_ECLOCKHERTZ(a6),d1
NoAdj:
		move.l	d1,TV_MICRO(a0)
Unset:
		rts

		
**********************************************************************
*
* mode specific systime field adjust
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*

AdjSTTOD:
		movem.l	a0/a6,-(sp)
		move.l	a6,a1
		bsr	TOD_Int			; clears TOD int and counter
		movem.l	(sp)+,a0/a6
		rts


**********************************************************************
*
* timerequest to mode specific systime conversion
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*

Req2STEClock:
		; this one returns secs/etics
		move.l	TV_MICRO(a0),d0		
		move.l	TD_ECLOCKCONST1(a6),d1
		bsr	Mult64			; convert usecs to etics
		move.w	d0,d1
		swap	d1			; Etics
		move.l	d1,TV_MICRO(a0)
		rts


**********************************************************************
*
* timerequest to mode specific systime conversion
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDDev (given)
*	A0 -- time field of request (given)
*

Req2STTOD:
		; this on returns secs/TOD-tics
		move.l	TV_MICRO(a0),d1
		divu.w	TD_TIMEPERTOD(a6),d1	; usecs to TOD-tics
		move.l	d1,d0
		ext.l	d1		; whole tics in d1

		swap	d0		; this block is the ceiling function
		neg.w	d0
		neg.w	d0		; X set if non-zero
		clr.l	d0		; X not affected
		addx.l	d0,d1		; add X to tics
		move.l	d1,TV_MICRO(a0)
		rts


******* timer.device/TR_GETSYSTIME ***********************************
*
*   NAME
*	TR_GETSYSTIME -- get the system time.
*
*   FUNCTION
*	Ask the system what time it is.  The system time starts off at
*	zero at power on, but may be initialized via the TR_SETSYSTIME
*	call.
*
*	System time is monotonically increasing, and guaranteed to be
*	unique (except when the system time is set backwards).
*
*   TIMER REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       preset by timer in OpenDevice
*	io_Unit         preset by timer in OpenDevice
*	io_Command      TR_GETSYSTIME
*	io_Flags        IOF_QUICK permitted
*
*   RESULTS
*	tr_time         a timeval structure with the current system
*	                    time
*
*   NOTES
*	This function may be called from interrupts.
*
*   SEE ALSO
*	timer.device/TR_SETSYSTIME,
*	timer.device/GetSysTime(),
*
*   BUGS
*
**********************************************************************
*****i* timer.device/internal/GetSysTimeIO ***************************
*
*   NAME
*	GetSysTimeIO -- Get the current system time.
*
*   SYNOPSIS
*	GetSysTimeIO( IOTimeRequest )
*	              A1
*
*	void GetSysTimeIO( struct timerequest * );
*
*   FUNCTION
*	This routine puts the current system time into the timeval
*	structure of the request.  This time is guaranteed to be
*	larger than the last time GetSysTime was called (e.g. it
*	is monotonically increasing).  In addition, the time is
*	updated every vertical blank, so time is discontinuous --
*	it will take large jumps every vertical blank.
*
*   INPUTS
*	IOTimeRequest -- an IOTV request block for the timer
*
*   RESULTS
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
*

GetSysTimeIO:
*$* debug
*		ERRMSG	"GetSysTimeIO"
*$* debug
		move.l	a1,-(sp)
		lea	IOTV_TIME(a1),a0	; find time field
		bsr	GetSysTime		; do it
		move.l	(sp)+,a1

		;------ and send it back to him
		bra	TermIO			; reply the request
		; RFT


	END
