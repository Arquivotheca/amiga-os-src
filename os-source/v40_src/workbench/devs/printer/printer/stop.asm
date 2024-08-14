	TTL    '$Id: stop.asm,v 1.5 91/02/14 15:26:26 darren Exp $'
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
*	device dispatch commands
*
*   Source Control
*   --------------
*   $Id: stop.asm,v 1.5 91/02/14 15:26:26 darren Exp $
*
*   $Locker:  $
*
*   $Log:	stop.asm,v $
*   Revision 1.5  91/02/14  15:26:26  darren
*   Autodoc stuff
*   
*   Revision 1.4  90/07/27  02:19:36  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.3  90/04/06  19:25:37  daveb
*   for rcs 4.x header change
*   
*   Revision 1.2  88/08/16  16:30:04  daveb
*   made changes to autodoc(s)
*   
*   Revision 1.1  87/10/01  09:37:25  daveb
*   V1.3 beta 4 check-in
*   
*   Revision 1.0  87/08/21  17:28:56  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:47:06  daveb
*   added to rcs
*   
*   Revision 1.2  85/10/10  00:30:17  kodiak
*   remove +du_CmdQueues from pd_Unit reference
*   
*   Revision 1.1  85/10/10  00:00:32  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/10  00:00:22  kodiak
*   added to rcs for updating in version 1
*   
*   Revision 25.0  85/06/13  18:53:40  kodiak
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
	XREF_EXE	PutMsg
	XREF_EXE	ReplyMsg

	XREF		AbortIO
	XREF		EndCommand


*------ Exported Functions -------------------------------------------

	XDEF		PCInvalid
	XDEF		PCStop
	XDEF		PCStart
	XDEF		PCFlush

	XDEF		PCBInvalid
	XDEF		PCBStop
	XDEF		PCBStart
	XDEF		PCBFlush


******* printer.device/CMD_INVALID ***************************************
*
*   NAME
*	CMD_INVALID -- invalid command
*
*   FUNCTION
*	CMD_INVALID is always an invalid command, and sets the device
*	error appropriately.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Command	CMD_INVALID
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
PCBInvalid	EQU	-1

PCInvalid:
		MOVEQ	#IOERR_NOCMD,D0
		BSR	EndCommand
		RTS


******* printer.device/CMD_STOP ******************************************
*
*   NAME
*	CMD_STOP -- pause current and queued I/O requests (immediate)
*
*   FUNCTION
*	CMD_STOP pauses all queued requests for the unit, and tries to
*	pause the current I/O request.  The only commands that will
*	be subsequently allowed to be performed are immediate I/O
*	requests, which include those to start, flush, and finish the
*	I/O after the stop command.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	CMD_STOP
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
PCBStop	EQU	-1

PCStop:
		BSET	#DUB_STOPPED,pd_Unit+du_Flags(A6)
		RTS


******* printer.device/CMD_START *****************************************
*
*   NAME
*	CMD_START -- restart after stop (immediate)
*
*   FUNCTION
*	CMD_START restarts the unit after a stop command.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	CMD_START
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
PCBStart	EQU	-1

PCStart:
		BCLR	#DUB_STOPPED,pd_Unit+du_Flags(A6)
		RTS


******* printer.device/CMD_FLUSH *****************************************
*
*   NAME
*	CMD_FLUSH -- abort all I/O requests (immediate)
*
*   FUNCTION
*	CMD_FLUSH aborts all stopped I/O at the unit.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	CMD_FLUSH
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
PCBFlush	EQU	-1

PCFlush:
		MOVE.L	pd_Unit+MP_MSGLIST(A6),A1
		TST.L	(A1)
		BEQ.S	fRts
		BSR	AbortIO
		BRA.S	PCFlush

fRts:
		RTS

		END
