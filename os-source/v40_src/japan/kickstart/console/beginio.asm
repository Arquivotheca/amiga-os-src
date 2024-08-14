**
**	$Id: beginio.asm,v 36.5 90/06/07 12:10:54 kodiak Exp $
**
**      device command dispatch
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/errors.i"


**	Exports

	XDEF	BeginIO
	XDEF	AbortIO
	XDEF	QueueCommand
	XDEF	EndCommand


**	Imports

	XLVO	Disable			; Exec
	XLVO	Enable			;
	XLVO	PutMsg			;
	XLVO	ReplyMsg		;


**	Assumptions

	IFNE	IOB_CURRENT-7
	FAIL	"IOB_CURRENT not sign bit, recode"
	ENDC


*------ Device/BeginIO -----------------------------------------------
*
*   NAME
*	BeginIO - dispatch a device command
*
*   SYNOPSIS
*	BeginIO(iORequest), deviceBase
*		a1	    a6
*
*   FUNCTION
*	BeginIO has the responsibility of dispatching all device
*	commands.  Immediate commands are always called directly, and
*	all other commands are queued to make them single threaded.
*
*   INPUTS
*	iORequest -- the I/O Request for this command.
*
*---------------------------------------------------------------------
BeginIO:
		movem.l	d2/a3,-(sp)
		move.l	a1,a3		; save the I/O Request
IO_TEMP		EQU	(~(IOF_QUEUED!IOF_CURRENT!IOF_SERVICING!IOF_DONE))&$0ff
		and.b	#IO_TEMP,IO_FLAGS(a3)	
		clr.b	IO_ERROR(a3)	; clear the error field

		;------ is the command in bounds?
		move.w	IO_COMMAND(a3),d2
		cmp.w	cd_NumCommands(a6),d2
		blt.s	cmdExists
		moveq	#0,d2

cmdExists:
		move.l	cd_CmdBytes(a6),a0
		tst.b	0(a0,d2.w)
		bmi.s	performCmd	; it's guarenteed immediate

		bsr.s	QueueCommand	; queue the command

		;------ is this the first command in its queue?
		tst.b	IO_FLAGS(a3)	; btst #IOB_CURRENT,
		bpl.s	notQuickCmd

		;------ is the unit running?
		move.l	IO_UNIT(a3),a0
		btst	#DUB_STOPPED,du_Flags(a0)
		beq.s	performCmd

notQuickCmd:
		bclr	#IOB_QUICK,IO_FLAGS(a3)
		bra.s	endBIO

performCmd:
		lsl.w	#2,d2
		move.l	cd_CmdVectors(a6),a0
		move.l	0(a0,d2.w),a0
		move.l	a3,a1
		jsr	(a0)

endBIO:
		movem.l	(sp)+,d2/a3
		rts


*------ QueueCommand -------------------------------------------------
*
*   NAME
*	QueueCommand - queue a device command
*
*   SYNOPSIS
*	QueueCommand(iORequest)
*		     a1
*
*   FUNCTION
*	Queue a device command for later processing by the device,
*	unless the command error is non-zero.
*
*   INPUTS
*	iORequest -- the I/O Request for the command to queue.
*
*   RESULTS
*	The command is marked CURRENT if it is the first one in the
*	queue, else it is marked QUEUED.
*
*---------------------------------------------------------------------
QueueCommand:
		move.l	a3,-(sp)
		move.l	a1,a3
		LINKEXE	Disable

		andi.b	#(~(IOF_CURRENT!IOF_QUEUED))&$ff,IO_FLAGS(a3)

		tst.b	IO_ERROR(a3)	; don't queue it if already error
		bne.s	queueEnable

		;------	queue the I/O request
		move.w	IO_COMMAND(a3),d0
		move.l	cd_CmdBytes(a6),a0
		move.b	0(a0,d0.w),d0
		ext.w	d0
		mulu	#MP_SIZE,d0
		move.l	IO_UNIT(a3),a0
		add.l	d0,a0
		move.l	a0,-(a7)
		move.l	a3,a1
		LINKEXE PutMsg

		;------	check if the I/O request is first in the queue
		move.l	(a7)+,a0
		move.l	MP_MSGLIST+LH_HEAD(a0),a0
		cmpa.l	a0,a3
		beq.s	queueCurrent

		bset	#IOB_QUEUED,IO_FLAGS(a3)
		bra.s	queueEnable

queueCurrent:
		bset	#IOB_CURRENT,IO_FLAGS(a3)

queueEnable:
		LINKEXE	Enable
		move.l	(sp)+,a3
		rts


*------ Device/AbortIO -----------------------------------------------
*
*   NAME
*	AbortIO - abort a device command
*
*   SYNOPSIS
*	AbortIO(iORequest), deviceBase
*		a1	    a6
*
*   FUNCTION
*	AbortIO will try to abort a device command.  It is allowed to
*	not be successful.
*
*   INPUTS
*	iORequest -- the I/O Request for the command to abort.
*
*---------------------------------------------------------------------
AbortIO:
		moveq	#IOERR_ABORTED,d0
		bsr.s	EndCommand
		rts


*------ EndCommand ---------------------------------------------------
*
*   NAME
*	EndCommand - reply a device command
*
*   SYNOPSIS
*	EndCommand(error, iORequest), deviceBase
*		   d0	  a1	      a6
*
*   FUNCTION
*	EndCommand will reply to the command invoker that the command
*	has completed.  Only the first EndCommand for a command will
*	have effect, and set the error of the request according to 
*	the error parameter.
*
*   INPUTS
*	error -- the error to report, if any
*	iORequest -- the I/O Request for the command to end
*
*---------------------------------------------------------------------
EndCommand:
		movem.l	d2/a3,-(sp)
		move.l	d0,d2
		move.l	a1,a3

		LINKEXE	Disable

		bset	#IOB_DONE,IO_FLAGS(a3)
		bne.s	endAlreadyDone

		move.b	d2,IO_ERROR(a1)

		;------ is this command in the queue?
		tst.b	IO_FLAGS(a3)	; btst #IOB_CURRENT,
		bpl.s	endNotCurrent

		;------ REMOVE
		move.l	(a3),a0
		move.l	LN_PRED(a3),a1
		move.l	a0,(a1)
		move.l	a1,LN_PRED(a0)

		tst.l	(a0)		; is this list now empty?
		beq.s	endReply

		bset	#IOB_CURRENT,IO_FLAGS(a0)
		bra.s	endReply

endNotCurrent:
		btst	#IOB_QUEUED,IO_FLAGS(a3)
		beq.s	endReply

		;---------- remove the command from the queue
		move.l	a3,a1
		REMOVE

endReply:
		LINKEXE	Enable
		;------  is this command still quick?
		btst	#IOB_QUICK,IO_FLAGS(a3)
		bne.s	endDone

		;---------- reply the command
		move.l	a3,a1
		LINKEXE	ReplyMsg

endDone:
		movem.l	(sp)+,d2/a3
		rts

endAlreadyDone:
		LINKEXE	Enable
		bra.s	endDone

		END
