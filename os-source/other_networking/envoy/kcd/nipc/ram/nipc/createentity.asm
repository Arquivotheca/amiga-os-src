*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CreateEntity ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_CreateEntity
_CreateEntity:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		lea	8(a7),a0
		jsr	-138(a6)
		move.l	(a7)+,a6
		rts
	END