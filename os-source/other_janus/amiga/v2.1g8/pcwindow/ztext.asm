
* *** ztext.asm *************************************************************
*
* Text Routines for the Zaphod Project
* The FastText algorithms were created by =Robert J. Mical= in January, 1986
* FastText Algorithms Copyright (C) 1986, =Robert J. Mical=
* 
* Copyright (C) 1986, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
* 13 Jan 87     =RJ=            Added text-color filter
* 8 Mar 86	RJ Mical>:-{)*	I'm a little over 2 meters tall, quite handso
*
* ***************************************************************************

	INCLUDE "zaphod.i"

	IFND	AZTEC ; If AZTEC not defined, do it the standard way
	XDEF _ZText
	ENDC
	IFD	AZTEC ; If AZTEC defined, do it the non-standard way
	CSEG
	PUBLIC _ZText
	ENDC



_ZText:
* *************************************************************************
*
* This routine sets for a color call to FastText()
*
* ON ENTRY:
*
* A0 = Address of the Display structure
* D0 = Pens
* D3 = Character count
* D4 = Character grid column
* D7 = Character grid row
*
* Prepares for call to ZText()
*	- Loads A0-A4 with Color-specific addresses
*	- Calculates register D1 from D4/D7
*	- Loads register D2
 
	MOVEM.L A0-A4/D1-D4/D7,-(SP)
		
	MOVE.L	A0,A1
	LEA	TextNormalPlane(A0),A2
	LEA	TextInversePlane(A0),A3
	LEA	TextOutputBuffer(A0),A4

	MOVE.L	_NormalFont,_FontData


* D0 should have the pens when called.	If this is a Color display, use
* the pens as specified.  If this is a Monochrome display, translate the 
* pens to Amiga Monochrome.

	MOVE.W	Modes(A0),D1
	ANDI.W	#COLOR_DISPLAY,D1
	BEQ	MONO_PENS
	   

COLOR_PENS:
	MOVE.W	Modes(A0),D1
	ANDI.W	#BLINK_TEXT,D1
	BEQ	PENS_CORRECT

	BTST	#7,D0
	BEQ	PENS_CORRECT

	ORI.W	#$08,D0
	ANDI.W	#$7F,D0
	BRA	PENS_CORRECT


MONO_PENS:
	MOVE.B	D0,D1			; Get a work copy of the pens
	ANDI.B	#$77,D1 		; Strip off blink and intensity

	CMPI.B	#$07,D1 		; Test for the usual case first
	BEQ	NORMAL_PENS

	CMPI.B	#$70,D1 		; Test for inverse text next
	BEQ	INVERSE_PENS

	CMPI.B	#$01,D1 		; Test for underlined text
	BEQ	UNDERLINE_TEXT

	CMPI.B	#$00,D1 		; Test for invisible text, and if
	BNE	NORMAL_PENS		; it's not this combo either, then
					; do the default of NORMAL_PENS

* The pen pattern $00 selects invisible text.
	CLEAR	D0
	BRA	PENS_CORRECT
	

INVERSE_PENS:
* The pattern $70 selects inverse text.
* Set the foreground pen to 0.	Set the background pen to 1 for low 
* intensity, and 3 for high intensity.
	MOVE.B	D0,D1			; Get a copy of the current pens
	MOVE.B	#$10,D0 		; and set the pens.
	ANDI.B	#$88,D1 		; Test if intensity or blink was set
	BEQ	PENS_CORRECT		; and split if not
	ORI.B	#$20,D0 		; else set the intensity color
	BRA	PENS_CORRECT


UNDERLINE_TEXT:
* For underline text, set the font data to the underlined characters, and
* then set the pens to normal.
	MOVE.L	_UnderlineFont,_FontData

* and intentionally fall into NORMAL_PENS ...

NORMAL_PENS:
* This one is easy.  For normal pens, set the foreground pen to 1, the 
* background to 0.  If the colors are intensified, set the foreground pen
* to 3 instead.
	MOVE.B	D0,D1		; Get a copy of the current pens
	MOVE.B	#$01,D0 	; and set the pens.
	ANDI.B	#$88,D1 	; Test the intensity and blink bits
	BEQ	PENS_CORRECT	; and skip if neither was set
	ORI.B	#$02,D0 	; else set the other pen bit


PENS_CORRECT:

	MULU	#CHAR_WIDTH,D4
	ADD.W	TextBaseCol(A0),D4
	MOVE.W	D4,D1
	SWAP	D1
	MULU	#CHAR_HEIGHT,D7
	ADD.W	TextBaseRow(A0),D7
	MOVE.W	D7,D1
 
* D3 should have the character count when called

	CLEAR	D2
	LEA	TextBitMap(A0),A0
	MOVE.B	bm_Depth(A0),D2

	JSR	_TextColorJammer

	JSR	_FastText

	MOVEM.L (SP)+,A0-A4/D1-D4/D7

	RTS

* **********************************************************************
* The following code could be used to test if the text about to be drawn
* is all blanks.  However, I'm not sure at this point whether or not
* the overhead would be worth it, since most lines will not be
* all blank.  Still, the non-blank collision at the beginning of the
* line is optimized here, so ...
*
*	 MOVEM.L A4/D5,-(SP)
*
*	 MOVE.L  #$20202020,D7
*
*	 MOVE.W  D3,D5
*	 LSR.W	 #2,D5
*	 BEQ	 TEST_SMALL_CHANGE
*
*	 SUBQ.W  #1,D5
*
*TEST_BIG_ONES:
*	 CMP.L	 (A4)+,D7
*	 DBNE	 D5,TEST_BIG_ONES
*
*	 BNE	 DO_FAST_TEXT
*
*TEST_SMALL_CHANGE:
*	 TST.W	 D3
*	 BEQ	 DO_FAST_TEXT
*
*	 CMP.W	 (A4),D7
*	 BNE	 DO_FAST_TEXT
*
*	 MOVE.B  D0,D5
*	 LSR.B	 #4,D5
*	 ANDI.B  #$F0,D0
*	 OR.B	 D5,D0
*
*DO_FAST_TEXT:
*	 MOVEM.L (SP)+,A4/D5
*
*	 BSR	 _FastText
*
*	 MOVEM.L (SP)+,A0-A4/D1-D4/D7
*
*	 RTS



_TextColorJammer:
* There is an idiosyncracy in the way text is rendered in the
* Zaphod display:  when fewer than the full number of bit-planes are
* used to represent the text, text may become invisible.  For example,
* in a two-plane (i.e. "monochrome") display that's been reduced to one
* plane, if the foreground text is in color 1 and the background is
* color 3, the result will be that the text will be in color 1 against
* a background of color 1, which renders the text invisible.
* 
* The workaround to this problem is to detect this condition 
* and artificially change the text color to keep it visible.
* 
* Algorithm:
* 	If the original pens were different
* 		- If the current pens aren't different
* 			- Toggle bit 0 of the text color
*
* ON ENTRY:
* D0.B = BgPen/FgPens (lower bytes)
* D2.W = Depth
*
* ON EXIT:
* D0.B = Pens with any necessary modifications

	MOVEM.L	D1/D3,-(SP)

	CMPI.B	#4,D2		; If the depth is 4 there's nothing to do
	BGE	FILTER_DONE

	MOVE.B	D0,D3		; Get a work copy of the background pen
	LSR.B	#4,D3

	MOVE.B	D3,D1		; Copy the background pen
	EOR.B	D0,D1		; and compare it with the foreground pen
	ANDI.B	#$0F,D1		; (full-bore, as it were)
	BEQ	FILTER_DONE	; If they were equal, nothing more to do

* Pens weren't equal originally.  Are they now?  If so, must correct.
* Get the mask for this depth into D2.  Depth is presumed to be 1, 2, or 3.
* This weird way of constructing the mask is on average much faster than
* using a straightforward table lookup, as I have these known depth 
* restrictions to leverage off of; in particular, depth of 1 takes 4 
* clock cycles more, depth of 2 takes an equal amount of cycles, 
* and depth of 3 takes 26 fewer cycles.

	MOVEQ	#7,D1		; Start with the full mask
	CMPI.B	#3,D2		; and if the depth is 3,
	BEQ	1$		; use the full mask
	ANDI.B	#3,D1		; else strip off the high bit.
	CMPI.B	#2,D2		; If the depth is 2
	BEQ	1$		; the mask is set
	MOVEQ	#1,D1		; else strip off both high bits the fast way
1$

	EOR.B	D0,D3		; Compare pens again
	AND.B	D1,D3		; but this time use depth mask
	BNE	FILTER_DONE	; If they are not equal, nothing more to do

	EORI	#$01,D0		; else toggle the foreground pen low bit

FILTER_DONE:

	MOVEM.L	(SP)+,D1/D3

	RTS


_FastText:
* ***************************************************************************
* These FastText algorithms were created by =Robert J. Mical= in January, 1986
* Copyright (C) 1986, =Robert J. Mical=
* All Rights Reserved
* This is my brute-force FastText() routine, doctored up to work well with
* Zaphod.  It presumes many things about text:	that the characters come in
* pairs, that the characters are 8-bits wide (this one is not easily 
* undone), and more.
*
* The theory of operation here is that I will build a single plane of
* normal characters (character data bits set, background bits clear)
* in the Normal Text Plane, using the data from the Output Characters
* Buffer.  As needed, I will invert this plane (character bits clear,
* background bits set) into the Inverted Text Plane.  There's two other
* globally accessible (pre-initialized) planes used:  _AllClearPlane Plane
* of all bits clear, and _AllSetPlane Plane of all bits set.
*
* Using these four planes, I can construct a bitmap of all possible pen
* settings for the foreground and background.  If corresponding bits
* in the FgPen and BgPen are:
*	 -----	   -----
*	   0	     0	     Use _AllClearPlane Plane
*	   0	     1	     Use Inverted Text Plane
*	   1	     0	     Use Normal Text Plane
*	   1	     1	     Use _AllSetPlane Plane
*
* Note that if FgPen and BgPen are equal, I don't have to bother
* constructing the Normal or Inverted Text Planes, since they'll
* never be used.
*		   
* An optimization that evolves out of this fact is that you can fill a line
* or part of a line of your text with spaces (blank characters) much more
* quickly by setting the foreground pen equal to the background pen.  The
* trick with this optimization is to not spend too much time detecting
* whether or not an area of text is all blank, for you may lose the
* increased performance during the handling of normal text lines.
*
* A further optimization is that I'm guessing that many text calls
* will not require the inverted plane, so I won't make it until I 
* discover that I need it.  If programmers follow the
* Intuition-encouraged standard of pen 1 for foreground and pen 0
* for background, then the bits are:	FgPen -- 00001
*					BgPen -- 00000
* In fact, if the text is *any* color against a background of zero,
* this optimization works.
*
* For example, the plain PC monochrome text turns out to be pen 1 on
* pen 0.  No need to invert here!
*
* So I won't bother constructing the inverted plane until I discover
* that it's needed.  The cost of this is that I have to check a flag
* once per time that I find an inverted bit plane is called for (6 times 
* maximum (though of course Dale would say 8 times maximum)), and the 
* savings is avoiding an unnecessary inversion.
*
* Note that this routine works with character pairs while building the
* buffer.  If you specify an odd number of characters, this routine will 
* round up the character count to the next higher even number, and then 
* build the buffer with that many characters.  However, only the number
* of characters you specify will actually be printed to the screen.
*
* Here's the registers that I will be using below:
*
* A0.L = Source BitMap (init to size of text plane, ignore plane ptrs)
* A1.L = Address of Display structure
* A2.L = Normal Text Plane (buffer for the normal plane of output chars)
* A3.L = Inverted Text Plane (buffer for inverted copy of normal plane)
* A4.L = Output Chars Buffer until end of PAIRLOOP, then BitMap Planes ptr
* A5   = (scratch)
* A6   = (scratch)
*
* D0.B = BgPen/FgPens (lower nybbles)
* D1.L = X/Y position (one word each respectively) for final blit
* D2.W = Depth
* D3.B = Char Count
* D4   = (scratch)
* D5   = (scratch)
* D6   = (scratch)
* D7   = (scratch)
 
DREGS	EQU	0		; Offset to the pushed D-Registers
DREGCOUNT EQU	8		; How many D-Registers were pushed
AREGS	EQU	(DREGS+(DREGCOUNT*4)) ; Offset to the pushed A-Registers

	MOVEM.L A0-A6/D0-D7,-(SP)


	MOVE.W	D3,D4		; Get the PAIRLOOP counter
	BEQ	RETURN		; and split if there's none to do
	ADDQ	#1,D4		; (will be rounding up below)
	LSR.B	#1,D4		; Turn the char count into a pair count
	SUBQ.W	#1,D4		; (-1 for DBRA)

	MOVE.L	#_custom,A5	; Get the address of the custom chip
	MOVE.L	_GfxBase,A6	; Load up A6 for graphics calls

	CALLSYS OwnBlitter	; Get that blitter

	MOVE.B	D0,D5		; Get a work copy of the pens
	MOVE.B	D0,D6
	LSR.B	#4,D5		; Get the BgPen into the lower byte
	ANDI.B	#$0F,D6
	CMP.B	D5,D6		; Are they equal?
	BEQ	PLANESET	; If so, skip building the planes

* Now that the blitter is mine all mine .  Wait 'til it's done blitting, 
* and then initialize that baby for my personal use ...
	CALLSYS WaitBlit	; and wait for that baby to be free!

	CLEAR	D5
	MOVE.W	D5,bltamod(A5)	; Set up the SRCA and SRCB modulos
	MOVE.W	D5,bltbmod(A5)
	SUBI.W	#1,D5
	MOVE.W	D5,bltafwm(A5)	; Masks are all set
	MOVE.W	D5,bltalwm(A5)

	MOVE.W	#BUFFER_WIDTH-2,bltdmod(A5)
	MOVE.W	#$0DFC,bltcon0(A5)
	MOVE.W	#$8000,bltcon1(A5)

	MOVE.L	A2,D5		; Get a scratch copy of the NormalPlane buffer
	MOVE.L	_FontData,A0	; Get the address of this font's data

PAIRLOOP:
* Do as much pre-calculating as possible before actually waiting for the
* blitter to be ready for re-use.
	CLEAR	D6		; Get the address of the font data of
	MOVE.B	(A4)+,D6	; the next character
	LSL.W	#4,D6		; (This presumes that each char is 16 bytes)
	ADD.L	A0,D6

	CLEAR	D7		; Get font data of next in pair (if there's
	MOVE.B	(A4)+,D7	; not really a second character (odd-numbered
	LSL.W	#4,D7		; string lengths) then this second move of
	ADD.L	A0,D7		; data will be unnecessary, but at least is
				; harmless since the buffer is *always*
				; pair-sized and the speed improvement is
				; great handling two characters at once).

	CALLSYS WaitBlit	; (this is redundant the first time through)

	MOVE.L	D6,bltapt(A5)
	MOVE.L	D7,bltbpt(A5)
	MOVE.L	D5,bltdpt(A5)

	MOVE.W	#$0201,bltsize(A5)	; Bombs away!

	ADDQ	#2,D5

	DBRA	D4,PAIRLOOP


PLANESET:
* Well, that was quick, wasn't it.  Here all of the required planes (except
* the bothersome Inverted Text Plane) are ready to go.	So let's figure
* out how to initialize the plane pointers in the BitMap ...

	MOVE.B	(DREGS+3)(SP),D4  ; Fetch the foreground pen into D4 
	MOVE.B	D4,D6		; and get the background pen into D6
	LSR.B	#4,D6		; and the background pen in D0
	CLEAR	D5		; Set up the pen-test mask
	MOVE.L	AREGS(SP),A4	; Get the address of the BitMap arg
	LEA	bm_Planes(A4),A4  ; and get address of first plane pointer
	CLEAR	D7		; Clear the Inverted flag
	SUBQ.W	#1,D2		; Set the loop counter for DBRA

PLANELOOP:

* First, find out if the foreground/background pattern for this plane 
* is 00, 01, 10 or 11 ...
	BTST	D5,D4		; If the bit is set in the foreground pen
	BNE	PAT_ONE 	; then go process 1x combinations

* Else we have a 0x combination.  Which one is it?
	BTST	D5,D6		; This time test the background pen
	BNE	PAT_01		; and if set then our pattern is 01

PAT_00:
	MOVE.L	_AllClearPlane,A5    ; This plane gets the 00 plane pointer
	BRA	PLANE_STUFF

PAT_01:
* The dreaded Inverse Text Plane selector.  If it doesn't exist yet,
* I have to create it now.  At the time of this writing, I don't know
* how painful this is going to be, though I suspect it won't be too bad.
*
* I have two algorithms for doing this.  This first (the original) just
* inverts the entire plane, regardless of how many characters have been
* written into the plane.  The second algorithm inverts only those 
* characters that need inverting, more or less (actually, it inverts
* characters in groups of 4 characters per shot).  The first algorithm
* takes 5068 clock cycles.  The second algorithm requires clock cycles
* as described by this formula:
*	n = number of groups of 4 characters 
*	cycles = ((42n + 30) + 4) * 8) + 42 + 4
* The break-even point between these two algorithms is between 56 and 57
* characters (14 - 15 groups).	So if there's 56 or fewer characters, I'll
* invert using the selective algorithm, else I'll use the brute-force
* method.  The cost of this is one test, and the savings when inverting
* just one group (1 - 4 characters) is significant:  654 cycles 
* versus 5068 cycles.  This seems worth the cost of the one compare.

	MOVE.L	A3,A5		; We'll be using the Inverted Plane
	TST.B	D7		; Test our Inverted Plane flag
	BNE	PLANE_STUFF	; and skip this mess if it's already set
				; else we'll have to invert it.
	ADDQ.B	#1,D7		; Set the inversion flag

* === Inversion loop ========================================================
	MOVEM.L A0-A1/A5/D0-D3,-(SP)

* OK, so use the blitter inversion algorithm:  Dest = NOT SRC.

	MOVE.W	#BUFFER_WIDTH,D2   ; Build the Group-of-2 values:
	ADDQ.W	#1,D3		; Get the first multiple of 2 that's greater
	ANDI.W	#$FFFE,D3	; than or equal to the character count, and
	SUB.W	D3,D2		; subtract that from the total buffer width
				; to make the Group-of-2 row modulo

	LSR.W	#1,D3		; Turn char count into pair count
	ORI.W	#$0200,D3	; and prepare for starting the blitter

	MOVE.L	#_custom,A5	; Get the address of the custom chip

	CALLSYS WaitBlit	; Wait for the blitter before I jam it.

	MOVE.L	A2,bltbpt(A5)
	MOVE.L	A3,bltdpt(A5)

	MOVE.W	D2,bltbmod(A5)	; Set up the SRCB and DEST modulos
	MOVE.W	D2,bltdmod(A5)

	MOVE.W	#$0533,bltcon0(A5)
	MOVE.W	#$0000,bltcon1(A5)

	MOVE.W	D3,bltsize(A5)	    ; Bombs away!
			
	MOVEM.L (SP)+,A0-A1/A5/D0-D3


* **************************************************************************
* Here lay the old algorithms, before I started using the blitter to do text.
* I should just throw these away, but I worked so hard on them that I can't
* bring myself to discard them just yet.
* ... here it is, 2 years later, and I *still* don't want to throw them away!
*
*	MOVEM.L A2-A3/A5/D0-D6,-(SP)
*
*
*	 MOVE.L  #-1,D6 		 ; Get our inversion value
*	 CMPI.W  #56,D3 		 ; Test the breaker point
*	 BGT	 BRUTE_INVERT
*
*
** OK, so use the selective invert algorithm.
*	 MOVEQ	 #CHAR_HEIGHT-1,D0  ; Loop counter for number of lines
*	 MOVE.W  #BUFFER_WIDTH,D1   ; Build the Group-of-4 values:
*	 ADDQ.W  #3,D3		 ; Get the first multiple of 4 that's greater
*	 ANDI.W  #$FFFC,D3	 ; than or equal to the character count, and
*	 SUB.W	 D3,D1		 ; subtract that from the total buffer width
*				; to make the group-of-4 row modulo
*
*	 LSR.W	 #2,D3		 ; Turn it into a Group-of-4 counter
*	 SUBQ.W  #1,D3		 ; (-1 for DBRA)
*
*10$	 MOVE.W  D3,D4		 ; Get a new copy for this row
*
*20$	 MOVE.L  (A2)+,D2	 ; EOR the characters of this row
*	 EOR.L	 D6,D2
*	 MOVE.L  D2,(A3)+
*	 DBRA	 D4,20$
*
*	 ADD.W	 D1,A2		 ; Skip to the next row
*	 ADD.W	 D1,A3
*	 DBRA	 D0,10$
*
*	 BRA	 INVERT_DONE
*
*
*BRUTE_INVERT:
** Get the number of 20-byte passes with the inversion loop.
** This presumes that BUFFER_WIDTH is a multiple of 20.
*	 MOVE.W  #((BUFFER_WIDTH/20)*8)-1,D5  ; -1 for DBRA
*
** Invert the next 20 bytes (5 longs)
*1$	 MOVEM.L (A2)+,D0-D4
*	 EOR.L	 D6,D0
*	 EOR.L	 D6,D1
*	 EOR.L	 D6,D2
*	 EOR.L	 D6,D3
*	 EOR.L	 D6,D4
*	 MOVEM.L D0-D4,(A3)
*	 LEA	 20(A3),A3
*  
*	 DBRA	 D5,1$
*
*
*INVERT_DONE:
*	 MOVEM.L (SP)+,A2-A3/A5/D0-D6

* === End Inversion loop ====================================================

	BRA	PLANE_STUFF


PAT_ONE:
* We've got a 1x pattern.  Which one do you suppose it is?  Hmm ...
	BTST	D5,D6		; This time, test the back pen only
	BNE	PAT_11
		     

PAT_10:
* Normal Text Plane?  No problem.
	MOVE.L	A2,A5
	BRA	PLANE_STUFF

PAT_11:
	MOVE.L	_AllSetPlane,A5  ; This plane must have all bits set

* and fall into ...


PLANE_STUFF:
* OK, so A5 has the address of this plane's data.  Stuff that baby
* into the BitMap structure.
	MOVE.L	A5,(A4)+
		     
	ADDQ	#1,D5		; Advance our mask to the next position

	DBRA	D2,PLANELOOP	; Loop on the depth of the BitMap


* Now, all done with the private use of the blitter.  This is actually
* an unfortunate misdesign in the graphics library.  I would prefer
* to retain exclusive use of the blitter even throughout the call to
* BlitBMRP below.  Too bad!


	CALLSYS DisownBlitter


* Well, now the BitMap is all ready to blast out our new line
* of text.  Wasn't that fun?  Now the simple final stroke:  zap that
* data into the RastPort, using the ever-popular, cleverly-named
* BltBitMapRastPort function (if Dale wasn't so good, I'd suggest
* that we take him out and shoot him for that name).
*
* BlitBMRP wants:
*
* A0 = Source BitMap  
* A1 = Destination RastPort
* A6 = GfxBase
*
* D0 = Source X
* D1 = Source Y
* D2 = Destination X
* D3 = Destination Y
* D4 = Pixel Width of block to be moved
* D5 = Scan-line Height of block to be moved
* D6 = Minterm

	MOVE.L	#$C0,D6 		; Minterm (simple transfer)

	MOVE.L	#CHAR_HEIGHT,D5 	; Height of transfer

	MOVE.W	#8,D4
	MULU	D3,D4			; Width of transfer (creates LONG)
	 
	MOVE.L	DREGS+(1*4)(SP),D2	; Fetch the original x/y position
	MOVE.W	D2,D3			; Get the Y position into D3
	SWAP	D2			; Get the X position from the high

	CLEAR	D1			; Source x and y are 0
	MOVE.L	D1,D0

	MOVE.L	AREGS+(1*4)(SP),A1	; Get the address of the display
	MOVE.L	FirstWindow(A1),A1	; Get the address of the first super
	MOVE.L	DisplayWindow(A1),A1	; Get the address of its window
	MOVE.L	wd_RPort(A1),A1 	; and get the RPort of the display

	MOVE.L	AREGS(SP),A0		; Get the address of the BitMap arg

	CALLSYS BltBitMapRastPort


RETURN:
	MOVEM.L (SP)+,A0-A6/D0-D7

	RTS
			       

	END

