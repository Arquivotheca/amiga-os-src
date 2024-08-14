
************************************************************************
***
***  Standard Defines
***
************************************************************************

SysBase		equ	4

LF		equ	10
NL		equ	10
CR		equ	13
TAB		equ	9
FF		equ	12

************************************************************************
***
***  Standard Macros
***
************************************************************************

clear		macro	data-register
		moveq.l	#0,\1
		endm

save		macro	(regs)
		movem.l	\1,-(sp)
		endm

restore		macro	(regs)
		movem.l	(sp)+,\1
		endm

push		macro	reg
		move.l	\1,-(sp)
		endm

pop		macro	reg
		move.l	(sp)+,\1
		endm

call		macro	library,function
		move.l	\1,a6
		jsr	LVO.\2(A6)
		endm

call2		macro	function
		jsr	LVO.\1(A6)
		endm

exec		macro	function
		move.l	a6,-(sp)
		move.l	4,a6
		jsr	LVO.\1(A6)
		move.l	(sp)+,a6
		endm

calls		macro	library,function
		move.l	a6,-(sp)
		move.l	\1,a6
		jsr	LVO.\2(A6)
		move.l	(sp)+,a6
		endm

FUNCTION	macro
		XDEF	\1
\1:
		endm

************************************************************************
***
***  Library Vector Offsets (LVO's)
***
************************************************************************

FUNCDEF		macro
LVO.\1		equ	LVOFF
LVOFF		set	LVOFF-6
		endm
LVOFF		set	-5*6
	include "include:exec/exec_lib.i"

LVO.FindConfigDev equ -72

