* Interrupt code for the copper interrupt.

	xdef	_CopServer

_CopServer:
	move.l	8(a1),$dff1a6
	add.l	#$111111,8(a1)
	rts
