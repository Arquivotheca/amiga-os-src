**
**	$Id:
**
**	PrepCard start child code
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

	INCLUDE	"exec/types.i"

	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/tasks.i"

	XDEF	_StartChild
	XREF	_PrepCardTask

ABSEXECBASE	EQU	4

_StartChild:

		movem.l	a2/a3/a6,-(sp)

		move.l	ABSEXECBASE,a6

		lea	parentname(pc),a1
		JSRLIB	FindPort

		tst.l	d0
		beq.s	start_fail

		move.l	d0,a2

		lea	childname(pc),a1
		JSRLIB	CreateMsgPort

		move.l	d0,a3

		jsr	_PrepCardTask

start_fail:

		movem.l	(sp)+,a2/a3/a6
		rts

parentname:
		dc.b	'PrepCard.Parent',0
childname:
		dc.b	'PrepCard.Child',0

		END		