; unxorcomp.asm

	XDEF _DecodeXORPlane

	section code

; LONG ASM DecodeXORPlane (REG (a0) char *in, REG(a2) char *out, REG (d2) LONG linebytes, REG (a3) WORD *ytable);

_DecodeXORPlane:
	movem.l	a2/a3/d2-d6,-(sp)	; save registers
	move.w	d2,d4			; make a copy of linebytes to use as a counter
	bra	zdcp
dcp
	move.l	a2,a1			; get copy of dest pointer
	clr.w	d0			; clear hi byte of op_count
	move.b	(a0)+,d0		; fetch number of ops in this column
	bra	zdcvclp

dcvclp	clr.w	d1			; clear hi byte of op
	move.b	(a0)+,d1		; fetch next op
	bmi.s	dcvskuniq		; if hi-bit set branch to "uniq" decoder
	beq.s dcvsame			; if it's zero branch to "same" decoder

skip					; otherwise it's just a skip
	add.w	d1,d1			; use amount to skip as index into word-table
	adda.w	0(a3,d1.w),a1
	dbra	d0,dcvclp		; go back to top of op loop
	addq.l	#1,a2			; so move the dest pointer to next column
	dbra	d4,dcp			; and go do it again what say?
	movem.l	(sp)+,a2/a3/d2-d6	; restore registers
	rts

dcvsame					; here we decode a "vertical same run"
	move.b	(a0)+,d1		; fetch the count
	move.b	(a0)+,d3		; fetch the value to repeat
	move.w	d1,d5			; and do what it takes to fall into a "tower"
	asr.w	#3,d5			; d5 holds # of times to loop through tower
	and.w	#7,d1			; d1 is the remainder
	add.w	d1,d1
	add.w	d1,d1
	neg.w	d1
	jmp	8*4+same_tower(pc,d1.w)	; 8 x 4 (size of tower)

same_tower
	eor.b	d3,(a1)
	adda.w	d2,a1
	eor.b	d3,(a1)
	adda.w	d2,a1
	eor.b	d3,(a1)
	adda.w	d2,a1
	eor.b	d3,(a1)
	adda.w	d2,a1
	eor.b	d3,(a1)
	adda.w	d2,a1
	eor.b	d3,(a1)
	adda.w	d2,a1
	eor.b	d3,(a1)
	adda.w	d2,a1
	eor.b	d3,(a1)
	adda.w	d2,a1
	dbra	d5,same_tower
	dbra	d0,dcvclp
	addq.l	#1,a2			; so move the dest pointer to next column
	dbra	d4,dcp			; and go do it again what say?
	movem.l	(sp)+,a2/a3/d2-d6	; restore registers
	rts

dcvskuniq				; here we decode a "unique" run
	and.b	#$7f,d1			; setting up a tower as above....
	move.w	d1,d5
	asr.w	#3,d5
	and.w	#7,d1
	add.w	d1,d1	;*2
	move.w	d1,d6
	add.w	d1,d1	;*4
	add.w	d6,d1	;*4+*2=*6
	neg.w	d1
	jmp	8*6+uniq_tower(pc,d1.w)	; 8 six bytes instructions

uniq_tower
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	move.b	(a0)+,d1
	eor.b	d1,(a1)
	adda.w	d2,a1
	dbra	d5,uniq_tower		; branch back up to "op" loop
zdcvclp	dbra	d0,dcvclp		; branch back up to "column loop"
					; now we've finished decoding a single column
z1dcp	addq.l	#1,a2			; so move the dest pointer to next column
zdcp	dbra	d4,dcp			; and go do it again what say?
	movem.l	(sp)+,a2/a3/d2-d6	; restore registers
	rts

	end
