	IFND CXLIB_I
CXLIB_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "exec/interrupts.i"

;----------------------------------------------------------------------

NUMPOOLS EQU 33

   STRUCTURE CxLib,LIB_SIZE
        UWORD   cx_UsageCnt
	ULONG	cx_UtilityBase
	ULONG   cx_KeymapBase
	ULONG	cx_SysBase
	ULONG	cx_GfxBase
	ULONG	cx_SegList

        STRUCT	cx_IHandlerReq,IOSTD_SIZE
        STRUCT  cx_IHandlerNode,IS_SIZE
        STRUCT	cx_IHandlerPort,MP_SIZE

	STRUCT	cx_Objects,MLH_SIZE
	STRUCT	cx_Messages,MLH_SIZE
	APTR	cx_Zero

        STRUCT	cx_LibLock,SS_SIZE
        WORD	cx_Pad0

        APTR    cx_EndChain
        APTR	cx_ReturnChain

        STRUCT  cx_Pools,NUMPOOLS*4

	STRUCT	cx_EventReply,MP_SIZE
   LABEL CxLib_SIZEOF

;---------------------------------------------------------------------------

CALL MACRO <Fuction_Name>
	xref _LVO\1
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

GO   MACRO <Function_Name>
	xref _LVO\1
 	jmp _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; CXLIB_I
