*************************************************
*               FFPFLOOR FFPCEIL	            *
*     FAST FLOATING POINT FLOOR/CEILING			*
*                                               *
*  INPUT:   D0 - INPUT ARGUMENT					*
*                                               *
*  OUTPUT:  D0 - FUNCTION RESULT                *
*                                               *
*     ALL OTHER REGISTERS TOTALLY TRANSPARENT   *
*                                               *
*  CODE SIZE: ??? BYTES   STACK WORK: ?? BYTES  *
*                                               *
*  CONDITION CODES:                             *
*        Z - SET IF RESULT IN D0 IS ZERO        *
*        N - SET IF RESULT IN D0 IS NEGATIVE    *
*        C - UNDEFINED                          *
*        V - UNDEFINED							*
*        X - UNDEFINED                          *
*                                               *
*  FUNCTIONS:                                   *
*             FFPFLOOR   -  FLOOR RESULT		*
*Floor returns the largest integer not greater than x.
*             FFPCEIL -  CEIL RESULT		*
*Ceil returns the smallest integer not less than x.
*                                               *
*************************************************

		xref	FFPNEG,FFPSUB	* External functions utilized
		xref	FFPADD,FFPSUB

		xdef	FFPFLOOR,FFPCEIL		* Function entry points
		xdef	MFSPFloor,MFSPCeil

one		equ		$80000041				* FFP 1.0




	xref	FFPNEG
******* mathffp.library/SPCeil ***********************************
*
*   NAME
*	SPCeil -- Compute Ceil function of a number.
*
*   SYNOPSIS
*	x = SPCeil(y)
*	D0         D0
*
*	float SPCeil(float y);
*
*   FUNCTION
*	Calculate the least integer greater than or equal to x and return it.
*	This identity is true.  Ceil(x) = -Floor(-x).
*
*   INPUTS
*	y 	- Motorola Fast Floating Point Format Number.
*
*   RESULT
*	x 	- Motorola Fast Floating Point Format Number.
*
*   BUGS
*	None.
*
*   SEE ALSO
*	SPFloor()
*
******************************************************************************
FFPCEIL:
*   use identity from Sun
*	Ceil(x) = -Floor(-x)
MFSPCeil:
		bsr	FFPNEG
		bsr.s FFPFLOOR
		bra FFPNEG

*	end of routine MFSPCeil

******* mathffp.library/SPFloor ***********************************
*
*   NAME
*	SPFloor -- compute Floor function of a number.
*
*   SYNOPSIS
*	x = SPFloor(y)
*	D0 	    D0
*
*	float SPFloor(float y);
*
*   FUNCTION
*	Calculate the largest integer less than or equal to x and return it.
*
*   INPUTS
*	y 	- Motorola Fast Floating Point number.
*
*   RESULT
*	x 	- Motorola Fast Floating Point number.
*
*   BUGS
*	None.
*
*   SEE ALSO
*	SPCeil()
*
******************************************************************************
FFPFLOOR:
MFSPFloor:
		move.b	d0,d1					* save sign/exponent
		bmi.s	fpimi					* is input negative number?
		beq.s	fpirtn					* return if zero
		clr.b	d0			* clear for shift
		sub.b	#65,d1		* exponent-1 to binary
		bmi.s	fpirt0		* return 0 (only fraction left)
		sub.b	#31,d1		* ?overflow
		bpl.s	fpiovp		* if overflow then there is no fraction
		neg.b	d1			* adjust
		lsr.l	d1,d0		* remove fraction bits
		lsl.l	d1,d0		* put back in float format
		neg.b	d1			* undo exponent stuff
fpiovp:	add.b	#31,d1		* restore exponent
		add.b	#65,d1		* "
		move.b	d1,d0		* insert back in
fpirtn:	rts
fpirt0:
* return 0.0
		moveq	#0,d0
		rts
* handle negative numbers
fpimi:
		bsr	FFPNEG			* make it positive
		move.l	d0,-(sp)	* save original positive value
		bsr	FFPFLOOR		* get floor(-x)
		move.l	d0,-(sp)	* save floor(-x)
		move.l	d0,d1
		move.l	4(sp),d0	* get original -x
*		compute (-x) - Floor(-x)  = fraction
		bsr	FFPSUB			* result should be fraction
		movem.l	(sp),d0		* get back Floor(-x) without killing cc
		beq.s	dont_bump	* if no fraction then do not bump
			move.l	#one,d1
			bsr	FFPADD
dont_bump:
		addq.l	#8,sp
		bra	FFPNEG

		end

