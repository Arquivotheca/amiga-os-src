;  3-26-90 newer cordic algorithm from Al Aburto

******* mathieeesingtrans.library/IEEESPSqrt ********************************
*
*   NAME
*	IEEESPSqrt -- compute the square root of a number
*
*   SYNOPSIS
*	  x   = IEEESPSqrt(  y  );
*	 d0 	             d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute square root of y in IEEE single precision
*
*   INPUTS
*	y - IEEE single precision floating point value
*
*   RESULT
*	x - IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
**************************************************************************/

********************* SWFP_SPSqrt ***************************************
*************************************************************************
*** Program:  SWFP_SPSqrt.asm
*** Function: Computes in Software the IEEESP Sqrt(x), where 'x'
***           is an IEEE Format Single Precision Floating-Point number.
***
*** INPUT:    D0, an IEEESP floating-point number.
***
*** OUTPUT:   D0 = Sqrt(D0), an IEEESP floating-point number.
***
*** Date:     23 Nov 1988 --- Initial Version.
***           10 Oct 1989 --- Revised program so it exits quicker on
***                           x = +0.0 .
***           12 Oct 1989 --- Revised program so that it returns NAN
***                           ($7F880000) when x < 0.0  (even -0.0).
***           30 Mar 1990 --- Revised program to handle case where
***                           SQRT( 2 ** e * (1.0 + epsilon) )   =
***                           2 ** (e/2) * SQRT( 1.0 + epsilon ) =
***                           2 ** (e/2) to first order where epsilon
***                           is the least significant bit of mantissa.
***           08 Apr 1990 --- Revised program to not round up for numbers in
***                           the range of $XX7FFE00 through $XX7FFFFF! I
***                           did this to bring this results in line with
***                           the 68881/68882 results. Now, for example
***                           Sqrt($3F7FFFFF) = $3F7FFFFF instead of the
***                           previous result of $3F800000.  Note: these are
***                           near power of two numbers and we compute them
***                           quickly.
*** Dale Changes:
***		sqrt(-0.0) is 0.0
***		denormalized numbers are handled properly now.
***		   They are normalized and the sqrted.
***		Changed some register usage to speed up abit.
***		Handles NAN and INF properly
***		sped up exponent calculation a bit
***		Many modifications to speed up pieces.
***		dropped usage to 4 data registers, now can save
***		them in address registers while working
***
*** By:       Alfred A. Aburto Jr.
***           3016 Forrester Court
***           San Diego, Calif, 92123
***           (619) 553-1495 (Work)
***           (619) 278-8521 (Home)
***
*** Input:    IEEESP x in D0.
*** Output:   IEEESP sqrt(x) in D0.
***
*** Bugs:
***
*** Following timings are from initial version.
*** Timing (microseconds) for 'JSR SWFP_SPSqrt':
***
***                      68020 14.32 MHz     68000  7.16 MHz    Error
***                     32-bit Mem,ICache       16-bit Mem
*** SPSqrt( x < 0.0)         2.4 usec            10.2 usec       ---
*** SPSqrt( 0.0 )            2.3 usec            10.5 usec       0.0
*** SPSqrt( 1.0 )            7.8 usec            30.6 usec       0.0
*** SPSqrt( 2**N ), N Even  10.6 usec            47.6 usec       0.0
*** SPSqrt( 2**N ), N Odd   13.0 usec            52.6 usec       0.0
***
*** Special Near Integer Cases Like:
*** SPSqrt( 3.9999 )        17.0 usec            64.6 usec
***
*** All Other Non-Special Cases:
*** SPSqrt( 3.9998 )        35.4 usec           135.6 usec
*** SPSqrt( pi )            35.8 usec           151.0 usec
*** SPSqrt( e )             35.4 usec           152.6 usec
*** SPSqrt( 5.0 )           35.6 usec           154.2 usec
***
*************************************************************************
***
NAN:	move.l	d1,d0	; sqrt(nan)=nan sqrt(inf)=inf
	move.l	a0,d4
	move.l	a1,d5
	rts
nonnorm:
;	enter d0 = 0 (0 exponent)
;	enter d1 = mantissa
	moveq	#1,d5
	move.l	d5,d0
	repeat
		sub.w	d5,d0
		add.l	d1,d1
		btst	#23,d1
	until <>
	lsl.l	#8,d1
	bra normal
Sqrt_Neg:
	add.l	d0,d0	; check for -0.0
	beq.s	Sqrt_Zero
	MOVE.L   #$7F880000,D0	; create NAN
exit:
Sqrt_Zero:  RTS

peek	macro
;	move.l	\1,-(sp)
;	addq.l	#4,sp
	endm


	xdef	_rsqrt
_rsqrt:	move.l	4(sp),d0	; C interface
*				; fall into the real routine

*************************************************************************
	xdef	MIIEEESPSqrt
MIIEEESPSqrt:
SWFP_SPSqrt:
            MOVE.L   D0,D1
            BEQ.S    Sqrt_Zero            ;Yes, Just Exit then.
	bmi.s	Sqrt_Neg

	move.l	d4,a0		; save registers here
	move.l	d5,a1

*			play with exponent first
	move.l	#$7f800000,d5
	and.l	d5,d0
	beq.s	nonnorm
	cmp.l	d5,d0
	beq.s	NAN

*			play with mantissa now
	not.l	d5	; d5 = $807FFFFF
	and.l	d5,d1	; we already know that d1 < 0
	moveq    #1,d5		(pwr2 + e)
	cmp.l    d5,d1
        BLE      Sqrt_PTwo            ;Yes.


            LSL.L    #$08,D1              ;Mantissa in D1.
            BSET.L   #$001F,D1            ;Set the upper bit.

            SWAP     D0
            ROR.W    #$07,D0
normal:
	moveq	#$7e,d5
	sub.w	d5,d0			; unbias exponent
	asr.w	#1,d0			; /2 and test for odd
	bcc.s	waseven
	addq.w	#1,d0			; adjust for odd exponent
	lsr.l	#1,d1			; and mantissa too

waseven:
	add.w	d5,d0			; rebias
            CMPI.L   #$FFFE0000,D1        ;Check for near power of two,
                                          ;where all Mantissa bits are set,
                                          ;except those indicated.  Note:
                                          ;first 9-bits of D1 are not part
                                          ;of the Original Mantissa.
            BCS.S      S902
                                          ;Near power of two case ...

            SUBI.L   #$00000100,D1        ;Reduce least significant bit of
                                          ;Mantissa to bring our results
                                          ;in line with the 68881/68882.
; Dale - original code had these here, they look like they don't do
;        anything significant when you look at what S904 does
;            MOVE.L   D1,D3                ;Special case, near integer number.
;            LSL.L    #$01,D3              ;set the x-bit and
            BRA.S      S904               ;skip to end of program.

S902:       		              ;Note: f = fu : fl
	peek d1

	move.l	d1,d4
	swap	d4
	mulu.w	#$9715,d4		; B = INT( 0.59016 * 65536 ) (B * fu)
	swap	d4			; setup for A + B * fu.
	addi.w	#$6ad5,d4		; A = INT( 0.41713 * 65536) =$6ad5

                                          ;Only about 3 bits accuracy here.
            BCC.S    S903
            MOVEQ.L   #-1,D4            ;Set all bits in this case.

S903:       MOVE.L   D1,D5                ;f
            DIVU.W   D4,D5                ;f / y0
	peek d4
	peek d5
            ADD.W    D5,D4                ;y0 + f / y0
	peek d4
            ROXR.W   #$01,D4              ;y1 = ( y0 + f / y0 ) / 2
	peek d4
                                          ;Minimum of 7-bits accuracy now.

            MOVE.L   D1,D5                ;f
            DIVU.W   D4,D5                ;f / y1
	peek d5
            ADD.W    D5,D4                ;y1 + f / y1
	peek d4
            ROXR.W   #$01,D4              ;y2 = ( y1 + f / y1 ) / 2
	peek d4
                                          ;14 to 15-bits accuracy now. I
                                          ;took no special steps to preserve
                                          ;15-bits in all cases.

            MOVE.L   D1,D5                ;f
            DIVU.W   D4,D5                ;f / y2
	peek d5
            MOVE.W   D5,D1                ;f / y2.
            CLR.W    D5                   ;Clear old quotient so we can
                                          ;do another divide by y2 on the
                                          ;Remainder.

            DIVU.W   D4,D5                ;Remainder / y2. Lower word of
	peek d5
                                          ;sqrt(f).

            ADD.W    D4,D1                ;y2 + f / y2. sqrt(f) Upper word.
	peek d1
            SWAP     D1
            MOVE.W   D5,D1                ;sqrt(f). 29 to 31-bits accuracy
	peek d1
                                          ;but we only need 23-bits. 

S904:	move.b	d0,d1	; bring everything together
	ror.l	#8,d1	; get bit close, get ready to round
	addq.l	#1,d1	; round off, overflow into exponent
	lsr.l	#1,d1	; shove everything right and clear sign bit
	move.l	d1,d0	; done

	move.l	a0,d4
	move.l	a1,d5

            RTS


Sqrt_PTwo:
;	checking for sqrt one probably will slow down the other
;	powers of two too much I feel so let's compute it.
;	It does not take very long anyway

            SWAP     D0
            LSR.L    #$07,D0              ;e = Biased Exponent.

	moveq	#$7f,d5
	sub.w	d5,d0
	asr.w	#1,d0
	bcs	exp_odd
;		is even, simple just return with exp/2
		add.w	d5,d0
		lsl.w	#7,d0
		swap	d0
		move.l	a0,d4
		move.l	a1,d5
		rts

exp_odd:
;		almost as easy return with exp/2 and sqrt(2) in mantissa
	add.w	d5,d0
	lsl.w	#7,d0
	swap	d0
	or.l	#$003504F3,d0
	move.l	a0,d4
	move.l	a1,d5
	rts


***************** End of SWFP_SPSqrt.asm Code ***************************

	end
