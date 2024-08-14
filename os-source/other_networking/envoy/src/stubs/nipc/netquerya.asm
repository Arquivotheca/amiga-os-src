*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NetQueryA ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_NetQueryA
_NetQueryA:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		move.l	8(a7),a0
		movem.l	12(a7),d0/d1/a1
		jsr	-210(a6)
		move.l	(a7)+,a6
		rts
	END
