**
**	buserror - "see no evil, hear no evil, say no evil, no!"
**
**	Try to ignore or prevent harm from any illegal memory access.
**

*************************************************************************
*									*
*   Copyright 1990 Commodore-Amiga, Inc.				*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

		INCLUDE "exec/types.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/resident.i"
		INCLUDE "exec/memory.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/execbase.i"
		INCLUDE "libraries/dosextens.i"

DEBUG		EQU	0
DEBUG_DETAIL	SET	500


BusErrorVector	EQU	$08
IllegalVector	EQU	$10

ABSEXECBASE	EQU	4

		XREF	_LVOSupervisor
		XREF	_LVOFindTask


		move.l	ABSEXECBASE,a6
		btst.b	#AFB_68020,AttnFlags+1(a6)  ;>=68020 includes cache
		beq.s	not_020

		bsr	_GetVBR
		move.l	#_NewBusError,BusErrorVector(a0)

		suba.l	a1,a1
		jsr	_LVOFindTask(a6)
		move.l	d0,a0
		move.l	pr_CLI(a0),a0
		add.l	a0,a0
		add.l	a0,a0
		move.l	a0,d0
		beq.s	not_cli
		clr.l	cli_Module(a0)
not_020:
not_cli:	moveq	#0,d0
		rts




beer_residentTag:
		DC.W	RTC_MATCHWORD
		DC.L	beer_residentTag
		DC.L	beer_EndMarker
		DC.B	RTF_COLDSTART
		DC.B	36		; version
		DC.B	0		; no type
		DC.B	115		; priority
		DC.L	IDString
		DC.L	IDString
		DC.L	beer_setvector


;
;   Overwrite the Exec default bus error handler with the slut version.
;
	       ;[A6-ExecBase]
beer_setvector: bsr	_GetVBR
		move.l	#_NewBusError,BusErrorVector(a0)
		rts


;
;   A6 - ExecBase
;
_GetVBR:	move.l	a5,-(sp)
		lea.l	getvbr(pc),a5
		JSR	_LVOSupervisor(a6)
		move.l	(sp)+,a5
		rts
getvbr: 	dc.w	$4e7a,$8801		; movec.l   vbr,a0
		rte









;
;   "Easy" Bus error handler.
;
_NewBusError:
		IFNE	DEBUG
		bchg.b	#1,$bfe001
		move.l	ABSEXECBASE,a6
		movem.l d0/d1/a0/a1,-(sp)
		lea.l	$02+16(sp),a0           ;pc
		lea.l	$10+16(sp),a1           ;fault address
		PRINTF	10,<'Bus error- pc %lx, fa %lx'>,a0,a1
		movem.l (sp)+,d0/d1/a0/a1
		ENDC
		bclr.b	#8-8,$a+0(sp)
		beq.s	be_NonData	;Not a data cycle fault...
		btst.b	#6-0,$a+1(sp)
		beq.s	be_Ignore	;Ignore writes
		clr.l	$2c(sp)         ;Nicest read data possilbe
be_Ignore:	rte			;(exit 1)


;
;   Not a data fault?  Too bad, we'll have to crash you.  The illegal
;   instruction vector points to the same place as bus error used to.
;
be_NonData:	movem.l a1/a0,-(sp)         ;stack A0 lowest, A1 placeholder
		dc.w	$4e7a,$8801	    ;movec.l vbr,a0
		move.l	IllegalVector(a0),4(sp)
		move.l	(sp)+,a0
		rts			    ;rts to value we left on stack


;IDString:	dc.b	'beer drinker',10,0
IDString:	dc.b	'_The Permissor_',10,0
		CNOP	0,4

beer_EndMarker:


	END
