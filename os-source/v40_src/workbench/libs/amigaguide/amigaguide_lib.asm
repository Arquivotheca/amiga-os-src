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

        INCLUDE "amigaguide_rev.i"
        INCLUDE "amigaguidebase.i"

        LIST

;---------------------------------------------------------------------------

	XREF	_LibInit
	XREF	_LibOpenLVO
	XREF	_LibCloseLVO
	XREF	_LibExpungeLVO
	XREF	_ARexxHostLVO
	XREF	_LockAmigaGuideBaseLVO
	XREF	_UnlockAmigaGuideBaseLVO
*	XREF	_ExpungeDataBasesLVO
	XREF	_OpenAmigaGuideALVO
	XREF	_OpenAmigaGuideAsyncALVO
	XREF	_CloseAmigaGuideLVO
	XREF	_AmigaGuideSignalLVO
	XREF	_GetAmigaGuideMsgLVO
	XREF	_ReplyAmigaGuideMsgLVO
	XREF	_SetAmigaGuideContextALVO
	XREF	_SendAmigaGuideContextALVO
	XREF	_SendAmigaGuideCmdALVO
	XREF	_SetAmigaGuideAttrsALVO
	XREF	_GetAmigaGuideAttrLVO
*	XREF	_SetAmigaGuideHookLVO
	XREF	_LoadXRefLVO
	XREF	_ExpungeXRefLVO
	XREF	_AddAmigaGuideHostALVO
	XREF	_RemoveAmigaGuideHostALVO
	XREF	_OpenELVO
	XREF	_LockELVO
	XREF	_CopyPathListLVO
	XREF	_AddPathEntriesLVO
	XREF	_FreePathListLVO
	XREF	_ParsePathStringLVO
*	XREF	_OpenDataBaseLVO
*	XREF	_LoadNodeLVO
*	XREF	_UnloadNodeLVO
*	XREF	_CloseDataBaseLVO
	XREF	_GetAmigaGuideStringLVO

        XREF    ENDCODE

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

LibName DC.B 'amigaguide.library',0
LibId   VSTRING

        CNOP    0,4

LibInitTable:
        DC.L    AmigaGuideLib_SIZEOF
        DC.L    LibFuncTable
        DC.L    LibDataTable
        DC.L    _LibInit

V_DEF	MACRO
	DC.W	\1+(*-LibFuncTable)
	ENDM

LibFuncTable:
	DC.W	-1
        V_DEF	_LibOpenLVO
        V_DEF	_LibCloseLVO
        V_DEF	_LibExpungeLVO
        V_DEF	LibReserved
	V_DEF	ARexxHost
	V_DEF	_LockAmigaGuideBaseLVO
	V_DEF	_UnlockAmigaGuideBaseLVO
	V_DEF	LibReserved			; _ExpungeDataBasesLVO
	V_DEF	_OpenAmigaGuideALVO
	V_DEF	_OpenAmigaGuideAsyncALVO
	V_DEF	_CloseAmigaGuideLVO
	V_DEF	_AmigaGuideSignalLVO
	V_DEF	_GetAmigaGuideMsgLVO
	V_DEF	_ReplyAmigaGuideMsgLVO
	V_DEF	_SetAmigaGuideContextALVO
	V_DEF	_SendAmigaGuideContextALVO
	V_DEF	_SendAmigaGuideCmdALVO
	V_DEF	_SetAmigaGuideAttrsALVO
	V_DEF	_GetAmigaGuideAttrLVO
	V_DEF	LibReserved			; _SetAmigaGuideHookLVO
	V_DEF	_LoadXRefLVO
	V_DEF	_ExpungeXRefLVO
	V_DEF	_AddAmigaGuideHostALVO
	V_DEF	_RemoveAmigaGuideHostALVO
	V_DEF	_OpenELVO
	V_DEF	_LockELVO
	V_DEF	_CopyPathListLVO
	V_DEF	_AddPathEntriesLVO
	V_DEF	_FreePathListLVO
	V_DEF	_ParsePathStringLVO
	V_DEF	LibReserved			; _OpenDataBaseLVO
	V_DEF	LibReserved			; _LoadNodeLVO
	V_DEF	LibReserved			; _UnloadNodeLVO
	V_DEF	LibReserved			; _CloseDataBaseLVO
	V_DEF	_GetAmigaGuideStringLVO

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

ARexxHost:
	subq.l	#4,sp		; room where to put result
	move.l	sp,a1
	bsr	_ARexxHostLVO	; call the real function
	movea.l	(sp)+,a0	; put the result string into A0
	rts			; bye

;-----------------------------------------------------------------------

        END
