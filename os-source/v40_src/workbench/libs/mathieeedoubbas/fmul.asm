
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeedoubbas.library/IEEEDPMul ***********************************
*
*   NAME
*	IEEEDPMul -- multiply one double precision IEEE number by another
*
*   SYNOPSIS
*	  x   = IEEEDPMul(  y  ,  z  );
*	d0/d1		  d0/d1 d2/d3
*
*	double	x,y,z;
*
*   FUNCTION
*	Compute x = y * z in IEEE double precision.
*
*   INPUTS
*	y -- IEEE double precision floating point value
*	z -- IEEE double precision floating point value
*
*   RESULT
*	x -- IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPDiv
******************************************************************************

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	include	"float.i"

	xref	_CXREN5		; Renormalize operand
	xref	_cxadj5		; Adjust after multiply/divide
	xref	_cxovf5		; Overflow handler
	xref	_cxnan5		; NAN handler
	xref	_cxind5		; Indefinite handler
	xref	_cxinf5		; Infinite handler
	xref	_cxzer5		; Zero handler

PRESHIFT	equ	7


mul20_bar:
;**	If X is NAN, return X without further ado.

	cmp.w	d2,d0		; Y higher precedence?
	blt.s	mul13		; - why yes

	cmp.l	a0,d0	; Test upper mantissa
	bne.s	mul11		; - NAN

	tst.l	d1		; Test lower mantissa
	beq.s	mul12		; - INF

mul11:
donan:
	bra	_cxnan5		; Process NAN

;**	If Y is NAN, return Y without further ado.

mul12:
	cmp.w	d7,d2		; Y Exponent NAN or INF?
	blt.s	mul14		; - nope

	cmp.l	a0,d2	; Test upper mantissa
	bne.s	mul13		; - INF

	tst.l	d3		; Test lower mantissa
	beq.s	mul14		; - INF

mul13:
	move.l	d2,d0		; Process NAN
	move.l	d3,d1
	bra.s	mul11

;**	X is infinite.  If Y is zero, result is indefinite.

mul14:
	tst.l	d2		; Test upper Y
	bne.s	mul15		; - non-zero

	tst.l	d3		; Test lower Y
	beq	_cxind5		; Indefinate

;**	Otherwise, result is infinite.

mul15:
doinf:
	bra	_cxinf5		; Infinite

mul30_bar:
;**	If Y is NAN, result is Y

	cmp.l	a0,d2	; Test Y upper
	bne.s	mul21		; - NAN

	tst.l	d3		; Test Y lower
	beq.s	mul22		; - INF

mul21:
	move.l	d2,d0
	move.l	d3,d1
	bra	donan

;**	Y is INF.  If X == 0, result is indefinite.

mul22:
	tst.l	d0		; Test X upper
	bne.s	doinf		; - non-zero

	tst.l	d1		; Test X lower
	bne.s	doinf		; - non-zero

	bra	_cxind5		; Indefinite result

;*************************************************************
;*	Multiply interface routines.                         *
;*************************************************************

;**	Entry:
;**		d0,d1 = Operand 1
;**		d2,d3 = Operand 2
;**	Exit:
;**		d0,d1 = Result
;**	Uses:

	xdef	MIIEEEDPMul
MIIEEEDPMul:
	movem.l	d2-d7,-(a7)	; Save d2-d7

;*****
;*****	Unpack sign and form result sign.
;*****
	move.w	#$8000,d6	; Sign mask
	move.w	#$7ff0,a0	; sign extends
	move.w	a0,d7		; Exponent mask

	swap	d0		; Swap mantissas
	swap	d2

	move.w	d0,d4		; Isolate sign X
	and.w	d6,d4		; and magnitude X
	eor.w	d4,d0

	and.w	d2,d6		; Isolate sign Y
	eor.w	d6,d2		; and magnitude Y

	eor.w	d6,d4		; Get result sign

;*****
;*****	Process X NAN or INF.
;*****

mul10:
	cmp.w	d7,d0		; Exponent NAN or INF?
;	blt.s	mul20		; - nope
	bge.s	mul20_bar	; - yes

;*****
;*****	Test for Y INF or NAN.
;*****

mul20:
	cmp.w	d7,d2		; Exponent NAN or INF?
;	blt.s	mul30		; - no
	bge.s	mul30_bar	; - yes

;*****
;*****	Process zero X exponent.
;*****

mul30:
	move.w	d0,d5		; Unpack exponent
	and.w	d7,d5
	bne.s	mul32		; - non-zero; jump forward

	tst.l	d0		; Test upper mantissa
	bne.s	mul31		; - non-zero

	tst.l	d1		; Test lower mantissa
	bne.s	mul31		; - non-zero

	bra	_cxzer5

mul31:
	bsr	_CXREN5		; Renormalize zero exponent
	bra.s	mul40



;**	If Y is zero, return zero.

mul42_bar:
	tst.l	d2		; Test upper mantissa
	bne.s	mul41		; - non-zero

	tst.l	d3		; Test lower mantissa
	bne.s	mul41		; - non-zero

	bra	_cxzer5		; Return signed zero

mul41:
	exg	d0,d2		; Switch X,Y for denormalize
	exg	d1,d3
	exg	d5,d7

	bsr	_CXREN5		; Renormalize zero exponent

	exg	d0,d2		; Switch back
	exg	d1,d3
	exg	d5,d7
	bra.s	mul50

doovf:
	bra	_cxovf5

mul32:
	eor.w	d5,d0		; Get mantissa
	eor.w	#$0010,d0	; Add hidden bit

;*****
;*****	Process zero Y exponent.
;*****

mul40:
	and.w	d2,d7		; Get exponent Y
	;bne.s	mul42		; - non-zero
	beq.s	mul42_bar	; - non-zero


mul42:
	eor.w	d7,d2		; Get mantissa
	or.w	#$0010,d2	; Add hidden bit


;*****
;*****	Prepare for multiply.
;*****

mul50:
	sub.w	#$3ff0,d5	; Subtract exponent base
	add.w	d7,d5
	bvs.s	doovf		; - no overflow

mul51:
	swap	d0		; Left justify X
;	move.l	d1,d7
;	lsl.l	#8,d0
;	lsl.l	#8,d1
;	rol.l	#8,d7
;	eor.w	d1,d7
;	eor.w	d7,d0

	swap	d2		; Left justify Y
;	move.l	d3,d7
;	lsl.l	#8,d2
;	lsl.l	#8,d3
;	rol.l	#8,d7
;	eor.w	d3,d7
;	eor.w	d7,d2

* if we want to shift left by 8, it is quicker to use:
;	lsl.l	#8,d0
;	rol.l	#8,d1
;	move.b	d1,d0
;	clr.b	d1


;*****
;*****	Multiply
;*****

;**	d0,d1 = mantissa X
;**	d2,d3 = mantissa Y
;**	d4 = result sign
;**	d5 = result exponent
;**
;**	Sum:
;**				x0 * y3
;**				x1 * y2
;**				x2 * y1
;**				x3 * y0
;**			x0 * y2
;**			x1 * y1			
;**			x2 * y0			
;**		x0 * y1
;**		x1 * y0
;**	x0 * y0
;**	---------------------------------
;**	z0	z1	z2	z3

mul60:
	move.l	d1,d7
	if =	.extend			; x2/x3=#0?
		lsl.l	#PRESHIFT,d0		; do fast justify
		move.l	d3,d7
		if =			; y2/y3=#0?
			lsl.l	#PRESHIFT,d2	; do fast justify
*                       do superfast version
*                       only need:
*  x0 * y0
*       x0 * y1
*       y0 * x1
*            x1 * y1
*---------------------
*  z0   z1   z2   z3
			move.w	d0,d1
			mulu	d2,d1		; (x1*y1)
			move.w	d0,d7
			move.w	d2,d6
			swap	d2		; y1/y0
			mulu	d2,d7		; (y0*x1)
			swap	d0		; x1/x0
			mulu	d0,d6		; (y1*x0)
			add.l	d6,d7		; no carry
			mulu	d2,d0		; (x0 * y0)

			swap	d1
			add.w	d7,d1
			clr.w	d7
			swap	d7
			addx.l	d7,d0
			swap	d1
			bra	_cxadj5		; adjust the result
		else
*			d1.l==#0, use modified algorgithm
;**	d0,d1 = mantissa X,x2/x3=#0
;**	d2,d3 = mantissa Y
;**	d4 = result sign
;**	d5 = result exponent
;**
;**	Sum:
;**				x0 * y3
;**				x1 * y2
;**			x0 * y2
;**			x1 * y1			
;**		x0 * y1
;**		x1 * y0
;**	x0 * y0
;**	---------------------------------
;**	z0	z1	z2	z3
;			finish y justification
			lsl.l	#PRESHIFT,d2
			lsl.l	#PRESHIFT,d3
			rol.l	#PRESHIFT,d7
			eor.w	d3,d7
			eor.w	d7,d2
go_half_fast:

			swap	d0		; x1/x0

			move.w	d3,d1
			mulu	d0,d1		; y3*x0

			swap	d3		; y3/y2

			swap	d0		; x0/x1

			move.w	d3,d6
			mulu	d0,d6		; y2*x1
			add.l	d6,d1		; z2/z3 += (y2*x1)
;	above may generate, xcarry
			clr.w	d1
			swap	d1

			move.l	d0,d6		; d6 = x0/x1
			swap	d0		; x1/x0
			swap	d2		; y1/y0

			mulu	d0,d3		; x0*y2
			addx.l	d3,d1		; z2/z3 += (x0*y2)

			move.w	d2,d7
			mulu	d2,d0		; (x0*y0)

			swap	d2		; y0/y1

			move.w	d6,d3
			mulu	d2,d3		; (x1*y1)
			add.l	d3,d1		; z2/z3 += (x1*y1)
			clr.l	d3
			addx.l	d3,d0

			mulu	d6,d7		; (x1*y0)
			swap	d6		; x1/x0
			mulu	d2,d6		; (x0*y1)
			add.l	d6,d7		; (x0*y1)+(x1*y0)

			swap	d1
			add.w	d7,d1
			clr.w	d7
			swap	d7
			addx.l	d7,d0
			swap	d1
			bra	_cxadj5		; adjust the result
		endif
	else .extend
;		continue justifying x
		lsl.l	#PRESHIFT,d0
		lsl.l	#PRESHIFT,d1
		rol.l	#PRESHIFT,d7
		eor.w	d1,d7
		eor.w	d7,d0

		move.l	d3,d7
		if =			; is y2/y3=#0?
			exg.l	d0,d2
			move.l	d1,d3
			lsl.l	#PRESHIFT,d0	; quick justify
			bra	go_half_fast
		endif
;		finish y justification
		lsl.l	#PRESHIFT,d2
		lsl.l	#PRESHIFT,d3
		rol.l	#PRESHIFT,d7
		eor.w	d3,d7
		eor.w	d7,d2

		swap	d0		; x1/x0
		swap	d2		; y1/y0
		move.w	d1,d7
		mulu	d2,d7		; x3*y0

		move.w	d3,d6
		mulu	d0,d6		; y3*x0
		add.l	d6,d7
		clr.w	d7
		swap	d7		; (y3*x0)+(x3*y0)  -> z2/z3

		swap	d1		; x3/x2
		swap	d3		; y3/y2

		swap	d0		; x0/x1
		swap	d2		; y0/y1

		move.w	d2,d6
		mulu	d1,d6		; x2*y1
		clr.w	d6
		swap	d6
		add.l	d6,d7		; z2/z3 += (x2*y1)

		move.w	d3,d6
		mulu	d0,d6		; y2*x1
		clr.w	d6
		swap	d6
		add.l	d6,d7		; z2/z3 += (y2*x1)

		move.l	d0,d6		; x0/x1
		swap	d0		; x1/x0
		swap	d2		; y1/y0

		mulu	d2,d1		; y0*x2
		add.l	d7,d1		; z2/z3 += (y0*x2)

		mulu	d0,d3		; x0*y2
		add.l	d3,d1		; z2/z3 += (x0*y2)

		move.w	d2,d7
		mulu	d2,d0		; (x0*y0)

		swap	d2		; y0/y1

		move.w	d6,d3
		mulu	d2,d3		; (x1*y1)
		add.l	d3,d1		; z2/z3 += (x1*y1)
		clr.l	d3
		addx.l	d3,d0

		mulu	d6,d7		; (x1*y0)
		swap	d6		; x1/x0
		mulu	d2,d6		; (x0*y1)
		add.l	d6,d7		; (x0*y1)+(x1*y0)

		swap	d1
		add.w	d7,d1
		clr.w	d7
		swap	d7
		addx.l	d7,d0
		swap	d1
	endif

	bra	_cxadj5		; adjust the result

	end
