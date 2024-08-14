	IFND UTILITY_UTILITY_I
UTILITY_UTILITY_I SET 1
**
**	$Id: utility.i,v 39.3 92/09/18 11:38:36 vertex Exp $
**
**	utility.library include file
**
**	(C) Copyright 1989-1992 Commodore-Amiga, Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

UTILITYNAME MACRO
	DC.B 'utility.library',0
	ENDM

   STRUCTURE UtilityBase,LIB_SIZE
   	UBYTE ub_Language
   	UBYTE ub_Reserved

;---------------------------------------------------------------------------

	ENDC	; UTILITY_UTILITY_I
