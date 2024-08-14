
******* mathieeesingtrans.library/IEEESPTan ********************************
*
*   NAME
*	IEEESPTan -- compute the tangent of a floating point number 
*
*   SYNOPSIS
*	  x   = IEEESPTan(  y  );
*	d0		  d0
*
*	float	x,y;
*
*   FUNCTION
*	Compute tangent of y in IEEE single precision
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
*	IEEESPAtan(), IEEESPSin(), IEEESPCos()
**************************************************************************/
; Down coded tan function
; Copyright 1989 Commodore-Amiga, Inc.
; original down code by Dale Luck

	xref	_MathIeeeSingBasBase
	xref	_LVOSPTst
	xref	_LVOSPAdd
	xref	_LVOSPMul
	xref	_LVOSPDiv
	xref	_LVOSPSub
	xref	_LVOSPFix
	xref	_LVOSPFlt

	xdef	MIIEEESPTan

show	macro
;	move.l	\1,-(sp)
;	addq.l	#4,sp
	endm

MIIEEESPTan:
;	d4 is temp,x_squared
;	d3 is ipart
;	d2 is x
;	tst.w	$222222
	movem.l	d2/d3/d4/a6,-(sp)	; need some spare registers
	move.l	_MathIeeeSingBasBase,a6	; used alot


;****************************************************************
;    if (x < 0.0){
;	x = -x;
;	neg = 1;
;    }
;---------------------------------------------------------
	move.l	d0,d2
	jsr	_LVOSPTst(a6)
	if <
		bchg	#31,d2	; make negative
		move.w	#-1,-(sp)	; input was negative
	else
		clr.w	-(sp)		; input was nonnegative
	endif
;****************************************************************
;	x *= 1.2732395447351626861;

	move.l	d2,d0
	move.l	#1067645315,d1
	jsr	_LVOSPMul(a6)
	move.l	d0,d2
;****************************************************************
;	temp = x
	move.l	d2,d4
;****************************************************************
;	ipart = EXTRACTEXPONENT(temp);
	swap	d0
	and.w	#$7f80,d0	; get exponent
	asr.w	#7,d0
	sub.w	#126,d0
	move.w	d0,d3
	show	d3
	show	d2
;****************************************************************
;   if (ipart >= 61) {
;	x = 0.0;
;	ipart = 0;
;   }
	if #61<=d3.w
		clr.l	d2	; x
		clr.w	d3	; ipart
	else
;****** else { **************************************************
;	if (ipart >= 31) {
;	    AUGMENTEXPONENT(temp, -30);
;	    temp -= (ipart = temp);
;	    AUGMENTEXPONENT(temp, 30);
;	}
;	x = temp;
;	x -= (ipart = x);
;    }
		if #31<=d3.w
			sub.l	#30<<23,d4
			move.l	d4,d0
			jsr	_LVOSPFix(a6)
			move.l	d0,d3
			jsr	_LVOSPFlt(a6)
			move.l	d0,d1
			move.l	d4,d0
			jsr	_LVOSPSub(a6)
			move.l	d0,d4
			add.l	#30<<23,d4
		endif
		move.l	d4,d2
		move.l	d2,d0
		jsr	_LVOSPFix(a6)
		move.l	d0,d3
		jsr	_LVOSPFlt(a6)
		move.l	d0,d1
		move.l	d2,d0
		jsr	_LVOSPSub(a6)
		move.l	d0,d2
	endif
	show d2
;****************************************************************
;	ipart &= 3;
	and.w	#3,d3
;****************************************************************
;	if (ipart & 1)
;		x = 1.0 - x;
	show	d2
	btst	#0,d3
	if <>
		move.l	#1065353216,d0
		move.l	d2,d1
		jsr	_LVOSPSub(a6)
		move.l	d0,d2
	endif
;****************************************************************
;	if (ipart & 2)
;		neg = !neg;
	btst	#1,d3
	if <>
		not.w	(sp)
	endif
;****************************************************************
;	x_squared = x*x;
	move.l	d2,d0
	move.l	d0,d1
	show	d1
	jsr	_LVOSPMul(a6)
	move.l	d0,d4
	show	d4
;****************************************************************
	move.l	#940444609,d0	;	f0 = 0.3386638642677172096076369e-4;
	move.l	d4,d1
	jsr	_LVOSPMul(a6)	;	f0 *= x_squared;
	move.l	#1024208917,d1
	jsr	_LVOSPAdd(a6)	;	f0 += 0.3422554387241003435328470489e-1;
	move.l	d4,d1
	jsr	_LVOSPMul(a6)	;	f0 *= x_squared;
	move.l	#-1049093098,d1
	jsr	_LVOSPAdd(a6)	;	f0 += -0.1550685653483266376941705728e+2;
	move.l	d4,d1
	jsr	_LVOSPMul(a6)	;	f0 *= x_squared;
	move.l	#1149501202,d1
	jsr	_LVOSPAdd(a6)	;	f0 += 0.1055970901714953193602353981e+4;
	move.l	d4,d1
	jsr	_LVOSPMul(a6)	;	f0 *= x_squared;
	move.l	#-968085296,d1
	jsr	_LVOSPAdd(a6)	;	f0 += -0.1306820264754825668269611177e+5;
	show	d0

	move.l	d2,d1
	jsr	_LVOSPMul(a6)	;	x *= f0;
	move.l	d0,d2

	move.l	d4,d0		;	f0 = x_squared;
	move.l	#-1021607719,d1
	jsr	_LVOSPAdd(a6)	;	f0 += -0.1555033164031709966900124574e+3;
	move.l	d4,d1
	jsr	_LVOSPMul(a6)	;	f0 *= x_squared;
	move.l	#1167388163,d1
	jsr	_LVOSPAdd(a6)	;	f0 += 0.4765751362916483698926655581e+4;
	move.l	d4,d1
	jsr	_LVOSPMul(a6)	;	f0 *= x_squared;
	move.l	#-964559384,d1
	jsr	_LVOSPAdd(a6)	;	f0 += -0.1663895238947119001851464661e+5;

	move.l	d0,d1
	move.l	d2,d0
	jsr	_LVOSPDiv(a6)	;	x /= f0;
	show	d0
;	now x in d0.l

;    if (ipart == 1 || ipart == 2)
;	if (x == 0.0) {
;	    x = HUGE;
;	} else {
;	    f0 = x;
;	    x = 1.0;
;	    x /= f0;
;	}

	if d3>#0.w
		if #2>=d3.w
			move.l	d0,d2		; save x here
			jsr	_LVOSPTst(a6)
			if =
				move.l	#2139095040,d0
			else
				move.l	d2,d1
				move.l	#1065353216,d0
				jsr	_LVOSPDiv(a6)
			endif
		endif
	endif
;	now have x in d0.l
	show	d0

	if (sp)+.w		; was it negative?
		bchg	#31,d0
	endif

	movem.l	(sp)+,d2/d3/d4/a6	; restore some spare registers
	rts


	end
