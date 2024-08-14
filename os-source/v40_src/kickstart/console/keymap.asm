**
**	$Id: keymap.asm,v 36.8 91/11/04 14:57:19 darren Exp $
**
**      console device keymap interface
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/errors.i"


**	Exports

	XDEF	CDAskKeyMap
	XDEF	CDSetKeyMap
	XDEF	CDAskDefaultKeyMap
	XDEF	CDSetDefaultKeyMap

	XDEF	CDCBAskKeyMap
	XDEF	CDCBSetKeyMap
	XDEF	CDCBAskDefaultKeyMap
	XDEF	CDCBSetDefaultKeyMap


**	Imports

	XLVO	Forbid			; Exec
	XLVO	Permit			;

	XLVO	AskKeyMapDefault	; Keymap
	XLVO	SetKeyMapDefault	;

	XREF	EndCommand


******* console.device/CD_ASKKEYMAP *********************************
*
*   NAME
*	CD_ASKKEYMAP -- Get the current key map structure for this console.
*
*    FUNCTION
*	Fill the io_Data buffer with the current KeyMap structure in
*	use by this console unit.
*
*    IO REQUEST INPUT
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CD_ASKKEYMAP
*	io_Flags	IOF_QUICK if quick I/O possible, else zero
*	io_Length	sizeof(*keyMap)
*	io_Data		struct KeyMap *keyMap
*			pointer to a structure that describes 
*			the raw keycode to byte stream conversion.
*
*    IO REQUEST RESULT
*	This function sets the io_Error field in the IOStdReq, and fills
*	the structure the structure pointed to by io_Data with the current
*	 key map.
*
*    SEE ALSO
*	exec/io.h, devices/keymap.h, devices/console.h
*
**********************************************************************

CDCBAskKeyMap	EQU	-1		; immediate

CDAskKeyMap:
		MOVE.L	IO_UNIT(A1),d0
		;-- check for fake unit
		btst	#0,d0
		bne.s	CDAskDefaultKeyMap

		MOVE.L	A2,-(A7)

		move.l	d0,a0
		LEA	cu_KeyMapStruct(A0),A0
		MOVE.L	IO_DATA(A1),A2
		BRA.S	transferMap


******* console.device/CD_SETKEYMAP ******************************************
*
*    NAME
*	CD_SETKEYMAP -- set the current key map structure for this console
*
*    FUNCTION
*	Set the current KeyMap structure used by this console unit to
*	the structure pointed to by io_Data.
*
*    IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CD_SETKEYMAP
*	io_Flags	IOF_QUICK if quick I/O possible, else zero
*	io_Length	sizeof(*keyMap)
*	io_Data		struct KeyMap *keyMap
*			pointer to a structure that describes 
*			the raw keycode to byte stream conversion.
*
*    RESULTS
*	This function sets the io_Error field in the IOStdReq, and fills
*	the current key map from the structure pointed to by io_Data.
*
*    BUGS
*
*    SEE ALSO
*	exec/io.h, devices/keymap.h, devices/console.h
*
********************************************************************************

CDCBSetKeyMap	EQU	-1		; immediate

CDSetKeyMap:
		MOVE.L	IO_UNIT(A1),d0
		;-- check for fake unit
		btst	#0,d0
		bne.s	CDSetDefaultKeyMap

		MOVE.L	A2,-(A7)
		MOVE.L	IO_DATA(A1),A0
		MOVE.L	d0,A2
		LEA	cu_KeyMapStruct(A2),A2
		BRA.S	transferMap


******* console.device/CD_ASKDEFAULTKEYMAP ***********************************
*
*    NAME
*	CD_ASKDEFAULTKEYMAP -- get the current default keymap
*
*    FUNCTION
*	Fill the io_Data buffer with the current console device
*	default keymap, which is used to initialize console unit
*	keymaps when opened, and by RawKeyConvert with a null
*	keyMap parameter.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CD_ASKDEFAULTKEYMAP
*	io_Flags	IOF_QUICK if quick I/O possible, else zero
*	io_Length	sizeof(*keyMap)
*	io_Data		struct KeyMap *keyMap
*			pointer to a structure that describes 
*			the raw keycode to byte stream conversion.
*
*    RESULTS
*	This function sets the io_Error field in the IOStdReq, and fills
*	the structure pointed to by io_Data with the current device 
*	default key map.
*
*    BUGS
*
*    SEE ALSO
*	exec/io.h, devices/keymap.h, devices/console.h
*
********************************************************************************

CDCBAskDefaultKeyMap	EQU	-1		; immediate

CDAskDefaultKeyMap:
		MOVE.L	A2,-(A7)

cdAskDefaultEntry:
		move.l	a1,-(a7)
		LINKKEY	AskKeyMapDefault
		move.l	(a7)+,a1
		move.l	d0,a0
		MOVE.L	IO_DATA(A1),A2

transferMap:
		;------ check size of buffer
		MOVEQ	#km_SIZEOF,D0
		CMP.L	IO_LENGTH(A1),D0
		BNE.S	adkm3

		;------ set up protected copy
		MOVEQ	#(km_SIZEOF/4)-1,D1
		LINKEXE	Forbid		; doesn't trash D0/D1/A0/A1

		;------ copy table
adkm1:
		MOVE.L  (A0)+,(A2)+
		DBF	D1,adkm1

		;------ allow access to table
		LINKEXE	Permit		; doesn't trash D0/D1/A0/A1

		;------ end command
		MOVE.L	D0,IO_ACTUAL(A1)
		MOVEQ	#0,D0
adkm2:
		MOVE.L	(A7)+,A2
		BRA	EndCommand

		;------ report bad length
adkm3:
		MOVEQ	#IOERR_BADLENGTH,D0
		BRA.S	adkm2


******* console.device/CD_SETDEFAULTKEYMAP ***********************************
*
*    NAME
*	CD_SETDEFAULTKEYMAP -- set the current default keymap
*
*    FUNCTION
*	This console command copies/uses the keyMap structure pointed to
*	by io_Data to the console device default keymap, which is used
*	to initialize console units when opened, and by RawKeyConvert
*	with a null keyMap parameter.
*
*    IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CD_SETDEFAULTKEYMAP
*	io_Flags	IOF_QUICK if quick I/O possible, else zero
*	io_Length	sizeof(*keyMap)
*	io_Data		struct KeyMap *keyMap
*			pointer to a structure that describes 
*			the raw keycode to byte stream conversion.
*
*    RESULTS
*	This function sets the io_Error field in the IOStdReq, and fills
*	the current device default key map from the structure pointed to
*	by io_Data.
*
*    BUGS
*	As of V36, this command no longer copies the keymap structure,
*	and the keymap must remain in memory until the default key map
*	is changed.  In general there is no reason for applications to
*	use this command.  The default key map will generally be set by
*	the user using a system provided command/tool.
*
*    SEE ALSO
*	exec/io.h, devices/keymap.h, devices/console.h
*
********************************************************************************

CDCBSetDefaultKeyMap	EQU	-1		; immediate

CDSetDefaultKeyMap:
		move.l	a1,-(a7)

cdSetDefaultEntry:
		MOVEQ	#km_SIZEOF,D0
		CMP.L	IO_LENGTH(A1),D0
		BNE.S	badMapLength

		MOVE.L	D0,IO_ACTUAL(A1)
		MOVE.L	IO_DATA(A1),A0

		LINKKEY	SetKeyMapDefault

dmDone:
		move.l	(a7)+,a1
		MOVEQ	#0,D0
		BRA	EndCommand

badMapLength:
		MOVEQ	#IOERR_BADLENGTH,D0
		BRA.S	dmDone

		END
