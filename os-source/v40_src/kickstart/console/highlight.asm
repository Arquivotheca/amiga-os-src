**
**	$Id: highlight.asm,v 36.11 92/03/23 12:44:35 darren Exp $
**
**      manipulate console highlighting
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"intuition/intuition.i"


**	Exports

	XDEF	UpdateHighlight


**	Imports

	XLVO	RectFill		; Graphics
	XLVO	SetDrMd			;

	XREF	CursDisable
	XREF	CursEnable


**	Assumptions

	IFNE	CMAB_HIGHLIGHT-CMAB_SELECTED-1
	FAIL	"HIGHLIGHT not the bit above SELECTED, recode below"
	ENDC


*------	UpdateHighlight ----------------------------------------------
*
*   a2	unit data
*

;
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
UpdateHighlight:
		tst.l	cu_CM+cm_AllocSize(a2)
		beq	uhRts

		btst	#CUB_TOOSMALL,cu_Flags(a2)
		bne	uhRts

		bsr	CursDisable

		;-- set drawing mode to complement
		moveq	#0,d0
		move.b	cd_RastPort+rp_DrawMode(a6),d0
		lea	cd_RastPort(a6),a1

		movem.l	d0/d2-d7/a1/a3-a4,-(a7)	; save registers

		moveq	#RP_COMPLEMENT,d0
		LINKGFX	SetDrMd

		;-- highlight with mask for AA machines

		move.b	cu_CursorMask(a2),cd_RastPort+rp_Mask(a6)

		;-- set area pattern to solid
		clr.l	cd_RastPort+rp_AreaPtrn(a6)

		;-- initialize buffer variables
		move.w	cu_CM+cm_DisplayWidth(a2),d5	; first unused cell
		move.w	cu_DisplayYL(a2),d6
		move.l	cu_CM+cm_AttrDispLines(a2),a4

		;-- initialize Y raster position
		move.w	cu_YROrigin(a2),d7	
		sub.w	cu_YRSize(a2),d7

uhRowLoop:
		moveq	#0,d4			; index of first cell
		tst.w	d6
		bne.s	uhHaveXMax
		move.w	cu_DisplayXL(a2),d5	; index of first unused cell
uhHaveXMax:
		add.w	cu_YRSize(a2),d7
		move.l	(a4)+,a3		; get this attribute line (/2)
		add.l	a3,a3

uhColLoop:
		move.w	#CMAF_HIGHLIGHT!CMAF_SELECTED,d3
		bra.s	uhSkipEntry

		;--	scan chars that don't need updating
uhSkipLoop:
		addq.w	#1,d4			; update to current cell
uhSkipEntry:
		cmp.w	d4,d5
		ble.s	uhRowDBF
		move.w	(a3)+,d1
		and.w	d3,d1
		beq.s	uhSkipLoop		; HIGHLIGHT & SELECTED clear
		eor.w	d3,d1
		beq.s	uhSkipLoop		; HIGHLIGHT & SELECTED set

		;--	scan chars that need updating
		move.w	d4,d0			; save X start index
		subq.l	#2,a3			; rewind attr to match index

uhUpdateLoop:
		eori.w	#CMAF_HIGHLIGHT,(a3)+
		addq.w	#1,d4
		cmp.w	d4,d5
		ble.s	uhUpdate
		move.w	(a3),d1
		and.w	d3,d1
		beq.s	uhUpdate
		eor.w	d3,d1
		bne.s	uhUpdateLoop

uhUpdate:
		;--	d4,a3 point to cell beyond that to update
		;--	perform update
		mulu	cu_XRSize(a2),d0	; calculate X start position
		add.w	cu_XROrigin(a2),d0

		move.w	d7,d1			; get Y start position

		move.w	d4,d2			; calculate X end position
		mulu	cu_XRSize(a2),d2
		add.w	cu_XROrigin(a2),d2
		subq.w	#1,d2
		tst.w	-2(a3)			; check if last is rendered
		bmi.s	uhYEnd
		move.w	cu_XRSize(a2),d3
		lsr.w	#1,d3
		sub.w	d3,d2

uhYEnd:
		move.w	cu_YRSize(a2),d3	; calculate Y end position
		add.w	d1,d3
		subq.w	#1,d3

		lea	cd_RastPort(a6),a1

		LINKGFX RectFill

		bra.s	uhColLoop

uhRowDBF:
		dbf	d6,uhRowLoop

		movem.l	(a7)+,d0/d2-d7/a1/a3-a4	; restore registers
		LINKGFX	SetDrMd

		;-- restore mask

		move.b	cu_Mask(a2),cd_RastPort+rp_Mask(a6)

		bsr	CursEnable

uhRts:
		rts


	END
