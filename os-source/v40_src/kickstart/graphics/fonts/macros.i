**
**	$Id: macros.i,v 33.3 90/04/13 11:49:35 kodiak Exp $
**
**      macros for font initialization code
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
