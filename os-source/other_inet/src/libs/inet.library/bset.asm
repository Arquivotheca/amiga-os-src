;BZERO.ASM
;
;   Uses longword operations if data is aligned on a longword boundry
;   and the size is a mulitple of 4.  Otherwise, uses byte operations.
;A0 = address    
;D0 = byte count 

	xdef  _bzero

_bzero  move.l	d0,d1
	andi.b	#3,d1		;byte count on long word boundry?
	bne.b	loadd
	move.l	a0,d1
	andi.b	#3,d1		;address on longword boundry?
	bne.b	loadd
	bra.b	lwb
loadd	moveq	#0,D1
	bra.b	drop
loop	move.b	D1,(A0)+	;BYTE SET LOOP
drop	dbf.w	D0,loop 	;remember, only effects lower word
	sub.l	#$10000,D0	;for buffers >65535
	bpl.b	loop		;branch to loop because D0.W now is FFFF
	rts

lwb	lsr.l	#2,D0		;byte count / 4 (longword chunks)
	moveq	#0,D1
	bra.b	dropl

loopl	move.l	D1,(A0)+	;LONGWORD LOOP
dropl	dbf.w	D0,loopl	;remember, only effects lower word
	sub.l	#$10000,D0	;for buffers >65535
	bpl.b	loopl		;branch to loop because D0.W now is FFFF
	rts



