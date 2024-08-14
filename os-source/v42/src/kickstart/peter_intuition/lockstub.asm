* $Id: lockstub.asm,v 38.0 91/06/12 14:26:27 peter Exp $

*****************************************************************
* This stuff is so that I can turn on lock diagnostics without	*
* recompiling the world.  Note that this should always		*
* replace these calls to graphics/layers locking functions,	*
* so we don't bring them in from amiga.lib (rom.lib)		*
*****************************************************************

LOCKDEBUG	EQU	0

	XREF	_GfxBaseOffset
	XREF	_LayersBaseOffset

	XREF	_fakeLockLayerRom
	XREF	_fakeLockLayerInfo
	XREF	_fakeLockLayers
	XREF	_fakeUnlockLayerRom
	XREF	_fakeUnlockLayerInfo
	XREF	_fakeUnlockLayers

	XDEF	_LockLayerRom
	XDEF	_LockLayerInfo
	XDEF	_LockLayers
	XDEF	_UnlockLayerRom
	XDEF	_UnlockLayerInfo
	XDEF	_UnlockLayers

	IFNE	LOCKDEBUG

	; make these conditional, so I'll know if I forgot to 
	; assemble these in when I try to link with ilock.c
	XDEF	_realLockLayerRom
	XDEF	_realLockLayerInfo
	XDEF	_realLockLayers
	XDEF	_realUnlockLayerRom
	XDEF	_realUnlockLayerInfo
	XDEF	_realUnlockLayers

	ENDC

_LockLayerRom:
	IFNE	LOCKDEBUG
	jmp	_fakeLockLayerRom
	ENDC
_realLockLayerRom;
	movem.l	a5/a6,-(a7)
	move.l	_GfxBaseOffset(a6),a6
	move.l	12(a7),a5
	jsr	-432(a6)
	movem.l	(a7)+,a5/a6
	rts

_LockLayerInfo:
	IFNE	LOCKDEBUG
	jmp	_fakeLockLayerInfo
	ENDC
_realLockLayerInfo:
	move.l	a6,-(a7)
	move.l	_LayersBaseOffset(a6),a6
	move.l	8(a7),a0
	jsr	-120(a6)
	move.l	(a7)+,a6
	rts

_LockLayers:
	IFNE	LOCKDEBUG
	jmp	_fakeLockLayers
	ENDC
_realLockLayers:
	move.l	a6,-(a7)
	move.l	_LayersBaseOffset(a6),a6
	move.l	8(a7),a0
	jsr	-108(a6)
	move.l	(a7)+,a6
	rts


_UnlockLayerRom:
	IFNE	LOCKDEBUG
	jmp	_fakeUnlockLayerRom
	ENDC
_realUnlockLayerRom:
	movem.l	a5/a6,-(a7)
	move.l	_GfxBaseOffset(a6),a6
	move.l	12(a7),a5
	jsr	-438(a6)
	movem.l	(a7)+,a5/a6
	rts

_UnlockLayerInfo:
	IFNE	LOCKDEBUG
	jmp	_fakeUnlockLayerInfo
	ENDC
_realUnlockLayerInfo:
	move.l	a6,-(a7)
	move.l	_LayersBaseOffset(a6),a6
	move.l	8(a7),a0
	jsr	-138(a6)
	move.l	(a7)+,a6
	rts

_UnlockLayers:
	IFNE	LOCKDEBUG
	jmp	_fakeUnlockLayers
	ENDC
_realUnlockLayers:
	move.l	a6,-(a7)
	move.l	_LayersBaseOffset(a6),a6
	move.l	8(a7),a0
	jsr	-114(a6)
	move.l	(a7)+,a6
	rts

	END

