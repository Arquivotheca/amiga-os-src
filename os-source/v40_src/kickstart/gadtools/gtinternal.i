	IFND GTINTERNAL_I
GTINTERNAL_I	SET	1

*	gtinternal.i
*
*	$Id: gtinternal.i,v 39.2 92/05/29 15:37:02 vertex Exp $
*
*	Internal structures and definitions for Gadget Toolkit.
*
*	Contains a subset of stuff from gtinternal.h, placed here
*	as needed.
*

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "utility/hooks.i"

;-----------------------------------------------------------------------

   STRUCTURE GadToolsLib,LIB_SIZE
	UWORD	gtb_Pad
	APTR	gtb_SysBase
	APTR	gtb_UtilityBase
	APTR	gtb_GfxBase
	APTR	gtb_IntuitionBase
	APTR	gtb_LayersBase
; cached match function pointers
; ORDER MATTERS: MATCHES UTILITY LIBRARY ORDERING
	APTR	gtb_SMult32
	APTR	gtb_UMult32
	APTR	gtb_SDivMod32
	APTR	gtb_UDivMod32
	APTR	gtb_GTButtonIClass
	STRUCT	gtb_PaletteGHook,h_SIZEOF
	LABEL	GadToolsLib_SIZEOF

;-----------------------------------------------------------------------

	ENDC	; GTINTERNAL_I
