**
**	$Id: cursor.asm,v 36.29 92/06/16 10:47:43 darren Exp $
**
**      enable and render console cursor
**
**      (C) Copyright 1985,1987,1989,1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"intuition/intuition.i"
	INCLUDE	"intuition/intuitionbase.i"

	INCLUDE	"debug.i"

**	Exports

	XDEF	CursOn
	XDEF	CursOff
	XDEF	CursRefresh
	XDEF	CursRender
	XDEF	CursDisable
	XDEF	CursEnable
	XDEF	CursUpdate
	XDEF	CursUp
	XDEF	CursDown
	XDEF	CursLeft
	XDEF	CursRight

	XDEF	IfGhostCursor

**	Imports

	XLVO	ReadPixel		; Graphics
	XLVO	RectFill		;
	XLVO	SetDrMd			;
	XLVO	SetAPen			;

	XLVO	InstallClipRegion	; Layers

	XREF	ScrollYDisplay


**	Assumptions

*------ IfGhostCursor ----------------------------------------------
*
*   NAME
*	IfGhostCursor - check if ghosting is allowed for this window
*			we can't properly ghost simple/non mapped type
*
*   SYNOPSIS
*	zero flag (T/F) = IfGhostCursor(consoleUnit)
*	                                  a2
*	a0 - used as window ptr
*	d0 - used
*---------------------------------------------------------------------
IfGhostCursor:
		tst.l	cu_CM+cm_AllocSize(a2)	;buffered?
		bne.s	IfGhostExit
		
		move.l	cu_Window(a2),a0
		move.l	wd_Flags(a0),d0
		andi.l	#REFRESHBITS,d0
		cmpi.l	#SIMPLE_REFRESH,d0
				
IfGhostExit:	rts



*------ CursOn ------------------------------------------------------
*
*   NAME
*	CursOn - turn on text cursor
*
*   SYNOPSIS
*	CursOn(consoleUnit)
*	      a2
*
*---------------------------------------------------------------------

crPatterns:
		dc.l	$00000000	; off inactive unselected
		dc.l	$5555aaaa	; on  inactive unselected
		dc.l	$00000000	; off inactive selected
		dc.l	$5555aaaa	; on  inactive selected
		dc.l	$00000000	; off active   unselected
		dc.l	$ffffffff	; on  active   unselected
		dc.l	$00000000	; off active   selected
		dc.l	$0000ffff	; on  active   selected

CursRefresh:
		clr.l	cu_CursorPattern(a2)
		bra.s	CursRender

CursOn:
		bset	#CUB_CURSON,cu_CursorFlags(a2)

CursRender:
		;-- get cursor flags, ensuring CUF_CURSACTIVE set if active
		
		;-- special case for SIMPLE refresh, no map
		;-- always active cursor in this rare case - can't use EOR
		;-- drawing, and undrawing.

		bsr.s	IfGhostCursor
		beq.s	SetActive

		moveq	#0,d1

		btst	#CUB_ISACTIVE,cu_States(a2)
		beq.s	crGotActive

SetActive:
		moveq	#CUF_CURSACTIVE,d1
crGotActive:
		or.b	cu_CursorFlags(a2),d1

		;-- check for disabled cursor
crCheckNest:
		tst.b	cu_CDNestCnt(a2)
		bmi.s	crNotDisabled
		bclr	#CUB_CURSON,d1
crNotDisabled:

		;-- get desired pattern & xor needed to create it

		move.l	crPatterns(pc,d1.w),d0
		move.l	cu_CursorPattern(a2),d1
		move.l	d0,cu_CursorPattern(a2)
		eor.l	d0,d1

		beq.s	crDone		; zero xor means no change needed

		;--	ensure window big enough for any rendering
		btst	#CUB_TOOSMALL,cu_Flags(a2)
		bne.s	crtoosmall

		;-- render cursor
		movem.l	d2-d6,-(a7)

		;both .s!!!
		bsr.s	crGetCursRectangle
		bgt.s	crRectangleDone

		move.b	cd_RastPort+rp_DrawMode(a6),d6
		move.l	cd_AreaPtrn(a6),a0
		move.l	d1,(a0)
		addq.l	#1,d1
		beq.s	crSolid
		move.l	a0,d1

crSolid:
		move.l	d1,cd_RastPort+rp_AreaPtrn(a6)

		moveq	#RP_COMPLEMENT,d0
		lea	cd_RastPort(a6),a1
		LINKGFX	SetDrMd

		move.l	d4,d0
		move.l	d5,d1
		lea	cd_RastPort(a6),a1
		move.b	cu_CursorMask(a2),rp_Mask(a1)
		LINKGFX	RectFill

		lea	cd_RastPort(a6),a1
		move.b	cu_Mask(a2),rp_Mask(a1)
		moveq	#0,d0
		move.b	d6,d0
		LINKGFX	SetDrMd

crRectangleDone:
		movem.l	(a7)+,d2-d6

crDone:
		rts

crtoosmall:
		clr.l	cu_CursorPattern(a2)	;reset cursor pattern

		rts


;------ crGetCursRectangle
;
;   RESULT
;   d2,d3	rectangle max X Y
;   d4,d5	rectangle min X Y
;   ccr		"gt" indicates empty rectangle
;
crGetCursRectangle:
		move.w	cu_XCCP(a2),d4
		mulu	cu_XRSize(a2),d4
		add.w	cu_XROrigin(a2),d4
		move.w	d4,d2
		add.w	cu_XRSize(a2),d2
		subq.w	#1,d2
		cmp.w	cu_XRExtant(a2),d2
		ble.s	crgcrXOK
		move.w	cu_XRExtant(a2),d2
		cmp.w	d2,d4
		bgt.s	crgcrDone
crgcrXOK:
		move.w	cu_YCCP(a2),d5
		mulu	cu_YRSize(a2),d5
		add.w	cu_YROrigin(a2),d5
		move.w	d5,d3
		add.w	cu_YRSize(a2),d3
		subq.w	#1,d3
		cmp.w	cu_YRExtant(a2),d3
		ble.s	crgcrDone
		move.w	cu_YRExtant(a2),d3
		cmp.w	d3,d5
crgcrDone:
		rts


*------ CursOff -----------------------------------------------------
*
*   NAME
*	CursOff - turn off text cursor
*
*   SYNOPSIS
*	CursOff(consoleUnit)
*		a2
*
*---------------------------------------------------------------------
CursOff:
		bclr	#CUB_CURSON,cu_CursorFlags(a2)
		bra	CursRender


*------ CursDisable -------------------------------------------------
*
*   NAME
*	CursDisable - turn off text cursor temporarily
*
*   SYNOPSIS
*	CursDisable(consoleUnit)
*		    a2
*
*---------------------------------------------------------------------
CursDisable:
		addq.b	#1,cu_CDNestCnt(a2)
		bgt.s	cdDeferred

cdSaveRegsRender:
		movem.l d0-d1/a0-a1,-(sp)
		bsr	CursRender
		movem.l	(sp)+,d0-d1/a0-a1
cdDeferred:
		rts


*------ CursEnable --------------------------------------------------
*
*   NAME
*	CursEnable - cancel the effect of a CursDisable
*
*   SYNOPSIS
*	CursEnable(consoleUnit)
*		   a2
*
*---------------------------------------------------------------------
CursEnable:
		subq.b	#1,cu_CDNestCnt(a2)
		bpl.s	cdDeferred
		bra.s	cdSaveRegsRender


*------ CursUpdate --------------------------------------------------
*
*   NAME
*	CursUpdate - update cursor at new position
*
*   SYNOPSIS
*	CursUpdate(consoleUnit)
*		   a2
*
*---------------------------------------------------------------------
CursUpdate:
		moveq	#0,d0
		move.l	cu_XCP(a2),d1
		cmp.l	cu_XCCP(a2),d1
		beq.s	cupNoChange

		and.b	#(~(CUF_IMPLICITNL!CUF_TABBED))&$ff,cu_Flags(a2)
		bsr.s	CursDisable
		move.l	d1,cu_XCCP(a2)
		bsr.s	CursEnable
cupNoChange:
		rts


*------ CursUp ------------------------------------------------------
*
*   NAME
*	CursUp - cursor position up
*
*   SYNOPSIS
*	CursUp(consoleUnit)
*	       a2
*
*---------------------------------------------------------------------
CursUp:
		move.w	cu_YCP(a2),d1
		sub.w	d0,d1
		bcs.s	cUpWrap
		move.w	d1,cu_YCP(a2)
		bra.s	CursUpdate

cUpWrap:
		clr.w	cu_YCP(a2)
		btst	#(PMB_ASM&07),cu_Modes+(PMB_ASM/8)(a2)
		beq.s	cVertBound
	;-- scroll for the remaining
scrollVert:
		move.w	d1,d0
		bsr	ScrollYDisplay
cVertBound:
		bra.s	CursUpdate


*------ CursDown ----------------------------------------------------
*
*   NAME
*	CursDown - cursor position down
*
*   SYNOPSIS
*	CursDown(consoleUnit)
*		 a2
*
*---------------------------------------------------------------------
CursDown:
		move.w	cu_YCP(a2),d1
		add.w	d0,d1
		cmp.w	cu_YMax(a2),d1
		bhi.s	cDownWrap
		move.w	d1,cu_YCP(a2)
		bra.s	CursUpdate

cDownWrap:
	;-- scroll for the remaining
		move.w	cu_YMax(a2),cu_YCP(a2)
		btst	#(PMB_ASM&07),cu_Modes+(PMB_ASM/8)(a2)
		beq.s	cVertBound
		sub.w	cu_YMax(a2),d1
		bra.s	scrollVert


*------ CursRight ---------------------------------------------------
*
*   NAME
*	CursRight - cursor position right
*
*   SYNOPSIS
*	CursRight(consoleUnit)
*		  a2
*
*---------------------------------------------------------------------
CursRight:
		add.w	cu_XCP(a2),d0
		cmp.w	cu_XMax(a2),d0
		bhi.s	cRightWrap
		move.w	d0,cu_XCP(a2)
cRightBound:
		bra.s	CursUpdate

cRightWrap:
		btst	#(PMB_AWM&07),cu_Modes+(PMB_AWM/8)(a2)
		bne.s	cRightAutoWrap
		move.w	cu_XMax(a2),cu_XCP(a2)
		bra.s	cRightBound
cRightAutoWrap:
	;-- wrap for the remaining
		clr.w	cu_XCP(a2)
		sub.w	cu_XMax(a2),d0
		subq	#1,d0
		move.w	d0,-(a7)
		moveq	#1,d0
		bsr.s	CursDown
		move.w	(a7)+,d0
		bne.s	CursRight
		bset	#CUB_IMPLICITNL,cu_Flags(a2)
		rts


*------ CursLeft ----------------------------------------------------
*
*   NAME
*	CursLeft - cursor position left
*
*   SYNOPSIS
*	CursLeft(consoleUnit)
*		 a2
*
*---------------------------------------------------------------------
CursLeft:
		move.w	cu_XCP(a2),d1
		sub.w	d0,d1
		bcs.s	cLeftWrap
		move.w	d1,cu_XCP(a2)
cLeftBound:
		bra	CursUpdate

cLeftWrap:
		btst	#(PMB_AWM&07),cu_Modes+(PMB_AWM/8)(a2)
		bne.s	cLeftAutoWrap
		clr.w	cu_XCP(a2)
		bra.s	cLeftBound
cLeftAutoWrap:
	;-- wrap for the remaining
		move.w	cu_XMax(a2),cu_XCP(a2)
		neg.w	d1
		subq	#1,d1
		move.w	d1,-(a7)
		moveq	#1,d0
		bsr	CursUp
		move.w	(a7)+,d0
		bra.s	CursLeft

	END
