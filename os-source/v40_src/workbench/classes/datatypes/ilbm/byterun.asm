; VOID ASM ExpandByteRun (REG (a0) BYTE *srcData, REG (a1) BYTE *dstData, REG (d0) WORD srcBytes);
; returns with a0/a1 updated!
; limited to 64k of source data

	OPTIMON

;---------------------------------------------------------------------------

	XDEF	_ExpandByteRun

;---------------------------------------------------------------------------

_ExpandByteRun:

	move.l	d2,-(a7)		; get some working space
	bra.s	endloop			; skip to end of loop to start things off

next_opcode:
	move.b	(a0)+,d1		; get the next opcode byte
	bmi.s	replicate		; if the byte is < 0,

	ext.w	d1			; turn it into a word
1$:	; copy n+1 litteral bytes
	move.b	(a0)+,(a1)+		; get a source byte, and put it in dest
	dbra	d1,1$			; play it again Scham...

endloop:
	dbra	d0,next_opcode		; decrement source counter and loop
	move.l	(a7)+,d2		; if we're done, cleanup
	rts				; bye

replicate:
	neg.b	d1			; n = -n
	bmi.s	endloop			; if -128, NOP opcode, so loop back

	ext.w	d1			; turn it into a word
	move.b	(a0)+,d2		; get byte to replicate
	subq.w	#1,d0			; decrement source counter
	beq.s	exit			; if no more source bytes, split

; replicate next -n+1 bytes
1$:	move.b	d2,(a1)+		; replicate byte
	dbra	d1,1$			; loopy the magic counter

	dbra	d0,next_opcode		; decrement source counter and loop

exit:
	move.l	(a7)+,d2		; if we're done, cleanup
	rts				; bye

;---------------------------------------------------------------------------

	END
