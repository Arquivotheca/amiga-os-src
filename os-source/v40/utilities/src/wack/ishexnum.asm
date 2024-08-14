* $Id: ishexnum.asm,v 1.3 91/04/24 20:53:19 peter Exp $

	xdef	_IsHexNum,IsHexNum
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
*	is not effected.
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

fmt: dc.b ' %lx ',0
  ds.w 0

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
