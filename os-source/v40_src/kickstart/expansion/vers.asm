*************************************************************************
*                                                                       *
* Copyright (C) 1985-1991 Commodore Amiga Inc.  All rights reserved.    *
*                                                                       *
*************************************************************************

*************************************************************************
*
* $Id: vers.asm,v 39.0 91/10/28 16:25:13 mks Exp $
*
* $Log:	vers.asm,v $
* Revision 39.0  91/10/28  16:25:13  mks
* First release of native build V39 expansion code
* 
*************************************************************************


;****** Exports ******************************************************

	XDEF	VERNUM
	XDEF	REVNUM
	XDEF	VERSTRING
        XDEF    subSysName
        XDEF    ExpansionName

;****** Included Files ***********************************************

	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/resident.i"

	INCLUDE	"expansion_rev.i"
	INCLUDE "messages.i"

;****** Imports ******************************************************

;	XREF	InitTable
	XREF	initRoutine
	XREF	DoDiagList	;call diag areas (step one)
	XREF	EndCode


;	moveq	#-1,d0	;In case we are accidentally executed from disk
;	RTS

initDDescrip:				;STRUCTURE RT,0
		DC.W    RTC_MATCHWORD   ; UWORD RT_MATCHWORD
		DC.L    initDDescrip    ; APTR  RT_MATCHTAG
		DC.L    initDDescrip2   ; APTR  RT_ENDSKIP
		IFNE	OnePointFourExec
		  DC.B  RTF_SINGLETASK	; UBYTE RT_FLAGS
		ENDC
		IFEQ	OnePointFourExec
		  DC.B  RTF_COLDSTART	; UBYTE RT_FLAGS
		ENDC
		DC.B    VERSION         ; UBYTE RT_VERSION
		DC.B    NT_LIBRARY      ; UBYTE RT_TYPE
		DC.B    110             ; BYTE  RT_PRI
		DC.L    subSysName      ; APTR  RT_NAME
		DC.L    VERSTRING       ; APTR  RT_IDSTRING
		DC.L    initRoutine     ; APTR  RT_INIT
					; LABEL RT_SIZE

initDDescrip2:				;STRUCTURE RT,0
		DC.W    RTC_MATCHWORD   ; UWORD RT_MATCHWORD
		DC.L    initDDescrip2   ; APTR  RT_MATCHTAG
		DC.L    EndCode         ; APTR  RT_ENDSKIP
		DC.B    RTF_COLDSTART	; UBYTE RT_FLAGS
		DC.B    VERSION         ; UBYTE RT_VERSION
		DC.B    0		; UBYTE RT_TYPE
		DC.B    105             ; BYTE  RT_PRI
		DC.L    subSysName2     ; APTR  RT_NAME
		DC.L    subSysName2     ; APTR  RT_IDSTRING
		DC.L    DoDiagList      ; APTR  RT_INIT
					; LABEL RT_SIZE

subSysName:
ExpansionName:	dc.b	'expansion.library',0
subSysName2:	dc.b	'diag init',13,10,0

VERSTRING:	VSTRING
VERNUM:		EQU	VERSION
REVNUM:		EQU	REVISION

		END
