
*  $Id: $

	INCLUDE 'exec/types.i'
	INCLUDE 'utility/hooks.i'

	xdef	_hookEntry
	xdef	_stubReturn


* entry interface for C code (large-code, stack parameters)
_hookEntry:
	move.l	a1,-(sp)
	move.l	a2,-(sp)
	move.l	a0,-(sp)
	move.l	h_SubEntry(a0),a0	; C entry point
	jsr	(a0)
	lea	12(sp),sp
_stubReturn:
	rts

