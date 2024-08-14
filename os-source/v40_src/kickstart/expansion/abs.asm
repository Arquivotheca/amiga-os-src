*************************************************************************
*                                                                       *
* Copyright (C) 1985-1991 Commodore Amiga Inc.  All rights reserved.    *
*                                                                       *
*************************************************************************

*************************************************************************
*
* $Id: abs.asm,v 39.0 91/10/28 16:24:41 mks Exp $
*
* $Log:	abs.asm,v $
* Revision 39.0  91/10/28  16:24:41  mks
* First release of native build V39 expansion code
* 
*************************************************************************

;****** Included Files ***********************************************

	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/initializers.i'

	INCLUDE 'configregs.i'
	INCLUDE 'configvars.i'
	INCLUDE 'expansion.i'
	INCLUDE 'private_base.i'

	INCLUDE 'asmsupp.i'
	INCLUDE 'messages.i'
	LIST


;****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

*------ Data ---------------------------------------------------------

;	XDEF	InitTable
	XDEF	funcTable
	XDEF	dataTable


;****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	subSysName
;	XREF    initRoutine
	XREF    VERNUM
	XREF    REVNUM
	XREF    VERSTRING

	XLIB	FreeMem

;****** Local Definitions ********************************************



*******************************************************************************

;InitTable:
;	dc.l	ExpansionBase_SIZEOF
;	dc.l	funcTable
;	dc.l	dataTable
;	dc.l	initRoutine

dataTable:
	INITSTRUCT	2,LN_TYPE,0,(2-1)	;Hit LN_TYPE and LN_PRI
	DC.B		NT_LIBRARY		;bryce:Was NT_RESOURCE
	DC.B		-20			;bryce:No need for high pri
****	INITBYTE        LN_TYPE,NT_LIBRARY	;bryce:was NT_RESOURCE
****	INITBYTE        LN_PRI,-20		;bryce:No need for high pri
	INITWORD        LIB_REVISION,REVNUM
	IFNE		OnePointFourExec
	 INITLONG       LN_NAME,subSysName
	 INITBYTE       LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	 INITWORD       LIB_VERSION,VERNUM
	 INITLONG       LIB_IDSTRING,VERSTRING
	ENDC
	DC.W    0			;terminate list


;Build table of relative vectors
FUNCDEF     MACRO   * function
	    XREF    \1
	    ;Kludge for Metacomco assembler
	    DC.W    \1+(*-funcTable)
	    ENDM


funcTable:
	DC.W	-1	;short function vectors
	DC.W	Open-funcTable
	DC.W	Close-funcTable
	DC.W	Expunge-funcTable
	DC.W	Null-funcTable
	INCLUDE "expansion_lib.i"
	DC.W	-1	;short function vectors
;-----

Open:
	IFGE	INFOLEVEL-100
	move.l	4,a0
	move.l	$114(a0),a0
	move.l	a0,-(sp)
	move.l	LN_NAME(a0),-(sp)
        INFOMSG 900,<'expansion.library open by task %s ($%lx)'>
	addq.l	#8,sp
	ENDC

	addq.w	#1,LIB_OPENCNT(a6)
	move.l	a6,d0
	rts

Close:
	IFGE	INFOLEVEL-100
	move.l	4,a0
	move.l	$114(a0),a0
	move.l	a0,-(sp)
	move.l	LN_NAME(a0),-(sp)
        INFOMSG 900,<'expansion.library close by task %s ($%lx)'>
	addq.l	#8,sp
	ENDC

	subq.w	#1,LIB_OPENCNT(a6)
	;beq.s	Expunge
Expunge:
Null:	MOVEQ   #0,D0
	RTS


	IFD	NOTDEFINED
Expunge:
        INFOMSG 100,<'Possible expunge'>
	TSTLST2	eb_BoardList(a6),a0
	bne.s	Null		;Not empty, can't expunge!
	TSTLST2	eb_MountList(a6),a0
	bra.s	Null		;Not empty, can't expunge!
****:TEST:changed bne.s to bra.s until Exec can search ROMTags ****
        INFOMSG 20,<'No boards, no eb_Mountlist, expunge it!'>

	move.l	a6,a1
	REMOVE
	move.l	a6,a1
	moveq	#0,d0
	move.w	LIB_NEGSIZE(a6),d0
	suba.l	d0,a1
	add.w	LIB_POSSIZE(a6),d0
	move.l	ABSEXECBASE,a6
	JMPSYS	FreeMem
	ENDC


	END
