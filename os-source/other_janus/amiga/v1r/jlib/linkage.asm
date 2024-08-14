
;********************************************************************
;
; linkage.asm -- link library for janus stuff
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************

	INCLUDE "exec/types.i"


FUNCDEF MACRO
	XREF	_LVO\1
	XDEF	_\1
	ENDM

	INCLUDE "jfuncs.i"

SAVE	MACRO
	move.l	a6,-(sp)
	move.l	_JanusBase,a6
	ENDM


RESTORE MACRO
	move.l	(sp)+,a6
	rts
	ENDM


CALL	MACRO
	JSR	_LVO\1(a6)
	ENDM


	XREF	_JanusBase



_SetJanusHandler:	; ( jintnum, intserver )(d0/a1)
	SAVE
	movem.l 8(sp),d0/a1
	CALL	SetJanusHandler
	RESTORE


_SetJanusEnable:	; ( jintnum, newvalue )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	SetJanusEnable
	RESTORE


_SetJanusRequest:	; ( jintnum, newvalue )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	SetJanusRequest
	RESTORE


_SendJanusInt:		; ( jintnum )(d0)
	SAVE
	move.l 8(sp),d0
	CALL	SendJanusInt
	RESTORE


_CheckJanusInt: 	; ( jintnum )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	CheckJanusInt
	RESTORE


_AllocJanusMem: 	; ( size, type )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	AllocJanusMem
	RESTORE


_FreeJanusMem:		; ( ptr, size )(a1,d0)
	SAVE
	move.l	8(sp),a1
	move.l	12(sp),d0
	CALL	FreeJanusMem
	RESTORE


_JanusMemBase:		; ( type )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	JanusMemBase
	RESTORE


_JanusMemType:		; ( ptr )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	JanusMemType
	RESTORE


_JanusMemToOffset:	; ( ptr )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	JanusMemToOffset
	RESTORE


_GetParamOffset:	 ; ( jintnum )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	GetParamOffset
	RESTORE


_SetParamOffset:	; ( jintnum, offset )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	SetParamOffset
	RESTORE


_GetJanusStart: 	; ()()
	SAVE
	CALL	GetJanusStart
	RESTORE


_SetupJanusSig: 	; ( jintnum, signum, memsize, memtype )(d0/d1/d2/d3)
	SAVE
	movem.l d2/d3,-(sp)
	movem.l 8+8(sp),d0/d1/d2/d3
	CALL	SetupJanusSig
	movem.l (sp)+,d2/d3
	RESTORE


_CleanupJanusSig:	; ( setupsig )(a0)
	SAVE
	move.l	8(sp),a0
	CALL	CleanupJanusSig
	RESTORE


_JanusLock:		; ( ptr )(a0)
	SAVE
	move.l	8(sp),a0
	CALL	JanusLock
	RESTORE


_JanusUnLock:		; ( ptr )(a0)
	SAVE
	move.l	8(sp),a0
	CALL	JanusUnLock
	RESTORE

_JBCopy:		; ( source, destination, length )(a0/a1,d0)
	SAVE
	movem.l 8(sp),a0/a1
	move.l	16(sp),d0
	CALL	JBCopy	      
	RESTORE

	END


