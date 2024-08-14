    IFND UTILITY_HOOKS_I
UTILITY_HOOKS_I SET 1
**
**	$Id: hooks.i,v 36.1 90/07/12 15:49:06 rsbx Exp $
**
**	callback hooks
**
**	(C) Copyright 1989,1990 Commodore-Amiga Inc.
**		All Rights Reserved
**

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND EXEC_NODES_I
    INCLUDE "exec/nodes.i"
    ENDC


; New Hook conventions
; A0 - pointer to hook itself
; A1 - pointer to parameter packed ("message")
; A2 - Hook specific address data ("object," e.g, gadget )

 STRUCTURE HOOK,MLN_SIZE
    APTR	h_Entry			; assembler entry point
    APTR	h_SubEntry		; optional HLL entry point
    APTR	h_Data			; owner specific
 LABEL		h_SIZEOF


 ENDC
