*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** PingEntity ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_PingEntity
_PingEntity:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		move.l	8(a7),a0
		move.l	12(a7),d0
		jsr	-216(a6)
		move.l	(a7)+,a6
		rts
	END
