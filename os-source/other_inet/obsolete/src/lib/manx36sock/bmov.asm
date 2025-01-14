
;BMOV.ASM
;      4    8	12
;BMOV(src,dest,bytes)
;BCOPY(src,dest,bytes)
;
;   Matthew Dillon
;
;   -Handles ascending/descending moves
;   -Optimizes the move if addresses longword boundries and #bytes
;    is a multiple of 4.
;   -Completely tested

	xdef  _bmov
	xdef  _bcopy

_bcopy
_bmov
	movem.l 4(A7),A0/A1	;source, destination
	move.l	12(A7),D0	;bytes
	clr.b	D1		;long word boundry flag
	andi.b	#3,15(A7)	;byte count multiple of 4?
	bne	notlwb
	andi.b	#3,7(A7)	;source mulitple of 4?
	bne	notlwb
	andi.b	#3,11(A7)	;destination multiple of 4?
	seq.b	D1		;set longword boundry flag if true
notlwb	cmp.l	A0,A1
	beq	end		;trivial case
	ble	ascend		;ascending copy
	add.l	D0,A0		;descending copy
	add.l	D0,A1

decend	tst.b	D1		;DESCEND
	beq	decbyte
declong lsr.l	#2,D0		;DESCEND LONGWORD
	bra	ab00
ab0	move.l	-(A0),-(A1)
ab00	dbf.w	D0,ab0
	sub.l	#$10000,D0
	bpl	ab0
end	rts

ab1	move.b	-(A0),-(A1)	;DESCEND BYTE
decbyte dbf.w	D0,ab1
	sub.l	#$10000,D0
	bpl	ab1
	rts

ascend	tst.b	D1		;ASCEND
	beq	ascbyte
asclong lsr.l	#2,D0		;ASCEND LONGWORD
	bra	ab22
ab2	move.l	(A0)+,(A1)+
ab22	dbf.w	D0,ab2
	sub.l	#$10000,D0
	bpl	ab2
	rts

ab3	move.b	(A0)+,(A1)+	;ASCEND BYTE
ascbyte dbf.w	D0,ab3
	sub.l	#$10000,D0
	bpl	ab3
	rts


