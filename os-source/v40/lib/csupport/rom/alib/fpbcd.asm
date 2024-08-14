*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** fpbcd rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_fpbcd
_fpbcd:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	8(a7),d0/a0
		jsr	-168(a6)
		move.l	(a7)+,a6
		rts
	END
