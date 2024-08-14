	IFND	LIBRARIES_EXPANSION_I
LIBRARIES_EXPANSION_I	SET	1
**
**      $Id: expansion.i,v 39.0 91/10/28 16:27:53 mks Exp $
**
**	External definitions for expansion.library
**
**      (C) Copyright 1989,1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	IFND	EXEC_TYPES_I
	INCLUDE	"exec/types.i"
	ENDC	;EXEC_TYPES_I



EXPANSIONNAME	MACRO
		dc.b	'expansion.library',0
		ENDM


;flag for the AddDosNode() call
	BITDEF	ADN,STARTPROC,0

	ENDC	;LIBRARIES_EXPANSION_I
