*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DoSuperMethodA rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_DoSuperMethodA
_DoSuperMethodA:
		movem.l	a2/a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	12(a7),a0/a2
		move.l	20(a7),a1
		jsr	-258(a6)
		movem.l	(a7)+,a2/a6
		rts
	END