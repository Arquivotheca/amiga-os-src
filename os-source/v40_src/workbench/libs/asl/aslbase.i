	IFND ASLBASE_I
ASLBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE	"utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE ASLLib,LIB_SIZE
        UWORD	ASL_UsageCnt
	ULONG	ASL_IntuitionBase
	ULONG	ASL_GfxBase
	ULONG	ASL_DosBase
	ULONG	ASL_UtilityBase
	ULONG	ASL_GadToolsBase
	ULONG	ASL_LayersBase
	ULONG   ASL_DiskfontBase
	ULONG	ASL_WorkbenchBase
	ULONG	ASL_SysBase
	ULONG	ASL_IconBase
	ULONG	ASL_SegList

	APTR	ASL_SMult32
	APTR	ASL_UMult32
	APTR	ASL_SDivMod32
	APTR	ASL_UDivMod32

	APTR	ASL_FFont
	APTR	ASL_FTextAttr

        APTR    ASL_AFH
        STRUCT  ASL_FontCache,MLH_SIZE
        STRUCT  ASL_FontCacheLock,SS_SIZE
   LABEL ASLLib_SIZEOF

;-----------------------------------------------------------------------

	LIBINIT

	LIBDEF	_LVOObsAllocFileRequest
	LIBDEF	_LVOObsFreeFileRequest
	LIBDEF	_LVOObsRequestFile

	LIBDEF	_LVOAllocAslRequest
	LIBDEF	_LVOFreeAslRequest
	LIBDEF	_LVOAslRequest

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

	ENDC	; ASLBASE_I
