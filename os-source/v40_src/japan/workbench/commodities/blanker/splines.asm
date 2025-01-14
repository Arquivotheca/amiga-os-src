	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "hardware/custom.i"
	INCLUDE "hardware/blit.i"
	INCLUDE "hardware/dmabits.i"

        LIST

;---------------------------------------------------------------------------

	XDEF _DrawSpline

;---------------------------------------------------------------------------

	XREF _custom
	XREF _oldx
	XREF _oldy
	XREF _oldwidth
	XREF _oldptr
	XREF _oldcol
	XREF _GfxBase
	XREF _LVOWaitBlit

;---------------------------------------------------------------------------

_DrawSpline:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(a7),a0
	move.l	52(a7),a1
	move.l	56(a7),a2
	move.l	60(a7),a3
	move.l	64(a7),a5
	move.l	68(a7),a6
	move.l	72(a7),d6
	move.l	76(a7),d7
	bsr.s	rspline
	movem.l	(sp)+,d2-d7/a2-a6
	rts

rspline
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
	ble	pushem
	move.l	a5,d0
	sub.l	a0,d0
	move.l	a6,d1
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
	movem.l	d2-d5/a0-a1,-(sp)
	move.l	_oldx,d2
	move.l	_oldy,d3
	move.l	d0,_oldx
	move.l	d1,_oldy
	move.l	_oldwidth,d4
	move.l	_oldptr,a0
	bsr	mdraw
	movem.l	(sp)+,d2-d5/a0-a1
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
	sub.l	a6,d1
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
	move.l	a6,d1
	add.l	d7,d1
	asr.l	#1,d1
	movem.l	d0/d1,-(sp)
	move.l	a2,d2
	add.l	a5,d2
	asr.l	#1,d2
	move.l	a3,d3
	add.l	a6,d3
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
	move.l	d1,a6
	bsr	rspline
	movem.l	(sp)+,a0/a1
	movem.l	(sp)+,a2/a3/a5/a6
	movem.l	(sp)+,d6/d7
	bra	rspline
;
;
;   Our entry point.
;
mdraw:
	lea.l	_custom,a1	; Manx requires this
	sub.w	d0,d2		; calculate dx
	bmi.s	xneg		; if negative, octant is one of [3,4,5,6]
	sub.w	d1,d3		; calculate dy   ''   is one of [1,2,7,8]
	bmi.s	yneg		; if negative, octant is one of [7,8]
	cmp.w	d3,d2		; cmp |dx|,|dy|  ''   is one of [1,2]
	bmi.s	ygtx		; if y>x, octant is 2
	moveq.l	#OCTANT1+LINEMODE,d5	; otherwise octant is 1
	bra.s	lineagain	; go to the common section
ygtx:
	exg	d2,d3		; X must be greater than Y
	moveq.l	#OCTANT2+LINEMODE,d5	; we are in octant 2
	bra.s	lineagain	; and common again.
yneg:
	neg.w	d3		; calculate abs(dy)
	cmp.w	d3,d2		; cmp |dx|,|dy|, octant is [7,8]
	bmi.s	ynygtx		; if y>x, octant is 7
	moveq.l	#OCTANT8+LINEMODE,d5	; otherwise octant is 8
	bra.s	lineagain
ynygtx:
	exg	d2,d3		; X must be greater than Y
	moveq.l	#OCTANT7+LINEMODE,d5	; we are in octant 7
	bra.s	lineagain
xneg:
	neg.w	d2		; dx was negative! octant is [3,4,5,6]
	sub.w	d1,d3		; we calculate dy
	bmi.s	xyneg		; if negative, octant is one of [5,6]
	cmp.w	d3,d2		; otherwise it's one of [3,4]
	bmi.s	xnygtx		; if y>x, octant is 3
	moveq.l	#OCTANT4+LINEMODE,d5	; otherwise it's 4
	bra.s	lineagain
xnygtx:
	exg	d2,d3		; X must be greater than Y
	moveq.l	#OCTANT3+LINEMODE,d5	; we are in octant 3
	bra.s	lineagain
xyneg:
	neg.w	d3		; y was negative, in one of [5,6]
	cmp.w	d3,d2		; is y>x?
	bmi.s	xynygtx		; if so, octant is 6
	moveq.l	#OCTANT5+LINEMODE,d5	; otherwise, octant is 5
	bra.s	lineagain
xynygtx:
	exg	d2,d3		; X must be greater than Y
	moveq.l	#OCTANT6+LINEMODE,d5	; we are in octant 6
lineagain:
	mulu.w	d4,d1		; Calculate y1 * width
	ror.l	#4,d0		; move upper four bits into hi word
	add.w	d0,d0		; multiply by 2
	add.l	d1,a0		; ptr += (x1 >> 3)
	add.w	d0,a0		; ptr += y1 * width
	swap	d0		; get the four bits of x1
	or.w	_oldcol,d0	; or with USEA, USEC, USED, F=A~C+~AC
	lsl.w	#2,d3		; Y = 4 * Y
	add.w	d2,d2		; X = 2 * X
	move.w	d2,d1		; set up size word
	lsl.w	#5,d1		; shift five left
	add.w	#$42,d1		; and add 1 to height, 2 to width
waitblit:
	move.l	a6,-(sp)
	move.l	_GfxBase,a6
	jsr	_LVOWaitBlit(a6)
	move.l	(sp)+,a6

	move.w	d3,bltbmod(a1)	; B mod = 4 * Y
	sub.w	d2,d3
	ext.l	d3
	move.l	d3,bltapt(a1)	; A ptr = 4 * Y - 2 * X
	bpl.s	lineover	; if negative,
	or.w	#SIGNFLAG,d5	; set sign bit in con1
lineover:
	move.w	d0,bltcon0(a1)	; write control registers
	move.w	d5,bltcon1(a1)
	move.w	d4,bltcmod(a1)	; C mod = bitplane width
	move.w	d4,bltdmod(a1)	; D mod = bitplane width
	sub.w	d2,d3
	move.w	d3,bltamod(a1)	; A mod = 4 * Y - 4 * X
	move.w	#$8000,bltadat(a1)	; A data = 0x8000
	moveq.l	#-1,d5		; Set masks to all ones
	move.l	d5,bltafwm(a1)	; we can hit both masks at once
	move.l	a0,bltcpt(a1)	; Pointer to first pixel to set
	move.l	a0,bltdpt(a1)
	move.w	d1,bltsize(a1)	; Start blit
	rts			; and return, blit still in progress.

;---------------------------------------------------------------------------

	END
