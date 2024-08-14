*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** fputc rom interface
	XREF	_NoneOffset
	SECTION	alib_stdio
	XDEF	_fputc
_fputc:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	8(a7),d0/d1
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
