********************************************************************************
*
*	$Id: freefx.asm,v 39.2 93/07/19 11:46:47 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/memory.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX_internal.i"

	xdef	_LVOFreeFX
	xref	_LVOFreeMem

******* specialfx.library/FreeFX ***********************************************
*
*   NAME
*	FreeFX -- Free the memory associated with the Special Effect
*
*   SYNOPSIS
*	FreeFX(FXHandle)
*	       a0
*
*	void FreeFX(APTR);
*
*   FUNCTION
*	To free the data associated with a 'Special Effect'.
*
*   INPUTS
*	FXHandle - the handle obtained from AllocFX(). Passing NULL is legal.
*
*   RESULT
*
*   SEE ALSO
*	AllocFX()
*
********************************************************************************

_LVOFreeFX:
	move.l	a6,-(sp)
	move.l	a0,d0
	beq.s	FreeFX.
	move.l	ToFXHandle(a0),a1	; The FXHandle passed is a pointer to
					; the array list, so find the start of
					; the memory allocation.
	move.l	sfxb_ExecBase(a6),a6
	move.l	fxh_AllocSize(a1),d0
	jsr	_LVOFreeMem(a6)
FreeFX.:
	move.l	(sp)+,a6
	rts
