*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** UserRequest ram interface
	XREF	_EnvoyBase
	SECTION	envoy
	XDEF	_UserRequest
_UserRequest:
		move.l	a6,-(a7)
		move.l	_EnvoyBase,a6
		lea	8(a7),a0
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
