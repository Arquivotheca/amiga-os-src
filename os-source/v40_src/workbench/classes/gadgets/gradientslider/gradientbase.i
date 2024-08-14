	IFND GRADIENTBASE_I
GRADIENTBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"

;-----------------------------------------------------------------------

   STRUCTURE GradientLib,LIB_SIZE
        UWORD	gb_Pad
	ULONG	gb_SysBase
	ULONG   gb_UtilityBase
	ULONG	gb_IntuitionBase
	ULONG	gb_GfxBase
	ULONG	gb_SegList

	APTR	gb_SMult32
	APTR	gb_UMult32
	APTR	gb_SDivMod32
	APTR	gb_UDivMod32

	APTR    gb_Class
   LABEL GradientLib_SIZEOF

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

	ENDC	; GRADIENTBASE_I
