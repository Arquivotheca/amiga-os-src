
* ***************************************************************************
*
* New Text Line Length Routines for the Zaphod Project
* 
* Copyright (C) 1986, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
* 26 Feb 86	=RJ Mical=	Created this file
*
* ***************************************************************************

	INCLUDE "zaphod.i"
		 

	XDEF	_RecalcNewLengths


_RecalcNewLengths:
******************************************************************************
* This routine examines the current color text buffer against the PC text
* page and, for each line starting at the end of the line, finds the first
* location where the text information doesn't match.  The color text buffer
* is updated as needed, and the CT_NewLength array gets the value 
* describing how many characters were copied, which also happens to be 
* the number of characters that need re-rendering on this line.
*
* The reason why the lines are examined from back to front is to optimize
* the text rendering by taking into account the white space usually found
* at the end of the line.
*
* ON ENTRY:
*	- Address of Display structure
*	- Start Row to check for new lengths
*	- End Row to check for new lengths

ARGS	EQU	((6+1)*4)
	MOVEM.L A2-A3/D2-D5,-(SP)

	MOVE.L	ARGS(SP),A3
   
* Get the (Height - 1) of the examination area into D5
	MOVE.W	ARGS+(2*4)+2(SP),D5	; Get the end row arg
	SUB.W	ARGS+(1*4)+2(SP),D5	; and subtract the start row arg
	BMI	RETURN			; If minus, there's no height!

* Derive the address of the correct text page in the PC memory area

	MOVE.L	PCDisplay(A3),A0	; Base address of display text buffer

	MOVE.W	Modes(A3),D1		; Find out if we're in medium- or
	ANDI.W	#MEDIUM_RES,D1		; high-res mode, and get the number
	BNE	1$			; number of characters below used
					; in several places below
	MOVE.W	#80,D2
	BRA	2$

1$	MOVE.W	#40,D2			; Get the char line count (for below)


* Now, since we're going from the back toward the front, offset the address
* to the character just beyond the last character of the last visible line.
* We want to start at one character beyond the end because of the 
* pre-decrement addressing mode used below.

2$	MOVE.W	D5,D0			; Get (height - 1) and turn it into
	ADD.W	TextStartRow(A3),D0	; an index to the last row.
	MULU	D2,D0			; D2 has char count from above
	ADD.W	D0,D0			; Each character takes up a word
	ADD.W	D0,A0

	MOVE.W	TextWidth(A3),D0	; Going beyond the last char
	BEQ	RETURN			; If zero, there's no width!
	SUB.W	D0,D2			; Now D2 has the PC excess width
	ADD.W	D2,D2			; which must be word-sized
	ADD.W	TextStartCol(A3),D0
	ADD.W	D0,D0			; Each character takes up a word
	ADD.W	D0,A0


* Now, derive the address of the Color Buffer pointer

	MOVE.L	Buffer(A3),A1		; Get the address of the color buffer

	MOVE.W	ARGS+(2*4)+2(SP),D0	; Get the end row argument and
	ADDQ.W	#1,D0			; point it to the row after the last
	MULU	TextWidth(A3),D0
	ADD.W	D0,D0			; Each character uses a word
	ADD.W	D0,A1

		     
* One last global address, the address of the element after the last of the 
* NewLength array

	LEA	TextNewLength(A3),A2
	MOVE.W	ARGS+(2*4)+2(SP),D0	; Get the end row argument and
	ADDQ.W	#1,D0			; point it to the row after the last
	ADD.W	D0,D0
	ADD.W	D0,A2

* For every line, compare longs between the color text buffer and the
* pc text memory.  Taking long bites like this means that characters
* are always handled in pairs.	This speeds things up, at the minor cost
* of sometimes rendering whole characters offscreen, which hopefully doesn't
* slow things down so much that the performance improvement is lost!

	MOVE.W	D5,D0		; Loop on the number of display lines

* Calculate this once and keep the value around for the loop below

	MOVE.W	TextWidth(A3),D1   ; Guaranteed (poof) to be a multiple of 2
	LSR.W	#1,D1		; so turn it into a pair count
	SUBQ.W	#1,D1		; and adjust for the decrement branch
	BMI	RETURN		; Safety (paranoia)

DO_NEWLENGTHS:
* This is the top of the line examination loop.
* When we get here, the registers should have:
*	D0 = Number of display lines - 1
*	D1 = Number of character pairs (long words) - 1
*	D2 = Unexamined portion of the PC line (for skipping to next)
*	A0 = Address of first char after last in PC memory
*	A1 = Address of first char after last in text buffer
*	A2 = Address of one past the last element of the NewLength array
  
NEWLINE_LOOP:
	MOVE.W	D1,D3

CMP_LOOP:
	MOVE.L	-(A0),D4	; Check each pair to ascertain whether they
	CMP.L	-(A1),D4	; match.  Fall out of this loop either when
	DBNE	D3,CMP_LOOP	; they don't match or all have been checked
				      
* If we got out of the loop with all characters matching, we should split
* now without copying.
 
	BEQ	LENGTH_DONE

* Else there were some characters that didn't match, so we should copy from
* here to the beginning of the line.
   
	LEA	4(A0),A0	; Push the pointers back one to make
	LEA	4(A1),A1	; the following loop more efficient

	MOVE.W	D3,D4		; Preserve the count value

COPY_LOOP:
	MOVE.L	-(A0),-(A1)
	DBRA	D4,COPY_LOOP


* OK, we're done with this line, so save the count into the NewLengths array
			   
LENGTH_DONE:
	ADDQ.W	#1,D3		; Adjust the count to reflect number of pairs
	LSL.W	#1,D3		; Change the count from a pair count to actual
	MOVE.W	D3,-(A2)	; Save the current count
		  

* Lastly, before the final branch, push the PC address to one past the last 
* character of the next line.  This not done for the display address as well
* because the buffer is packed, so it's not necessary (this was part of the
* reason for packing it).

	SUB.W	D2,A0		


	DBRA	D0,NEWLINE_LOOP ; Loop on the number of lines to transfer


RETURN:
	MOVEM.L (SP)+,A2-A3/D2-D5
	RTS

	END



