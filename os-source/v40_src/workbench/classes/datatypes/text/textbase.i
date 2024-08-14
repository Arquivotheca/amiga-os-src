	IFND TEXTBASE_I
TEXTBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE	"utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE TextLib,LIB_SIZE
        UWORD	txl_Pad
	ULONG	txl_SysBase
	ULONG	txl_DOSBase
	ULONG	txl_IntuitionBase
	ULONG	txl_GfxBase
	ULONG	txl_UtilityBase
	ULONG	txl_LayersBase
	ULONG	txl_IFFParseBase
	ULONG	txl_DataTypesBase
	ULONG	txl_SegList
	STRUCT	txl_Lock,SS_SIZE
	ULONG	txl_Class
   LABEL TextLib_SIZEOF

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

	ENDC	; TEXTBASE_I
