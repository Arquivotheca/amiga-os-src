	IFND DATATYPESBASE_I
DATATYPESBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE	"utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE AmigaGuideLib,LIB_SIZE
        UWORD	ag_UsageCnt				; private use count
	ULONG	ag_SegList				; Segment list

	;--- ROM libraries -------------
	ULONG	ag_SysBase
	ULONG	ag_DOSBase
	ULONG	ag_UtilityBase
	ULONG	ag_GfxBase
	ULONG	ag_IntuitionBase
	ULONG	ag_GadToolsBase
	ULONG	ag_WorkbenchBase

	;--- disk libraries ------------
	ULONG	ag_AslBase
	ULONG	ag_RexxSysBase
	ULONG	ag_LocaleBase
	ULONG	ag_DataTypesBase
	ULONG	ag_AmigaGuideClassBase

	;--- other stuff ---------------
	ULONG	ag_Token				; Global cross reference information
	ULONG	ag_Catalog				; Locale catalog
	STRUCT	ag_Lock,SS_SIZE				; Exclusive access
	ULONG	ag_WindowClass;				; Window class
	ULONG	ag_MemoryPool				; Memory pool
	ULONG	ag_Flags				; Useage flags
	STRUCT	ag_Work,512				; Temporary work buffer

   LABEL AmigaGuideLib_SIZEOF

;-----------------------------------------------------------------------

	LIBINIT

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

	ENDC	; DATATYPESBASE_I
