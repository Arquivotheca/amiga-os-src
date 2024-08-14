**  munge.asm - memory munging module for munglist

	INCLUDE "exec/types.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/execbase.i"

	section code

	XDEF	_InitMunging

_InitMunging:
		move.l #$ABADCAFE,a0
		bra.s Mung

_PreMunging:
		move.l #$DEADF00D,a0
		bra.s Mung

_PostMunging:
		move.l #$DEADBEEF,a0
Mung:
		;   A0=MungValue
		;   A1=Base Address
		;   D0=raw size
		;
		move.l	a1,d1
		beq.s	BadCall
		tst.l	d0
		beq.s	BadCall		; zero size
		move.l	a0,d1		; mung value to d1
		addq.l	#7,d0
		lsr.l	#3,d0
		beq.s	BadCall
		cmpi.l	#16,d0		; go mingo for <128 byte mung
		bmi.s	mingo	

mongo:
		move.l	d0,a0		; save length before additional shifts
		lsr.l	#1,d0		; we'll do 16 bytes at a time
		bra.s	inloop2
loop2:		move.l	d1,(a1)+
		move.l	d1,(a1)+
		move.l	d1,(a1)+
		move.l	d1,(a1)+
inloop2:	dbra	d0,loop2
		sub.l	#$10000,d0
		bpl.s	loop2

		move.l	a0,d0		; get back size before shift
		andi.l	#$1,d0
		beq.s	MungExit	; drop thru munges leftover bytes

mingo:
		bra.s	inloop
loop:		move.l	d1,(a1)+
		move.l	d1,(a1)+
inloop:		dbra	d0,loop

*		sub.l	#$10000,d0	; no longer needed because
*		bpl.s	loop		; we only come here for <128 bytes

MungExit:
BadCall:
		rts

        END
