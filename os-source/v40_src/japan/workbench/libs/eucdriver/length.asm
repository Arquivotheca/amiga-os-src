**
**	$Id: length.asm,v 1.1 93/02/01 12:18:31 darren Exp $
**
**      EUC text fitting primitives
**
**      (C) Copyright 1992-1993 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		eucdriver


*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/macros.i"
	INCLUDE		"exec/alerts.i"
	include		"exec/execbase.i"

	INCLUDE		"graphics/text.i"

	INCLUDE		"graphics/gfx.i"
	INCLUDE		"graphics/clip.i"
	INCLUDE		"graphics/rastport.i"
	INCLUDE		"graphics/gfxbase.i"
	INCLUDE		"graphics/text.i"

	INCLUDE		"hardware/blit.i"

	INCLUDE		"driver.i"
	INCLUDE		"debug.i"

*------ Imported Names -----------------------------------------------

	IFEQ	FAST_PRECHAR
		XREF	EUCSetPreChar
		XREF	EUCGetPreChar
	ENDC

*------ Exported Names -----------------------------------------------

	XDEF		_EUCTextLength
	XDEF		_EUCtextLength
	XDEF		_EUCTextExtent
	XDEF		_EUCTextFit

*------ Functions ----------------------------------------------------

*------ Macros -------------------------------------------------------

*------ Assumptions --------------------------------------------------

	IFNE	EUC_HICHAR-$FF
	FAIL	"EUC high char is not $FF; recode!"
	ENDC

******* eucdriver.library/EUCTextFit *********************************
*
*   NAME
*	EUCTextFit - count "bytes" that will fit in a given extent.
*
*   SYNOPSIS
*	chars = EUCTextFit(rastport, string, strLen, textExtent,
*	D0                 A1        A0      D0      A2
*	        constrainingExtent, strDirection,
*	        A3                  D1
*	        constrainingBitWidth, constrainingBitHeight)
*	        D2                    D3
*
*	ULONG EUCTextFit(struct RastPort *, STRPTR, UWORD,
*	    struct TextExtent *, struct TextExtent *, WORD, UWORD, UWORD);
*
*   FUNCTION
*	See graphics/TextFit().  Like EUCTextLength(), does not
*	take into account any pre-character byte.  See note below
*	regarding interpretation of this functions return value.
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
*	    is valid.  Note that this description is not entirely
*	    true for EUC, rather -1 means negative text direction
*	    since EUC byte pairs must be interpreted as 1 character.
*	constrainingBitWidth - an alternative way to specify the
*	    rendering box constraint width that is independent of
*	    the rendering origin.  Range 0..32767.
*	constrainingBitHeight - an alternative way to specify the
*	    rendering box constraint height that is independent of
*	    the rendering origin.  Range 0..32767.
*
*   RESULTS
*	chars - See graphics/TextFit().  The result must be
*		interpreted as the number of BYTES which can be
*		printed with the Text() function, and not the
*		number of Japanese characters (or glyphs) since
*		EUC is a mixed single/double byte codeset.
*
*   NOTES
*	This function interprets text strings as EUC strings, so
*	returns a "chars" value on single byte, or EUC double byte
*	boundaries.  This is true whether or not you indicate
*	positive, or negative direction, however if the string
*	ends in an ambiguous EUC byte (a single byte whose value is
*	>= 0xA1), the resulting "chars" length is interpreted via
*	the following rules:
*
*	o If the entire string fits in the contraints, "chars"
*	  is equal to the length of the string (number of BYTES)
*	  including the ambiguous byte.
*	o If a postive string direction is indicated in register
*	  D1, and the entire string does not fit, the ambiguous
*	  byte will be truncated along with whatever number of
*	  bytes must be truncated to fit in the contraints.
*	o If a negative string direction is indicated in register
*	  D1, and the entire string does not fit, the amiguous
*	  byte plus N non-ambiguous bytes proceeding it will be
*	  returned in register D0.
*	o In all cases, the string is assumed to start on a
*	  non-ambiguous EUC boundary.  Random results can be
*	  returned if you arbitrarily select string starting
*	  position in the middle of an EUC byte pair since this
*	  function will then incorrectly parse/identify EUC
*	  byte pairs.  For example --
*
*	  0xA1, 0xA2, 0xBE, 0xBF
*
*	  If these bytes represent two EUC characters, but the
*	  string starting position is submitted starting on 0xA2,
*	  then this routine will parse 0xA2BE as an EUC byte pair, and
*	  0xBF as an ambiguous ending byte.
*	  
*	  This kind of parsing problem has never been true in the
*	  past because of the assumption that all character strings can
*	  be treated as single byte, ECMA Latin1 character streams.
*
*   BUGS
*
*   SEE ALSO
*
**********************************************************************
tfZeroExtent:
		add.w	#(te_SIZEOF),sp
		movem.l	(sp)+,d2-d7/a2-a6
		bra	ete_zeroExtent		; returns 0 in d0

_EUCTextFit:
	PRINTF	DBG_ENTRY,<'EUC--EUCTextFit()'>

		movem.l	d2-d7/a2-a6,-(sp)

	; build default contraint extent

		move.l	#$7fff7fff,-(sp)	; te_Extent.Max
		move.l	#$80008000,-(sp)	; te_Extent.Min
		move.l	#$7fff7fff,-(sp)	; te_Width & te_Height

		move.l	sp,a6

	; check for zero length string

		tst.w	d0
		beq.s	tfZeroExtent

	PUSHWORD	D0
	PRINTF	DBG_FLOW,<'EUC--[TextFit %ld bytes]'>
	POPLONG		1

	; check early reasons to allow no characters

		move.w	rp_TxHeight(a1),d7

	PUSHWORD	D7
	PRINTF	DBG_FLOW,<'EUC--[Font Height %ld]'>
	POPLONG		1

		cmp.w	d7,d3			;contraining bit height
		BLT_S	tfZeroExtent

	PUSHWORD	D3
	PRINTF	DBG_FLOW,<'EUC--[Font height <= contraint %ld]'>
	POPLONG		1

	; and check contraining extent of provided

		move.l	a3,d6
		BEQ_S	tfCheckX

		; copy specified constraints

		move.l	(a3)+,(a6)		;te_Width & te_Height
		move.l	(a3)+,4(a6)		;te_Extent.Min
		move.l	(a3)+,8(a6)		;te_Extent.Max

	PRINTF	DBG_FLOW,<'EUC--[Contraining Extent provided]'>

	PUSHWORD	te_Width(A6)
	PRINTF	DBG_FLOW,<'EUC--[te_Width = %ld]'>
	POPLONG		1

	PUSHWORD	te_Height(A6)
	PRINTF	DBG_FLOW,<'EUC--[te_Height = %ld]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MinY(A6)
	PRINTF	DBG_FLOW,<'EUC--[MinY = $%lx]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MaxY(A6)
	PRINTF	DBG_FLOW,<'EUC--[MaxY = %ld]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MinX(A6)
	PRINTF	DBG_FLOW,<'EUC--[MinX = $%lx]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MaxX(A6)
	PRINTF	DBG_FLOW,<'EUC--[MaxX = %ld]'>
	POPLONG		1

		; check contraining Extent Min Y

		move.w	rp_TxBaseline(a1),d6

	PUSHWORD	D6
	PRINTF	DBG_FLOW,<'EUC--[Font Baseline %ld]'>
	POPLONG		1

		neg.w	d6			;min is -baseline
		cmp.w	te_Extent+ra_MinY(a6),d6
		BLT_S	tfZeroExtent

	PRINTF	DBG_FLOW,<'EUC--[MinY GE MinY extent of font]'>

		; check contraining Extent Max Y

		add.w	d7,d6			;height + (-baseline)
		subq.w	#1,d6			;-1 (not inclusive)
		cmp.w	te_Extent+ra_MaxY(a6),d6
		BGT_S	tfZeroExtent

	PRINTF	DBG_FLOW,<'EUC--[MaxY GE MaxY extent of font]'>

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
        ; up front initialization
		move.b	rp_AlgoStyle(a1),d4

	PUSHBYTE	D4
	PRINTF	DBG_FLOW,<'EUC--[rp_AlgoStyle = $%lx]'>
	POPLONG		1

		swap	d4

		move.w	rp_TxSpacing(a1),d4

	; do not allow negative spacing

		BMI_S	tfZeroExtent

	PUSHWORD	D4
	PRINTF	DBG_FLOW,<'EUC--[rp_TxSpacing = $%lx]'>
	POPLONG		1

		move.l	rp_Font(a1),a1

		;-- trim constraints for algorithmic elbow room
		btst	#FSB_BOLD+16,d4
		BEQ_S	tfConstrainItalic

		move.w	tf_BoldSmear(a1),d7
		sub.w	d7,d2
		sub.w	d7,te_Extent+ra_MaxX(a6)

	PUSHWORD	D2
	PRINTF	DBG_FLOW,<'EUC--[BOLD NEW D2 constraint %ld]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MaxX(a6)
	PRINTF	DBG_FLOW,<'EUC--[BOLD MaxX constraint %ld]'>
	POPLONG		1


tfConstrainItalic:
		btst	#FSB_ITALIC+16,d4
		BEQ_S	tfEnsureX00

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

	PUSHWORD	D2
	PRINTF	DBG_FLOW,<'EUC--[ITALIC NEW D2 constraint %ld]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MaxX(a6)
	PRINTF	DBG_FLOW,<'EUC--[ITALIC MaxX constraint %ld]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MinX(a6)
	PRINTF	DBG_FLOW,<'EUC--[ITALIC MinX constraint $%lx]'>
	POPLONG		1


tfEnsureX00:
		;--	check constrainingExtent MaxX
		tst.w	te_Extent+ra_MaxX(a6)
		blt	tfZeroExtent

	PRINTF	DBG_FLOW,<'EUC--[MaxX contraint GE 0]'>

		;--	check constrainingExtent MinX
		tst.w	te_Extent+ra_MinX(a6)
		bgt	tfZeroExtent

	PRINTF	DBG_FLOW,<'EUC--[MinX contraint LE 0]'>



;
;	d0	strLen, then length
;	d1	direction, then temp, thenmax
;	d2	bounding bit size, then width count, then min
;	d3	character count
;	d4	rp_AlgoStyle / rp_TxSpacing
;	d5	te_Width
;	d6	bounding bit size
;	d7	XSize of char
;	a1	textFont
;	a2	result te_Extent
;	a6	constraining te_Extent adjusted for algorithmic style
;

		move.w	te_Width(a6),d5
		bmi	tfZeroExtent		;if width is negative, no way

	PUSHWORD	D5
	PRINTF	DBG_FLOW,<'EUC--[te_Width %ld]'>
	POPLONG		1

	PUSHWORD	D2
	PRINTF	DBG_FLOW,<'EUC--[contraint width %ld]'>
	POPLONG		1


		moveq	#00,d6
		move.w	te_Extent+ra_MaxX(a6),d6
	; we are going to bump MaxX by 1, since it is nominally 1 less
	; than te_Width, but lets not bother if its been set to $7FFF
	; which indicates no bounds were set by the caller.  We also
	; know that MaxX is not negative because of a previous test
	; before we got here

		cmp.w	#$7fff,d6
		beq.s	tf_maxmaxx

		addq.w	#1,d6
tf_maxmaxx:

	PUSHWORD	D6
	PRINTF	DBG_FLOW,<'EUC--[MaxX+1 == %ld]'>
	POPLONG		1

		cmp.w	d2,d6
		ble.s	tf_haveminmax

	PRINTF	DBG_FLOW,<'EUC--[MaxX GT contraint width / swap]'>

		move.w	d2,d6			;this is smaller, so its the max

tf_haveminmax:

	; normally MaxX will be >= te_Width, but could be smaller if
	; someone sets them equal, but algol styling is applied.  For example,
	; te_Width == 8, and MaxX = 9, but algol italics requires 2 pixels
	; greater than width, so therefore MaxX is made equal to 7.  This
	; is ok to use as a max value since it is less than te_Width, and
	; if used, means MaxX will work out to 9 once we do algol styling
	;
	; only if it works out that less chars fit in te_Width than MaxX
	; do we need to bound, since it is required that the chars fit in
	; te_Width before algol style is applied

		cmp.w	d5,d6
		ble.s	tf_havemaxmax

	PRINTF	DBG_FLOW,<'EUC--[MaxX GT te_Width / use te_Width for CP bounding]'>

		move.w	d5,d6

tf_havemaxmax:
		moveq	#00,d2			;width count
		move.w	d0,d3			;if zero width, all chars fit

		move.w	tf_XSize(a1),d7

	PUSHWORD	D7
	PRINTF	DBG_FLOW,<'EUC--[tf_XSize %ld]'>
	POPLONG		1

		add.w	d4,d7
		beq.s	tf_havemax

		sub.w	d4,d7			;restore (now XSize)

		moveq	#00,d3			;char count not known

		tst.w	d1
		bmi	tf_Reversed		;read from end of string

	PRINTF	DBG_FLOW,<'EUC--[Forward direction]'>

	; for forward direction, must interpret EUC bytes

		bra.s	tf_frwdirec
tf_forwardsize:

		tst.w	d0			;if last char, never EUC
		beq.s	frw_oneascii

		move.b	(a0)+,d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	frw_oneascii

		move.b	(a0),d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	frw_oneascii

	; is EUC pair - see if it will fit

		subq.w	#1,d0
		addq.l	#1,a0

		sub.w	d7,d6			;XSize * 2 + TxSpacing 
		sub.w	d7,d6
		sub.w	d4,d6
		bmi.s	tf_havemax

		add.w	d7,d2
		add.w	d7,d2
		add.w	d4,d2
		addq.w	#2,d3			;2 more bytes fits
		bra.s	tf_frwdirec

frw_oneascii:
		sub.w	d7,d6			;if MaxX exhausted?
		sub.w	d4,d6
		bmi.s	tf_havemax

		add.w	d7,d2			;nope, so +XSize+TxSpacing
		add.w	d4,d2
		addq.w	#1,d3			;1 more byte fits
tf_frwdirec:
		dbf	d0,tf_forwardsize

tf_havemax:

	PRINTF	DBG_FLOW,<'EUC--[Char fit count %ld]'>,D3

	PRINTF	DBG_FLOW,<'EUC--[result te_Width = %ld]'>,D2

	; must modify XMax for BOLD and ITALIC algol style

		move.w	d2,te_Width(a2)
		move.w	d2,d1			;MAX
		moveq	#00,d2			;MIN

		btst	#FSB_BOLD+16,d4
		beq.s	tf_ChkItalic
		add.w	tf_BoldSmear(a1),d1

tf_ChkItalic:
		btst	#FSB_ITALIC+16,d4
		beq.s	tf_setextent

		move.w	tf_Baseline(a1),d5
		move.w	tf_YSize(a1),d6
		sub.w	d5,d6
		addq.w	#1,d6
		lsr.w	#1,d5
		lsr.w	#1,d6
		add.w	d5,d1
		sub.w	d6,d2

tf_setextent:
		subq.w	#1,d1

	PUSHWORD	D1
	PRINTF	DBG_FLOW,<'EUC--[result MaxX = %ld]'>
	POPLONG		1

	PUSHWORD	D2
	PRINTF	DBG_FLOW,<'EUC--[result MinX = $%lx]'>
	POPLONG		1


		move.w	d1,te_Extent+ra_MaxX(a2)
		move.w	d2,te_Extent+ra_MinX(a2)
		move.l	a2,a3
		move.l	a1,a2
		bsr	yExtent

	PUSHWORD	te_Extent+ra_MinY(a3)
	PRINTF	DBG_FLOW,<'EUC--[result MinY = $%lx]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MaxY(a3)
	PRINTF	DBG_FLOW,<'EUC--[result MaxY = %ld]'>
	POPLONG		1


		move.l	d3,d0			;return char length (clears high word)

	PRINTF	DBG_FLOW,<'EUC--[result char count = $%ld]'>,D0


		add.w	#(te_SIZEOF),sp
		movem.l	(sp)+,d2-d7/a2-a6
		rts

	; the reverse case must be parsed forward and then backwards
	; again so that we are in sync with the EUC stream - failure
	; to do this can cause confusion if the stream ends in an
	; ambiguous character

tf_Reversed:
	PRINTF	DBG_FLOW,<'EUC--[Reverse direction]'>

	; if the last character is not a potential EUC character, we
	; can just skip this sync test

		move.b	(a0),d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	tf_RevSynced
		
	PRINTF	DBG_FLOW,<'EUC--[Must scan forward first]'>

		move.w	d0,d5
		move.l	a0,a3			;use A3, save A0
		sub.w	d0,a3			;find beg of string
		addq.w	#1,a3			;if 1 byte long, same byte

		bra.s	tf_RevSyncEnd
tf_RevSync:
		moveq	#00,d1			;not in EUC state

		tst.w	d5
		beq.s	tf_RevSynced		;last byte - cannot be EUC pair

		move.b	(a3)+,d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	tf_RevSyncEnd

		move.b	(a3),d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	tf_RevSyncEnd

		subq.w	#1,d5
		addq.l	#1,a3			;d1 is in EUC sync mode

tf_RevSyncEnd:
		dbf	d5,tf_RevSync
tf_RevSynced:

	PUSHBYTE	D1
	PRINTF	DBG_FLOW,<'EUC--[Last BYTE of string $%lx]'>
	POPLONG		1

	; now if we are in EUC state, then we must start by comparing the
	; current character, and the previous character, but if we are
	; not in EUC state, just treat the string end character as a
	; ambiguous, or single byte char

		subq.w	#1,d0

		cmp.b	#EUC_LOCHAR,d1
		bcs.s	tf_revasciistate

		addq.w	#1,a0			;use pre-decrement mode

tf_reversesize:
		tst.w	d0
		beq.s	tf_revasciistate

		move.b	-(a0),d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	tf_revasciistate

		move.b	-1(a0),d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	tf_revasciistate

		subq.w	#1,d0
		subq.l	#1,a0

		sub.w	d7,d6			;XSize * 2 + TxSpacing 
		sub.w	d7,d6
		sub.w	d4,d6
		bmi	tf_havemax

		add.w	d7,d2
		add.w	d7,d2
		add.w	d4,d2
		addq.w	#2,d3			;2 more bytes fits
		bra.s	tf_revdirec

tf_revasciistate:
		
		sub.w	d7,d6
		sub.w	d4,d6
		bmi	tf_havemax

		add.w	d7,d2
		add.w	d4,d2
		addq.w	#1,d3
tf_revdirec:
		dbf	d0,tf_reversesize
		bra	tf_havemax

******* eucdriver.library/EUCFontExtent ******************************
*
*   NAME
*	EUCFontExtent -- get the font attributes of the current font
*
*   SYNOPSIS
*	EUCFontExtent(font, fontExtent)
*	              A0    A1
*
*	void EUCFontExtent(struct TextFont *, struct TextExtent *);
*
*   FUNCTION
*	See graphics/FontExtent().
*
*   INPUTS
*	font       - the TextFont from which the font metrics are extracted.
*	fontExtent - the TextExtent structure to be filled.
*
*   RESULT
*	fontExtent is filled.
*
*   NOTES
*	This function is meaningless for a Japanese font, so a
*	redirection vector is not provided.  The te_width returned
*	has no meaning because this function assumes that you plan on
*	taking the result, and multiplying it by the maximum number
*	of columns you want to print, thereby obtaining the maximum
*	bounding box requrired to print a string containing any
*	characters.  This assumes that the codeset of the font is
*	1 byte per 1 glyph per 1 column of text, which is not true
*	for Japanese fonts.  Therefore this function will return
*	an extent equivalent to that which would be returned for any
*	other ECMALATIN1 codeset font, which if you think about it is
*	correct.  Because Japanese double-byte characters are twice as
*	wide as single-byte characters, the multiplication to determine
*	worst case is still true.
*
*   SEE ALSO
*	TextExtent()  graphics/text.h
*
**********************************************************************
******* eucdriver.library/EUCTextExtent ******************************
*
*   NAME
*	EUCTextExtent -- Determine raster extent of text data.
*
*   SYNOPSIS
*	EUCTextExtent(rp, string, count, textExtent)
*	              A1  A0      D0:16  A2
*
*	void EUCTextExtent(struct RastPort *, STRPTR, WORD,
*	     struct TextExtent *);
*
*   FUNCTION
*	See graphics/TextExtent().  Like EUCTextLength(), does
*	not take into account any pre-character byte.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text attributes reside.
*	string - the address of the string to determine the length of.
*	count  - the number of characters in the string.
*                If zero, there are no characters in the string.
*	textExtent - a structure to hold the result.
*
*   RESULT
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
*
*   NOTES
*
*   SEE ALSO
*	EUCTextLength()
*
**********************************************************************

ete_zeroExtent:
		moveq	#-1,d0
		move.l	d0,te_Extent+ra_MaxX(a2)
		moveq	#0,d0
		move.l	d0,te_Width(a2)
		move.l	d0,te_Extent+ra_MinX(a2)
	;must return 0 in D0
		rts

yExtent:
		move.w	tf_YSize(a2),d0
		move.w	d0,te_Height(a3)
		move.w	tf_Baseline(a2),d1
		neg.w	d1
		move.w	d1,te_Extent+ra_MinY(a3)
		add.w	d0,d1
		subq.w	#1,d1
		move.w	d1,te_Extent+ra_MaxY(a3)		
		rts

_EUCTextExtent:

	PUSHWORD	D0
	PRINTF	DBG_ENTRY,<'EUC--EUCTextExtent(%ld)'>
	POPLONG		1

	
		tst.w	d0
		beq.s	ete_zeroExtent

		movem.l	d2/d7/a2-a5,-(sp)
		move.l	a1,a4
		move.l	a0,a3
		move.w	d0,d7
		move.l	a2,a5			;cache
		move.l	rp_Font(a4),a2
		BSR_S	_EUCtrueLength
		move.l	a5,a3
		move.w	d0,te_Width(a3)
		subq.w	#1,d1
		move.w	d1,te_Extent+ra_MaxX(a3)
		move.w	d2,te_Extent+ra_MinX(a3)
		bsr.s	yExtent
		movem.l	(sp)+,d2/d7/a2-a5

	PUSHWORD	te_Width(a2)
	PRINTF	DBG_FLOW,<'EUC--[te_Width = %ld]'>
	POPLONG		1
	
	PUSHWORD	te_Height(a2)
	PRINTF	DBG_FLOW,<'EUC--[te_Height = %ld]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MinY(a2)
	PRINTF	DBG_FLOW,<'EUC--[result MinY = $%lx]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MaxY(a2)
	PRINTF	DBG_FLOW,<'EUC--[result MaxY = %ld]'>
	POPLONG		1


	PUSHWORD	te_Extent+ra_MinX(a2)
	PRINTF	DBG_FLOW,<'EUC--[result MinX = $%lx]'>
	POPLONG		1

	PUSHWORD	te_Extent+ra_MaxX(a2)
	PRINTF	DBG_FLOW,<'EUC--[result MaxX = %ld]'>
	POPLONG		1


		rts

******* eucdriver.library/EUCTextLength ******************************
*
*   NAME
*	EUCTextLength -- Calculates length in pixels of EUC strings
*
*   SYNOPSIS
*	length = EUCTextLength(rp, string, count)
*	D0:16	               A1  A0      D0:16
*
*	WORD EUCTextLength(struct RastPort *, STRPTR, WORD);
*
*   FUNCTION
*	See graphics/TextLength().  Does not check the RastPort
*	for a pre-character byte.  Assumes that a non-ambiguous
*	EUC string is provided, so your calculations may be off
*	if mixing calls to this function with Text() calls
*	printing ambiguous strings.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text is to be output
*	string - the address of string to output
*	length - the number of BYTES in the string.
*	         If zero, there are no characters to be output.
*
*   RETURNS
*	length - the number of pixels in x this text would occupy, not
*	         including any negative kerning that may take place at
*	         the beginning of the text string, nor taking into
*	         account the effects of any clipping that may take
*	         place.
*
*   NOTES
*	Does not check for pre-char which is supported strictly
*	as a kludge for retrofitting Japanese output in some
*	applications which print single bytes.
*
*	Assumes string does not end in ambiguous character - if so,
*	treats it as 1 single-byte character.
*
*	Negative spacing is not supported for EUC fonts.
*
*	Assumes single-byte characters are mono-spaced, and half the
*	display width of Japanese double-byte characters.
*
*   BUGS
*
*   SEE ALSO
*
**********************************************************************

_EUCTextLength:
	PUSHWORD	D0
	PRINTF	DBG_ENTRY,<'EUC--EUCTextLength(%ld)'>
	POPLONG		1

	
		ext.l	d0
		beq.s	etl_tlEmpty

		movem.l	d2/d7/a2-a5,-(sp)
		move.l	a1,a4
		move.l	a0,a3
		move.w	d0,d7
		move.l	rp_Font(a4),a2
		bsr.s	_EUCtrueLength
		movem.l	(sp)+,d2/d7/a2-a5

etl_tlEmpty:

	PRINTF	DBG_EXIT,<'EUC--%ld=EUCTextLength()'>,D0

		rts

*****i* eucdriver.library/EUCtextLength ******************************
*
*   NAME
*	EUCtextLength -- Calculates length in pixels of EUC strings
*
*   SYNOPSIS
*	EUCtextLength(textfont, string, rp, length, userdata)
*		      A2        A3      A4   D7      A6
*
*   FUNCTION
*	Returns pixel length, MAX size (modified via BOLD or ITALIC)
*	and MIN (often 0, unless Italic)
*
*   INPUTS
*	textfont - Pointer to base textfont structure.
*
*	rp     - a pointer to the RastPort which describes where the
*	         text is to be output
*	string - the address of string to output
*	length - the number of characters in the string.
*	         If zero, there are no characters to be output.
*
*	userdata - Pointer to this fonts userdata for EUC driver
*
*   RETURNS
*	D0 - Pixel width
*	D1 - MAX
*	D2 - MIN
*
*   NOTES
*	Does not check for pre-char which is supported strictly
*	as a kludge for retrofitting Japanese output in some
*	applications which print single bytes.
*
*	Assumes string does not end in ambiguous character - if so,
*	treats it as 1 mono-spaced character/
*
*	Negative spacing is not supported for EUC fonts.
*
*	Assumes Kanji glyphs are mono-spaced, and exactly 2x base
*	font width per glyph.
*
*   BUGS
**********************************************************************
;
;	negative spacing is not supported for EUC fonts
;
* True length, excluding any pre-char byte
_EUCtrueLength:

	IFEQ	FAST_PRECHAR
	;;;	move.l	a4,a1
		bsr.s	EUCGetPreChar
	ENDC
	IFNE	FAST_PRECHAR
		move.b	rp_Dummy(a4),d1
	ENDC

		move.w	d1,-(sp)		; SAVE

	IFEQ	FAST_PRECHAR
		moveq	#00,d1
		bsr.s	EUCSetPreChar		; CLEAR
	ENDC
	IFNE	FAST_PRECHAR
		clr.b	rp_Dummy(a4)
	ENDC


		bsr.s	_EUCtextLength		; GET LENGTH

		swap	d1
		move.w	(sp)+,d1

	IFEQ	FAST_PRECHAR
		move.l	a4,a1
		bsr.s	EUCSetPreChar		; RESTORE
	ENDC
	IFNE	FAST_PRECHAR
		move.b	d1,rp_Dummy(a4)
	ENDC

		swap	d1

		rts

;
; Length including pre-char for use by Text() to obtain template
;
_EUCtextLength:

	PRINTF	DBG_ENTRY,<'ECT--ECLtextLength()'>

		movem.l	d3-d6,-(sp)

		moveq	#00,d0			; LENGTH
		moveq	#00,d1			; MAX
		moveq	#00,d2			; MIN

		moveq	#00,d3			; X width
		moveq	#00,d4			; Tx_Spacing per character

	IFEQ	FAST_PRECHAR
		move.l	a4,a1
	ENDC
		move.w	d7,d5

		move.w	tf_XSize(a2),d3
		move.w	rp_TxSpacing(a4),d4
		BMI_S	ecl_gotlength		; illegal for EUC fonts
		bne.s	ecl_slowsize

	; no TxSpacing - simple multiplication (plus maybe 1 byte for pre-char)

	IFEQ	FAST_PRECHAR
		bsr	EUCGetPreChar
	ENDC
	IFNE	FAST_PRECHAR
		move.b	rp_Dummy(a4),d1
	ENDC

		cmp.b	#EUC_LOCHAR,d1
		bcs.s	ecl_fastsize

		addq.w	#1,d5

ecl_fastsize:
		mulu	d5,d3
		move.l	d3,d0
		BRA_S	ecl_haveXlength

	; must scan string because must add TxSpacing per glyph
ecl_slowsize:
		add.l	d3,d4
		
	; scan EUC string - adjusting by TxSpacing per character

		move.l	a3,a0

	; check for pre-char, and treat as first byte of string if
	; present
	IFEQ	FAST_PRECHAR
		bsr	EUCGetPreChar
	ENDC
	IFNE	FAST_PRECHAR
		move.b	rp_Dummy(a4),d1
	ENDC

		cmp.b	#EUC_LOCHAR,d1
		bcs.s	ecl_scanstring		;BLT unsigned - normal processing
						;if pre-char not EUC
		add.l	d4,d0

		bra.s	ecl_DummyByte		;but not necessarily Kanji?

ecl_scanstring:
		subq.w	#1,d5
		bmi.s	ecl_haveXlength
ecl_nextchar:
	; each char is at least XSize + TxSpacing wide

		add.l	d4,d0

	PRINTF	(DBG_FLOW+1),<'[A]'>

ecl_nextscan:

	; assume EUC hichar is $FF, so no need to check upper bounds

		move.b	(a0)+,d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	ecl_scanstring

	; may be kanji, so check next byte

ecl_DummyByte:
		tst.w	d5
		beq.s	ecl_haveXlength

	PRINTF	(DBG_FLOW+1),<'[?]'>

	; assume EUC hichar is $FF, so no need to check upper bounds

		move.b	(a0),d1
		cmp.b	#EUC_LOCHAR,d1
		bcs.s	ecl_scanstring

	PRINTF	(DBG_FLOW+1),<'[E]'>

	;
	; must be double byte code, so width is basefont XSize * 2 PLUS
	; TxSpacing (already added above)
	;
	; if not double byte code, width is just XSize PLUS TxSpacing
	;

		add.l	d3,d0

		addq.w	#1,a0

		subq.w	#1,d5
		bne.s	ecl_scanstring

ecl_haveXlength:

	PRINTF	DBG_FLOW,<' length = %ld'>,D0

	; now must adjust if Bold or Italic

		move.l	d0,d1			;clone length
		btst	#FSB_BOLD,rp_AlgoStyle(a4)
		beq.s	ecl_checkitalic
		add.w	tf_BoldSmear(a2),d1	;add to max

ecl_checkitalic:
		btst	#FSB_ITALIC,rp_AlgoStyle(a4)
		beq.s	ecl_gotlength

	;
	; ----------------- top
	;
	;                               YSize = 8
	;                               BaseLine = 6
	;
	; ----------------- baseline
	;
	; ----------------- bottom  
	;
	; D5 = BaseLine (6)
	; D6 = YSize    (8)
	;
	; D6 = YSize - BaseLine + 1 (2+1 = 3 inclusive lines)
	; D5 = BaseLine / 2 = 3 lines above baseline
	; D6 = 3 / 2 = 1 below baseline
	; so MAX = MAX + overhang
	;    MIN = - (underhang)
	;
	;
		move.w	tf_Baseline(a2),d5
		move.w	tf_YSize(a2),d6

		sub.w	d5,d6
		addq.w	#1,d6			;YSize - baseline + 1
		lsr.w	#1,d5			;BaseLine/2
		lsr.w	#1,d6			;(YSize - baseline + 1)/2
		add.w	d5,d1			;max = len + overhang
		sub.w	d6,d2			;underhang

ecl_gotlength:

		movem.l	(sp)+,d3-d6

	PRINTF	DBG_EXIT,<'ECT--%ld %ld %ld = ECLtextLength()'>,D0,D1,D2

		rts

