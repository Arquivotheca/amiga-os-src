********************************************************************************
*
*	$Id: LoadView.asm,v 39.1 93/09/21 16:51:22 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/lists.i'
	include	'exec/memory.i'
	include	'graphics/gfxbase.i'
	include	'graphics/view.i'

	include	"SpecialFXBase.i"
	include	"specialfx.i"
	include	"specialfx_internal.i"

	xdef	SFXLoadView
	xref	SfxBase
	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_LVOAllocVec
	xref	_LVOFreeVec
	xref	_LVOFreeMem
	xref	_LVOMakeVPort
	xref	_LVOFreeCopList

*
* SFXLoadView() calls the graphics.library LoadView() LVO. It then checks to see
* if either the GfxBase->Modes or GfxBase->current_monitor were changed by
* LoadView(). If they were, then we need to remove all the current UCopLists for
* the AnimHandles in this View, and reinstall them.
*

SFXLoadView:
	movem.l	d2-d3,-(sp)
	move.l	SfxBase(pc),a0
	move.l	sfxb_OldLoadView(a0),a0
	move.w	gb_Modes(a6),d2
	move.l	gb_current_monitor(a6),d3
	move.l	a1,-(sp)	; save the View
	jsr	(a0)		; call the old LoadView
	move.l	(sp)+,a1
	cmp.w	gb_Modes(a6),d2
	sne.w	d0
	cmp.l	gb_current_monitor(a6),d3
	sne.w	d1
	or.w	d1,d0		; d0 will be -1 if either the Modes or the Monitor changed
	movem.l	(sp)+,d2-d3

	; Look through the list of AnimHandles for those in this View.
	movem.l	a2-a6/d2-d7,-(sp)
	move.w	d0,d5		; store the 'change' flag
	move.l	SfxBase(pc),a2
	move.l	a1,d3		; d3 -> View

	; and get the ActiViewCprSemaphore
	move.l	sfxb_GfxBase(a2),a0
	move.l	sfxb_ExecBase(a2),a6
	move.l	gb_ActiViewCprSemaphore(a0),a0
	jsr	_LVOObtainSemaphore(a6)

	; get the AnimHandle semaphore
	move.l	sfxb_AHListSemaphore(a2),a0
	jsr	_LVOObtainSemaphore(a6)

	; walk through the AnimHandle list, looking for an AnimHandle with
	; the same View.
	lea	sfxb_AHList(a2),a0
search_ahlist:
	lea	ahl_AnimHandles(a0),a3
	move.w	ahl_Count(a0),d4
	bra.s	search_ahlist.
search_ahlist_next:
	move.l	(a3)+,d0
	beq.s	search_ahlist.
	move.l	d0,a4		; the AnimHandle
	cmp.l	ah_View(a4),d3	; the same View?
	beq.s	found_ah
search_ahlist.:
	dbra.s	d4,search_ahlist_next
	; wasn't in that list, so check the next.
	move.l	MLN_SUCC(a0),a0
	TSTLIST	a0
	bne.s	search_ahlist
	; is empty
	bra.s	all_coerced

found_ah:
	; call the rebuild vector for each FxHandle in the AnimHandle's list.

	movem.l	a0/a2/a3/d3/d4,-(sp)
	lea	AnimHandle_SIZEOF(a4),a5	; point to list of FxHandles
	move.l	ah_HandleCount(a4),d7
	bra.s	rebuild_fx.
rebuild_fx:
	move.l	(a5)+,a0

	; Do we need to recalculate the Offset?
	btst.b	#AHB_CALCOFFSET,ah_Flags+1(a4)
	bne.s	2$
	tst.l	fxh_Offset(a0)		; has the offset been calculated yet?
	bne.s	no_offset_calc
	; calculate the offset value
2$:
	move.l	fxh_UCopList(a0),a1
	move.l	ucl_FirstCopList(a1),a1
	move.l	cl_CopLStart(a1),d0	; first copper instruction of this effect
	move.l	ah_View(a4),a1
	move.l	v_LOFCprList(a1),a1
	sub.l	crl_start(a1),d0	; offset value
	move.l	d0,fxh_Offset(a0)
no_offset_calc:
	move.l	fxh_Type(a0),d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	sfxb_RebuildVectors(a2),a1
	move.l	-4(a1,d0),d0
	beq.s	rebuild_fx.
	move.l	d0,a1
	jsr	(a1)
rebuild_fx.:
	dbra.s	d7,rebuild_fx
	bclr.b	#AHB_CALCOFFSET,ah_Flags+1(a4)
	movem.l	(sp)+,a0/a2/a3/d3/d4
	bra	search_ahlist.

all_coerced:
	; Release the AHListSemaphore
	move.l	sfxb_AHListSemaphore(a2),a0
	move.l	sfxb_ExecBase(a2),a6
	move.l	sfxb_GfxBase(a2),a2
	jsr	_LVOReleaseSemaphore(a6)

	; and the ActiViewCprSenmaphore
	move.l	gb_ActiViewCprSemaphore(a2),a0
	jsr	_LVOReleaseSemaphore(a6)

	movem.l	(sp)+,a2-a6/d2-d7
SFXLoadView.:
	rts

