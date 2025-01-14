**
**	$Id: font.asm,v 39.6 92/09/17 15:12:40 spence Exp $
**
**      font and text attribute specification
**
**      (C) Copyright 1985-1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		graphics

**	Includes

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/interrupts.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ables.i"
	INCLUDE		"exec/alerts.i"
	INCLUDE		"exec/macros.i"

	INCLUDE		"/gfxbase.i"
	INCLUDE		"/rastport.i"

	INCLUDE		"utility/tagitem.i"
	INCLUDE		"diskfont/diskfonttag.i"

	INCLUDE		"text.i"

	INCLUDE		"txtdata.i"
	INCLUDE		"macros.i"


**	Exports

	XDEF		_OpenFont
	XDEF		_CloseFont
	XDEF		_AddFont
	XDEF		_RemFont
	XDEF		_AskFont
	XDEF		_SetFont
	XDEF		_AskSoftStyle
	XDEF		_SetSoftStyle
	XDEF		_FontExtent
	XDEF		_ExtendFont
	XDEF		_StripFont
	XDEF		_WeighTAMatch


**	Imports

	XLVO	Alert			; Exec
	XLVO	AllocMem		;
	XLVO	AllocVec		;
	XLVO	Enqueue			;
	XLVO	FindName		;
	XLVO	FreeMem			;
	XLVO	FreeVec			;
	XLVO	Permit			;
	XLVO	ReplyMsg		;

	XLVO	AskSoftStyle		; Graphics
	XLVO	ExtendFont		;
	XLVO	StripFont		;
	XLVO	WeighTAMatch		;

	XLVO	GetTagData		; Utility
	XLVO	UDivMod32		;
	XLVO	UMult32			;


**	Assumptions
	IFNE	tfe_MatchWord
	FAIL	"tfe_MatchWord not zero, recode"
	ENDC


******* graphics.library/OpenFont ************************************
*
*   NAME
*	OpenFont -- Get a pointer to a system font.
*
*   SYNOPSIS
*	font = OpenFont(textAttr)
*	D0              A0
*
*	struct TextFont *OpenFont(struct TextAttr *);
*
*   FUNCTION
*	This function searches the system font space for the graphics
*	text font that best matches the attributes specified.  The
*	pointer to the font returned can be used in subsequent
*	SetFont and CloseFont calls.  It is important to match this
*	call with a corresponding CloseFont call for effective
*	management of ram fonts.
*
*   INPUTS
*	textAttr - a TextAttr or TTextAttr structure that describes the
*	           text font attributes desired.
*
*   RESULT
*	font is zero if the desired font cannot be found.  If the named
*	font is found, but the size and style specified are not
*	available, a font with the nearest attributes is returned.
*
*   BUGS
*	Prior to V39 this function would return a TextFont pointer
*	for any font which matched exactly in Y size, regardless of
*	differences in DPI, or DotSize.
*
*	As part of fixing this bug it is REQUIRED that you use pass the
*	same TextAttr (or TTextAttr) to this function that was used when
*	OpenDiskFont() was called.
*
*	OpenFont(), and OpenDiskFont() use WeighTAMatch() to measure
*	how well two fonts match.  WeightTAMatch() was a public function
*	in graphics.library V36-V37; it is now a system PRIVATE function
*	as of V39.
*
*   SEE ALSO
*	CloseFont()  SetFont()
*	diskfont.library/OpenDiskFont  graphics/text.h
*	intuition/intuition.h
*
**********************************************************************
*
*   REGISTERS
*	d0
*	d1	font style and flags
*	d2	font and text attribute style and flags xored
*	d3	working weight
*	d4	best weight so far
*	d5	bit mask
*	d6	longword index

*	a0
*	a1
*	a2	
*	a3	font under consideration (name matches)
*	a4	font with best weight so far
*	a5	textAttr parameter

_OpenFont:
		movem.l d2-d6/a2-a5,-(a7)
		move.l	a0,a5

		move.l	gb_ExecBase(a6),a0
		FORBID	a0,NOFETCH

		move.l	(a5),a1		; ta_Name(a5) get name

		lea	gb_TextFonts(a6),a0
		LINKEXE	FindName

*	    ;------ check if name exists

		tst.l	d0
		beq.s	openRts
		moveq	#0,d4		; no previous best match

testMatch:
		move.l	d0,a3

		move.l	d0,a0
		suba.l	a1,a1
		JSRLIB	ExtendFont
		tst.l	d0
		beq.s	noBetter

		move.l	a5,a0
		lea	tf_YSize-ta_YSize(a3),a1
		move.l	tf_Extension(a3),a2
		move.l	tfe_Tags(a2),a2
		JSRLIB	WeighTAMatch

	    ;-- Check private return to see if caller asked for
	    ;-- DPI or DotSize (means they are Tag Aware)

		;--	check for request aspect
		tst.w	d1
		bne.s	opf_DPISpecified

	    ;------ otherwise only accept match of a font which has
	    ;------ not been marked a duplicate.

		move.l	tf_Extension(a3),a0
		btst	#TE0B_DUPLICATE,tfe_Flags0(a0)
		beq.s	opf_DPISpecified	; acceptable

		moveq	#00,d0			; not acceptable


opf_DPISpecified:

		cmp.w	#MAXFONTMATCHWEIGHT,d0
		bne.s	notPerfect

		move.l	a3,a4
		bra.s	gotBest

notPerfect:
		cmp.w	d0,d4		; compare with best weight
		bge.s	noBetter	;   not bigger, already have best
		move.w	d0,d4
		move.l	a3,a4

noBetter:
	    ;------ check for any more of this name
		move.l	a3,a0		; start with this font
		move.l	(a5),a1		; ta_Name(a5) get name
		LINKEXE FindName

		tst.l	d0
		bne.s	testMatch

		move.l	d4,d0		; check if acceptable font
		beq.s	openRts

gotBest:
		addq.w	#1,tf_Accessors(a4)
		move.l	a4,d0


openRts:
		move.l	gb_ExecBase(a6),a0
		PERMIT	a0,NOFETCH

		movem.l (a7)+,d2-d6/a2-a5
		rts


******* graphics.library/CloseFont ***************************************
*
*   NAME
*	CloseFont -- Release a pointer to a system font.
*
*   SYNOPSIS
*	CloseFont(font)
*	          A1
*
*	void CloseFont(struct TextFont *);
*
*   FUNCTION
*	This function indicates that the font specified is no longer
*	in use.  It is used to close a font opened by OpenFont, so
*	that fonts that are no longer in use do not consume system
*	resources.
*	
*   INPUTS
*	font -	a font pointer as returned by OpenFont() or OpenDiskFont()
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	OpenFont()  diskfont.library/OpenDiskFont  graphics/text.h
*
**************************************************************************
_CloseFont:
		subq.w	#1,tf_Accessors(a1)
		bgt.s	cfRts
		btst	#FPB_REMOVED,tf_Flags(a1)
		bne.s	rfClosedRemove

cfRts:
		rts


******* graphics.library/AddFont *************************************
*
*   NAME
*	AddFont -- add a font to the system list
*
*   SYNOPSIS
*	AddFont(textFont)
*	        A1
*
*	void AddFont(struct TextFont *);
*
*   FUNCTION
*	This function adds the text font to the system, making it
*	available for use by any application.  The font added must be
*	in public memory, and remain until successfully removed.
*
*   INPUTS
*	textFont - a TextFont structure in public ram.
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	SetFont()  RemFont()  graphics/text.h
*
**********************************************************************
_AddFont:
		bclr.b	#FPB_REMOVED,tf_Flags(a1)
		clr.w	tf_Accessors(a1)
		move.b	#NT_FONT,LN_TYPE(a1)
		lea	gb_TextFonts(a6),a0
		move.l	a6,-(a7)
		move.l	gb_ExecBase(a6),a6
		FORBID
		CALLLVO	Enqueue
		CALLLVO	Permit
		move.l	(a7)+,a6
		rts


******* graphics.library/RemFont *************************************
*
*   NAME
*	RemFont -- Remove a font from the system list.
*
*   SYNOPSIS
*	RemFont(textFont)
*	        A1
*
*	void RemFont(struct TextFont *);
*
*   FUNCTION
*	This function removes a font from the system, ensuring that
*	access to it is restricted to those applications that
*	currently have an active pointer to it: i.e. no new SetFont
*	requests to this font are satisfied.
*
*   INPUTS
*	textFont - the TextFont structure to remove.
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	SetFont()  AddFont()  graphics/text.h
*
**********************************************************************
_RemFont:
		;-- check quickly if this font has been compiled yet
		move.l	tf_Extension(a1),d1
		beq.s	rfRemoveOK
		move.l	d1,a0
		cmp.w	#TE_MATCHWORD,(a0)	; tfe_MatchWord(a0)
		bne.s	rfRemoveOK
		cmp.l	tfe_BackPtr(a0),a1
		bne.s	rfRemoveOK
		btst	#TE0B_NOREMFONT,tfe_Flags0(a0)
		bne.s	rfRts

rfRemoveOK:
		bset	#FPB_REMOVED,tf_Flags(a1)
		bne.s	rfRedundant

		move.b	#NT_MESSAGE,LN_TYPE(a1)

		move.l	gb_ExecBase(a6),a0
		FORBID	a0,NOFETCH

		;--	REMOVE w/o destroying a1
		move.l	(a1),d0
		move.l	LN_PRED(a1),a0
		move.l	d0,(a0)
		exg	d0,a0
		move.l	d0,LN_PRED(a0)

		move.l	gb_ExecBase(a6),a0
		PERMIT	a0,NOFETCH

rfRedundant:
		tst.w	tf_Accessors(a1)
		bgt.s	rfRts

rfClosedRemove:
		move.l	a1,-(a7)
		move.l	a1,a0
		CALLLVO	StripFont
		move.l	(a7)+,a1

		LINKEXE	ReplyMsg	; reply font

rfRts:
		rts


******* graphics.library/AskFont *************************************
*
*   NAME
*	AskFont -- get the text attributes of the current font
*
*   SYNOPSIS
*	AskFont(rp, textAttr)
*	        A1  A0
*
*	void AskFont(struct RastPort *, struct TextAttr *);
*
*   FUNCTION
*	This function fills the text attributes structure with the
*	attributes of the current font in the RastPort.
*
*   INPUTS
*	rp       - the RastPort from which the text attributes are
*	           extracted
*	textAttr - the TextAttr structure to be filled.  Note that
*	           there is no support for a TTextAttr.
*
*   RESULT
*	The textAttr structure is filled with the RastPort's text
*	attributes.
*
*   BUGS
*
*   SEE ALSO
*	graphics/text.h
*
**********************************************************************
_AskFont:
		move.l	rp_Font(a1),a1
		move.l	LN_NAME(a1),ta_Name(a0)
		;------ copy over longword attributes
		move.l	tf_YSize(a1),ta_YSize(a0)
		rts


******* graphics.library/SetFont ******************************************
*
*   NAME
*	SetFont -- Set the text font and attributes in a RastPort.
*
*   SYNOPSIS
*	SetFont(rp, font)
*	        A1   A0
*
*	void SetFont(struct RastPort *, struct TextFont *);
*
*   FUNCTION
*	This function sets the font in the RastPort to that described
*	by font, and updates the text attributes to reflect that
*	change.  This function clears the effect of any previous
*	soft styles.
*
*   INPUTS
*	rp   - the RastPort in which the text attributes are to be changed
*	font - pointer to a TextFont structure returned from OpenFont()
*	       or OpenDiskFont()
*
*   RESULT
*
*   NOTES
*	This function had previously been documented that it would
*	accept a null font.  This practice is discouraged.
*	o   Use of a RastPort with a null font with text routines has
*	    always been incorrect and risked the guru.
*	o   Keeping an obsolete font pointer in the RastPort is no more
*	    dangerous than keeping a zero one there.
*	o   SetFont(rp, 0) causes spurious low memory accesses under
*	    some system software releases.
*
*	As of V36, the following Amiga font variants are no longer
*	directly supported:
*	    fonts with NULL tf_CharSpace and non-NULL tf_CharKern.
*	    fonts with non-NULL tf_CharSpace and NULL tf_CharKern.
*	    fonts with NULL tf_CharSpace and NULL tf_CharKern with
*		a tf_CharLoc size component greater than tf_XSize.
*	Attempts to SetFont these one of these font variants will
*	cause the system to modify your font to make it acceptable.
*
*   BUGS
*	Calling SetFont() on in-code TextFonts (ie fonts not
*	OpenFont()ed) will result in a loss of 24 bytes from
*	the system as of V36.
*	This can be resolved by calling StripFont().
*
*   SEE ALSO
*	OpenFont()  StripFont()
*	diskfont.library/OpenDiskFont()  graphics/text.h
*
**********************************************************************
_SetFont:
		move.l	a1,d0
		beq.s	_SetFont.	; spence - Jan 18 1991

		movem.l	a2-a3,-(a7)
		move.l	a0,a2		; save font
		move.l	a1,a3		; save rp

		move.l	a0,d0		; tst.l	a0
		beq.s	sfNullFont


		suba.l	a1,a1
		CALLLVO	ExtendFont
		tst.l	d0
		beq.s	sfDone

		move.w	tf_YSize(a2),rp_TxHeight(a3)
		move.l	tf_XSize(a2),rp_TxWidth(a3)	; and Baseline

sfNullEntry:
		clr.b	rp_AlgoStyle(a3)
		move.l	a2,rp_Font(a3)

sfDone:
		movem.l	(a7)+,a2-a3
_SetFont.
		rts

sfNullFont:
		clr.w	rp_TxHeight(a3)
		clr.l	rp_TxWidth(a3)			; and Baseline
		bra.s	sfNullEntry


******* graphics.library/AskSoftStyle ********************************
*
*   NAME
*	AskSoftStyle -- Get the soft style bits of the current font.
*
*   SYNOPSIS
*	enable = AskSoftStyle(rp)
*	D0                    A1
*
*	ULONG AskSoftStyle(struct RastPort *);
*
*   FUNCTION
*	This function returns those style bits of the current font
*	that are not intrinsic in the font itself, but
*	algorithmically generated.  These are the bits that are
*	valid to set in the enable mask for SetSoftStyle().
*
*   INPUTS
*	rp - the RastPort from which the font and style	are extracted.
*
*   RESULTS
*	enable - those bits in the style algorithmically generated.
*	         Style bits that are not defined are also set.
*
*   BUGS
*
*   SEE ALSO
*	SetSoftStyle()  graphics/text.h
*
**********************************************************************
_AskSoftStyle:
		move.l	rp_Font(a1),d0
		beq.s	assRts1			* no Font - just assume nothing is algorithmic - spence Jan  2 1991
		move.l	d0,a0
		moveq	#0,d0
		move.b	tf_Style(a0),d0
		btst	#FSB_UNDERLINED,d0
		bne.s	assRts
		move.b	tf_Baseline(a0),d1
		addq	#2,d1
		cmp.w	tf_YSize(a0),d1
		ble.s	assRts
		bset	#FSB_UNDERLINED,d0
assRts:
		not.l	d0
assRts1:
		rts


******* graphics.library/SetSoftStyle ********************************
*
*   NAME
*	SetSoftStyle -- Set the soft style of the current font.
*
*   SYNOPSIS
*	newStyle = SetSoftStyle(rp, style, enable)
*	D0                      A1  D0     D1
*
*	ULONG SetSoftStyle(struct RastPort *, ULONG, ULONG);
*
*   FUNCTION
*	This function alters the soft style of the current font.  Only
*	those bits that are also set in enable are affected.  The
*	resulting style is returned, since some style request changes
*	will not be honored when the implicit style of the font
*	precludes changing them.
*
*   INPUTS
*	rp     - the RastPort from which the font and style
*	         are extracted.
*	style  - the new font style to set, subject to enable.
*	enable - those bits in style to be changed.  Any set bits here
*	         that would not be set as a result of AskSoftStyle will
*	         be ignored, and the newStyle result will not be as
*	         expected. 
*
*   RESULTS
*	newStyle - the resulting style, both as a result of previous
*	           soft style selection, the effect of this function,
*	           and the style inherent in the set font.
*
*   BUGS
*
*   SEE ALSO
*	AskSoftStyle()  graphics/text.h
*
**********************************************************************
_SetSoftStyle:
		move.l	d2,-(a7)
		move.l	rp_Font(a1),d2
		beq.s	nofont			* force resulting style to 0 - spence Jan  2 1991
		moveq	#0,d2
		movem.l	d0-d1/a1,-(a7)
		CALLLVO	AskSoftStyle
		move.b	d0,d2
		movem.l	(a7)+,d0-d1/a1
		move.l	rp_Font(a1),a0
		and.b	d2,d1
		move.b	rp_AlgoStyle(a1),d2
		and.b	d1,d0
		not.b	d1
		and.b	d1,d2
		or.b	d0,d2
		move.b	d2,rp_AlgoStyle(a1)
		or.b	tf_Style(a0),d2
nofont:
		move.l	d2,d0

		move.l	(a7)+,d2
		rts


******* graphics.library/FontExtent **********************************
*
*   NAME
*	FontExtent -- get the font attributes of the current font (V36)
*
*   SYNOPSIS
*	FontExtent(font, fontExtent)
*	           A0    A1
*
*	void FontExtent(struct TextFont *, struct TextExtent *);
*
*   FUNCTION
*	This function fills the text extent structure with a bounding
*	(i.e. maximum) extent for the characters in the specified font.
*
*   INPUTS
*	font       - the TextFont from which the font metrics are extracted.
*	fontExtent - the TextExtent structure to be filled.
*
*   RESULT
*	fontExtent is filled.
*
*   NOTES
*	The TextFont, not the RastPort, is specified -- unlike
*	TextExtent(), effect of algorithmic enhancements is not
*	included, nor does te_Width include any effect of
*	rp_TxSpacing.  The returned te_Width will be negative only
*	when FPF_REVPATH is set in the tf_Flags of the font -- the
*	effect of left-moving characters is ignored for the width of
*	a normal font, and the effect of right-moving characters is
*	ignored if a REVPATH font.  These characters will, however,
*	be reflected in the bounding extent.
*
*   SEE ALSO
*	TextExtent()  graphics/text.h
*
**********************************************************************
_FontExtent:
		movem.l	d2-d5/a2-a4,-(a7)

		move.l	tf_CharLoc(a0),a2
		move.l	tf_CharSpace(a0),a3
		move.l	tf_CharKern(a0),a4
		addq.l	#2,a2
		moveq	#0,d3		; CP bound
		move.l	a4,d4		; bit min
		beq.s	feInitBitMax	; is zero if no kern
		move.w	#32767,d4	; bit min
feInitBitMax:
		move.w	#-32768,d5	; bit max

		;-- get character loop counter
		clr.w	d0
		move.b	tf_HiChar(a0),d0
		sub.b	tf_LoChar(a0),d0

		;-- bounding loop
feLoop:
		move.l	a4,d1		; tst.l a4 & clr.l d1 if zero
		beq.s	feAddBitSize
		move.w	(a4)+,d1	; Kern
		cmp.w	d1,d4		; min
		ble.s	feAddBitSize
		move.w	d1,d4
feAddBitSize:
		move.w	d1,d2		; save kern for CP bound later
		add.w	(a2),d1		; Loc bit size
		addq.l	#4,a2
		cmp.w	d1,d5		; max
		bge.s	feBoundCP
		move.w	d1,d5
feBoundCP:
		move.l	a3,d1
		beq.s	feConstantSpace
		add.w	(a3)+,d2	; Space
		bra.s	feDirection
feConstantSpace:
		add.w	tf_XSize(a0),d2
feDirection:
		btst	#FPB_REVPATH,tf_Flags(a0)
		bne.s	feRevPath
		cmp.w	d2,d3
		bge.s	feNextChar
		move.w	d2,d3
		bra.s	feNextChar
feRevPath:
		cmp.w	d2,d3
		ble.s	feNextChar
		move.w	d2,d3
feNextChar:
		dbf	d0,feLoop

		move.w	d3,te_Width(a1)
		move.w	d4,te_Extent+ra_MinX(a1)
		subq.w	#1,d5
		move.w	d5,te_Extent+ra_MaxX(a1)
		move.w	tf_YSize(a0),d0
		move.w	d0,te_Height(a1)
		move.w	tf_Baseline(a0),d1
		neg.w	d1
		move.w	d1,te_Extent+ra_MinY(a1)
		add.w	d0,d1
		subq.w	#1,d1
		move.w	d1,te_Extent+ra_MaxY(a1)

		movem.l	(a7)+,d2-d5/a2-a4
		rts


******* graphics.library/ExtendFont *******************************************
*
*   NAME
*	ExtendFont -- ensure tf_Extension has been built for a font (V36)
*
*   SYNOPSIS
*	success = ExtendFont(font, fontTags)
*	D0                   A0    A1
*
*	ULONG ExtendFont(struct TextFont *, struct TagItem *);
*
*	success = ExtendFontTags(font, Tag1, ...)  (V39)
*
*	ULONG ExtendFontTags(struct TextFont *, ULONG, ...);
*
*   FUNCTION
*	To extend a TextFont structure.
*
*   INPUTS
*	font - The font to extend.
*	fontTags - An optional taglist.  If NULL, then a default is used.
*	           Currently, the only tag defined is TA_DeviceDPI.
*
*   RESULT
*	success - 1 if the TextFont was properly extended, else 0.
*
*   NOTES
*	The varargs stub was missing from amiga.lib until V39.
*
*   SEE ALSO
*	graphics/text.h
*
*******************************************************************************
_ExtendFont:
		;-- check quickly if this font has been compiled yet
		move.l	a1,d0
		move.l	tf_Extension(a0),d1
		beq.s	efUncompiled
		move.l	d1,a1
		cmp.w	#TE_MATCHWORD,(a1)	; tfe_MatchWord(a1)
		bne.s	efUncompiled
		cmp.l	tfe_BackPtr(a1),a0
		bne.s	efUncompiled
		moveq	#1,d0
		rts


efDefaultTags:
		dc.l	0
		dc.l	efTallDotTags
		dc.l	efWideDotTags
		dc.l	0

efTallDotTags:
		dc.l	TA_DeviceDPI,(100<<16)!50
		dc.l	TAG_DONE

efWideDotTags:
		dc.l	TA_DeviceDPI,(50<<16)!100
		dc.l	TAG_DONE


;
;   d0	temporary, e.g. origin
;   d1	temporary, e.g. size
;   d2	temporary, e.g. fontTags, #8, space
;   d3	kern
;   d4	decrementing character count
;   d5	tf_XSize
;   d6	tf_Modulo*8
;
;   a0	
;   a1	
;   a2	
;   a3	TextFont
;   a4	TextFontExtension
;
efUncompiled:
		movem.l	d2-d6/a2-a4,-(a7)
		move.l	a0,a3		; save font
		move.l	d0,d2		; and fontTags

		addq.w	#1,tf_Accessors(a3)	; ensure not expunged

		;-- get memory for TextFontExtension
		moveq	#tfe_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		LINKEXE	AllocMem

		tst.l	d0
		beq	efDone

		;-- initialize TextFontExtension
		move.l	d0,a4
		move.w	#TE_MATCHWORD,(a4)	; tfe_MatchWord(a4)
		move.l	a3,tfe_BackPtr(a4)
		move.l	tf_Extension(a3),tfe_OrigReplyPort(a4)
		move.l	d2,tfe_Tags(a4)
		bne.s	efCompileFlags0

		;-- use default tags if TALLDOT or WIDEDOT
		move.b	tf_Flags(a3),d0
		and.w	#FPF_TALLDOT!FPF_WIDEDOT,d0
		lsr.w	#FPB_TALLDOT-2,d0
		move.l	efDefaultTags(pc,d0.w),tfe_Tags(a4)

efCompileFlags0:
		moveq	#8,d2
		move.w	tf_XSize(a3),d5
		move.w	tf_Modulo(a3),d6
		lsl.w	#3,d6			; pixel width: origin bound
		;-- look to see if it's a byte font
		btst	#FPB_PROPORTIONAL,tf_Flags(a3)
		bne.s	efChkKernless
		btst	#FSB_COLORFONT,tf_Style(a3)
		bne.s	efChkKernless
		cmp.w	d2,d5
		bne.s	efChkKernless
		;--	check for byte fonts
		tst.l	tf_CharKern(a3)
		bne.s	efChkKernless
		tst.l	tf_CharSpace(a3)
		bne.s	efChkKernless

		;-- perform the longest bytefont test
		moveq	#0,d4
		move.b	tf_HiChar(a3),d4
		sub.b	tf_LoChar(a3),d4
		move.l	tf_CharLoc(a3),a0
efChkSize8:
		move.w	(a0)+,d0		; check for start location
		cmp.w	d6,d0			;   within strike font
		bge.s	efChkKernless
		andi.w	#7,d0			;   and byte offset
		bne.s	efChkKernless
		cmp.w	(a0)+,d2		; check for byte size
		dbne	d4,efChkSize8
		bne.s	efChkKernless

		;-- this is a bytefont (which is also kernless)
		move.b	#TE0F_KERNLESS!TE0F_BYTECELL,tfe_Flags0(a4)
		bra.s	efCompiled

	    ;-- look harder to see if it's a kernless font
efChkKernless:
		moveq	#0,d4
		move.b	tf_HiChar(a3),d4
		sub.b	tf_LoChar(a3),d4
		move.l	tf_CharLoc(a3),a0
		move.l	tf_CharKern(a3),d0
		beq.s	efNoKern
		move.l	tf_CharSpace(a3),d1
		beq	efOSpace

		;-- both
		move.l	d0,a1
		move.l	d1,a2
efBothLoop:
		move.w	(a0)+,d0	; origin
		move.w	(a0)+,d1	; size
		move.w	(a2)+,d2	; space
		move.w	(a1)+,d3	; kern
		bmi.s	efNotKernless	; negative kern
		add.w	d2,d3
		cmp.w	d3,d5
		bne.s	efNotKernless	; width != tf_XSize
		cmp.w	d1,d2
		blt.s	efNotKernless	; space < bit width
		tst.w	d1
		beq.s	efBothDBF	; no width, so don't care where
		add.w	d1,d0
		cmp.w	d6,d0
efBothDBF:
		dbgt	d4,efBothLoop
		ble.s	efKernless

		;--	bad offset but kernless so far
efBadKernlessOffset:
		tst.w	d4		; for undefined character?
;13Jun90    for EA's ShangHai's gene.font -- don't obsolete it.
;13Jun90	bne	efObsoleteFont
		bne.s	efKernless
		;--	patch bad undefined character by making it the same
		;	as the HiChar itself
		subq.b	#1,tf_HiChar(a3)
		bra.s	efKernless


		;-- no kern & ?
efNoKern:
		tst.l	tf_CharSpace(a3)
		bne.s	efOKern

		;-- neither
efNeitherLoop:
		move.w	(a0)+,d0	; origin
		move.w	(a0)+,d1	; size
		beq.s	efNeitherDBF	; no width, so don't care where
		cmp.w	d1,d5
		blt.s	efOKernSpace	; space < bit width
		add.w	d1,d0
		cmp.w	d6,d0
efNeitherDBF:
		dbgt	d4,efNeitherLoop
		bgt.s	efBadKernlessOffset

efKernless:
		move.b	#TE0F_KERNLESS,tfe_Flags0(a4)

efCompiled:
		move.l	a4,tf_Extension(a3)
**************  removed for 2.03 -- bart ****************************
*	
* the problem was that old programs are initializing textattr structs
* with the first n bytes of a textfont, to force it to remain open.
* however they were getting the tagged bit set in the style so that
* an improper parsing of non-existant tags was taking place.
*	
*		bset	#FSB_TAGGED,tf_Style(a3)
*	
**************  removed for 2.03 -- bart ****************************
		moveq	#1,d0

efDone:
		subq.w	#1,tf_Accessors(a3)

		movem.l	(a7)+,d2-d6/a2-a4
		rts


efnkLoop:
		move.w	(a0)+,d0
		move.w	(a0)+,d1
efNotKernless:
		tst.w	d1
		beq.s	efnkDBF		; no width, so don't care where
		add.w	d1,d0
		cmp.w	d6,d0
efnkDBF:
		dbgt	d4,efnkLoop
		ble.s	efCompiled

		tst.w	d4		; for undefined character?
;13Jun90    for EA's ShangHai's gene.font -- don't obsolete it.
;13Jun90	bne.s	efObsoleteFont
		bne.s	efCompiled
		;--	patch bad undefined character by making it the same
		;	as the HiChar itself
		subq.b	#1,tf_HiChar(a3)
		bra.s	efCompiled


efOKernSpace:
		bsr.s	efOAllocVec
		move.l	a0,tfe_OFontPatchK(a4)
		move.l	a0,tf_CharKern(a3)
		;--	default kern is zero so no initialization needed

efOSpace:
		bsr.s	efOAllocVec
		move.l	a0,tfe_OFontPatchS(a4)
		move.l	a0,tf_CharSpace(a3)
		move.w	d4,d1
efOBuildSpace:
		move.w	d5,(a0)+
		dbf	d1,efOBuildSpace
		bra.s	efOPatched

efOKern:
		bsr.s	efOAllocVec
		move.l	a0,tfe_OFontPatchK(a4)
		move.l	a0,tf_CharKern(a3)
		;--	default kern is zero so no initialization needed

efOPatched:
		move.l	tf_CharLoc(a3),a0
		move.l	tf_CharKern(a3),a1
		move.l	tf_CharSpace(a3),a2
		bra	efBothLoop

efOAllocVec:
		moveq	#0,d4
		move.b	tf_HiChar(a3),d4
		sub.b	tf_LoChar(a3),d4
		move.l	d4,d0
		addq.l	#1,d0
		add.l	d0,d0
		move.l	#MEMF_CLEAR,d1
		LINKEXE	AllocVec
		move.l	d0,a0
		move.l	(a7)+,a1
		tst.l	d0
		beq.s	efObsoleteFont
		jmp	(a1)

		;-- font is obsolete and unpatchable
efObsoleteFont:
		movem.l	d7/a6,-(a7)
		move.l	#AN_ObsoleteFont,d7
		move.l	gb_ExecBase(a6),a6
		CALLLVO	Alert
		move.l	tfe_OFontPatchS(a4),a1
		CALLLVO	FreeVec
		move.l	tfe_OFontPatchK(a4),a1
		CALLLVO	FreeVec
		move.l	a4,a1
		moveq	#tfe_SIZEOF,d0
		CALLLVO	FreeMem
		movem.l	(a7)+,d7/a6
		moveq	#0,d0
		bra	efDone


******* graphics.library/StripFont ***********************************
*
*   NAME
*	StripFont -- remove the tf_Extension from a font (V36)
*
*   SYNOPSIS
*	StripFont(font)
*	          A0
*
*	VOID StripFont(struct TextFont *);
*
**********************************************************************
_StripFont:
		;-- check if this font has been compiled
		move.l	tf_Extension(a0),d1
		beq.s	sfRts
		move.l	d1,a1
		cmp.w	#TE_MATCHWORD,(a1)	; tfe_MatchWord(a1)
		bne.s	sfRts
		cmp.l	tfe_BackPtr(a1),a0
		bne.s	sfRts

		movem.l	a2/a3/a6,-(a7)
		move.l	a0,a2
		move.l	a1,a3
		move.l	gb_ExecBase(a6),a6
		;--	put original reply port back in font message
		move.l	tfe_OrigReplyPort(a3),tf_Extension(a2)
		move.l	tfe_OFontPatchS(a3),d0
		beq.s	sfUnPatchK
		clr.l	tf_CharSpace(a2)
		move.l	d0,a1
		CALLLVO	FreeVec
sfUnPatchK:
		move.l	tfe_OFontPatchK(a3),d0
		beq.s	sfFreeTFE
		clr.l	tf_CharKern(a2)
		move.l	d0,a1
		CALLLVO	FreeVec
sfFreeTFE:
		move.l	a3,a1
		moveq	#tfe_SIZEOF,d0
		CALLLVO	FreeMem
		movem.l	(a7)+,a2/a3/a6

sfRts:
		rts


*****i* graphics.library/WeighTAMatch ********************************
*
*   NAME
*	WeighTAMatch -- Get a measure of how well two fonts match. (V36)
*
*   SYNOPSIS
*	weight = WeighTAMatch(reqTextAttr, targetTextAttr, targetTags)
*	D0                    A0           A1              A2
*	D1
*
*	WORD WeighTAMatch(struct TTextAttr *, struct TextAttr *,
*	     struct TagItem *);
*
*	weight = WeighTAMatchTags(reqTextAttr, targetTextAttr, Tag1, ...) (V39)
*
*	WORD WeighTAMatchTags(struct TTextAttr *,struct TextAttr *, ULONG, ...)
*
*   FUNCTION
*	This function provides a metric to describe how well two fonts
*	match.  This metric ranges from MAXFONTMATCHWEIGHT (perfect match)
*	through lower positive numbers to zero (unsuitable match).
*
*   INPUTS
*	reqTextAttr    - the text attributes requested.
*	targetTextAttr - the text attributes of a potential match.
*	targetTags     - tags describing the extended target attributes, or
*	                 zero if not available.
*
*	The [t]ta_Name fields of the [T]TextAttr structures are not used.
*
*	The tags affect the weight only when both a) the reqTextAttr
*	has the FSF_TAGGED bit set in ta_Style, and b) targetTags is
*	not zero.  To fairly compare two different weights, the inclusion
*	or exclusion of tags in the weighing must be the same for both.
*
*   RESULTS
*	weight -- a positive weight describes suitable matches, in
*	      increasing desirability.  MAXFONTMATCHWEIGHT is a perfect
*	      match.  A zero weight is an unsuitable match.
*
*	D1 -- 1-DPI specified in request.  2-DotSize specified in request.
*	      3-DPI and DotSize specified in request.  0-Neither
*	      specified in request.  To be used as a TRUE/FALSE value
*	      for OpenFont() to decide if caller is TAG aware.
*
*   BUGS
*	Prior to V39, this function did not factor differences in
*	font DPI if the Y size of the reqTextAttr, and targetTextAttr
*	matched.  This function also failed to compared DPI if the
*	target did not have a TA_DeviceDPI tag.  As of V39, this function
*	assumes the target has a 1:1 DPI if the TA_DeviceDPI tag is
*	absent.   As of V39, this function will return an imperfect
*	match if the OT_DotSize tag of the source font does not exactly
*	match the request.
*
*	This function is now private; it is meant to be used by diskfont
*	and OpenFont() only.
*
*   SEE ALSO
*	OpenFont()
*
**********************************************************************
*
*   REGISTERS
*	d2	temp
*	d3	temp
*	d4	flag (00-no tags; 01-DPI specified; 02-DOT specified; 03-BOTH)
*	d5	DPI error

*	a0	passed in
*	a1	passed in
*	a2	passed in


WEIGHTSHIFT	EQU	5		; * 32
WEIGHTXMAX	EQU	(1<<WEIGHTSHIFT)

wfmGetDPI:
		beq.s	wfmDefaultTag	; test D1
		move.l	#TA_DeviceDPI,d3
wfmGetTag:

		movem.l	a0-a1/a6,-(a7)
		move.l	d1,a0
		move.l	d0,d1		; set default
		move.l	d3,d0		; tag to search for
		move.l	gb_UtilBase(a6),a6
		JSRLIB	GetTagData
		movem.l	(a7)+,a0-a1/a6
wfmDefaultTag:
		rts

wfmGetDOT:
		moveq	#00,d0		; set default
		tst.l	d1		; test D1
		beq.s	wfmDefaultTag
		move.l	#OT_DotSize,d3
		bra.s	wfmGetTag

_WeighTAMatch:
		movem.l	d2-d5,-(a7)

		;-- check for aspect effect

		moveq	#00,d5		; Set Default DPI error (no-error)

		moveq	#00,d4		; assume DPI/DOT tags not requested

		;--	check for target aspect
		move.l	#$00200020,d0	; default is 1:1
		move.l	a2,d1
		bsr.s	wfmGetDPI
		move.l	d0,d2

		;--	check for request aspect
		btst	#FSB_TAGGED,tta_Style(a0)
		beq.s	wfmGetYComponent
		moveq	#00,d0		; default is DPI tag not specified
		move.l	tta_Tags(a0),d1
		bmi.s	wfmGetYComponent	; kludge for LSE

		bsr.s	wfmGetDPI

		;--	calculate aspect correction

		move.l	d0,d1
		beq.s	wfmNoAspect	; No DPI specified?

		moveq	#01,d4		; DPI specified flag

		swap	d0
		mulu	d2,d0		; Req XDPI * Target YDPI
		beq.s	wfmNoAspect
		swap	d2
		mulu	d2,d1		; Req YDPI * Target XDPI
		beq.s	wfmNoAspect
		cmp.l	d0,d1

		;--	ratio MUST be <= 1:1
		;--	Divisor must be >= Dividend (BCC is unsigned BGE)

		bcc.s	wfmCalcAspectCorrection
		exg	d0,d1

wfmCalcAspectCorrection:

		;--     Calculate a DPI error value of <= a minimum
		;--	Y error.  A DPI error of 0 means no error, or
		;--	perfect match.  A DPI error of 1 means the
		;--	DPI of the TARGET and REQUEST are very close,
		;--	but not perfect.  A DPI error of WEIGHTMAX is
		;--     possible, but unlikely.  It requires comparing
		;--	a 1:1 DPI font with a 1:33, or 33:1 DPI font.
		;--	In actual use such distorted fonts are rare, and
		;--	we can assume that somewhere on the font list
		;--	a perfect match exists, so the calculated weight
		;--	for this comparison is essentially ignored.
		;
		;--     ratio of <= 1:1 * WEIGHTXMAX:1 MUST be <= WEIGHTXMAX
		;--

		lsl.l	#WEIGHTSHIFT,d0
		movem.l	a0-a1/a6,-(a7)
		move.l	gb_UtilBase(a6),a6
		JSRLIB	UDivMod32
		movem.l	(a7)+,a0-a1/a6

		;--	transform WEIGHTXMAX-0 into 0-WEIGHTXMAX; this value
		;--	is then subtracted from MAXFONTMATCHWEIGHT along with
		;--	some integer multiple of WEIGHTXMAX if there is any
		;--	difference in Y size.
		
		moveq	#WEIGHTXMAX,d5
		sub.l	d0,d5

wfmNoAspect:

		;--	Check OT_DotSize tag for diffs (FSF_TAGGED already
		;--	known to be TRUE)

		move.l	tta_Tags(a0),d1
		bsr.s	wfmGetDOT
		move.l	d0,d2
		beq.s	wfmGetYComponent

		addq.w	#02,d4		; OT_DotSize tag specified in request

		;--	OT_DotSize requested, see if the font has one, and
		;--	compare for exact match

		move.l	a2,d1
		bsr.s	wfmGetDOT
		
                cmp.l	d0,d2		; dotsize must match exactly
		beq.s	wfmGetYComponent

		addq.l	#1,d5		; increment error by fudge factor
					; to indicate imperfect match		
wfmGetYComponent:
		;-- get Y effect
		moveq	#0,d1
		move.w	ta_YSize(a0),d1
		ble.s	wfmFail		; if YSize <= 0 bail out here
		move.w	ta_YSize(a1),d2
		ble.s	wfmFail		; if YSize <= 0 bail out here

		sub.w	d2,d1		; req - target
		bpl.s	wfmApplyAspectCorrection ; target <= req?

		;	target is larger than req
		neg.w	d1
		lsl.l	#2,d1		; (e.g., -128, not -32, for every pixel larger)

wfmApplyAspectCorrection:

		lsl.l	#WEIGHTSHIFT,d1	; (Y diff * MAX) + 0-MAX for X diff
		add.l	d5,d1		

	; start with largest

		move.l	#MAXFONTMATCHWEIGHT,d0
		sub.l	d1,d0

		;-- involve weights for style & flags
		move.w	ta_Style(a0),d2	; and ta_Flags(a0)
		move.w	ta_Style(a1),d3	; and ta_Flags(a1)
		lea	wfmR1T0Weights(pc),a0
		lea	wfmR0T1Weights(pc),a1

		;--	ensure "designed" is set appropriately for target
		move.w	d3,d1
		and.w	#FPF_ROMFONT!FPF_DISKFONT,d1
		beq.s	wfmBitWeights

		or.w	#FPF_DESIGNED,d3

wfmBitWeights:
		moveq	#0,d1
		eor.w	d2,d3

		;--	loop while some bits differ
wfmBitLoop:
		beq.s	wfmDone
		bpl.s	wfmbNext
		tst.w	d2		; check current (MSB) of req
		bpl.s	wfmbR0T1

		;--	    set in request, clear in target
		move.w	(a0),d1
		bra.s	wfmbDiff

		;--	    clear in request, set in target
wfmbR0T1:
		move.w	(a1),d1
wfmbDiff:
		sub.l	d1,d0

wfmbNext:
		addq.l	#2,a0
		addq.l	#2,a1
		add.w	d2,d2
		add.w	d3,d3
		bra.s	wfmBitLoop

wfmDone:
		tst.l	d0
		bge.s	wfmRts

wfmFail:
		moveq	#0,d0		; collapse all unsuitable to 0

wfmRts:
		move.w	d4,d1		; return PRIVATE

		movem.l	(a7)+,d2-d5
		rts



;	range of numbers for size difference:
;	    y differences: 32..<8K would be nice>?
;
;			,color,,extended,italic,bold,underlined
;			,designed,propor,wide,tall,revpath,diskfont,romfont
wfmR1T0Weights:
		dc.w	0,0,0,0,0,16,8,4
		dc.w	0,MAXFONTMATCHWEIGHT,0,0,0,MAXFONTMATCHWEIGHT,0,0

wfmR0T1Weights:
		dc.w	0,0,0,0,0,1024,512,2048
		dc.w	0,0,0,0,0,MAXFONTMATCHWEIGHT,0,0


	END
