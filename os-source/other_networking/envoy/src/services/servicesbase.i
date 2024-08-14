        IFND SERVICESBASE_I
SERVICESBASE_I SET   1

;-----------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/semaphores.i"
        INCLUDE "utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE ServicesLib,LIB_SIZE
        ULONG   SVCS_NIPCBase
        ULONG   SVCS_UtilityBase
        ULONG   SVCS_SysBase
        ULONG   SVCS_SegList

        STRUCT  SVCS_ServicesLock,SS_SIZE
        STRUCT  SVCS_Services,MLH_SIZE
        STRUCT  SVCS_OpenLock,SS_SIZE

   LABEL ServicesLib_SIZEOF

;-----------------------------------------------------------------------

        LIBINIT

        LIBDEF  _LVOFindService
        LIBDEF  _LVOLoseService

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
