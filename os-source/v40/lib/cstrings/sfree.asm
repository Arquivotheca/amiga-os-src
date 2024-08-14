
	section	strings.lib

; only works on strings up to 64k -1
; only touches a0,d0

	xdef	_sfree
	xdef	sfree

	xref	strlen
	xref	strcpy

	xref	_AbsExecBase
	xref	_LVOFreeMem

_sfree:
	move.l	4(sp),a0	; string

sfree:
	movem.l	d2/a6,-(sp)

	; save the string pointer
	move.l	a0,d2
	beq.s	sfree_end

	bsr	strlen

	; allow extra room for the null
	addq.l	#1,d0
	move.l	d2,a1

	move.l	_AbsExecBase,A6
	jsr	_LVOFreeMem(A6)

sfree_end:
	movem.l	(sp)+,d2/a6
	rts

	end
