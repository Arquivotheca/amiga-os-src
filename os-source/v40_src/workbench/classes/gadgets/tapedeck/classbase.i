	IFND CLASSBASE_I
CLASSBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"

;-----------------------------------------------------------------------

   STRUCTURE ClassLib,LIB_SIZE
        UWORD	cb_Pad
	ULONG	cb_SysBase
	ULONG   cb_UtilityBase
	ULONG	cb_IntuitionBase
	ULONG	cb_GfxBase
	ULONG	cb_SegList

	APTR	cb_SMult32
	APTR	cb_UMult32
	APTR	cb_SDivMod32
	APTR	cb_UDivMod32

	APTR    cb_Class
   LABEL ClassLib_SIZEOF

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

	ENDC	; CLASSBASE_I
