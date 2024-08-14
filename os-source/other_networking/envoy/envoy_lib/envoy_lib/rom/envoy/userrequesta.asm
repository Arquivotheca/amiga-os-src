*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** UserRequestA rom interface
	XREF	_EnvoyBaseOffset
	SECTION	envoy
	XDEF	_UserRequestA
_UserRequestA:
		move.l	a6,-(a7)
		move.l	_EnvoyBaseOffset(a6),a6
		move.l	8(a7),a0
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
