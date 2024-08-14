;******************************************************************************
;*
;* COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
;* Commodore-Amiga, Inc.  All rights reserved.
;*
;* DISCLAIMER: This software is provided "as is".  No representations or
;* warranties are made with respect to the accuracy, reliability, performance,
;* currentness, or operation of this software, and all use is at your own risk.
;* Neither commodore nor the authors assume any responsibility or liability
;* whatsoever with respect to your use of this software.
;*
;******************************************************************************
;* sprintf.asm
;* Simple sprintf() based on exec/RawDoFmt()

	XDEF _sprintf
	XREF _AbsExecBase
	XREF _LVORawDoFmt

_sprintf:	; ( ostring, format, {values} )
	movem.l a2/a3/a6,-(sp)

	move.l	4*4(sp),a3       ;Get the output string pointer
	move.l	5*4(sp),a0       ;Get the FormatString pointer
	lea.l	6*4(sp),a1       ;Get the pointer to the DataStream
	lea.l	stuffChar(pc),a2
	move.l	_AbsExecBase,a6
	jsr	_LVORawDoFmt(a6)

	movem.l (sp)+,a2/a3/a6
	rts

;------ PutChProc function used by RawDoFmt -----------
stuffChar:
	move.b	d0,(a3)+        ;Put data to output string
	rts
