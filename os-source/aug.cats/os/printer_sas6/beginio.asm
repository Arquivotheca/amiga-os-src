	TTL    '$Id: beginio.asm,v 1.5 90/07/25 21:25:46 valentin Exp $'
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
*   Source Control
*   --------------
*   $Id: beginio.asm,v 1.5 90/07/25 21:25:46 valentin Exp $
*
*   $Locker:  $
*
*   $Log:	beginio.asm,v $
*   Revision 1.5  90/07/25  21:25:46  valentin
*   $Header changed to $Id
*   
*   Revision 1.4  90/07/25  17:33:11  valentin
*   RCS directory change
*   
*   Revision 1.4  90/07/25  09:33:44  valentin
*   AbortIO() was completely screwed up. It now checks whether the request
*   is active or merely queued, and does not WaitIO() on requests that it
*   aborts and which belong to the printer device task.
*   
*   Revision 1.3  90/04/06  19:22:48  daveb
*   for rcs 4.x header change
*   
*   Revision 1.2  88/06/14  16:31:33  daveb
*   changed AbortIO to cancel the parallel/serial/timer requests
*   V1.3 Gamma 18
*   
*   Revision 1.1  87/09/09  06:53:46  daveb
*   changed AbortIO code so that it waits for the abort to finish
*   
*   Revision 1.0  87/08/21  17:25:12  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:36:25  daveb
*   added to rcs
*   
*   Revision 1.1  86/09/15  14:32:19  andy
*   *** empty log message ***
*   
*   Revision 1.0  86/09/11  20:18:43  andy
*   added to rcs
*   
*   Revision 1.2  85/10/10  00:26:55  kodiak
*   remove +du_CmdQueues from unit variable names
*   
*   Revision 1.1  85/10/09  23:59:21  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/09  23:59:14  kodiak
*   added to rcs for updating in version 1
*   
*   Revision 1.0  85/09/25  15:40:41  kodiak
*   added to rcs for updating in version 1
*   
*   Revision 1.0  85/09/03  18:23:16  kodiak
*   added to rcs for updating in version 1
*   
*   Revision 25.1  85/06/16  01:04:06  kodiak
*   *** empty log message ***
*   
*   Revision 25.0  85/06/13  18:52:50  kodiak
*   added to rcs
*   
*   Revision 1.0  85/04/23  18:32:13  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

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
	INCLUDE		"prtbase.i"


*------ Imported Names -----------------------------------------------

*------ Imported Functions -------------------------------------------

	XREF_EXE	Disable
	XREF_EXE	Enable
	XREF_EXE	Forbid
	XREF_EXE	Permit
	XREF_EXE	PutMsg
	XREF_EXE	ReplyMsg
	XREF_EXE	Wait
	XREF_EXE	WaitIO
	XREF_EXE	AbortIO
	XREF		_SysBase
	XREF		cancel
	XREF		abortTimeout

*------ Exported Functions -------------------------------------------

	XDEF		BeginIO
	XDEF		AbortIO
	XDEF		EndCommand


*------ Device/BeginIO -----------------------------------------------
*
*   NAME
*	BeginIO - dispatch a device command
*
*   SYNOPSIS
*	BeginIO(iORequest), deviceBase
*                  A1          A6
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

		LINKEXE	Forbid

		TST.B	IO_ERROR(A3)	; don't queue it if already error
		BNE.S	queuePermit

		;------	queue the I/O request
		LEA	pd_Unit(A6),A0
		MOVE.L	A3,A1
		LINKEXE PutMsg

		BSET	#IOB_QUEUED,IO_FLAGS(A3)
		BCLR	#IOB_QUICK,IO_FLAGS(A3)

queuePermit:
		LINKEXE	Permit

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


*------ Device/AbortIO -----------------------------------------------
*
*   NAME
*	AbortIO - abort a device command
*
*   SYNOPSIS
*	AbortIO(iORequest), deviceBase
*                   A1          A6
*
*   FUNCTION
*	AbortIO will try to abort a device command.  It is allowed to
*	not be successful.
*
*   INPUTS
*	iORequest -- the I/O Request for the command to abort.
*
*---------------------------------------------------------------------
*
*   If this request is active, then we abort the parallel (or serial)
*   requests, which in turn cause the printer task to abort the timer
*   request and reply to this one. Hopefully, both parallel (serial)
*   requests were initiated for the current printer request.
*  
*   If this request is not active, but rather in the queue, then we
*   remove it from that queue, and perform a ReplyMsg on it ourselves.
*  
*   Multiple IO requests to the printer device have never been tried
*   up to now (25 Jul 90), therefore it may take some time until
*   multiple IO requests to the printer device are fully tested out.
*  
*   Valentin
*
*---------------------------------------------------------------------

AbortIO:
	move.l	a3,-(sp)			; Save this register
	move.l	a1,a3				; Put IO request here
	LINKEXE	Disable

	BTST	#IOB_DONE,IO_FLAGS(A3)		; Is this request already done?
	BNE.S	abort_end			; Branch if so
	BTST	#IOB_CURRENT,IO_FLAGS(a3)	; Is the request active?
	BEQ.S	abort_remove			; Branch if not
	MOVE.B	#IOERR_ABORTED,IO_ERROR(A1)	; Mark it as aborted

	; Tell the parallel or serial device to give up on all
	; current/pending requests. This will cause the printer device
	; to cancel the timer timeout request and reply to this
	; printer request.

	BCLR	#PB_IOR0,pd_Flags(A6)	;is req1 in use?
	BEQ.S	abort_req2		;no
	LEA	pd_IOR0(A6),A1		;yes, get ptr in A1
	LINKEXE	AbortIO			;abort req1

abort_req2:
	BCLR	#PB_IOR1,pd_Flags(A6)	;is req2 in use?
	BEQ.S	abort_end		;no
	LEA	pd_IOR1(A6),A1		;yes, get ptr in A1
	LINKEXE	AbortIO			;abort req2

abort_end:
	LINKEXE Enable	
	MOVEQ.L	#0,D0			; flag successful abort
	MOVE.L	(sp)+,a3
	RTS

abort_remove:
	;------ is this command in the queue?
	BTST	#IOB_QUEUED,IO_FLAGS(A3)
	BEQ.S	abort_selfreply

	;------ REMOVE
	MOVE.L	(A3),A0
	MOVE.L	LN_PRED(A3),A1
	MOVE.L	A0,(A1)
	MOVE.L	A1,LN_PRED(A0)

abort_selfreply:
	;------  is this command still quick?
	BTST	#IOB_QUICK,IO_FLAGS(A3)
	BNE.S	abort_end

	;---------- reply the command
	MOVE.L	A3,A1
	LINKEXE	ReplyMsg
	BRA.S	abort_end


*------ EndCommand ---------------------------------------------------
*
*   NAME
*	EndCommand - reply a device command
*
*   SYNOPSIS
*	EndCommand(error, iORequest), deviceBase
*                   D0        A1          A6
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
		BNE.S	endEnable

		MOVE.B	D2,IO_ERROR(A1)

		;------ is this command in the queue?
		BTST	#IOB_QUEUED,IO_FLAGS(A3)
		BEQ.S	endReply

		;------ REMOVE
		MOVE.L	(A3),A0
		MOVE.L	LN_PRED(A3),A1
		MOVE.L	A0,(A1)
		MOVE.L	A1,LN_PRED(A0)

endReply:
		;------  is this command still quick?
		BTST	#IOB_QUICK,IO_FLAGS(A3)
		BNE.S	endEnable

		;---------- reply the command
		MOVE.L	A3,A1
		LINKEXE	ReplyMsg

endEnable:
		LINKEXE	Enable

		MOVEM.L	(SP)+,D2/A3
		RTS

		END
