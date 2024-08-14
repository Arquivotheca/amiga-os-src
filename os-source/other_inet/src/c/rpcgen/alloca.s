;
; alloca for MANX.  This proc currently doesn't check stack limits, but
; it should.
;

	public	_alloca

_alloca:
	move.l	(sp)+,a0	; return address
	move.l	(sp)+,d0	; requested size
	addq.l	#3,d0		; round up size to mult of 4
	and.l	#~3,d0		; 
	sub.l	d0,sp		; crank sp back
	move.l	sp,d0		; for return
	subq.l	#4,sp		; to account for <size> we pulled off stack
	jmp	(a0)		; return to caller
