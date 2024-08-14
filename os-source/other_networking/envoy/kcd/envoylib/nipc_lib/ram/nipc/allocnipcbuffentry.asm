*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocNIPCBuffEntry ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_AllocNIPCBuffEntry
_AllocNIPCBuffEntry:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		jsr	-240(a6)
		move.l	(a7)+,a6
		rts
	END
