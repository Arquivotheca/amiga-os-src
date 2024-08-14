**
**	$Id: stop.asm,v 36.5 90/11/02 08:12:38 darren Exp $
**
**      console device start/stop commands
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/errors.i"


**	Exports

	XDEF	Invalid
	XDEF	ResetCmd
	XDEF	StopCmd
	XDEF	Start
	XDEF	Flush

	XDEF	CBInvalid
	XDEF	CBResetCmd
	XDEF	CBStopCmd
	XDEF	CBStart
	XDEF	CBFlush


**	Imports

	XLVO	Disable			; Exec
	XLVO	Enable			;
	XLVO	PutMsg			;
	XLVO	ReplyMsg		;

	XREF	AbortIO
	XREF	EndCommand


*------ console.device/CMD_INVALID -----------------------------------
*
*   NAME
*	Invalid - invalid command
*
*   FUNCTION
*	Invalid is always an invalid command, and sets the device
*	error appropriately.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Command	CMD_INVALID
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
*---------------------------------------------------------------------
CBInvalid	EQU	-1

Invalid:
		moveq	#IOERR_NOCMD,d0
		bsr	EndCommand
		rts

*------ console.device/CMD_RESET -------------------------------------
*
*   NAME
*	Reset - reset the device
*
*   FUNCTION
*	Reset resets the device without destroying handles
*	to the open device.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_RESET
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
*---------------------------------------------------------------------
CBResetCmd	EQU	-1

ResetCmd:
		bsr.s	StopCmd
		bsr.s	Flush
		bsr.s	Start
		rts


*------ console.device/CMD_STOP --------------------------------------
*
*   NAME
*	Stop - pause current and queued I/O requests (immediate)
*
*   FUNCTION
*	Stop pauses all queued requests for the unit, and tries to
*	pause the current I/O request.  The only commands that will
*	be subsequently allowed to be performed are immediate I/O
*	requests, which include those to start, flush, and finish the
*	I/O after the stop command.
*
*	For the console, only the Read command is stopped --
*	all other commands are immediate.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_STOP
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
*---------------------------------------------------------------------
CBStopCmd	EQU	-1

StopCmd:
		move.l	IO_UNIT(a1),a0
		bset	#DUB_STOPPED,du_Flags(a0)
		bsr	EndCommand
		rts


*------ console.device/CMD_START -------------------------------------
*
*   NAME
*	Start - restart after stop (immediate)
*
*   FUNCTION
*	Start restarts the unit after a stop command.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_START
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
*---------------------------------------------------------------------
CBStart	EQU	-1

Start:
		move.l	a1,-(a7)
		move.l	IO_UNIT(a1),a0
		bclr	#DUB_STOPPED,du_Flags(a0)
	    ;-- if I/O is already pending, see if this will satisfy it
		move.l	MP_MSGLIST(a0),a1
		tst.l	(a1)
		beq.s	sRts

		move.w	IO_COMMAND(a1),d0
		lsl.w	#2,d0
		move.l	cd_CmdVectors(a6),a0
		move.l	0(a0,d0.w),a0
		jsr	(a0)

sRts:
		move.l	(a7)+,a1
		bsr	EndCommand
		rts


*------ console.device/CMD_FLUSH -------------------------------------
*
*   NAME
*	Flush - abort all I/O requests (immediate)
*
*   FUNCTION
*	Flush aborts all stopped I/O at the unit.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_FLUSH
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
*---------------------------------------------------------------------
CBFlush	EQU	-1

Flush:
		movem.l	a1/a2,-(a7)
		move.l	IO_UNIT(a1),a2
fLoop:
		move.l	MP_MSGLIST(a2),a1
		tst.l	(a1)
		beq.s	fRts
		bsr	AbortIO
		bra.s	fLoop

fRts:
		movem.l	(a7)+,a1/a2
		bsr	EndCommand
		rts

	END
