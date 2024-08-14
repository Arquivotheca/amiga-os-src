	IFND ASCIIBASE_I
ASCIIBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE	"utility/tagitem.i"

;-----------------------------------------------------------------------

   STRUCTURE ASCIILib,LIB_SIZE
        UWORD	ascii_Pad
	ULONG	ascii_SysBase
	ULONG	ascii_DOSBase
	ULONG	ascii_IntuitionBase
	ULONG	ascii_GfxBase
	ULONG	ascii_LayersBase
	ULONG	ascii_UtilityBase
	ULONG	ascii_FileTypesBase
	ULONG	ascii_SuperClassBase;
	ULONG	ascii_SegList
	STRUCT	ascii_Lock,SS_SIZE
	ULONG	ascii_Class
   LABEL ASCIILib_SIZEOF

;-----------------------------------------------------------------------

ASCIISaveRegs	reg	D2-D7/A2-A5

;-----------------------------------------------------------------------

	LIBINIT

	LIBDEF	_LVODispatch

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

	ENDC	; ASCIIBASE_I
