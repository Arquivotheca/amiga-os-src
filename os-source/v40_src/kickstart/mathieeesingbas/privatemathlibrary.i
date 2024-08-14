	IFND	LIBRARIES_MATHLIBRARY_I
LIBRARIES_MATHLIBRARY_I	SET	1
**
**	FILENAME: libraries/mathlibrary.i
**	RELEASE:  1.3
**
**
**
**	(C) Copyright 1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

	ifnd	EXEC_TYPES_I
	include "exec/types.i"
	endc

	ifnd EXEC_LIBRARIES_I
	include "exec/libraries.i"
	endc

	STRUCTURE MathIEEEBase,0
		STRUCT	MathIEEEBase_LibNode,LIB_SIZE
		UBYTE	MathIEEEBase_Flags
		UBYTE	MathIEEEBase_reserved1
		APTR	MathIEEEBase_68881	; ptr to base of 68881 io
		APTR	MathIEEEBase_SysLib
		APTR	MathIEEEBase_SegList
	;Gone	APTR	MathIEEEBase_Resource	; ptr to math resource found
		APTR	MathIEEEBase_TaskOpenLib	; hook
	;Gone	APTR	MathIEEEBase_TaskCloseLib	; hook
*	This structure may be extended in the future */
	LABEL	MathIEEEBase_SIZE

	ENDC	; LIBRARIES_MATHLIBRARY_I
