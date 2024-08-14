*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** fclose rom interface
	XREF	_NoneOffset
	SECTION	alib_stdio
	XDEF	_fclose
_fclose:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),d0
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
