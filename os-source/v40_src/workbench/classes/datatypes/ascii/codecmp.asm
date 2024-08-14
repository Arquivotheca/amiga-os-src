	xdef	_Num1

_Num1:	move.l	4(sp),d0

	cmp.w	#'0',d0
	blt.s	1$	; d0 < '0'

	cmp.w	#'9',d0
	bgt.s	1$	; d0 > '9'

	moveq	#1,d0
	rts

1$	moveq	#0,d0
	rts

*--------------------------------------------------------------------------

	xdef	_Num2

_Num2:	move.l	4(sp),d0

	cmp.w	#'0',d0
	blt.s	3$	; d0 < '0'

	cmp.w	#'9',d0
	bgt.s	1$	; d0 > '9'

2$	moveq	#1,d0
	rts

1$	cmp.w	#';',d0	; d0 = ';'
	beq.s	2$

3$	moveq	#0,d0
	rts

*--------------------------------------------------------------------------

	xdef	_Num3

_Num3:	move.l	4(sp),d0

	cmp.w	#'?',d0	; d0 = '?'
	beq.s	2$

	cmp.w	#'0',d0
	blt.s	1$	; d0 < '0'

	cmp.w	#'9',d0
	bgt.s	1$	; d0 > '9'

2$	moveq	#1,d0
	rts

1$	moveq	#0,d0
	rts

*--------------------------------------------------------------------------

	xdef	_Num4

_Num4:	move.l	4(sp),d0

	cmp.w	#'=',d0	; d0 = '='
	beq.s	2$

	cmp.w	#'0',d0
	blt.s	1$	; d0 < '0'

	cmp.w	#'9',d0
	bgt.s	1$	; d0 > '9'

2$	moveq	#1,d0
	rts

1$	moveq	#0,d0
	rts

	end
