	XDEF	_ULMult

_ULMult:
		movem.l 4(a7),d0/d1

		movem.l d2/d3,-(a7)

		move.l	d0,d2
		move.l	d1,d3
			   
		mulu	d1,d0		; l1,l0
		swap	d2
		mulu	d2,d1		; l1,h0
		swap	d1
		clr.w	d1		; loose MSB's here
		add.l	d1,d0
		swap	d3
		swap	d2
		mulu	d3,d2		; h1,l0
		swap	d2
		clr.w	d2		; loose MSB's here
		add.l	d2,d0
		;------ h1,h0 gets all lost anyway, ignore it  

		movem.l (a7)+,d2/d3
		rts

	END