**
**	$Id: able.asm,v 36.17 92/05/19 12:28:29 darren Exp $
**
**      acquire use of device rastport
**
**      (C) Copyright 1989,1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"debug.i"

	INCLUDE	"intuition/intuition.i"


**	Exports

	XDEF	LockDRPort
	XDEF	UnLockRPort


**	Imports

	XLVO	Forbid			; Exec
	XLVO	Permit			;
	XLVO	ObtainSemaphore		;
	XLVO	ReleaseSemaphore	;

	XLVO	LockLayerRom		; Graphics
	XLVO	UnlockLayerRom		;
	XLVO	SetDrMd			;

	XLVO	LockLayerInfo		; Layers
	XLVO	InstallClipRegion	;
	XLVO	UnlockLayerInfo		;

	XREF	ClearRaster		; clear.asm

	XREF	ObtainMapSemaphore
	XREF	ReleaseMapSemaphore

*------ LockDRPort ---------------------------------------------------
*
*   NAME
*	LockDRPort - lock the Window's RPort from the user for graphics
*
*   SYNOPSIS
*	LockDRPort(consoleUnit)
*		  a2
*
*---------------------------------------------------------------------
LockDRPort:
		lea	cd_RPSemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		addq.b	#1,cu_RPNestCnt(a2)
		bgt.s	ldrDone

		move.l	cu_Window(a2),a1
		move.l	wd_RPort(a1),a1
		lea	cd_RastPort(a6),a0
		move.l	rp_BitMap(a1),rp_BitMap(a0)
		move.l	rp_Layer(a1),a1
		move.l	a1,rp_Layer(a0)
		move.l	a0,lr_rp(a1)

		move.l	a5,-(a7)
		move.l	cd_RastPort+rp_Layer(a6),a5
		move.l	lr_LayerInfo(a5),a0
		LINKLAY	LockLayerInfo
		LINKGFX	LockLayerRom
		move.l	(a7)+,a5

ldrDone:

		rts


*------ UnLockRPort -------------------------------------------------
*
*   NAME
*	UnLockRPort - unlock the Window's RPort to the user
*
*   SYNOPSIS
*	UnLockRPort(consoleUnit)
*	            a2
*
*---------------------------------------------------------------------
UnLockRPort:
		LINKEXE	Forbid
		subq.b	#1,cu_RPNestCnt(a2)
		bpl.s	uDone

		bclr.b	#CDB_CLIPINSTALLED,cd_Flags(a6)
		beq.s	uUnlock

		move.l	cd_RastPort+rp_Layer(a6),a0
		move.l	cd_PrevClipRegion(a6),a1
		LINKLAY	InstallClipRegion

uUnlock:
	;-- check/clear delay border fill flag (typically not set)

		bclr	#CUB_BORDERFILL,cu_States(a2)
		bne.s	doborderfill

nodelayborderfill:
		move.l	a5,-(a7)
		move.l	cd_RastPort+rp_Layer(a6),a5
		LINKGFX	UnlockLayerRom
		move.l	lr_LayerInfo(a5),a0
		LINKLAY	UnlockLayerInfo
		move.l	(a7)+,a5

		move.l	cd_RastPort+rp_Layer(a6),a0
		move.l	cu_Window(a2),a1
		move.l	wd_RPort(a1),a1
		move.l	a1,lr_rp(a0)

uDone:
		LINKEXE	Permit

		lea	cd_RPSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore
		rts

	;-- delayed border fill - only safe time to do this without
	;-- fiddling with major portions of the code which draw inside
	;-- of a more restrictive clip region

doborderfill:


		move.w	cu_FullXRExtant(a2),d2
		move.w	cu_FullYRExtant(a2),d3

		cmp.w	cu_XRExtant(a2),d2
		bne.s	borderXR

checkYR:
		cmp.w	cu_YRExtant(a2),d3
		beq.s	nodelayborderfill

borderYR:
		move.w	cu_XROrigin(a2),d0
		move.w	cu_YRExtant(a2),d1
		addq.w	#1,d1
		bsr.s	ClearBorder
		bra	nodelayborderfill
borderXR:
		
		move.w	cu_XRExtant(a2),d0
		addq.w	#1,d0
		move.w	cu_YROrigin(a2),d1
		bsr.s	ClearBorder
		bra.s	checkYR		
ClearBorder:

	; ClearBorder with explicit draw mode

		movem.l	d0-d1,-(sp)
		lea	cd_RastPort(a6),a1
		move.b	cu_ClearDrMd(a2),d0

	; Use same drawing mode as when we last cleared the screen
	; incase we are in INVERSVID mode (esc[7m) when we cleared
	; the screen

		LINKGFX	SetDrMd
		movem.l	(sp)+,d0-d1

	; ClearRaster() checks for positive rectangle

		bra	ClearRaster
	;;;	rts

	END
