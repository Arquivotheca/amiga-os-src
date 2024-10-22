        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/alerts.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "libraries/dos.i"

        INCLUDE "datatypes_rev.i"
        INCLUDE "datatypesbase.i"

        LIST

;---------------------------------------------------------------------------

	XREF	_LibInit
	XREF	_LibOpen
	XREF	_LibClose
	XREF	_LibExpunge

	XREF	_RLDispatch
	XREF	_ObtainDataTypeA
	XREF	_ReleaseDataType
	XREF	_NewDTObjectA
	XREF	_DisposeDTObject
	XREF	_SetDTAttrsA
	XREF	_GetDTAttrsA
	XREF	_AddDTObject
	XREF	_RefreshDTObjectA
	XREF	_DoAsyncLayout
	XREF	_DoDTMethodA
	XREF	_RemoveDTObject
	XREF	_GetDTMethods
	XREF	_GetDTTriggerMethods
	XREF	_PrintDTObjectA
	XREF	_ObtainDTDrawInfoA
	XREF	_DrawDTObjectA
	XREF	_ReleaseDTDrawInfo
	XREF	_GetDTStringLVO

        XREF    ENDCODE

;---------------------------------------------------------------------------

        XDEF    LibReserved
	XDEF	ARLDispatch

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
        moveq   #-1,d0
        rts

;-----------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD           ; UWORD RT_MATCHWORD
        DC.L    ROMTAG                  ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                 ; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT            ; UBYTE RT_FLAGS
        DC.B    VERSION                 ; UBYTE RT_VERSION
        DC.B    NT_LIBRARY              ; UBYTE RT_TYPE
        DC.B    0                       ; BYTE  RT_PRI
        DC.L    LibName                 ; APTR  RT_NAME
        DC.L    LibId                   ; APTR  RT_IDSTRING
        DC.L    LibInitTable            ; APTR  RT_INIT

LibName DC.B 'datatypes.library',0
LibId   VSTRING

        CNOP    0,4

LibInitTable:
        DC.L    DataTypesLib_SIZEOF
        DC.L    LibFuncTable
        DC.L    LibDataTable
        DC.L    _LibInit

V_DEF	MACRO
	DC.W	\1+(*-LibFuncTable)
	ENDM

LibFuncTable:
	DC.W	-1
        V_DEF	_LibOpen
        V_DEF	_LibClose
        V_DEF	_LibExpunge
        V_DEF	LibReserved

	V_DEF	ARLDispatch
	V_DEF	_ObtainDataTypeA
	V_DEF	_ReleaseDataType
	V_DEF	_NewDTObjectA
	V_DEF	_DisposeDTObject
	V_DEF	_SetDTAttrsA
	V_DEF	_GetDTAttrsA
	V_DEF	_AddDTObject
	V_DEF	_RefreshDTObjectA
	V_DEF	_DoAsyncLayout
	V_DEF	_DoDTMethodA
	V_DEF	_RemoveDTObject
	V_DEF	_GetDTMethods
	V_DEF	_GetDTTriggerMethods
	V_DEF	_PrintDTObjectA
	V_DEF	_ObtainDTDrawInfoA
	V_DEF	_DrawDTObjectA
	V_DEF	_ReleaseDTDrawInfo
	V_DEF	_GetDTStringLVO

        DC.W   -1

LibDataTable:
        INITBYTE   LN_TYPE,NT_LIBRARY
        INITLONG   LN_NAME,LibName
        INITBYTE   LIB_FLAGS,(LIBF_SUMUSED!LIBF_CHANGED)
        INITWORD   LIB_VERSION,VERSION
        INITWORD   LIB_REVISION,REVISION
        INITLONG   LIB_IDSTRING,LibId
        DC.W       0

        CNOP    0,4

;-----------------------------------------------------------------------

LibReserved:
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------
; LONG RLDispatch (struct RexxMsg * rmsg, STRPTR * result);

ARLDispatch:
	movem.l	d2/d3/d4-d7/a2-a6,-(sp)	; save the registers
	subq.l	#4,sp			; realign the stack
	move.l	sp,-(sp)		; push return pointer
	move.l	a0,-(sp)		; push REXX message
	jsr	_RLDispatch		; call the 'C' Dispatch function
	addq.l	#8,sp			; restore the stack pointer
	movea.l	(sp)+,a0		; put the result string into A0
	movem.l	(sp)+,d2/d3/d4-d7/a2-a6	; restore the registers
	rts				; return

;-----------------------------------------------------------------------

        END
