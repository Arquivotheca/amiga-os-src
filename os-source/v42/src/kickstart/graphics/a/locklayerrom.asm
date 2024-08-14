*******************************************************************************
*
*	$Id: LockLayerRom.asm,v 42.0 93/06/16 11:13:13 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _LockLayerRom
******* graphics.library/LockLayerRom ***************************************
*
*   NAME
*	LockLayerRom -- Lock Layer structure by ROM(gfx lib) code.
*
*   SYNOPSIS
*	LockLayerRom( layer )
*		       a5
*
*	void LockLayerRom( struct Layer * );
*
*   FUNCTION
*	Return when the layer is locked and no other task may
*	alter the ClipRect structure in the Layer structure.
*	This call does not destroy any registers.
*	This call nests so that callers in this chain will not lock
*	themselves out.
*	Do not have the Layer locked during a call to intuition.
*	There is a potential deadlock problem here, if intuition
*	needs to get other locks as well.
*	Having the layer locked prevents other tasks from using the
*	layer library functions, most notably intuition itself. So
*	be brief.
*	layers.library's LockLayer is identical to LockLayerRom.
*
*   INPUTS
*	layer - pointer to Layer structure
*
*   RESULTS
*	The layer is locked and the task can render assuming the
*	ClipRects will not change out from underneath it until
*	an UnlockLayerRom is called.
*
*   SEE ALSO
*	UnlockLayerRom() layers.library/LockLayer() graphics/clip.h
*
*****************************************************************************
	include 'exec/types.i'
	include 'graphics/clip.i'

EXECSEMAPHORES	equ 1

_LockLayerRom:
	move.l	a0,-(sp)
	lea		lr_Lock(a5),a0

	ifd	EXECSEMAPHORES
		xref	_LVOObtainSemaphore
		move.l	a6,-(sp)
		move.l	4,a6
		jsr		_LVOObtainSemaphore(a6)
		move.l	(sp)+,a6
	endc

	ifnd	EXECSEMAPHORES
		xref	_LVOObtainGfxSemaphore
		jsr	_LVOObtainGfxSemaphore(a6)
	endc

	move.l	(sp)+,a0
	rts


	end
