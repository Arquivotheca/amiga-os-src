	TTL	'$Id: keycmds.asm,v 36.9 90/12/14 18:59:34 darren Exp $
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*	Keyboard device commands
*
*   Source Control
*   --------------
*   $Id: keycmds.asm,v 36.9 90/12/14 18:59:34 darren Exp $
*
*   $Locker:  $
*
*   $Log:	keycmds.asm,v $
*   Revision 36.9  90/12/14  18:59:34  darren
*   Fixes memory trashing bug introduced in late 89 after
*   adding the code to allow for any size for IO_LENGTH.
*   Also now returns IO_ACTUAL as it should.
*   
*   Revision 36.8  90/04/13  12:44:04  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.7  90/04/02  12:59:59  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.6  89/11/02  17:21:34  kodiak
*   *** empty log message ***
*   
*   Revision 36.5  89/11/01  14:23:38  kodiak
*   allow any length for ReadMatrix command
*   
*   Revision 36.4  89/06/09  12:12:45  kodiak
*   autodoc changes
*   
*   Revision 36.3  88/11/03  12:56:22  kodiak
*   use AUTOINIT in ResidentTag
*   
*   Revision 36.2  88/10/07  10:51:14  kodiak
*   modify autodocs
*   
*   Revision 36.1  88/10/03  11:57:45  kodiak
*   allow larger io_Length than MATRIX_BYTES in ReadMatrix.
*   
*   Revision 35.0  87/10/26  11:31:55  kodiak
*   initial from V34, but w/ stripped log
*   
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
	INCLUDE		"exec/strings.i"
	INCLUDE		"exec/interrupts.i"
	INCLUDE		"exec/errors.i"

	INCLUDE		"devices/timer.i"
	INCLUDE		"devices/inputevent.i"

	INCLUDE		"keyboard.i"

	INCLUDE		"macros.i"
	INCLUDE		"kbdata.i"


*------ Imported Names -----------------------------------------------

*------ Imported Functions -------------------------------------------

	XREF_EXE	Disable
	XREF_EXE	Enable
	XREF_EXE	Enqueue

	XREF		EndCommand
	XREF		_ciaacra


*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF		KDClear
	XDEF		KDReadEvent
	XDEF		KDReadMatrix
	XDEF		KDAddResetHandler
	XDEF		KDRemResetHandler
	XDEF		KDResetHandlerDone

	XDEF		KDCBClear
	XDEF		KDCBReadEvent
	XDEF		KDCBReadMatrix
	XDEF		KDCBAddResetHandler
	XDEF		KDCBRemResetHandler
	XDEF		KDCBResetHandlerDone


******* keyboard.device/CMD_CLEAR ************************************
*
*   NAME
*	CMD_CLEAR -- Clear the keyboard input buffer.
*
*   FUNCTION
*	Remove from the input buffer any keys transitions waiting to
*	satisfy read requests.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	CMD_CLEAR
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
KDCBClear	EQU	-1

KDClear:
		LINKEXE Disable
		clr.w	kd_BufHead(a6)
		clr.w	kd_BufTail(a6)
		LINKEXE Enable
		bra	endOKCommand


******* keyboard.device/KBD_READEVENT ********************************
*
*   NAME
*	KBD_READEVENT -- Return the next keyboard event.
*
*   FUNCTION
*	Read raw keyboard events from the keyboard and put them in the
*	data area of the iORequest.  If there are no pending keyboard
*	events, this command will not be satisfied, but if there are
*	some events, but not as many as can fill IO_LENGTH, the
*	request will be satisfied with those currently available.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	KBD_READEVENT
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	the size of the io_Data area in bytes: there
*			are sizeof(inputEvent) bytes per input event.
*	io_Data		a buffer area to fill with input events.  The
*			fields of the input event are:
*	    ie_NextEvent
*			links the events returned
*	    ie_Class
*			is IECLASS_RAWKEY
*	    ie_Code 
*			contains the next key up/down reports
*	    ie_Qualifier 
*			only the shift and numeric pad bits are set
*	    ie_SubClass, ie_X, ie_Y, ie_TimeStamp 
*			are not used, and set to zero
*
*   RESULTS
*	This function sets the error field in the IORequest, and fills
*	the IORequest with the next keyboard events (but not partial
*	events).
*
**********************************************************************
KDCBReadEvent	EQU	0

KDReadEvent:
		movem.l d2-d3/a2-a3,-(a7)
		move.l	a1,a3

		LINKEXE Disable

		;------ check if ReadEvent already running here
		bset	#IOB_SERVICING,IO_FLAGS(a3)
		bne	readEnableRts

		;------ check for room in ReadEvent
		moveq	#ie_SIZEOF,d1
		moveq	#0,d2
		move.l	IO_LENGTH(a3),d3
		sub.l	d1,d3
		blt	readTooSmall

		;------ check for data for ReadEvent
		move.w	kd_BufHead(a6),d0
		cmp.w	kd_BufTail(a6),d0
		beq	readPending

		LINKEXE Enable

		;------ initialize transfer variables
		move.l	IO_DATA(a3),a2

readLoop:
		LINKEXE	Disable

		;------ get data from buffer
		move.w	kd_BufHead(a6),d0
		cmp.w	kd_BufTail(a6),d0
		beq.s	readDone

		move.b	#IECLASS_RAWKEY,ie_Class(a2)
		clr.b	ie_SubClass(a2)
		move.w	kd_BufQueue(a6,d0.W),ie_Code(a2)
		move.w	kd_BufQueue+2(a6,d0.W),ie_Qualifier(a2)
		clr.l	ie_X(a2)			 ; and ie_Y
		clr.w	ie_TimeStamp(a2)
		addq.w	#4,d0
		add.l	d1,a2
		move.l	a2,ie_NextEvent-ie_SIZEOF(a2)

		andi.w	#(KBBUFSIZE-1),d0
		move.w	d0,kd_BufHead(a6)

		add.l	d1,d2			;increment actual

		LINKEXE	Enable

		;------ check for more space in I/O request data area
		sub.l	d1,d3
		blt.s	readDone

		bra.s	readLoop


readPending:
		andi.b	#~(IOF_SERVICING!IOF_QUICK),IO_FLAGS(a3)

readEnableRts:
		LINKEXE	Enable

readRts:
		movem.l (a7)+,d2-d3/a2-a3
		rts

readTooSmall:
		LINKEXE	Enable
		moveq	#IOERR_BADLENGTH,d0
		move.l	a3,a1
		bsr	EndCommand
		bra.s	readNextCommand

readDone:
*	    ;------ the destination for the read is full
		clr.l	ie_NextEvent-ie_SIZEOF(a2)
		move.l	d2,IO_ACTUAL(a3)
		move.l	a3,a1
		bsr.s	endOKCommand

readNextCommand:
		move.l	kd_Unit+MP_MSGLIST(a6),a1
		tst.l	(a1)
		beq.s	readRts

		bsr	KDReadEvent
		bra.s	readRts


******* keyboard.device/KBD_READMATRIX *******************************
*
*   NAME
*	KBD_READMATRIX -- Read the current keyboard key matrix.
*
*   FUNCTION
*	This function reads the up/down state of every key in the
*	key matrix.
*
*   IO REQUEST INPUT
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	KBD_READMATRIX
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	the size of the io_Data area in bytes: this
*			must be big enough to hold the key matrix.
*	io_Data		a buffer area to fill with the key matrix:
*			an array of bytes whose component bits reflect
*			each keys state: the state of the key for
*			keycode n is at bit (n MOD 8) in byte
*			(n DIV 8) of this matrix.
*
*   IO REQUEST OUTPUT
*	io_Error
*	     IOERR_BADLENGTH - the io_Length was not exactly 13 bytes.
*			The buffer is unchanged.  This is only returned
*			by V33/V34 kickstart.
*	io_Actual	the number of bytes filled in io_Data with key
*			matrix data, i.e. the minimum of the supplied
*			length and the internal key matrix size.
*
*   NOTE
*	For V33/V34 Kickstart, io_Length must be set to exactly 13 bytes.
*
*   RESULTS
*	This function sets the error field in the IORequest, and sets
*	matrix to the current key matrix.
*
**********************************************************************
KDCBReadMatrix	EQU	-1

KDReadMatrix:
		move.l	a1,-(a7)
		moveq	#MATRIX_BYTES,d0
		move.l	IO_LENGTH(a1),d1
		move.l	IO_DATA(a1),a0
		cmp.l	d0,d1
		bls.s	rmLenOK
		move.w	d0,d1
rmLenOK:
		move.l	d1,IO_ACTUAL(a1)
		lea	kd_Matrix(a6),a1
		bra.s	rmDBF
rmLoop:
		move.b  (a1)+,(a0)+
rmDBF:
		dbf	d1,rmLoop

		move.l	(a7)+,a1
endOKCommand:
		moveq	#0,d0
		bra	EndCommand


******* keyboard.device/KBD_ADDRESETHANDLER **************************
*
*   NAME
*	KBD_ADDRESETHANDLER -- Add a keyboard reset handler.
*
*   FUNCTION
*	Add a function to the list of functions called to clean up
*	before a hard reset generated at the keyboard.  The reset
*	handler is called as:
*	    ResetHandler(handlerData)
*	                 a1
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	KBD_ADDRESETHANDLER
*	io_Data		a pointer to an interrupt structure.
*	    is_Data 	the handlerData pointer described above
*	    is_Code	the Handler function address
*
*   NOTES
*	Few of the Amiga keyboard models generate the communication codes
*	used to implement this reset processing.  Specifically, only the
*	Euro a1000 (rare), and the B2000 keyboard generate them.
*
*	The interrupt structure is kept by the keyboard device until a
*	RemResetHandler command is satisfied for it, but the
*	KBD_ADDRESETHANDLER command itself is replied immediately.
*
**********************************************************************
KDCBAddResetHandler EQU	-1

KDAddResetHandler:
		move.l	a1,-(a7)
		move.l	IO_DATA(a1),a1
		lea	kd_HandlerList(a6),a0
		LINKEXE	Enqueue
		move.l	(a7)+,a1
		bra.s	endOKCommand


******* keyboard.device/KBD_REMRESETHANDLER **************************
*
*   NAME
*	KBD_REMRESETHANDLER -- Remove a keyboard reset handler.
*
*   FUNCTION
*	Remove a function previously added to the list of reset
*	handler functions with KBD_ADDRESETHANDLER.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	KBD_REMRESETHANDLER
*	io_Data		a pointer to the handler interrupt structure.
*
**********************************************************************
KDCBRemResetHandler EQU	-1

KDRemResetHandler:
		move.l	a1,-(a7)
		move.l	IO_DATA(a1),a1
		REMOVE
		move.l	(a7)+,a1
		bra.s	endOKCommand


******* keyboard.device/KBD_RESETHANDLERDONE *************************
*
*   NAME
*	KBD_RESETHANDLERDONE -- Indicate that reset handling is done.
*
*   FUNCTION
*	Indicate that reset cleanup associated with the handler has
*	completed.  This command should be issued by all keyboard
*	reset handlers so that the reset may proceed.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	KBD_RESETHANDLERDONE
*	io_Data		a pointer to the handler interrupt structure.
*
*   NOTES
*	The keyboard processor itself performs the hardware reset, and
*	will time out and perform the reset even if some reset handlers
*	have not indicated yet that the reset may proceed.  This
*	timeout is several seconds.
*
**********************************************************************
KDCBResetHandlerDone EQU -1

KDResetHandlerDone:
		subq	#1,kd_OutstandingResetHandlers(a6)
		bne.s	rhdContinue

		and.b	#$0BF,_ciaacra	    ; complete reset handshake

rhdContinue
		bra.s	endOKCommand

		END
