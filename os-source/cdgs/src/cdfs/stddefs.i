
LF		equ	10
NL		equ	10
CR		equ	13
TAB		equ	9
FF		equ	12

****** MACROS, CONSTANTS, and other DEFINITIONS *****

CODE		MACRO
		CSEG
		ENDM

DATA		MACRO
		DSEG
		ENDM

save		MACRO	(regs)
		movem.l	\1,-(sp)
		ENDM

restore		MACRO	(regs)
		movem.l	(sp)+,\1
		ENDM

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

execl		macro	function
		move.l	a6,-(sp)
		move.l	4,a6
		jsr	LVO.\1(A6)
		move.l	(sp)+,a6
		endm

exec		macro	function
		move.l	4,a6
		jsr	LVO.\1(A6)
		endm

calls		macro	library,function
		move.l	a6,-(sp)
		move.l	\1,a6
		jsr	LVO.\2(A6)
		move.l	(sp)+,a6
		endm

clear		macro	data-register
		moveq.l	#0,\1
		endm


SysBase     equ     4

*******	System Library Offsets
FUNCDEF		macro
LVO.\1		equ	LVOFF
LVOFF		set	LVOFF-6
		endm

LVOFF		set	-5*6

	include "V37:include/exec/exec_lib.i"


LVO.Open	equ	-5*6
LVO.Close	equ	-6*6
LVO.Read	equ	-7*6
LVO.Write	equ	-8*6
LVO.Input	equ	-9*6
LVO.Output	equ	-10*6
LVO.LoadSeg	equ	-25*6


AllocMem	macro
		move.l	#\1,d0
		move.l	#\2,d1
		exec	AllocMem
		endm

FreeMem		macro
		move.l	\1,a1
		move.l	#\2,d0
		exec	FreeMem
		endm

