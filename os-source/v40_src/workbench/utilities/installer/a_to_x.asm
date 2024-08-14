*****************************************************
*		Parsec Soft Systems string functions		*
*		Copyright 1989 Parsec Soft Systems			*
*****************************************************

;---------------------------------------------
;		a_to_x(str) - convert hex string to long

		section code

		xdef	_a_to_x
_a_to_x:
		move.l	4(sp),a0	; get string address
		move.l	a0,a1		; save a copy

		cmp.b	#'-',(a0)	; is first letter a minus sign?
		beq.s	5$			; yes, so increment needed
		cmp.b	#'+',(a0)	; is first letter a plus sign?
		bne.s	1$			; no, so no increment 
5$		addq.w	#1,a0		; increment

1$		moveq	#0,d0		; d0 = current value

2$		moveq	#0,d1		; set up for a long value
		move.b	(a0)+,d1	; get a letter
		sub.b	#'0',d1		; is it < '0'?
		bmi.s	3$			; yes, end of number

		cmp.b	#9,d1		; is it > '9'?
		bls.s	6$			; no, so have a number 0 to 9

		sub.b	#'A'-'0',d1	; is it < 'A'
		bmi.s	3$			; yes, end of number

		cmp.b	#5,d1		; is it > 'F'
		bls.s	7$			; no, so digit between A and F

		sub.b	#'a'-'A',d1	; is it < 'a'
		bmi.s	3$			; yes, end of number

		cmp.b	#5,d1		; is it > 'F'
		bhi.s	3$			; yes, so end of number
							; else digit between a and f

7$		add.l	#10,d1
6$		lsl.l	#4,d0		; 16 * d0
		add.l	d1,d0		; 16 * d0 + d1
		bra.s	2$

3$		cmp.b	#'-',(a1)	; was it negative?
		bne.s	4$
		neg.l	d0			; negate the value
4$		rts

		end
