**
**	$Id: write.asm,v 1.2 93/04/23 09:05:16 darren Exp $
**
**      EUC text write primitive
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

DEBUG_DETAIL	SET	0

*------ Imported Names -----------------------------------------------

	XREF		_OpenEUCFont

	XREF		_EUCtextLength
	XREF		_EUCtrueLength


*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF		_EUCText

	IFEQ	FAST_PRECHAR
		XDEF		EUCGetPreChar
		XDEF		EUCSetPreChar
	ENDC

*------ Macros -------------------------------------------------------

*------ Assumptions --------------------------------------------------

	IFNE	EUC_HICHAR-$FF
	FAIL	"EUC high char is not $FF; recode!"
	ENDC

******* eucdriver.library/EUCText ************************************
*
*   NAME
*	EUCText -- Write EUC text characters.
*
*   SYNOPSIS
*	EUCText(rp, string, length)
*	        A1  A0      D0-0:16
*
*	void EUCText(struct RastPort *, STRPTR, WORD);
*
*   FUNCTION
*	See graphics/Text().  Replaces graphics/Text() when
*	rendering an EUC font.
*
*	Single character output is partially supported, though
*	not recommended (it is recommended that complete, and
*	non-ambiguous EUC text strings be printed by the application).
*	Ambiguous EUC bytes are stored in the RastPort->rp_Dummy
*	field, and the ambiguous character is printed as white-space.
*	Upon subsequent calls to Text(), this function checks for
*	a previous character, and if found, backspaces the
*	RastPort->rp_cp_x position, and prints the text string as
*	if the previous character was pre-pended to the string.
*
*	When calling the various text fitting functions, such as
*	TextLength(), the previous character (if any) is not
*	checked for, so may throw off your calculations if intermixed
*	with Text() calls printing ambiguous strings.  Once again, the
*	proper solution is to print whole lines of text, and avoid
*	printing strings which end in ambiguous EUC bytes.
*
*	Japanese fonts are marked as mono-spaced fonts.  This is not
*	entirely true because not every glyph has the same width,
*	however ... for many cases it is true that there is a direct
*	relationship of number of bytes in a string and printed width.
*	This is because the single-byte characters in an EUC string
*	are printed as half the width of the double-byte Japanese
*	character codes.  The application may mix use of fonts
*	(e.g., a multi-font word-processor), but for those applications
*	which require mono-spaced fonts (or console.device which
*	requires mono-spaced fonts), the Japanese fonts are most likely
*	as usable as any other mono-spaced font.  There is one catch
*	here ... if RastPort->TxSpacing is greater than 0 (most
*	applications do not use RastPort->TxSpacing), the amount of
*	space added is added per glyph, so the printed pixel width
*	of the string is no longer a simple multiplication of byte
*	length times font->XSize.
*
*	Use of the graphics text fitting functions does not check
*	for pre-character bytes.  It is assumed that if you are
*	using these functions that you are also printing non-ambiguous
*	EUC text strings.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text is to be output
*	string - the address of string to output
*	length - the number of characters in the string.
*	         If zero, there are no characters to be output.
*
*
*   NOTES
*	o   Assumes that the string being printed is the EUC codeset
*	    so character codes greater than or equal to 0xA1 are
*	    interpreted as half of an EUC byte pair.
*	o   Printing of Latin1 characters is supported if followed
*	    by a second byte which is less than 0xA1.  While this
*	    practice is not recommended, it may be required by some
*	    applications.
*	o   Printing EUC character codes greater than 0xF4 will return
*	    unpredictable results.  These codes are reserved for
*	    future expansion, and gaiji.
*	o   Changing the text direction with RastPort->TxSpacing is
*	    not supported.
*	o   Negative RastPort->TxSpacing is not supported.
*	o   Positive RastPort->TxSpacing is added per glyph, so
*	    for example, a spacing value of one (1) pixel will
*	    add one pixel of white space per ROMAN single-byte
*	    character printed, and one per JAPANESE double-byte
*	    character printed.
*	o   Roman single-byte characters must be half the width
*	    of Japanese double byte characters.
*
*	Color fonts are supported, though slow because they are
*	rendered one character at a time.  In the future a multi-
*	plane color font template may be constructed (as we do for
*	mono-plane fonts), but in the interim, this solution is
*	effective and avoids duplication of a great deal of code
*	which exists in KS ROM.  Also in the future we are looking
*	at opening up the Text() code as a set of subroutines so that
*	font drivers like the EUC font driver do not have to
*	duplicate code which already exists in ROM (e.g., template
*	construction).  Note that because we do not create a multi-
*	plane template when rendering color fonts, algorithmic bold,
*	italic, and underlined do not work well.  This is probably
*	not a significant loss since algorthmic style enhancements
*	are not suitable for DTV use, nor do they work well for
*	Japanese text in any case.  
*
*	!!!IMPORTANT!!!
*
*	Because of the large amount of data required to store all
*	glyphs in a Japanese font, it is not ideal to require that
*	the font data be entirely loaded off disk and cached in RAM
*	when a Japanese font is opened.  In general the short delays
*	required to load Japanese font data during rendering do not
*	adversely affect productivity software, however DTV applications
*	which require real-time scrolling, or other video effects
*	may expect that once a font is opened, it can be rendered without
*	any further disk access (which was TRUE for our Latin1 fonts).
*
*	There are a couple of ways to "pre-load" Japanese font glyph
*	data.  First, you can simply pre-render all of the text you
*	will be scrolling into an off-screen rastport.  This forces all
*	glyph data to be pre-loaded, though note that during low-memory
*	conditions the glyph data may be partially flushed, and low-memory
*	conditions can occur at any time in a multi-tasking environment.
*
*	The second way is to render the glyphs you need with the Text()
*	function, and cache these yourself.  There are no complex metrics
*	associated with Japanese fonts, so Text() is adequate for the
*	purpose of rendering Japanese glyphs.
*
*	Finally it is possible to create a utility which pre-opens all
*	or a subset of the "sub-fonts" associated with a Japanese
*	font family (and of interest, it is also possible to install
*	all sub-fonts by providing these on an XIP PCMCIA Japanese
*	font card).  We may for example provide such a utility with
*	the system software.
*
*   BUGS
**********************************************************************

*****i* graphics.library/Text ****************************************
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
*****i* eucdriver.library/EUCText ************************************
*
*   NAME
*	EUCText -- Write text characters (EUC text).
*
*   SYNOPSIS
*	EUCText(rp, string, length, func, userdata)
*	        A1  A0      D0-0:16   A5    A6
*
*	void EUCText(struct RastPort *, STRPTR, WORD, APTR, APTR);
*
*   FUNCTION
*	See graphics/Text() -- specific too EUC text output
*
*	This code is basically a perverted version of the
*	graphics.library Text() function with EUC pre/post-char
*	support, parsing support, no proportional font support, and
*	code for rendering white-space enhanced EUC fonts.
*
*   INPUTS
*	rp     - a pointer to the RastPort which describes where the
*	         text is to be output
*	string - the address of string to output
*	length - the number of characters in the string.
*	         If zero, there are no characters to be output.
*
*	func   - Pointer to original Text() function
*
*	userdata - Pointer to this fonts userdata for EUC driver
*
*   NOTES
*	o   This function may use the blitter.
*	o   Changing the text direction with RastPort->TxSpacing is
*	    not supported.
*
*   BUGS
**********************************************************************
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
; d7 - length
;
; a2 - TextFont pointer
; a3 - string parameter
; a4 - rastport parameter

_EUCText:

	PRINTF	DBG_FLOW,<'EUCText($%lx,$%lx,%ld,$%lx,$%lx)'>,A1,A0,D0,A5,A6


	;-- check length

		tst.w	d0
		beq	etx_notext

		movem.l	d2-d7/a2-a6,-(sp)

		movem.l	d0/a0/a1,-(sp)
		movem.l	(a7)+,d7/a3/a4		; from d0/a0/a1
		move.l	rp_Font(a4),a2

	;-- check if color font - if so, use regular Text() routine (slow, but
	;-- effective for this rare case).

		btst.b	#FSB_COLORFONT,tf_Style(a2)
		bne	etx_ColorFontMode

	;-- check if need to lock layer

		move.l	(a4),d1			;rp_Layer
		beq.s	etx_getlength

		movem.l	a5-a6,-(sp)
		move.l	efh_GfxBase(a6),a6
		move.l	d1,a5
		JSRLIB	LockLayerRom
		movem.l	(sp)+,a5-a6

etx_getlength:

	;-- calc length of this string 

*
*	D0/D1/D2 = EUCtextLength(textfont, string, rp, length, userdata)
*		                 A2        A3      A4  D7      A6

		bsr	_EUCtextLength


	;-- reserve space for local variables

		suba.w	#TW_SIZE,a7
		move.l	a7,a5

		move.b	#1,TW_BITMAP+bm_BytesPerRow(a5)

	;-- initialize variables

		movem.w	d0/d1/d2,TW_CP(a5)

	;-- preclip

		move.l	(a4),d3
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
		BEQ_S	freeLocal
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

;  d0	CP advance - width not taking into account BOLD or ITALIC advance
;  d1   true +width, taking into account BOLD or ITALIC advance
;  d2	MIN
;  d3	TW_BITMAP+bm_BytesPerRow
;  d6	YSize
;  a1	TEMPLATE
;
		move.w	tf_YSize(a2),d6		; cache YSize
		beq.s	freeLocal		; no height => no rendering

etx_notwhitespaced:
		
	;-- get template

		;-- bm_BytesPerRow is based on rendering length
		;-- the subtraction is really addition because
		;-- we subtract <=0 MIN from >=0 MAX, the only way
		;-- this can be 0 is if both MIN and MAX are 0.

		move.w	d1,d3			; MAX
		sub.w	d2,d3			;     - MIN
		beq.s	freeLocal		; no width => no rendering

	;-- for simplicity, do not bother trying to handle byte-wide
	;-- case for Roman chars - just not worth it

		add.w	#79,d3			;round up to word + 4 word pad
						;16*4 = 64 + 15 = 79

		add.w	tf_Baseline(a2),d3	;what the heck does this do
						;in Text()???  for italic??
	;--^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	;
	;-- This is supposed to fix a problem with Italics when using
	;-- tall fonts.  It was added for V39, and now even the author
	;-- is not sure why it fixes the bug, or what the bug is.
	;-- My recollection is that when algol italics were done on
	;-- a font whose baseline is >32, there was some trash in the
	;-- first line of template.
	;
	;-- Looking at this code, my belief is it assumes that baseline
	;-- is >= YSize/2, and that because of this, is always greater
	;-- than the worst number of extra pixels needed for padding when
	;-- using the italic code in Text() - because the Italic code
	;-- assumes NULL bit padding is available for shifting, and because
	;-- for a font whose baseline is >32, this can mean a greater than
	;-- 16 pixel shift, meaning that the source and destination are not
	;-- the same word.
	;
	;-- In all cases it seems to me that our Italics code reads past
	;-- the end of its allocated memory.
	;

		bsr	getTemplate
		tst.l	d1
		bne.s	gotTemplate

**
** Low memory condition - try one byte at a time - better than Alert, and
** chances are it will succeed because 1 char is likely to require less
** than MAXMUSTMEM memory.  Fortunately, the mono-spaced Japanese fonts
** will look OK when output via the 1 character method.
**
	PRINTF	DBG_FLOW,<'EUC--LOW MEM TEXT LOOP!'>

		subq.w	#1,d7			;ack, even one character is too big!
		beq.s	freelowmem

		subq.l	#2,sp			;place byte on stack
LowMemTextloop:
		move.b	(a3)+,(sp)
		move.l	sp,a0
		move.l	a4,a1			;rastport
		moveq	#1,d0			;length is 1
	;-- A5 does not point at graphics.library _Text(), but is not needed
	;-- since it is only needed for ColorFonts.  A6 is still valid

		bsr	_EUCText		;recurse!
		dbf	d7,LowMemTextloop

		addq.l	#2,sp
		bra.s	freelowmem

	;-- restore stack/registers/get-out

freeLocal:
		move.w	TW_CP(a5),d0		;update current position
		add.w	d0,rp_cp_x(a4)

freelowmem:
		adda.w	#TW_SIZE,sp

	;-- unlock the layer if required

		move.l	(a4),d0			;if rp_Layer
		beq.s	etx_endtext


	;-- exiting, no need to preserve A5 or A6 - restored below

		move.l	efh_GfxBase(a6),a6
		move.l	d0,a5
		JSRLIB	UnlockLayerRom

etx_endtext:
		movem.l	(sp)+,d2-d7/a2-a6
etx_notext:
		rts

**
** Now we have a template, so this code does the rendering
**

gotTemplate:

		;-- get TW_EXTRABITMAP+bm_Planes address based on MIN
		sub.w	tf_Baseline(a2),d2
		move.w	d2,d0			; MIN
		asr.w	#4,d0			; get offset word
		add.w	d0,d0			;   in bytes
		neg.w	d0			;
		lea	4(a1,d0.w),a0		; pad with 2 words
		move.l	a0,TW_EXTRABITMAP+bm_Planes(a5)	; thus X=0 is here

	;-- clear template

		move.w	d6,d0			; height
		swap	d0			; in high word
		move.w	d3,d0			;     and width
		moveq	#3,d1			; row/word count, synchronous clear

		move.l	a6,-(sp)
		move.l	efh_GfxBase(a6),a6
		JSRLIB	BltClear
		move.l	(sp)+,a6

**
** EUC parsing support
**
** Since EUC bytes are byte pairs, but ROMAN ASCII are 8-bit bytes,
** this Text() code must parse byte streams for EUC pairs, and
** treat all other bytes as standard 8 bit ECMA Latin1 characters.
** As a side effect, this means we can support printing of Latin1
** characters if followed by a non-EUC byte.
**
** Single character output support:
**
**	This is done by caching ambiguous bytes in the rastport,
**	and picking up ambiguous bytes for the next Text() call.
**	White space is printed for ambiguous bytes, and backspaced
**	over on the next Text() call.
**
**	This is problematic when mixing Text Fitting functions, but
**	I haven't found a good solution for this other than the
**	obvious - don't print ambiguous strings!  Most applications
**	which are helped by this single char support hopefully don't
**	use text fitting
**
**
** Double byte character, but font file missing:
**
**	These are printed as white space because of precedence
**	in DOS-V.  Apparently NEC's PC does not use the JIS
**	glyph set, but a variation in which some of the sets
**	of characters (e.g., symbols in the $A2XX range) are
**	unique to the NEC.  DOS-V just prints white space, while
**	we will print from the JIS glyph set.  However we have the
**	option of removing, or replacing a group of glyphs with
**	NEC compatable glyphs if desired.  I've left either option
**	open depending on what we decide later.
**
**	Infact, we want to print white-space plus rp_TxSpacing
**	so that what we print matches what the text fitting
**	functions would have allocated.
**

	;-- process EUC pre-char byte (if any)

		clr.w	TW_CP(a5)		; no advance yet
		move.l	a3,TW_STRING(a5)
		move.w	d7,TW_LENGTH(a5)

	IFEQ	FAST_PRECHAR

		move.l	a4,a1
		bsr	EUCGetPreChar
	ENDC

	IFNE	FAST_PRECHAR
		
		move.b	rp_Dummy(a4),d1
	ENDC

		cmp.b	#EUC_LOCHAR,d1
		bcs.s	nbProcessString		;BLT unsigned - normal processing

	;-- Must output pre-char, plus first char of string this time

	;-- back-up cp_x for rendering

		move.w	rp_cp_x(a4),d0
		sub.w	tf_XSize(a2),d0
		sub.w	rp_TxSpacing(a4),d0
		move.w	d0,rp_cp_x(a4)

		moveq	#00,d0
		move.b	d1,d0

	IFEQ	FAST_PRECHAR
		moveq	#00,d1
		bsr	EUCSetPreChar		;clear it - have it, process it
	ENDC
	IFNE	FAST_PRECHAR
		moveq	#00,d1
		move.b	d1,rp_Dummy(a4)
	ENDC

		move.b	(a3)+,d5		;check first byte of provided string
		sub.w	#1,d7			;length - 1

		move.l	a3,TW_STRING(a5)
		move.w	d7,TW_LENGTH(a5)

		move.l	d0,d1			;save

		cmp.b	#EUC_LOCHAR,d5
		bcs.s	nbLatin1		;must be a Latin1 and 7-bit ASCII

		bsr	_OpenEUCFont

		tst.l	d0			;if no font, fill with white space
		beq.s	nbPreWhiteSpace

		movem.l	a2/a6,-(sp)
		move.l	d0,a2			;font pointer
		move.l	d5,d0			;char code to draw
		bsr	nbDrawKanji
		move.l	a2,a1
		move.l	efh_GfxBase(a6),a6
		JSRLIB	CloseFont		;done with this font - close it

		movem.l	(sp)+,a2/a6
		bra.s	nbProcessString

nbPreWhiteSpace:
		bsr	nbDrawWhiteKanji
		bra.s	nbProcessString
nbLatin1:
		move.l	d1,d0			;restore
		bsr	nbDrawChar		;draw char in D0 in template
		move.b	d5,d0
		bsr	nbDrawChar		;draw next char in D0 in template

	;-- save some used registers

nbProcessString:

nbCharLoop:
		move.w	TW_LENGTH(a5),d7
		subq.w	#1,d7
		bmi.s	nbCheckBold

		move.l	TW_STRING(a5),a0
		moveq	#00,d1
		move.b	(a0)+,d1

		cmp.b	#EUC_LOCHAR,d1
		bcs.s	romanchar

	;-- maybe EUC

		subq.w	#1,d7
		bmi.s	postchar

		move.b	(a0)+,d5

		move.l	a0,TW_STRING(a5)
		move.w	d7,TW_LENGTH(a5)

		cmp.b	#EUC_LOCHAR,d5
		bcs.s	tworomanchar		;nope, just a Latin1 plus other

	;-- open EUC font for this EUC code

		moveq	#00,d0
		move.b	d1,d0

		bsr	_OpenEUCFont
		tst.l	d0
		beq.s	nbWhiteSpace		;gack - no Kanji font - print white space plus TxSpacing

		movem.l	a2/a6,-(sp)
		move.l	d0,a2			;font pointer
		move.l	d5,d0			;char code to draw
		bsr	nbDrawKanji
		move.l	a2,a1
		move.l	efh_GfxBase(a6),a6
		JSRLIB	CloseFont		;done with this font - close it

		movem.l	(sp)+,a2/a6
		bra.s	nbCharLoop

	;-- Kanji font file not available - draw white space

nbWhiteSpace:
		bsr	nbDrawWhiteKanji
		bra.s	nbCharLoop

	;-- standard single byte roman char

romanchar:
		move.b	d1,d0
		move.l	a0,TW_STRING(a5)
		move.w	d7,TW_LENGTH(a5)
		bsr	nbDrawChar
		bra.s	nbCharLoop

	;-- got a couple of Roman chars to output

tworomanchar:

		move.b	d1,d0
		bsr	nbDrawChar
		move.b	d5,d0
		bsr	nbDrawChar
		bra.s	nbCharLoop

	;-- postchar - store it, and print space

postchar:
	IFEQ	FAST_PRECHAR
		move.l	a4,a1
		bsr	EUCSetPreChar
	ENDC
	IFNE	FAST_PRECHAR
		move.b	d1,rp_Dummy(a4)
	ENDC

		moveq	#' ',d0
		bsr	nbDrawChar

	;--
	;-- BOLD handling
	;--

nbCheckBold:

	;-- check bold enhancement

		btst	#FSB_BOLD,rp_AlgoStyle(a4)
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
		movem.l	d2-d7/a2/a6,-(a7)
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
		move.l	efh_GfxBase(a6),a6
		JSRLIB	BltBitMap
		movem.l	(a7)+,d2-d7/a2/a6

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
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a0
		move.l	a4,a1
		move.w	TW_MIN(a5),d0
		move.w	TW_BITMAP+bm_BytesPerRow(a5),d1
		move.w	rp_cp_x(a4),d2
		add.w	d0,d2
		move.w	rp_cp_y(a4),d3
		sub.w	tf_Baseline(a2),d3
		move.w	TW_MAX(a5),d4

		sub.w	d0,d4

		move.w	tf_YSize(a2),d5		; get sizeY

		move.l	a6,-(sp)
		move.l	efh_GfxBase(a6),a6
		JSRLIB	BltTemplate
		move.l	(sp)+,a6

	;-- free template if not TmpRas

		move.l	TW_TEMPLATESIZE(a5),d0

		move.l	rp_TmpRas(a4),d1
		beq.s	freeTemplate
		move.l	d1,a0
		move.l	tr_RasPtr(a0),d1
		beq.s	freeTemplate
		cmp.l	tr_Size(a0),d0
		bls	freeLocal

	    ;-- free the template
freeTemplate:
		move.l	a6,-(sp)
		move.l	efh_GfxBase(a6),a6
		JSRLIB	WaitBlit
		move.l	(sp)+,a6

		move.l	TW_BITMAP+bm_Planes(a5),d0
		bsr	EUCFreeMustMem
		bra	freeLocal

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

**
** Place character in template - character in D0
**
** A2 - preserved - font pointer
** A3 - destroyed
** A4 - preserved - rastport pointer
** D5 - preserved - char from D0
**

	;
	;- Draw a Kanji white space (plus TxSpacing * 1)
	;

nbDrawWhiteKanji:
		move.w	TW_CP(a5),d3
		move.w	tf_XSize(a2),d0
		add.w	d0,d0
		add.w	d0,d3
		add.w	rp_TxSpacing(a4),d3
		move.w	d3,TW_CP(a5)		;template already clear - just advance
		rts
		
	;
	;- Draw a Kanji character
	;
nbDrawKanji:
		moveq	#01,d4
		btst.b	#FFB_WHITESPACED,efh_FontFlags+1(a6)
		bne.s	nbdoDraw

	;
	;- Draw a Latin1 character
	;
nbDrawChar:
		moveq	#00,d4			;flag
nbdoDraw:
		movem.l	d5/a2,-(sp)
	;-- bound to known glyphs in this font

		moveq	#00,d5
		move.b	d0,d5
		cmp.b	tf_HiChar(a2),d5
		bhi.s	nbUnknownChar
		sub.b	tf_LoChar(a2),d5
		bcc.s	nbKnownChar

	;-- use undefined glyph

nbUnknownChar:

		move.b	tf_HiChar(a2),d5
		sub.b	tf_LoChar(a2),d5
		addq.w	#1,d5

	;-- Though Japanese fonts are assumed to be mono spaced for now,
	;-- font files may have compressed leading white space, and
	;-- trailing space, so we still have to look at space/kerning

nbKnownChar:
		add.w	d5,d5			;offset for word table lookup
		move.w	TW_CP(a5),d3
		move.l	tf_CharKern(a2),d0
		beq.s	nbGotNear
		move.l	d0,a0
		add.w	0(a0,d5.w),d3
nbGotNear:

		move.w	d3,d7			;save CP of char start
		move.l	tf_CharSpace(a2),d0
		bne.s	nbGotFar
		add.w	tf_XSize(a2),d3
		bra.s	nbSpacing

nbGotFar:
		move.l	d0,a0
		add.w	0(a0,d5.w),d3

nbSpacing:

		add.w	rp_TxSpacing(a4),d3

		tst.w	d4
		beq.s	nbnokanji1

		addq.w	#1,d7			;skip one pixel of white space
		addq.w	#2,d3			;and one at end & beg of line
nbnokanji1:


	;-- save CP

		move.w	d3,TW_CP(a5)

	;-- find character data start and size

		add.w	d5,d5			;long offset now
		move.l	tf_CharLoc(a2),a0
		move.l	0(a0,d5.w),d0
		move.w	d0,d5			;char data bit length
		beq	doneRendering

		swap	d0
		move.w	d0,d3
		and.w	#$0F,d3			;char data start bit
		lsr.w	#4,d0			;(always positive)
		add.w	d0,d0			;even word address
		move.l	tf_CharData(a2),a1
		add.w	d0,a1
		move.w	tf_Modulo(a2),a3

	;-- get number of lines to copy

		move.w	tf_YSize(a2),d6

	;-- calculate the 32 bit mask

		move.w	d5,d0
		subq.w	#1,d0			; make 16 fast?
		and.w	#15,d0			; # of bits to save
		add.w	d0,d0			; word index
		lea	_lwmaskTable(pc),a0		
		move.w	d4,-(sp)
		move.w	0(a0,d0.w),d4		; 1 = first entry
		swap	d4
		clr.w	d4
		lsr.l	d3,d4			; shift into position

	;-- calculate start line position

		move.w	d7,d0
		and.w	#$0F,d7
		asr.w	#4,d0
		add.w	d0,d0
		move.l	TW_EXTRABITMAP+bm_Planes(a5),a0
		add.w	d0,a0
		move.w	TW_BITMAP+bm_BytesPerRow(a5),a2

	;-- adjust for white-spaced font

		move.w	(sp)+,d0
		beq.s	nbcopytype

		add.w	a2,a0			; skip line

nbcopytype:
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
		bra.s	doneRendering

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
		bra.s	doneRendering


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
		bra.s	doneRendering

thinLeftLoop:
		move.l	(a1),d0
		and.l	d4,d0
		lsl.l	d3,d0
		or.l	d0,(a0)

		adda.w	a3,a1		; bump char data by it's modulo
		adda.w	a2,a0		; bump template by it's modulo
thinLeft:
		dbf	d6,thinLeftLoop

doneRendering:
		movem.l	(sp)+,d5/a2
		rts


**
** allocate template - use pre-alloced memory if possible, and should
** always be possible except for huge memory case
**

getTemplate:
		lsr.w	#4,d3			;convert to words
		add.w	d3,d3			;convert to bytes
		move.w	d3,TW_BITMAP+bm_BytesPerRow(a5)

		;-- calc template size

		move.w	d6,d0			;YSize cached above
		mulu	d3,d0
		move.l	d0,TW_TEMPLATESIZE(a5)

		;-- get template

		move.l	rp_TmpRas(a4),d1
		beq.s	needTemplate

		move.l	d1,a0
		move.l	tr_RasPtr(a0),d1
		beq.s	needTemplate

		cmp.l	tr_Size(a0),d0
		bls.s	cacheTAddr

needTemplate:
		bsr.s	EUCGetMustMem
		move.l	d0,d1

cacheTAddr:
		move.l	d1,a1
		move.l	a1,TW_BITMAP+bm_Planes(a5)
		rts


**
** This code handles printing of color fonts via multi calls to Text() -
** slow, but effective for this rare case
**

etx_ColorFontMode
		bra	etx_endtext


*****i* eucdriver.library/EUCGetMustMem ******************************
*
*   NAME
*	EUCGetMustMem -- allocates from private or public memory      
*
*   SYNOPSIS
*	success = EUCGetMustMem(size,userdata)
*	D0	                D0   A6
*
*   FUNCTION
*	Returns CHIP ram from pre-alloced memory, or alloced memory
*	if pre-alloced is in use or too small.
*
*   INPUTS
*	size - amount of CHIP RAM required
*
*	userdata - pointer to this fonts handle
*
*   RETURNS
*	D0 - Memory pointer or NULL for failure
*
*   NOTES
*
*   BUGS
**********************************************************************
EUCGetMustMem:

	PRINTF	DBG_ENTRY,<'EUC--EUCMustMem($%lx)'>,D0

		movem.l	d2/a5/a6,-(sp)
		move.l	efh_DriverBase(a6),a5
		move.l	efh_SysBase(a6),a6
		move.l	d0,d2

		cmp.l	#MAXMUSTMEM,d0
		bhi.s	mustTryAlloc

		lea	edvr_MustMemSemaphore(a5),a0
		JSRLIB	AttemptSemaphore
		
		tst.l	d0
		beq.s	mustmem_inuse
returnmustmem:

		move.l	edvr_MustMem(a5),d0
gotmustmem:

	PRINTF	DBG_EXIT,<'EUC--$%lx=EUCMustMem()'>,D0

		movem.l	(sp)+,d2/a5/a6
		rts

mustmem_inuse:
		move.l	d2,d0		;recover
mustTryAlloc:
		move.l	#MEMF_CHIP,d1
		JSRLIB	AllocVec

		tst.l	d0
		bne.s	gotmustmem

		cmp.l	#MAXMUSTMEM,d2
		bhi.s	gotmustmem	;return 0

	PRINTF	DBG_FLOW,<'EUC--[MUST wait for MUSTMEM to be free]'>

		lea	edvr_MustMemSemaphore(a5),a0
		JSRLIB	ObtainSemaphore
		BRA_S	returnmustmem

**
** The inverse - free MustMemory
**
EUCFreeMustMem:

	PRINTF	DBG_ENTRY,<'EUC--EUCFreeMustMem($%lx)'>,D0

		movem.l	a5/a6,-(sp)
		move.l	efh_DriverBase(a6),a5
		move.l	efh_SysBase(a6),a6

		cmp.l	edvr_MustMem(a5),d0
		bne.s	freeallocmust

		lea	edvr_MustMemSemaphore(a5),a0
		JSRLIB	ReleaseSemaphore
freedMem:
		movem.l	(sp)+,a5-a6
		rts
freeallocmust:
		move.l	d0,a1
		JSRLIB	FreeVec
		bra.s	freedMem


*****i* eucdriver.library/GetPreChar *********************************
*
*   NAME
*	GetPreChar -- function for getting EUC pre-char
*
*   SYNOPSIS
*	prechar = EUCGetPreChar(rp)
*	D1	                A1
*	UBYTE
*
*   FUNCTION
*	Returns pre-char byte.  Function in case we need to move
*	it later, or support storing in rp_Dummy or rastport Layer
*	if available.
*
*   INPUTS
*	rp     - rastport pointer
*
*   RETURNS
*	D1:16 - prechar byte
*
*   NOTES
*	D0/A0/A1 preserved.  Upper word of D1 unaffected.
*
*   BUGS
**********************************************************************
	IFEQ	FAST_PRECHAR
EUCGetPreChar:
		move.b	rp_Dummy(a1),d1
		rts

*****i* eucdriver.library/SetPreChar *********************************
*
*   NAME
*	SetPreChar -- function for setting EUC pre-char
*
*   SYNOPSIS
*	EUCSetPreChar(rp, prechar)
*		      A1  D1
*
*   FUNCTION
*	Set pre-char byte.  Function in case we need to move
*	it later, or support storing in rp_Dummy or rastport Layer
*	if available.
*
*   INPUTS
*	rp     - rastport pointer
*       prechar - UBYTE in D1
*
*   RETURNS
*
*   NOTES
*	D0/D1/A0/A1 preserved.
*   BUGS
**********************************************************************

EUCSetPreChar:

		move.b	d1,rp_Dummy(a1)
		rts
	ENDC


**
** Mask table from graphics.library
**
_lwmaskTable:
		dc.w    $08000
		dc.w	$0C000
		dc.w	$0E000
		dc.w	$0F000
		dc.w	$0F800
		dc.w	$0FC00
		dc.w	$0FE00
		dc.w	$0FF00
		dc.w	$0FF80
		dc.w	$0FFC0
		dc.w	$0FFE0
		dc.w	$0FFF0
		dc.w	$0FFF8
		dc.w	$0FFFC
		dc.w	$0FFFE
_fwmaskTable:
		dc.w    $0FFFF
		dc.w	$07FFF
		dc.w	$03FFF
		dc.w	$01FFF
		dc.w	$00FFF
		dc.w	$007FF
		dc.w	$003FF
		dc.w	$001FF
		dc.w	$000FF
		dc.w	$0007F
		dc.w	$0003F
		dc.w	$0001F
		dc.w	$0000F
		dc.w	$00007
		dc.w	$00003
		dc.w	$00001

	END
