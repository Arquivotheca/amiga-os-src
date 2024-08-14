


	section	strings.lib

	xdef	_strncat
	xdef	strncat

	xref	strncpy

_strncat:
	movem.l	4(sp),a0/a1	; to, from
	move.l	12(sp),d0	; count

strncat:

	tst.b	(a0)+
	beq.s	strncat_cpy

	subq.l	#1,d0
	bgt.s	strncat
	bra.s	strncat_end

strncat_cpy:
	subq.l	#1,a0
	bsr	strncpy

strncat_end:
	rts

	end
