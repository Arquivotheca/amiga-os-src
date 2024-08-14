**********************************************************************
*
*			---------------
*   macros.i		RAWINPUT MODULE		misc macros
*			---------------
*
*   Copyright 1985, 1987 Commodore-Amiga Inc.
*
*   Source Control	$Locker:  $
*
**********************************************************************

*------ external definition macros -----------------------------------

XREF_EXE	MACRO
	XREF		_LVO\1
		ENDM

XREF_CIA	MACRO
	XREF		_LVO\1
		ENDM

XREF_PGR	MACRO
	XREF		_LVO\1
		ENDM

*------ library dispatch macros --------------------------------------

CALLEXE		MACRO
		CALLLIB _LVO\1
		ENDM

LINKEXE		MACRO
		LINKLIB _LVO\1,dd_ExecBase(a6)
		ENDM

LINKCIA		MACRO
		LINKLIB _LVO\1,kd_CIAAResource(A6)
		ENDM

LINKPGR		MACRO
		LINKLIB _LVO\1,gd_PotgoResource(A6)
		ENDM
