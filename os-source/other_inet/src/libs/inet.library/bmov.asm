
;BMOV.ASM
;      A0  A1   D0 
;BMOV(src,dest,bytes)
;BCOPY(src,dest,bytes)
;
;   Matthew Dillon
;
;   -Handles ascending/descending moves
;   -Optimizes the move if addresses longword boundries and #bytes
;    is a multiple of 4.
;   -Completely tested

	xdef  _bcopy

_bcopy
	move.l	d2,-(sp)	; save d2 so we can use it
	moveq	#0,D1		;long word boundry flag
	move.l	d0,d2
	andi.l	#3,d2		;byte count multiple of 4?
	bne.b	notlwb
	move.l	a0,d2
	andi.l	#3,d2		;source mulitple of 4?
	bne.b	notlwb
	move.l	a1,d2
	andi.l	#3,d2		;destination multiple of 4?
	seq.b	D1		;set longword boundry flag if true
notlwb	cmp.l	A0,A1
	beq.b	end		;trivial case
	ble.b	ascend		;ascending copy
	add.l	D0,A0		;descending copy
	add.l	D0,A1

decend	tst.b	D1		;DESCEND
	beq.b	decbyte
declong lsr.l	#2,D0		;DESCEND LONGWORD
	bra.b	ab00
ab0	move.l	-(A0),-(A1)
ab00	dbf.w	D0,ab0
	sub.l	#$10000,D0
	bpl.b	ab0
end	move.l	(sp)+,d2
	rts

ab1	move.b	-(A0),-(A1)	;DESCEND BYTE
decbyte dbf.w	D0,ab1
	sub.l	#$10000,D0
	bpl.b	ab1
	bra.b	end

ascend	tst.b	D1		;ASCEND
	beq.b	ascbyte
asclong lsr.l	#2,D0		;ASCEND LONGWORD
	bra.b	ab22
ab2	move.l	(A0)+,(A1)+
ab22	dbf.w	D0,ab2
	sub.l	#$10000,D0
	bpl.b	ab2
	bra.b	end

ab3	move.b	(A0)+,(A1)+	;ASCEND BYTE
ascbyte dbf.w	D0,ab3
	sub.l	#$10000,D0
	bpl.b	ab3
	bra.b	end


