	SECTION	MathStuff

	XDEF	SMult32S
	XDEF	UMult32S
	XDEF	SDivMod32S
	XDEF	UDivMod32S

	XDEF	_SMult32H
	XDEF	_UMult32H
	XDEF	_SDivMod32H
	XDEF	_UDivMod32H



******* utility.library/SMult32 **************************************
*
*   NAME
*	SMult32 -- Signed 32 by 32 bit multiply with 32 bit result. (V36)
*
*   SYNOPSIS
*	Result = SMult32( Arg1, Arg2 )
*	D0                D0    D1
*
*	LONG SMult32( LONG, LONG );
*
*   FUNCTION
*	Returns the signed 32 bit result of multiplying Arg1 by Arg2.
*
*   INPUTS
*	Arg1, Arg2 -- Signed multiplicands.
*
*   RESULTS
*	Result -- The signed 32 bit result of multiplying Arg1 by Arg2.
*
*   NOTES
*
*   SEE ALSO
*	SDivMod32(), UDivMod32(), UMult32()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

_SMult32H:
	muls.l	d1,d0
	rts


******* utility.library/UMult32 **************************************
*
*   NAME
*	UMult32 -- Unsigned 32 by 32 bit multiply with 32 bit result. (V36)
*
*   SYNOPSIS
*	Result = UMult32( Arg1, Arg2 )
*	D0                D0    D1
*
*	ULONG UMult32( ULONG, ULONG );
*
*   FUNCTION
*	Returns the unsigned 32 bit result of multiplying Arg1 by Arg2.
*
*   INPUTS
*	Arg1, Arg2 -- Unsigned multiplicands.
*
*   RESULTS
*	Result -- The unsigned 32 bit result of multiplying Arg1 by Arg2.
*
*   NOTES
*
*   SEE ALSO
*	SDivMod32(), SMult32(), UDivMod32()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

_UMult32H:
	mulu.l	d1,d0
	rts


SMult32S:
UMult32S:
	movem.l	D2-D3,-(A7)	; need these
	move.l	D0,D2		; d0=Al
	move.l	D1,D3		; d1=Bl
	swap	D2		; d2=Ah
	swap	D3		; d3=Bh
	mulu.w	D1,D2		; d2=Ah*Bl
	mulu.w	D0,D3		; d3=Al*Bh
	mulu.w	D1,D0		; d0=Al*Bl
	add.w	D3,D2		; ignore overflow
	swap.w	D2		; shift to where it belongs
	clr.w	D2
	add.l	D2,D0		; ignore overflow
	movem.l	(A7)+,D2-D3
	rts


******* utility.library/SDivMod32 ************************************
*
*   NAME
*	SDivMod32 -- Signed 32 by 32 bit division and modulus. (V36)
*
*   SYNOPSIS
*	Quotient:Remainder = SDivMod32( Dividend, Divisor )
*	D0       D1                     D0        D1
*
*	LONG SDivMod32( LONG, LONG );
*
*   FUNCTION
*	Divides the signed 32 bit dividend by the signed 32 bit divisor
*	   and returns a signed 32 bit quotient and remainder.
*
*   INPUTS
*	Dividend -- Signed 32 bit dividend.
*	Divisor -- Signed 32 bit divisor.
*
*   RESULTS
*	Quotient -- Signed 32 quotient of the division.
*	Remainder -- Signed 32 remainder of the division.
*
*   NOTES
*
*   SEE ALSO
*	SMult32(), UDivMod32(), UMult32()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

_SDivMod32H:
	divsl.l	d1,d1:d0
	rts


SDivMod32S:
	tst.l	d0
	bpl.s	NumPos
	neg.l	d0
	tst.l	d1
	bpl.s	DenomPos
	neg.l	d1
	bsr.s	UDivMod32S
	neg.l	d1
	rts
DenomPos
	bsr.s	UDivMod32S
	neg.l	d0
	neg.l	d1
	rts
NumPos
	tst.l	d1
	bpl.s	UDivMod32S
	neg.l	d1
	bsr.s	UDivMod32S
	neg.l	d0
	rts


******* utility.library/UDivMod32 ************************************
*
*   NAME
*	UDivMod32 -- Unsigned 32 by 32 bit division and modulus. (V36)
*
*   SYNOPSIS
*	Quotient:Remainder = UDivMod32( Dividend, Divisor )
*	D0       D1                     D0        D1
*
*	ULONG UDivMod32( ULONG, ULONG );
*
*   FUNCTION
*	Divides the unsigned 32 bit dividend by the unsigned 32 bit divisor
*	   and returns a unsigned 32 bit quotient and remainder.
*
*   INPUTS
*	Dividend -- Unsigned 32 bit dividend.
*	Divisor -- Unsigned 32 bit divisor.
*
*   RESULTS
*	Quotient -- Unsigned 32 quotient of the division.
*	Remainder -- Unsigned 32 remainder of the division.
*
*   NOTES
*
*   SEE ALSO
*	SDivMod32(), SMult32(), UMult32()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

_UDivMod32H:
	divul.l	d1,d1:d0
	rts


UDivMod32S:
	move.l	d3,-(sp)
	cmpi.l	#$0ffff,d1	; can we do this easily?
	bhi.s	BigDenom	; no, we have to work for it
	move.l	d1,d3
	swap.w	d0
	move.w	d0,d3
	beq.s	SmallNum
	divu.w	d1,d3
	move.w	d3,d0
SmallNum
	swap.w	d0
	move.w	d0,d3
	divu.w	d1,d3
	move.w	d3,d0
	swap.w	d3
	move.w	d3,d1
	move.l	(sp)+,d3
	rts

BigDenom:
	move.l	d2,-(sp)
	move.l	d1,d3
	move.l	d0,d1
	clr.w	d1
	swap	d1
	swap	d0
	clr.w	d0
	moveq	#15,d2		; 16 times through the loop
DMls	add.l	d0,d0		
	addx.l	d1,d1
	cmp.l	d1,d3
	bhi.s	DMle
	sub.l	d3,d1
	addq.w	#1,d0
DMle	dbf	d2,DMls
	movem.l	(sp)+,d2-d3
	rts

	end
