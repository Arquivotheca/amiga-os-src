*******************************************************************************
*
*	$Id: locklayer.asm,v 38.4 92/03/26 18:04:33 mks Exp $
*
*******************************************************************************
*
	include "exec/types.i"
	include	"graphics/clip.i"
	include	"graphics/layers.i"
	include "layersbase.i"
*
*******************************************************************************
*
LockLayer:	xdef	LockLayer
		move.l	a6,-(sp)		; Save LayersBase
		lea	lr_Lock(a1),a0		; Get semaphore
		move.l	lb_ExecBase(a6),a6	; Get execbase...
		CALLSYS	ObtainSemaphore		; Get the semaphore
		move.l	(sp)+,a6		; Restore LayersBase
		rts
*
*******************************************************************************
*
UnlockLayer:	xdef	UnlockLayer
		move.l	a6,-(sp)		; Save LayersBase
		lea	lr_Lock(a0),a0		; Get semaphore...
		move.l	lb_ExecBase(a6),a6	; Get ExecBase
		CALLSYS	ReleaseSemaphore	; Release the semaphore
		move.l	(sp)+,a6
		rts
*
*******************************************************************************
*
LockLayerInfo:	xdef	LockLayerInfo
_LockLayerInfo:	xdef	_LockLayerInfo
		move.l	a6,-(sp)
		lea	li_Lock(a0),a0
		move.l	lb_ExecBase(a6),a6
		CALLSYS	ObtainSemaphore
		move.l	(sp)+,a6
		rts
*
*******************************************************************************
*
UnlockLayerInfo:	xdef	UnlockLayerInfo
_UnlockLayerInfo:	xdef	_UnlockLayerInfo
			move.l	a6,-(sp)
			lea	li_Lock(a0),a0
			move.l	lb_ExecBase(a6),a6
			jsr	_LVOReleaseSemaphore(a6)
			move.l	(sp)+,a6
			rts
*
*******************************************************************************
*
		end
