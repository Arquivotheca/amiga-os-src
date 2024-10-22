*******************************************************************************
*
*	$Id: fmul.asm,v 40.1 93/03/11 17:22:23 mks Exp $
*
*******************************************************************************

******* mathieeesingbas.library/IEEESPMul ***********************************
*
*   NAME
*	IEEESPMul -- multiply one double precision IEEE number by another
*
*   SYNOPSIS
*	  x   = IEEESPMul(  y  ,  z  );
*	 d0		   d0    d1
*
*	float	x,y,z;
*
*   FUNCTION
*	Compute x = y * z in IEEE single precision.
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
*	IEEESPDiv
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

STRICTIEEE	equ 1

	include	"float.i"

	ifd	FFP		; FFP Compatibility
	xdef	_SPMul		; Single multiply
	endc

	xref	_CXREN4		; Renormalize
	xref	_CXADJ4		; Adjust after multiply/divide
	xref	_CXOVF4		; Overflow handler
	xref	_CXIND4		; Indefinite handler
	xref	_CXINF4		; Infinite handler
	xref	_CXNAN4		; NAN handler
	xref	_CXZER4		; Zero handler

********************************************************************************
*
* The following is 68000 CPU and up.  It is only built into the 68000 compatible
* version of the library...
*
	IFD	CPU_68000

; exception handling

realxnan:

;**	If X is NAN, return X without further ado.

	move.w	d1,d2		; Y higher precedence?
	or.w	#$0007,d2
	sub.w	d0,d2
	and.w	#$fff8,d2
	bgt.s	mul13		; - why yes

	cmp.l	#$00007f80,d0	; Test upper mantissa
	beq.s	mul12		; - INF
	bra	_CXNAN4		; Process NAN

;**	If Y is NAN, return Y without further ado.

mul12:
	cmp.w	d7,d1	; Y Exponent NAN or INF?
	blt.s	mul14		; - nope

	cmp.l	#$7f80,d1	; Test upper mantissa
	beq.s	mul14		; - INF

mul13:
	move.l	d1,d0		; Process NAN
	bra	_CXNAN4

;**	X is infinite.  If Y is zero, result is indefinite.

mul14:
	tst.l	d1		; Test upper Y
	bne.s	mul15		; - non-zero

	bra	_CXIND4		; Indefinite

;**	Otherwise, result is infinite.

mul15:
	bra	_CXINF4		; Infinite

realynan:
;**	If Y is NAN, result is Y

	cmp.l	#$7f80,d1	; Test Y upper
	beq.s	mul22		; - INF

	move.l	d1,d0
	bra	_CXNAN4

;**	Y is INF.  If X == 0, result is indefinite.

mul22:
	tst.l	d0		; Test X upper
	bne.s	mul23		; - non-zero

	bra	_CXIND4		; Indefinite result

;**	Otherwise, result is infinite.

mul23:
	bra	_CXINF4		; Infinite result

realxzerexp:
	tst.l	d0		; Test upper mantissa
	bne.s	mul31		; - non-zero

	bra	_CXZER4		; Return zero

mul31:
	bsr	_CXREN4		; Denormalize zero exponent
	bra.s	mul40
realyzerexp:
;**	If Y is zero, return zero.

	tst.l	d1		; Test mantissa
	bne.s	mul41		; - non-zero

	bra	_CXZER4		; Return zero

mul41:
	exg	d0,d1		; Switch X,Y for denormalize
	exg	d6,d7

	bsr	_CXREN4		; Denormalize zero exponent

	exg	d0,d1		; Switch back
	exg	d6,d7
	bra.s	mul50


;*************************************************************
;*	Multiply interface routines.                         *
;*************************************************************

;**	float			/* Product */
;**	SPMul(x,y)
;**	float x ;		/* Multiplicand 2 */
;**	float y ;		/* Multiplicand 1 */

;**	Entry:
;**		d0 = single multiplicand 1
;**		d1 = single multiplicand 2
;**	Exit:
;**		d0 = single product
;**	Uses:
;**		a0,a1,d1

	xdef	MIIEEESPMul
MIIEEESPMul
	move.l	d6,a0		; Save d2,d6,d7
	move.l	d7,a1
	move.l	d2,-(a7)

	bsr.s	mul0		; Multiply operands

	move.l	a0,d6		; Restore d2,d6,d7
	move.l	a1,d7
	move.l	(a7)+,d2
	rts

; targets for exception branches that are close
; this is for flow optimization. Make the most common path
; straight thru with no branches.
XNAN:	bra	realxnan
YNAN:	bra	realynan
XZEREXP:	bra	realxzerexp
YZEREXP:	bra	realyzerexp
_myCXOVF4	bra	_CXOVF4	; overflow do it now

;******************************************************
;*	Floating Multiply                             *
;******************************************************

;**	Entry:
;**		d0 = single multiplicand 1
;**		d1 = single multiplicand 2
;**	Exit:
;**		d0 = single product
;**	Uses:
;**		d1,d6,d7

mul0:

;*****
;*****	Unpack sign and form result sign.
;*****

	move.l	d0,d6		; Find sign of result
	eor.l	d1,d6		; save in msb

	move.w	#$7f80,d7	; Exponent mask

	swap	d0		; Remove (X) sign
	and.w	#$7fff,d0

	swap	d1		; Remove (Y) sign
	and.w	#$7fff,d1

;*****
;*****	Process X NAN or INF.
;*****

mul10:
	cmp.w	d7,d0		; Exponent NAN or INF?
;	blt.s	mul20		; - nope
	bge	XNAN

;*****
;*****	Test for Y INF or NAN.
;*****

mul20:
	cmp.w	d7,d1		; Exponent NAN or INF?
;	blt.s	mul30		; - no
	bge	YNAN


;*****
;*****	Process zero X exponent.
;*****

mul30:
	move.w	d0,d6		; Unpack exponent
	and.w	d7,d6
;	bne.s	mul32		; - non-zero
	beq	XZEREXP


mul32:
	eor.w	d6,d0		; Get mantissa
;	eor.w	#$0080,d0	; Add hidden bit
	eor.b	d7,d0		; d7.b is already $80

;*****
;*****	Process zero Y exponent.
;*****

mul40:
	and.w	d1,d7		; Get exponent Y
;	bne.s	mul42		; - non-zero
	beq.s	YZEREXP


mul42:
	eor.w	d7,d1		; Get mantissa
	or.w	#$0080,d1	; Add hidden bit


;*****
;*****	Prepare for multiply.
;*****

mul50:
	sub.w	#$3f80,d6	; Subtract exponent base
	add.w	d7,d6

;	dale optimized here
	bvs.s	_myCXOVF4	; overflow do it now
;	bvc.s	mul51		; - no overflow

;	bra	_CXOVF4		; Multiply overflow

mul51:
	swap	d0		; Left justify X
	lsl.l	#7,d0

	swap	d1		; Left justify Y
	lsl.l	#7,d1



;*****
;*****	Multiply
;*****

;**	d0 = mantissa X
;**	d1 = mantissa Y
;**	d6 = result sign, exponent
;**
;**	Sum:
;**		x0 * y1
;**		x1 * y0
;**	x0 * y0
;**	---------------
;**	z0	z1

show	macro
	move.l	\1,-(sp)
	addq.l	#4,sp
	endm

mul60:
	move.w	d0,d7		; x1 Setup registers for multiply
	move.w	d1,d2		; y1

NEW	equ 1

	ifd NEW

	mulu	d2,d7		; get least bits  x1*y1
	clr.w	d7
	swap	d7		; first partial

	swap	d0		; x1/x0
	mulu	d0,d2		; x0*y1
	add.l	d2,d7

	swap	d1		; y1/y0
	swap	d0		; x0/x1
	move.l	d0,d2
	mulu	d1,d2		; x1*y0
	add.l	d2,d7

	swap	d0		; x1/x0
	mulu	d1,d0		; x0*y0

	clr.w	d7
	swap	d7
	add.l	d7,d0

	bra	_CXADJ4		; Adjust the result
	endc

	ifnd	NEW
	move.w	d0,d7		; x1 Setup registers for multiply
	move.w	d1,d2		; y1

	swap	d0
	swap	d1

	mulu	d0,d2		; z01 = x0 * y1 + y0 * x1
	mulu	d1,d7
	add.l	d2,d7

	ifd	STRICTIEEE
;	Get the last possible bit of precision from the lsb
;	this is needed to pass paranoia round
	swap	d0
	move.l	d0,d2
	swap	d1
	mulu	d1,d2
	swap	d0
	swap	d1
	clr.w	d2
	swap	d2
	add.l	d2,d7
	endc


	clr.w	d7
	swap	d7

	mulu	d1,d0		; z01 += x0 * y0
	add.l	d7,d0

	bra	_CXADJ4		; Adjust the result
	endc

	ENDC

********************************************************************************
*
* If this is a 68020 build, we make these into the default versions...
*
	IFD	CPU_68020
		xref	MIIEEESPMul020
		xdef	MIIEEESPMul
MIIEEESPMul:	equ	MIIEEESPMul020
	ENDC

	ENDC

	end
