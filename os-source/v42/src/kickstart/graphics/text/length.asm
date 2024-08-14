**
**	$Id: length.asm,v 42.0 93/06/16 11:42:52 chrisg Exp $
**
**      output length of text string
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		graphics

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"

	INCLUDE		"graphics/rastport.i"

	INCLUDE		"text.i"

	INCLUDE		"txtdata.i"


*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF		_TextFit
	XDEF		_TextLength
	XDEF		_TextExtent

	XDEF		textLength


******* graphics.library/TextLength **********************************
*
*   NAME
*	TextLength -- Determine raster length of text data.
*
*   SYNOPSIS
*	length = TextLength(rp, string, count)
*	D0                  A1  A0      D0:16
*
*	WORD TextLength(struct RastPort *, STRPTR, WORD);
*
*   FUNCTION
*	This graphics function determines the length that text data
*	would occupy if output to the specified RastPort with the
*	current attributes.  The length is specified as the number of
*	raster dots: to determine what the current position would be
*	after a Text() using this string, add the length to cp_x
*	(cp_y is unchanged by Text()).  Use the newer TextExtent() to
*	get more information.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text attributes reside.
*	string - the address of string to determine the length of
*	count  - the string length.  If zero, there are no characters
*	         in the string.
*
*   RESULTS
*	length - the number of pixels in x this text would occupy, not
*	         including any negative kerning that may take place at
*	         the beginning of the text string, nor taking into
*	         account the effects of any clipping that may take
*	         place.
*
*   NOTES
*	Prior to V36, the result length occupied only the low word of
*	d0 and was not sign extended into the high word.
*
*   BUGS
*	A length that would overflow single word arithmetic is not
*	calculated correctly.
*
*   SEE ALSO
*	TextExtent()  Text()  TextFit()
*	graphics/text.h  graphics/rastport.h
*
**********************************************************************
******* graphics.library/TextExtent **********************************
*
*   NAME
*	TextExtent -- Determine raster extent of text data. (V36)
*
*   SYNOPSIS
*	TextExtent(rp, string, count, textExtent)
*	           A1  A0      D0:16  A2
*
*	void textExtent(struct RastPort *, STRPTR, WORD,
*	     struct TextExtent *);
*
*   FUNCTION
*	This function determines a more complete metric of the space
*	that a text string would render into than the TextLength()
*	function.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text attributes reside.
*	string - the address of the string to determine the length of.
*	count  - the number of characters in the string.
*                If zero, there are no characters in the string.
*	textExtent - a structure to hold the result.
*
*   RESULTS
*	textExtent is filled in as follows:
*	    te_Width  - same as TextLength() result: the rp_cp_x
*	                advance that rendering this text would cause.
*	    te_Height - same as tf_YSize.  The height of the
*	                font.
*	    te_Extent.MinX - the offset to the left side of the
*	                rectangle this would render into.  Often zero.
*	    te_Extent.MinY - same as -tf_Baseline.  The offset
*	                from the baseline to the top of the rectangle
*	                this would render into.
*	    te_Extent.MaxX - the offset of the left side of the
*	                rectangle this would render into.  Often the
*	                same as te_Width-1.
*	    te_Extent.MaxY - same as tf_YSize-tf_Baseline-1.
*	                The offset from the baseline to the bottom of
*	                the rectangle this would render into.
*
*   SEE ALSO
*	TextLength()  Text()  TextFit()
*	graphics/text.h  graphics/rastport.h
*
**********************************************************************
;   supported characters
;
;   Full Forward, any spacing
;
;		0---------L
;	      K?K?K-----B?B?B
;	    F?F?F?F?F?F?F?F?F?F
;
;   Full Reverse, any spacing
;
;		L---------0
;	      K?K?K-----B?B?B
;	    F?F?F?F?F?F?F?F?F?F
;
;   Kernless Forward, no reversing spacing
;		0---------L
;		+-------B?B
;		F?F?F?F?F?F?F
;   
_TextLength:
		ext.l	d0
		beq.s	tlEmpty
		movem.l d2-d6/a2-a5,-(a7)
		bsr.s	textLength
		movem.l	(a7)+,d2-d6/a2-a5
		ext.l	d0
tlEmpty:
		rts
	    
_TextExtent:
		tst.w	d0
		beq.s	zeroExtent
		movem.l d2-d6/a2-a5,-(a7)
		bsr.s	textLength
		move.w	d0,te_Width(a2)
		subq.w	#1,d1
		move.w	d1,te_Extent+ra_MaxX(a2)
		move.w	d2,te_Extent+ra_MinX(a2)
		bsr.s	yExtent
		movem.l	(a7)+,d2-d6/a2-a5
		rts

yExtent:
		move.w	tf_YSize(a1),d0
		move.w	d0,te_Height(a2)
		move.w	tf_Baseline(a1),d1
		neg.w	d1
		move.w	d1,te_Extent+ra_MinY(a2)
		add.w	d0,d1
		subq.w	#1,d1
		move.w	d1,te_Extent+ra_MaxY(a2)
		rts

zeroExtent:
		moveq	#-1,d0
		move.l	d0,te_Extent+ra_MaxX(a2)
		moveq	#0,d0
		move.l	d0,te_Width(a2)
		move.l	d0,te_Extent+ra_MinX(a2)
		rts			; must return d0 = 0 for TextFit call


;------ textLength ---------------------------------------------------
;	length, max, min = textLength(strLen, string, rastPort)
;	d0      d1   d2               d0      a0      a1
;
;	write.asm reference allows to destroy d0-d7/a0-a5
;
;	d3	decrementing strLen
;	d4	rp_AlgoStyle, rp_TxSpacing
;	d5	char index
;	d6	temp
;	a1	rastPort, then textFont
;	a3	textFontExtension, then tf_CharSpace
;	a4	tf_CharKern
;	a5	tf_CharLoc
;
;---------------------------------------------------------------------
;
;	spacing that reverses text is only completely supported for
;	kernless characters -- other cases will risk unrendered
;	background seams in JAM2 mode.
;
textLength:
	    ;-- up front initialization
		move.b	rp_AlgoStyle(a1),d4
		swap	d4
		move.w	rp_TxSpacing(a1),d4
		move.l	rp_Font(a1),a1
		move.l	tf_Extension(a1),a3
		bne.s	tlSpacingExists	; rp_TxSpacing

		btst	#TE0B_BYTECELL,tfe_Flags0(a3)
		bne.s	tlByteCase

tlSpacingExists:
		btst	#TE0B_KERNLESS,tfe_Flags0(a3)
		bne.s	tlFixedCase

	    ;-- full kern/space case
		;-- initialize max/min
		move.w	d0,d3		; strLen for decrementing
		moveq	#0,d0		; zero length
		moveq	#0,d1		; always include CP in bits
		moveq	#0,d2		;

		move.l	tf_CharSpace(a1),a3
		move.l	tf_CharKern(a1),a4
		move.l	tf_CharLoc(a1),a5
		bra.s	tlBothDBF


		;-- kern & space loop
tlBothLoop:
		clr.w	d5
		move.b	(a0)+,d5
		;--	get character bounder
		cmp.w	#$20ff,tf_LoChar(a1)
		beq.s	tlbStdBound

		cmp.b	tf_HiChar(a1),d5
		bhi.s	tlbBadChar
		sub.b	tf_LoChar(a1),d5
		bcc.s	tlbGotChar

tlbBadChar:
		move.b	tf_HiChar(a1),d5
		sub.b	tf_LoChar(a1),d5
		addq.w	#1,d5
		bra.s	tlbGotChar

tlbStdBound
		sub.b	#$20,d5
		bcs.s	tlbBadChar

tlbGotChar:
		add.w	d5,d5
		add.w	0(a4,d5.w),d0	; CP + Kern
		cmp.w	d0,d2		; min
		ble.s	tlbAddSpace
		move.w	d0,d2
tlbAddSpace:
		move.w	d0,d6		; save kern for bit size later
		add.w	0(a3,d5.w),d0	; CP + Space
		add.w	d4,d0		; CP + Spacing

		add.w	d5,d5
		add.w	2(a5,d5.w),d6	; Loc bit size
		cmp.w	d6,d1		; max
		bge.s	tlbCPBound
		move.w	d6,d1

		;-- bound to CP
tlbCPBound:
		cmp.w	d0,d1		; max
		bge.s	tlbCPBMin
		move.w	d0,d1
		bra.s	tlBothDBF
tlbCPBMin
		cmp.w	d0,d2		; min
		ble.s	tlBothDBF
		move.w	d0,d2

tlBothDBF:
		dbf	d3,tlBothLoop
		bra.s	tlAlgoStyleAdj


	    ;-- byte case
tlByteCase:
		lsl.w	#3,d0
		move.w	d0,d1
		moveq	#0,d2
		bra.s	tlAlgoStyleAdj


	    ;-- fixed width case
tlFixedCase:
		moveq	#0,d2			; min is zero
		move.w	tf_XSize(a1),d3
		add.w	d4,d3
		bmi.s	tlIllegalReverse
		mulu	d3,d0			; get CP advance
		move.w	d0,d1			; clone in max
		tst.w	d4			; check for CP < max
		bpl.s	tlAlgoStyleAdj
		sub.w	d4,d1			; max is more than CP advance

tlAlgoStyleAdj:
		btst	#FSB_BOLD+16,d4
		beq.s	tlaChkItalic
		add.w	tf_BoldSmear(a1),d1
tlaChkItalic:
		btst	#FSB_ITALIC+16,d4
		beq.s	tlDone
		move.w	tf_Baseline(a1),d5
		move.w	tf_YSize(a1),d6
		sub.w	d5,d6
		addq.w	#1,d6
		lsr.w	#1,d5
		lsr.w	#1,d6
		add.w	d5,d1
		sub.w	d6,d2
tlDone:
		rts


tlIllegalReverse:
		moveq	#0,d0
		moveq	#0,d1
		bra.s	tlDone


******* graphics.library/TextFit *************************************
*
*   NAME
*	TextFit - count characters that will fit in a given extent (V36)
*
*   SYNOPSIS
*	chars = TextFit(rastport, string, strLen, textExtent,
*	D0              A1        A0      D0      A2
*	        constrainingExtent, strDirection,
*	        A3                  D1
*	        constrainingBitWidth, constrainingBitHeight)
*	        D2                    D3
*
*	ULONG TextFit(struct RastPort *, STRPTR, UWORD,
*	    struct TextExtent *, struct TextExtent *, WORD, UWORD, UWORD);
*
*   FUNCTION
*	This function determines how many of the characters of the
*	provided string will fit into the space described by the
*	constraining parameters.  It also returns the extent of
*	that number of characters.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text attributes reside.
*	string - the address of string to determine the constraint of
*	strLen - The number of characters in the string.
*	         If zero, there are no characters in the string.
*	textExtent - a structure to hold the extent result.
*	constrainingExtent - the extent that the text must fit in.
*	    This can be NULL, indicating only the constrainingBit
*	    dimensions will describe the constraint.
*	strDirection - the offset to add to the string pointer to
*	    get to the next character in the string.  Usually 1.
*	    Set to -1 and the string to the end of the string to
*	    perform a TextFit() anchored at the end.  No other value
*	    is valid.
*	constrainingBitWidth - an alternative way to specify the
*	    rendering box constraint width that is independent of
*	    the rendering origin.  Range 0..32767.
*	constrainingBitHeight - an alternative way to specify the
*	    rendering box constraint height that is independent of
*	    the rendering origin.  Range 0..32767.
*
*   RESULTS
*	chars - the number of characters from the origin of the
*	        given string that will fit in both the constraining
*	        extent (which specifies a CP bound and a rendering
*	        box relative to the origin) and in the rendering width
*	        and height specified.
*
*   NOTES
*	The result is zero chars and an empty textExtent when the fit
*	cannot be performed.  This occurs not only when no text will
*	fit in the provided constraints, but also when:
*	-   the RastPort rp's rp_TxSpacing sign and magnitude is so
*	    great it reverses the path of the text. 
*	-   the constrainingExtent does not include x = 0.
*
*
*   BUGS
*	Under V37, TextFit() would return one too few characters if the
*	font was proportional. This can be worked around by passing
*	(constrainingBitWidth + 1) for proportional fonts. This is fixed
*	for V39.
*
*   SEE ALSO
*	TextExtent()  TextLength()  Text()
*	graphics/text.h  graphics/rastport.h
*
**********************************************************************
tfZeroExtent:
		add.w	#te_SIZEOF+8,a7
		movem.l	(a7)+,d2-d7/a3-a6
		bsr	zeroExtent		; sets d0 = 0
		rts

_TextFit:
	    ;-- save registers
		movem.l d2-d7/a3-a6,-(a7)

		clr.l	-(a7)			; store for bounding width &
		clr.l	-(a7)			;   last chars CP/max/min

		;-- build default constraining extent
		move.l	#$7fff7fff,-(a7)	; te_Extent.Max
		move.l	#$80008000,-(a7)	; te_Extent.Min
		move.l	#$7fff7fff,-(a7)	; te_Width & te_Height
		btst.b	#FPB_REVPATH,rp_TxFlags(a1)
		beq.s	tfPWidth
		not.w	2(a7)			; te_Width -> $8000
tfPWidth:
		move.l	a7,a6
	    
		;-- check zero string length
		tst.w	d0
		beq.s	tfZeroExtent

		;-- check early reason to allow no characters
		;--	check constrainingBitHeight
		move.w	rp_TxHeight(a1),d7
		cmp.w	d7,d3
		blt.s	tfZeroExtent

	    ;-- get constraining extent
		move.l	a3,d6
		beq.s	tfCheckX

		;-- copy specified constraining extent
		move.l	(a3)+,(a6)
		move.l	(a3)+,4(a6)
		move.l	(a3)+,8(a6)

		;-- check more early reasons to allow no characters
		;--	check constrainingExtent MaxY
		move.w	rp_TxBaseline(a1),d6
		neg.w	d6			; d6=-baseline
		cmp.w	te_Extent+ra_MinY(a6),d6
		blt.s	tfZeroExtent		; is -baseline < miny?
		;--	check constrainingExtent MinY
		add.w	d7,d6			; d6=height-base
		subq.w	#1,d6			; height-base-1
		cmp.w	te_Extent+ra_MaxY(a6),d6
		bgt.s	tfZeroExtent		; abort if h-base-1 > maxy

;
;	d0	strLen
;	d1	string direction
;	d2	bounding bit size
;	d4	rp_AlgoStyle / rp_TxSpacing
;	d7	temp
;	a0	string
;	a1	rastPort, then textFont
;	a6	constraining te_Extent adjusted for algorithmic style
;
tfCheckX:
	    ;-- up front initialization
		move.b	rp_AlgoStyle(a1),d4
		swap	d4
		move.w	rp_TxSpacing(a1),d4

		move.l	rp_Font(a1),a1

		;-- trim constraints for algorithmic elbow room
		btst	#FSB_BOLD+16,d4
		beq.s	tfConstrainItalic

		move.w	tf_BoldSmear(a1),d7
		sub.w	d7,d2
		sub.w	d7,te_Extent+ra_MaxX(a6)

tfConstrainItalic:
		btst	#FSB_ITALIC+16,d4
		beq.s	tfEnsureX00

		move.w	tf_Baseline(a1),d5
		move.w	tf_YSize(a1),d6
		sub.w	d5,d6
		addq.w	#1,d6
		lsr.w	#1,d5
		lsr.w	#1,d6
		sub.w	d5,te_Extent+ra_MaxX(a6)
		sub.w	d5,d2
		add.w	d6,te_Extent+ra_MinX(a6)
		sub.w	d6,d2

tfEnsureX00:
		;--	check constrainingExtent MaxX
		tst.w	te_Extent+ra_MaxX(a6)
		blt	tfZeroExtent
		;--	check constrainingExtent MinX
		tst.w	te_Extent+ra_MinX(a6)
		bgt	tfZeroExtent

		move.l	tf_Extension(a1),a4
		btst	#TE0B_KERNLESS,tfe_Flags0(a4)
		bne	tfFixedCase

;
;	d0	cumulative length
;	d1	sign of string direction / working max
;	d2	working min
;	d3	decrementing strLen
;	d4	rp_AlgoStyle / rp_TxSpacing
;	d5	char index
;	d6	temp
;	d7	original strLen / temp
;	a0	string
;	a1	rastPort, then textFont
;	a3	tf_CharSpace
;	a4	tf_CharKern
;	a5	tf_CharLoc
;	a6	constraining te_Extent adjusted for algorithmic style,
;		store for last character's CP/max/min
;
		move.w	d0,d7		; original strLen
		swap	d7		;   in high word
		move.l	tf_CharSpace(a1),a3
		move.l	tf_CharKern(a1),a4
		move.l	tf_CharLoc(a1),a5
		move.w	d0,d3
		ext.l	d1		; move direction into sign bit
		move.w	d2,te_SIZEOF(a6) ; bounding bit width
		moveq	#0,d0		; working CP = 0;
		clr.w	d1		; working max = 0
		moveq	#0,d2		; working min = 0
		moveq	#-1,d5		; initialize to illegal value

		bra	tfBothDBF

		;-- kern & space loop
tfBothLoop:
		clr.w	d5
		move.b	(a0),d5
		cmp.w	#$20ff,tf_LoChar(a1)
		beq.s	tfbStdBound

		cmp.b	tf_HiChar(a1),d5
		bhi.s	tfbBadChar
		sub.b	tf_LoChar(a1),d5
		bcc.s	tfbGotChar

tfbBadChar:
		move.b	tf_HiChar(a1),d5
		sub.b	tf_LoChar(a1),d5
		addq.w	#1,d5
		bra.s	tfbGotChar

tfbStdBound:
		sub.b	#$20,d5
		bcs.s	tfbBadChar

tfbGotChar:
		add.w	d5,d5
		;-- get new CP
		move.w	0(a4,d5.w),d7	; Kern
		add.w	0(a3,d5.w),d7	; + Space
		add.w	d4,d7		; + Spacing
		add.w	d0,d7
		move.w	te_Width(a6),d6	; check new width
		bpl.s	tfbPosWidth

		;--	check negative width constraint
		tst.w	d7
		bpl.s	tfbCheckDirection
		cmp.w	d6,d7
		bge.s	tfbCheckDirection
		bra.s	tfExhausted

tfbPosWidth:
		;--	check positive width constraint
		tst.w	d7
		bmi.s	tfbCheckDirection
		cmp.w	d6,d7
		bgt.s	tfExhausted

		;-- check string direction
tfbCheckDirection:
		tst.l	d1
		bmi	tfbRightFit

		;-- left fit
		addq.w	#1,a0
		add.w	0(a4,d5.w),d0	; CP + Kern

tfCheckBitMin:
		cmp.w	d0,d2		; min
		ble.s	tfbCheckBitMax
		cmp.w	te_Extent+ra_MinX(a6),d0 ; out of bounds?
		blt.s	tfExhausted
		move.w	d0,d2
tfbCheckBitMax:
		add.w	d5,d5		; double d5 temporarily
		add.w	2(a5,d5.w),d0	; Loc bit size
		lsr.w	#1,d5		; restore d5

		cmp.w	d0,d1		; max
		bge.s	tfbSetCP
		cmp.w	te_Extent+ra_MaxX(a6),d0 ; out of bounds?
		bgt.s	tfExhausted
		move.w	d0,d1

tfbSetCP:
		move.w	d7,d0

		;-- include CP in bound
tfbCheckCP:
		cmp.w	d0,d2		; min
		ble.s	tfbChkWMax
		cmp.w	te_Extent+ra_MinX(a6),d0 ; out of bounds?
		blt.s	tfExhausted
		move.w	d0,d2
tfbChkWMax:
		cmp.w	d0,d1		; max
		bge.s	tfbChkBWidth
		cmp.w	te_Extent+ra_MaxX(a6),d0 ; out of bounds?
		bgt.s	tfExhausted
		move.w	d0,d1

		;--	check constrainingBitWidth
tfbChkBWidth:
		move.w	d1,d7		; working max
		sub.w	d2,d7		; max - min
		cmp.w	te_SIZEOF(a6),d7 ;   out of bounds?
*		bge.s	tfExhausted	; make inclusive
		bgt.s	tfExhausted

tfbCacheLast:
		movem.w	d0/d1/d2,te_SIZEOF+2(a6) ; save working to last char's
tfBothDBF:
		swap	d5
		dbf	d3,tfBothLoop


	    ;-- proportional wrapup
tfExhausted:
		swap	d7		; recover original strLen
		sub.w	d3,d7		; find what was used up
		subq.w	#1,d7		; this is the chars result
		tst.l	d1
		bmi.s	1$
		movem.w	te_SIZEOF+2(a6),d0/d1/d2 ; get CP, max, and min
2$		bsr	tlAlgoStyleAdj
		bra	tfResult

* direction is right to left. Handle special cases:

1$		movem.w	te_SIZEOF+2(a6),d0/d1/d2 ; get CP, max, and min
		tst.l	d5
		bmi.s	2$		; no previous character
		swap	d5		; get the previous character (which will have fit)
		move.w	0(a4,d5.w),d3	; final Kern
*		bmi.s	2$		; check for positive sign
		bmi.s	3$
		sub.w	d3,d2		; min - final Kern if positive
*		bra.s	2$

* have to check for -ve space in the font definition. 
* - spence Dec 12 1990

3$		move.w	0(a3,d5.w),d3	; previous space
		bpl.s	2$
*		add.w	d3,d2
		clr.w	d2		; BLEUCHHH! (but it makes Peter happy)
		bra.s	2$

		;-- right fit
tfbRightFit:
		subq.w	#1,a0
		sub.w	d0,d7		; recover CP increment
		add.w	d7,d1		; adjust max/min
		add.w	d7,d2

		;--	check adjusted max against constraining bit size

		cmp.w	te_SIZEOF(a6),d1 ;   out of bounds?
		bge.s	tfExhausted	; make inclusive

		;--	check adjusted max/min
		cmp.w	te_Extent+ra_MaxX(a6),d1
		bgt.s	tfExhausted
		cmp.w	te_Extent+ra_MinX(a6),d2
		blt.s	tfExhausted

		add.w	d0,d7		; recover next CP
		move.w	0(a4,d5.w),d0	; Kern
		bra	tfCheckBitMin


;
;	d0	strLen, then length
;	d1	, then max
;	d2	bounding bit size, then min
;	d4	rp_AlgoStyle / rp_TxSpacing
;	d5	fit count
;	d6	tf_XSize
;	d7	temp, then fitLen
;	a1	textFont
;	a2	result te_Extent
;	a6	constraining te_Extent adjusted for algorithmic style
;
	    ;-- fixed width case
tfFixedCase:
		moveq	#0,d5
		move.w	tf_XSize(a1),d7
		add.w	d4,d7		; add rp_TxSpacing
		blt	tfZeroExtent
		beq.s	tffHaveD5

		;--	get CP advance contribution
		move.w	te_Width(a6),d5
		bmi.s	tffHaveD5
		divu	d7,d5
		beq	tfZeroExtent

		;--	get bit contribution
tffHaveD5:
		moveq	#0,d1
		move.w	te_Extent+ra_MaxX(a6),d1
		cmp.w	d2,d1
		ble.s	tffBitSrc
		move.w	d2,d1
tffBitSrc:
		tst.w	d7
		beq.s	tffZeroAdvance
		tst.w	d4
		bpl.s	tffBitDest
		add.w	d4,d1		; without contribution of last spacing
tffBitDest:

		divu	d7,d1

		tst.w	d5
		beq.s	tffMinD0D1
		cmp.w	d1,d5
		bgt.s	tffMinD0D1
		move.w	d5,d1
tffMinD0D1:
		cmp.w	d0,d1
		bgt.s	tffD0
		move.w	d1,d0

tffD0:
		move.w	d0,d7
		bsr	tlFixedCase

tfResult:
		move.w	d0,te_Width(a2)
		subq.w	#1,d1
		move.w	d1,te_Extent+ra_MaxX(a2)
		move.w	d2,te_Extent+ra_MinX(a2)
		bsr	yExtent
		moveq	#0,d0
		move.w	d7,d0
		add.w	#te_SIZEOF+8,a7
		movem.l	(a7)+,d2-d7/a3-a6
		rts


tffZeroAdvance:
		add.w	d4,d1		; see if room for bits
		bge.s	tffD0		; yes: enough room for all characters
		bra	tfZeroExtent


	END
