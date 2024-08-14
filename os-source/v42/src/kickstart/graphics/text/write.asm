**
**	$Id: write.asm,v 42.0 93/06/16 11:25:12 chrisg Exp $
**
**      text write primitive
**
**      (C) Copyright 1985,1987,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		graphics

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/alerts.i"
	include		"exec/execbase.i"

	INCLUDE		"text.i"

	INCLUDE		"graphics/gfx.i"
	INCLUDE		"graphics/clip.i"
	INCLUDE		"graphics/rastport.i"
	INCLUDE		"graphics/gfxbase.i"

	INCLUDE		"hardware/blit.i"

	INCLUDE		"txtdata.i"
	INCLUDE		"macros.i"
	include	"/macros.i"

	IFNE		rp_Layer
	FAIL		"recode rp_Layer instructions"
	ENDC

*------ Imported Names -----------------------------------------------

	XREF		FreeMustMem
	XREF		GetMustMem
	XREF		textLength
	XREF		waitblitdone

	XREF		_lwmaskTable

	XLVO	AllocMem		; Exec
	XLVO	FreeMem			;

	XLVO	BltBitMap		; Graphics
	XLVO	BltBitMapRastPort	;
	XLVO	BltMaskBitMapRastPort	;
	XLVO	BltClear		;
	XLVO	BltTemplate		;
	XLVO	LockLayerRom		;
	XLVO	UnlockLayerRom		;


*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF		_Text


******* graphics.library/Text ****************************************
*
*   NAME
*	Text -- Write text characters (no formatting).
*
*   SYNOPSIS
*	Text(rp, string, length)
*	     A1  A0      D0-0:16
*
*	void Text(struct RastPort *, STRPTR, WORD);
*
*   FUNCTION
*	This graphics function writes printable text characters to the
*	specified RastPort at the current position.  No control meaning
*	is applied to any of the characters, thus only text on the
*	current line is output.
*
*	The current position in the RastPort is updated to the next
*	character position.
*	If the characters displayed run past the RastPort boundary,
*	the current position is truncated to the boundary, and
*	thus does not equal the old position plus the text length.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text is to be output
*	string - the address of string to output
*	length - the number of characters in the string.
*	         If zero, there are no characters to be output.
*
*   NOTES
*	o   This function may use the blitter.
*	o   Changing the text direction with RastPort->TxSpacing is
*	    not supported.
*
*   BUGS
*	For V34 and earlier:
*	o   The maximum string length (in pixels) is limited to
*	    (1024 - 16 = 1008) pixels wide.
*	o   A text string whose last character(s) have a
*	    tf_CharLoc size component that extends to the right of
*	    the rightmost of the initial and final CP positions
*	    will be (inappropriately) clipped.
*
*   SEE ALSO
*	Move()  TextLength()  graphics/text.h  graphics/rastport.h
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	Text is drawn by taking the byte data in a string and using
*	it as an index into font data.	The byte data may range from
*	0-255.
*
*	A font will have the following characteristics:
*	  - fast and efficient access to the data to draw a character
*	  - support for proportional spacing and kerning
*	  - compact data representation of characters
*	  - easy to manipulate to create new "constructed" fonts
*	  - able to represent characters that are 1K by 1K
*	  - special "fast" support structure for byte-wide fonts
*
*	Text that is drawn will have the standard graphics attributes,
*	i.e. color, drawing mode, write mask, bounds and clipping, as
*	well as the text specific attributes, i.e. reverse video,
*	underline, font, and optionally blinking.  Other attributes
*	often associated with text, such as bold and italic, will be
*	implemented via alternate fonts, which may be either designed
*	as a specific family, or constructed algorithmically from a
*	base font.
*
*	Text can be drawn in any drawing mode, but one color mode is
*	suggested:  Text fonts can have non-spacing characters or
*	characters that kern, which, when the text is output a
*	character at a time in either two color or xor mode, can cause
*	incorrect results at the overlap.  This does not happen,
*	however, when such characters are all output in the same
*	string: the characters are then formed in a temporary buffer
*	using one color mode, then the entire string is written to
*	the RastPort, so overlap among those characters is as if the
*	string were written in one color mode, regardless.
*
*	The temporary buffer is equal in size to (the height of the
*	font + 1) * (width of the RastPort + (height/2) + boldSmear)
*	so the characters can grow either to the left or right of the
*	CP and have algorithmic enhancements performed en masse.
*
*---------------------------------------------------------------------
 STRUCTURE  TW,0
	WORD	TW_LENGTH
	APTR	TW_STRING
	WORD	TW_CP
	WORD	TW_MAX
	WORD	TW_MIN
	WORD	TW_FONTDEPTH		; only used for colorFont
	WORD	TW_OFFSETX0		;
	WORD	TW_OFFSETBM		;
	WORD	TW_OFFSETCHAR		;
	LONG	TW_TEMPLATESIZE		; for each plane
	STRUCT	TW_BITMAP,bm_SIZEOF
	STRUCT	TW_EXTRABITMAP,bm_SIZEOF
	LABEL	TW_SIZE

;
;   d7	length parameter
;
;   a2	TextFont of RastPort
;   a3	string parameter
;   a4	RastPort parameter
;   a5	local variables (TW structure)
;   a6	GfxBase
;   a7	stack pointer
;
_Text:
		movem.l d2-d7/a2-a5,-(a7)

	    ;-- save the text parameters
		tst.w	d0			; check length
		beq	endText			; if no data, return

	    ;-- check if need to lock layer
		move.l	(a1),d1			; rp_Layer
		beq.s	getLength
		move.l	d1,a5
		CALLLVO LockLayerRom		; preserves d0/d1/a0/a1

getLength:
	    ;-- get length of this string
		movem.l	d0/a0/a1,-(a7)
		bsr	textLength		; may destroy d0-d7/a0-a5
						; length in d0
						; max in d1
						; min in d2

		movem.l	(a7)+,d7/a3/a4		; from d0/a0/a1

	    ;-- reserve space for local variables
		suba.w	#TW_SIZE,a7
		move.l	a7,a5

		move.b	#1,TW_BITMAP+bm_BytesPerRow(a5)

	    ;-- initialize variables
		movem.w	d0/d1/d2,TW_CP(a5)	; MAX, and MIN


		move.l	rp_Font(a4),a2		; get the current font

		;-- preclip
		move.l	(a4),d3			; rp_Layer
		beq.s	noPreclip

		move.l	d3,a0

		;--	if super bit map exists, then assume no preclip
		tst.l	lr_SuperClipRect(a0)
		bne.s	noPreclip

		;--	calculate text box in d4,d5 d6,a1
		move.w	rp_cp_x(a4),d4
		add.w	lr_MinX(a0),d4
		sub.w	lr_Scroll_X(a0),d4
		move.w	rp_cp_y(a4),d5
		add.w	lr_MinY(a0),d5
		sub.w	lr_Scroll_Y(a0),d5
		move.w	d4,d6
		add.w	d2,d4
		add.w	d1,d6
		sub.w	tf_Baseline(a2),d5
		move.w	d5,a1
		add.w	tf_YSize(a2),a1
		lea	lr_ClipRect(a0),a0

		;--	cycle for all ClipRects
preclipClipRect:
		move.l	(a0),d3			; cr_Next
		beq	freeLocal
		move.l	d3,a0

		cmp.w	cr_MaxX(a0),d4
		bgt.s	preclipClipRect
		cmp.w	cr_MinX(a0),d6
		blt.s	preclipClipRect

		cmp.w	cr_MaxY(a0),d5
		bgt.s	preclipClipRect
		cmp.w	cr_MinY(a0),a1
		blt.s	preclipClipRect
		tst.l	cr_lobs(a0)
		beq.s	noPreclip
		tst.l	cr_BitMap(a0)
		beq.s	preclipClipRect

noPreclip:

;
;  d2	MIN
;  d3	TW_BITMAP+bm_BytesPerRow
;  d5.b	byteCase if non-zero
;  d6	YSize
;  a1	TEMPLATE
;
		move.w	tf_YSize(a2),d6		; cache YSize
		beq	freeLocal		; no height => no rendering

	;-- get template

		;-- bm_BytesPerRow is based on rendering length
		move.w	d1,d3			; MAX
		sub.w	d2,d3			;     - MIN
		beq	freeLocal		; no width => no rendering
		moveq	#0,d5

		;-- check quickly if this font has been compiled yet
		move.l	tf_Extension(a2),d1
		beq		notByteModulo
		move.l	d1,a0
		cmp.w   #TE_MATCHWORD,(a0)      ; tfe_MatchWord(a0)
		bne		notByteModulo
		cmp.l   tfe_BackPtr(a0),a2
		bne.s	notByteModulo

		;-- check for byte font special case
		btst	#TE0B_BYTECELL,tfe_Flags0(a0)
		beq.s	notByteModulo
		tst.w	rp_TxSpacing(a4)
		bne.s	notByteModulo
		move.b	rp_AlgoStyle(a4),d5
		andi.b	#FSF_ITALIC+FSF_BOLD+FSF_UNDERLINED,d5
		bne.s	notByteModulo

	    ;-- byte font special case
		addq.w	#8,d3			; round up to even bytes
		bsr.s	getTemplate
		tst.l	d1
		beq.s	LowMemText
		move.l	a1,TW_EXTRABITMAP+bm_Planes(a5)
		bsr	waitblitdone
		bra	bInitialization

* LowMemText - GetMustMem() no longer GURUs, but returns NULL in no-mem
* situations. As an alternative to the GURU, Text() should now try to
* write each character one at a time. This may produce a differently
* rendered text ('cos of kerning), but WTF, we've no memory!!
*
* Call Text() (recursively) one character at a time. The only way
* Text() can now fail is if a single character is larger than
* MAXBYTESPERROW (= 4096 bytes). That's a pretty big font to fail,
* eg 256 * 128 pixels
*
* - spence Jan 16 1991

LowMemText:
*	d7:16 = length
*	a4	-> rp
*	a3	-> string 
		subq.w	#1,d7
		beq		_Text.			; one character is too big! exit quietly.
LowMemTextloop:
		subq.l	#2,sp			; keep it aligned for normal stack usage
		move.b	(a3)+,(sp)		; use stack as string pointer, for 1 char at a time
		move.l	sp,a0
		move.l	a4,a1			; rastport
		moveq	#1,d0			; string length
		bsr		_Text			; recurse
		addq.l	#2,sp
LowMemTextdbra:
		dbra	d7,LowMemTextloop
		bra		_Text.
	
	    ;-- shared code to get template
getTemplate:
		lsr.w	#4,d3			; converted to words
		add.w	d3,d3			; converted to bytes
		move.w	d3,TW_BITMAP+bm_BytesPerRow(a5)

		;-- get template size
		move.w	d6,d0			; get height
		mulu	d3,d0			; get the total needed size
		move.l	d0,TW_TEMPLATESIZE(a5)

		;-- get the template
		move.l	rp_TmpRas(a4),d1
		beq.s	needTemplate
		move.l	d1,a0
		move.l	tr_RasPtr(a0),d1
		beq.s	needTemplate
		cmp.l	tr_Size(a0),d0
		ble.s	cacheTAddr

		;-- no template or not big enough
needTemplate:
		moveq	#MEMF_CHIP,d1
		bsr	GetMustMem		; returns OK or alerts
		move.l	d0,d1

cacheTAddr:
		move.l	d1,a1
		move.l	a1,TW_BITMAP+bm_Planes(a5)
		rts


	    ;-- non-byte case, or byte case w/ spacing or algorithmic
notByteModulo:
		add.w	#79,d3			; round up to word + 4 word pad
		add.w	tf_Baseline(a2),d3
		btst.b	#FSB_COLORFONT,tf_Style(a2)
		bne	colorFont
notColorFont:
		bsr.s	getTemplate
		tst.l	d1
		beq.s	LowMemText

		;-- get TW_EXTRABITMAP+bm_Planes address based on MIN
		sub.w	tf_Baseline(a2),d2
		move.w	d2,d0			; MIN
		asr.w	#4,d0			; get offset word
		add.w	d0,d0			;   in bytes
		neg.w	d0			;
		lea	4(a1,d0.w),a0		; pad with 2 words
		move.l	a0,TW_EXTRABITMAP+bm_Planes(a5)	; thus X=0 is here

		tst.b	d5
		beq	notByteCode

	;-- special case byte fonts
;   d1	tf_LoChar
;   d2	tf_YSize-1 (dbf copy count)
;   d3	TW_BITMAP+bm_BytesPerRow
;   d4	font modulo
;   d5	character index
;   d6	tf_HiChar
;   d7	chars to output
;
;   a2	tf_CharData
;   a4	tf_CharLoc
;   a5	CP (byte address in template)
;   a6  initial loop entry
;

bClearTemplate:
	    ;-- clear the template plane for use
		move.w	d6,d0			; get height
		swap	d0			;   (in high word)
		move.w	d3,d0			; and width
		moveq	#3,d1			; synchronous clear,
						; row/word count
		CALLLVO BltClear

bnoclear:
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a1	; CP (byte address)

bInitialization:
		movem.l	a2/a4/a5/a6,-(a7)
		;-- calculate bLoop entry offset
		move.w	d6,d2
		subq.w	#1,d2
		move.w	d2,d0
		and.w	#7,d0			; note high byte is cleared
		move.b	bLoopOffsets(pc,d0.w),d0
		lea	bHunkStart(pc,d0.w),a6
		;-- get DBF counter
		lsr.w	#3,d2
		;-- initialize other registers
		subq.w	#1,d7			; adjust length for dbf
		move.b	tf_HiChar(a2),d6
		move.w	tf_Modulo(a2),d4
		move.b	tf_LoChar(a2),d1
		move.l	tf_CharLoc(a2),a4
		move.l	tf_CharData(a2),a2
		move.l	a1,a5			; CP (byte address)

bCharLoop:
		moveq	#0,d5
		move.b	(a3)+,d5		; get next character in string
		cmp.b	d6,d5			; check character range
		bhi.s	bUnKnownRC
		sub.b	d1,d5
		bcc.s	bKnownRC

	    ;-- the character code is not in the font, use last char + 1
bUnKnownRC:
		move.b	d6,d5  			 ;get last character
		sub.b	d1,d5
		addq.w	#1,d5

bKnownRC:
	    ;-- find character data start and size
		add.w	d5,d5
		add.w	d5,d5
		move.w	0(a4,d5.w),d0
		lsr.w	#3,d0			; get start byte offset
		lea	0(a2,d0.w),a1		; character data start byte

		move.l	a5,a0			; template starting address
		move.w	d2,d0			; line dbf count
		jmp	(a6)

bLoopOffsets:
		dc.b	42,36,30,24,18,12,6,0
		ds.w	0


bLoop:
		adda.w	d4,a1
		adda.w	d3,a0
bHunkStart:
		move.b	(a1),(a0)		; remainder 0
		adda.w	d4,a1			; bump char data by it's modulo
		adda.w	d3,a0			; bump template by it's modulo
bHunkMark:
		move.b	(a1),(a0)		; remainder 7
		adda.w	d4,a1
		adda.w	d3,a0
		move.b	(a1),(a0)		; remainder 6
		adda.w	d4,a1
		adda.w	d3,a0
		move.b	(a1),(a0)		; remainder 5
		adda.w	d4,a1
		adda.w	d3,a0
		move.b	(a1),(a0)		; remainder 4
		adda.w	d4,a1
		adda.w	d3,a0
		move.b	(a1),(a0)		; remainder 3
		adda.w	d4,a1
		adda.w	d3,a0
		move.b	(a1),(a0)		; remainder 2
		adda.w	d4,a1
		adda.w	d3,a0
		move.b	(a1),(a0)		; remainder 1
		dbf	d0,bLoop

	    ;-- get next character
		addq.w	#1,a5			; update CP byte address
		dbf	d7,bCharLoop

		movem.l	(a7)+,a2/a4/a5/a6
		bra	chkBold

	IFNE	bHunkMark-bHunkStart-6
	FAIL	"bHunk not 6 bytes long, recode bLoopOffsets"
	ENDC

;
;   d5	character index
;

notByteCode:

; now, call special 020 code
;		move.l	gb_ExecBase(a6),a0
;		btst	#AFB_68020,AttnFlags+1(a0)	; 68020 or above for bitfield instructions?
;		bne	special_020_text

	    ;-- clear the template plane for use
non_68020:
		move.w	d6,d0			; get height
		swap	d0			;   (in high word)
		move.w	d3,d0			; and width
		moveq	#3,d1			; synchronous clear,
						; row/word count
		CALLLVO BltClear

;-- save registers used for other things here
		move.w	d7,TW_LENGTH(a5)
		move.l	a3,TW_STRING(a5)
		clr.w	TW_CP(a5)

nbCharLoop:
		moveq	#0,d5
		move.l	TW_STRING(a5),a0
		move.b	(a0)+,d5    ;get next character in string
		move.l	a0,TW_STRING(a5)
		cmp.b	tf_HiChar(a2),d5   ;check character range
		bhi.s	nbUnKnownRC
		sub.b	tf_LoChar(a2),d5
		bcc.s	nbKnownRC

	    ;-- the character code is not in the font, use last char + 1
nbUnKnownRC:
		move.b	tf_HiChar(a2),d5   ;get last character
		sub.b	tf_LoChar(a2),d5
		addq.w	#1,d5

	    ;-- get start position of character
nbKnownRC:
		add.w	d5,d5
		move.w	TW_CP(a5),d3
		move.l	tf_CharKern(a2),d0
		beq.s	nbGotNear
		move.l	d0,a0
		add.w	0(a0,d5.w),d3
nbGotNear:
		move.w	d3,d7			; save CP of char start
		move.l	tf_CharSpace(a2),d0
		bne.s	nbGotFar
		add.w	tf_XSize(a2),d3
		bra.s	nbSpacing
nbGotFar:
		move.l	d0,a0
		add.w	0(a0,d5.w),d3
nbSpacing:
		add.w	rp_TxSpacing(a4),d3

	    ;-- save CP
		move.w	d3,TW_CP(a5)

	    ;-- find character data start and size
		add.w	d5,d5
		move.l	tf_CharLoc(a2),a0
		move.l	0(a0,d5.w),d0
		move.w	d0,d5	    ;character data bit length
		beq	checkDoneRendering
		swap	d0
		move.w	d0,d3
		and.w	#$0F,d3			; character data start bit
		lsr.w	#4,d0			; (always positive)
		add.w	d0,d0			;   even word address
		move.l	tf_CharData(a2),a1
		add.w	d0,a1			; character data start word
		move.w	tf_Modulo(a2),a3

	    ;-- get number of lines to copy
		move.w	tf_YSize(a2),d6

	    ;--	calculate the 32 bit mask
		move.w	d5,d0
		subq.w	#1,d0			; make 16 fast?
		and.w	#15,d0			; get number of bits to save
		add.w	d0,d0			; word index
		lea	_lwmaskTable,a0
		move.w	0(a0,d0.w),d4		; 1 = first entry
		swap	d4
		clr.w	d4
		lsr.l	d3,d4			; shift into position

	    ;-- calculate start line position
		move.w	d7,d0
		andi.w	#$0F,d7	    ;get line buffer starting bit position
		asr.w	#4,d0	    ;get line buffer starting byte offset
		add.w	d0,d0	    ;  then word offset
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a0  ;get word address
		add.w	d0,a0	    ;line buffer starting word address
		move.w	TW_BITMAP+bm_BytesPerRow(a5),a2

	    ;-- check for characters no wider than 16 bits
		cmp.w	#16,d5
		bls.s	thinCopy

	    ;-- calculate additional variables for wide characters
		move.l	#$ffff0000,d2		; intermediate mask
		lsr.l	d3,d2
		sub.w	#17,d5			; fix up width dbf counter
		lsr.w	#4,d5
		move.w	d5,d0			; fix up modulos
		add.w	d0,d0
		addq.w	#2,d0
		sub.w	d0,a2
		sub.w	d0,a3
;
;  d0	 **
;  d1	  *
;  d2	>  mask associated with char data (longword)
;  d3	  *start bit associated with char data (0-15)
;  d4	   mask associated with last word of char data (longword)
;  d5	>  dbf counter for intermediate char words
;  d6	  *counter for height (tf_YSize)
;  d7	   start bit associated with CP (0-15)
;  a0	 **template initial longword address
;  a1	 **character data initial longword address
;  a2	   template adjusted modulo
;  a3	   font adjusted modulo
;	>  =only in wideCopy
;	 **=destroyed
;
wideCopy:
		sub.w	d7,d3
		bge.s	wideLeft
		neg.w	d3
		bra.s	wideRight
wideRightLoop:
		move.l	(a1),d0
		and.l	d2,d0
		lsr.l	d3,d0
		or.l	d0,(a0)
		addq.l	#2,a1
		addq.l	#2,a0
		dbf	d1,wideRightLoop

		move.l	(a1),d0
		and.l	d4,d0
		lsr.l	d3,d0
		or.l	d0,(a0)

		adda.w	a3,a1		; bump char data by it's modulo
		adda.w	a2,a0		; bump template by it's modulo
wideRight:
		move.w	d5,d1		; initialize x counter
		dbf	d6,wideRightLoop
		bra.s	checkDoneRendering

wideLeftLoop:
		move.l	(a1),d0
		and.l	d2,d0
		lsl.l	d3,d0
		or.l	d0,(a0)
		addq.l	#2,a1
		addq.l	#2,a0
		dbf	d1,wideLeftLoop

		move.l	(a1),d0
		and.l	d4,d0
		lsl.l	d3,d0
		or.l	d0,(a0)

		adda.w	a3,a1		; bump char data by it's modulo
		adda.w	a2,a0		; bump template by it's modulo
wideLeft:
		move.w	d5,d1		; initialize x counter
		dbf	d6,wideLeftLoop
		bra.s	checkDoneRendering


thinCopy:
		sub.w	d7,d3
		bge.s	thinLeft
		neg.w	d3
		bra.s	thinRight
thinRightLoop:
		move.l	(a1),d0
		and.l	d4,d0
		lsr.l	d3,d0
		or.l	d0,(a0)

		adda.w	a3,a1		; bump char data by it's modulo
		adda.w	a2,a0		; bump template by it's modulo
thinRight:
		dbf	d6,thinRightLoop
		bra.s	checkDoneRendering

thinLeftLoop:
		move.l	(a1),d0
		and.l	d4,d0
		lsl.l	d3,d0
		or.l	d0,(a0)

		adda.w	a3,a1		; bump char data by it's modulo
		adda.w	a2,a0		; bump template by it's modulo
thinLeft:
		dbf	d6,thinLeftLoop

checkDoneRendering:
		move.l	rp_Font(a4),a2
		subq.w	#1,TW_LENGTH(a5)
		bne	nbCharLoop
;		bra.s	chkBold


    ;-- character loop is done

chkBold:
	;-- check bold enhancement
		btst	#FSB_BOLD,rp_AlgoStyle(a4) ;bold text?
		beq.s	chkItalic
	;-- embolden the data in the buffer
		move.w	tf_BoldSmear(a2),d1
		cmp	#16,d1
		bgt.s	must_make_really_bold
		move.l	TW_BITMAP+bm_Planes(a5),a0
		move.l	TW_TEMPLATESIZE(a5),d0
		lsr.l	#1,d0		; fix size for word count,
		subq.l	#2,d0		;   DBF, & not last word
		moveq	#0,d3
		move.w	(a0)+,d3

emboldenLoop:
		swap	d3
		move.l	d3,d2
		lsr.l	d1,d2
		moveq	#0,d3
		move.w	(a0)+,d3
		or.l	d2,-4(a0)
		dbf	d0,emboldenLoop
		bra.s	chkItalic
must_make_really_bold:
		movem.l	a2/d2-d7,-(a7)
		lea	TW_BITMAP(a5),a0	; srcbm
		move.l	a0,a1			; destbm
		moveq	#0,d0			; srcx
		move.w	d1,d2			; destx
		moveq	#0,d1			; srcy
		moveq	#0,d3			; desty
		move.w	bm_BytesPerRow(a0),d4
		lsl.w	#3,d4
		sub.w	d2,d4			; width
		move.w	tf_YSize(a2),d5		; sizey
		move.w	#$e5,d6			; minterm
		moveq	#1,d7
		sub.l	a2,a2
		jsr	_LVOBltBitMap(a6)
		movem.l	(a7)+,a2/d2-d7
chkItalic:
	;-- check italic enhancement
		btst	#FSB_ITALIC,rp_AlgoStyle(a4) ;italic text?
		beq.s	chkUnderline

	;-- italicize the data in the buffer
	    ;-- initialize variables to point to baseline
		move.l	TW_BITMAP+bm_Planes(a5),a0
		move.w	tf_Baseline(a2),d0
		move.w	d0,d6		; save baseline for later
		addq.w	#1,d0		; at the end of the line
		mulu	TW_BITMAP+bm_BytesPerRow(a5),d0
		add.l	d0,a0
		move.l	a0,a1		; generate start for right shift
		addq.w	#1,d6		; BaseLine + 1
		move.w	tf_YSize(a2),d5 ; generate number of left shift
		sub.w	d6,d5		;   lines
		blt.s	chkUnderline	; bad BaseLine!

		moveq	#1,d1		; starts shifted below baseline
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d7
		lsr.w	#1,d7
		subq.w	#1,d7
		move.w	d7,-(sp)
		bra.s	startLShift
shiftLeft:
		move.w	(sp),d7		; recover the count
		addq.w	#1,d1		; increment count away from baseline
		;-- first, DBF, and last word not counted
		move.w	d1,d2		; move count into halvsies register
		lsr.w	#1,d2		; get half of baseline distance
		move.w	d2,d3		; clone distance
		andi.w	#$0F,d2		;   extract bit part
		lsr.w	#4,d3		;   extract word offset
		add.w	d3,d3		;     (which is really a byte offset)
shiftLLine:
		move.l	0(a0,d3.w),d0	; get longword to shift
		lsl.l	d2,d0		; shift longword
		swap	d0		; get the complete word portion
		move.w	d0,(a0)+	;   update word in shifted position
		dbf	d7,shiftLLine	;

startLShift:
		dbf	d5,shiftLeft	


		moveq	#-1,d1		; starts at baseline
		bra.s	startRShift
shiftRight:
		move.w	(sp),d7
		addq.w	#1,d1		; increment count away from baseline
		;-- first, DBF, and last word not counted
		;-- get shift and offset
		move.w	d1,d2		; move count into halvsies register
		lsr.w	#1,d2		; get half of baseline distance
		move.w	d2,d3		; clone distance
		andi.w	#$0F,d2		;   extract bit part
		lsr.w	#4,d3		;   extract word offset
		add.w	d3,d3		;     (which is really a byte offset)
		neg.w	d3		;     to the right is lower addresses
shiftRLine:
		move.l	-4(a1,d3),d0	; get longword to shift
		lsr.l	d2,d0		; shift longword
		move.w	d0,-(a1)	;   update word in shifted position
		dbf	d7,shiftRLine	;

startRShift:
		dbf	d6,shiftRight	
		addq.l	#2,sp

chkUnderline:
	;-- check underline enhancement
		btst	#FSB_UNDERLINED,rp_AlgoStyle(a4) ;underlined text?
		beq.s	templateOut
	;-- underline the data in the buffer
	    ;-- initialize variables to point to baseline
		move.l	TW_BITMAP+bm_Planes(a5),a0
		;------ underline under the baseline
		move.w	tf_Baseline(a2),d0
		addq.w	#1,d0		;the template starts one line down
		cmp.w	tf_YSize(a2),d0
		bge.s	templateOut

		mulu	TW_BITMAP+bm_BytesPerRow(a5),d0
		add.l	d0,a0
		move.l	a0,a1
		bsr.s	underlineA0

	;-- output the buffer data to the raster
;	BltTemplate(source, srcX, srcMod, destRastPort,
;		    a0	    d0	  d1	  a1
;		    destX,  destY, sizeX, sizeY),  graphicsLib
;		    d2	    d3	   d4	  d5	   a6
;
templateOut:

		move.l	TW_EXTRABITMAP+bm_Planes(a5),a0	; srcTemplate

		move.l	a4,a1			; destRastPort

		move.w	TW_MIN(a5),d0		; srcX

		move.w	TW_BITMAP+bm_BytesPerRow(a5),d1	; srcMod

		move.w	rp_cp_x(a4),d2		; get destX
		add.w	d0,d2			; adjusted for srcX

		move.w	rp_cp_y(a4),d3		; get destY
		sub.w	tf_Baseline(a2),d3	;   adjusted for even baselines

		move.w	TW_MAX(a5),d4		; get sizeX
		sub.w	d0,d4

		move.w	tf_YSize(a2),d5		; get sizeY

		CALLLVO BltTemplate

	;-- free the template if required
		move.l	TW_TEMPLATESIZE(a5),d0
	    ;-- check if this is the RastPort TmpRas
		move.l	rp_TmpRas(a4),d1
		beq.s	freeTemplate
		move.l	d1,a0
		move.l	tr_RasPtr(a0),d1
		beq.s	freeTemplate
		cmp.l	tr_Size(a0),d0
		ble.s	freeLocal

	    ;-- free the template
freeTemplate:
		bsr	waitblitdone
		move.l	TW_BITMAP+bm_Planes(a5),a1
		bsr	FreeMustMem

	    ;-- free the local variables
freeLocal:
		move.w	TW_CP(a5),d0		; update current position
		add.w	d0,rp_cp_x(a4)		;

_Text.:
		adda.w	#TW_SIZE,a7		; dump local variables

	    ;-- unlock the layer if required
		move.l	(a4),d0			; rp_Layer
		beq.s	endText
		move.l	d0,a5
		CALLLVO UnlockLayerRom

	    ;-- restore the caller's registers
endText:
		movem.l (a7)+,d2-d7/a2-a5
		rts



;---------------------------------------------------------------------
underlineA0:
	    ;-- get length, sans first and DBF word
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d4
		lsr.w	#1,d4
		subq.w	#1,d4
	    ;-- first word (don't shift bit in from left)
		move.l	(a0),d0
		move.l	d0,d1
		swap	d1
		lsr.w	#1,d1
		bra.s	uFirstContd
	    ;-- middle words
uMidLoop:
		or.w	d3,(a1)+
		move.l	d0,d1
		addq.l	#2,a0
		move.l	(a0),d0
		lsr.l	#1,d1
uFirstContd:
		move.l	d0,d2
		add.l	d2,d2
		swap	d2
		move.l	d0,d3
		swap	d3
		or.w	d2,d3
		or.w	d1,d3
		not.w	d3
		dbf	d4,uMidLoop
	    ;-- last word (ensure bit wasn't masked from right)
		btst	#16,d0		; is it not set from char in last word?
		bne.s	uLastWord
		bset	#0,d3		; ensure right bit set
uLastWord:
		or.w	d3,(a1)+
		rts


	ifne	0
do_line	macro
	bfextu	(a2){d4:d3},d6
	add.w	d0,a2
	bfins	d6,(a6){d1:d5}
	add.w	d2,a6
	endm


	OPT	P=68020

jptbl:	dc.l	high1
	dc.l	high2
	dc.l	high3
	dc.l	high4
	dc.l	high5
	dc.l	high6
	dc.l	high7
	dc.l	high8
	dc.l	high9
	dc.l	high10
	dc.l	high11
	dc.l	high12
	dc.l	high13
	dc.l	high14
	dc.l	high15

special_020_text:
; special routines which use the 020's bitfiled instructions to render text.
; entr d7=len a3=source chars d6=height a2=font TW_EXTRA_BITMAP=allocated
; a1=tmprast
		tst.l	tf_CharSpace(a2)	; no proportional for now
		bne	non_68020
		tst.l	tf_CharKern(a2)
		bne	non_68020
		cmp.w	#15,tf_YSize(a2)
		bgt	non_68020

		tst.b	rp_AlgoStyle(a4)
		beq.s	noclear_020
	    ;-- clear the template plane for use
		move.w	d6,d0			; get height
		swap	d0			;   (in high word)
		move.w	d3,d0			; and width
		moveq	#3,d1			; synchronous clear,
						; row/word count
		CALLLVO BltClear
noclear_020:
		move.l	TW_BITMAP+bm_Planes(a5),a1

		movem.l	a2/a4/a5/a6,-(a7)
		moveq	#0,d1
		move.w	TW_MIN(a5),d1
		move.l	tf_CharLoc(a2),a0
		move.w	tf_Modulo(a2),d0
		swap	d7
		clr.w	d7
		move.b	tf_LoChar(a2),d7
		swap	d7
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d2
		swap	d2
		move.b	tf_HiChar(a2),d2
		move.w	tf_XSize(a2),d5
		move.w	tf_YSize(a2),d4
		subq	#1,d4
		add.w	d4,d4
		add.w	d4,d4
		lea	jptbl(pc),a5
		move.l	0(a5,d4.w),a5
		move.l	tf_CharData(a2),-(a7)
		subq	#1,d7
		moveq	#0,d4
char_loop:	swap	d7
		moveq	#0,d3
		move.b	(a3)+,d3
		cmp.b	d2,d3
		bhi.s	spec_badchar
		sub.w	d7,d3			; subtract lochar
		bpl.s	spec_goodchar
spec_badchar:	moveq	#0,d3
		move.b	d2,d3
		sub.w	d7,d3
spec_goodchar:
		add.w	d3,d3
		add.w	d3,d3
		move.w	0(a0,d3.w),d4		; field start
		move.w	2(a0,d3.w),d3		; field width
		move.l	(a7),a2
		move.l	a1,a6
		swap	d2
		jmp	(a5)
high15:		do_line
high14:		do_line
high13:		do_line
high12:		do_line
high11:		do_line
high10:		do_line
high9:		do_line
high8:		do_line
high7:		do_line
high6:		do_line
high5:		do_line
high4:		do_line
high3:		do_line
high2:		do_line
high1:		do_line
		add.w	d5,d1
		add.w	rp_TxSpacing(a4),d1
		swap	d2
		swap	d7
end_lp1:	dbra	d7,char_loop
		lea	4(a7),a7
		movem.l	(a7)+,a2/a4/a5/a6
		move.w	d1,TW_CP(a5)
		move.l	TW_BITMAP+bm_Planes(a5),a1
		move.l	#$f0f0f0f0,(a1)
		bra	chkBold
	endc


;---------------------------------------------------------------------
;
;	font		planePick		minterm
;		ctf_FgColor	planeOnOff		rastport bitMap
;
;	+-+	+-+	+-+	+-+		+-+	+-+
;	|a|	|=|--->	|1|--->	|X|- font a -o>	| |	| |
;	+-+	+-+	+-+	+-+	     |	+-+	+-+
;	|b|	|=|\	|0|	|0|- zeros --o>	| |	| |
;	+-+	+-+ \	+-+	+-+	     |	+-+	+-+
;		     ~>	|1|--->	|X|- font b -o>	| |	| |
;		+-+	+-+	+-+	     |	+-+	+-+
;		|c|\	|0|	|1|- ones ---o>	| |	| |
;		+-+ \	+-+	+-+	    /	+-+	+-+
;		     \			   /
;		      `-mask & FgPen------'
;
;	ctf_Depth (<=8)	8			8
;		1 (mask plane)	8			rp_BitMap->depth (<=8)
;

;
;   d2	MIN
;   d3	width in pixels
;   d5	(must be zero when returning to notColorFont)
;   d6	YSize
;   d7	chars to output
;   a1	TEMPLATE
;
colorFont:
	    ;-- calculate bitmap and font depths
		;-- get rp_BitMap bm_Depth
		moveq	#0,d4
		move.l	rp_BitMap(a4),a0
		move.b	bm_Depth(a0),d4
		;-- get ctf_Depth
		moveq	#0,d1
		move.b	ctf_Depth(a2),d1
		;-- check if color remapping of FgColor to FgPen is requested
		btst	#CTB_MAPCOLOR&$7,(1-(CTB_MAPCOLOR/8))+ctf_Flags(a2)
		beq.s	cfMinFontDepth
		move.b	ctf_FgColor(a2),d0
		cmp.b	ctf_High(a2),d0
		bhi.s	cfMinFontDepth	; bgt unsigned
		cmp.b	ctf_Low(a2),d0
		bcc.s	cfHaveFontDepth	; bge unsigned
		;--	check if rp_BitMap bm_Depth is smaller
cfMinFontDepth:
		cmp.b	d1,d4
		bge.s	cfHaveFontDepth
		move.b	d4,d1
cfHaveFontDepth:
		move.w	d1,TW_FONTDEPTH(a5)
		;--	check if rp_BitMap bm_Depth is greater
		cmp.b	d1,d4
		bge.s	cfHaveDepth
		move.b	d1,d4
cfHaveDepth:
	    ;-- size template planes
		;-- get template modulo
		move.w	d3,d0
		lsr.w	#4,d0			; converted to words
		add.w	d0,d0			; converted to bytes
		move.w	d0,TW_BITMAP+bm_BytesPerRow(a5)
		move.w	d0,TW_EXTRABITMAP+bm_BytesPerRow(a5)

		;-- get template size
		mulu	d6,d0			; get the total needed size
		move.l	d0,TW_TEMPLATESIZE(a5)

	    ;-- get BitMap
		;-- initialize BitMap header
		move.w	d4,TW_BITMAP+bm_Flags(a5)	; + bm_Depth
		beq	notColorFont
		move.w	d4,TW_EXTRABITMAP+bm_Flags(a5)	; + bm_Depth
		clr.w	TW_BITMAP+bm_Pad(a5)
		clr.w	TW_EXTRABITMAP+bm_Pad(a5)
		move.w	d6,TW_BITMAP+bm_Rows(a5)
		move.w	d6,TW_EXTRABITMAP+bm_Rows(a5)
		moveq	#7,d5
		lea	TW_BITMAP+bm_Planes(a5),a0
cfClearPlanes:
		clr.l	(a0)+
		dbf	d5,cfClearPlanes

		;-- allocate all the planes required
		move.w	d4,d5
		subq	#1,d5
cfAllocPlanes:
		move.l	TW_TEMPLATESIZE(a5),d0
		moveq	#MEMF_CHIP,d1
		LINKEXE	AllocMem
		move.w	d5,d1		; get a slot for this plane
		lsl.w	#2,d1
		move.l	d0,TW_BITMAP+bm_Planes(a5,d1.w)
		beq.s	cfFailPlaneAlloc
		;-- clear the template plane for use
		move.l	d0,a1
		move.l	TW_TEMPLATESIZE(a5),d0
		moveq	#0,d1
		CALLLVO BltClear
		dbf	d5,cfAllocPlanes

		;-- use the TmpRas for the EXTRAPLANE if available
		move.l	TW_TEMPLATESIZE(a5),d0
		move.l	rp_TmpRas(a4),d1
		beq.s	cfNeedExtra
		move.l	d1,a0
		move.l	tr_RasPtr(a0),d1
		beq.s	cfNeedExtra
		cmp.l	tr_Size(a0),d0
		ble.s	cfCacheExtra
cfNeedExtra:
		moveq	#MEMF_CHIP,d1
		LINKEXE	AllocMem
		move.l	d0,d1
cfCacheExtra:
		moveq	#7,d0
		lea	TW_EXTRABITMAP+bm_Planes(a5),a0
cfCachePlanes:
		move.l	d1,(a0)+
		dbeq	d0,cfCachePlanes
		bne.s	cfGotBitmap

		;-- free any allocated planes and go render as normal text
cfFailPlaneAlloc:
		bsr	waitblitdone
		moveq	#7,d5
cfFFreePlanes:
		move.w	d5,d1
		lsl.w	#2,d1
		move.l	TW_BITMAP+bm_Planes(a5,d1.w),d0
		beq.s	cfFNextFreePlane
		move.l	d0,a1
		move.l	TW_TEMPLATESIZE(a5),d0
		LINKEXE	FreeMem
cfFNextFreePlane:
		dbf	d5,cfFFreePlanes
		moveq	#0,d5
		bra	notColorFont

cfGotBitmap:
		;-- get TW_OFFSETX0 based on MIN
		move.w	d2,d0			; MIN
		neg.w	d0			; -MIN
		asr.w	#4,d0			; get offset word
		add.w	d0,d0			;   in bytes
		addq.w	#2,d0
		move.w	d0,TW_OFFSETX0(a5)	; thus X=0 is here

		;-- save registers used for other things here
		move.w	d7,TW_LENGTH(a5)
		move.l	a3,TW_STRING(a5)
		clr.w	TW_CP(a5)

		;-- wait for last plane to clear
		bsr	waitblitdone

	;-- render characters into template
cfCharLoop:
		moveq	#0,d2
		move.l	TW_STRING(a5),a0
		move.b	(a0)+,d2		; get next character in string
		move.l	a0,TW_STRING(a5)
		cmp.b	tf_HiChar(a2),d2	; check character range
		bhi.s	cfUnKnownRC
		sub.b	tf_LoChar(a2),d2
		bcc.s	cfKnownRC

		;-- the character code is not in the font, use last char + 1
cfUnKnownRC:
		move.b	tf_HiChar(a2),d2   ;get last character
		sub.b	tf_LoChar(a2),d2
		addq.w	#1,d2

	    ;-- get start position of character
cfKnownRC:
		add.w	d2,d2
		move.w	TW_CP(a5),d1
		move.l	tf_CharKern(a2),d0
		beq.s	cfGotNear
		move.l	d0,a0
		add.w	0(a0,d2.w),d1
cfGotNear:
		move.w	d1,d3			; save CP of char start
		move.l	tf_CharSpace(a2),d0
		bne.s	cfGotFar
		add.w	tf_XSize(a2),d3
		bra.s	cfSpaced
cfGotFar:
		move.l	d0,a0
		add.w	0(a0,d2.w),d3
cfSpaced:
		add.w	rp_TxSpacing(a4),d3
		move.w	d3,TW_CP(a5)		; save CP

	    ;-- calculate bitmap bit and offset
		move.w	d1,d0
		andi.w	#$0F,d1		; get line buffer starting bit position
		asr.w	#4,d0		; get line buffer starting word offset
		add.w	d0,d0		;   then byte offset
		add.w	TW_OFFSETX0(a5),d0
		move.w	d0,TW_OFFSETBM(a5)

	    ;-- find character data bit, offset, and size
		add.w	d2,d2
		move.l	tf_CharLoc(a2),a0
		move.l	0(a0,d2.w),d0
		move.w	d0,d7			; character data bit length
		beq	cfCheckDone
		swap	d0
		move.w	d0,d6
		and.w	#$0F,d6			; character data start bit

		;-- calculate TW_OFFSETCHAR
		lsr.w	#4,d0			; (always positive)
		add.w	d0,d0			;   even word address
		move.w	d0,TW_OFFSETCHAR(a5)	;

		;-- calculate X counter initial value
		move.w	d7,d0
		sub.w	#1,d7
		lsr.w	#4,d7
		;-- look up last word mask
		subq.w	#1,d0			; make 16 fast?
		and.w	#15,d0			; get number of bits to save
		add.w	d0,d0			; word index
		lea	_lwmaskTable,a0
		move.w	0(a0,d0.w),d5		; 1 = first entry
		swap	d5
		clr.w	d5
		lsr.l	d6,d5			; shift into position

		;-- create intermediate mask
		move.l	#$ffff0000,d4
		lsr.l	d6,d4

		;-- get font bit map depth
		move.w	TW_FONTDEPTH(a5),d3

		;-- initialize plane array pointers
		move.l	a4,-(a7)		; save rastPort
		lea	TW_BITMAP+bm_Planes(a5),a3
		lea	ctf_CharData(a2),a4

		;-- determine copy direction and branch
		sub.w	d1,d6
		bge	cfLeft
		neg.w	d6
		bra.s	cfRight
;
;   d0	temporary for character bits
;   d1	temporary for X counter
;   d2	temporary for Y counter
;   d3  Z counter
;   d4	longword mask associated with intermediate words of char data
;   d5	longword mask associated with last word of char data
;   d6	shift associated with char data (0-15)
;   d7	initial value for x counter
;   a0	current font source
;   a1	current template destination
;   a2	tf_ current textFont
;   a3	template bm_Planes array
;   a4	font ctf_Planes array
;   a5	TW variables
;   a6	gfxBase (unused, preserved)
;   a7	stack
;
cfRightZLoop:
		move.l	(a4)+,a0
		add.w	TW_OFFSETCHAR(a5),a0
		move.l	(a3)+,a1
		add.w	TW_OFFSETBM(a5),a1
cfRightYLoop:
		move.w	d7,d1		; initialize x counter
		movem.l	a0/a1,-(a7)
		bra.s	cfRightXDBF
cfRightXLoop:
		move.l	(a0),d0
		and.l	d4,d0
		lsr.l	d6,d0
		or.l	d0,(a1)
		addq.l	#2,a1
		addq.l	#2,a0
cfRightXDBF:
		dbf	d1,cfRightXLoop

		move.l	(a0),d0
		and.l	d5,d0
		lsr.l	d6,d0
		or.l	d0,(a1)

		movem.l	(a7)+,a0/a1
		add.w	tf_Modulo(a2),a0
		add.w	TW_BITMAP+bm_BytesPerRow(a5),a1
		dbf	d2,cfRightYLoop

cfRight:
		move.w	tf_YSize(a2),d2	; initialize y counter
		subq	#1,d2
		dbf	d3,cfRightZLoop
		bra.s	cfLeftEnd

cfLeftZLoop:
		move.l	(a4)+,a0
		add.w	TW_OFFSETCHAR(a5),a0
		move.l	(a3)+,a1
		add.w	TW_OFFSETBM(a5),a1
cfLeftYLoop:
		move.w	d7,d1		; initialize x counter
		movem.l	a0/a1,-(a7)
		bra.s	cfLeftXDBF
cfLeftXLoop:
		move.l	(a0),d0
		and.l	d4,d0
		lsl.l	d6,d0
		or.l	d0,(a1)
		addq.l	#2,a1
		addq.l	#2,a0
cfLeftXDBF:
		dbf	d1,cfLeftXLoop

		move.l	(a0),d0
		and.l	d5,d0
		lsl.l	d6,d0
		or.l	d0,(a1)

		movem.l	(a7)+,a0/a1
		add.w	tf_Modulo(a2),a0
		add.w	TW_BITMAP+bm_BytesPerRow(a5),a1
		dbf	d2,cfLeftYLoop

cfLeft:
		move.w	tf_YSize(a2),d2	; initialize y counter
		subq	#1,d2
		dbf	d3,cfLeftZLoop


cfLeftEnd:
		move.l	(a7)+,a4
cfCheckDone:
		subq.w	#1,TW_LENGTH(a5)
		bne	cfCharLoop
	    
	    ;-- check bold enhancement
		btst	#FSB_BOLD,rp_AlgoStyle(a4)
		beq.s	cfChkItalic
		;-- set up parameters to OR BitMap into itself BoldSmear later
		lea	TW_BITMAP(a5),a0		; SrcBitMap
		moveq	#0,d0				; SrcX
		moveq	#0,d1				; SrcY
		move.l	a0,a1				; DstBitMap
		move.w	tf_BoldSmear(a2),d2		; DstX
		moveq	#0,d3				; DstY
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d4	; SizeX
		lsl.w	#3,d4				;
		sub.w	d2,d4				;
		move.w	tf_YSize(a2),d5			; SizeY
		moveq	#$ffffffea,d6			; Minterm
		moveq	#-1,d7				; Mask
		move.l	d0,a2				; TempA
		CALLLVO	BltBitMap
		move.l	rp_Font(a4),a2

	    ;-- check italic enhancement
cfChkItalic:
		btst	#FSB_ITALIC,rp_AlgoStyle(a4)
		beq	cfChkPen
		;-- get font variables
		move.w	tf_YSize(a2),d3
		move.w	tf_Baseline(a2),d5
		sub.w	d3,d5				; Baseline - YSize
		bge	cfChkPen			; bad Baseline!
		;-- get the TempA now once
		moveq	#0,d0
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d0
		addq.w	#2,d0
		move.l	d0,-(a7)
		moveq	#MEMF_CHIP,d1
		bsr	GetMustMem
		move.l	d0,a2				; TempA
		;-- setup for loop
		move.w	d5,d0				; SrcX =
		neg.w	d0				;   (YSize - Baseline)
		lsr.w	#1,d0				;   /2
		bcc.s	cfiOddSizeY			; check if odd start
		moveq	#2,d5				; (YSize-Baseline odd)
		bra.s	cfiSetDstX
cfiOddSizeY:
		moveq	#1,d5				; (YSize-Baseline even)
cfiSetDstX:
		moveq	#0,d2				; DstX

		move.w	TW_BITMAP+bm_BytesPerRow(a5),d4	; SizeX
		lsl.w	#3,d4				;
		sub.w	d0,d4

		moveq	#-1,d7				; Mask
		;-- loop thru Bitmap and shift pairs of rows
cfiShift:
		sub.w	d5,d3				; next Y
		bpl.s	cfisYOK
		addq.w	#1,d3				; last single line?
		bmi.s	cfisDone
		moveq	#1,d5				; last single SizeY
cfisYOK:
		cmp.w	d0,d2				; check for baseline
		beq.s	cfisNext
		move.w	d3,d1				; SrcY (from DstY)

		movem.w	d0/d2/d4,-(a7)			; save SrcX,DestX,SizeX
		;--	shift row
		moveq	#$ffffffc0,d6			; Minterm: copy
		lea	TW_BITMAP(a5),a0		; SrcBitMap
		move.l	a0,a1				; DstBitMap
		CALLLVO	BltBitMap

		cmp.w	(a7),d2				; SrcX ? DestX
		blt.s	cfisCleared			; below baseline, no clr
		move.w	d2,d4				; Size is DestX
		beq.s	cfisCleared			; first line, no clr

		;--	clear upper left
		moveq	#0,d0				; Src doesn't matter
		moveq	#0,d1				;
		moveq	#0,d2				; DestX is 0
		moveq	#$00000000,d6			; Minterm: clear
		lea	TW_BITMAP(a5),a0		; SrcBitMap
		move.l	a0,a1
		CALLLVO	BltBitMap

cfisCleared:
		movem.w	(a7)+,d0/d2/d4			; get SrcX,DestX,SizeX

cfisNext:
		addq.w	#1,d2				; DstX
		subq.w	#1,d4				; SizeX
		moveq	#2,d5				; intermediate SizeY
		bra.s	cfiShift
cfisDone:
		;-- release the TempA
		bsr	waitblitdone
		move.l	(a7)+,d0
		move.l	a2,a1
		bsr	FreeMustMem
		move.l	rp_Font(a4),a2

	    ;-- check for color remapping of FgColor to FgPen
cfChkPen:
		btst	#CTB_MAPCOLOR&$7,(1-(CTB_MAPCOLOR/8))+ctf_Flags(a2)
		beq	cfChkUnderline
		move.b	ctf_FgColor(a2),d7
		cmp.b	ctf_High(a2),d7
		bhi	cfChkUnderline	; bgt unsigned
		cmp.b	ctf_Low(a2),d7
		bcs	cfChkUnderline	; blt unsigned
		;-- generate mask for rp_FgPen based on rp BitMap Depth
		;   and check if need to change color
		moveq	#-1,d0			; move.l #$ffff0000,d0
		clr.w	d0			;
		move.l	rp_BitMap(a4),a0	; get rp BitMap
		move.b	bm_Depth(a0),d1		;   Depth
		rol.l	d1,d0			; get mask in low word
		and.b	rp_FgPen(a4),d0		; get masked rp_FgPen
		cmp.b	d0,d7			; check if need to change color
		beq.s	cfChkUnderline
		;-- construct a mask of the inverse of the FgColor
		;------ clear the mask
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a1
		move.l	TW_TEMPLATESIZE(a5),d0
		moveq	#0,d1
		CALLLVO	BltClear
		;------ OR in data from planes clear where FgColor is set
		lea	TW_BITMAP(a5),a0
		moveq	#0,d0
		moveq	#0,d1
		lea	TW_EXTRABITMAP(a5),a1
		moveq	#0,d2
		moveq	#0,d3
		moveq	#0,d4
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d4
		lsl.l	#3,d4
		moveq	#0,d5
		move.w	TW_BITMAP+bm_Rows(a5),d5
		moveq	#$ffffffba,d6		; ABC ANBC ANBNC NABC NANBC
		move.b	ctf_FgColor(a2),d7
		CALLLVO	BltBitMap
		;------ OR in data from planes set where FgColor is clear
		lea	TW_BITMAP(a5),a0
		moveq	#0,d0
		moveq	#0,d1
		lea	TW_EXTRABITMAP(a5),a1
		moveq	#$ffffffea,d6		; ABC ABNC ANBC NABC NANBC
		not.b	d7			; ~ctf_FgColor
		CALLLVO	BltBitMap
		;------ OR in the data to planes were FgPen is set
		lea	TW_EXTRABITMAP(a5),a0
		moveq	#0,d0
		moveq	#0,d1
		lea	TW_BITMAP(a5),a1
		moveq	#$ffffffba,d6		; ABC ANBC ANBNC NABC NANBC
		move.b	rp_FgPen(a4),d7
		CALLLVO	BltBitMap
		;------ AND in the data to planes were FgPen is clear
		lea	TW_EXTRABITMAP(a5),a0
		moveq	#0,d0
		moveq	#0,d1
		lea	TW_BITMAP(a5),a1
		moveq	#$ffffff8a,d6		; ABC NABC NANBC
		not.b	d7			; ~rp_FgPen
		CALLLVO	BltBitMap


	    ;-- check underline enhancement
cfChkUnderline:
		btst	#FSB_UNDERLINED,rp_AlgoStyle(a4)
		beq	cfChkInverse
		;-- get the text mask
		bsr	cfTextMask
		;-- get the underline template
		;--	get the memory
		moveq	#0,d0
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d0
		move.l	d0,d7
		moveq	#MEMF_CHIP,d1
		bsr	GetMustMem
		move.l	d0,a3
		;--	clear the memory
		move.l	d7,d0
		moveq	#1,d1			; synchronous clear
		move.l	a3,a1
		CALLLVO	BltClear
		;-- render underline template
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a0
		move.w	tf_Baseline(a2),d6
		addq.w	#1,d6
		cmp.w	tf_YSize(a2),d6
		bge.s	cfuFreeMustMem
		mulu	TW_BITMAP+bm_BytesPerRow(a5),d6
		add.l	d6,a0
		move.l	a3,a1
		bsr	underlineA0
		;-- render underline from template
		lea	TW_BITMAP+bm_Planes(a5),a2
		moveq	#0,d5
		move.b	TW_BITMAP+bm_Depth(a5),d5
		moveq	#0,d4
		move.b	rp_FgPen(a4),d4
		bra.s	cfuZDBF
cfuZLoop:
		move.l	a3,a0
		move.l	(a2)+,a1
		add.l	d6,a1
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d3
		lsr.w	#1,d3
		lsr.w	#1,d4
		bcc.s	cfuX0DBF
		bra.s	cfuX1DBF
cfuX0Loop:
		move.w	(a0)+,d0
		not.w	d0
		and.w	d0,(a1)+
cfuX0DBF:
		dbf	d3,cfuX0Loop
		bra.s	cfuZDBF
cfuX1Loop:
		move.w	(a0)+,d0
		or.w	d0,(a1)+
cfuX1DBF:
		dbf	d3,cfuX1Loop
cfuZDBF:
		dbf	d5,cfuZLoop

		;-- free the underline template
cfuFreeMustMem:
		move.l	d7,d0
		move.l	a3,a1
		bsr	FreeMustMem
		;-- restore font pointer
		move.l	rp_Font(a4),a2

cfChkInverse:
		btst	#2,rp_DrawMode(a4)	; check INVERSE bit
		beq.s	cfChkJam1
	    ;-- invert the character data
		lea	TW_BITMAP(a5),a0
		move.l	a0,a1
		moveq	#0,d0
		moveq	#0,d1
		moveq	#0,d2
		moveq	#0,d3
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d4
		lsl.l	#3,d4
		move.w	TW_BITMAP+bm_Rows(a5),d5
		moveq	#$3a,d6			; ANBNC ANBC NABC NANBC
		moveq	#-1,d7
		CALLLVO	BltBitMap

cfChkJam1:
	    ;-- check for JAM1 write
		move.b	rp_DrawMode(a4),d0
		and.b	#RP_COMPLEMENT+RP_JAM2,d0 ; check for other than JAM1
		bne.s	cfNotMaskedBlt		; no need to make text mask

		;-- get the text mask
		bsr.s	cfTextMask
		;-- clear under the mask
		bsr	cfGetBltParms
		lea	TW_EXTRABITMAP(a5),a0	; srcBitMap
		moveq	#$2a,d6			; ANBC NABC NANBC
		CALLLVO BltBitMapRastPort

cfNotMaskedBlt:
	    ;-- copy the text to the rastPort
		bsr	cfGetBltParms
		lea	TW_BITMAP(a5),a0	; srcBitMap
		moveq	#0,d6			; minterm
		move.b	rp_DrawMode(a4),d6
		and.b	#3,d6
		move.b	cfMinterms(pc,d6.w),d6
		CALLLVO BltBitMapRastPort

	    ;-- free the EXTRAPLANE
		bsr	waitblitdone
		;-- check if this is the RastPort TmpRas
		move.l	TW_TEMPLATESIZE(a5),d0
		move.l	rp_TmpRas(a4),d1
		beq.s	cfFreeExtra
		move.l	d1,a0
		move.l	tr_RasPtr(a0),d1
		beq.s	cfFreeExtra
		cmp.l	tr_Size(a0),d0
		ble.s	cfFreeBitmap

		;-- free the TmpRas
cfFreeExtra:
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a1
		LINKEXE	FreeMem

	    ;-- free the BitMap planes
cfFreeBitmap:
		moveq	#7,d5
cfFreePlanes:
		move.w	d5,d1
		lsl.w	#2,d1
		move.l	TW_BITMAP+bm_Planes(a5,d1.w),d0
		beq.s	cfNextFreePlane
		move.l	d0,a1
		move.l	TW_TEMPLATESIZE(a5),d0
		LINKEXE	FreeMem
cfNextFreePlane:
		dbf	d5,cfFreePlanes
		bra	freeLocal
		
cfMinterms:
		;--				      _  _   __ _   _ _ __  ___
		;				ABC ABC ABC ABC ABC ABC ABC ABC
		; JAM1: copy under mask
		dc.b	$ea
		; JAM2: copy block
		dc.b	$ca
		; COMPLEMENT: complement under mask
		dc.b	$6a
		; JAM2 COMPLEMENT: complement block
		dc.b	$5a

cfTextMask:
	    ;-- construct the text mask
		;-- clear the mask
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a1
		move.l	TW_TEMPLATESIZE(a5),d0
		moveq	#0,d1
		CALLLVO	BltClear
		;-- OR in all the text planes
		lea	TW_BITMAP(a5),a0
		moveq	#0,d0
		moveq	#0,d1
		lea	TW_EXTRABITMAP(a5),a1
		moveq	#0,d2
		moveq	#0,d3
		moveq	#0,d4
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d4
		lsl.l	#3,d4
		moveq	#0,d5
		move.w	TW_BITMAP+bm_Rows(a5),d5
		moveq	#$ffffffea,d6		; ABC ABNC ANBC NABC NANBC
		moveq	#-1,d7
		CALLLVO	BltBitMap
		rts


cfGetBltParms:
		move.l	a4,a1			; destRastPort
		move.w	TW_OFFSETX0(a5),d0	; srcX
		lsl.w	#3,d0
		move.w	TW_MIN(a5),d6
		add.w	d6,d0
		ext.l	d0
		moveq	#0,d1
		move.w	rp_cp_x(a4),d2		; get destX
		add.w	d6,d2			; adjusted for MIN
		ext.l	d2
		move.w	rp_cp_y(a4),d3		; get destY
		sub.w	tf_Baseline(a2),d3	;   adjusted for even baselines
		ext.l	d3
		move.w	TW_MAX(a5),d4		; get sizeX
		sub.w	d6,d4
		ext.l	d4
		move.w	tf_YSize(a2),d5		; get sizeY
		ext.l	d5
		rts

	END
