**
**	$Id: refresh.asm,v 36.29 92/03/20 14:37:30 darren Exp $
**
**      refresh a console unit
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"intuition/intuition.i"

	INCLUDE	"debug.i"

**	Exports

	XDEF	RefreshUnit
	XDEF	RefreshDamage
	XDEF	IfNewSize

**	Imports

	XREF	_custom

	XLVO	AndRectRegion		; Graphics
	XLVO	SetAPen			;
	XLVO	SetBPen			;
	XLVO	SetDrMd			;
	XLVO	SetSoftStyle		;
	XLVO	Text			;

	XLVO	BeginUpdate		; Layers
	XLVO	EndUpdate		;

	XREF	LockDRPort
	XREF	UnLockRPort

	XREF	ClearScreen
	XREF	CursRefresh
	XREF	RestoreRP

	XREF	UpdateHighlight

	XREF	ReSizeUnit		;wreset

*------ RefreshDamage ------------------------------------------------
*
*   a2	unit data
*
RefreshDamage:
		bsr	LockDRPort

		;-- check for window size changes

		bsr.s	IfNewSize
		bne.s	rdResize

		btst	#CUB_TOOSMALL,cu_Flags(a2)
		bne.s	rdUnlock

		move.l	cd_RastPort+rp_Layer(a6),a0
		move.l	lr_DamageList(a0),d0
		beq.s	rdBeginUpdate

		move.l	d0,a0
		lea	cu_ClipRect(a2),a1
		LINKGFX	AndRectRegion

rdBeginUpdate:
		move.l	cd_RastPort+rp_Layer(a6),a0
		LINKLAY	BeginUpdate
		tst.l	d0
		beq.s	rdAbortUpdate

		tst.l	cu_CM+cm_AllocSize(a2)	; check if map exists
		beq.s	rdEndUpdate

		bsr.s	RefreshUnit

rdEndUpdate:
		moveq	#1,d0
		move.l	cd_RastPort+rp_Layer(a6),a0
		LINKLAY	EndUpdate    

		move.l	cd_RastPort+rp_Layer(a6),a0
		and.w	#~LAYERREFRESH,lr_Flags(a0)

rdUnlock:
		bsr	UnLockRPort

rdDone:
		rts


rdAbortUpdate:
		move.l	cd_RastPort+rp_Layer(a6),a0
		LINKLAY	EndUpdate
		bra.s	rdUnlock

rdResize:
		bsr	ReSizeUnit
		bra.s	rdUnlock


*------ Check for a newsize condition --------------------------------
*
*   a2  unit data
*
*   Quick check for RESIZE flag followed by a check for window
*   width, and height compared against last cached values.
*
*   != means a return of TRUE (yes, newsize)
*   == means a return of FALSE (not newsize)
*
IfNewSize:
		btst	#CUB_RESIZE,cu_Flags(a2)
		bne.s	newsize

		movea.l	cu_Window(a2),a0
		move.w	wd_Height(a0),d0
		cmp.w	cu_WHeight(a2),d0
		bne.s	newsize

		move.w	wd_Width(a0),d0
		cmp.w	cu_WWidth(a2),d0

newsize:	rts
				

*------ RefreshUnit --------------------------------------------------
*
*   a2	unit data
*

;
;   d3	current attributes
;   d4	incrementing X line position
;   d5	X max
;   d6	decrementing Y line counter
;   d7	Y raster position
;
;   a2	console unit
;   a3	running attribute line
;   a4	cm_AttrDispLines array
;   a6	console device
;
RefreshUnit:
		movem.l	d2-d7/a3-a4,-(a7)

		btst	#CUB_TOOSMALL,cu_Flags(a2)
		bne	ruDone

	;-- Refresh everything, regardless of concealed mode, or
	;-- optimized scrolling

		move.l	cu_Mask(a2),cd_RastPort+rp_Mask(a6)

		move.b	#$FF,cd_RastPort+rp_Mask(a6)

		move.l	cu_Font(a2),cd_RastPort+rp_Font(a6)
		move.l	cu_TxHeight(a2),cd_RastPort+rp_TxHeight(a6)
		move.l	cu_TxBaseline(a2),cd_RastPort+rp_TxBaseline(a6)

	;-- first pass at this problem: render all text

		bsr	ClearScreen

		;-- initialize variables
		move.w	cu_CM+cm_DisplayWidth(a2),d5
		move.w	cu_DisplayYL(a2),d6
		move.w	cu_YROrigin(a2),d7
		add.w	cu_TxBaseline(a2),d7
		sub.w	cu_YRSize(a2),d7

		move.l	cu_CM+cm_AttrDispLines(a2),a4

		moveq	#0,d2			; clear high bytes
		moveq	#0,d3			; current attrs

	    ;-- render this row
ruRowLoop:
		moveq	#0,d4
		tst.w	d6
		bne.s	ruHaveXMax
		move.w	cu_DisplayXL(a2),d5
ruHaveXMax:
		add.w	cu_YRSize(a2),d7
		move.l	(a4)+,a3
		add.l	a3,a3

		;-- find start of characters to render
ruColLoop:
		addq.w	#1,d4
		cmp.w	d4,d5
		blt	ruRowDBF
		move.w	(a3),d2
		and.w	#~CMAF_HIGHLIGHT,d2
		move.w	d2,(a3)+
		bpl.s	ruColLoop

		;-- ensure the attributes for the first character are set
		move.l	d2,d0
		and.w	#CMAM_SOFTSTYLE,d0
		move.w	d3,d1
		and.w	#CMAM_SOFTSTYLE,d1
		cmp.w	d0,d1
		bne.s	ruSetSoftStyle
		tst.w	d3
		bmi.s	ruChkDrawMode
ruSetSoftStyle:
		lsr.w	#CMAS_SOFTSTYLE,d0
		lea	cd_RastPort(a6),a1
		moveq	#(CMAM_SOFTSTYLE>>CMAS_SOFTSTYLE),d1
		LINKGFX	SetSoftStyle

ruChkDrawMode:
		move.l	d2,d0
		and.w	#CMAF_INVERSVID,d0
		move.w	d3,d1
		and.w	#CMAF_INVERSVID,d1
		cmp.w	d0,d1
		bne.s	ruSetDrawMode
		tst.w	d3
		bmi.s	ruAttrCurrent
ruSetDrawMode:
		tst.w	d0
		beq.s	ruStdDrawMode
		moveq	#RP_INVERSVID!RP_JAM2,d0
		bra.s	ruSetDrawModeCall
ruStdDrawMode:
		moveq	#RP_JAM2,d0
ruSetDrawModeCall:
		lea	cd_RastPort(a6),a1
		LINKGFX	SetDrMd

ruAttrCurrent:
		;--	ensure the color for the first character is set
		move.l	d2,d0
		and.w	#CMAM_FGPEN,d0
		move.w	d3,d1
		and.w	#CMAM_FGPEN,d1
		cmp.w	d0,d1
		bne.s	ruSetFGColor
		tst.w	d3
		bmi.s	ruChkBGColor
ruSetFGColor:
		lea	cd_RastPort(a6),a1
		LINKGFX	SetAPen

ruChkBGColor:
		move.l	d2,d0
		and.w	#CMAM_BGPEN,d0
		move.w	d3,d1
		and.w	#CMAM_BGPEN,d1
		cmp.w	d0,d1
		bne.s	ruSetBGColor
		tst.w	d3
		bmi.s	ruColorCurrent

ruSetBGColor:
		lsr.w	#CMAS_BGPEN,d0
		lea	cd_RastPort(a6),a1
		LINKGFX	SetBPen

ruColorCurrent:
		move.w	d2,d3			; save current attributes

		;-- ensure the text CP is correct
		lea	cd_RastPort(a6),a1
		move.w	d4,d1
		subq.w	#1,d1
		mulu	cu_XRSize(a2),d1
		add.w	cu_XROrigin(a2),d1
		move.w	d1,rp_cp_x(a1)
		move.w	d7,rp_cp_y(a1)

		;-- find the matching location in the character array
		move.l	a3,d0
		lsr.l	#1,d0
		subq.l	#1,d0
		add.l	cu_CM+cm_AttrToChar(a2),d0
		move.l	d0,a0

	    ;-- count the characters to render in this line
		move.w	d4,d1			; save start column + 1
ruCountLength:
		addq.w	#1,d4
		cmp.w	d4,d5
		blt.s	ruHaveLength
		move.w	(a3),d0
		and.w	#~CMAF_HIGHLIGHT,d0
		move.w	d0,(a3)+
		cmp.w	d0,d3
		beq.s	ruCountLength
ruHaveLength:
		move.w	d4,d0
		sub.w	d1,d0
		ext.l	d0

		LINKGFX	Text

		subq.w	#1,d4
		subq.l	#2,a3

		bra	ruColLoop

ruRowDBF:
		dbf	d6,ruRowLoop

		bsr	CursRefresh

		btst	#CUB_SELECTED,cu_Flags(a2)
		beq.s	ruRestoreRP

		bsr	UpdateHighlight

		;-- restore RastPort attributes
ruRestoreRP:
		bsr	RestoreRP

ruDone:
		bclr	#CUB_REFRESH,cu_Flags(a2)
		movem.l	(a7)+,d2-d7/a3-a4
		rts

	END
