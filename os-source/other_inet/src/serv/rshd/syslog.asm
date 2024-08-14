; syslog.asm
;
; implements normal printf-like sytax for syslog()
;

	section	CODE
	xdef	_syslog
	xref	_SockBase
	xref	_SysBase
	xref	_LVORawDoFmt

_syslog
	movem.l	A2/A3/A4/A6,-(A7)	; save registers
	lea.l	7*4(A7),A1	; A1 is pointer to args
	move.l	6*4(A7),A0	; A0 is format string
	move.l	5*4(A7),A4	; A4 is priority

	lea.l	stuffChar(PC),A2
	sub.l	#256,A7		; reserve 256 bytes on stack for buffer
	move.l	A7,A3		; A3 points to buffer

	move.l	_SysBase,A6	; load exec library base into A6
	jsr	_LVORawDoFmt(A6)

	move.l	A3,A0		; A0 is pointer to buffer
	move.l	A4,D0		; D0 is priority
	move.l	_SockBase,A6
	jsr	$FFFFFE5C(A6)	; syslog()

	lea.l	256(A7),A7	
	movem.l	(A7)+,A2/A3/A4/A6	; restore registers
	rts

	;------ PutChProc function used by RawDoFmt -----------
stuffChar:
	move.b	D0,(A3)+	; copy character to buffer
	rts

	END
