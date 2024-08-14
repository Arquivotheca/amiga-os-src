*************************************************************************
*                                                                       *
*   stricmp.asm 
*                                                                       *
*   Copyright (C) 1990 Commodore Amiga Inc.  All rights reserved. 	*
*                                                                       *
*   $Id: $
*************************************************************************

	include	'exec/types.i'
	include	'hooks.i'

	xdef stricmp
	xdef strnicmp

******* utility.library/Stricmp *****************************************
*
*    NAME
*	Stricmp	--  (New for V36) Case-insensitive string compare
*
*    SYNOPSIS
*	res = Stricmp(string1, string2)
*	D0		A0       A1
*
*	LONG Stricmp(char *, char *)
*
*    FUNCTION
*
*	Stricmp compares two strings, ignoring case.  It handles all
*	internationalization issues.  If the strings have different lengths,
*	the shorter is treated as if it were extended with zeros.
*
*    INPUTS
*	string1, string2 - strings to be compared
*
*    RESULT
*	res - negative if string1 is below string2, 0 if they're the same, and
*	      positive if string1 is above string2.
*
*****************************************************************************
*
* int __regargs stricmp (char * s1, char * s2)
* int __regargs strnicmp (char * s1, char * s2, long len)
* strcmp ignoring case, <0 = s1<s2, 0 = equal, >0 = s1>s2
*

stricmp:
	move.l	#$7fffffff,d0		; infinite length
	;-- fall through!

******* utility.library/Strnicmp *****************************************
*
*    NAME
*	Strnicmp--  (New V36) Case-insensitive string compare, length-limited
*
*    SYNOPSIS
*	res = Strnicmp(string1, string2, length)
*	D0		A0       A1	   D0
*
*	LONG Strnicmp(char *, char *, LONG length)
*
*    FUNCTION
*
*	Strnicmp compares two strings, ignoring case.  It handles all
*	internationalization issues.  If the strings have different lengths,
*	the shorter is treated as if it were extended with zeros.  It never
*	compares more than <length> characters.
*
*    INPUTS
*	string1, string2 - strings to be compared
*	length		 - maximum number of characters to examine
*
*    RESULT
*	res - negative if string1 is below string2, 0 if they're the same, and
*	      positive if string1 is above string2.
*
*****************************************************************************

	;-- note: stricmp falls through to here
strnicmp:
	movem.l	d2/d3/d4,-(a7)	; d4 for check_sub use
	move.l	d0,d3		; save length
	beq.s	exit		; if length == 0, return SAME
	moveq	#0,d0		; clear high 3 bytes, set equal if len=0

caseloop:
	subq.l	#1,d3		; done yet?
	bmi.s	make_res	; if done, make result
	move.b	(a0)+,d1	; Deal with either string ending first
	move.b	(a1)+,d2	; must fetch both for comparison, even if d0==0
	beq.s	make_res	; exit if *s2 == 0
	move.b	d1,d0
	beq.s	make_res	; exit if *s1 == 0

	bsr.s	check_sub
	beq.s	1$
	sub.b	#'a'-'A',d1
1$:
	move.b	d2,d0
	bsr.s	check_sub
	beq.s	2$
	sub.b	#'a'-'A',d2

2$:
	cmp.b	d2,d1		; do the comparison (must
	beq.s	caseloop	; check next character (after checking size)

make_res:
	move.b	d1,d0
	sub.b	d2,d0		; generate return value
exit:
	movem.l	(a7)+,d2/d3/d4
	rts

*
* returns d0 = 0 if no sub needed, = 1 if needed (cc's set!!!!)
* can't touch d1/d2/d3, d4 is scratch, d0 has character/return
*
check_sub:
	cmp.b	#$f7,d0		; these are special
	beq.s	no
	move.w	#'za',d4	; Z/z and A/a
	bclr.b	#7,d0		; clear high bit
	beq.s	normal_char
	move.w	#'~`',d4	; Þ/þ and À/à are the highest in the upper area
				; (~ = þ, ' = à minus bit 7)
normal_char:
	cmp.b	d4,d0		; a or à(`)
	blt.s	no
	lsr.l	#8,d4		; get upper bound
	cmp.b	d4,d0		; z or þ(~)
	bgt.s	no
	moveq	#1,d0
	rts
no:	moveq	#0,d0
	rts

