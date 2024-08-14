*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ACrypt rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_ACrypt
_ACrypt:
		movem.l	a2/a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	12(a7),a0/a1/a2
		jsr	-270(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
