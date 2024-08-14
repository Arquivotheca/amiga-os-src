	IFND	PREFS_PALETTE_I
PREFS_PALETTE_I	SET	1
**
**	$Id: palette.i,v 39.2 92/06/15 13:45:07 vertex Exp $
**
**	File format for palette preferences
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND INTUITION_INTUITION_I
    INCLUDE "intuition/intuition.i"
    ENDC

;---------------------------------------------------------------------------

ID_PALT equ "PALT"

   STRUCTURE PalettePrefs,0
	STRUCT pap_Reserved,4*4		; System reserved
	STRUCT pap_4ColorPens,32*2
	STRUCT pap_8ColorPens,32*2
	STRUCT pap_Colors,32*cs_SIZEOF  ; Used as full 16-bit RGB values
   LABEL PalettePrefs_SIZEOF

;---------------------------------------------------------------------------

	ENDC	; PREFS_PALETTE_I
