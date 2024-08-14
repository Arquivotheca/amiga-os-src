*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** fprintf rom interface
	XREF	_NoneOffset
	SECTION	alib_stdio
	XDEF	_fprintf
_fprintf:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		lea	8(a7),a0
		move.l	(a0)+,d0
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
