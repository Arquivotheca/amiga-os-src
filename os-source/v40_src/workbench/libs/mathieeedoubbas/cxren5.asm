
*******************************************************************************
*
*	$Header:
*
*******************************************************************************

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	include 'float.i'

	xdef	_CXREN5		; Renormalize denormalized number

	xref	_CXTAB5		; Normalization table


;*************************************************************
;**	Renormalize denormalized mantissa.                   *
;*************************************************************

;**	Entry:
;**		d0,d1 = Magnitude
;**	Exit:
;**		d0,d1 = Normalized Magnitude
;**		d5 = corresponding negative exponent
;**	Uses:
;**		d4 (upper word only)
;**		d6,a1

_CXREN5:
	swap	d0
	swap	d4
	moveq	#16,d5		; Initialize exponent

;**	Normalize in 16 bit chunks.

	cmp.l	#$20,d0	; Shift 16?
	bge.s	ren10		; - no

ren1:
	swap	d0		; Do 16 bit shift
	swap	d1
	move.w	d1,d0
	clr.w	d1

	sub.w	#16*16,d5	; Decrement exponent

	cmp.l	#$20,d0	; Shift another 16?
	blt.s	ren1		; - yes

;**	Do a series of shifts on the upper longword.
;**	Remember the shift count, so we can go back
;**	and shift the stuff in the lower longword to match.

ren10:
	clr.w	d4		; Initialize shift count

	cmp.l	#$2000,d0	; 8 bit normalize
	bge.s	ren11		; - nope

	lsl.l	#8,d0		; Shift d0
	addq.w	#8,d4		; Adjust shift count

ren11:
	swap	d0		; Swap for convenience

	tst.w	d0		; 4 bit normalize
	bne.s	ren12		; - nope

	rol.l	#4,d0		; Shift left
	addq.w	#4,d4		; Adjust shift count

ren12:
	moveq	#0,d6		; Get shift count
	lea	_CXTAB5,a1
	move.b	0(a1,d0.w),d6

	rol.l	d6,d0		; Shift left
	add.w	d6,d4		; Adjust shift count

;**	Shift the lower longword to match the upper
;**	longword normalize.

	swap	d0		; Shift left by saved count
	move.l	d1,d6
	lsl.l	d4,d1
	rol.l	d4,d6
	eor.w	d1,d6
	eor.w	d6,d0

	lsl.w	#4,d4		; Compute exponent
	sub.w	d4,d5

;**	Restore & return

	swap	d0
	swap	d4
	rts

	end
