

	section	strings.lib

	xdef	_strncpy
	xdef	strncpy

_strncpy:
	movem.l	4(sp),a0/a1	; to, from
	move.l	12(sp),d0	; count

strncpy:

	subq.l	#1,d0
	blt.s	strncpy_end

	move.b	(a1)+,(a0)+
	bne.s	strncpy

1$:
	subq.l	#1,d0
	ble.s	strncpy_end

	clr.b	(a0)+
	bra.s	1$

strncpy_end:
	rts

	end
