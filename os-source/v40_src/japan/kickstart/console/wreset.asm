**
**      $Id: wreset.asm,v 36.49 92/07/17 11:17:50 darren Exp $
**
**      reset and/or resize console unit
**
**      (C) Copyright 1985,1987,1989,1990,1991,1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/memory.i"

	INCLUDE	"intuition/intuition.i"

	INCLUDE	"debug.i"

**	Exports

	XDEF	WriteReset
	XDEF	ReSizePost
	XDEF	ReSizeUnit


**	Imports

	XLVO	AllocMem		; Exec

	XLVO	ClearRegion		; Graphics
	XLVO	FontExtent		;
	XLVO	OrRectRegion		;

	XLVO	BeginUpdate		; Layers
	XLVO	EndUpdate		;
	XLVO	InstallClipRegion	;

	XREF	LockDRPort
	XREF	UnLockRPort

	XREF	CursDisable
	XREF	CursEnable
	XREF	CursUpdate

	XREF	ClearRaster
	XREF	ClearScreen

	XREF	PackMap			;scroll
	XREF	UnpackMap
	XREF	ScrollYDisplay
	XREF	ResetBuffer

	XREF	InitTabs

	XREF	G0Handler

	XREF	RefreshUnit

	XREF	sgrPrimary

	XREF	FreeBuffer


*------ WriteReset ---------------------------------------------------
*
*   a2	unit data
*
WriteReset:
		movem.l	d2-d5,-(a7)

		bsr	LockDRPort

		move.l	cu_Mask(a2),cd_RastPort+rp_Mask(a6)

		bsr	CursDisable

		bsr	InitTabs

		move.l	#G0Handler,cu_GLHandler(a2)

		clr.w	cu_PState(a2)
		clr.l	cu_XCP(a2)		; also cu_YCP(a2)
		clr.l	cu_DisplayXL(a2)	; also cu_DisplayYL(a2)
		clr.l	cu_XMinShrink(a2)	; also cu_YMinShrink(a2)

		move.w	#9999,d0
		move.w	d0,cu_XRExtant(a2)	; absurdly large so that the
		move.w	d0,cu_YRExtant(a2)	;   GetBorders d6/d7 is used
						;   to clear.

		move.w  d0,cu_FullXRExtant(a2)
		move.w  d0,cu_FullYRExtant(a2)

		or.b	#CUF_CURSON,cu_CursorFlags(a2)	; turn on cursor

		moveq	#PMB_AWM/8,d0
		lea	cu_Modes(a2),a0
wrClrModes:
		clr.b	(a0)+
		dbf	d0,wrClrModes
		bset	#M_LNM&07,cu_Modes+(M_LNM/8)(a2)
		bset	#PMB_ASM&07,cu_Modes+(PMB_ASM/8)(a2)
		bset	#PMB_AWM&07,cu_Modes+(PMB_AWM/8)(a2)

		move.l	cu_Window(a2),a1
		move.l	wd_RPort(a1),a0

		move.b	#0,cu_AOLPen(a2)
		move.l	rp_Font(a0),cu_Font(a2)
		move.w	rp_AlgoStyle(a0),cu_AlgoStyle(a2)	; & TxFlags
		move.l	rp_TxHeight(a0),cu_TxHeight(a2)		; & TxWidth
		move.l	rp_TxBaseline(a0),cu_TxBaseline(a2)	; & TxSpacing

		move.w	cu_TxWidth(a2),d0
		add.w	cu_TxSpacing(a2),d0
		move.w	d0,cu_XRSize(a2)
		move.w	cu_TxHeight(a2),cu_YRSize(a2)

		;--	initialize cu_Flags
		and.b	#CU_RESET0MASK,cu_Flags(a2)

		lea	-te_SIZEOF(a7),a7
		move.l	rp_Font(a0),a0
		move.l	a7,a1
		LINKGFX	FontExtent
		move.w	cu_XRSize(a2),d0
		tst.w	te_Extent+ra_MinX(a7)
		bmi.s	wrHaveKernless		; not kernless
		cmp.w	te_Extent+ra_MaxX(a7),d0
		ble.s	wrHaveKernless		; not kernless within spacing

		bset.b	#CUB_KERNLESS,cu_Flags(a2)

wrHaveKernless:
		lea	te_SIZEOF(a7),a7

		;-- cache bitmap depth (no touch D0!)

		move.l	cu_Window(a2),a0
		move.l	wd_RPort(a0),a1
		move.l	rp_BitMap(a1),a1
		move.b	bm_Depth(a1),cu_Depth(a2)
		
		move.l	wd_WScreen(a0),a1
	;;;	move.w	sc_Flags(a1),d1

		move.b	#$FF,cu_CursorMask(a2)	;all planes by default

	;;;	and.w	#PENSHARED,d1
	;;;	beq.s	wrNotPenShared

	;;;	move.b	#$03,cu_CursorMask(a2)	;pen shared, mask cursor (2 planes)

wrNotPenShared:

	;;;;;
	;; New code to calculate x/y dimensions for char map.
	;;
	;; Uses Intuition Screen size - as of V39 we have interleaved
	;; bitmaps, and the BytesPerRow is bogus (is depth * old
	;; meaning of BytesPerRow)
	;;;;;

		;-- initialize the buffers if necessary
		btst	#0,cu_DevUnit+3(a2)
		beq	wrSetPens

		move.l	wd_Flags(a0),d1

		and.l	#REFRESHBITS,d1
		cmp.l	#SIMPLE_REFRESH,d1
		bne	wrSetPens

		moveq	#7,d2			;get maximum X width
		add.w	sc_Width(a1),d2		;screen width + 7 for rounding
		and.b	#%11111000,d2		;rounded to nearest byte

	;;;	move.l	cd_RastPort+rp_BitMap(a6),a0
	;;;	moveq	#0,d2
	;;;	move.w	bm_BytesPerRow(a0),d2	; get maximum X width
	;;;	lsl.l	#3,d2			; convert to pixels
		divu	d0,d2			; convert to char X locations
		move.w	d2,cu_CM+cm_BufferWidth(a2)
		clr.w	cu_CM+cm_BufferHeight(a2)

		moveq	#0,d3

		move.w	sc_Height(a1),d3

	;;;	move.w	bm_Rows(a0),d3		; get maximum Y height
		divu	cu_YRSize(a2),d3	; convert to char Y locations
		move.w	d3,cu_CM+cm_BufferLines(a2) ; (and save)
		move.w	d3,d0
		addq.w	#1,d0			; get number of char locations,
		mulu	d2,d0			;   which is 1.5 times the
		move.l	d0,d1			;   maximum number displayed
		lsr.l	#1,d1			;   with an extra line thrown
		add.l	d1,d0			;   in.
		move.l	d0,d5

		ext.l	d3
		lsl.l	#3,d3			; two line buffers
		move.l	d3,d4
		add.l	d0,d4			; one char buffer space
		add.l	d0,d4			; and one attr buffer space
		add.l	d0,d4			;

		clr.l	cu_CM+cm_BufferXL(a2)	; also cm_BufferYL

		;--	check if buffer allocation is OK
		cmp.l	cu_CM+cm_AllocSize(a2),d4
		beq.s	wrBufferLines

wrAllocBuffer:
		bsr	FreeBuffer		; free any old ones
		move.l	d4,cu_CM+cm_AllocSize(a2)
		move.l	d4,d0
		moveq	#0,d1

		LINKEXE	AllocMem
		move.l	d0,cu_CM+cm_AllocBuffer(a2)
		beq	wrBufAllocFail

wrBufferLines:
		;--	fill AttrBufLines
		;--	    generate first attribute line address/2
		move.l	cu_CM+cm_AllocBuffer(a2),a0
		add.l	a0,d3
		lsr.l	#1,d3			; Note: Attr index is address/2
		move.l	d3,cu_CM+cm_BufferStart(a2)

		;--	    calculate cm_AttrToChar
		move.l	a0,d0			; calculate char buffer address
		add.l	cu_CM+cm_AllocSize(a2),d0
		sub.l	d5,d0			;   character buffer start
		sub.l	d3,d0
		move.l	d0,cu_CM+cm_AttrToChar(a2)

		;--	    setup for filling cm_BufferLines elements
		move.w	cu_CM+cm_BufferLines(a2),d0
		ext.l	d2			; cm_BufferWidth
		bra.s	wrAttrBufLinesDBF

		;--	    fill array
wrAttrBufLinesLoop:
		move.l	d3,(a0)+
		add.l	d2,d3
wrAttrBufLinesDBF:
		dbf	d0,wrAttrBufLinesLoop

		move.l	a0,cu_CM+cm_AttrDispLines(a2)

wrSetPens:

		;-- FIX!!  Now resets background
		;-- color for a reset

		clr.b	cu_BgColor(a2)
		bclr	#CUFB_FIXEDBG,cu_FixedFlags(a2)

		; Clear all bitplanes this time through

		moveq	#01,d0

		move.b	d0,cu_MinMask(a2)	;1 plane minimum for scrolls
		move.b	d0,cu_ScrollMask(a2)	;reset

		;initialize Primary drawing colors, modes, styles, etc

		move.b	d0,cu_NormFG(a2)		;pen 1
		clr.b	cu_NormBG(a2)			;pen 0
		clr.b	cu_NormStyle(a2)		;flag OFF

		move.b	#RP_JAM2,cu_NormDMode(a2)	;DrawMode

		bsr	sgrPrimary

		bsr	ReSizeUnit

		bclr	#CUB_FIRSTTIME,cu_States(a2)	;clear 1st time

		move.w	cu_XROrigin(a2),cd_RastPort+rp_cp_x(a6)
		move.w	cu_YROrigin(a2),d0
		add.w	cu_TxBaseline(a2),d0
		move.w	d0,cd_RastPort+rp_cp_y(a6)

		bsr	CursUpdate

		bsr	CursEnable

		moveq	#01,d2				;TRUE

wrAbort:
		bsr	UnLockRPort

		move.l	d2,d0				;CC set
		movem.l	(a7)+,d2-d5			;MOVEM (no touch CC)

		rts

wrBufAllocFail:
		bsr	FreeBuffer

	; continue if this is not the first time through this code

		btst	#CUB_FIRSTTIME,cu_States(a2)
		beq.s	wrSetPens

	;
	; else return failure for OpenDevice() - simple refresh without
	; a character map (as requested!) works so lousy, there is
	; little point in trying to use it - better for the user
	;

		moveq	#00,d2				;FAILED
		bra.s	wrAbort

*------ GetBorders ---------------------------------------------------
*
*   a2	unit data
*
*   d6,d7: width, height returned
*
GetBorders:
		move.l	cu_Window(a2),a0
		move.l	wd_Flags(a0),d0
		and.l	#GIMMEZEROZERO,d0
		beq.s	noGimme00

		btst	#CUFB_FIXEDLO,cu_FixedFlags(a2)
		bne.s	fixedXRO00
		clr.w	cu_XROrigin(a2)
fixedXRO00:
		move.w	wd_Width(a0),d6			; width
		move.b	wd_BorderLeft(a0),d1		; left
		ext.w	d1
		sub.w	d1,d6
		move.b	wd_BorderRight(a0),d1
		ext.w	d1
		sub.w	d1,d6
		sub.w	cu_XROrigin(a2),d6

		btst	#CUFB_FIXEDTO,cu_FixedFlags(a2)
		bne.s	fixedYRO00
		clr.w	cu_YROrigin(a2)
fixedYRO00:
		move.w	wd_Height(a0),d7		; height
		move.b	wd_BorderTop(a0),d1		; top
		ext.w	d1
		sub.w	d1,d7
		move.b	wd_BorderBottom(a0),d1
		ext.w	d1
		sub.w	d1,d7
		sub.w	cu_XROrigin(a2),d7
		bra.s	gotBorders

noGimme00:
		btst	#CUFB_FIXEDLO,cu_FixedFlags(a2)
		bne.s	fixedXRO
		move.b	wd_BorderLeft(a0),d1		; left
		ext.w	d1
		move.w	d1,cu_XROrigin(a2)
fixedXRO:
		move.w	wd_Width(a0),d6			; width
		sub.w	cu_XROrigin(a2),d6
		move.b	wd_BorderRight(a0),d1
		ext.w	d1
		sub.w	d1,d6

		btst	#CUFB_FIXEDTO,cu_FixedFlags(a2)
		bne.s	fixedYRO
		move.b	wd_BorderTop(a0),d1		; top
		ext.w	d1
		move.w	d1,cu_YROrigin(a2)
fixedYRO:
		move.w	wd_Height(a0),d7		; height
		sub.w	cu_YROrigin(a2),d7
		move.b	wd_BorderBottom(a0),d1
		ext.w	d1
		sub.w	d1,d7

gotBorders:
		tst.w	d6
		bpl.s	gbBoundD7
		clr.w	d6
gbBoundD7:
		tst.w	d7
		bpl.s	gbRts
		clr.w	d7
gbRts:
		rts


*------ ReSizePost ---------------------------------------------------
*
*   a2	unit data
*
ReSizePost:
		movem.l	d6/d7,-(a7)
		bsr	GetBorders

		cmp.w	cu_XMinShrink(a2),d6
		bge.s	postY
		move.w	d6,cu_XMinShrink(a2)
postY:
		cmp.w	cu_YMinShrink(a2),d7
		bge.s	postPost
		move.w	d7,cu_YMinShrink(a2)
postPost:
		bset	#CUB_RESIZE,cu_Flags(a2)
		movem.l	(a7)+,d6/d7
		rts


*------ ReSizeUnit ---------------------------------------------------
*
*   a2	console unit
*
ReSizeUnit:

		movem.l	d2-d7,-(a7)


	    ;-- pack character map
		tst.l	cu_CM+cm_AllocSize(a2)	; check if map exists
		beq.s	rsuGetBorders		;   no map to pack

		tst.b	cu_SpecialModes(a2)	; maybe no redraw needed
		beq.s	rsuPackMap

		; Use the ResetBuffer call which sorts the off-screen
		; map before clearing BufferXL/YL - there is now
		; nothing left in the off-screen map after this call

		bsr	ResetBuffer

		; lie - say there is nothing in the display either

		bsr	ResetXY

		; We get a real fast pack, and unpack in this case,
		; But we also have to make sure that these selection
		; bits are cleared out since this code won't be
		; executed when the special NODRAW on resize
		; flag is set.

		;	    (packSorted clears selection as side effect)
		bclr	#CUB_SELECTED,cu_Flags(a2)
		beq.s	rsuPackMap

		and.b	#CDS_SELECTMASK,cd_SelectFlags(a6)
		clr.l	cd_SelectedUnit(a6)
		bclr	#CUB_CURSSELECT,cu_CursorFlags(a2)

rsuPackMap:
		bsr	PackMap

rsuGetBorders:

		bsr	GetBorders		; also sets ROrigin

		move.w	#9999,d4		; initialize large
		move.w	d4,d5

	    ;-- clear out right edge if this is buffered or smart refresh
**		tst.l	cu_CM+cm_AllocSize(a2)	; check if buffered
**		bne.s	rsuClearRightEdge

		move.l	cu_Window(a2),a0
		move.l	wd_Flags(a0),d0
		and.l	#REFRESHBITS,d0

		cmpi.l	#SUPER_BITMAP,d0
		beq.s	rsuRebound

	;-- clear out right, and bottom edge (calcing limits below) for
	;-- smart refresh, or even simple refresh which lacks a character map
	;-- 
	;-- Unlike the previous code which tried to do a clear bottom edge
	;-- only for SIMPLE_REFRESH, this also does right edge (why not???),
	;-- but works better because we calc D2/D3 instead of using junk -
	;-- certainly no worse than before this fix which cleared to D2/D3
	;-- which was junk!


	    ;-- clear the right edge
rsuClearRightEdge:
		;-- get the origin of the last destroyed (leftmost) char
		moveq	#0,d0
		move.w	cu_XMinShrink(a2),d0
		move.w	cu_XRSize(a2),d1
		divu	d1,d0
		move.w	d0,d4			; save origin of clearing
		mulu	cu_XRSize(a2),d0
		add.w	cu_XROrigin(a2),d0

		;-- get the lessor of the old x region or the smaller new one
		move.w	d6,d2
		add.w	cu_XROrigin(a2),d2
		subq	#1,d2
		cmp.w	cu_XRExtant(a2),d2
		ble.s	rsucre1
		move.w	cu_XRExtant(a2),d2
rsucre1:
		move.w	cu_YROrigin(a2),d1
		;-- get the lessor of the old y region or the smaller new one
		move.w	d7,d3
		add.w	cu_YROrigin(a2),d3
		subq	#1,d3
		cmp.w	cu_YRExtant(a2),d3
		ble.s	rsucre2
		move.w	cu_YRExtant(a2),d3
rsucre2:
		bsr	ClearRaster

	    ;-- clear out bottom edge
		;-- get the origin of the last destroyed (topmost) char
rsuClearBottomEdge:
		moveq	#0,d1
		move.w	cu_YMinShrink(a2),d1
		move.w	cu_YRSize(a2),d0
		divu	d0,d1
		move.w	d1,d5			; save origin of clearing
		mulu	cu_YRSize(a2),d1
		add.w	cu_YROrigin(a2),d1
		move.w	cu_XROrigin(a2),d0

		bsr	ClearRaster


	;-- rebound the window
rsuRebound:
		bclr	#CUB_TOOSMALL,cu_Flags(a2)
		moveq	#0,d0
		move.w	d6,d0
		move.w	cu_XRSize(a2),d1
		divu	d1,d0
		subq.w	#1,d0
		btst	#CUFB_FIXEDLL,cu_FixedFlags(a2)
		beq.s	xHaveMax
		cmp.w	cu_FixedXMax(a2),d0
		ble.s	xHaveMax
		move.w	cu_FixedXMax(a2),d0
xHaveMax:
		move.w	d0,cu_XMax(a2)
		bpl.s	xExtant
		bset	#CUB_TOOSMALL,cu_Flags(a2)
		bra.s	tooSmallBailOut
xExtant:
		move.w	cu_XMax(a2),d0
		addq.w	#1,d0
		mulu	cu_XRSize(a2),d0
		subq.w	#1,d0
		move.w	cu_XROrigin(a2),d1
		add.w	d1,d0
		move.w	d0,cu_XRExtant(a2)
		move.w	d0,d2

		btst	#CUFB_FIXEDLL,cu_FixedFlags(a2)
		bne.s	maxxedge
		move.w	d6,d2
		add.w	d1,d2
		subq.w	#1,d2
maxxedge:
		move.w	d2,cu_FullXRExtant(a2)
		move.w	d1,cu_ClipRect+ra_MinX(a2)
		move.w	d0,cu_ClipRect+ra_MaxX(a2)

		moveq	#0,d0
		move.w	d7,d0
		move.w	cu_YRSize(a2),d1
		divu	d1,d0
		subq.w	#1,d0
		btst	#CUFB_FIXEDPL,cu_FixedFlags(a2)
		beq.s	yHaveMax
		cmp.w	cu_FixedYMax(a2),d0
		ble.s	yHaveMax
		move.w	cu_FixedYMax(a2),d0
yHaveMax:
		move.w	d0,cu_YMax(a2)
		bpl.s	yExtant
		bset	#CUB_TOOSMALL,cu_Flags(a2)

tooSmallBailOut:
		
		bsr	ResetXY
		clr.l	cu_CursorPattern(a2)	; pattern cleared
						; pattern unknown

		btst	#CUB_FIRSTTIME,cu_States(a2)
		beq	reprimeShrink

**
** These flags are no longer defined -- Darren (cursor kludge removed
** for ROM space V39)
**
**		bclr	#CUB_CURSNDRAW,cu_CursorFlags(a2)
**		bclr	#CUB_CURSQDRAW,cu_CursorFlags(a2)

		tst.l	cu_CM+cm_AllocSize(a2)	; check if buffered
		beq	reprimeShrink

		bsr	UnpackMap
		bra	reprimeShrink

yExtant:

		move.w	cu_YMax(a2),d0
		addq.w	#1,d0
		mulu	cu_YRSize(a2),d0
		subq	#1,d0
		move.w	cu_YROrigin(a2),d1
		add.w	d1,d0
		move.w	d0,cu_YRExtant(a2)
		move.w	d0,d2

		btst	#CUFB_FIXEDPL,cu_FixedFlags(a2)
		bne.s	maxyedge

		move.w	d7,d2
		add.w	d1,d2
		subq.w	#1,d2
maxyedge:
		move.w	d2,cu_FullYRExtant(a2)
		move.w	d1,cu_ClipRect+ra_MinY(a2)
		move.w	d0,cu_ClipRect+ra_MaxY(a2)

	;-- set the ClipRegion
		move.l	cu_ClipRegion(a2),a0
		LINKGFX	ClearRegion
		move.l	cu_ClipRegion(a2),a0
		lea	cu_ClipRect(a2),a1
		LINKGFX	OrRectRegion

		tst.l	cu_CM+cm_AllocSize(a2)	; check if buffered
		beq.s	rsuDisableCursor
		
		;-- refresh unit
		bsr	CursDisable
		bsr	UnpackMap

		;-- install the clip region
		bset.b	#CDB_CLIPINSTALLED,cd_Flags(a6)
		bne.s	rsuRefreshUnit

		move.l	cd_RastPort+rp_Layer(a6),a0
		move.l	cu_ClipRegion(a2),a1
		LINKLAY	InstallClipRegion
		move.l	d0,cd_PrevClipRegion(a6)

rsuRefreshUnit:
		bsr	RefreshUnit

		;-- throw away any damage
		move.l	cd_RastPort+rp_Layer(a6),a0
		move.l	lr_DamageList(a0),d0
		beq.s	rsuNoDamage
		move.l	d0,a0
		LINKGFX	ClearRegion
rsuNoDamage:
		bsr	ThrowAwayDamage


		bra.s	restoreCursor

	;-- disable the cursor, i.e. insure it is not imaged
rsuDisableCursor
		bsr	ThrowAwayDamage

		move.w	cu_XCP(a2),d0
		cmp.w	d0,d4		; check if already cleared
		ble.s	rsuCursorAlreadyClear 
		move.w	cu_YCP(a2),d1
		cmp.w	d1,d5		; check if already cleared
		ble.s	rsuCursorAlreadyClear 
		bsr	CursDisable	; perform a real disable
		bra.s	repositionCursor

rsuCursorAlreadyClear:
		addq.b	#1,cu_CDNestCnt(a2)	; fake a disable
		clr.l	cu_CursorPattern(a2)	; FIX!! - we dont really
						; have a cursor pattern now

	;-- reposition the cursor if this is not a superbitmap window
repositionCursor:
		move.l	cu_Window(a2),a0
		move.l	wd_Flags(a0),d0
		and.l	#SUPER_BITMAP,d0
		bne.s	restoreCursor

		move.w	cu_XCP(a2),d0
		cmp.w	cu_XMax(a2),d0
		ble.s	cursXOK

		;-- FIX!! get rid of double cursors

		addq.w	#1,cu_YCP(a2)
		move.w	cu_XMax(a2),cu_XCP(a2)
		move.w	cu_XMax(a2),cu_XCCP(a2)

cursXOK:
		move.w	cu_YCP(a2),d0
		cmp.w	cu_YMax(a2),d0
		ble.s	restoreCursor

		clr.w	cu_XCP(a2)
		clr.w	cu_XCCP(a2)		;FIX!! for double cursors

		move.w	cu_YMax(a2),cu_YCP(a2)
		move.w	cu_YCP(a2),cu_YCCP(a2)	;FIX as above

		moveq	#1,d0
		bsr	ScrollYDisplay

restoreCursor:
		bsr	CursEnable

reprimeShrink:
		move.w	#9999,cu_XMinShrink(a2)	; absurdly large, yet not large
		move.w	#9999,cu_YMinShrink(a2)	;   enough such that adding a
						;   valid origin makes them go
						;   negative
		
		bclr	#CUB_RESIZE,cu_Flags(a2)

		bclr	#CUB_REFRESH,cu_Flags(a2)

		;save width, and height within lock layers

		move.l	cu_Window(a2),a0
		move.w	wd_Width(a0),cu_WWidth(a2)
		move.w	wd_Height(a0),cu_WHeight(a2)

		movem.l (a7)+,d2-d7
		rts


*------ Throw Away Damage areas ----------------------------------------------
*
*   a2	console unit
*
ThrowAwayDamage:

		move.l	cd_RastPort+rp_Layer(a6),a0
		LINKLAY	BeginUpdate

		move.l	cd_RastPort+rp_Layer(a6),a0
		LINKLAY	EndUpdate
		move.l	cd_RastPort+rp_Layer(a6),a0
		and.w	#~LAYERREFRESH,lr_Flags(a0)
		rts

*------ ResetXY ----------------------------------------------
*
*   a2	console unit
*

ResetXY:
		clr.l	cu_XCP(a2)		; and cu_YCP
		clr.l	cu_XCCP(a2)		; and cu_YCCP
		clr.l	cu_XMax(a2)		; and cu_YMax
		clr.l	cu_DisplayXL(a2)	; and DisplayYL

		rts

	END


