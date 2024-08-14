*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** crypt rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_crypt
_crypt:
		movem.l	a2/a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	12(a7),a0/a1/a2
		jsr	-246(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
