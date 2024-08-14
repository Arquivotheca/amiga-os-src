	XDEF _asprintf
	XREF _AbsExecBase
	XREF _LVORawDoFmt

; void ASM asprintf (REG (a3) STRPTR buffer, REG (a0) STRPTR fmt, REG (a1) APTR data);
_asprintf:	; ( ostring, format, {values} )
	movem.l a2/a6,-(sp)

	lea.l	stuffChar(pc),a2
	move.l	_AbsExecBase,a6
	jsr	_LVORawDoFmt(a6)

	movem.l (sp)+,a2/a6
	rts

;------ PutChProc function used by RawDoFmt -----------
stuffChar:
	move.b	d0,(a3)+        ;Put data to output string
	rts
