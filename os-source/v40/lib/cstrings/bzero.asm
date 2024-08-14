

	section	strings.lib

	xdef	_bzero
	xdef	bzero

_bzero:
	move.l	4(sp),a0	; buf
	move.l	8(sp),d0	; len

bzero:
	moveq.l	#0,d1
	bra.s	2$

1$:
	move.b	d1,(a0)+
2$:
	dbra	d0,1$

	rts
	end
