*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** LoseService ram interface
	XREF	_ServicesBase
	SECTION	services
	XDEF	_LoseService
_LoseService:
		move.l	a6,-(a7)
		move.l	_ServicesBase,a6
		move.l	8(a7),a0
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
