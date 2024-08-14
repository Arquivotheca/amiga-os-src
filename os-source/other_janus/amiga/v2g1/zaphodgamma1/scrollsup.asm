
* *** scrollsup.asm ***********************************************************
*
* Scroll Support Routines  --  For the Zaphod Project
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

	INCLUDE	"janus/janusbase.i"
	INCLUDE	"janus/janusreg.i"
	INCLUDE	"janus/janusvar.i"
	INCLUDE	"janus/services.i"


	PUBLIC	_GetScrollAddress
	PUBLIC	_ScrollTextBuffer


	PUBLIC	_JanusBase



_GetScrollAddress:
* *****************************************************************************
* This routine returns the address of the parameter offset word associated 
* with the janus scroll service.  The idea here is that Torsten and I will 
* be using this word to comunicate scroll information, rather than its 
* normal use of pointing to a parameter block which Torsten and I would use.
* The pointer is to the word-access block of Janus RAM:
* 
* The benefit of this technique:  faster scroll transmission, which 
* performance is part of our motivation for implementing scroll in the first 
* place.
* 
* The drawback of this technique:  it's non-standard, so if anyone else makes 
* Janus calls regarding the parameter memory (like GetParamOffset() or 
* SetupJanusSig()) the calls could result in screwy information.
* 
* This code is basically just a hack of the janus.library's GetParamOffset, 
* which code (like much of the anus.library code) cheats like hell.  For 
* example, it seems that this code presumes that the parameter base  
* will always be located as the first thing in parameter memory.  Doesn't 
* sound like a particularly safe assumption to me, especially if anyone other 
* than the originals ever starts hacking at janus.library.  
* *****************************************************************************

	MOVE.L	_JanusBase,A0		; Get the JanusBase structure
	MOVE.L	jb_ParamMem(A0),A0	; Get (???) the JanusAmiga structure?
	ADD.L	#WordAccessOffset,A0	; Use word-access method

	;------ Get the offset to the parameter word
	MOVE.W	#(JSERV_SCROLL*2),D0	; Turn scroll number into word offset
	ADD.W	ja_Parameters(A0),D0	; Add offset to start of parm offsets
	LEA	0(A0,D0.W),A0		; Get real address
	MOVE.L	A0,D0			; and return the real address

	RTS



_ScrollTextBuffer:
* This routine stuffs the text buffer with data from the current text page
* of the color graphics card according to the current text buffer parameters
*
* ENTRY:
*	- on stack, struct Display *display
*	SHORT i, i2, longtextwidth;
*	ULONG *pagepos, *bufferpos;
*	SHORT endrow;

ARGS	EQU	((1*4)+4)

	MOVEM.L	D2,-(SP)

	MOVE.L	ARGS(SP),A0		; Get the address of the display struct

*	longtextwidth = display->TextWidth >> 1;
	MOVE.W	TextWidth(A0),D0
	LSR.W	#1,D0

*	endrow = display->TextHeight - 1;
	MOVE.W	TextHeight(A0),D1
	SUBQ.W	#1,D1

*	bufferpos = (ULONG *)(display->Buffer);
	MOVE.L	Buffer(A0),A1

*	pagepos = bufferpos + longtextwidth;
	MOVE.W	D0,D2
	LSL.W	#2,D2
	MOVE.L	A1,A0
	ADD.W	D2,A0

*	for (i = 1; i <= endrow; i++)
*		{
*		for (i2 = 0; i2 < longtextwidth; i2++)
*			*bufferpos++ = *pagepos++;
*		}
	MULU	D0,D1
	BRA	moveTest

moveLoop:
	MOVE.L	(A0)+,(A1)+
moveTest:
	DBRA	D1,moveLoop

	MOVEM.L	(SP)+,D2


	RTS



	END



