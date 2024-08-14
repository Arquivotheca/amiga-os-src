	IFND AMIGAGUIDEBASE_I
AMIGAGUIDEBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE	"utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE AGLib,LIB_SIZE
	ULONG	ag_SegList
        UWORD	ag_UsageCnt

	; ROM libraries
	ULONG	ag_SysBase
	ULONG	ag_DOSBase
	ULONG	ag_IntuitionBase
	ULONG	ag_GfxBase
	ULONG	ag_UtilityBase
	ULONG	ag_LayersBase

	; Workbench libraries
	ULONG	ag_DiskfontBase
	ULONG	ag_DataTypesBase
	ULONG	ag_RexxSysBase
	ULONG	ag_LocaleBase
	ULONG	ag_IFFParseBase

	; other data
	ULONG	ag_Catalog
	ULONG	ag_Class				; AmigaGuide class
	ULONG	ag_ModelClass				; Glue it all together
	ULONG	ag_DatabaseClass			; Database handling class
	ULONG	ag_NodeClass				; Node handling class
	ULONG	ag_GlobalHelp				; Global help database
	ULONG	ag_GlobalPath				; Global search path
	ULONG	ag_Token				; Global cross reference information
	ULONG	ag_Flags				; Misc. flags
	STRUCT	ag_Lock,SS_SIZE				; Exclusive access
	STRUCT	ag_DataBaseList,LH_SIZE			; List of opened databases
	STRUCT	ag_HostList,LH_SIZE			; List of global dynamic node hosts

   LABEL AGLib_SIZEOF

;-----------------------------------------------------------------------

	LIBINIT

	LIBDEF	_LVODispatch

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
	xref _LVO\1
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

GO   MACRO <Function_Name>
	xref _LVO\1
 	jmp _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; AMIGAGUIDEBASE_I
