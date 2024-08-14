*******************************************************************************
*
*	$Id: UnlockLayerRom.asm,v 42.0 93/06/16 11:13:15 chrisg Exp $
*
*******************************************************************************

******* graphics.library/UnlockLayerRom ***************************************
*
*   NAME
*	UnlockLayerRom -- Unlock Layer structure by ROM(gfx lib) code.
*
*   SYNOPSIS
*	UnlockLayerRom( layer )
*			 a5
*
*	void UnlockLayerRom( struct Layer * );
*
*   FUNCTION
*	Release the lock on this layer. If the same task has called
*	LockLayerRom more than once than the same number of calls to
*	UnlockLayerRom must happen before the layer is actually freed
*	so that other tasks may use it.
*	This call does destroy scratch registers.
*	This call is identical to UnlockLayer (layers.library).
*
*   INPUTS
*	layer - pointer to Layer structure
*
*   BUGS
*
*   SEE ALSO
*	LockLayerRom() layers.library/UnlockLayer() graphics/clip.h
*
*********************************************************************

	include	'exec/types.i'
	include 'graphics/clip.i'

EXECSEMAPHORES	equ 1

	section	graphics
	xdef    _UnlockLayerRom
_UnlockLayerRom:
	move.l	a0,-(sp)
	lea		lr_Lock(a5),a0

	ifd	EXECSEMAPHORES
		xref	_LVOReleaseSemaphore
		move.l	a6,-(sp)
		move.l	4,a6
		jsr		_LVOReleaseSemaphore(a6)
		move.l	(sp)+,a6
	endc

	ifnd	EXECSEMAPHORES
		xref	_LVOReleaseGfxSemaphore
		jsr	_LVOReleaseGfxSemaphore(a6)
	endc

	move.l	(sp)+,a0
	rts

	end
