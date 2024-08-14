**
**	$Id: read.asm,v 36.8 91/02/14 14:56:43 darren Exp $
**
**      console device read command
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/ables.i"
	INCLUDE	"exec/errors.i"


**	Exports

	XDEF	CDCBRead
	XDEF	CDRead


**	Imports

	XLVO	Permit			; Exec

	XREF	EndCommand

	XREF	GetReadIO


******* console.device/CMD_READ **************************************
*
*   NAME
*	CMD_READ -- return the next input from the keyboard
*
*   FUNCTION
*	Read the next input, generally from the keyboard.  The form of
*	this input is as an ANSI byte stream: i.e. either ASCII text
*	or control sequences.  Raw input events received by the
*	console device can be selectively filtered via the aSRE and aRRE
*	control sequences (see the write command).  Keys are converted
*	via the keymap associated with the unit, which is modified
*	with AskKeyMap and SetKeyMap
*
*	If, for example, raw keycodes had been enabled by writing
*	<CSI>1{ to the console (where <CSI> is $9B or Esc[), keys
*	would return raw keycode reports with the information from
*	the input event itself, in the form:
*	<CSI>1;0;<keycode>;<qualifiers>;0;0;<seconds>;<microseconds>q
*
*	If there is no pending input, this command will not be
*	satisfied, but if there is some input, but not as much as can
*	fill io_Length, the request will be satisfied with the input
*	currently available.
*
*    IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_READ
*	io_Flags	IOF_QUICK if quick I/O possible, else zero
*	io_Length	sizeof(*buffer)
*	io_Data		char buffer[]
*			a pointer to the destination for the characters to read
*			from the keyboard.
*
*   RESULTS
*	This function sets the error field in the IOStdReq, and fills
*	    in the io_Data area with the next input, and io_Actual with
*	    the number of bytes read.
*
*    BUGS
*
*    SEE ALSO
*	exec/io.h, devices/console.h
*
********************************************************************************

CDCBRead	EQU	0

CDRead:
		movem.l d2/a2-a3,-(a7)

		move.l	a1,a3

		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH

readLoop:
		;------ check if ReadEvent already running here
		bset	#IOB_SERVICING,IO_FLAGS(a3)
		bne.s	readRts

		move.l	IO_UNIT(a3),a2

		;------ check for room in ReadEvent
		moveq	#0,d2
		tst.l	IO_LENGTH(a3)
		beq.s	readNaught

		;-- attempt the read
		bsr	GetReadIO
		tst.l	d0		; check if data available
		beq.s	readPending

		;-- data was transferred
		move.l	d0,IO_ACTUAL(a3)
		move.l	a3,a1
		moveq	#0,d0
		bsr	EndCommand

readNextCommand:
		move.l	MP_MSGLIST(a2),a3
		tst.l	(a3)
		bne.s	readLoop

readRts:
		move.l	cd_ExecLib(a6),a0
		PERMIT	a0,NOFETCH
		movem.l (a7)+,d2/a2-a3
		rts

readPending:
		andi.b	#~(IOF_SERVICING!IOF_QUICK),IO_FLAGS(a3)
		bra.s	readRts

readNaught:
		moveq	#IOERR_BADLENGTH,d0
		move.l	a3,a1
		bsr	EndCommand
		bra.s	readNextCommand

	END
