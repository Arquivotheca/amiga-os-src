; libinit.asm - standard library initialization

        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/alerts.i"
	INCLUDE "exec/memory.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "libraries/dos.i"

        INCLUDE "videocd_rev.i"
        INCLUDE "libbase.i"

        LIST

;---------------------------------------------------------------------------

	XREF	_LibInit
	XREF	_LibOpen
	XREF	_LibClose
	XREF	_LibExpunge

	XREF	_GetCDTypeA
	XREF	_ObtainCDHandleA
	XREF	_ReleaseCDHandle
	XREF	_GetVideoCDInfoA
	XREF	_FreeVideoCDInfo

        XREF    ENDCODE

;---------------------------------------------------------------------------

	XDEF	_LowMemory
        XDEF    LibReserved

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
        moveq   #-1,d0
        rts

;-----------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD			; UWORD RT_MATCHWORD
        DC.L    ROMTAG				; APTR  RT_MATCHTAG
        DC.L    ENDCODE				; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT|RTF_COLDSTART	; UBYTE RT_FLAGS
        DC.B    VERSION				; UBYTE RT_VERSION
        DC.B    NT_LIBRARY			; UBYTE RT_TYPE
        DC.B    0				; BYTE  RT_PRI
        DC.L    LibName				; APTR  RT_NAME
        DC.L    LibId				; APTR  RT_IDSTRING
        DC.L    LibInitTable			; APTR  RT_INIT

LibName DC.B 'videocd.library',0
LibId   VSTRING

        CNOP    0,4

LibInitTable:
        DC.L    LibBase_SIZEOF
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
        V_DEF	LibReserved

	V_DEF	_GetCDTypeA
	V_DEF	_ObtainCDHandleA
	V_DEF	_ReleaseCDHandle
	V_DEF	_GetVideoCDInfoA
	V_DEF	_FreeVideoCDInfo

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

_LowMemory:
	moveq.l	#MEM_DID_NOTHING,d0
	rts

;-----------------------------------------------------------------------

LibReserved:
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------
        END