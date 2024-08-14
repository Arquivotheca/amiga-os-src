

	section	strings.lib

;
; condition codes must be set to the same as d0
;
	xdef	_rindex
	xdef	rindex

	xref	strlen
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

	end
