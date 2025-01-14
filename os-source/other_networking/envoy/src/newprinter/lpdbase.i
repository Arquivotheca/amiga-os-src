        IFND SERVICESBASE_I
SERVICESBASE_I SET   1

;-----------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/semaphores.i"
        INCLUDE "utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE LPDSvc,LIB_SIZE
        ULONG   LPD_DOSBase
        ULONG   LPD_NIPCBase
        ULONG   LPD_SysBase
        ULONG   LPD_UtilityBase
        ULONG   LPD_Entity
        ULONG   LPD_SegList

        STRUCT  LPD_SpoolLock,SS_SIZE
        STRUCT  LPD_Spool,MLH_SIZE
        STRUCT  LPD_IncomingLock,SS_SIZE
        STRUCT  LPD_Incoming,MLH_SIZE
        STRUCT  LPD_OpenLock,SS_SIZE

   LABEL LPDSvc_SIZEOF

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
