
	section strings.lib
;
; returns pointer to trailing part of s if suf is the trailing part of s
;
	xdef	_suffix
	xdef	suffix

	xref	strlen
	xref	strcmp

_suffix:
	movem.l	4(sp),a0/a1	; s, suf

suffix:
	move.l	a2,-(sp)

	; find the length of s
	move.l	a0,a2
	bsr	strlen	; (leaves a1,d1 unchanged)

	; make a2 point at the end of the string
	add.l	d0,a2
	move.l	d0,d1

	; find the length of suf
	move.l	a1,a0
	bsr	strlen	; (leaves a1,d1 unchanged)

	cmp.l	d0,d1
	bcs.s	suffix_fail

	; make it point to the suffix part of s (if it has one...)
	sub.l	d0,a2

	move.l	a2,a0
	bsr	strcmp
	tst.l	d0
	bne.s	suffix_fail

	move.l	a2,d0			; return the suffix part of s

suffix_end:
	move.l	(sp)+,a2
	rts

suffix_fail:
	moveq.l	#0,d0
	bra.s	suffix_end

	end
