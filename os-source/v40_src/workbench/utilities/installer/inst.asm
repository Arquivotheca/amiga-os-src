*==========================================================================*
*  Installer Assemebly Code                                                *
*  By Talin. (c) 1990 Sylvan Technical Arts.                               *
*==========================================================================*

* case insensitive memory compare
* REM: Replace with improved, shorter memory compare on my system... --DJ

		section	TEXT,CODE

		include 'macros.i'

		xdef	_CmpMemI

; IMATCH - a macro that quickly does a case-insensitive character test
;	Note this function destroys the two registers passed to it.
;	Usage:
;			IMATCH		reg1,reg1,matchlabel,nomatchmabel

IMATCH		macro

			cmp.b		\1,\2					; if characters match
			beq.s		\3
			eor.b		\1,\2					; get exclusive or of characters
			cmp.b		#32,\2					; if different only by bit 32
			bne.s		\4						; then
			bset		#5,\1					; force to upper case
			cmp.b		#$61,\1					; if less than lower case 'a'
			blo.s		\4						; then no match
			cmp.b		#$7a,\1					; if less that or equal to lower 'z'
			bls.s		\3						; then match
			cmp.b		#$E0,\1					; if less than lower case '�'
			blo.s		\4						; then no match
			bra.s		\3						; then match

			endm

_CmpMemI
			move.l	4(sp),a0
			move.l	8(sp),a1
			move.l	12(sp),d1
			move.l	d2,-(sp)
			bra.s	3$

2$			move.b	(a1)+,d0		; are current bytes different?
			move.b	(a0)+,d2

			IMATCH	d0,d2,3$,1$

3$			dbra	d1,2$

			moveq	#1,d0
			move.l	(sp)+,d2
			rts

1$			moveq	#0,d0
			move.l	(sp)+,d2
			rts

			xdef		_or_mem
_or_mem:
			move.l		4(sp),a0
			move.l		8(sp),a1
			move.l		12(sp),d0

			bra.s		2$
1$			move.b		(a0)+,d1
			or.b		d1,(a1)+
2$			dbra		d0,1$

			rts

			xdef		_swap_mem
_swap_mem
			move.l		4(sp),a0
			move.l		8(sp),a1
			move.l		12(sp),d0
			bra.s		2$
1$			move.b		(a0),d1
			move.b		(a1),(a0)+
			move.b		d1,(a1)+
2$			dbra		d0,1$
			rts

;---------------------------------------------
;		a_to_b(str) - convert binary string to long

			xdef	_a_to_b

_a_to_b
			move.l	4(sp),a0	; get string address

			moveq	#0,d0		; d0 = current value

1$			moveq	#0,d1		; set up for a long value
			move.b	(a0)+,d1	; get a letter

			sub.b	#'0',d1		; is it < '0'?
			bmi.s	2$			; yes, end of number
			cmp.b	#1,d1		; is it > '1'?
			bhi.s	2$			; yes, end of number

			lsl.l	#1,d0		; d0 <- 2 * d0
			or.l	d1,d0		; or in either 0 or 1
			bra.s	1$			; loop

2$			rts					; result in d0

			end
