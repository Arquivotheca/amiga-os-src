
*******************************************************************************
*
*	$Header:
*
*******************************************************************************


;	:ts=8 bk=0 ma=1

	include	"float.i"

	xref	_cxnrm5		; Normalize
	xref	_cxunf5		; Underflow handler
	xref	_cxovf5		; Overflow handler
	xref	_cxnan5		; NAN handler
	xref	_cxind5		; Indefinite handler
	xref	_cxinf5		; Infinite handler
	xref	_cxzer5		; Zero handler


;*************************************************************
;*	Addition interface routines.                         *
;*************************************************************

;**	Entry:
;**		d0,d1 = operand 1
;**		d2,d3 = operand 2
;**	Exit:
;**		d0,d1 = result
;**	Uses:
;**		a1


;*************************************************************
;*	Subtraction interface routines.                      *
;*************************************************************

;**	Entry:
;**		d0,d1 = operand 1
;**		d2,d3 = operand 2
;**	Exit:
;**		d0,d1 = result
;**	Uses:
;**		a1

******* mathieeedoubbas.library/IEEEDPSub ***********************************
*
*   NAME
*	IEEEDPSub -- subtract one double precision IEEE number from another
*
*   SYNOPSIS
*	  x   = IEEEDPSub(  y  ,  z  );
*	d0/d1		  d0/d1 d2/d3
*
*	double	x,y,z;
*
*   FUNCTION
*	Compute x = y - z in IEEE double precision.
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
*	IEEEDPAdd
******************************************************************************

	xdef	MIIEEEDPSub
MIIEEEDPSub:
	bchg	#31,d2
	bsr.s	add0		; Subtract operands
	bchg	#31,d2
	rts


******* mathieeedoubbas.library/IEEEDPAdd ***********************************
*
*   NAME
*	IEEEDPAdd -- add one double precision IEEE number to another
*
*   SYNOPSIS
*	  x   = IEEEDPAdd(  y  ,  z  );
*	d0/d1		  d0/d1 d2/d3
*
*	double	x,y,z;
*
*   FUNCTION
*	Compute x = y + z in IEEE double precision.
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
*	IEEEDPSub
******************************************************************************

;*********************************************************
;*	Floating Addition                                *
;*********************************************************

;**	Entry:
;**		d0,d1 = Floating Operand 1
;**		d2,d3 = Floating Operand 2
;**
;**	Exit:
;**		d0,d1 = Floating Sum
;**
;**	Uses:
;**		d4,d5,d6,d7,a1

	xdef MIIEEEDPAdd
MIIEEEDPAdd:
add0:
	movem.l	d2-d7,-(a7)	; Save d2-d7

	move.w	#16,a0		; preload common constants
	move.w	#$7ff0,a1

;**	Unpack X.
	swap	d0		; Swap for unpack
	swap	d2

	move.w	#$8000,d6	; Setup mask
	move.w	a1,d7

	move.w	d0,d4		; Get sign(X)
	and.w	d6,d4
	eor.w	d4,d0
	move.w	d0,d5		; Get exponent(X)
	and.w	d7,d5
	eor.w	d5,d0		; Get mantissa(X)

;**	Unpack sign of Y and check for signed zero.

add2:
	and.w	d2,d6		; Get sign(Y)
	eor.w	d6,d2
	and.w	d2,d7		; Get exponent(Y)
	eor.w	d7,d2		; Get mantissa(Y)

	cmp.w	d7,d5		; compare exponents
	bne	add40		; - not equal


;****
;****	Handle Equal exponents
;****

;**	d0,d1 = Mantissa X
;**	d1,d2 = Mantissa Y
;**	d4 = sign X
;**	d5 = common exponent
;**	d6 = sign Y
;**	d7 = available
;**
;**	d0,d2 are word swapped.

add10:

;**	Check for both exponents INF or NAN.

	cmp.w	a1,d5	; Check for wierd exponent
	bne.s	add14		; - Regular number

;**	If X is NAN, and precedence(X) >= precedence(Y), return X.
;**	Otherwise if Y is NAN, return Y.

	cmp.w	d2,d0		; Test NAN precedence
	blt.s	add11		; - precedence(X) < precedence(Y)

	move.l	d0,d7		; Mantissa zero?
	or.l	d1,d7
	bne.s	add12		; - NAN

	move.l	d2,d7		; Mantissa(Y) zero?
	or.l	d3,d7
	beq.s	add13		; - INF

add11:
	move.l	d2,d0		; Set to return Y
	move.l	d3,d1

add12:
	bra	_cxnan5

;**	Both X and Y are infinity.   If they have the same
;**	sign, return infinity.   Otherwise generate indefinite.

add13:
	cmp.w	d4,d6		; Sign of infinity equal?
	beq.s	add24		; - yes
	bra	_cxind5		; - no
	
;**	Check for denormalized mantissa

add14:
	tst.w	d5		; Zero exponent?
	bne.s	add18		; - no

;**	If X is true zero, check Y.  If Y is also zero, logically
;**	and their signs together and return signed zero.  If Y
;**	is nonzero, return Y.

	swap	d0		; Test X mantissa
	bne.s	add16		; - nonzero
	tst.l	d1
	bne.s	add16		; - nonzero

	swap	d2		; Test Y mantissa
	bne.s	add15		; - nonzero
	tst.l	d3
	bne.s	add15		; - nonzero

	and.w	d6,d4		; Form result sign
	bra	_cxzer5		; return signed zero

add15:
	move.l	d2,d0		; Restore & return Y
	move.l	d3,d1
	swap	d0
	eor.w	d6,d0
	eor.w	d7,d0
	swap	d0
	movem.l	(a7)+,d2-d7	; Restore d2-d7
	rts

;**	If Y is zero, return X.

add16:
	swap	d2		; Test Y mantissa
	bne.s	add19		; - nonzero
	tst.l	d3
	bne.s	add19		; - nonzero

	swap	d0		; Restore X
	eor.w	d4,d0
	eor.w	d5,d0
	swap	d0
	movem.l	(a7)+,d2-d7	; Restore d2-d7
	rts

;**	Operands are normalized.  Add hidden bit.

add18:			; put in hidden bit
	sub.w	a0,d5
	add.w	a0,d0
	add.w	a0,d2

	swap	d0		; Unswap mantissas
	swap	d2

add19:
	cmp.w	d4,d6		; Same sign?
	beq.s	add30		; - add

;*****
;*****	Subtract operands with the same exponent.
;*****

add20:
	sub.l	d3,d1		; Subtract LSB
	bne.s	add21		; - LSB different

	subx.l	d2,d0		; Subtract MSB
	bne.s	add22		; - MSB different

	movem.l	(a7)+,d2-d7	; Restore d2-d7
	rts			; Return 0

add21:
	subx.l	d2,d0		; Subtract MSB
add22:
	bpl.s	add23		; - X >= Y

	neg.l	d1		; Change result sign
	negx.l	d0

	move.w	d6,d4
add23:
	bra	_cxnrm5		; Normalize

add24:
	bra	_cxinf5		; Handle infinite
	

;*****
;*****	Add Operands with same exponent.
;*****

add30:
	add.l	d3,d1		; Add LSB
	addx.l	d2,d0		; Add MSB

	cmp.l	#$00200000,d0	; Need to shift right?
	blt.s	add31		; - no

	lsr.l	#1,d0		; Shift right
	roxr.l	#1,d1

;	commented out by Dale
;	moveq	#0,d7		; Round
;	addx.l	d7,d1
;	addx.l	d7,d0

	add.w	a0,d5	; Compensate exponent a0==16

	cmp.w	#$7fe0,d5	; Test overflow

add31:
	bcs.s	add32		; - return value
	bra	_cxovf5		; Process overflow

add32:
	swap	d0		; Add in exponent
	add.w	d5,d0
	or.w	d4,d0
	swap	d0
	movem.l	(a7)+,d2-d7	; Restore d2-d7
	rts


;****
;****	Handle unequal exponents
;****

;**	d0,d1 = Mantissa X
;**	d2,d3 = Mantissa Y
;**	d4 = sign X
;**	d5 = exponent X
;**	d6 = sign Y
;**	d7 = exponent Y
;**
;**	d0,d2 are word swapped.

add40:

;**	Swap operands if necessary so X is the larger
;**	operand.

	bgt.s	add41		; - X larger

	exg	d0,d2
	exg	d1,d3
	exg	d4,d6
	exg	d5,d7

;**	Since we now know X > Y, test X only for INF and NAN,
;**	and test Y only for denormalized number and zero.

add41:
	cmp.w	a1,d5	; Wierd exponent?
	beq.s	add43		; - yes

	tst.w	d7		; Smaller value denormalized?
	bne.s	add45		; - no

	swap	d2		; Smaller value zero?
	bne.s	add42		; - no
	tst.l	d3
	bne.s	add42		; - no

;**	Y is zero.  Put X back together and return it.

	eor.w	d4,d0		; Put in sign
	eor.w	d5,d0		; Put in exponent
	swap	d0

	movem.l	(a7)+,d2-d7	; Restore d2-d7
	rts			; Return larger number

;**	Y is denormalized.  Shift it left, and check if X
;**	is also denormalized.  If so, give X the same treatment.

add42:
	add.l	d3,d3		; Shift left
	addx.l	d2,d2

	tst.w	d5		; Larger operand normalized?
	bne.s	add46		; - yes

	swap	d0		; Unswap mantissa
	add.l	d1,d1		; Shift left
 	addx.l	d0,d0
 	bra.s	add50

add43:
	tst.l	d0		; NAN?
	bne.s	add44		; - yes
	tst.l	d1
	bne.s	add44		; - yes

	bra	_cxinf5		; Handle INF

add44:
	bra	_cxnan5		; Handle NAN
	
;**	Put hidden bits in normalized Y and/or X.

add45:
	add.w	a0,d2	; Add hidden bit to smaller
	swap	d2		; Unswap mantissa

add46:
	add.w	a0,d0	; Add hidden bit to larger
	swap	d0		; Unswap mantissa



;****
;****	Operand Denormalization.
;****

;**	d0,d1 = larger mantissa
;**	d2,d3 = smaller mantissa
;**	d4 = larger operand sign
;**	d5 = larger operand exponent
;**	d6 = smaller operand sign
;**	d7 = smaller operand exponent

;**	Figure out the true right shift count for the
;**	normalized operand.

add50:
	sub.w	d5,d7		; Difference between exponents
	not.w	d7
	asr.w	#4,d7

	sub.w	#$0020,d5	; Adjust exponent

;**	Add guard bit to larger mantissa.

	add.l	d1,d1		; Shift LSB, MSB
	addx.l	d0,d0

; check for small shifts first
	cmp.w	a0,d7		; Shift 16 first?
	ble.s	add54		; - nope

;**	Select the proper case for a fast denormalize.

	cmp.w	#32,d7		; Shift 0-32 ?
	ble.s	add53		; - yes

;**	Check if smaller operand significant.
	if #52<d7.w
		move.w	#52,d7
	endif
;**	Shift count 32-52

add51:
	move.l	d2,d3		; Shift right 32
	moveq	#0,d2

	sub.w	#32,d7		; Decrement shift count

	lsr.l	d7,d3		; Shift right 1-20
	moveq	#0,d2
	cmp.w	d4,d6		; Signs equal?
	bne.s	add70		; - subtract operands
	bra.s	add60

;**	Shift 16-31 bits

add53:
	cmp.w	a0,d7		; Shift 16 first?
	ble.s	add54		; - nope

	move.w	d2,d3		; Shift right 16
	swap	d3
	clr.w	d2
	swap	d2

	sub.w	a0,d7		; Decrement shift count

add54:
	cmp.w	d4,d6		; Signs equal?
	bne.s	add69		; - subtract operands

	move.l	d2,d6		; Shift right 0-16 places
	lsr.l	d7,d2
	ror.l	d7,d6
	lsr.l	d7,d3
	eor.l	d2,d6
	eor.l	d6,d3

;*****
;*****	Add operands with different exponents.
;*****

add60:
	add.l	d3,d1		; Do the add
	addx.l	d2,d0

	add.w	a0,d5	; Adjust exponent

	lsr.l	#1,d0		; Shift off guard bit
	roxr.l	#1,d1

;	cmp.l	#$0200000,d0	; Need another shift?
;	blt.s	add61		; - no
	btst	#21,d0
	if <>
		add.w	a0,d5	; Adjust exponent
		lsr.l	#1,d0		; Shift again
		roxr.l	#1,d1
	endif

add61:
;	killing rounding
;	moveq	#0,d7		; Round to nearest
;	addx.l	d7,d1
;	addx.l	d7,d0

	swap	d0		; Add in exponent
	add.w	d5,d0

	cmp.w	a1,d5	; Overflow?
	bcc.s	add62		; - yes

	or.w	d4,d0		; Add in sign
	swap	d0
	movem.l	(a7)+,d2-d7	; Restore d2-d7
	rts

add62:
	bra	_cxovf5		; Handle overflow



;*****
;*****	Subtract operands with different exponents.
;*****
add69:
	move.l	d2,d6		; Shift right 0-16 places
	lsr.l	d7,d2
	ror.l	d7,d6
	lsr.l	d7,d3
	eor.l	d2,d6
	eor.l	d6,d3

add70:
;	subx.l	d7,d7		; Remember X flag

;	subx.l	d3,d1		; Subtract mantissas
	sub.l	d3,d1		; Subtract mantissas
	subx.l	d2,d0

	btst	#21,d0
	beq	_cxnrm5
;	cmp.l	#$00200000,d0	; Need to shift right?
;	blt	add23		; - no

;	kill round dale
;	neg.l	d7		; Round for shift
;	add.l	d7,d1
;	moveq	#0,d7
;	addx.l	d7,d0

	lsr.l	#1,d0		; Shift right
	roxr.l	#1,d1

	add.w	a0,d5	; Adjust exponent

	swap	d0		; Add in exponent
	add.w	d5,d0

	or.w	d4,d0		; Add sign and return
	swap	d0
	movem.l	(a7)+,d2-d7	; Restore d2-d7
	rts

	end
