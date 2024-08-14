*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DeleteEntity ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_DeleteEntity
_DeleteEntity:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		move.l	8(a7),a0
		jsr	-144(a6)
		move.l	(a7)+,a6
		rts
	END
