*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetEntityAttrs ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_GetEntityAttrs
_GetEntityAttrs:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		lea	8(a7),a1
		move.l	(a1)+,a0
		jsr	-234(a6)
		move.l	(a7)+,a6
		rts
	END
