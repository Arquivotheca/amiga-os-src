	OPTIMON

;---------------------------------------------------------------------------

	XDEF _CxDispatch

;---------------------------------------------------------------------------

_CxDispatch:
	move.l	a4,-(sp)	; save this guy
	move.l	d0,a4		; function pointer in a4

	movem.l	a0/a1,-(sp)     ; parameters on stack as well as in regs
	jsr	(a4)		; jump to function
	addq.l	#8,sp           ; remove args from stack

	move.l	(sp)+,a4        ; restore this one
	rts

;---------------------------------------------------------------------------

	END
