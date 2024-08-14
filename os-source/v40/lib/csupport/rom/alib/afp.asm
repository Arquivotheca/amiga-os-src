*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** afp rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_afp
_afp:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),a0
		jsr	-144(a6)
		move.l	(a7)+,a6
		rts
	END
