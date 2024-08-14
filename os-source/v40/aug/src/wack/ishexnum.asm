* Amiga Grand Wack
*
* IsHexNum.asm -- Hash and conversion routines
*
* $Id: ishexnum.asm,v 39.0 92/10/30 15:27:21 peter Exp $
*
* Routines to convert a string to hex number, a string to decimal
* number, Hash a name, or convert to upper case.

	xdef	_IsHexNum,IsHexNum
	xdef	_IsDecNum,IsDecNum
	xdef	_Hash,Hash
	xdef	_UpperCase,UpperCase

************************************************************************
*
*    SYNOPSIS
*	boolean IsHexNum (string, &number)
*	d0		  a0      a1
*
*    NOTES
*	Result will only return true for valid hex number.
*	If string is not a valid hex number, the &number location
*	is not affected.
*
*    BUGS
*	hex numbers longer than 8 digits will convert only last 8 digits
*
************************************************************************

_IsHexNum:
		movem.l	4(sp),a0/a1

IsHexNum:
		moveq	#0,d1
		move.l	d1,d0
		bra.s	ihn_entry

ihn_sumhex:	addq.l	#5,d0
		addq.l	#5,d0
ihn_sum:	lsl.l	#4,d1
		add.l	d0,d1
ihn_entry:
		move.b	(a0)+,d0
		beq.s	ihn_true
		sub.b	#'0',d0
		blt.s	ihn_fail
		cmp.b	#10,d0
		blt.s	ihn_sum
		sub.b	#'A'-'0',d0
		blt.s	ihn_fail
		cmp.b	#6,d0
		blt.s	ihn_sumhex
		sub.b	#'a'-'A',d0
		blt.s	ihn_fail
		cmp.b	#6,d0
		blt.s	ihn_sumhex

ihn_fail:	moveq	#0,d0
		rts
ihn_true:
		moveq	#-1,d0
		move.l	d1,(a1)
		rts

************************************************************************
*
*    SYNOPSIS
*	boolean IsDecNum (string, &number)
*	d0		  a0      a1
*
*    NOTES
*	Result will only return true for valid Decimal number.
*	If string is not a valid decimal number, the &number location
*	is not affected.
*
*    BUGS
*	Decimal numbers which overflow have the excess discarded.
*
************************************************************************

_IsDecNum:
		movem.l	4(sp),a0/a1

IsDecNum:
		move.l	d2,-(sp)
		moveq	#0,d1
		move.l	d1,d0
		bra.s	idn_entry

idn_sum:	move.l	d1,d2	; Number into d2
		add.l	d1,d1	; d1 = 2*num
		add.l	d1,d1	; d1 = 4*num
		add.l	d2,d1	; d1 = 5*num
		add.l	d1,d1	; d1 = 10*num
		add.l	d0,d1

idn_entry:	move.b	(a0)+,d0
		beq.s	idn_true

		sub.b	#'0',d0
		blt.s	idn_fail
		cmp.b	#10,d0
		blt.s	idn_sum

idn_fail:	moveq	#0,d0
		bra.s	idn_exit
idn_true:
		moveq	#-1,d0
		move.l	d1,(a1)

idn_exit:	move.l	(sp)+,d2
		rts

************************************************************************
*
*    SYNOPSIS
*	char UpperCase (char)
*	d0		d0
*
*    NOTES
*	The result will contain the upper cased char parameter.
*
************************************************************************

_UpperCase:
		move.l	4(sp),d0
UpperCase:
		cmp.b	#'a',d0
		blt.s	uc_exit
		cmp.b	#'z',d0
		bgt.s	uc_exit
		sub.b	#'a'-'A',d0
uc_exit:
		rts


************************************************************************
*
*    SYNOPSIS
*	hashNumber Hash (string)
*	d0		 a0
*
*    RESULT
*	hashNumber will range 0..13
*
************************************************************************

_Hash:
		move.l	4(sp),a0

Hash:
		moveq	#0,d0
h_next:
		move.b	(a0)+,d0
		beq.s	h_none
		cmp.b	#'_',d0
		beq.s	h_next
		sub.b	#'A'-1,d0
		ble.s	h_none
		cmp.b	#('Z'-'A')+1,d0
		ble.s	h_exit
		sub.b	#'a'-'A',d0
		ble.s	h_none
		cmp.b	#('z'-'a')+1,d0
		ble.s	h_exit
h_none:
		moveq	#0,d0
h_exit:
		rts

	end
