* Amiga Grand Wack
*
* nmprintf.asm -- Print functions.
*
* $Id: nmprintf.asm,v 39.1 93/07/16 18:25:02 peter Exp $
*
* Contains sprintf-family functions.

	XDEF _sprintf
	XDEF _sprintfA
	XREF _AbsExecBase
	XREF _LVORawDoFmt

_sprintf:	; ( ostring, format, {values} )
	movem.l a2/a3/a6,-(sp)

	lea.l	6*4(sp),a1       ;Get the pointer to the DataStream
sprintf_common:
	move.l	4*4(sp),a3       ;Get the output string pointer
	move.l	5*4(sp),a0       ;Get the FormatString pointer
	lea.l	stuffChar(pc),a2
	move.l	_AbsExecBase,a6
	jsr	_LVORawDoFmt(a6)

	movem.l (sp)+,a2/a3/a6
	rts

_sprintfA:	; ( ostring, format, valueaddr )
	movem.l a2/a3/a6,-(sp)

	move.l	6*4(sp),a1       ;Get the pointer to the DataStream
	bra.s	sprintf_common

;------ PutChProc function used by RawDoFmt -----------
stuffChar:
	move.b	d0,(a3)+        ;Put data to output string
	rts
