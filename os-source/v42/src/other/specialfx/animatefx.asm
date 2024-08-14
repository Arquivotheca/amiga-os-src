********************************************************************************
*
*	$Id: animatefx.asm,v 39.7 93/09/13 19:10:27 spence Exp $
*
********************************************************************************

	opt	p=68020
	section	code

	include	'exec/types.i'
	include	'exec/ables.i'
	include	'graphics/view.i'
	include	'graphics/copper.i'
	include	'graphics/gfxbase.i'
	include	'utility/tagitem.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX.i"
	include	"SpecialFX_internal.i"

	xdef	_LVOAnimateFX
	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_intena

******* specialfx.library/AnimateFX ********************************************
*
*   NAME
*	AnimateFX -- Animate an list of 'Special Effects'
*
*   SYNOPSIS
*	AnimateFX(DisplayHandle)
*	          a0
*
*	void AnimateFX(APTR);
*
*   FUNCTION
*	To animate the list of 'Special Effects' pointed to by the Handle
*	obtained from InstallFXA().
*
*   INPUTS
*	DisplayHandle - The handle obtained from InstallFXA().
*
*   RESULT
*
*   NOTES
*	This function will update the display according to the input. There is
*	no need to remake the display yourself afterwards.
*
*	The method of animation varies depending on the 'Special Effect' type.
*	Each is described in <libraries/specialfx.h>.
*
*	This function will also update the intermediate copper lists in the
*	ViewPort, so that when a screen is dragged by the user, the latest
*	changes are preserved. However, this consumes processor time, as the
*	list needs to be updated for every call to AnimateFX(). Under
*	graphics.library V40, you can use the VC_IntermediateCLUpdate
*	VideoControl() tag to disable intermediate copperlist updates.
*
*   SEE ALSO
*	<libraries/specialfx.h> RemoveFX() InstallFXA()
*	graphics.library/VideoControl() <graphics/videocontrol.h>
*
********************************************************************************

_LVOAnimateFX:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	a0,a5
	; locking:
	move.l	a6,a4
	move.l	sfxb_GfxBase(a6),a0
	move.l	gb_ActiViewCprSemaphore(a0),a0
	move.l	a0,-(sp)
	move.l	sfxb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)

	move.l	sfxb_AHListSemaphore(a4),a0
	move.l	a0,-(sp)
	jsr	_LVOObtainSemaphore(a6)
	move.l	a4,a6

	; walk through the list and FXHandles
	lea	AnimHandle_SIZEOF(a5),a2
	move.l	ah_HandleCount(a5),d7
	bra.s	anim_loop.
anim_loop:
	move.l	(a2)+,a3		; get the next FXHandle
	btst.b	#AHB_CALCOFFSET,ah_Flags+1(a5)
	bne.s	2$
	tst.l	fxh_Offset(a3)		; has the offset been calculated yet?
	bne.s	no_offset_calc
	; calculate the offset value
2$:
	move.l	fxh_UCopList(a3),a1
	move.l	ucl_FirstCopList(a1),a1
	move.l	cl_CopLStart(a1),d0	; first copper instruction of this effect
	move.l	ah_View(a5),a1
	move.l	v_LOFCprList(a1),a1
	sub.l	crl_start(a1),d0	; offset value
	move.l	d0,fxh_Offset(a3)
no_offset_calc:
	move.l	fxh_Type(a3),d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	sfxb_AnimateVectors(a6),a1
	move.l	-4(a1,d0),a1
	jsr	(a1)
anim_loop.:
	dbra.s	d7,anim_loop

	; if the View of this AnimHandle is the same as GfxBase->ActiView,
	; then display this copperlist.
	move.l	sfxb_GfxBase(a6),a0
	move.l	sfxb_ExecBase(a6),a6
	move.l	gb_ActiView(a0),d0	
	cmp.l	ah_View(a5),d0
	bne.s	no_toggle_copper
	move.w	ah_CopperToUse(a5),d0
	asl.w	#3,d0
	
	; ensure integrity of the two pointers in GfxBase by disabling
	; interrupts.
	DISABLE
	move.l	ah_Copper1L(a5,d0.w),gb_LOFlist(a0)
	move.l	ah_Copper1S(a5,d0.w),gb_SHFlist(a0)
	ENABLE

no_toggle_copper:
	; toggle the Copper list to use next time.
	eor.w	#1,ah_CopperToUse(a5)

	; reset the CALCOFFSET flag
	bclr.b	#AHB_CALCOFFSET,ah_Flags+1(a5)

	move.l	(sp)+,a0		; sfxb_AHListSemaphore
	jsr	_LVOReleaseSemaphore(a6)

	move.l	(sp)+,a0		; gb_ActiViewCprSemaphore
	jsr	_LVOReleaseSemaphore(a6)
	movem.l	(sp)+,d2-d7/a2-a6
	rts

