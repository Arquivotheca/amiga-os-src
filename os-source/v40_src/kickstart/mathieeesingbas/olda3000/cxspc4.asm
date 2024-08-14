;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8  bk=0  ma=1

	include	"float.i"

	ifd	FFP		; FFP Compatibility
	xdef	_SPTieee	; FFP to IEEE single
	xdef	_SPFieee	; FFP from IEEE single
	endc

	xdef	_CXZER4		; Zero handler
	xdef	_CXOVF4		; Overflow handler
	xdef	_CXUNF4		; Underflow handler
	xdef	_CXIND4		; Indefinite handler
	xdef	_CXNAN4		; NAN handler
	xdef	_CXINF4		; Infinite handler

	xref	_CXFERR		; Exception handler

;*****
;*****	Special IEEE to IEEE converters
;*****

	ifd	FFP
_SPTieee:
_SPFieee:
	move.l	4(sp),d0
	rts
	endc


;*****
;*****	Underflow handling
;*****

;**	Entry:
;**		d6 = Sign of result
;**	Exit:
;**		d0 = Signed zero

_CXUNF4:
	fptrap	1		; Underflow trap

;*****
;*****	Signed zero handling
;*****

;**	Entry:
;**		d6 = sign of result
;**	Exit:
;**		d0 = signed zero

	xdef	_cxzer4dale
_CXZER4:

	move.l	d6,d0		; Construct signed zero
_cxzer4dale:
	and.l	#$80000000,d0
	rts


;*****
;*****	Overflow handling
;*****

;**	Entry:
;**		d6 = Sign
;**	Exit:
;**		d0 = Signed infinity
;**	Uses:
;**		none

	xdef	_cxovf4dale
_cxovf4dale:
	fptrap	2
	and.l	#$80000000,d0
	eor.l	#$7f800000,d0
	rts

_CXOVF4:
	fptrap	2		; Overflow

	xdef	_cxinf4dale
_CXINF4:
	move.l	d6,d0		; Construct signed infinity
_cxinf4dale:
	and.l	#$80000000,d0
	eor.l	#$7f800000,d0
	rts

;*****
;*****	Indefinite
;*****

;**	Entry:
;**		none
;**	Exit:
;**		d0 = NAN
;**	Uses:
;**		none

_CXIND4:
	fptrap	4		; Indefinite

	move.l	#$7f880000,d0	; Construct NAN
	rts



;*****
;*****	NAN handling
;*****

;**	Entry:
;**		d0 = NAN mantissa
;**		(d0 swapped)
;**	Exit:
;**		d0 = NAN
;**	Uses:
;**		none

_CXNAN4:
	btst	#6,d0		; Check trap bit
	beq.s	nan1		; - not trapping

	fptrap	4		; Indefinite

	bclr	#6,d0		; Clear trap bit
	bset	#4,d0		; Set another bit

nan1:
	or.w	#$7f88,d0	; Add exponent
	swap	d0
	rts

	end
