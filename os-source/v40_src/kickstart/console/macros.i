**
**	$Id: macros.i,v 36.6 90/04/20 09:45:31 kodiak Exp $
**
**      macros for console device
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	IFND	EXEC_LIBRARIES_I
	INCLUDE	"exec/libraries.i"
	ENDC


*------ extended XREF	  declarations -------------------------------
XLVO	MACRO
	XREF	_LVO\1
	ENDM

*------ library dispatch macros --------------------------------------

CALLLVO		MACRO   * &offset
		CALLLIB _LVO\1
		ENDM

LINKEXE		MACRO   * &offset
		LINKLIB _LVO\1,cd_ExecLib(a6)
		ENDM

LINKKEY		MACRO   * &offset
		LINKLIB _LVO\1,cd_KeymapLib(a6)
		ENDM

LINKGFX		MACRO   * &offset
		LINKLIB _LVO\1,cd_GraphicsLib(a6)
		ENDM

LINKINT		MACRO   * &offset
		LINKLIB _LVO\1,cd_IntuitionLib(a6)
		ENDM

LINKLAY		MACRO   * &offset
		LINKLIB _LVO\1,cd_LayersLib(a6)
		ENDM
