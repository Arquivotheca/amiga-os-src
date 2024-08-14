**
**	$Id: macros.i,v 39.0 91/08/21 17:36:28 chrisg Exp $
**
**      macros for graphics text code
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

*------ library dispatch macros --------------------------------------

XLVO	MACRO
	XREF	_LVO\1
	ENDM

CALLLVO	MACRO
		CALLLIB _LVO\1
	ENDM

LINKEXE	MACRO
		LINKLIB _LVO\1,gb_ExecBase(a6)
	ENDM

LINKUTL	MACRO
		LINKLIB _LVO\1,gb_UtilityBase(a6)
	ENDM
