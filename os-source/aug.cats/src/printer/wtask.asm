	TTL    '$Id: wtask.asm,v 1.4 90/10/04 13:30:36 darren Exp $'
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
*	write task
*
*   Source Control
*   --------------
*   $Id: wtask.asm,v 1.4 90/10/04 13:30:36 darren Exp $
*
*   $Locker:  $
*
*   $Log:	wtask.asm,v $
*   Revision 1.4  90/10/04  13:30:36  darren
*   Put reply port, and IOExtPar
*   struct in BSS hunk.  Some extra
*   comments, and slight optimization.
*   
*   Revision 1.3  90/10/01  15:56:34  darren
*   Fix - allocates an IOEXTPar
*   struct so a query can be
*   sent to the parallel, or netprint.device.
*   
*   Revision 1.2  90/07/27  02:19:52  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:26:08  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:29:43  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:49:24  daveb
*   added to rcs
*   
*   Revision 1.3  85/10/10  00:30:23  kodiak
*   remove +du_CmdQueues from pd_Unit reference
*   
*   Revision 1.2  85/10/10  00:00:48  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.1  85/09/25  16:48:49  kodiak
*   set IOR variable for each io request dispatched
*   
*   Revision 1.0  85/09/25  15:40:55  kodiak
*   added to rcs for updating in version 1
*   
*   Revision 29.1  85/08/07  21:58:11  kodiak
*   allocate signals from printer.device task
*   handshake with CHILD signal for device initialization
*   
*   Revision 29.0  85/08/02  16:14:48  kodiak
*   added to rcs for updating in version 29
*   
*   Revision 25.1  85/06/16  01:05:39  kodiak
*   *** empty log message ***
*   
*   Revision 25.0  85/06/13  18:53:54  kodiak
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
	INCLUDE		"exec/strings.i"

	INCLUDE		"macros.i"
	INCLUDE		"prtbase.i"

*------ Imported Names -----------------------------------------------

	XREF_EXE	AllocSignal
	XREF_EXE	GetMsg
	XREF_EXE	Signal
	XREF_EXE	Wait
	XREF		_SysBase

	XREF		_IOR


*------ Exported Functions -------------------------------------------

	XDEF		PWTaskStart


*------ Exported storage ---------------------------------------------

	XDEF		_IOPQUERY
	XDEF		_IOPREPLY

*------ Printer Write task -------------------------------------
*
*   Handle the following signals:
*	REQUEST		A command has been posted to the device
*	IODONE		or an I/O write has been satisfied
*
*---------------------------------------------------------------------
*
*   REGISTERS
*	D5	REQUEST sigbit
*	D6	IODONE sigbit
*	D7	REQUEST | IODONE mask
*
*	A4	initializing task
*
*---------------------------------------------------------------------
PWTaskStart:
	;------ Grab the arguments
		MOVEM.L	4(A7),A4/A6

	;------ initialize the unit command queue
		MOVEQ	#-1,D0
		LINKEXE	AllocSignal
		MOVE.L	D0,D5
		MOVE.B	D5,pd_Unit+MP_SIGBIT(A6)
		LEA	pd_Unit+MP_MSGLIST(A6),A0
		NEWLIST	A0
		LEA	pd_TC(A6),A1
		MOVE.L	A1,pd_Unit+MP_SIGTASK(A6)

	;------ Initialize the I/O Requests
		MOVE.L	A1,pd_IORPort+MP_SIGTASK(A6)
		MOVEQ	#-1,D0
		LINKEXE	AllocSignal
		MOVE.L	D0,D6
		MOVE.B	D6,pd_IORPort+MP_SIGBIT(A6)
		LEA	pd_IORPort+MP_MSGLIST(A6),A0
		NEWLIST	A0

		LEA	pd_IORPort(A6),A0
		MOVE.L	A0,pd_TIOR+MN_REPLYPORT(A6)
		MOVE.L	A0,pd_IOR0+MN_REPLYPORT(A6)
		MOVE.L	A0,pd_IOR1+MN_REPLYPORT(A6)

	;------ Initialize extra IO Request for parallel/net device query

		LEA	_IOPREPLY,A0	;initialize message port

		MOVEQ	#IO_QUERY-_IOPREPLY-1,D0

		;-- We dont use AllocMem here
		;-- since we NEVER want to fail.

		;-- The printer.device is currently V33/34
                ;-- compatable, so we cant use the built-in
                ;-- functions to init this stuff which
                ;-- are in V36 exec.library :-(

		;-- There is no provision in the code
                ;-- for failure, so the assumption below
                ;-- that AllocSignal will return a sigbit
                ;-- is no worse of an offense than the existing code.

		MOVEQ	#00,D1		;fill with 0's
fillquery:
		MOVE.B	D1,(A0)+
		DBF	D0,fillquery

		LEA	_IOPREPLY,A0

		;-- A1 is still valid from the code above

		MOVE.L	A1,MP_SIGTASK(A0)

		MOVEQ	#-1,D0
		LINKEXE	AllocSignal

		LEA	_IOPREPLY,A0
		MOVE.B	D0,MP_SIGBIT(A0)
		MOVE.B	#NT_MESSAGE,LN_TYPE(A0)
		MOVE.B	#PA_SIGNAL,MP_FLAGS(A0)

		LEA	MP_MSGLIST(A0),A0
		NEWLIST	A0

		LEA	_IOPQUERY,A0
		MOVE.B	#NT_REPLYMSG,LN_TYPE(A0)
		MOVE.L	#_IOPREPLY,MN_REPLYPORT(A0)
		MOVE.W	#IOEXTPar_SIZE,MN_LENGTH(A0)


	;------ signal to initializing task that initialization is done here
		MOVEQ	#SIGF_CHILD,D0
		MOVE.L	A4,A1
		LINKEXE	Signal

	    ;------ mask for something to do
		MOVEQ	#0,D7
		BSET	D5,D7
		BSET	D6,D7

	;------ Wait for something to do
taskWait:
		MOVE.L	D7,D0
		LINKEXE Wait

		MOVE.L	D0,D2

		BTST	D6,D2
		BEQ.S	checkRequest
*	    ;------ See which output I/O request has completed
checkIOR:
		LEA	pd_IORPort(A6),A0
		LINKEXE	GetMsg
		TST.L	D0
		BEQ.S	checkRequest		; no more messages
		LEA	pd_IOR0(A6),A1
		CMP.L	A1,D0
		BNE.S	checkIOR1
		BCLR	#PB_IOR0,pd_Flags(A6)
		BRA.S	checkIOR
checkIOR1:
		LEA	pd_IOR1(A6),A1
		CMP.L	A1,D0
		BNE.S	checkIOR
		BCLR	#PB_IOR1,pd_Flags(A6)
		BRA.S	checkIOR


checkRequest:
		BTST	D5,D2
		BEQ.S	taskWait
*	    ;------ get and dispatch the printer I/O request
checkPIO:
		MOVE.L	pd_Unit+MP_MSGLIST(A6),A1
		TST.L	(A1)
		BEQ.S	taskWait
		BSET	#IOB_CURRENT,IO_FLAGS(A1)

		MOVE.L	A1,_IOR
		MOVE.W	IO_COMMAND(A1),D0
		LSL.W	#2,D0
		MOVE.L	dd_CmdVectors(A6),A0
		MOVE.L	0(A0,D0.W),A0
		JSR	(A0)

		BRA.S	checkPIO

** An IOEXTPar structure so we can query the parallel, or netprint devices

		SECTION printer,BSS

		CNOP	0,4

_IOPREPLY:	DS.B	MP_SIZE
_IOPQUERY:	DS.B	IOEXTPar_SIZE
IO_QUERY:	DS.W	1		;pad
		DS.W	0		;align

		END

