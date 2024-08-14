	IFND WHEELBASE_I
WHEELBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"

;-----------------------------------------------------------------------

   STRUCTURE WheelLib,LIB_SIZE
        UWORD	wb_Pad
	ULONG	wb_SysBase
	ULONG   wb_UtilityBase
	ULONG	wb_IntuitionBase
	ULONG	wb_GfxBase
	ULONG	wb_SegList

	APTR	wb_SMult32
	APTR	wb_UMult32
	APTR	wb_SDivMod32
	APTR	wb_UDivMod32

	APTR    wb_Class
   LABEL WheelLib_SIZEOF

;-----------------------------------------------------------------------

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

	ENDC	; WHEELBASE_I
