	IFND DATATYPESBASE_I
DATATYPESBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE	"utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE DataTypesLib,LIB_SIZE
        UWORD	dtl_UsageCnt

	ULONG	dtl_SysBase
	ULONG	dtl_DOSBase
	ULONG	dtl_IntuitionBase
	ULONG	dtl_GfxBase
	ULONG	dtl_LayersBase
	ULONG	dtl_UtilityBase
	ULONG	dtl_IFFParseBase
	ULONG	dtl_RexxSysBase
	ULONG	dtl_LocaleBase

	ULONG	dtl_Catalog
	ULONG	dtl_SegList

	STRUCT	dtl_Lock,(SS_SIZE*10)
	ULONG	dtl_Token;
	ULONG	dtl_DTClass;
	ULONG	dtl_DTNClass;
	ULONG	dtl_NotifyProc;
	ULONG	dtl_TickProc;
	ULONG	dtl_Flags

   LABEL DataTypesLib_SIZEOF

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
