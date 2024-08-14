
* *****************************************************************************
*
* Input Handler  --  For the Zaphod Project
* 
* Copyright (C) 1988, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
* 26 Jan 1988	-RJ Mical-	Created this file!
*
* *****************************************************************************

	INCLUDE "zaphod.i"
	INCLUDE	"devices/inputevent.i"
			  

	XDEF    _IHandler


	XREF	_MouseFlags
	XREF	_MouseEventToPC



_IHandler:
* If the PC has the mouse, trap all mouse events and send them to the PC
* 
* ON ENTRY:
*	A0 has the address of the input event
*	A1 has the contents of the _Data field

* Test whether the program running this input handler has set the flag 
* designating that the PC is currently the mouse owner

	MOVE.L	A0,D0

	MOVE.W	_MouseFlags,D1
	BTST	#PC_HAS_MOUSE_B,D1
	BEQ	DONE			; Branch if PC doesn't have the mouse

	MOVE.L	A0,-(SP)
	JSR	_MouseEventToPC
	LEA	4(SP),SP


DONE:

* Return the address of the first remaining input event in D0
	RTS



	END

