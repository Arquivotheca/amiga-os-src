*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** sprintf rom interface
	XREF	_NoneOffset
	SECTION	alib_stdio
	XDEF	_sprintf
_sprintf:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		lea	8(a7),a1
		move.l	(a1)+,a0
		jsr	-24(a6)
		move.l	(a7)+,a6
		rts
	END
