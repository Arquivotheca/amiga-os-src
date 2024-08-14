        IFND FSDBASE_I
FSDBASE_I SET   1

;-----------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/semaphores.i"
        INCLUDE "utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE FSDSvc,LIB_SIZE
        ULONG   FSD_DOSBase
        ULONG   FSD_NIPCBase
        ULONG   FSD_SysBase
        ULONG   FSD_UtilityBase
        ULONG   FSD_IntuitionBase
        APTR    FDB_IFFParseBase
        APTR    FSD_AccountsBase
        ULONG   FSD_Entity
        ULONG   FSD_SegList
        STRUCT  FSD_Mounts,LH_SIZE
	STRUCT	FSD_ExAllList,MLH_SIZE
        STRUCT  FSD_Current,MLH_SIZE
        STRUCT  FSD_CurrentLock,SS_SIZE
        STRUCT  FSD_OpenLock,SS_SIZE
        UBYTE   FSD_Mode
        UBYTE   FSD_Filler
	STRUCT	FSD_ExallBuffer,400

   LABEL FSDSvc_SIZEOF

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

        ENDC    ; FSDBASE_I

