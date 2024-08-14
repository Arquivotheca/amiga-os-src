**
**	$Id: ramlib.i,v 36.6 92/05/27 15:42:43 mks Exp $
**
**      ramlib private includes
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

**	Common Imports

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/alerts.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/errors.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"libraries/dos.i"
	INCLUDE	"libraries/dosextens.i"

**	Common Macros

XLVO	MACRO
	XREF	_LVO\1
	ENDM

CALLLVO	MACRO
		jsr	_LVO\1(a6)
	ENDM

**	Common Data

ABSEXECBASE	EQU	4


;
;   module data, pointed to by ex_RamLibPrivate in ExecBase
;
RL_NUMENTRIES	EQU	6

 STRUCTURE  RamLib,0
    ULONG   rl_DosBase			; (must be zero offset)
    STRUCT  rl_MemHandler,IS_SIZE	; EXEC memory handler...
    STRUCT  rl_OriginalExecJmp,RL_NUMENTRIES*6
    STRUCT  rl_LoadPort,MP_SIZE
    LABEL   RamLib_SIZEOF


;
;   stack variables during Open module
;
 STRUCTURE OpenParms,0
    STRUCT  op_SR_LMMsg,MN_SIZE
    STRUCT  op_LMPort,MP_SIZE		; reply port for LMMsg

    APTR    op_ExecList			; either DeviceList or LibList

    APTR    op_ReqName			; as presented in OpenXxxx argument
    APTR    op_OpenName			; as would appear on exec list
    STRUCT  op_MergedName,263		; used to merge DEVS:/LIBS: w/ name
    UBYTE   op_OpenLibFlag		; set for OpenLibrary
    APTR    op_LoadName			; as passed to dos LoadSeg

    BPTR    op_CurrentDir		; from invoking process
    BPTR    op_HomeDir			;
    ULONG   op_Segment
    APTR    op_WindowPtr

    LABEL   OpenParms_SIZEOF


;   the registers preserved during Open module
OR_SAVEREGS	REG	d0/d1/d2/a1/a2/a4/a5/a6
OR_RESTOREREGS	REG	d2/a1/a2/a4/a5/a6

or_D0		EQU	 0
or_D1		EQU	 4
or_A1		EQU	12		; offset from stack after preservation
