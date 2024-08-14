        OPTIMON

;---------------------------------------------------------------------------

        XREF    _Handler

;---------------------------------------------------------------------------


; This funny looking code is what is needed to trick DOS into thinking this
; module is a BCPL module.

CodeHeader:	DC.L	(CodeEnd-CodeHeader)/4	; BCPL & MCC sillyness

CodeStart:      movem.l	a0-a6,-(a7)	; save all these, BCPL uses them
		lsl.l	#2,d1		; startup packet
		move.l	d1,a0		; parameter packet in a0
		bsr.s	_Handler	; get the ball rolling
		movem.l	(a7)+,a0-a6     ; restore everyone
		jmp	(a6)		; BCPL-style return

BCPLTable:	CNOP 	0,4
		DC.L	0		; End of global list
		DC.L	1		; God knows what this is
		DC.L	CodeStart-CodeHeader
		DC.L	150		; max global used (standard value)

CodeEnd EQU	*

;---------------------------------------------------------------------------

	END
