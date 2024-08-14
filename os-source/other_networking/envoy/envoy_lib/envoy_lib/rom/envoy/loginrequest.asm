*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** LoginRequest rom interface
	XREF	_EnvoyBaseOffset
	SECTION	envoy
	XDEF	_LoginRequest
_LoginRequest:
		move.l	a6,-(a7)
		move.l	_EnvoyBaseOffset(a6),a6
		lea	8(a7),a0
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
