        IFND SERVICESBASE_I
SERVICESBASE_I SET   1

;-----------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/semaphores.i"
        INCLUDE "utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE ERCSvc,LIB_SIZE
        STRUCT  ERC_OpenLock,SS_SIZE
        ULONG   ERC_SegList
        ULONG   ERC_DOSBase
        ULONG   ERC_NIPCBase
        ULONG   ERC_SysBase
        ULONG   ERC_UtilityBase
        ULONG   ERC_Entity
	ULONG	ERC_Status
        ULONG   ERC_SigMask
        ULONG   ERC_LibProc
        ULONG   ERC_SpawnedProc
        ULONG	ERC_ResetSig

   LABEL ERCSvc_SIZEOF

;-----------------------------------------------------------------------

        LIBINIT

        LIBDEF  _LVOStartServce

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

        ENDC    ; SERVICESBASE_I
