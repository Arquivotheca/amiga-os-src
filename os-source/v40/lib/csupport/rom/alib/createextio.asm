*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CreateExtIO rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_CreateExtIO
_CreateExtIO:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),a0
		move.l	12(a7),d0
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END