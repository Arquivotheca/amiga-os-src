	IFND VERSIONBASE_I
VERSIONBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"

;-----------------------------------------------------------------------

   STRUCTURE VersionLib,LIB_SIZE
        UWORD	vl_Pad
	ULONG	vl_DOSBase
	ULONG	vl_UtilityBase
	ULONG	vl_SysBase
	ULONG	vl_SegList

	APTR	vl_SMult32
	APTR	vl_UMult32
	APTR	vl_SDivMod32
	APTR	vl_UDivMod32
   LABEL VersionLib_SIZEOF

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

	ENDC	; VERSIONBASE_I
