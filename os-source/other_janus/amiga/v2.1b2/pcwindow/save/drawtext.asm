
* *** drawtext.asm ************************************************************
*
* Text Output Routines for the Zaphod Project
* 
* Copyright (C) 1986, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
* 26 Feb 86	=RJ Mical=	Created this file
*
* *****************************************************************************

	INCLUDE "zaphod.i"

	IFND	AZTEC ; If AZTEC not defined, do it the standard way
	XREF	_ZText
	ENDC
	IFD	AZTEC ; If AZTEC defined, do it the non-standard way
	PUBLIC	_ZText
	ENDC

	IFND	AZTEC ; If AZTEC not defined, do it the standard way
	XDEF	_DrawText
	ENDC
	IFD	AZTEC ; If AZTEC defined, do it the non-standard way
	CSEG
	PUBLIC	_DrawText
	ENDC


			       
_DrawText:
******************************************************************************
* This routine uses the current NewLength array to derive which characters
* need to be redrawn.
* See the now-retired DrawColorText() for the original algorithm, which
* used the standard graphics library Text() routine.  This one now uses
* the FastText() routine optimized specifically for the PC text of the
* Zaphod Project.
* 
*
* ON ENTRY:
*	- On stack is the address of the Display structure
*
* This guy uses these registers:
*
* A0 = Address of the Display structure
* A1 = Pointer to current char in Buffer
* A2 = NewLength array pointer
* A3 = NewLength size of this row (misused address register, but I've no D's!)
* A4 = OutputBuffer pointer
* A5 = Buffer start of line pointer
* A6 = (unused)
*
* D0 = Current pens
* D1 = New pens
* D2 = Row size (number of bytes per Buffer row)
* D3 = (scratch, used as character count for call to ZText()
* D4 = Start column in Buffer of current text string
* D5 = Current column in Buffer of current text string
* D6 = (scratch)
* D7 = Current row

ARGS	EQU	((10+1)*4)
	MOVEM.L A2-A5/D2-D7,-(SP)

	MOVE.L	ARGS(SP),A0		; Get the address of the Display

	MOVE.L	Buffer(A0),A5		; First line starts at start of buf

	MOVE.W	(A5),D0 		; D0 gets initial pen

	CLEAR	D7			; First row of Buffer
	LEA	TextNewLength(A0),A2	; A2 is CT_NewLength array ptr
	LEA	TextOutputBuffer(A0),A4  ; A4 == Pointer into output buffer

	MOVE.W	TextWidth(A0),D2	; Create the row size, which is
	ADD.W	D2,D2			; the width * 2 (2 bytes per char)

ROW_LOOP:

	CMP.W	TextHeight(A0),D7	; If TextHeight <= row, then there
	BGE	ALL_DRAWN		; are no more rows to see

	CLEAR	D5			; Start from column 0 (for now!)
	MOVE.W	D5,D4			; copy start column

	MOVE.L	A5,A1			; Get the running ptr for this row

	MOVE.W	(A2)+,A3		; Get the length for this row

COL_LOOP:
	CMP.W	A3,D5			; Do this loop while NewLength > col
	BGE	L_100			; (skip out when we've looked at all)

	MOVE.B	(A1)+,D6		; Get the next char
	MOVE.B	(A1)+,D1		; Get the next attribute byte

	CMP.B	D1,D0			; Compare with CurrentPens
	BEQ	L_50			; and skip while they're equal

	MOVE.W	D5,D3			; Get a work copy of col
	SUB.W	D4,D3			; subtract startcol to get length
	BEQ	L_45			; and if they're equal, we're still
					; at the start of the line, else
					; we've got some characters to print!
	JSR	_ZText			; (with the old pens, you know)

	LEA	TextOutputBuffer(A0),A4  ; Reset pointer into output buffer

	MOVE.W	D5,D4			; startcol <- col

L_45	MOVE.B	D1,D0			; Save new CurrentPens

L_50	MOVE.B	D6,(A4)+		; Stuff the next character
	ADDQ.W	#1,D5			; Advance the column counter
	BRA	COL_LOOP		; and go loop while there's cols


* After the column loop, if col has advanced beyond startcol, there's some
* characters still to be output from this line.

L_100	MOVE.W	D5,D3			; Is startcol == col?
	SUB.W	D4,D3			; (D3 gets the string length)
	BEQ	L_110
					; If not, then
					; we've got some characters to print!
	JSR	_ZText			; Call that good ZText() routine

	LEA	TextOutputBuffer(A0),A4  ; Reset pointer into output buffer

L_110	ADD.W	D2,A5			; Advance Buffer ptr to next row
	ADDQ.W	#1,D7			; Increment the row counter
	BRA	ROW_LOOP		; Go test for more loops to do
	    
ALL_DRAWN:
	MOVEM.L (SP)+,A2-A5/D2-D7
	RTS

   

	END
