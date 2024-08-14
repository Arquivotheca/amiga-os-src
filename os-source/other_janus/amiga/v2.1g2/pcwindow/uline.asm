
* *** uline.asm ***************************************************************
*
* Underlined Font Routines for the Zaphod Project
* 
* Copyright (C) 1986, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
* 2 April 86	=RJ Mical=	Created this file.
*
* *****************************************************************************

	INCLUDE "zaphod.i"
			  
	IFND	AZTEC ; If AZTEC not defined, do it the standard way
	XDEF _Underliner
	ENDC
	IFD	AZTEC ; If AZTEC defined, do it the non-standard way
	CSEG
	PUBLIC _Underliner
	ENDC




_Underliner:
* *****************************************************************************
* This guy goes through the UnderlineFont and adds the underliners.  
*
* ON ENTRY:
*	- nothing
*
* This is the algorithm:
*	The bottom row of each character is examined.  If a pixel is clear
*	and the two pixels on either side of it are clear, then set that pixel.
* This is accomplished by:
*	Creating an artificial version of the bottom line, which version
*	has two clear pixels before and after the real pixels.	Then examine
*	the row with a mask of five bits, and wherever all five bits are
*	clear set the bit in the font.

	MOVEM.L D2-D5,-(SP)

	MOVE.L	_UnderlineFont,A0	; Get the address of the underline font
	ADD.L	#7*2,A0 		; and skip to the last character

	MOVE.W	#(256-1),D0		; Character count (-1 for DBRA)
	MOVE.W	#$F800,D5		; Initial mask of five bits


CHAR_LOOP:
	MOVE.W	(A0),D1 		; Get the next line of data (ends in 
	LSR.W	#2,D1			; a zero byte!) and pad front with 
					; two zero bits

	MOVEQ	#7,D2			; Pixel counter (and index)
	MOVE.W	D5,D3			; Initial mask

PIXEL_LOOP:
	MOVE.W	D1,D4			; Get a fresh copy of the line
	AND.W	D3,D4			; and test it with the current mask
	BNE	10$			; and skip if any of the bits were set

	BSET	D2,(A0) 		; else set this bit in the font

10$	LSR.W	#1,D3			; Advance the mask to the next position
	DBRA	D2,PIXEL_LOOP

	LEA	8*2(A0),A0		; Advance pointer to next character
	DBRA	D0,CHAR_LOOP		; and try again while there's more

	MOVEM.L (SP)+,D2-D5
	RTS

	END

