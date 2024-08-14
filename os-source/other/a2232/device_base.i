**
**	$Header: Work:src/cerial/RCS/device_base.i,v 1.1 91/08/09 21:06:40 bryce Exp $
**
**	Generic device driver: include file (base definition, flags)
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

;---------- Includes --------------------------------------------------------
	IFND	PREASSEMBLED_INCLUDES
	IFND	EXEC_TYPES_I
	INCLUDE "exec/types.i"
	ENDC
	IFND	EXEC_LISTS_I
	INCLUDE "exec/lists.i"
	ENDC
	IFND	EXEC_LIBRARIES_I
	INCLUDE "exec/libraries.i"
	ENDC
	ENDC

	INCLUDE "device_special.i"


;---------- Device Base -----------------------------------------------------
;
; This is the device's "library" base structure, available at a forward
; offset from the device pointer.  Below here is the jump table.  For this
; device the jumps will be Open, Close, Expunge, Null, BeginIO and AbortIO.
; All hardware-specific defines are in the unit structure for each available
; unit.
;
; The md_UnitArray may either be an array of pointers to unit structures,
; or the actual Unit structures.
;
  STRUCTURE MultiDev,LIB_SIZE
    UWORD   md_UnitSize     ;Size of each memory of unit array + align base
    ULONG   md_NumUnits     ;Number of supported units
    APTR    md_UnitArray    ;Pointer to array of unit structures or NULL
    APTR    md_SegList	    ;SegList passed to us at startup
    APTR    md_SysBase	    ;Copy of location 4 (faster than move.l 4,a6)
    SPECIAL
    LABEL   MultiDev_SIZE
