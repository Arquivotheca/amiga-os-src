*******************************************************************************
*
*	$Id: cxren4.asm,v 40.1 93/03/11 17:21:48 mks Exp $
*
*******************************************************************************

********************************************************************************
*
* If not FPU-Only, we need this...
*
	IFND	CPU_FPU
*
;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8  bk=0  ma=1

	idnt	CXREN4

	include	"float.i"

	xdef	_CXREN4		; Single renormalize


;*************************************************************
;*	Renormalize denormalized mantissa.                   *
;*************************************************************

;**	Entry:
;**		d0 = Magnitude
;**	Exit:
;**		d0 = Normalized Magnitude
;**		d6 = corresponding negative exponent
;**	Uses:
;**		none

_CXREN4:
	swap	d0
	move.w	#$80,d6		; Initialize exponent

;**	Normalize by shifting successively smaller chunks
;**	of the word.

	cmp.l	#$0100,d0	; 16 bit normalize?
	bge.s	ren1		; - nope

	sub.w	#16*$80,d6	; Decrement exponent
	bra.s	ren2

ren1:
	swap	d0		; Swap for convenience

	tst.w	d0		; 8 bit normalize?
	bne.s	ren2		; - nope

	rol.l	#8,d0		; Shift left
	sub.w	#8*$80,d6	; Adjust exponent

ren2:
	cmp.w	#$0010,d0	; 4 bit normalize?
	bge.s	ren3		; - nope

	rol.l	#4,d0		; Shift left
	sub.w	#4*$80,d6	; Adjust exponent

ren3:
	cmp.w	#$0040,d0	; 2 bit normalize?
	bge.s	ren4		; - nope

	rol.l	#2,d0		; Shift left
	sub.w	#2*$80,d6

ren4:
	tst.b	d0		; 1 bit normalize?
	bmi.s	ren5		; - nope

	rol.l	#1,d0		; Shift left
	sub.w	#$80,d6

ren5:
	rts

	ENDC

	end
