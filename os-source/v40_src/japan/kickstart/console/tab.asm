**
**	$Id: tab.asm,v 36.6 91/03/14 17:13:23 darren Exp $
**
**      console tab control
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"


**	Exports

	XDEF	InitTabs
	XDEF	Tab
	XDEF	BackTab
	XDEF	SetTab
	XDEF	ClearTab
	XDEF	ClearTabs


**	Imports

	XREF	CursUpdate
	XREF	CursDown
	XREF	CursUp


*------ InitTabs ----------------------------------------------------
*
*   NAME
*	InitTabs - initialize tab stops for a window
*
*   SYNOPSIS
*	InitTabs(consoleUnit)
*		 a2
*
*--------------------------------------------------------------------
InitTabs:
		moveq	#0,d0
		moveq	#MAXTABS-2,d1
		lea	cu_TabStops(a2),a0
initTabLoop:
		move.w	d0,(a0)+
		addq	#8,d0
		dbf	d1,initTabLoop

		move.w	#$FFFF,(a0)
		rts

		
*------ Tab ---------------------------------------------------------
*
*   NAME
*	Tab - tab forward
*
*   SYNOPSIS
*	Tab(consoleUnit)
*	    a2
*
*---------------------------------------------------------------------
Tab:
		bsr.s	findNextTab
		beq.s	atTab
		subq.l	#2,a0
atTab:
		move.w	(a0),d0
		cmp.w	cu_XMax(a2),d0
		bhi.s	tabWrap
		move.w	d0,cu_XCP(a2)
cTabBound:
		bsr	CursUpdate
cTabFlagged:
		bset	#CUB_TABBED,cu_Flags(a2)
		rts


tabWrap:
		btst	#(PMB_AWM&07),cu_Modes+(PMB_AWM/8)(a2)
		bne.s	cTabAutoWrap
		move.w	cu_XMax(a2),cu_XCP(a2)
		bra.s	cTabBound
cTabAutoWrap:
	;-- wrap for the remaining
		clr.w	cu_XCP(a2)
		moveq	#1,d0
		bsr	CursDown
		bset	#CUB_IMPLICITNL,cu_Flags(a2)
		bra.s	cTabFlagged


*---------------------------------------------------------------------

findNextTab:
		move.w	cu_XCP(a2),d0
		moveq	#-1,d1
		lea	cu_TabStops(a2),a0
loopNextTab:
		cmp.w	(a0)+,d0
		dbls	d1,loopNextTab
locRts:
		rts


*------ BackTab -----------------------------------------------------
*
*   NAME
*	BackTab - tab backward
*
*   SYNOPSIS
*	BackTab(consoleUnit)
*		a2
*
*---------------------------------------------------------------------
BackTab:
		tst.w	cu_XCP(a2)
		beq.s	backTabWrap
		bsr.s	findNextTab
		subq.l	#4,a0
		move.w	(a0),d0
		move.w	d0,cu_XCP(a2)
		bra	CursUpdate

backTabWrap:
		btst	#(PMB_AWM&07),cu_Modes+(PMB_AWM/8)(a2)
		beq.s	locRts
	;-- wrap for the remaining
		moveq	#1,d0
		bsr	CursUp
		move.w	cu_XMax(a2),d0
		addq	#1,d0
		move.w	d0,cu_XCP(a2)
		bra.s	BackTab

*------ SetTab ------------------------------------------------------
*
*   NAME
*	SetTab - set tab at current position
*
*   SYNOPSIS
*	SetTab(consoleUnit)
*	       a2
*
*---------------------------------------------------------------------
SetTab:
		bsr.s	findNextTab
		beq.s	locRts		; already set
		lea	cu_TabStops+((MAXTABS-1)*2)(a2),a0
		move.w	#$FFFF,(a0)	; ensure list is terminated
		subq.l	#2,a0

		add.w	#MAXTABS-1,d1
		bmi.s	stdone		; fix - can be neg if XCP == XMax
		bra.s	stDBF
stLoop:
		move.w	-(a0),2(a0)	; copy next over this one
stDBF:
		dbf	d1,stLoop
stdone:
		move.w	cu_XCP(a2),(a0)
		rts


*------ ClearTab ----------------------------------------------------
*
*   NAME
*	ClearTab - clear tab at current position
*
*   SYNOPSIS
*	ClearTab(consoleUnit)
*		 a2
*
*---------------------------------------------------------------------
ClearTab:
		tst.w	cu_XCP(a2)
		beq.s	locRts		; cannot clear tab at position 0
		bsr.s	findNextTab
		bne.s	locRts		; not set here
		add.w	#MAXTABS-1,d1
		bmi.s	ctdone		; safety check - if XCP == XMax
ctLoop:
		move.w	(a0),-2(a0)	; copy next over this one
		addq.l	#2,a0
		dbf	d1,ctLoop
ctdone:
		rts


*------ ClearTabs ---------------------------------------------------
*
*   NAME
*	ClearTabs - clear all tabs
*
*   SYNOPSIS
*	ClearTabs(consoleUnit)
*		  a2
*
*---------------------------------------------------------------------
ClearTabs:
		move.l	#$0000FFFF,cu_TabStops(a2)
		rts

		END
