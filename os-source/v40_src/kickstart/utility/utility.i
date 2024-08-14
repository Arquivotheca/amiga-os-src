	IFND UTILITY_UTILITY_I
UTILITY_UTILITY_I SET 1
**
**	$Id: utility.i,v 39.6 93/08/12 16:27:21 vertex Exp $
**
**	utility.library include file
**
**	(C) Copyright 1989-1992 Commodore-Amiga, Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

	IFND EXEC_TYPES_I
	INCLUDE	"exec/types.i"
	ENDC

	IFND EXEC_LIBRARIES_I
	INCLUDE	"exec/libraries.i"
	ENDC

;---------------------------------------------------------------------------

UTILITYNAME MACRO
	DC.B 'utility.library',0
	ENDM

   STRUCTURE UtilityBase,LIB_SIZE
   	UBYTE ub_Language
   	UBYTE ub_Reserved

;---------------------------------------------------------------------------

	ENDC	; UTILITY_UTILITY_I
