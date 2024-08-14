*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** HostRequest ram interface
	XREF	_EnvoyBase
	SECTION	envoy
	XDEF	_HostRequest
_HostRequest:
		move.l	a6,-(a7)
		move.l	_EnvoyBase,a6
		lea	8(a7),a0
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
