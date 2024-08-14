
		include "exec/types.i"
		include "exec/exec.i"
		include "exec/resident.i"

		section text,CODE

		xref	_Init
		xref	_CoProc
		xdef	_DeviceName
		xdef	_IdString
		xdef	_DUMmySeg


		moveq.l #0,D0		;   word
		rts			;   word

InitDesc:
		dc.w	RTC_MATCHWORD
		dc.l	InitDesc
		dc.l	EndCode
		dc.b	0		;   not auto-init
		dc.b	1		;   version
		dc.b	NT_DEVICE
		dc.b	0		;   priority
		dc.l	_DeviceName
		dc.l	_IdString
		dc.l	_Init
		dc.l	0		;   extra ?
		dc.l	0
		dc.l	0

		dc.w	0		;   word (long align)

		ds.l	0		;   LW align
		dc.l	16
_DUMmySeg:	dc.l	0
		nop
		nop
		jmp	_CoProc(PC)

		section DATA,DATA

_DeviceName:	dc.b	'fmsdisk.device',0
_IdString:	dc.b	'fmsdisk.device 1.1 (20 Jul 1993)',13,10,0

EndCode:

		END

