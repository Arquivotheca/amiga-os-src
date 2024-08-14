
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeedoubbas.library/IEEEDPDiv ***********************************
*
*   NAME
*	IEEEDPDiv -- divide one double precision IEEE by another
*
*   SYNOPSIS
*	  x   = IEEEDPDiv(  y  ,  z  );
*	d0/d1		  d0/d1 d2/d3
*
*	double	x,y,z;
*
*   FUNCTION
*	Compute x = y / z in IEEE double precision.
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
*	IEEEDPMul
******************************************************************************
;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1


	include	"float.i"

	xref	_CXREN5		; Renormalize number
	xref	_cxadj5		; Adjust after mult/divide
	xref	_cxovf5		; Overflow handler
	xref	_cxnan5		; NAN handler
	xref	_cxind5		; Indefinite handler
	xref	_cxinf5		; Infinite handler
	xref	_cxzer5		; Zero handler


;*****************************************************
;*	Division interface routines.                 *
;*****************************************************

;**	Entry:
;**		d0,d1 = Operand 1
;**		d2,d3 = Operand 2
;**	Exit:
;**		d0,d1 = Result
;**	Uses:
;**		a1

;******************************************************
;*	Floating Divide                               *
;******************************************************

;**	Entry:
;**		d0,d1 = operand 1
;**		d2,d3 = operand 2
;**	Exit:
;**		d0,d1 = result
;**	Uses:
;**		d4-d7, a1

	xdef	MIIEEEDPDiv
MIIEEEDPDiv:
	movem.l	d2-d7,-(a7)	; Save d2-d7

;*****
;*****	Unpack sign and form result sign.
;*****

	move.w	#$8000,d6	; Sign mask
	move.w	#$7ff0,d7	; Exponent mask
	move.w	d7,a0		; will be used again, sign extends also

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

div10:
	cmp.w	d7,d0		; Exponent NAN or INF?
	blt.s	div20		; - nope

;**	If X is NAN, return X without further ado.

	cmp.w	d2,d0		; Y higher precedence?
	blt.s	div13		; - why yes

	cmp.l	a0,d0		; Test upper mantissa
	bne.s	div11		; - NAN

	tst.l	d1		; Test lower mantissa
	beq.s	div12		; - INF

div11:
	bra	_cxnan5		; Process NAN

;**	X is infinite.  If Y is NAN, return Y without further ado.

div12:
	cmp.w	d7,d2		; Y Exponent NAN or INF?
	blt.s	div15		; - nope

	cmp.l	a0,d2		; Test upper mantissa
	bne.s	div13		; - NAN

	tst.l	d3		; Test lower mantissa
	beq.s	div14		; - INF

div13:
	move.l	d2,d0		; Process NAN
	move.l	d3,d1
	bra	_cxnan5

;**	X and Y are infinite.  Result is indefinite.

div14:
	bra	_cxind5		; Process indefinite.

;**	X is infinite.  If Y is zero, we have a zero divide.

div15:
	tst.l	d2		; Test upper Y
	bne.s	div17		; - non-zero

	tst.l	d3		; Test lower Y
	bne.s	div17		; - non-zero

;**	Handle divide by zero.

div16:
	fptrap	3		; Divide by zero

	move.l	a0,d0	; Construct INF
	eor.w	d4,d0
	moveq	#0,d1
	swap	d0
	movem.l	(sp)+,d2-d7
	rts

;**	Otherwise, result is infinite.

div17:
	bra	_cxinf5		; Infinite

;*****
;*****	Test for Y INF or NAN.
;*****

div20:
	cmp.w	d7,d2		; Exponent NAN or INF?
	blt.s	div30		; - no

;**	If Y is NAN, result is Y

	cmp.l	a0,d2	; Test Y upper
	bne.s	div21		; - NAN

	tst.l	d3		; Test Y lower
	beq.s	div22		; - INF

div21:
	move.l	d2,d0		; process NAN
	move.l	d3,d1
	bra	_cxnan5

;**	Y is infinite, so result is zero

div22:
	bra	_cxzer5		; Return signed zero


;*****
;*****	Process zero X exponent.
;*****

div30:
	move.w	d0,d5		; Unpack exponent
	and.w	d7,d5
	bne.s	div33		; - non-zero

	tst.l	d0		; Test upper mantissa
	bne.s	div32		; - non-zero

	tst.l	d1		; Test lower mantissa
	bne.s	div32		; - non-zero

;**	X is zero.   If Y is zero, result is indefinite.
;**	Otherwise result is zero.

	tst.l	d2		; Test Y upper mantissa
	bne.s	div31		; - non-zero

	tst.l	d3		; Test Y lower mantissa
	bne.s	div31		; - non-zero

	bra	_cxind5

div31:
	bra	_cxzer5		; Return signed zero

;**	If X is denormalized, renormalize it.   Otherwise
;**	strip off the exponent and put in the hidden bit.

div32:
	bsr	_CXREN5		; Renormalize zero exponent
	bra.s	div40

div33:
	eor.w	d5,d0		; Get mantissa
	eor.w	#$0010,d0	; Add hidden bit

;*****
;*****	Process zero Y exponent.
;*****

div40:
	and.w	d2,d7		; Get exponent Y
	bne.s	div42		; - non-zero

;**	If Y is zero, get a zero divide fault.

	tst.l	d2		; Test upper mantissa
	bne.s	div41		; - non-zero

	tst.l	d3		; Test lower mantissa
	bne.s	div41		; - non-zero

	bra.s	div16		; Zero divide

;**	If Y is denormalized, renormalize it.   Otherwise
;**	strip off the exponent and put in the hidden bit.

div41:
	exg	d0,d2		; Switch X,Y for denormalize
	exg	d1,d3
	exg	d5,d7

	bsr	_CXREN5		; Renormalize zero exponent

	exg	d0,d2		; Switch back
	exg	d1,d3
	exg	d5,d7
	bra.s	div50

div42:
	eor.w	d7,d2		; Get mantissa
	eor.w	#$0010,d2	; Add hidden bit


;*****
;*****	Prepare for divide.
;*****

div50:
	sub.w	#$3fe0,d7	; Form new exponent
	sub.w	d7,d5	
	bvc.s	div51		; - no overflow

	bra	_cxovf5 	; Divide overflow

div51:
	swap	d0		; Shift X left 4
	asl.l	#1,d1
	roxl.l	#1,d0

;	move.l	d1,d7
;	lsl.l	#1,d0
;	lsl.l	#1,d1
;	rol.l	#1,d7
;	eor.w	d1,d7
;	eor.w	d7,d0

;	swap	d0		; Shift X left 4
;	move.l	d1,d7
;	lsl.l	#2,d0
;	lsl.l	#2,d1
;	rol.l	#2,d7
;	eor.w	d1,d7
;	eor.w	d7,d0

	swap	d2		; Shift Y left 11
	move.l	d3,d7
	moveq	#11,d6
	lsl.l	d6,d2
	lsl.l	d6,d3
	rol.l	d6,d7
	eor.w	d3,d7
	eor.w	d7,d2

	swap	d4		; Save lsb d4,d5
	move.w	d5,d4
	move.l	d4,a1

;*****
;*****	Divide
;*****

;**	First divide step.
;**
;**	q0 = x01 / y0
;*	r03 = (x01 % y0) * m3 + x23 * m - q0 * y13
;**	if (r03 < 0) {
;**		q0 -= 1
;**		r03 += y03
;**		}

	sdebug	d0,d1
	sdebug	d2,d3

div60:
	swap	d2		; q0 = x01 % y0
	divu	d2,d0
	move.w	d0,d4

	swap	d1		; r03 = x01 % y * m3 + x23 * m
	move.w	d1,d0
	clr.w	d1

	swap	d2		; q0 * y1
	move.w	d2,d5
	mulu	d4,d5

	swap	d3		; q0 * y2
	move.w	d3,d6
	mulu	d4,d6

	swap	d3		; q0 * y3
	move.w	d3,d7
	mulu	d4,d7

	swap	d7		; q0 * y13
	add.w	d6,d7
	swap	d7
	clr.w	d6
	swap	d6
	addx.l	d5,d6

	sub.l	d7,d1		; r03 -= q0 * y13
	subx.l	d6,d0
	bcc	div6_end	; - result >= 0

	subq.w	#1,d4		; q0 -= 1
	add.l	d3,d1		; r03 += y03
	addx.l	d2,d0

div6_end:
	move.w	d4,-(sp)	;save q0 on stack

	clr.l	d5		;set r4 to 0

;**	Second divide step.
;**
;**	q1 = r01 / y0
;**	if (overflow on divide) {
;**		q1 = m - 1
;*		r02 = r03 - m * y02
;**		}
;**	else {
;*		r02 = (r01 % y0) * m2 + r23 - q1 * y12
;**		}
;**	if (r02 < 0) {
;**		q1 -= 1
;**		r02 += y02
;**		if (r02 < 0) {
;**			q1 -= 1
;**			r02 += y02
;**			}
;**		}


div70:
	sdebug	d0,d1
	sdebug	d2,d3

;	don't drop y3,  Dale
;	clr.w	d3		; Drop y3

	move.l	d0,d6		; q1 = r01 / y0
	swap	d2
	divu	d2,d0
	bvc.s	div71		; - no overflow

	clr.w	d4		; q1 = m % m

	move.l	d6,d0		; r02 = r03 - m * y02
	sub.l	d3,d1
	swap	d2
	subx.l	d2,d0
	swap	d0
	swap	d1
	move.w	d1,d0
	clr.w	d1
	sdebug	d0,d1
	bra.s	div72

div71:
	move.w	d0,d4		; r02 = (r01 % y0) * m2 + r23
	swap	d1
	move.w	d1,d0
	clr.w	d1

	swap	d2		; q1 * y1
	move.w	d2,d5
	mulu	d4,d5

	swap	d3		; q1 * y2
	move.w	d3,d6
	mulu	d4,d6

	swap	d3		; q1 * y3
	move.w	d3,d7
	mulu	d4,d7
	
	swap	d7		; q1 * y13
	add.w	d6,d7
	swap	d7
	clr.w	d6
	swap	d6
	addx.l	d5,d6

	sub.l	d7,d1		; r02 -= q1 * y12
	subx.l	d6,d0
	bcc.s	div7_end	; - r02 >= 0

div72:
	subq.w	#1,d4		; q1 -= 1

;	clr.w	d3		; now kill y03, Dale
	add.l	d3,d1		; r02 += y02
	addx.l	d2,d0
	bcs.s	div7_end	; - r02 >= 0

	subq.w	#1,d4		; q1 -= 1

	add.l	d3,d1		; r02 += y02
	addx.l	d2,d0

div7_end:
	move.w	d4,-(sp)	;save q1 on stack

;**	Third divide step.
;**
;**	q2 = r01 / y0
;**	if (overflow on divide) {
;**		q2 = m
;*		r01 = r02 - m * y01
;**		}
;**	else {
;*		r01 = (r01 % y0) * m + r2 - q1 * y1
;**		}
;**	if (r01 < 0) {
;**		r01 += y01
;**		q2 -= 1
;**		if (r01 < 0) {
;**			r01 += y01
;**			q2 -= 1
;**			}
;**		}

div80:
	sdebug	d0,d1
	sdebug	d2,d3

	move.l	d0,d6		; q2 = r01 / y0
	swap	d2
	divu	d2,d0
	bvc.s	div81		; - no overflow

	clr.w	d4		; q2 = m % m

	move.l	d6,d0		; r01 = (r02 - m * y01)
	sub.l	d3,d1		; Dale addition
	swap	d2
	subx.l	d2,d0		; was sub.w	d2,d0	Dale
	swap	d0
	swap	d1
	move.w	d1,d0
	bra.s	div82

div81:
	move.w	d0,d4		; r01 = (r01 % y0) * m + r2
	swap	d1
	move.w	d1,d0
	clr.w	d1

	swap	d2		; q1 * y1
	move.w	d2,d5
	mulu	d4,d5

	swap	d3		; q1 * y2
	move.w	d3,d6
	mulu	d4,d6

	swap	d3		; q1 * y3
	move.w	d3,d7
	mulu	d4,d7
	
	swap	d7		; q1 * y13
	add.w	d6,d7
	swap	d7
	clr.w	d6
	swap	d6
	addx.l	d5,d6

	sub.l	d7,d1		; r02 -= q1 * y12
	subx.l	d6,d0
	bcc.s	div8_end	; - r01 >= 0
div82:
	sub.w	#1,d4		; q2 -= 1

	add.l	d2,d0		; r01 += y01
	bcs.s	div8_end	; - r01 >= 0

	subq.w	#1,d4		; q2 -= 1

	add.l	d2,d0		; r01 += y01

div8_end:
	move.w	d4,-(sp)	;save q2 on stack

;**	Fourth divide step.
;**
;**	q3 = r01 / y0
;**	if (overflow on divide) q3 = m - 1

div90:
	sdebug	d0,d1
	sdebug	d2,d3

	swap	d2		; q3 = r01 / y0
	divu	d2,d0
	bvc.s	div91

	moveq	#0,d4		; q3 = m-1
	bra.s	div92

div91:				; calc last bit of remainder
	move.w	d0,d4		; q3 in d4
	swap	d1
	move.w	d1,d0		; r0 = (r01 % y0) *m + r2
	clr.w	d1

	swap	d2		; q1 * y1
	move.w	d2,d5
	mulu	d4,d5

	swap	d3		; q1 * y2
	move.w	d3,d6
	mulu	d4,d6

	swap	d3		; q1 * y3
	move.w	d3,d7
	mulu	d4,d7

	sdebug	d4,d5
	sdebug	d6,d7
	
	swap	d7		; q1 * y13
	add.w	d6,d7
	swap	d7
	clr.w	d6
	swap	d6
	addx.l	d5,d6

	sdebug	d0,d1
	sdebug	d6,d7

	sub.l	d7,d1		; r02 -= q1 * y12
	subx.l	d6,d0
	bcc.s	div9_end	; - r01 >= 0

div92:
	sub.w	#1,d4		;q3 -= 1

div9_end:
	move.w	d4,-(sp)

div100:
	move.l	(sp)+,d1
	move.l	(sp)+,d0
	swap	d0
	swap	d1

	move.l	a1,d4		; restore d4,d5
	move.w	d4,d5
	swap	d4

	asl.l	#1,d1
	roxl.l	#1,d0

;	asl.l	#1,d1
;	roxl.l	#1,d0
;	asl.l	#1,d1
;	roxl.l	#1,d0

	bra	_cxadj5

	end
