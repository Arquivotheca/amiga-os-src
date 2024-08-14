*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** getchar rom interface
	XREF	_NoneOffset
	SECTION	alib_stdio
	XDEF	_getchar
_getchar:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		jsr	-54(a6)
		move.l	(a7)+,a6
		rts
	END
