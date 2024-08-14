*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FindServiceA ram interface
	XREF	_ServicesBase
	SECTION	services
	XDEF	_FindServiceA
_FindServiceA:
		movem.l	a2/a3/a6,-(a7)
		move.l	_ServicesBase,a6
		movem.l	16(a7),a0/a1/a2/a3
		jsr	-30(a6)
		movem.l	(a7)+,a2/a3/a6
		rts
	END
