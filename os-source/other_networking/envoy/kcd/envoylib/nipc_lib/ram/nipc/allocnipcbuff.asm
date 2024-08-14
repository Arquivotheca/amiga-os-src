*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocNIPCBuff ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_AllocNIPCBuff
_AllocNIPCBuff:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		move.l	8(a7),d0
		jsr	-234(a6)
		move.l	(a7)+,a6
		rts
	END
