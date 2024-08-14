* HookEntry for Lattice

	INCLUDE	"exec/types.i"
	INCLUDE	"utility/hooks.i"

	XDEF	_HookEntry

	section code

 _HookEntry:
	move.l	a6,-(sp)
 	move.l	a1,-(sp)		; push message packet pointer
 	move.l	a2,-(sp)		; push object pointer
 	move.l	a0,-(sp)		; push hook pointer
	move.l	h_SubEntry(a0),a0	; fetch C entry point ...
	jsr	(a0)			; ... and call it
	lea	12(sp),sp		; fix stack
	move.l	(sp)+,a6
	rts

	end
