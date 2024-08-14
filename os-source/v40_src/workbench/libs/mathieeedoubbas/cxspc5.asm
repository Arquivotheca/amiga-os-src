
*******************************************************************************
*
*	$Header:
*
*******************************************************************************

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	include	"float.i"

	ifd	QWE
	xdef	_CXZER5		; Zero handler
	xdef	_CXOVF5		; Overflow handler
	xdef	_CXUNF5		; Underflow handler
	xdef	_CXNAN5		; NAN handler
	xdef	_CXIND5		; Indefinite handler
	xdef	_CXINF5		; Infinite handler
	endc
	xdef	_cxzer5		; Zero handler
	xdef	_cxovf5		; Overflow handler
	xdef	_cxunf5		; Underflow handler
	xdef	_cxnan5		; NAN handler
	xdef	_cxind5		; Indefinite handler
	xdef	_cxinf5		; Infinite handler

	ifd	QWE
	xref	_CXFERR		; Exception handler


;*************************************************
;*	Zero handler                             *
;*************************************************

;**	Entry:
;**		d4 = sign of result
;**	Exit:
;**		d0,d1 = signed zero

_CXZER5:
	moveq	#0,d0
	move.l	d0,d1
	eor.w	d4,d0
	swap	d0
	rts
	endc

_cxzer5:
;	bra into
	moveq	#0,d0
	move.l	d0,d1
	eor.w	d4,d0
	swap	d0
	movem.l	(sp)+,d2-d7
	rts


;*************************************************
;*	Underflow handler                        *
;*************************************************

;**	Entry:
;**		d4 = Sign of result
;**	Exit:
;**		d0,d1 = Signed zero

	ifd	QWE
_CXUNF5:
	fptrap	1		; Underflow trap

	moveq	#0,d0		; So return zero
	moveq	#0,d1

	eor.w	d4,d0		; Add sign
	swap	d0
	rts
	endc
_cxunf5:
	fptrap	1		; Underflow trap

	moveq	#0,d0		; So return zero
	moveq	#0,d1

	eor.w	d4,d0		; Add sign
	swap	d0
	movem.l	(sp)+,d2-d7
	rts


;*************************************************
;*	Overflow handler                         *
;*************************************************

;**	Entry:
;**		d4 = Sign
;**	Exit:
;**		d0,d1 = Signed infinity
;**	Uses:
;**		none
	ifd	QWE

_CXOVF5:
	fptrap	2		; Overflow trap

_CXINF5:
	move.l	#$00007ff0,d0	; Construct FP infinity
	eor.w	d4,d0		; With sign
	moveq	#0,d1
	swap	d0
	rts
	endc

_cxovf5:
	fptrap	2		; Overflow trap

;	for round to zero (trunc) we return largest (+/-) value
	move.l	#$ffff7fef,d0
	moveq	#-1,d1
	eor.w	d4,d0		; With sign
	swap	d0
	movem.l	(sp)+,d2-d7
	rts

_cxinf5:
	move.l	#$00007ff0,d0	; Construct FP infinity
	moveq	#0,d1
	eor.w	d4,d0		; With sign
	swap	d0
	movem.l	(sp)+,d2-d7
	rts


;*************************************************
;*	Indefinite handler                       *
;*************************************************

;**	Entry:
;**		d4 = Sign
;**	Exit:
;**		d0,d1 = NAN operand
;**	Uses:
;**		none

	ifd	QWE
_CXIND5:
	fptrap	4		; Indefinite trap

	move.l	#$7ff10000,d0	; Construct NAN
	moveq	#0,d1
	rts
	endc

_cxind5:
	fptrap	4		; Indefinite trap

	move.l	#$7ff10000,d0	; Construct NAN
	moveq	#0,d1
	movem.l	(sp)+,d2-d7
	rts


;*******************************************************
;*	NAN handler                                    *
;*******************************************************

;**	Entry:
;**		d0,d1 = NAN mantissa
;**		(d0 swapped)
;**	Exit:
;**		d0,d1 = NAN
;**	Uses:
;**		none

	ifd	QWE
_CXNAN5:
	btst	#3,d0		; Check trap bit
	beq.s	nan1		; - not trapping

	fptrap	4		; Indefinite trap

	bclr	#3,d0		; Clear trap bit
	bset	#1,d0		; Set another bit

nan1:
	or.w	#$7ff1,d0	; Add exponent
	swap	d0
	rts
	endc

_cxnan5:
	btst	#3,d0		; Check trap bit
	beq.s	nan2		; - not trapping

	fptrap	4		; Indefinite trap

	bclr	#3,d0		; Clear trap bit
	bset	#1,d0		; Set another bit

nan2:
	or.w	#$7ff1,d0	; Add exponent
	swap	d0
	movem.l	(sp)+,d2-d7
	rts
	end
