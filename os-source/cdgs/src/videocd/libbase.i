	IFND LIBBASE_I
LIBBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE	"utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE LibBase,LIB_SIZE
        UWORD	lb_Pad
	ULONG	lb_SysBase
	ULONG	lb_UtilityBase
	ULONG	lb_SegList

	STRUCT	lb_IS,IS_SIZE				; Low memory interrupt handler
	STRUCT	lb_Lock,SS_SIZE				; Exclusive access
	STRUCT	lb_HandleList,MLH_SIZE			; List of handles

   LABEL LibBase_SIZEOF

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

	ENDC	; LIBBASE_I
