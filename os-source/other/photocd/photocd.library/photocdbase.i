                IFND    PHOTOCDBASE_I
PHOTOCDBASE_I   SET     1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE	"utility/tagitem.i"
	INCLUDE "libraries/photocd.i"

;-----------------------------------------------------------------------

   STRUCTURE PhotoCDLib,LIB_SIZE
        UWORD	pcdl_UsageCnt
        BPTR    pcdl_SegList

	ULONG	pcdl_SysBase
	ULONG	pcdl_DOSBase
	ULONG	pcdl_IntuitionBase
	STRUCT  pcdl_ResWidth,2*PHOTOCD_RES_COUNT   ; UWORD (16-bit) per entry
	STRUCT  pcdl_ResHeight,2*PHOTOCD_RES_COUNT  ; UWORD (16-bit) per entry

   LABEL PhotoCDLib_SIZEOF

;-----------------------------------------------------------------------

	LIBINIT

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

	ENDC	; DATATYPESBASE_I
