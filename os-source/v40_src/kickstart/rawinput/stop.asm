	TTL    '$Id: stop.asm,v 35.2 90/04/13 12:45:56 kodiak Exp $
**********************************************************************
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*	rawinput device start/stop commands
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
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/errors.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"macros.i"
	INCLUDE		"stddevice.i"


*------ Imported Names -----------------------------------------------

*------ Imported Functions -------------------------------------------

	XREF_EXE	Disable
	XREF_EXE	Enable
	XREF_EXE	PutMsg
	XREF_EXE	ReplyMsg

	XREF		AbortIO
	XREF		EndCommand


*------ Exported Functions -------------------------------------------

	XDEF		Invalid
	XDEF		ResetCmd
	XDEF		StopCmd
	XDEF		Start
	XDEF		Flush

	XDEF		CBInvalid
	XDEF		CBResetCmd
	XDEF		CBStopCmd
	XDEF		CBStart
	XDEF		CBFlush


*------ rawinput.device/Invalid --------------------------------------
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
		MOVEQ	#IOERR_NOCMD,D0
		BSR	EndCommand
		RTS

*------ rawinput.device/Reset ----------------------------------------
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
		BSR	StopCmd
		BSR	Flush
		BSR	Start
		RTS


*------ rawinput.device/Stop -----------------------------------------
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
*	For the rawinput, only the ReadEvent command is stopped --
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
		MOVE.L	IO_UNIT(A1),A0
		BSET	#DUB_STOPPED,du_Flags(A0)
		BSR	EndCommand
		RTS


*------ rawinput.device/Start ----------------------------------------
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
		MOVE.L	A1,-(A7)
		MOVE.L	IO_UNIT(A1),A0
		BCLR	#DUB_STOPPED,du_Flags(A0)
	    ;-- if I/O is already pending, see if this will satisfy it
		MOVE.L	MP_MSGLIST(A0),A1
		TST.L	(A1)
		BEQ.S	sRts

		MOVE.W	IO_COMMAND(A1),D0
		LSL.W	#2,D0
		MOVE.L	dd_CmdVectors(A6),A0
		MOVE.L	0(A0,D0.W),A0
		JSR	(A0)

sRts:
		MOVE.L	(A7)+,A1
		BSR	EndCommand
		RTS


*------ rawinput.device/Flush ----------------------------------------
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
		MOVEM.L	A1/A2,-(A7)
		MOVE.L	IO_UNIT(A1),A2
fLoop:
		MOVE.L	MP_MSGLIST(A2),A1
		TST.L	(A1)
		BEQ.S	fRts
		BSR	AbortIO
		BRA.S	fLoop

fRts:
		MOVEM.L	(A7)+,A1/A2
		BSR	EndCommand
		RTS

		END
