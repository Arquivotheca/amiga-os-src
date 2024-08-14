********************************************************************************
*
*	$Id: allocfx.asm,v 39.8 93/08/11 17:19:41 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/memory.i'
	include	'graphics/gfxbase.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX.i"
	include	"SpecialFX_internal.i"

	xdef	_LVOAllocFX
	xref	_LVOAllocMem

******* specialfx.library/AllocFX **********************************************
*
*   NAME
*	AllocFX -- Allocate memory and handle for a Special Effect.
*
*   SYNOPSIS
*	FXHandle = AllocFX(ViewPort, Type, Number, Error)
*	d0                 a0        d0    d1      a1
*
*	APTR AllocFX(struct ViewPort *, ULONG, UWORD, ULONG *);
*
*   FUNCTION
*	To allocate the data needed to control a 'special effect'. This builds
*	a handle, and a list of pointers to the various structures needed.
*
*   INPUTS
*	ViewPort - ViewPort the effect will be used on.
*	Type   - The type of effect desired, as follows:
*	SFX_ColorControl - Builds a copperlist to alter the colour of
*	                  specified pens at specified lines, in the maximum
*	                  possible colourspace of the chip set.
*
*	SFX_LineControl - Builds a copperlist to scroll a group of lines
*	                  horizontally, independant from the rest of the
*	                  ViewPort.
*
*	SFX_IntControl  - Builds a copperlist to cause a copper interrupt at the
*	                  specified line. Note that the interrupt is not enabled
*	                  until you have installed an interrupt server with
*	                  exec.library/AddIntServer().
*
*	SFX_FineVideoControl
*	                - Builds a copperlist to create a subset of the effects
*	                  that can be generated with graphics.library's
*	                  VideoControl() function. See <libraries/specialfx.h>
*	                  for a list of the effects.
*
*	SFX_SpriteControl
*	                - Builds a copperlist to allow the reuse of sprites in
*	                  a ViewPort. This is similar to VSprites, but this can
*	                  also handle AA wide sprites which VSprites do not
*	                  support.
*
*	Number - The number of elements in the effect.
*	Error - Pointer to a ULONG for secondary error information
*
*   RESULT
*	FXHandle - will point to an array of 'Number' pointers to structures
*	           associated with the effect type.
*	           If NULL, then Error will contain an error code.
*
*   SEE ALSO
*	<libraries/specialfx.h> InstallFXA() FreeFX()
*
********************************************************************************

_LVOAllocFX:
	movem.l	d2-d6/a2/a3/a5/a6,-(sp)
	move.l	a6,a5
	move.l	a1,a2
	move.l	a0,a3		; safe keeping
	move.l	sfxb_AllocVectors(a6),a0
	; Find the type
	and.l	#~$80000000,d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	-4(a0,d0.w),a0
	jsr	(a0)
AllocFX.:
	movem.l	(sp)+,d2-d6/a2/a3/a5/a6
	rts
