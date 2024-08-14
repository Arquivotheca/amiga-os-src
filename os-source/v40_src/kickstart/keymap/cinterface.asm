**
**      $Filename cinterface.asm $
**      $Release: 1.4 $
**      $Revision: 36.2 $
**      $Date: 89/04/14 16:25:46 $
**
**      keymap c interface
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
**	$Header: cinterface.asm,v 36.2 89/04/14 16:25:46 kodiak Exp $

	SECTION	keymap

	XDEF	_SysBase
_SysBase	EQU	4

	XREF	_Open
	XDEF	_AOpen
_AOpen:
		move.l	a6,-(a7)
		jsr	_Open
		addq.l	#4,a7
		rts

	XREF	_Close
	XDEF	_AClose
_AClose:
		move.l	a6,-(a7)
		jsr	_Close
		addq.l	#4,a7
		rts

	XREF	_Expunge
	XDEF	_AExpunge
_AExpunge:
		move.l	a6,-(a7)
		jsr	_Expunge
		addq.l	#4,a7
		rts

	XREF	_AskKeyMapDefault
	XDEF	_AAskKeyMapDefault
_AAskKeyMapDefault:
		move.l	a6,-(a7)
		jsr	_AskKeyMapDefault
		addq.l	#4,a7
		rts

	XREF	_SetKeyMapDefault
	XDEF	_ASetKeyMapDefault
_ASetKeyMapDefault:
		movem.l	a0/a6,-(a7)
		jsr	_SetKeyMapDefault
		addq.l	#8,a7
		rts
	END
