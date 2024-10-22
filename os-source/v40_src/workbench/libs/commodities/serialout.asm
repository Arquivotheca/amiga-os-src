	OPTIMON

;---------------------------------------------------------------------------

	XREF	_LVORawPutChar
	XREF	_LVORawDoFmt
	XDEF	_kprintf
;	XDEF	_SerialPutStr

;---------------------------------------------------------------------------

;;_SerialPutStr:
; A0 - string pointer
;
;	move.l	a6,-(sp)	    	; save CxBase
;	move.l	4,a6			; load ExecBase
;
;1$:	move.b  (a0)+,d0	    	; next character
;	beq.s   2$		    	; done with string
;	jsr	_LVORawPutChar(a6)	; output the character
;	bra.S   1$			; do next char
;2$:
;	move.l	(sp)+,a6		; restore CxBase
;	rts				; bye
;
;;---------------------------------------------------------------------------
;
;	END


SerPutChar:
	move.l	a6,-(sp)
	move.l	4,a6
	jsr	_LVORawPutChar(a6)
	move.l	(sp)+,a6
	rts


_kprintf:
	move.l	4(sp),a0            * format string
	lea	8(sp),a1            * values to print

	movem.l	a2,-(sp)
	lea	SerPutChar,a2

KDoFmt:
	move.l	a6,-(sp)
	move.l	4,a6
	jsr	_LVORawDoFmt(a6)
	move.l	(sp)+,a6

	movem.l	(sp)+,a2
	rts

;---------------------------------------------------------------------------

	END
