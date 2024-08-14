
	section	strings.lib


	xdef	_index
	xdef	index
	xdef	_rindex
	xdef	rindex

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


; condition codes must be set to the same as d0
;

_rindex:
	move.l	4(sp),a0
	move.l	8(sp),d0

rindex:
	move.l	a0,a1
	move.l	d0,d1

	bsr	strlen	(a1,d1 unchanged)
	beq.s	rindex_fail

	add.l	d0,a1

	bra.s	rindex_entry

rindex_loop:
	cmp.b	-(a1),d1
rindex_entry:
    	dbeq	d0,rindex_loop

	bne.s	rindex_fail

	move.l	a1,d0

rindex_end:
	rts

rindex_fail:
	moveq.l	#0,d0
	bra.s	rindex_end


; only works on strings up to 64k -1
; only touches a0,d0


strlen:
	moveq.l	#-1,d0

1$:
	tst.b	(a0)+
	dbeq	d0,1$

	not.l	d0
	rts

	end
