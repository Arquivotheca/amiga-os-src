
	section	strings.lib

; only works on strings up to 64k -1
; only touches a0,d0

	xdef	_scopy
	xdef	scopy

	xref	strlen
	xref	strcpy

	xref	_AbsExecBase
	xref	_LVOAllocMem

_scopy:
	move.l	4(sp),a0	; string

scopy:
	movem.l	a2/a6,-(sp)

	; save the string pointer
	move.l	a0,a2

	bsr	strlen

	; allow extra room for the null
	addq.l	#1,d0

	moveq.l	#0,d1
	move.l	_AbsExecBase,A6
	jsr	_LVOAllocMem(A6)

	tst.l	d0
	beq.s	scopy_end

	move.l	d0,a0
	move.l	a2,a1
	move.l	a0,a2
	bsr	strcpy

	move.l	a2,d0

scopy_end:

	movem.l	(sp)+,a2/a6
	rts

	end
