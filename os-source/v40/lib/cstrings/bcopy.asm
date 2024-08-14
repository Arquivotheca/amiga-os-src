

	section	strings.lib

	xdef	_bcopy
	xdef	bcopy

_bcopy:
	movem.l	4(sp),a0/a1	; source, dest
	move.l	12(sp),d0	; len

bcopy:
	move.w	d0,d1		; least significant word
	swap	d0		; most significant word
	bra.s	2$

1$:
	move.b	(a0)+,(a1)+
2$:
	dbra	d1,1$
	dbra	d0,1$

	rts
	end
