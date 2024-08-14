*****************************************************
*		Parsec Soft Systems string functions		*
*****************************************************

;---------------------------------------------
;		l_to_a(num,str) - convert long to string

		section	l_to_a.asm,code

		include	'macros.i'

		DECLAREL l_to_a

		movem.l	a2/a3,-(sp)				; save regs
		lea		1$(pc),a0				; format string
		lea		12(sp),a1				; address of long on stack
		lea		2$(pc),a2				; routine to call per character
		move.l	16(sp),a3				; string buffer

		move.l	a6,-(sp)
		CallEx	RawDoFmt
		move.l	(sp)+,a6

		movem.l	(sp)+,a2/a3
		move.l	8(sp),d0				; return string buffer
		rts

1$		dc.b	'%ld',0

2$		move.b	d0,(a3)+
		rts

		end
