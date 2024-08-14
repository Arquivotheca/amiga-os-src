********************************************************************************
*
*	$Id: RemoveFX.asm,v 39.9 93/09/21 16:48:45 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/memory.i'
	include	'exec/ables.i'
	include 'graphics/copper.i'
	include	'graphics/view.i'
	include	'graphics/gfxbase.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX_internal.i"

	xdef	_LVORemoveFX
	xref	_LVOFreeCopList
	xref	_LVOWaitTOF
	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_LVOFreeMem
	xref	_intena

******* specialfx.library/RemoveFX *********************************************
*
*   NAME
*	RemoveFX -- Removes a 'Special Effect', and frees the memory.
*
*   SYNOPSIS
*	RemoveFX(Handle)
*	         a0
*
*	void RemoveFX(APTR);
*
*   FUNCTION
*	To remove the 'Special Effect' group, set up in InstallFXA().
*
*   INPUTS
*	Handle - the handle obtained from InstallFXA()
*
*   RESULT
*
*   NOTES
*	After calling RemoveFX(), you should remake the display, either
*	with intuition's MakeScreen()/RethinkDisplay() if using intuition
*	screens, or with graphics' MakeVPort()/MrgCop()/LoadView() if using
*	a custom View and ViewPort.
*
*	After calling this function, the Handle is no longer valid. If you want
*	to redisplay the effect, get a new handle from InstallFXA().
*	
*   SEE ALSO
*	InstallFXA()
*
********************************************************************************

_LVORemoveFX:
	movem.l	d2-d4/d7/a2-a6,-(sp)
	move.l	a0,d0
	beq	RemoveFX.
	move.l	a0,a2
	move.l	a6,d4
	; first, remove the UCopList from the ViewPort, safely!
	move.l	sfxb_GfxBase(a6),a4
	move.l	gb_ActiViewCprSemaphore(a4),a0
	move.l	sfxb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)
	move.l	ah_ViewPort(a2),a0
	move.l	ah_HandleCount(a2),d7
	move.l	a2,d3
	lea	AnimHandle_SIZEOF(a2),a2
	bra.s	remove_loop.
remove_loop:
	sub.l	a3,a3
	move.l	(a2)+,a5		; next FXHandle
	move.l	fxh_UCopList(a5),d0	; look for this UCopList
	move.l	vp_UCopIns(a0),a1
1$:
	cmp.l	a1,d0	
	beq.s	remove_ucl
	move.l	a1,a3			; a3 will be the previous copperlist
	move.l	ucl_Next(a1),a1
	bra.s	1$
remove_ucl:
	; a1 -> this UCL
	; a3 -> previous UCL
	move.l	a3,d0
	beq.s	no_back_link
	move.l	ucl_Next(a1),ucl_Next(a3)
	bra.s	removed
no_back_link:
	move.l	ucl_Next(a1),vp_UCopIns(a0)
removed:
	; free the CopLists
	move.l	a0,-(sp)
	move.l	a1,a3
	move.l	ucl_FirstCopList(a3),a0
	exg	a4,a6
	jsr	_LVOFreeCopList(a6)
	; and free the UCopList
	exg	a4,a6
	move.l	a3,a1
	moveq	#ucl_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	move.l	(sp)+,a0

	; Check for external calls
	movem.l	a0/a6,-(sp)
	move.l	d4,a6
	move.l	fxh_Type(a5),d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	sfxb_RemoveVectors(a6),a1
	move.l	-4(a1,d0),d0
	beq.s	1$
	move.l	d0,a1
	jsr	(a1)
1$:
	movem.l	(sp)+,a0/a6

remove_loop.:
	dbra.s	d7,remove_loop

	move.l	gb_ActiViewCprSemaphore(a4),a0
	jsr	_LVOReleaseSemaphore(a6)

	; remove the handle from the AHList.
	; d3 -> AnimHandle
	; d4 -> SfxBase
	; a4 -> GfxBase
	; a6 -> ExecBase
	; First, get the Semaphore
	move.l	d4,a5
	move.l	sfxb_AHListSemaphore(a5),a0
	jsr	_LVOObtainSemaphore(a6)

	; look through the list for this AnimHandle, and remove it.
	lea	sfxb_AHList(a5),a0
search_ahlist_r:
	move.w	ahl_Count(a0),d0
	subq.w	#1,d0
	lea	ahl_AnimHandles(a0),a1
11$:
	cmp.l	(a1)+,d3	; is this the AnimHandle?
	beq.s	10$
	dbra.s	d0,11$
	; wasn't in that list, so check the next list.
	move.l	MLN_SUCC(a0),a0	; no need to error check, as we
				; know it's in here somewhere
	bra.s	search_ahlist_r

10$:
	clr.l	-4(a1)
	sub.w	#1,ahl_Count(a0)

	move.l	sfxb_AHListSemaphore(a5),a0
	jsr	_LVOReleaseSemaphore(a6)

	; Free the copper cache
	; We need to ensure that we are not Freeing the copperlists as they
	; are being viewed!
	DISABLE
	move.l	d3,a0			; Get the View
	move.l	ah_View(a0),a0
	move.l	v_LOFCprList(a0),d0
	beq.s	1$
	move.l	d0,a1
	move.l	crl_start(a1),gb_LOFlist(a4)
1$:
	btst.b	#2,gb_Modes+1(a4)	; are we laced?
	beq.s	2$
	move.l	v_SHFCprList(a0),d0
	beq.s	2$
	move.l	d0,a1
	move.l	crl_start(a1),gb_SHFlist(a4)
2$:
	ENABLE
	exg	a4,a6			; exchg GfxBase<->ExecBase
	jsr	_LVOWaitTOF(a6)		; make sure the original copperlists
					; are being used.
	jsr	_LVOWaitTOF(a6)
	exg	a4,a6

	move.l	d3,a4
	move.l	ah_Copper2L(a4),d0
	beq.s	no_cop2L_free
	move.l	d0,a1
	move.l	ah_Copper2LSize(a4),d0
	beq.s	no_cop2L_free
	jsr	_LVOFreeMem(a6)
no_cop2L_free:
	move.l	ah_Copper2S(a4),d0
	beq.s	no_cop2S_free
	move.l	d0,a1
	move.l	ah_Copper2SSize(a4),d0
	beq.s	no_cop2S_free
	jsr	_LVOFreeMem(a6)
no_cop2S_free:

	; and free the handle
	move.l	d3,a1
	move.l	ah_HandleSize(a1),d0
	jsr	_LVOFreeMem(a6)

	; Unlock the AnimationSemaphore.
	move.l	sfxb_AnimationSemaphore(a5),a0
	jsr	_LVOReleaseSemaphore(a6)

RemoveFX.
	movem.l	(sp)+,d2-d4/d7/a2-a6
	rts
