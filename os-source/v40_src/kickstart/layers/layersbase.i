*******************************************************************************
*
*	$Id: layersbase.i,v 39.1 92/06/05 11:47:33 mks Exp $
*
*******************************************************************************

    IFND    GRAPHICS_LAYERSBASE_I
GRAPHICS_LAYERSBASE_I  SET 1

    IFND    EXEC_LISTS_I
	include 'exec/lists.i'
    ENDC

    IFND    EXEC_LIBRARIES_I
    include 'exec/libraries.i'
    ENDC

    IFND    UTLITY_HOOKS_H
    include 'utility/hooks.i'
    ENDC

LMN_REGION	EQU	-1
LMN_BITMAP	EQU	-2

 STRUCTURE  LayerInfo_extra,0
        STRUCT  lie_env,13*4
        STRUCT  lie_mem,MLH_SIZE
	UBYTE	lie_opencount
	STRUCT	lie_pad,3
 LABEL      lie_SIZEOF

 STRUCTURE  LayersBase,LIB_SIZE
	APTR	lb_GfxBase
	APTR	lb_ExecBase
	APTR	lb_UtilityBase
	LABEL	lb_SIZEOF
*
*******************************************************************************
*
CALLSYS		MACRO
		IFND	_LVO\1
		xref	_LVO\1		; Set the external reference
		ENDC
		jsr	_LVO\1(a6)
		ENDM
*
*******************************************************************************
*
* A macro for PRINTF that does not touch the registers
*
		IFND	PRINTF
PRINTF		MACRO	; <string>,...
		XREF	KPrintF
PUSHCOUNT	SET	0

		IFNC	'\9',''
		move.l	\9,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\8',''
		move.l	\8,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\7',''
		move.l	\7,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\6',''
		move.l	\6,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\5',''
		move.l	\5,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\4',''
		move.l	\4,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\3',''
		move.l	\3,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\2',''
		move.l	\2,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		movem.l a0/a1/d0/d1,-(a7)
		lea.l	PSS\@(pc),a0
		lea.l	16(a7),a1
		jsr	KPrintF
		movem.l (a7)+,a0/a1/d0/d1
		bra.s	PSE\@

PSS\@		dc.b	\1
		dc.b	10
		dc.b	0
		cnop	0,2
PSE\@
		IFNE	PUSHCOUNT
		lea.l	PUSHCOUNT(a7),a7
		ENDC	;IFNE	PUSHCOUNT
		ENDM	;PRINTF	MACRO
		ENDC	;IFND	PRINTF
*
*******************************************************************************

    ENDC
