**
**	$Header: /usr.MC68010/ghostwheel/darren/V38/clipboard/RCS/adev.asm,v 36.4 90/11/02 13:59:48 darren Exp $
**
**      clipboard assembly interface
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	CODE

**	Includes

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"


**	Exports

	XDEF	AInit
	XDEF	AOpen
	XDEF	AClose
	XDEF	AExpunge
	XDEF	AExtFunc
	XDEF	ABeginIO
	XDEF	AAbortIO


**	Imports

	XREF	_CInit
	XREF	_COpen
	XREF	_CClose
	XREF	_CExpunge
	XREF	_CBeginIO
	XREF	_CAbortIO


**********************************************************************

AInit:
		movem.l	d0/a0/a6,-(a7)
		jsr	_CInit
		lea	12(a7),a7
		rts

AOpen:
		movem.l	d0/a1/a6,-(a7)
		jsr	_COpen
		lea	12(a7),a7
		rts

AClose:
		movem.l	a1/a6,-(a7)
		jsr	_CClose
		addq	#8,a7
		rts

AExpunge:
		move.l	a6,-(a7)
		jsr	_CExpunge
		addq	#4,a7
		rts

ABeginIO:
		move.l	a1,-(a7)
		jsr	_CBeginIO
		addq	#4,a7
		rts

AAbortIO:
		move.l	a1,-(a7)
		jsr	_CAbortIO
		addq	#4,a7
AExtFunc:	
		rts

	END
