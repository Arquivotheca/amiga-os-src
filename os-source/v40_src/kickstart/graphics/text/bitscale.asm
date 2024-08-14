**
**	$Id: bitscale.asm,v 39.1 92/09/03 15:38:23 spence Exp $
**
**      bit scaling primitive
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		graphics

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/alerts.i"
	INCLUDE		"graphics/gfx.i"
	INCLUDE		"graphics/gfxbase.i"
	INCLUDE		"hardware/blit.i"
	INCLUDE		"hardware/custom.i"

	INCLUDE		"macros.i"
	INCLUDE		"scale.i"

    BITDEF  BSAF,XSHRINK,0
    BITDEF  BSAF,YSHRINK,1

	XDEF	_BitMapScale
	XDEF	_ScalerDiv

	XLVO	Alert			; Exec

	XREF	_custom

	XLVO	DisownBlitter		; Graphics
	XLVO	OwnBlitter		;

	XREF	_fwmaskTable
	XREF	_lwmaskTable
	XREF	waitblitdone

**	Assumptions

	IFNE	bsa_DestHeight-bsa_DestWidth-2
	FAIL	"bsa_DestHeight does not follow bsa_DestWidth, recode"
	ENDC


******* graphics.library/BitMapScale *********************************
*
*   NAME
*	BitMapScale -- Perform raster scaling on a bit map. (V36)
*
*   SYNOPSIS
*	BitMapScale(bitScaleArgs)
*	            A0
*
*	void BitMapScale(struct BitScaleArgs *);
*
*   FUNCTION
*	Scale a source bit map to a non-overlapping destination
*	bit map.
*
*   INPUTS
*	bitScaleArgs - structure of parameters describing scale:
*	    bsa_SrcX, bsa_SrcY - origin of the source bits.
*	    bsa_SrcWidth, bsa_SrcHeight - number of bits to scale from in x
*	    and y.
*	    bsa_DestX, bsa_DestY - origin of the destination.
*	    bsa_DestWidth, bsa_DestHeight - resulting number of bits in x
*	    and y.  NOTE: these values are set by this function.
*	    bsa_XSrcFactor:bsa_XDestFactor - equivalent to the ratio
*	        srcWidth:destWidth, but not necessarily the same
*	        numbers.  Each must be in the range 1..16383.
*	    bsa_YSrcFactor:bsa_YDestFactor - equivalent to the ratio
*	        srcHeight:destHeight, but not necessarily the same
*	        numbers.  Each must be in the range 1..16383.
*	    bsa_SrcBitMap - source of the bits to scale.
*	    bsa_DestBitMap - destination for the bits to scale.  This had
*	        better be big enough!
*	    bsa_Flags - future scaling options.  Set it to zero!
*	    bsa_XDDA, bsa_YDDA - for future use.  Need not be set by user.
*	    bsa_Reserved1, bsa_Reserved2 - for future use.  Need not be set.
*
*   RESULT
*	The destWidth, destHeight fields are set by this function as
*	described above.
*
*   NOTES
*	o   This function may use the blitter.
*	o   Overlapping source and destination bit maps are not
*	    supported.
*	o   No check is made to ensure destBitMap is big enough: use
*	    ScalerDiv to calculate a destination dimension.
*
*   BUGS
*	o   This function does not use the HighRes Agnus 'Big Blit'
*	    facility. You should not use XSrcFactor == XDestFactor,
*	    where SrcWidth or DestWidth > 1024.
*
*	o   Also, the blitter is used when expanding in the Y direction.
*	    You should not expand in the Y direction if
*	    ((DestX & 0xf) + DestWidth) >= 1024 pixels. (Up to 1008 pixels
*	    is always safe).
*
*   SEE ALSO
*	ScalerDiv()  graphics/scale.h
*
**********************************************************************
;------ Bresenham scaling: theory ------------------------------------
;
;   PROBLEM DESCRIPTION
;	Given a source and a destination raster size, the scaling will
;	be performed independently in x and y.  The following, then,
;	just describes a linear scaling.  For source and destination
;	raster sizes, specified as the number of dots in each, the
;	larger is n, the smaller is m.
;
;	Example: n = 5, m = 4.
;	      1   2   3   4   5		Stretch m to the same length
;	n   |-v-|-v-|-v-|-v-|-v-|	as n, and for each interval
;					in m, count the n intervals
;	m   |----|----|----|----|	whose centers are in that m
;	      1    2    3    4		interval.
;			
;	This gives the information that pixels 1..4 in m must be
;	replicated by {1,2,1,1} to expand to pixels 1..5 in n, or, to
;	shrink the raster, that pixels 2&3 of n must be coalesced into
;	pixel 2 of m.
;
;   INITIAL ALGORITHM
;	Initialization:
;	    e = m/2n - 1
;	For next pixel count:
;	    count = 0
;	    do
;		e += m/n
;		count++
;	    until e > 0
;	    e -= 1
;	    return count
;
;   FINAL ALGORITHM: initial algorithm multiplied by 2n
;	Initialization:
;	    e = m - 2n
;	For next pixel count:
;	    count = 0
;	    do
;		e += 2m
;		count++
;	    until e > 0
;	    e -= 2n
;	    return count
;
;---------------------------------------------------------------------
 STRUCTURE	BitScaleVariables,0
    UWORD   bsv_YSrcCount
    UWORD   bsv_YDestCount
    UWORD   bsv_ZCount
    UWORD   bsv_SrcX
    UWORD   bsv_DestX
    UWORD   bsv_XDDAInit
    UWORD   bsv_XNegDelt
    UWORD   bsv_XPosCorrec
    UWORD   bsv_YDDAInit
    UWORD   bsv_YDDAError
    UWORD   bsv_YNegDelt
    UWORD   bsv_YPosCorrec
    UWORD   bsv_SrcMod
    UWORD   bsv_DestMod
    APTR    bsv_SrcLine
    APTR    bsv_DestLine
    APTR    bsv_SrcPlane
    APTR    bsv_DestPlane
    LABEL   bsv_SIZEOF
;
;   d0	source word bit count 15..0
;   d1	destination word bit count 15..0
;   d2	source word, spilling left over MSB
;   d3	destination word, filling left from MSB
;   d4	dda error term
;   d5	negative delta
;   d6	source delta
;   d7	dest width
;
;   a0	next source word address
;   a1	next destination word address
;   a2	initial source line word address
;   a3	initial destination line word address
;   a4	argument block (bsa_)
;   a5	local variable block (bmv_)
;   a6	GfxBase
;
_BitMapScale:
		movem.l	d2-d7/a2-a6,-(a7)
		move.l	a0,a4			; save BitScaleArgs
		sub.w	#bsv_SIZEOF,a7		; get BitScaleVariables
		move.l	a7,a5
		;-- verify no reserved Flags are set: see, I learn.
		move.l	bsa_Flags(a4),d0
		and.l	#~(BSAFF_XSHRINK!BSAFF_YSHRINK),d0
		beq.s	bsChkSize

		move.l	a6,-(a7)
		move.l	gb_ExecBase(a6),a6
		ALERT	AN_GraphicsLib!AG_BadParm
		move.l	(a7)+,a6

bsEmpty:
		clr.l	bsa_DestWidth(a4)	; and bsa_DestHeight

bsDone:
		add.w	#bsv_SIZEOF,a7
		movem.l	(a7)+,d2-d7/a2-a6
		rts

bsChkSize:
		;-- verify non-zero source dimensions here
		tst.w	bsa_SrcWidth(a4)
		beq.s	bsEmpty
		tst.w	bsa_SrcHeight(a4)
		beq.s	bsEmpty

		;-- set up X scaling variables and function
		move.w	bsa_XSrcFactor(a4),d0
		move.w	bsa_XDestFactor(a4),d1
		cmp.w	d0,d1
		bne.s	bsXScaled
		lea	BSXCopy(pc),a2
		bra.s	bsGetY
bsXScaled:
		moveq	#BSAFB_XSHRINK,d4
		bsr	initDDA
		movem.w	d4/d5/d6,bsv_XDDAInit(a5)	; XNegDelt, XPosCorrec
		move.l	bsa_Flags(a4),d0
		btst.b	#BSAFB_XSHRINK,d0
		bne.s	bsXShrink
		lea	BSXExpand(pc),a2
		bra.s	bsGetY
bsXShrink:
		lea	BSXShrink(pc),a2

		;-- set up Y scaling variables and function
bsGetY:
		move.w	bsa_YSrcFactor(a4),d0
		move.w	bsa_YDestFactor(a4),d1
		cmp.w	d0,d1
		bne.s	bsYScaled
		lea	BSYCopy(pc),a3
		clr.w	bsa_YDDA(a4)			; cache ending DDA
		bra.s	bsGetZ
bsYScaled:
		moveq	#BSAFB_YSHRINK,d4
		bsr	initDDA
		move.w	d4,bsv_YDDAInit(a5)
		movem.w	d5/d6,bsv_YNegDelt(a5)		; YPosCorrec
		move.l	bsa_Flags(a4),d0
		btst.b	#BSAFB_YSHRINK,d0
		bne.s	bsYShrink
		lea	BSYExpand(pc),a3
		bra.s	bsGetZ
bsYShrink:
		lea	BSYShrink(pc),a3

		;-- set up depth variables
bsGetZ:
		move.l	bsa_SrcBitMap(a4),a0
		move.l	bsa_DestBitMap(a4),a1
		moveq	#0,d0
		move.b	bm_Depth(a0),d0
		cmp.b	bm_Depth(a1),d0
		ble.s	bsHaveDepth
		move.b	bm_Depth(a1),d0
bsHaveDepth:
		move.w	d0,bsv_ZCount(a5)
		move.w	bm_BytesPerRow(a0),bsv_SrcMod(a5)
		move.w	bm_BytesPerRow(a1),bsv_DestMod(a5)
		addq.l	#bm_Planes,a0
		move.l	a0,bsv_SrcPlane(a5)
		addq.l	#bm_Planes,a1
		move.l	a1,bsv_DestPlane(a5)

bsNextPlane:
		subq.w	#1,bsv_ZCount(a5)
		bmi	bsDone
		;-- get next source plane and initialize line start
		;	get next plane in source
		move.l	bsv_SrcPlane(a5),a0
		move.l	(a0)+,a1
		move.l	a0,bsv_SrcPlane(a5)
		;	adjust line to SrcY
		move.w	bsa_SrcY(a4),d0
		mulu	bsv_SrcMod(a5),d0
		add.l	d0,a1
		;	adjust line to SrcX
		move.w	bsa_SrcX(a4),d0
		lsr.w	#4,d0
		add.w	d0,d0
		add.w	d0,a1
		move.l	a1,bsv_SrcLine(a5)
		;-- get next destination plane and initialize line start
		;	get next plane in source
		move.l	bsv_DestPlane(a5),a0
		move.l	(a0)+,a1	; get next plane in destination
		move.l	a0,bsv_DestPlane(a5)
		;	adjust line to DestY
		move.w	bsa_DestY(a4),d0
		mulu	bsv_DestMod(a5),d0
		add.l	d0,a1
		;	adjust line to DestX
		move.w	bsa_DestX(a4),d0
		lsr.w	#4,d0
		add.w	d0,d0
		add.w	d0,a1
		move.l	a1,bsv_DestLine(a5)
		;-- initialize Y Count and Error
		move.w	bsv_YDDAInit(a5),bsv_YDDAError(a5)
		move.w	bsa_SrcHeight(a4),bsv_YSrcCount(a5)
		clr.w	bsv_YDestCount(a5)

bsNextLine:
		;-- set up first source word
		move.l	bsv_SrcLine(a5),a0
		move.w	bsa_SrcX(a4),d4
		and.w	#$f,d4
		move.w	(a0)+,d2
		lsl.w	d4,d2
		moveq	#15,d0
		sub.w	d4,d0
		;-- set up first destination word
		move.l	bsv_DestLine(a5),a1
		move.w	bsa_DestX(a4),d4
		and.w	#$f,d4
		move.w	(a1),d3
		rol.w	d4,d3
		moveq	#15,d1
		sub.w	d4,d1
		;-- initialize X scale loop variables
		movem.w	bsv_XDDAInit(a5),d4/d5	; XNegDelt
		move.w	bsa_SrcWidth(a4),d6
		subq.w	#1,d6
		moveq	#0,d7
		jmp	(a2)		; BSXCopy, BSXExpand, or BSXShrink

;------ BSXExpand ----------------------------------------------------
BSXExpand:
		tst.w	d2
		bra.s	bsxeSwitch01	; dispatch to shift in 0s or 1s
bsxeNextSrc:
		moveq	#15,d0		; restart source word bit count
		move.w	(a0)+,d2	; get next source word
		bra.s	bsxeSwitch01	; dispatch to shift in 0s or 1s
bsxeSrcLoop:
		subq.w	#1,d0		; decrement source bit count
		blt.s	bsxeNextSrc
		add.w	d2,d2		; move next src bit up to MSB
bsxeSwitch01:
		bmi.s	bsxe1Loop

bsxe0Loop:
		add.w	d3,d3		; shift in a zero
		addq.w	#1,d7		; count up destination length
		subq.w	#1,d1		; count down destination word
		bge.s	bsxe0NegDelta	; skip below if destination not full
		move.w	d3,(a1)+	; update destination
		moveq	#15,d1		; restart destination word bit count
bsxe0NegDelta:
		add.w	d5,d4		; error += negative delta
		ble.s	bsxe0Loop	; while error <= 0
		sub.w	bsv_XPosCorrec(a5),d4	; error -= positive correction
		dbf	d6,bsxeSrcLoop	; until source width exhausted
		bra.s	bsxEndLine

bsxe1Loop:
		add.w	d3,d3		; shift in a
		addq.w	#1,d3		;   one
		addq.w	#1,d7		; count up destination length
		subq.w	#1,d1		; count down destination word
		bge.s	bsxe1NegDelta	; skip below if destination not full
		move.w	d3,(a1)+	; update destination
		moveq	#15,d1		; restart destination word bit count
bsxe1NegDelta:
		add.w	d5,d4		; error += negative delta
		ble.s	bsxe1Loop	; while error <= 0
		sub.w	bsv_XPosCorrec(a5),d4	; error -= positive correction
		dbf	d6,bsxeSrcLoop	; until source width exhausted
		bra.s	bsxEndLine

;------ BSXShrink ----------------------------------------------------
bsxsPosCorrect:
		sub.w	bsv_XPosCorrec(a5),d4	; error -= positive correction
BSXShrink:
		;-- sample this first source bit
		add.w	d2,d2		; shift source bit out into eXtend
		addx.w	d3,d3		; shift it in to destination
		addq.w	#1,d7		; count up destination
		;-- check if this sample filled destination
		subq.w	#1,d1		; decrement destination bit count
		bge.s	bsxsTossSrc	; skip below if destination not full
		;-- get next destination word
		move.w	d3,(a1)+	; update destination
		moveq	#15,d1		; restart destination word bit count
		bra.s	bsxsTossSrc

		;-- throw away subsequent source bits
bsxsNextToss:
		add.w	d2,d2		; toss out next source bit
		;-- check if done
bsxsTossSrc:
		subq.w	#1,d6		; check for source width exhausted
		blt.s	bsxEndLine	;
		;-- check if source word is empty
		subq.w	#1,d0		; check for source word exhausted
		bge.s	bsxsNegDelta
		;-- get next source word
		move.w	(a0)+,d2	; get next source word
		moveq	#15,d0		; restart source word bit count
bsxsNegDelta:
		;-- check error term
		add.w	d5,d4		; error += negative delta
		ble.s	bsxsNextToss	; while error <= 0
		bra.s	bsxsPosCorrect

; - - -	BSX shared end code - - - - - - - - - - - - - - - - - - - - -
bsxEndLine:
		cmp.w	#15,d1		; check if full destination
		beq	bsEndLine
		addq.w	#1,d1		; adjust to number of bits unaffected
		move.w	(a1),d0		; get original destination
		ror.l	d1,d0		; put remaining tail into top of d0
		move.w	d3,d0		; put new bits into bottom of d0
		rol.l	d1,d0		; roll concatination into word
		move.w	d0,(a1)		; update destination
		bra	bsEndLine

;------ BSXCopy ------------------------------------------------------
BSXCopy:
		CALLLVO	OwnBlitter
		bsr	waitblitdone
		lea	_custom,a0
		move.w	bsa_SrcX(a4),d5
		and.w	#$f,d5
		move.w	bsa_DestX(a4),d6
		and.w	#$f,d6
		;-- fwmask for dest
		move.w	d6,d0
		add.w	d0,d0
		lea	_fwmaskTable(pc),a1
		move.w	0(a1,d0.w),bltafwm(a0)
		;-- B shift into con1
		move.w	d6,d0
		sub.w	d5,d0
		ror.w	#4,d0
		move.w	d0,bltcon1(a0)
		;-- lwmask for dest + size - 1
		move.w	d6,d0
		add.w	bsa_SrcWidth(a4),d0
		subq.w	#1,d0
		move.w	d0,d4			; save dest + size - 1
		and.w	#$f,d0
		add.w	d0,d0
		lea	_lwmaskTable(pc),a1
		move.w	0(a1,d0.w),bltalwm(a0)
		;-- constant A data
		move.w	#$ffff,bltadat(a0)
		;-- massage blit size
		lsr.w	#4,d4			; blit words - 1
		addq.w	#1,d4			; blit words
		move.w	d4,d0
		;-- adjust modulos
		add.w	d0,d0			; blit bytes
		neg.w	d0
		move.w	d0,d1
		add.w	bsv_SrcMod(a5),d0
		move.w	d0,bltbmod(a0)
		add.w	bsv_DestMod(a5),d1
		move.w	d1,bltcmod(a0)
		move.w	d1,bltdmod(a0)
		move.l	bsv_SrcLine(a5),d0
		move.l	d0,bltbpt(a0)
		move.l	bsv_DestLine(a5),d0
		move.l	d0,bltcpt(a0)
		move.l	d0,bltdpt(a0)
		;-- con0: copy
		move.w	#ABC+ABNC+NABC+NANBC+SRCB+SRCC+DEST,bltcon0(a0)
		;-- size & start blit
		or.w	#$40,d4			; y size is 1
		move.w	d4,bltsize(a0)
		CALLLVO	DisownBlitter
		moveq	#0,d4
		move.w	bsa_SrcWidth(a4),d7

;---------------------------------------------------------------------
bsEndLine:
		move.w	d4,bsa_XDDA(a4)	; cache ending DDA
		move.w	d7,bsa_DestWidth(a4) ; cache destination width
		jmp	(a3)		; BSYCopy, BSYExpand, or BSYShrink


;------ BSYExpand ----------------------------------------------------
BSYExpand:
		bsr	bsyCount
		move.w	d0,d7

		cmp.w	#1,d0
		beq	bsyeUnit		; don't replicate for 1

		;-- copy the previous destination line d0 times
		CALLLVO	OwnBlitter
		bsr	waitblitdone
		lea	_custom,a0
		move.w	bsa_DestX(a4),d6
		and.w	#$f,d6
		;-- fwmask for dest
		move.w	d6,d0
		add.w	d0,d0
		lea	_fwmaskTable(pc),a1
		move.w	0(a1,d0.w),bltafwm(a0)
		;-- lwmask for dest + size - 1
		move.w	d6,d0
		add.w	bsa_DestWidth(a4),d0
		subq.w	#1,d0
		move.w	d0,d4			; save dest + size - 1
		and.w	#$f,d0
		add.w	d0,d0
		lea	_lwmaskTable(pc),a1
		move.w	0(a1,d0.w),bltalwm(a0)
		;-- massage blit size
		lsr.w	#4,d4			; blit words - 1
		addq.w	#1,d4			; blit words
		move.w	d4,d0
		;-- adjust modulos
		add.w	d0,d0			; blit bytes
		neg.w	d0
		move.w	d0,bltbmod(a0)
		move.w	bsv_DestMod(a5),d1
		add.w	d1,d0
		move.w	d0,bltcmod(a0)
		move.w	d0,bltdmod(a0)
		move.l	bsv_DestLine(a5),a1
		move.l	a1,bltbpt(a0)
		add.w	d1,a1
		move.l	a1,bltcpt(a0)
		move.l	a1,bltdpt(a0)
		;-- constant A data
		move.w	#$ffff,bltadat(a0)
		;-- con0: copy
		move.w	#ABC+ABNC+NABC+NANBC+SRCB+SRCC+DEST,bltcon0(a0)
		;-- no shift into con1
		move.w	#0,bltcon1(a0)
		;-- size & start blit
		move.w	d7,d0
		subq.w	#1,d0
		lsl.w	#6,d0
		or.w	d0,d4
		move.w	d4,bltsize(a0)
		CALLLVO	DisownBlitter

bsyeUnit:
		;-- update counters
		add.w	d7,bsv_YDestCount(a5)
		subq.w	#1,bsv_YSrcCount(a5)
		ble.s	bsyEndPlane

		;-- get next source line
		cmp.w	#1,d7			; special case for 1
		beq.s	bsyc1Entry		;
		move.l	bsv_SrcLine(a5),a0
		add.w	bsv_SrcMod(a5),a0
		move.l	a0,bsv_SrcLine(a5)
		;-- get next destination line
		move.l	bsv_DestLine(a5),a0
		move.w	bsv_DestMod(a5),d0
		mulu	d7,d0
		add.l	d0,a0
		move.l	a0,bsv_DestLine(a5)
		bra	bsNextLine

		
;------ BSYShrink ----------------------------------------------------
BSYShrink:
		bsr.s	bsyCount

		;-- update counters
		addq.w	#1,bsv_YDestCount(a5)
		move.w	bsv_YSrcCount(a5),d1
		sub.w	d0,d1
		move.w	d1,bsv_YSrcCount(a5)	; save source down counter
		ble.s	bsyEndPlane

		;-- get next source line
		cmp.w	#1,d0			; special case for 1
		beq.s	bsyc1Entry		;
		move.l	bsv_SrcLine(a5),a0
		move.w	bsv_SrcMod(a5),d1
		mulu	d0,d1
		add.l	d1,a0
		bra.s	bsycShrinkEntry

; - - -	BSY shared counting code - - - - - - - - - - - - - - - - - - -
bsyCount:
		moveq	#-1,d0		; start with nothing
		movem.w	bsv_YDDAError(a5),d4/d5	; YNegDelt
		
		;-- count in d0
bsyNextCount:
		add.w	d5,d4		; error += negative delta
		dbgt	d0,bsyNextCount	; until error > 0
		sub.w	bsv_YPosCorrec(a5),d4	; error -= positive correction
		move.w	d4,bsv_YDDAError(a5)	; save this error
		neg.w	d0
		rts


bsyEndPlane:
		move.w	d4,bsa_YDDA(a4)	; cache ending DDA
		move.w	bsv_YDestCount(a5),bsa_DestHeight(a4)	; cache height
		bra	bsNextPlane	; go handle next Plane

;------ BSYCopy ------------------------------------------------------
BSYCopy:
		subq.w	#1,bsv_YSrcCount(a5)
		bgt.s	bsyc1Entry

		;-- source height exhausted
		move.w	bsa_SrcHeight(a4),bsa_DestHeight(a4)
		bra	bsNextPlane

bsyc1Entry:
		;-- get next source line
		move.l	bsv_SrcLine(a5),a0
		add.w	bsv_SrcMod(a5),a0
bsycShrinkEntry:
		move.l	a0,bsv_SrcLine(a5)
		;-- get next destination line
		move.l	bsv_DestLine(a5),a0
		add.w	bsv_DestMod(a5),a0
		move.l	a0,bsv_DestLine(a5)
		bra	bsNextLine


;------ initDDA ------------------------------------------------------
;
;   INPUTS
;   d0	src
;   d1	dest
;   d4	x/y: 0 for x, 1 for y
;   a4	bsa_
;
;   RESULTS
;	m <= n
;   d4	dda error term: m - 2n
;   d5	negative delta: 2m
;   d6	positive correction: 2n
;  (a4)	bsa_
;	    bsa_Flags
initDDA:
		cmp.w	d0,d1
		bge.s	expandingDDA
		;-- coalescing DDA
		exg	d0,d1
		move.l	bsa_Flags(a4),d5
		bset	d4,d5
		move.l	d5,bsa_Flags(a4)
		bra.s	calcDDA
		;-- expanding DDA
expandingDDA:
		move.l	bsa_Flags(a4),d5
		bclr	d4,d5
		move.l	d5,bsa_Flags(a4)
		;-- m in d0, n in d1
calcDDA:
		move.w	d0,d4
		move.w	d0,d5
		add.w	d5,d5
		move.w	d1,d6
		add.w	d6,d6
		sub.w	d6,d4
		rts
 

******* graphics.library/ScalerDiv **************************************
*
*   NAME
*	ScalerDiv -- Get the scaling result that BitMapScale would. (V36)
*
*   SYNOPSIS
*	result = ScalerDiv(factor, numerator, denominator)
*	D0                 D0      D1         D2
*
*	UWORD ScalerDiv(UWORD, UWORD, UWORD);
*
*   FUNCTION
*	Calculate the expression (factor*numerator/denominator) such
*	that the result is the same as the width of the destination
*	result of BitMapScale when the factor here is the width of
*	the source, and the numerator and denominator are the
*	XDestFactor and XSrcFactor for BitMapScale.
*
*   INPUTS
*	factor                 - a number in the range 0..16383
*	numerator, denominator - numbers in the range 1..16383
*
*   RESULT
*	this returns factor*numerator/denominator
*
*************************************************************************
_ScalerDiv:
		cmp.w	d1,d2
		bgt.s	sdShrink
	;   d0: f;  d1: n;  d2:m
	;   d = f * n / m;
	;   t = (f * n) % m;
	;   if (t*2 >= m) d++;
		mulu	d0,d1
		divu	d2,d1
		moveq	#0,d0
		move.w	d1,d0
		swap	d1
		add.w	d1,d1
		cmp.w	d1,d2
		bgt.s	sdRts
		addq.w	#1,d0
sdRts:
		rts

	;   d0: f;  d1: m;  d2:n
	;   d = f * m / n;
	;   t = (f * m) % n;
	;   if (t*2 > m) d++;
sdShrink:
		mulu	d1,d0
		divu	d2,d0
		swap	d0
		add.w	d0,d0
		cmp.w	d0,d1
		bge.s	sdNoAdjust
		add.l	#$10000,d0
sdNoAdjust:
		clr.w	d0
		swap	d0
		bra.s	sdRts

	END
