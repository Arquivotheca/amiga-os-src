
;	SECT	text,0,0,1,2

	XDEF	_ChangeServer

	XREF	_LVOSignal

_ChangeServer
* a1 should point to mydata, structure contains task, signal to send
* see main
	move.l  $4,a6
	move.l	4(a1),d0	; get signal to send
	move.l	0(a1),a1	; get task to signal
	jsr     _LVOSignal(a6)
	moveq.l	#0,d0
	rts

	END


