

	section	strings.lib

;
; returns:
;	-1 if a < b
;	 0 if a = b
;	 1 if a > b
; all comparisons use unsigned arithmetic
;
	xdef	_strcmp
	xdef	strcmp

_strcmp:
	movem.l	4(sp),a0/a1	; string_a, string_b

strcmp:
	moveq.l	#0,d0		; set up the return value

1$:
	move.b	(a0)+,d1
	cmp.b	(a1)+,d1
	bne.s	2$

	tst.b	d1
	bne.s	1$		; go back and get the next character

	bra.s	strcmp_end	; strings are equal
	
2$:
	bhi.s	3$
	moveq.l	#-1,d0		; (a0) was < (a1)
	bra.s	strcmp_end

3$:
	moveq.l	#1,d0

strcmp_end:
	rts
	end
