**
**	$Id: macros.i,v 42.0 93/06/16 11:25:29 chrisg Exp $
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
