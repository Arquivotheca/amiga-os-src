*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StartService ram interface
	XREF	_SvcBase
	SECTION	svc
	XDEF	_StartService
_StartService:
		move.l	a6,-(a7)
		move.l	_SvcBase,a6
		lea	8(a7),a0
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
