**
**	$Header: V38:src/workbench/libs/diskfont/RCS/macros.i,v 35.3 90/04/09 18:04:37 kodiak Exp $
**
**      diskfont.library assembler macros
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

*------ external definition macros -----------------------------------

XLVO	MACRO
	XREF	_LVO\1
	ENDM

*------ library dispatch macros --------------------------------------

CALLLVO		MACRO
		CALLLIB _LVO\1
		ENDM

LINKEXE		MACRO
		LINKLIB _LVO\1,dfl_SysBase(a6)
		ENDM

LINKUTL		MACRO
		LINKLIB _LVO\1,dfl_UtilityBase(a6)
		ENDM

LINKGFX		MACRO
		LINKLIB _LVO\1,dfl_GfxBase(a6)
		ENDM
