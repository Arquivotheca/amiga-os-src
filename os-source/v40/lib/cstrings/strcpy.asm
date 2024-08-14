

	section	strings.lib

	xdef	_strcpy
	xdef	strcpy

_strcpy:
	movem.l	4(sp),a0/a1	; to, from

strcpy:
	move.b	(a1)+,(a0)+
	bne.s	strcpy

	rts
	end
