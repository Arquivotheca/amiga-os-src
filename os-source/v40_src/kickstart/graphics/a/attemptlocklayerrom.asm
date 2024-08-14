*******************************************************************************
*
*	$Id: AttemptLockLayerRom.asm,v 39.1 92/09/03 15:34:38 spence Exp $
*
*******************************************************************************

	section	graphics
    xdef    _AttemptLockLayerRom
******* graphics.library/AttemptLockLayerRom *******************************
*                           *
*   NAME
*	AttemptLockLayerRom -- Attempt to Lock Layer structure
*					 by ROM(gfx lib) code
*
*   SYNOPSIS
*	gotit = AttemptLockLayerRom( layer )
*	 d0			      a5
*
*	BOOL AttempLockLayerRom( struct Layer * );
*
*   FUNCTION
*	Query the current state of the lock on this Layer. If it is
*	already locked then return FALSE, could not lock. If the
*	Layer was not locked then lock it and return TRUE.
*	This call does not destroy any registers.
*	This call nests so that callers in this chain will not lock
*	themselves out.
*
*   INPUTS
*	layer - pointer to Layer structure
*
*   RESULT
*	gotit - TRUE or FALSE depending on whether the Layer was
*		successfully locked by the caller.
*
*   SEE ALSO
*	LockLayerRom() UnlockLayerRom()
*
*********************************************************************
	include 'exec/types.i'
	include 'graphics/clip.i'

EXECSEMAPHORES	equ 1

_AttemptLockLayerRom:
	move.l	a0,-(sp)
	lea		lr_Lock(a5),a0

	ifd	EXECSEMAPHORES
		xref	_LVOAttemptSemaphore
		move.l	a6,-(sp)
		move.l	4,a6
		jsr		_LVOAttemptSemaphore(a6)
		move.l	(sp)+,a6
	endc

	move.l	(sp)+,a0
	rts


	end
