
* *** curse.asm ***************************************************************
*
* Cursor Blink Routines for the Zaphod Project
* 
* Copyright (C) 1986, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
* 24 Mar 1986	=RJ Mical=	Created this very file
* 31 March 86	=RJ=		Split Curse() into Curse() and NewCursings()
*
* *****************************************************************************

	INCLUDE "zaphod.i"


	IFND	AZTEC ; If AZTEC not defined, do it the standard way
	XDEF	_NewCursings,_Curse
	ENDC
	IFD	AZTEC ; If AZTEC defined, do it the non-standard way
	CSEG
	PUBLIC	_NewCursings,_Curse
	ENDC



* ****************************************************************************
* The Cursor routines draw the cursor rectangle(s) at a given location of the
* given display.  The routines are comprised of two parts:  the first part,
* NewCursings(), calculates the cursor box variables for the current cursor
* size and position.  The second routine, Curse(), then complements the 
* cursor box(es) at the current positions.
*
* As you know, the PC cursor can appear as one or two rectangles, depending
* on whether the start row is above or below the end row.  If the start row
* is lower than the end row, the cursor wraps around to the top.  As far
* as the PC hardware is concerned, this is no problem.	But Zaphod is
* obliged to concern itself over this, which it does by handling the
* cursor as potentially two blocks.  The distinction is made with the
* CursorAltTop variable, which can have an alternate top line number to
* complement the CursorTop and CursorBottom variables (if there is a 
* second cursor box, it necessarily extends to the bottom of the character,
* and thus there is no need for for a CursorAltBottom variable).  When
* the CursorAltTop variable is set to -1, there is no second box.
* ****************************************************************************




_NewCursings:
* ****************************************************************************
* This routine derives the true cursor box sizes based on the
* current cursor specifications and clipping that must be done at the
* display's edges (because of borders and off-display cursors).
*
* ON ENTRY:
*	- On the stack are:
*		- Display structure address
*		- X text-grid coordinate for cursor
*		- Y text-grid coordinate for cursor
*
* Register usage:
*
* A0 = Display structure address
* A1 = Window structure address
* A2 = (unused)
* A3 = (unused)
* A4 = (unused)
* A5 = (unused)
* A6 = (unused)
*
* D0 = Derived value for StartX, then BaseY derived value
* D1 = Derived value for EndX, then line number of first visible display line
* D2 = Horizontal edge checker, then line number of last visible display line
* D3 = Derived value of StartY and EndY
* D4 = (unused)
* D5 = (unused)
* D6 = A long of -1
* D7 = (scratch)
*
* The column coordinates of the cursor are constant whether there's one or 
* two cursor boxes.  That's why these are derived only once.  The base (top) 
* row is calculated and used to derive the start and end rows for the
* boxes.  
*
* And I think that that's enough talk for now.  Let's get to some action!

ARGS	EQU	((4+1)*4)
	MOVEM.L D2-D3/D6-D7,-(SP)

	MOVE.L	ARGS(SP),A0		; Get the display address
	MOVE.L	FirstWindow(A0),A1
	MOVE.L	DisplayWindow(A1),A1	; Get the address of the window
	MOVEQ	#-1,D6			; Load up a long of -1

* Derive the correctly-clipped StartX and EndX values.
* Start this by first getting the unclipped StartX and EndX.
* D0 = StartX = ((x - TextStartCol) * CHAR_WIDTH) + TextBaseCol
* D1 = EndX = StartX + CHAR_WIDTH - 1

	MOVE.L	ARGS+4(SP),D0
	SUB.W	TextStartCol(A0),D0
	LSL.W	#3,D0			; This presumes that chars are 8 wide
	ADD.W	TextBaseCol(A0),D0

	MOVE.W	D0,D1
	ADDQ.W	#CHAR_WIDTH-1,D1

* Now clip these to the border edges.  First check the right edge.

	MOVE.W	wd_Width(A1),D2
	CLEAR	D7
	MOVE.B	wd_BorderRight(A1),D7
	SUB.W	D7,D2			; First offscreen column

	CMP.W	D2,D0			; Is StartX >= first offscreen col?
	BGE	NO_CURSOR		; If so, there's no cursor visible

	CMP.W	D2,D1			; Is EndX < first offscreen col?
	BLT	10$			; Branch if EndX is not offscreen
	MOVE.W	D2,D1			; else force EndX onscreen
	SUBQ.W	#1,D1

* Now check where the cursor is with respect to the left edge.

10$	CLEAR	D2
	MOVE.B	wd_BorderLeft(A1),D2	; First visible column

	CMP.W	D2,D1			; Is EndX < first visible column?
	BLT	NO_CURSOR		; If so, there's no cursor visible.

	CMP.W	D2,D0			; Is StartX >= first visible column?
	BGE	20$			; Branch if so (all is well)
	MOVE.W	D2,D0			; else start at first visible column


20$
* OK, so fine.	Save the x-coordinates into the structure.
	MOVE.W	D0,CursorRealLeft(A0)
	MOVE.W	D1,CursorRealRight(A0)


* Get the y-axis variables for deriving the cursor(s) for the call(s)
* to RectFill().  Derive BaseY, the top line in the display of 
* the character position for this cursor.  The StartY and EndY variables
* of the cursor box(es) will be derived relative to the base.  
* D0 = BaseY = ((Y - TextStartRow) * CHAR_HEIGHT) + TextBaseRow
* D1 = First visible line of the display
* D2 = Last visible line of the display

	MOVE.L	ARGS+8(SP),D0		; Get the character row
	SUB.W	TextStartRow(A0),D0
	LSL.W	#3,D0			; (presumes text is 8 lines tall)
	ADD.W	TextBaseRow(A0),D0

	CLEAR	D1
	MOVE.B	wd_BorderTop(A1),D1

	MOVE.W	wd_Height(A1),D2
	CLEAR	D7
	MOVE.B	wd_BorderBottom(A1),D7
	SUB.W	D7,D2
	SUBQ.W	#1,D2
				      

*****
* Now we're set up to make the settings for the calls to RectFill().
* In other words, we're done with the initial initializing.
* First, set up for the top cursor.

	MOVE.W	CursorTop(A0),D3
	ADD.W	D0,D3			; StartY, the cursor top row

	CMP.W	D2,D3			; If StartY > last visible line
	BGT	NO_UPPER		; then skip, cursor not visible

	CMP.W	D1,D3			; If StartY >= first visible line
	BGE	30$			; then leave StartY as is
	MOVE.W	D1,D3			; else use the first visible line

30$	MOVE.W	D3,CursorRealTop(A0)	; OK, save this one

	MOVE.W	CursorBottom(A0),D3	; Now go for the bottom
	ADD.W	D0,D3			; EndY = the cursor bottom row

	CMP.W	D1,D3			; If EndY < first visible line
	BLT	NO_UPPER		; then skip, cursor not visible

	CMP.W	D2,D3			; If EndY <= last visible line
	BLE	40$			; then use as is
	MOVE.W	D2,D3			; else go down only to the last line

40$	MOVE.W	D3,CursorRealBottom(A0)
	BRA	LOWER_CURSOR
   

NO_UPPER:
	MOVE.W	D6,CursorRealTop(A0)	; Set it to -1 for "no cursor here."

* And fall into the test for the lower cursor



LOWER_CURSOR:
*****
* Now let's try for the lower cursor.  If the CursorAltTop == -1,
* there's no alternate box to render, else render from CursorAltTop to the
* bottom of the character box.
  
	MOVE.W	CursorAltTop(A0),D3	; Trying to make alternate StartY
	CMP.W	D3,D6			; Was it -1?
	BEQ	NO_LOWER		; If so, none to do

	ADD.W	D0,D3			; StartY, the cursor top row

	CMP.W	D2,D3			; If StartY > last visible line
	BGT	NO_LOWER		; then skip, cursor not visible

	CMP.W	D1,D3			; If StartY >= first visible line
	BGE	30$			; then leave StartY as is
	MOVE.W	D1,D3			; else use the first visible line

30$	MOVE.W	D3,CursorRealAltTop(A0) ; Save this one as is

	MOVE.W	#CHAR_HEIGHT-1,D3	; The alternate goes to the bottom
	ADD.W	D0,D3			; EndY = the cursor bottom row

	CMP.W	D1,D3			; If EndY < first visible line
	BLT	NO_LOWER		; then skip, cursor not visible

	CMP.W	D2,D3			; If EndY <= last visible line
	BLE	40$			; then use as is
	MOVE.W	D2,D3			; else go down only to the last line

40$	MOVE.W	D3,CursorRealAltBottom(A0)
	BRA	RETURN

NO_CURSOR:
	MOVE.W	D6,CursorRealTop(A0)	; Turn off both cursors

NO_LOWER:
	MOVE.W	D6,CursorRealAltTop(A0) ; Whoops, denote no lower cursor.


RETURN:
	MOVEM.L (SP)+,D2-D3/D6-D7
	RTS

       



_Curse:
* ****************************************************************************
* This routine presumes that the layer is locked already on
* entry, and that the draw mode is already initialized (presumably to
* COMPLEMENT mode).  It draws the cursor box(es), if any, according to
* the current cursor box specifications as initialized by a call to 
* NewCursings().
*
* ON ENTRY:
*	- On the stack are:
*		- Display structure address
*
* Register usage:
*
* A0 = (scratch) 
* A1 = RPort on call to RectFill()
* A2 = Display structure address
* A3 = RPort structure address for calls to RectFill()
* A4 = (unused)
* A5 = (unused)
* A6 = GfxBase
*
* D0 = Start X on call to RectFill()
* D1 = Start Y on call to RectFill()
* D2 = End X on call to RectFill()
* D3 = End Y on call to RectFill()
* D4 = A long of -1
* D5 = (unused)
* D6 = (unused)
* D7 = (unused)
*
* The column coordinates of the cursor are constant whether there's one or 
* two cursor boxes.  That's why these are derived only once.  The base (top) 
* row is calculated and used to derive the start and end rows for the
* boxes.  
*
* And I think that that's enough talk for now.  Let's get to some action!

ARGS2	EQU	((7+1)*4)
	MOVEM.L A2-A3/A6/D2-D5,-(SP)

	MOVE.L	ARGS2(SP),A2
	MOVE.L	FirstWindow(A2),A3
	MOVE.L	DisplayWindow(A3),A3
	MOVE.L	wd_RPort(A3),A3
	MOVE.L	_GfxBase,A6
	MOVE.W	CursorRealRight(A2),D2
	MOVEQ	#-1,D4
				  

	MOVE.B	rp_DrawMode(A3),D5	; Save the current draw mode
	MOVE.B	#RP_COMPLEMENT,D0	; and set the mode to COMPLEMENT
	MOVE.L	A3,A1
	CALLSYS SetDrMd

* First, test the top cursor, and do it if there's one to do
	MOVE.W	CursorRealTop(A2),D1
	CMP.W	D1,D4
	BEQ	TRY_ALT 		; If equal to -1, none to do

	MOVE.W	CursorRealBottom(A2),D3
	MOVE.W	CursorRealLeft(A2),D0
	MOVE.L	A3,A1
	CALLSYS RectFill

* Then, test for the alternate cursor box, and do that too
TRY_ALT:
	MOVE.W	CursorRealAltTop(A2),D1
	CMP.W	D1,D4
	BEQ	CURSE_RETURN

	MOVE.W	CursorRealAltBottom(A2),D3
	MOVE.W	CursorRealLeft(A2),D0
	MOVE.L	A3,A1
	CALLSYS RectFill

CURSE_RETURN:

	MOVE.B	D5,D0		; And finally restore the old draw mode
	MOVE.L	A3,A1
	CALLSYS SetDrMd

	MOVEM.L (SP)+,A2-A3/A6/D2-D5
	RTS


	END


