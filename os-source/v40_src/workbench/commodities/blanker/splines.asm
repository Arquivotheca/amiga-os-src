	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"

        LIST

;---------------------------------------------------------------------------

	XDEF _DrawSpline

;---------------------------------------------------------------------------

	XREF _oldx
	XREF _oldy
	XREF _rastPort
	XREF _GfxBase
	XREF _LVODraw
	XREF _LVOMove

;---------------------------------------------------------------------------

_DrawSpline:
	movem.l	d2-d7/a2-a6,-(sp)

	movem.l a0/a1,-(sp)

        move.l  a6,a4
	move.l  _GfxBase,a6

	move.l	_oldx,d0
	move.l	_oldy,d1
	move.l  _rastPort,a1
	jsr     _LVOMove(a6)

	movem.l	(sp)+,a0/a1

	bsr.s	rspline
	movem.l	(sp)+,d2-d7/a2-a6
	rts

rspline:
	move.l	a0,d0
	sub.l	d6,d0
	move.l	d0,d3
	bpl.s	save1
	neg.l	d0
save1
	move.l	a1,d1
	sub.l	d7,d1
	move.l	d1,d4
	bpl.s	save2
	neg.l	d1
save2
	move.l	d0,d2
	cmp.l	d0,d1
	bmi.s	save3
	lsr.l	#3,d2
	bra.s	save9
save3
	lsr.l	#3,d1
save9
	add.l	d1,d2
	asr.l	#3,d2
	beq.s	check2
	asr.l	#5,d3
	asr.l	#5,d4
	move.l	a2,d0
	sub.l	a0,d0
	move.l	a3,d1
	sub.l	a1,d1
	asr.l	#5,d0
	asr.l	#5,d1
	muls.w	d4,d0
	muls.w	d3,d1
	sub.l	d1,d0
	bpl.s	save4
	neg.l	d0
save4
	cmp.l	d0,d2
	ble.s	pushem
	move.l	a5,d0
	sub.l	a0,d0
	move.l	a4,d1
	sub.l	a1,d1
	asr.l	#5,d0
	asr.l	#5,d1
	muls.w	d4,d0
	muls.w	d3,d1
	sub.l	d1,d0
	bpl.s	save5
	neg.l	d0
save5
	cmp.l	d0,d2
	ble.s	pushem
makeline
	lsr.l	#7,d7
	move.l	d7,d1
	lsr.l	#7,d6
	move.l	d6,d0
	move.l	a1,-(sp)
	move.l  a0,-(sp)
	move.l	d0,_oldx
	move.l	d1,_oldy

	move.l  _rastPort,a1
	jsr     _LVODraw(a6)
	move.l	(sp)+,a0
	move.l  (sp)+,a1
	rts

check2
	move.l	a0,d0
	sub.l	a2,d0
	bpl.s	ch1
	neg.l	d0
ch1
	move.l	a1,d1
	sub.l	a3,d1
	bpl.s	ch2
	neg.l	d1
ch2
	add.l	d0,d1
	asr.l	#3,d1
	bne.s	pushem
	move.l	a0,d0
	sub.l	a5,d0
	bpl.s	ch3
	neg.l	d0
ch3
	move.l	a1,d1
	sub.l	a4,d1
	bpl.s	ch4
	neg.l	d1
ch4
	add.l	d0,d1
	asr.l	#3,d1
	beq.s	makeline
pushem
	movem.l	d6/d7,-(sp)
	move.l	a5,d0
	add.l	d6,d0
	asr.l	#1,d0
	move.l	a4,d1
	add.l	d7,d1
	asr.l	#1,d1
	movem.l	d0/d1,-(sp)
	move.l	a2,d2
	add.l	a5,d2
	asr.l	#1,d2
	move.l	a3,d3
	add.l	a4,d3
	asr.l	#1,d3
	move.l	d0,d4
	add.l	d2,d4
	asr.l	#1,d4
	move.l	d1,d5
	add.l	d3,d5
	asr.l	#1,d5
	movem.l	d4/d5,-(sp)
	move.l	a0,d6
	add.l	a2,d6
	asr.l	#1,d6
	move.l	a1,d7
	add.l	a3,d7
	asr.l	#1,d7
	move.l	d2,d0
	add.l	d6,d0
	asr.l	#1,d0
	move.l	d3,d1
	add.l	d7,d1
	asr.l	#1,d1
	move.l	d6,a2
	move.l	d7,a3
	move.l	d0,d6
	add.l	d4,d6
	asr.l	#1,d6
	move.l	d1,d7
	add.l	d5,d7
	asr.l	#1,d7
	movem.l	d6/d7,-(sp)
	move.l	d0,a5
	move.l	d1,a4
	bsr	rspline
	movem.l	(sp)+,a0/a1
	movem.l	(sp)+,a2/a3/a4/a5
	exg     a4,a5
	movem.l	(sp)+,d6/d7
	bra	rspline

;---------------------------------------------------------------------------

	END
