**
**	$Id: scaledfont.asm,v 36.20 92/01/30 11:02:23 davidj Exp $
**
**      diskfont.library font scaler
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION	diskfont

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"graphics/rastport.i"
	INCLUDE	"graphics/scale.i"
	INCLUDE	"graphics/text.i"
	INCLUDE	"graphics/view.i"

	INCLUDE	"macros.i"
	INCLUDE	"dfdata.i"
	INCLUDE	"diskfont.i"

	XDEF	_DFNewScaledDiskFont

	XLVO	AllocMem
	XLVO	CopyMem
	XLVO	FreeMem

	XLVO	BitMapScale
	XLVO	ExtendFont
	XLVO	ScalerDiv
	XLVO	WaitBlit

	XLVO	CloneTagItems
	XLVO	FreeTagItems
	XLVO	FindTagItem
	XLVO	GetTagData
	XLVO	NextTagItem
	XLVO	UMult32

**	Assumptions

	IFNE	cfc_Reserved
	FAIL	"cfc_Reserved not zero, recode"
	ENDC
	IFNE	cfc_Count-cfc_Reserved-2
	FAIL	"cfc_Count does not follow cfc_Reserved, recode"
	ENDC
	IFNE	cfc_ColorTable-cfc_Count-2
	FAIL	"cfc_ColorTable does not follow cfc_Count, recode"
	ENDC

******* diskfont.library/NewScaledDiskFont ***************************
*
*   NAME
*	NewScaledDiskFont -- Create a DiskFont scaled from another. (V36)
*
*   SYNOPSIS
*	header = NewScaledDiskFont(srcFont, destTextAttr)
*	D0                         A0       A1
*
*	struct DiskFontHeader *NewScaledDiskFont( struct TextFont *,
*		struct TTextAttr * );
*
*   INPUTS
*	srcFont - the font from which the scaled font is to be
*	    constructed.
*	destTextAttr - the desired attributes for the new scaled
*	    font.  This may be a structure of type TextAttr or
*	    TTextAttr.
*
*   RESULT
*	header - a pointer to a DiskFontHeader structure.  This is not
*		being managed by the diskfont.library, however.
*
*   NOTES
*	o   This function may use the blitter.
*	o   Fonts containing characters that render wholly outside
*	    the character advance cell are currently not scalable.
*	o   The font, and memory allocated for the scaled font can
*	    can be freed by calling StripFont() on the font,
*	    and then calling UnLoadSeg() on the segment created
*	    by this function.
*
*	    Both the TextFont structure, and segment pointer are contained
*	    within the DiskFontHeader struct.  The DiskFontHeader structure
*	    will also be freed as part of the UnLoadSeg() call.
*	    StripFont() is a new graphics.library call as of V36.
*
******* Font scaling strategy ****************************************
;
;   PROBLEM DESCRIPTION
;	Given an existing font 0 of height h0, construct a font 1 of
;	height h1 with character widths w1 according to the following:
;	1.  the width of each character is scaled by either the factor
;	    of nominal widths (wc1 = wc0 * (w1/w0)), or if the width
;	    is not specified (TextAttr), by the factor of the different
;	    heights (w1 = w0 * (h1/h0)).
;	2.  each character is scaled independently to assure that
;	    fixed width fonts remain fixed width.
;
;   IMPLEMENTATION
;	Fonts are scaled first in x, then in y.
;
;	The m and n terms for x scaling are derived from the font
;	heights.  A multiplicative width modification is applied
;	to the destination height, a divisitive modification is
;	applied as a multiplication to the source height.
;
;	Coalesced pixels use the first pixel as their value.  For
;	normal text, it might be preferrable to OR them together, but
;	this creates problems for color text: pen numbers outside the
;	original range.
;
;	Before a character is scaled, it's CharLoc0 is compared with
;	previous CharLoc0's, and, if an identical one is found, the
;	corresponding CharLoc1 is used for this character.  Note that
;	this may re-order the storage releationship between the font
;	data and character values, and that this breaks some text
;	speed enhancers (for byte-wide characters).
;
;	The blitter is used to create the final font data from the
;	intermediate results.
;
;---------------------------------------------------------------------


 STRUCTURE	DiskFontSegment,0
    LONG	dfs_AllocationBytes
    LONG	dfs_NextSegment
    LONG	dfs_Reserved
    LABEL	dfs_DFH

 STRUCTURE	ScaledFontVariables,0
    STRUCT	sfv_DPITag,8+8		; tag list for scaled font DPI
    APTR	sfv_TagClone		; clone of original font tag list
    STRUCT	sfv_BSA,bsa_SIZEOF	; BitMapScale argument structure
    ULONG	sfv_CCDSize		; size of chip version of srcFont
    APTR	sfv_CCDAddr		;   CharData, and address
    STRUCT	sfv_SBM,bm_SIZEOF	; source bit map for scaling
    STRUCT	sfv_DBM,bm_SIZEOF	; destination bit map for scaling
    LABEL	sfv_SIZEOF		; 48+4+4+40+40

;
;   d7	YSize
;   a3	BitScaleArgs
;   a4	destFont
;   a5	srcFont
;
_DFNewScaledDiskFont:

		movem.l	d2-d7/a2-a6,-(a7)

		move.l	a0,a5			; save srcFont
		move.l	a1,a2			; save destTextAttr for a while

		;-- ensure tf_Extension
		suba.l	a1,a1
		LINKGFX	ExtendFont
		tst.l	d0
		beq	sfFail1

		;-- allocate the local variables
		move.l	#sfv_SIZEOF,d0
		move.l	#MEMF_CLEAR,d1
		LINKEXE	AllocMem
		tst.l	d0
		beq	sfFail1

		move.l	d0,a3

	    ;-- initialize the BitScaleArgs
		move.w	tf_YSize(a5),d6		; source YSize temporary
		move.w	ta_YSize(a2),d7		; save dest YSize

		;-- check for aspect ratio information
		btst	#FSB_TAGGED,ta_Style(a2)
		beq	noSpecifiedWidth

		;--	look in TTextAttr
		move.l	#TA_DeviceDPI,d0
		moveq	#0,d1
		move.l	tta_Tags(a2),a0
		LINKUTL	GetTagData
		move.l	d0,d3			; save attrDPI
		beq.s	noSpecifiedWidth

		move.l	#TA_DeviceDPI,sfv_DPITag(a3)
		move.l	d0,sfv_DPITag+4(a3)	; DPI of resultant font

		;--	look in Font
		move.l	tf_Extension(a5),a0
		move.l	tfe_Tags(a0),a0
		move.l	#TA_DeviceDPI,d0
		move.l	#$00320032,d1		; 50:50 DPI
		LINKUTL	GetTagData
		move.l	d0,d4			; save fontDPI


		;--
		;
		;	src:			dest:
		;
		;	fontY			attrY
		;	----- * fontDPIX	----- * attrDPIX
		;	fontDPIY		attrDPIY
		;
		;	fontY * fontDPIX * attrDPIY
		;				attrY * attrDPIX * fontDPIY
		;

		clr.w	d0			; fontDPI
		swap	d0			;   fontDPIX
		move.l	d3,d1			; attrDPIY
		mulu	d6,d1			;   * fontY
		bsr	_LMul			;   * fontDPIX

		movem.l	d0/d1,-(sp)		; save src (64 bits)

		move.w	d4,d0			; fontDPIY
		mulu	d7,d0			;   * attrY
		move.l	d3,d1			; (attrDPI)
		clr.w	d1			;
		swap	d1			;   (attrDPIX)
		bsr	_LMul			;   * attrDPIX


	; Reduce Src:Dest ratio until BOTH 64 bit values fit in 1 (one)
	; word (14 bits   (signed integer size - 1 bit))
	;
	; ARGHH!! The bit scaler is broken in V37, and math overflows
	; if shrinking X using values > 8192!!!!
	;
	; OR 64 bit values together - this gives us a mask which we will
	; use to determine the most signficant bit in both 64 bit values
	;

		movem.l	(sp)+,d2/d3

		move.l	d0,d4
		move.l	d1,d5

		or.l	d2,d4			;Mask bits 31-0
		or.l	d3,d5			;Mask bits 63-32

	; Shift (divide by 2) 64 bit values until they BOTH fit in
	; one (1) long word
	;
	; Fast path through for smaller ratios
	;
	;	Use registers for speed here, though note this code
	;	is only executed once.  Assumes typical case of
	;	less precision (no tags specified, or less precise
	;	ratio in tags with reasonable size fonts).
	;


doULShift:
		tst.l	d5
		beq.s	doneULShift

		lsr.l	#1,d5			; OR Mask >> 1
		roxr.l	#1,d4

		lsr.l	#1,d1			; Dest >> 1
		roxr.l	#1,d0

		lsr.l	#1,d3			; Src >> 1
		roxr.l	#1,d2
		bra.s	doULShift

doneULShift:

	; Shift (divide by 2) 32 bit values until both fit in 14 bits
	;
	; Fast path through for smaller ratios
	;
	;
		move.l	#$ffffC000,d3	; mask bits 63->14
	;
	;	significant bits left in 13-0
	;

doLLShift:
		move.l	d4,d1
		and.l	d3,d1			; while any of bits 31-14
		beq.s	checkEqualFactors	; are true, divide by 2

		lsr.l	#1,d4			; OR Mask >> 1
		lsr.l	#1,d0			; Dest >> 1
		lsr.l	#1,d2			; Src >> 1
		bra.s	doLLShift


noSpecifiedWidth:

	    ;-- Default (same factors for X as for Y)

		move.w	d6,d2
		move.w	d7,d0


	; Special case - BitMapScale breaks with a perfect 1:1
	; X ratio (special blitter based code is used which can
	; fail with some destination, and source offsets.


checkEqualFactors:

	;
		cmp.l	d2,d0
		bne.s	setXFactors

		move.w	#$2000,d0		; Set ratio of almost 1
		move.w	d0,d2
		subq.w	#1,d2

setXFactors:

		move.w	d2,sfv_BSA+bsa_XSrcFactor(a3)
		move.w	d0,sfv_BSA+bsa_XDestFactor(a3)

setYFactors:
		move.w	d6,sfv_BSA+bsa_YSrcFactor(a3)
		move.w	d7,sfv_BSA+bsa_YDestFactor(a3)
		lea	sfv_SBM(a3),a0
		move.l	a0,sfv_BSA+bsa_SrcBitMap(a3)
		lea	sfv_DBM(a3),a0
		move.l	a0,sfv_BSA+bsa_DestBitMap(a3)
		move.w	tf_YSize(a5),sfv_BSA+bsa_SrcHeight(a3)

	    ;-- create a header for the new font
		suba.l	a4,a4			; show no font yet
		;-- get base font header size
		moveq	#dfh_SIZEOF+dfs_DFH,d0
		move.w	d0,d2
		btst	#FSB_COLORFONT,tf_Style(a5)
		beq.s	stdFont
		tst.b	ctf_Depth(a5)
		beq	sfFail2
		add.w	#ctf_SIZEOF-tf_SIZEOF,d0
		move.w	d0,d2
		;	check if ctf_ColorFontColors is of a known type
		move.l	ctf_ColorFontColors(a5),d1
		beq.s	stdFont
		move.l	d1,a0		; lea cfc_Reserved(d1),a0
		tst.w	(a0)+
		bne.s	stdFont
		;	append space for the color map
		move.w	(a0),d1		; cfc_Count(a0)
		add.w	d1,d1
		addq.w	#cfc_SIZEOF,d1
		add.w	d1,d0
stdFont:
		;-- get number of character storage locations
		moveq	#0,d1
		move.b	tf_HiChar(a5),d1
		sub.b	tf_LoChar(a5),d1
		addq.w	#2,d1
		add.w	d1,d1
		;-- get space for optional arrays
		moveq	#0,d3
		moveq	#0,d4
		tst.l	tf_CharSpace(a5)
		beq.s	charDataStorage	; never kern w/o space after ExtendFont
		move.w	d0,d3
		move.w	d1,d4
		add.w	d1,d0
		add.w	d1,d0
		;-- get space for CharLoc
charDataStorage:
		move.l	d0,d5
		add.w	d1,d1
		add.w	d1,d0
		addq.l	#3,d0		; ensure even number of longwords
		and.w	#$fffc,d0
		move.l	d0,d6

		;-- allocate the header
		move.l	#MEMF_CHIP+MEMF_CLEAR,d1
		LINKEXE	AllocMem
		tst.l	d0
		beq	sfFail2

	    ;-- initialize some of the header for the new font
		move.l	d0,a4
		;-- set the allocation size of this header
		move.l	d6,(a4)		; dfs_AllocationBytes

		;-- set the segment address of this diskfont
		move.w	#DFH_ID,dfs_DFH+dfh_FileID(a4)
		lsr.l	#2,d0
		addq.l	#1,d0		; point to dfs_NextSegment
		move.l	d0,dfs_DFH+dfh_Segment(a4)

		;-- copy the source font header into this new one
		move.l	a5,a0
		lea	dfs_DFH+dfh_TF(a4),a1
		sub.w	#dfs_DFH+dfh_TF+2,d2
		lsr.w	#1,d2
fontHeaderLoop:
		move.w	(a0)+,(a1)+
		dbf	d2,fontHeaderLoop

		;-- clone the original message reply port (usually zero)
		move.l	tf_Extension(a5),a0
		move.l	tfe_OrigReplyPort(a0),dfs_DFH+dfh_TF+MN_REPLYPORT(a4)

		;-- clear the font priority
		clr.b	dfs_DFH+dfh_TF+LN_PRI(a4)

		;-- copy the source font name into the destination
		move.l	LN_NAME(a5),a0
		lea	dfs_DFH+dfh_Name(a4),a1
		move.l	a1,dfs_DFH+dfh_TF+LN_NAME(a4)
		moveq	#MAXFONTNAME-1,d0
fontNameLoop:
		move.b	(a0)+,(a1)+
		dbeq	d0,fontNameLoop
		clr.b	-(a1)			; ensure null terminated

		;-- copy ctf_ColorFontColors if applicable
		btst	#FSB_COLORFONT,tf_Style(a5)
		beq.s	spacePatch
		;	copy ctf_ColorFontColors if of a known type
		move.l	ctf_ColorFontColors(a5),d1
		beq.s	spacePatch
		move.l	d1,a0		; lea cfc_Reserved(d1),a0
		tst.w	(a0)+
		bne.s	spacePatch
		;	initialize the color map pointer
		lea	dfs_DFH+dfh_TF+ctf_SIZEOF(a4),a1
		move.l	a1,dfs_DFH+dfh_TF+ctf_ColorFontColors(a4)
		;	copy the color map
		move.w	(a0)+,d1	; cfc_Count(a0)
		move.w	d1,cfc_Count(a1)
		addq.l	#cfc_SIZEOF,a1
		move.l	a1,dfs_DFH+dfh_TF+ctf_SIZEOF+cfc_ColorTable(a4)
		move.l	(a0)+,a0	; cfc_ColorTable(a0)
		bra.s	colorMapDBF
colorMapLoop:
		move.w	(a0)+,(a1)+
colorMapDBF:
		dbf	d1,colorMapLoop

		;-- patch up the optional array pointers
spacePatch:
		tst	d3
		beq.s	charSpacePatch	; never kern w/o space after ExtendFont
		lea	0(a4,d3.w),a0
		move.l	a0,dfs_DFH+dfh_TF+tf_CharSpace(a4)
		add.w	d4,a0
		move.l	a0,dfs_DFH+dfh_TF+tf_CharKern(a4)
		;-- patch up the CharSpace pointer
charSpacePatch:
		lea	0(a4,d5.w),a0
		move.l	a0,dfs_DFH+dfh_TF+tf_CharLoc(a4)

		;-- clear the accessor count
		clr.w	dfs_DFH+dfh_TF+tf_Accessors(a4)
		;-- show scaled
FP_SCALEDMASK	EQU	(~(FPF_DESIGNED!FPF_DISKFONT!FPF_ROMFONT))&$ff
		and.b	#FP_SCALEDMASK,dfs_DFH+dfh_TF+tf_Flags(a4)

		;-- set Y Size
		move.w	d7,dfs_DFH+dfh_TF+tf_YSize(a4)

		;-- adjust Baseline
		move.w	tf_Baseline(a5),d0
		move.w	sfv_BSA+bsa_YDestFactor(a3),d1
		move.w	sfv_BSA+bsa_YSrcFactor(a3),d2
		LINKGFX	ScalerDiv
		move.w	d0,d3
		move.w	tf_Baseline(a5),d0
		addq.w	#1,d0
		move.w	sfv_BSA+bsa_YDestFactor(a3),d1
		LINKGFX	ScalerDiv
		subq.w	#1,d0
		cmp.w	d3,d0
		bge.s	newBaseline
		move.w	d3,d0
newBaseline:
		move.w	d0,dfs_DFH+dfh_TF+tf_Baseline(a4)

		;-- set nominal X Size
		move.w	tf_XSize(a5),d0
		bsr	scaleX
		move.w	d0,dfs_DFH+dfh_TF+tf_XSize(a4)

		;-- adjust BoldSmear
		move.w	tf_BoldSmear(a5),d0
		bsr	scaleX

	; There is a bug in the graphic text routine when dealing with boldsmear.
	; It just shifts over the amount of bits in boldsmear.  I put in a check
	; to max out at 16, but V39 and greater of graphics has it fixed.

		move.l	dfl_GfxBase(a6),a0
		cmp.w	#39,LIB_VERSION(a0)
		bge.s	less16
		cmp.w	#16,d0
		blt.s	less16
		moveq	#16,d0
less16:
		move.w	d0,dfs_DFH+dfh_TF+tf_BoldSmear(a4)

	    ;-- get the CharData
		;-- sum character widths
		;	d3  summed width
		;	d4  incrementing char long index
		;	d5  decrementing char count
		moveq	#0,d5
		move.b	tf_HiChar(a5),d5
		sub.b	tf_LoChar(a5),d5
		addq.w	#1,d5
		moveq	#0,d4
		moveq	#0,d3
		;--	cycle thru characters, scaling in x
		;--	    check for previous instance of this CharData
sumWidthsLoop:
		move.l	tf_CharLoc(a5),a0
		move.l	0(a0,d4.w),d0
		move.w	d4,d1
		lsr.w	#2,d1
		bra.s	checkDupWidthDBF
checkDupWidthLoop:
		cmp.l	(a0)+,d0
		beq.s	sumWidthsNext
checkDupWidthDBF:
		dbf	d1,checkDupWidthLoop

		;--	    scale this width
		move.w	sfv_BSA+bsa_XDestFactor(a3),d1	;
		LINKGFX	ScalerDiv			; * f1/f0
		add.w	d0,d3
sumWidthsNext:
		addq.w	#4,d4
		dbf	d5,sumWidthsLoop

		moveq	#1,d4			; guess depth is 1
		btst	#FSB_COLORFONT,tf_Style(a5)
		beq.s	unitDepth
		move.b	ctf_Depth(a5),d4
unitDepth:
		move.w	tf_YSize(a5),d6
		move.w	tf_Modulo(a5),d5

		;-- calculate the size
		move.w	d3,d0
		add.w	#15,d0				; rounded up to word
		lsr.w	#4,d0				; number of words
		add.w	d0,d0				; number of bytes
		move.w	d0,dfs_DFH+dfh_TF+tf_Modulo(a4)
		move.w	d0,sfv_DBM+bm_BytesPerRow(a3)

		mulu	d4,d0
		mulu	d7,d0			; template size
		addq.l	#8,d0			; + size for segment header

		;-- allocate the data
		addq.l	#3,d0		; ensure even number of longwords
		and.w	#$fffc,d0
		move.l	d0,d2
		move.l	#MEMF_CHIP,d1
		LINKEXE	AllocMem
		tst.l	d0
		beq	sfFail2

		;-- save pointers to this data
		move.l	d0,a2
		move.l	d2,(a2)+
		clr.l	(a2)+
		lsr.l	#2,d0
		addq.l	#1,d0
		move.l	d0,4(a4)

	    ;-- get memory to ensure source char data is in chip memory
		move.w	d5,sfv_SBM+bm_BytesPerRow(a3)
		move.w	d5,d0
		mulu	d4,d0
		mulu	d6,d0
		move.l	d0,d2

		;-- allocate the data
		move.l	#MEMF_CHIP,d1
		LINKEXE	AllocMem

		;-- save pointers to this data
		move.l	d0,sfv_CCDAddr(a3)
		beq	sfFail2
		move.l	d2,sfv_CCDSize(a3)

	    ;-- build the source and destination bitmaps for BitMapScale
		move.w	d6,sfv_SBM+bm_Rows(a3)
		move.w	d7,sfv_DBM+bm_Rows(a3)
		move.b	d4,sfv_SBM+bm_Depth(a3)
		move.b	d4,sfv_DBM+bm_Depth(a3)

		mulu	d5,d6			; number of bytes in source

		;-- copy the data from the font into chip memory,
		;   and initialize destination planes
		btst	#FSB_COLORFONT,tf_Style(a5)
		beq.s	singleBitMap

		;-- color font
		move.l	d0,d5			; initial src chip CharData
		mulu	dfs_DFH+dfh_TF+tf_Modulo(a4),d7	; dest plane size
		moveq	#0,d2			; plane index [0,1,...]
copyCPlanes:
		move.w	d2,d1
		lsl.w	#2,d1
		lea	dfs_DFH+dfh_TF+ctf_CharData(a4),a0
		move.l	a2,0(a0,d1.w)			; dest CharData
		move.l	a2,sfv_DBM+bm_Planes(a3,d1.w)	; is dest plane
		move.l	ctf_CharData(a5,d1.w),a0	; src non-chip CharData
		cmp.l	tf_CharData(a5),a0		; check if default data
		bne.s	pastDefaultStuff
		move.l	a2,dfs_DFH+dfh_TF+tf_CharData(a4)
pastDefaultStuff:
		move.l	d5,a1				; src chip CharData
		move.l	a1,sfv_SBM+bm_Planes(a3,d1.w)	; is src plane
		move.l	d6,d0
		LINKEXE	CopyMem

		add.l	d6,d5				; next chip CharData
		add.l	d7,a2				; next final CharData
		addq.w	#1,d2				; bump plane counter
		cmp.w	d4,d2				; check vs. #planes
		blt.s	copyCPlanes
		bra.s	scaleSetup

		;-- normal font
singleBitMap:
		move.l	tf_CharData(a5),a0		; font CharData
		move.l	d0,a1				; chip CharData
		move.l	a1,sfv_SBM+bm_Planes(a3)
		move.l	d6,d0
		LINKEXE	CopyMem

		move.l	a2,dfs_DFH+dfh_TF+tf_CharData(a4)
		move.l	a2,sfv_DBM+bm_Planes(a3)

	    ;-- set up for each character in font
;   d4	array offset for this character
;   d5	character data origin
;   d6	character up counter
;   d7	character down counter
scaleSetup:
		moveq	#0,d7
		move.b	tf_HiChar(a5),d7
		sub.b	tf_LoChar(a5),d7
		addq.w	#1,d7
		moveq	#0,d6
		moveq	#0,d5
	    ;-- cycle thru characters, scaling in x
scaleLoop:
		;-- check for previous instance of this CharData
		move.w	d6,d4
		lsl.w	#2,d4
		move.l	tf_CharLoc(a5),a0
		move.l	0(a0,d4.w),d1
		move.w	d6,d0
		bra.s	checkDupDBF
checkDupLoop:
		cmp.l	(a0)+,d1
		bne.s	checkDupDBF
		;-- use data from duplicate
		move.l	a0,d1
		sub.l	tf_CharLoc(a5),d1
		move.l	dfs_DFH+dfh_TF+tf_CharLoc(a4),a0
		move.l	-4(a0,d1.w),0(a0,d4.w)
		bra.s	scaleKernSpace

checkDupDBF:
		dbf	d0,checkDupLoop

		;-- scale this CharData
		lea	sfv_BSA(a3),a0
		move.w	d1,bsa_SrcWidth(a0)
		swap	d1
		move.w	d1,bsa_SrcX(a0)
		move.w	d5,bsa_DestX(a0)
		clr.l	bsa_Flags(a0)
		LINKGFX	BitMapScale
		move.w	d5,d0
		swap	d0
		move.w	sfv_BSA+bsa_DestWidth(a3),d0
		move.l	dfs_DFH+dfh_TF+tf_CharLoc(a4),a0
		move.l	d0,0(a0,d4.w)
		add.w	d0,d5

scaleKernSpace:
		move.l	tf_CharSpace(a5),d0
		beq.s	nextChar	; never kern w/o space after ExtendFont

		lsr.w	#1,d4
		;--	scale character width
		move.l	d0,a0
		move.w	0(a0,d4.w),d0	; space0
		move.l	tf_CharKern(a5),a0
		add.w	0(a0,d4.w),d0	; + kern0

		bsr	scaleX
		move.w	d0,d3		; save character width
		bpl.s	forwardPath

		;--	kern is (probably) bigger in magnitude than space
		move.l	tf_CharKern(a5),a0
		move.w	0(a0,d4.w),d0
		bsr	scaleX
		;--	set kern & space for reverse path
		move.l	dfs_DFH+dfh_TF+tf_CharKern(a4),a0
		move.w	d0,0(a0,d4.w)
		move.l	dfs_DFH+dfh_TF+tf_CharSpace(a4),a0
		bra.s	stuffD3

forwardPath:
		;--	space is (probably) bigger in magnitude than kern
		move.l	tf_CharSpace(a5),a0
		move.w	0(a0,d4.w),d0
		bsr	scaleX
		;--	set kern & space for forward path
		move.l	dfs_DFH+dfh_TF+tf_CharSpace(a4),a0
		move.w	d0,0(a0,d4.w)
		move.l	dfs_DFH+dfh_TF+tf_CharKern(a4),a0
stuffD3:
		sub.w	d0,d3
		move.w	d3,0(a0,d4.w)

nextChar:
		addq.w	#1,d6
		dbf	d7,scaleLoop

	    ;-- ensure the blitter is done from the last scale
		LINKGFX	WaitBlit

	    ;-- coalesce tags
		;--	clone source tags
		move.l	tf_Extension(a5),a0
		move.l	tfe_Tags(a0),a0
		LINKUTL	CloneTagItems
		move.l	d0,sfv_TagClone(a3)
		beq.s	nullTags	; memory failure: bail out

		;--	link any DPI specified in request to any clone
		lea	sfv_DPITag(a3),a0
		tst.l	(a0)
		beq.s	testNullClone	; zero: DPI not specified

		addq.l	#8,a0		; bump past DPI specification
		bra.s	linkClone

testNullClone:
		move.l	d0,a1
		tst.l	(a1)		; empty clone?
		bne.s	linkClone	;   no: TAG_MORE immediately to clone

nullTags:
		suba.l	a1,a1
		bra.s	extendFont

linkClone:
		move.l	#TAG_MORE,(a0)+
		move.l	sfv_TagClone(a3),(a0)+

		;-- create taglist segment
		;--	size taglist
		pea	sfv_DPITag(a3)
		moveq	#8,d2		; space for seg header
sizeTagsLoop:
		move.l	a7,a0
		LINKUTL	NextTagItem
		addq.l	#8,d2		; bump even for TAG_DONE
		tst.l	d0
		bne.s	sizeTagsLoop
		addq.l	#4,a7

		move.l	d2,d0
		move.l	#MEMF_CLEAR,d1
		LINKEXE	AllocMem
		tst.l	d0
		beq.s	sfFail2

		move.l	4(a4),d1		; get second segment address
		lsl.l	#2,d1			;
		move.l	d1,a1			;

		move.l	d0,a2
		move.l	d2,(a2)+
		clr.l	(a2)+
		lsr.l	#2,d0
		addq.l	#1,d0
		move.l	d0,(a1)
		move.l	a2,d3

		pea	sfv_DPITag(a3)
copyTagsLoop:
		move.l	a7,a0
		LINKUTL	NextTagItem
		tst.l	d0
		beq.s	doneTags
		move.l	d0,a0
		move.l	(a0)+,(a2)+
		move.l	(a0),(a2)+
		bra.s	copyTagsLoop
doneTags:
		addq.l	#4,a7
		clr.l	(a2)+
		clr.l	(a2)+
		move.l	d3,a1

	    ;-- compile font
extendFont:
		lea	dfs_DFH+dfh_TF(a4),a0
		LINKGFX	ExtendFont
		tst.l	d0
		beq.s	sfFail2

	    ;-- release the intermediate data
		bsr.s	sfFreeArgs

	    ;-- return this font
		lea	dfs_DFH(a4),a0
		move.l	a0,d0

sfDone:
		movem.l	(a7)+,d2-d7/a2-a6
		rts

sfFail2:
		;-- delete any allocated memory
		bsr.s	sfFreeAll

sfFail1:
		moveq	#0,d0
		bra.s	sfDone

;------ sfFreeAll ----------------------------------------------------
sfFreeAll:
		move.l	a4,d0		; test for null
		beq.s	sfFreeArgs
		bra.s	sffFreeMem
sffFreeLoop:
		;-- delete segments
		move.l	a4,d0
		beq.s	sfFreeArgs
		subq.l	#1,d0
		lsl.l	#2,d0
		move.l	d0,a4
sffFreeMem:
		move.l	a4,a1
		move.l	(a4),d0		; size of this segment
		move.l	4(a4),a4
		LINKEXE	FreeMem
		bra.s	sffFreeLoop

sfFreeArgs:
		move.l	a3,d0
		beq.s	sffRts
		move.l	sfv_CCDSize(a3),d0
		beq.s	sffFreeArg
		move.l	sfv_CCDAddr(a3),a1
		LINKEXE	FreeMem

sffFreeArg:
		move.l	sfv_TagClone(a3),a0
		LINKUTL	FreeTagItems

		move.l	#sfv_SIZEOF,d0
		move.l	a3,a1
		LINKEXE	FreeMem
sffRts:
		rts

;------	scaleX -------------------------------------------------------
;
;   destroys d2
;
scaleX:
		moveq	#0,d2
		tst.w	d0
		bpl.s	scaleXPos
		neg.w	d0
		bset	#31,d2
scaleXPos:
		move.w	sfv_BSA+bsa_XDestFactor(a3),d1
		move.w	sfv_BSA+bsa_XSrcFactor(a3),d2
		LINKGFX	ScalerDiv
		btst	#31,d2
		beq.s	scaleXDone
		neg.w	d0
scaleXDone:

		rts

; - - -	LMul  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_LMul:
		; long multiplication - D0 x D1
		;
		; result in [d1    ][d0     ]
		;	     63 - 32 31 - 0

		movem.l	d2-d5,-(a7)

		move.l	d0,d2
		move.l	d1,d3


		; find absolute value of arguments and compute final sign
		move.l	d2,d4
		bpl.s	lmPos1
		neg.l	d2
lmPos1:
		move.l	d3,d5
		bpl.s	lmPos2
		neg.l	d3
lmPos2:
		eor.l	d4,d5		; final sign in d5

		;-- compute partial products
		;			; d2   d3
		move.w	d2,d1
		mulu	d3,d1		; lo x lo in d1
		move.w	d2,d4
		swap	d2
		move.w	d2,d0
		mulu	d3,d2		; hi x lo in d2
		swap	d3
		mulu	d3,d0		; hi x hi in d0
		mulu	d4,d3		; lo x hi in d3

		;-- add partial products
					; d1    d0	^
					; lo*lo		d1
					;    hi*lo	d2
					;    lo*hi	d3
					;	hi*hi	d0
		moveq	#0,d4
		add.l	d3,d2		; mid: hi*lo + lo*hi
		swap	d0		; (take care of any carry)
		addx.w	d4,d0		;
		swap	d0		;

		swap	d1		; d1.hi += mid.lo
		add.w	d2,d1		;
		addx.l	d4,d0		; (take care of any carry)
		swap	d1

		clr.w	d2
		swap	d2		; d0.lo += mid.hi
		add.l	d2,d0		;

		;-- ensure sign is correct
		tst.l	d5
		bpl.s	lmDone
		neg.l	d1
		negx.l	d0

lmDone:
		exg	d0,d1
		movem.l	(a7)+,d2-d5
		rts

	END
