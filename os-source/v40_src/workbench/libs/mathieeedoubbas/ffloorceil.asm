
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
        SECTION         mathieeedoubbas

	xdef	MIIEEEDPFloor,MIIEEEDPCeil

*
* general equates
*
sign	equ	$80000000	sign bit mask
signb	equ	31		sign bit no.
notsign	equ	$7fffffff	everything but sign
zero	equ	$00000000	IEEE double precision 0.0 (lower half)
one	equ	$3FF00000	IEEE double precision 1.0 (lower half)
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

******* mathieeedoubbas.library/IEEEDPCeil ***********************************
*
*   NAME
*	IEEEDPCeil -- compute Ceil function of IEEE double precision number
*
*   SYNOPSIS
*	  x   = IEEEDPCeil(  y  );
*	d0/d1		   d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Calculate the least integer greater than or equal to x and return it.
*	This value may have more than 32 bits of significance.
*	This identity is true.  Ceil(x) = -Floor(-x).
*
*   INPUTS
*	y -- IEEE double precision floating point value
*
*   RESULT
*	x -- IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPFloor
******************************************************************************

MIIEEEDPCeil:
	bchg	#31,d0
	jsr	_LVOIEEEDPFloor(a6)
*	is the value = 0.0?
	if d0=#0.l
		tst.l	d1
		beq.s	QUICKZ	; do not make -0.0
	endif
	bchg	#31,d0
QUICKZ:	rts

MQUICKZ:
*	unnormalized numbers will floor to zero
	clr.l	d1
	rts
nearzero:
	if (sp)<#0.w
		move.l	#neg_one,d0	; return -1.0
	else
		clr.l	d0	; return 0.0
	endif
	clr.l	d1
	addq.l	#8,sp
	rts

******* mathieeedoubbas.library/IEEEDPFloor ***********************************
*
*   NAME
*	IEEEDPFloor -- compute Floor function of IEEE double precision number
*
*   SYNOPSIS
*	  x   = IEEEDPFloor(  y  );
*	d0/d1		    d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Calculate the largest integer less than or equal to x and return it.
*	This value may have more than 32 bits of significance.
*
*   INPUTS
*	y -- IEEE double precision floating point value
*
*   RESULT
*	x -- IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPCeil
******************************************************************************

MIIEEEDPFloor:
	tst.l	d0		; is it a zero?
	beq	MQUICKZ		; yup, just return
	move.l	d1,-(sp)	; operate on copy on stack
	move.l	d0,-(sp)
* check exponent, are there any fraction bits?
	swap	d0
	and.w	#$7ff0,d0	; extract exponent
	beq	nearzero	; unnormalized?
	asr.w	#4,d0		; align to work with it
	sub.w	#1023,d0	; unbias it
	blt	nearzero	; range is -1.0 < x < 1.0
* d1 is positive or 0, it tells about how many bits are to the left
* of binary point, so 0 means 1 bit left of binary point
* We have 53 total bits
* If d1 >= 52 then there are no fraction bits

	if #52>d0.w	; any fraction bits to eliminate?
*	d1 is 0 <= d1 < 52
*       0 means clean out all 52 bits, leaving the hidden bit
*       1 means clean out 51 bits
* there are some fraction bits, we eliminate these now
* if there were no fraction bits then we can just return with
* the original value
		neg.w	d0
		add.w	#52,d0	; how many bits to clear
				; starting from right end
*	need to save current values for later comparison
		move.l	(sp),a0
		move.l	4(sp),a1
		if #32<=d0.w
			clr.l	4(sp)	; zap 32 bits right away
			sub.w	#32,d0
			lsl.w	#2,d0	; access a table
			move.l	table(PC,d0.w),d0
			and.l	d0,(sp)
		else	only have to clear last word
			lsl.w	#2,d0	; access a table
			move.l	table(PC,d0.w),d0
			and.l	d0,4(sp)
		endif
*	have to check sign bit to round down
WORKING	equ 1
		if (sp)<#0.w
*			did we really change anything?
			if (sp)=a0.l
				if 4(sp)=a1.l
					bra.s	no_change
				endif
			endif
*	something changed, so we have to decrement it by -1
			move.l	(sp)+,d0
			move.l	(sp)+,d1
			movem.l	d2/d3,-(sp)	; save these away
*			put -1.0 in d2/d3
			move.l	#neg_one,d2
			clr.l	d3
*			now add them, rounding to -infinity
			jsr	_LVOIEEEDPAdd(a6)
			movem.l	(sp)+,d2/d3	; restore temps
			rts		; stack already ok
		endif
no_change:
	endif

	move.l	(sp)+,d0
	move.l	(sp)+,d1
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
	dc.l	$ff000000
	dc.l	$fe000000
	dc.l	$fc000000
	dc.l	$f8000000
	dc.l	$f0000000
	dc.l	$e0000000
	dc.l	$c0000000
	dc.l	$80000000

      end

