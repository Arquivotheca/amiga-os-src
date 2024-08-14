*******************************************************************************
*
*	$Id: abort.asm,v 39.1 92/06/05 11:46:58 mks Exp $
*
*******************************************************************************


	include "exec/types.i"
	include	"exec/lists.i"
	include "graphics/layers.i"
	include "layersbase.i"

	section layers
	xdef	_aborted

*
*   setjmp( jmpvec ) -- marks a place in the calling stack than may be
*	later longjmp'ed to.  Will return 0 if it is the initial setjmp,
*	and the longjmp code if it is longjmp'ed to.
*
*   longjmp( jmpvec, code ) -- jumps to the saved jmpvec, passing the
*	code along.
*
*   jmpvec should be an array of 13 longs.  The first element is the
*	saved return pc.  The others are the saved registers.
*
	xref	LockLayerInfo
	xref	UnlockLayerInfo
	xref	_common_cleanup
	xref	_NewList

_aborted:
	move.l	4(sp),a0
	bsr	LockLayerInfo

	move.l	4(sp),a0		* get layerinfo *
	move.l	li_LayerInfo_extra(a0),a0
	lea	lie_mem(a0),a1
	bsr	_NewList

	movem.l	d2-d7/a2-a7,4(a0)
	move.l	(a7),(a0)
	moveq	#0,d0
	rts

	xdef	abort
abort:
	move.l	a0,-(sp)	; Save it
	subq.l	#4,sp		; Give one more word...
	;drop into _abort
;
	xdef	_abort
_abort:
	moveq.l	#1,d0		* TRUE
	move.l	4(sp),a0	* get LayerInfo *
	bsr	_common_cleanup
	move.l	4(sp),a0	* get layerinfo *
	move.l	a0,a1
	move.l	li_LayerInfo_extra(a0),a0
	movem.l	4(a0),d2-d7/a2-a7	* restore registers
	move.l	(a0),(a7)	* restore pc
	move.l	a1,a0
	bsr	UnlockLayerInfo
	moveq	#1,d0		* return to abort
	rts


	END
