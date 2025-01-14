*******************************************************************************
*
*	$Id: fdiv.asm,v 40.1 93/03/11 17:22:09 mks Exp $
*
*******************************************************************************

******* mathieeesingbas.library/IEEESPDiv ***********************************
*
*   NAME
*	IEEESPDiv -- divide one single precision IEEE by another
*
*   SYNOPSIS
*	  x   = IEEESPDiv(  y  ,  z  );
*	 d0		   d0    d1
*
*	float	x,y,z;
*
*   FUNCTION
*	Compute x = y / z in IEEE single precision.
*	Note that the Motorola fast floating point Div routine reverses
*	the order of the arguments for the C interface, although the
*	dividend is still in d0 and the divisor is in d1.
*
*   INPUTS
*	y -- IEEE single precision floating point value
*	z -- IEEE single precision floating point value
*
*   RESULT
*	x -- IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPMul
******************************************************************************

********************************************************************************
*
* If not FPU-Only, we need this...
*
	IFND	CPU_FPU
*
;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8  bk=0  ma=1

	include	"float.i"

	xref	_CXREN4		; Renormalize
	xref	_CXADJ4		; Adjust after multiply/divide
	xref	_CXIND4		; Indefinite handler
	xref	_CXINF4		; Infinite handler
	xref	_CXOVF4		; Overflow handler
	xref	_CXNAN4		; NAN handler
	xref	_CXZER4		; Zero handler
	xref	_CXFERR		; Exception handler


********************************************************************************
*
* The following is 68000 CPU and up.  It is only built into the 68000 compatible
* version of the library...
*
	IFD	CPU_68000

;*************************************************************
;*	Division interface routines.                         *
;*************************************************************

;**	Entry:
;**		d0 = single dividend
;**		d1 = single divisor
;**	Exit:
;**		d0 = single quotient
;**	Uses:
;**		a0,a1,d1

	xdef	MIIEEESPDiv
MIIEEESPDiv:
	move.l	d6,a0		; Save d2,d6,d7
	move.l	d7,a1
	move.l	d2,-(a7)

	bsr.s	div0		; Divide operands

	move.l	a0,d6		; Restore d2,d6,d7
	move.l	a1,d7
	move.l	(a7)+,d2
	rts



;******************************************************
;*	Floating Divide                               *
;******************************************************

;**	Entry:
;**		d0 = single dividend
;**		d1 = single divisor
;**	Exit:
;**		d0 = single quotient
;**	Uses:
;**		d1,d6,d7

div0:

;*****
;*****	Unpack sign and form result sign.
;*****

	move.l	d0,d6		; Find sign of result
	eor.l	d1,d6

	move.w	#$7f80,d7	; Exponent mask

	swap	d0		; Remove (X) sign
	and.w	#$7fff,d0

	swap	d1		; Remove (Y) sign
	and.w	#$7fff,d1

;*****
;*****	Process X NAN or INF.
;*****

div10:
	cmp.w	d7,d0		; Exponent NAN or INF?
	blt.s	div20		; - nope

;**	If X is NAN, return X without further ado.

	move.w	d1,d2		; Y higher precedence?
	or.w	#$0007,d2
	sub.w	d0,d2
	and.w	#$fff8,d2
	bgt.s	div13		; - why yes

	cmp.l	#$7f80,d0	; Test upper mantissa
	beq.s	div12		; - INF

	bra	_CXNAN4		; Process NAN

;**	X is infinite.  If Y is NAN, return Y without further ado.

div12:
	cmp.w	d7,d1		; Y Exponent NAN or INF?
	blt.s	div15		; - nope

	cmp.l	#$7f80,d1	; Test upper mantissa
	beq.s	div14		; - INF

div13:
	move.l	d1,d0		; Process NAN
	bra	_CXNAN4

;**	X and Y are infinite.  Result is indefinite.

div14:
	bra	_CXIND4		; Handle indefinite

;**	X is infinite.  If Y is zero, we have a zero divide.

div15:
	tst.l	d1		; Test upper Y
	bne.s	div17		; - non-zero

div16:
	fptrap	3		; Zero divide

	move.l	d6,d0		; Construct Signed INF
	and.l	#$80000000,d0
	eor.l	#$7f800000,d0
	rts

;**	Otherwise, result is infinite.

div17:
	bra	_CXINF4		; Handle infinite

;*****
;*****	Test for Y INF or NAN.
;*****

div20:
	cmp.w	d7,d1		; Exponent NAN or INF?
	blt.s	div30		; - no

;**	If Y is NAN, result is Y

	cmp.l	#$7f80,d1	; Test Y upper
	beq.s	div22		; - INF

	move.l	d1,d0		; process NAN
	bra	_CXNAN4

;**	Y is infinite, so result is zero

div22:
	bra	_CXZER4		; Return signed zero


;*****
;*****	Process zero X exponent.
;*****

div30:
	move.w	d0,d6		; Unpack exponent
	and.w	d7,d6
	bne.s	div33		; - non-zero

	tst.l	d0		; Test upper mantissa
	bne.s	div32		; - non-zero

;**	X is zero.   If Y is zero, result is indefinite.
;**	Otherwise result is zero.

	tst.l	d1		; Test Y upper mantissa
	bne.s	div31		; - non-zero

	bra	_CXIND4		; Handle indefinite

div31:
	bra	_CXZER4		; Return signed zero

;**	If X is denormalized, renormalize it.   Otherwise
;**	strip off the exponent and put in the hidden bit.

div32:
	bsr	_CXREN4		; Denormalize zero exponent
	bra.s	div40

div33:
	eor.w	d6,d0		; Get mantissa
	eor.w	#$0080,d0	; Add hidden bit

;*****
;*****	Process zero Y exponent.
;*****

div40:
	and.w	d1,d7		; Get exponent Y
	bne.s	div42		; - non-zero

;**	If Y is zero, get a zero divide fault.

	tst.l	d1		; Test mantissa
	bne.s	div41		; - non-zero

	bra.s	div16		; Zero divide

;**	If Y is denormalized, renormalize it.   Otherwise
;**	strip off the exponent and put in the hidden bit.

div41:
	exg	d0,d1		; Switch X,Y for denormalize
	exg	d6,d7

	bsr	_CXREN4		; Denormalize zero exponent

	exg	d0,d1		; Switch back
	exg	d6,d7
	bra.s	div50

div42:
	eor.w	d7,d1		; Get mantissa
	eor.w	#$0080,d1	; Add hidden bit


;*****
;*****	Prepare for divide.
;*****

div50:
	sub.w	#$3f00,d7	; Form new exponent
	sub.w	d7,d6

;dale got rid of bra to bra
	bvs	_CXOVF4
;	bvc.s	div51		; - no overflow

;	bra	_CXOVF4		; Divide overflow

div51:
	swap	d0		; Shift X left 5
	lsl.l	#5,d0

	swap	d1		; Shift Y left 8
	lsl.l	#8,d1


;*****
;*****	Divide
;*****

show	macro
;	move.l	\1,-(sp)
;	addq.l	#4,sp
	endm

;**	First divide step.
;**
;**	q0 = x01 / y0
;**	r01 = (x01 % y0) * m1 - q0 * y1
;**	if (r01 < 0) {
;**		q0 -= 1
;**		r01 += y01
;**		}

div60:
	show	d0	; d0 = dividend
	show	d1	; d1 = divisor

	swap	d1		; q0 = x01 % y0
	divu	d1,d0
	move.w	d0,d2

	clr.w	d0		; r01 = x01 % y0 - q0 * y1
	swap	d1
	move.w	d1,d7
	mulu	d2,d7
	sub.l	d7,d0

	bcc.s	div70		; - result >= 0

	subq.w	#1,d2		; q0 -= 1

	add.l	d1,d0		; r01 += y01

;**	Second divide step.
;**
;**	q1 = r01 / y0
;**	if (overflow on divide) q1 = m - 1

div70:
	swap	d2
	show	d0		; dividend
	show	d1		; divisor

	swap	d1		; q1 = r01 / y0
	move.l	d0,d7
	divu	d1,d0
	bvc.s	div71		; - no overflow

	move.w	#-1,d2		; q1 = m-1
	bra.s	div73
div71:
;			remainder/quotient in d0
	move.w	d0,d2	;construct next part of quotient
	clr.w	d0
	swap	d1
	move.w	d1,d7
	mulu	d2,d7
	sub.l	d7,d0
	bcc.s	div73
	subq.l	#1,d2

div73:
	move.l	d2,d0
	show	d0		; result

;	swap	d0		; get final result
;	move.w	d2,d0
;	swap	d0

;	show	d0

	bra	_CXADJ4

	ENDC

********************************************************************************
*
* If this is a 68020 build, we make these into the default versions...
*
	IFD	CPU_68020
		xref	MIIEEESPDiv020
		xdef	MIIEEESPDiv
MIIEEESPDiv:	equ	MIIEEESPDiv020
	ENDC

	ENDC

	end
