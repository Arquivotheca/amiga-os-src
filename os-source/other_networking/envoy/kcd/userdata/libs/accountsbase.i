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
        ULONG   ACNTS_SegList

	ULONG	ACNTS_Entity

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
