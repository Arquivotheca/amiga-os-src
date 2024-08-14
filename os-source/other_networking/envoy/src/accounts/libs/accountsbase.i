        IFND ACCOUNTSBASE_I
ACCOUNTSBASE_I SET   1

;-----------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/semaphores.i"
        INCLUDE "utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE AccountsLib,LIB_SIZE
        ULONG   ACNTS_NIPCBase
        ULONG   ACNTS_SysBase
        ULONG	ACNTS_DOSBase
        ULONG	ACNTS_UtilityBase
        ULONG   ACNTS_SegList

	ULONG	ACNTS_Entity
	ULONG	ACNTS_AccEntity

	STRUCT  ACNTS_Lock,SS_SIZE
	ULONG	ACNTS_HTag
	ULONG	ACNTS_GVCode
	STRUCT  ACNTS_HostName,128

   LABEL AccountsLib_SIZEOF

;-----------------------------------------------------------------------

        LIBINIT

	LIBDEF	_LVOAllocUserInfo
	LIBDEF	_LVOAllocGroupInfo
	LIBDEF	_LVOFreeUserInfo
	LIBDEF	_LVOFreeGroupInfo
	LIBDEF	_LVOVerifyUser
	LIBDEF	_LVOMemberOf
	LIBDEF	_LVONameToUser
	LIBDEF	_LVONameToGroup
	LIBDEF	_LVOIDToUser
	LIBDEF	_LVOIDToGroup
	LIBDEF	_LVONextUser
	LIBDEF	_LVONextGroup
	LIBDEF	_LVONextMember
	LIBDEF	_LVOECrypt
	LIBDEF	_LVOVerifyUserCrypt

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

        ENDC    ; ACCOUNTSBASE_I
