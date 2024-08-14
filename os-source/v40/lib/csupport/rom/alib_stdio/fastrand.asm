*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FastRand rom interface
	XREF	_NoneOffset
	SECTION	alib_stdio
	XDEF	_FastRand
_FastRand:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),d0
		jsr	-72(a6)
		move.l	(a7)+,a6
		rts
	END
