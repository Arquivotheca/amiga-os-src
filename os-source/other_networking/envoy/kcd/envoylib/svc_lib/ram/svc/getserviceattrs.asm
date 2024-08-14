*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetServiceAttrs ram interface
	XREF	_SvcBase
	SECTION	svc
	XDEF	_GetServiceAttrs
_GetServiceAttrs:
		move.l	a6,-(a7)
		move.l	_SvcBase,a6
		lea	8(a7),a0
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
