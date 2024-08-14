	IFND	RESOURCES_MISCPRIVATE_I
RESOURCES_MISCPRIVATE_I	SET	1
**
**      $Id: miscprivate.i,v 36.11 90/05/06 00:41:23 bryce Exp $
**
**	Private include file for use by misc.resource.
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

        IFND    EXEC_TYPES_I
        INCLUDE "exec/types.i"
        ENDC    !EXEC_TYPES_I

        IFND    EXEC_LIBRARIES_I
        INCLUDE "exec/libraries.i"
        ENDC    !EXEC_LIBRARIES_I


NUMMRTYPES      EQU 4

    STRUCTURE MiscResource,LIB_SIZE
        STRUCT  mr_AllocArray,4*NUMMRTYPES
        LABEL   mr_Sizeof


	ENDC	;RESOURCE_MISCPRIVATE_I
