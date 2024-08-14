
	section	strings.lib


	xdef	_index
	xdef	index

_index:
	move.l	4(sp),a0
	move.l	8(sp),d0

index:
	move.b	(a0)+,d1
	beq.s	index_fail

	cmp.b	d0,d1
	bne.s	index

	subq.l	#1,a0
	move.l	a0,d0

index_end:
	rts

index_fail:
	moveq	#0,d0
	bra.s	index_end

	end
