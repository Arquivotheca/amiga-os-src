;----------------------------------------------------------------
;	zero(start,length) -- zero a range of memory quickly (max size 1 << 21)

			section	zero.asm,code

			xdef	_zero,zero
_zero
			move.l	4(sp),a0
			move.l	8(sp),d0
zero								; assembly entry (a0 = start, d0 = length)
			tst.l	d0				; allow size of zero and just return
			beq.s	6$

			move.l	a0,d1			; need address in data register
			btst.l	#0,d1			; is this an odd address?
			beq.s	8$				; yes, so no initial byte to clear
			clr.b	(a0)+
			subq.l	#1,d0			; one less byte
			bne.s	8$				; was it one byte? no, continue
6$			rts

8$			add.l	d0,a0			; add in size
			bclr	#0,d0			; clear a final odd byte?
			beq.s	9$
			clr.b	-(a0)

*	At this point, size is even and address is even
9$			bclr	#1,d0			; test and reset bit 1
			beq		1$
			clr.w	-(a0)			; if set then clear 1 word

1$			bclr	#2,d0			; test and reset bit 2
			beq		2$
			clr.l	-(a0)			; if set then clear 1 longword

2$			bclr	#3,d0			; test and reset bit 3
			beq		3$
			clr.l	-(a0)			; if set then move 2 longwords
			clr.l	-(a0)

3$			bclr	#4,d0			; test and reset bit 4
			beq		4$
			clr.l	-(a0)			; if set then move 4 longwords
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)

; at this point we should of copied just enough data to clear bits
;	0-4 of size. This means that any further copies can be modulo 32.

4$
			lsr.l	#5,d0
			bra.s	7$
5$
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
7$			dbra	d0,5$
			rts

			end
