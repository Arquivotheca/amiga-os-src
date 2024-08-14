*******************************************************************************
*
*	$Id: ffloorceil.asm,v 37.1 91/02/07 16:28:10 mks Exp $
*
*******************************************************************************
        SECTION         mathieeesingbas

	xdef	MIIEEESPFloor,MIIEEESPCeil

*
* general equates
*
sign	equ	$80000000	sign bit mask
signb	equ	31		sign bit no.
notsign	equ	$7fffffff	everything but sign
zero	equ	$00000000	IEEE single precision 0.0
one	equ	$3F800000	IEEE single precision 1.0 (lower half)
neg_one	equ	one+sign	negative one


***************************************************
***************************************************
***						***
***  DOUBLE PRECISION C ENTRY POINTS BEGIN HERE	***
***						***
***************************************************
***************************************************

* new code from Dale
* parameters are in registers

	xref	_LVOIEEEDPAdd
	xref	_LVOIEEEDPFloor

******* mathieeesingbas.library/IEEESPCeil ***********************************
*
*   NAME
*	IEEESPCeil -- compute Ceil function of IEEE single precision number
*
*   SYNOPSIS
*	  x   = IEEESPCeil(  y  )
*	 d0		     d0
*
*	  float IEEESPCeil(float);
*
*   FUNCTION
*	Calculate the least integer greater than or equal to x and return it.
*	This identity is true.  Ceil(x) = -Floor(-x).
*
*   INPUTS
*	y -- IEEE single precision floating point value
*
*   RESULT
*	x -- IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPFloor()
******************************************************************************

MIIEEESPCeil:
	bchg	#31,d0
	jsr	_LVOIEEEDPFloor(a6)
	tst.l	d0	; don't turn 0.0 into -0.0
	if <>
		bchg	#31,d0
	endif
QUICKZ:	rts

nearzero:
	if d1<#0.l
		move.l	#neg_one,d0	; return -1.0
	else
		clr.l	d0	; return 0.0
	endif
	rts

******* mathieeesingbas.library/IEEESPFloor ***********************************
*
*   NAME
*	IEEESPFloor -- compute Floor function of IEEE single precision number
*
*   SYNOPSIS
*	  x   = IEEESPFloor(  y  )
*	  d0		      d0
*
*	  float IEEESPFloor(float);
*
*   FUNCTION
*	Calculate the largest integer less than or equal to x and return it.
*
*   INPUTS
*	y -- IEEE single precision floating point value
*
*   RESULT
*	x -- IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPCeil()
******************************************************************************

MIIEEESPFloor:
	move.l	d0,d1		; is it a zero? and save in d1 for later
	beq	QUICKZ		; yup, just return
* check exponent, are there any fraction bits?
	swap	d0
	and.w	#$7f80,d0	; extract exponent
	beq	nearzero	; unnormalized?
	asr.w	#7,d0		; align to work with it
	sub.w	#127,d0		; unbias it
	blt	nearzero	; range is -1.0 < x < 1.0
* d1 is positive or 0, it tells about how many bits are to the left
* of binary point, so 0 means 1 bit left of binary point
* We have 24 total bits
* If d1 >= 24 then there are no fraction bits

	if #23>d0.w	; any fraction bits to eliminate?
*		d1 is 0 <= d1 < 23
*       	0 means clean out all 23 bits, leaving the hidden bit
*       	1 means clean out 22 bits
* 		there are some fraction bits, we eliminate these now
* 		if there were no fraction bits then we can just return with
* 		the original value
		neg.w	d0
		add.w	#23,d0	; how many bits to clear
				; starting from right end
*		need to save current values for later comparison
		move.l	d1,a1
		lsl.w	#2,d0
		and.l	table(PC,d0.w),d1
		move.l	d1,d0
*		have to check sign bit to round down
		if <	; was it negative?
*			did we really change anything?
			if d1=a1.l
				rts
;				bra.s	no_change
			endif
*			something changed, so we have to decrement it by -1
			move.l	#neg_one,d1
*			now add them, rounding to -infinity
			jmp	_LVOIEEEDPAdd(a6)
;			jsr	_LVOIEEEDPAdd(a6)
;			rts		; stack already ok
		endif
	else
no_change:
		move.l	d1,d0		; restore original input
	endif
	rts

table:				;used to clear fraction bits
	dc.l	$ffffffff
	dc.l	$fffffffe
	dc.l	$fffffffc
	dc.l	$fffffff8
	dc.l	$fffffff0
	dc.l	$ffffffe0
	dc.l	$ffffffc0
	dc.l	$ffffff80
	dc.l	$ffffff00
	dc.l	$fffffe00

	dc.l	$fffffc00
	dc.l	$fffff800
	dc.l	$fffff000
	dc.l	$ffffe000
	dc.l	$ffffc000
	dc.l	$ffff8000
	dc.l	$ffff0000
	dc.l	$fffe0000
	dc.l	$fffc0000
	dc.l	$fff80000

	dc.l	$fff00000
	dc.l	$ffe00000
	dc.l	$ffc00000
	dc.l	$ff800000
*	don't need any more bits
      end

