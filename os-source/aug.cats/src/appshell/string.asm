
*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* string.asm
*
* Source Control
* ------ -------
* 
* $Header:
*
* $Locker:
*
* $Log:
*
*************************************************************************
	XDEF	_stricmpn
*
* int stricmp (char * __reg a0 s1, char * __reg a1 s2, int __reg d2 len)
* strcmpn ignoring case, 0 = equal !0 = not equal
*
_stricmpn:			; 'C' entry point
	move.l	d2,-(sp)	; save d2
	movem.l	8(sp),a0/a1	; a0=s1, a1=s2
	move.l	16(sp),d2	; d2=len
	bsr.s	stricmpn	; do it
	move.l	(sp)+,d2	; restore d2
	rts

stricmpn:			; 'asm' entry point
	moveq	#0,d0
	subq	#1,d2

caseloop:
	move.b	(a0)+,d0	; is *s1 == 0, and *s2 != 0, will fall out
	move.b	(a1)+,d1	; the bottom.
	beq.s	make_res	; exit if *s2 == 0
	tst.b	d0
	beq.s	make_res	; exit if *s1 == 0

	cmp.b	#'a',d0		; convert d0 to upper case
	blt.s	check2
	cmp.b	#'z',d0
	bgt.s	check2
	sub.b	#'a'-'A',d0
check2:
	cmp.b	#'a',d1		; convert d1 to upper case
	blt.s	cmpit
	cmp.b	#'z',d1
	bgt.s	cmpit
	sub.b	#'a'-'A',d1
cmpit:
	cmp.b	d1,d0		; are chars equivalent?
	bne.s	make_res	; no, so exit.
	dbra	d2,caseloop	; yes, loop if more chars to check

make_res:
	sub.b	d1,d0
	rts

	END
