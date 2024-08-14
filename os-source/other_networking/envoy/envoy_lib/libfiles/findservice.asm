*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FindService ram interface
	XREF	_ServicesBase
	SECTION	services
	XDEF	_FindService
_FindService:
		movem.l	a2/a3/a6,-(a7)
		move.l	_ServicesBase,a6
		lea	16(a7),a3
		movem.l	(a3)+,a0/a1/a2
		jsr	-30(a6)
		movem.l	(a7)+,a2/a3/a6
		rts
	END
