	IFND	LIBRARIES_EXPANSIONBASE_I
LIBRARIES_EXPANSIONBASE_I	SET	1
**
**	$Id: private_base.i,v 39.0 91/10/28 16:28:01 mks Exp $
**
**	definitions for expansion library base
**
**	(C) Copyright 1987,1988,1989,1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	IFND	EXEC_TYPES_I
	INCLUDE "exec/types.i"
	ENDC	; EXEC_TYPES_I
	IFND	EXEC_LIBRARIES_I
	INCLUDE "exec/libraries.i"
	ENDC	; EXEC_LIBRARIES_I
	IFND	EXEC_INTERRUPTS_I
	INCLUDE "exec/interrupts.i"
	ENDC	; EXEC_INTERRUPTS_I
	IFND	EXEC_SEMAPHORES_I
	INCLUDE "exec/semaphores.i"
	ENDC	; EXEC_SEMAPHORES_I
	IFND	LIBRARIES_CONFIGVARS_I
	INCLUDE "libraries/configvars.i"
	ENDC	; LIBRARIES_CONFIGVARS_I


TOTALSLOTS	EQU	256

  STRUCTURE	ExpansionInt,0
	UWORD	ei_IntMask	; mask for this list
	UWORD	ei_ArrayMax	; current max valid index
	UWORD	ei_ArraySize	; allocated size
	LABEL	ei_Array	; actual data is after this
	LABEL	ExpansionInt_SIZEOF

  STRUCTURE	BootNode,LN_SIZE
	UWORD	bn_Flags
	CPTR	bn_DeviceNode
	LABEL	BootNode_SIZEOF

**
** ALL PRIVATE EXCEPT FOR eb_Mountlist!!!
**
  STRUCTURE	ExpansionBase,LIB_SIZE
	UBYTE	eb_Flags
	UBYTE	eb_pad0		; longword align
	ULONG	eb_Obsolete1
	UWORD	eb_Z3_ConfigAddr
	UWORD	eb_pad1
	STRUCT	eb_CurrentBinding,CurrentBinding_SIZEOF
	STRUCT	eb_BoardList,LH_SIZE
	STRUCT	eb_MountList,LH_SIZE	; contains struct BootNode entries
	STRUCT	eb_AllocTable,TOTALSLOTS
	STRUCT	eb_BindSemaphore,SS_SIZE
*	STRUCT	eb_Int2List,IS_SIZE
*	STRUCT	eb_Int6List,IS_SIZE
*	STRUCT	eb_Int7List,IS_SIZE
	LABEL	ExpansionBase_SIZEOF


; error codes
EE_OK		EQU 0
EE_LASTBOARD	EQU 40	; could not shut him up
EE_NOEXPANSION	EQU 41	; not enough expansion mem; board shut up
EE_NOMEMORY	EQU 42	; not enough normal memory
EE_NOBOARD	EQU 43	; no board at that address
EE_BADMEM	EQU 44	; tried to add a bad memory card

; Flags
	BITDEF	EB,CLOGGED,0	; someone could not be shutup
	BITDEF	EB,SHORTMEM,1	; ran out of expansion mem
	BITDEF	EB,BADMEM,2	; tried to add a bad memory card
	BITDEF	EB,DOSFLAG,3	; reserved for use by AmigaDOS
	BITDEF	EB,KICKBACK33,4	; reserved for use by AmigaDOS
	BITDEF	EB,KICKBACK36,5	; reserved for use by AmigaDOS
	BITDEF	EB,SILENTSTART,6 ; See public file for def.

;-----------------internal information-----------------------------------------
BusErrorVector			EQU $08
AdddressErrorVector		EQU $0C
IllegalInstructionVector	EQU $10
PrivTrapVector			EQU $20
Line1010Vector			EQU $28
Line1111Vector			EQU $2C


EXPANSION_RAM_PRIORITY	EQU 0		;Zorro II expansion memory
KICKIT_RAM_PRIORITY	EQU 5		;Kickit leftover
Z3_RAM_PRIORITY		EQU 20		;Zorro II
LOW_RAM_PRIORITY	EQU 30		;Good motherboard stuff
HIGH_RAM_PRIORITY	EQU 40		;Better CPU stuff

;------------------------------------------------------------------------------

	ENDC	; LIBRARIES_EXPANSIONBASE_I
