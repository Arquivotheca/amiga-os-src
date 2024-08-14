	TTL    '$Id: beginio.asm,v 35.4 91/04/09 13:43:34 darren Exp $
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
*	device dispatch code
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


*------ Exported Functions -------------------------------------------

	XDEF		BeginIO
	XDEF		AbortIO
	XDEF		QueueCommand
	XDEF		EndCommand


*------ Device/BeginIO -----------------------------------------------
*
*   NAME
*	BeginIO - dispatch a device command
*
*   SYNOPSIS
*	BeginIO(iORequest), deviceBase
*		A1	    A6
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
		MOVEM.L	D2/A3,-(SP)
		MOVE.L	A1,A3		; save the I/O Request
IO_TEMP		EQU	(~(IOF_QUEUED!IOF_CURRENT!IOF_SERVICING!IOF_DONE))&$0ff
		AND.B	#IO_TEMP,IO_FLAGS(A3)	
		CLR.B	IO_ERROR(A3)	; clear the error field

		;------ is the command in bounds?
		MOVE.W	IO_COMMAND(A3),D2
		CMP.W	dd_NumCommands(A6),D2
		BLT.S	cmdExists
		MOVEQ	#0,D2

cmdExists:
		MOVE.L	dd_CmdBytes(A6),A0
		TST.B	0(A0,D2.W)
		BMI.S	performCmd	; it's guarenteed immediate

		BSR.S	QueueCommand	; queue the command

		;------ is this the first command in its queue?
		BTST	#IOB_CURRENT,IO_FLAGS(A3)
		BEQ.S	notQuickCmd

		;------ is the unit running?
		MOVE.L	IO_UNIT(A3),A0
		BTST	#DUB_STOPPED,du_Flags(A0)
		BEQ.S	performCmd

notQuickCmd:
		BCLR	#IOB_QUICK,IO_FLAGS(A3)
		BRA.S	endBIO

performCmd:
		LSL.W	#2,D2
		MOVE.L	dd_CmdVectors(A6),A0
		MOVE.L	0(A0,D2.W),A0
		MOVE.L	A3,A1
		JSR	(A0)

endBIO:
		MOVEM.L	(SP)+,D2/A3
		RTS


*------ QueueCommand -------------------------------------------------
*
*   NAME
*	QueueCommand - queue a device command
*
*   SYNOPSIS
*	QueueCommand(iORequest)
*		     A1
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
		MOVE.L	A3,-(SP)
		MOVE.L	A1,A3
		LINKEXE	Disable

		ANDI.B	#~(IOF_CURRENT!IOF_QUEUED),IO_FLAGS(A3)

		TST.B	IO_ERROR(A3)	; don't queue it if already error
		BNE.S	queueEnable

***********************************************************************
**
** Original code here - needed if CmdByte > 0, but in the case of
** rawinput its not needed, and we need the speed here for MIDI
** performance.
**
**		;------	queue the I/O request
**		MOVE.W	IO_COMMAND(A3),D0
**		MOVE.L	dd_CmdBytes(A6),A0
**		MOVE.B	0(A0,D0.W),D0
**		EXT.W	D0
**		MULU	#MP_SIZE,D0
**		MOVE.L	IO_UNIT(A3),A0
**		ADD.L	D0,A0
**		MOVE.L	A0,-(A7)
**		MOVE.L	A3,A1
**		LINKEXE PutMsg

		;------ queue the I/O request

		MOVE.L	IO_UNIT(A3),A0	;msg port is first
		MOVE.L	A3,A1		;message
		LINKEXE	PutMsg

		;------	check if the I/O request is first in the queue
		MOVE.L	IO_UNIT(A3),A0
		MOVE.L	MP_MSGLIST+LH_HEAD(A0),A0
		CMPA.L	A0,A3
		BEQ.S	queueCurrent

		BSET	#IOB_QUEUED,IO_FLAGS(A3)
		BRA.S	queueEnable

queueCurrent:
		BSET	#IOB_CURRENT,IO_FLAGS(A3)

queueEnable:
		LINKEXE	Enable
		MOVE.L	(SP)+,A3
		RTS


*------ Device/AbortIO -----------------------------------------------
*
*   NAME
*	AbortIO - abort a device command
*
*   SYNOPSIS
*	AbortIO(iORequest), deviceBase
*		A1	    A6
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
		MOVEQ	#IOERR_ABORTED,D0
		BSR.S	EndCommand
		RTS


*------ EndCommand ---------------------------------------------------
*
*   NAME
*	EndCommand - reply a device command
*
*   SYNOPSIS
*	EndCommand(error, iORequest), deviceBase
*		   D0	  A1	      A6
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
		MOVEM.L	D2/A3,-(SP)
		MOVE.L	D0,D2
		MOVE.L	A1,A3

		LINKEXE	Disable

		BSET	#IOB_DONE,IO_FLAGS(A3)
		BNE.S	endAlreadyDone

		MOVE.B	D2,IO_ERROR(A1)

		;------ is this command in the queue?
		BTST	#IOB_CURRENT,IO_FLAGS(A3)
		BEQ.S	endNotCurrent

		;------ REMOVE
		MOVE.L	(A3),A0
		MOVE.L	LN_PRED(A3),A1
		MOVE.L	A0,(A1)
		MOVE.L	A1,LN_PRED(A0)

		TST.L	(A0)		; is this list now empty?
		BEQ.S	endReply

		BSET	#IOB_CURRENT,IO_FLAGS(A0)
		BRA.S	endReply

endNotCurrent:
		BTST	#IOB_QUEUED,IO_FLAGS(A3)
		BEQ.S	endReply

		;---------- remove the command from the queue
		MOVE.L	A3,A1
		REMOVE

endReply:
		LINKEXE	Enable
		;------  is this command still quick?
		BTST	#IOB_QUICK,IO_FLAGS(A3)
		BNE.S	endDone

		;---------- reply the command
		MOVE.L	A3,A1
		LINKEXE	ReplyMsg

endDone:
		MOVEM.L	(SP)+,D2/A3
		RTS

endAlreadyDone:
		LINKEXE	Enable
		bra.s	endDone

		END
