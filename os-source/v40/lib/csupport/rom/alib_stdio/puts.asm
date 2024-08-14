*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** puts rom interface
	XREF	_NoneOffset
	SECTION	alib_stdio
	XDEF	_puts
_puts:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),a0
		jsr	-66(a6)
		move.l	(a7)+,a6
		rts
	END
