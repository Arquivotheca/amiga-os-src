
;BCMP.ASM
;
;   If both source and dest are on longword boundries, and if the byte
;   count is a multiple of 4, then use longword operations.
;
;   if byte count < 16 then just use byte operations
;
;   BCMP(p1,p2,n)   return 1=failed, 0=compare ok

     xdef  _bcmp

_bcmp:
	movem.l 4(A7),A0/A1	;A0 = ptr1, A1 = ptr2
	move.l	12(A7),D0	;# bytes
	andi.b	#3,15(A7)	;# bytes multiple of 4?
	bne	onbyte
	andi.b	#3,7(A7)	;ptr1 on lwb?
	bne	onbyte
	andi.b	#3,11(A7)	;ptr2 on lwb?
	bne	onbyte
	lsr.l	#2,D0		;YES, LONG COMPARE LOOP
	moveq.l	#0,D1		;sets Z flag
	bra	dropl
loopl	cmpm.l	(A0)+,(A1)+
dropl	dbne.w	D0,loopl
	bne	false
	sub.l	#$10000,D0
	bpl	loopl
	moveq.l	#0,D0
	rts			;return TRUE

onbyte	moveq.l	#0,D1		;sets Z flag
	bra	dropb
loopb	cmpm.b	(A0)+,(A1)+	;BYTE COMPARE
dropb	dbne.w	D0,loopb	;until count exhausted or compare failed
	bne	false
	sub.l	#$10000,D0	;for buffers >65535
	bpl	loopb		;branch to loop because D0.W now is FFFF
	moveq.l	#0,D0
	rts			;return TRUE

false	moveq.l	#1,D0
	rts			;return FALSE


