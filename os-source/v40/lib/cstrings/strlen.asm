

	section	strings.lib

; only works on strings up to 64k -1
; only touches a0,d0

	xdef	_strlen
	xdef	strlen

_strlen:
	move.l	4(sp),a0	; string

strlen:
	moveq.l	#-1,d0

1$:
	tst.b	(a0)+
	dbeq	d0,1$

	not.l	d0
	rts

	end
