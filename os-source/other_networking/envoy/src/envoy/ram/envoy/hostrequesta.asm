*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** HostRequestA ram interface
	XREF	_EnvoyBase
	SECTION	envoy
	XDEF	_HostRequestA
_HostRequestA:
		move.l	a6,-(a7)
		move.l	_EnvoyBase,a6
		move.l	8(a7),a0
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
