
* $Id: actions.asm,v 1.4 91/04/24 20:51:18 peter Exp $


	include 'tokens.i'

REMOTE		EQU	1


	xref	_flags
	xref	_values
	xref	_LastNum
	xref	_CurrAddr
	xref	_DisasmAddr
	xref	_HoldNum
	xref	_FrameSize

	xref	_printf
	xref	_atoi
	xref	_getchar
	xref	_putchar
	xref	_SetValue
	xref	_GetValue
	xref	_do_decode
	xref	_FindSymbol
	xref	_SaveSym
	xref	_assNum

	xref	_TokenValue
	xref	_InputPtr
	xref	_stream
	xref	_Backup
	xref	_GetMem
	xref	_GetMemL
	xref	_PutMem
	xref	_setBrkPt
	xref	_getBrkCnt
	xref	_evalBrkPts
	xref	_freeCPU

	xdef	_test
	xdef	assign
	xdef	setCurr
	xdef	newline
	xdef	termState
	xdef	store
	xdef	execute
	xdef	breakpt
	xdef	setBrkCnt
	xdef	abortCmd
	xdef	pokeAddr
	xdef	pokeVal

	xdef	_Execute


SHOWHEX		equ	0
SHOWCODE	equ	1
TERMSTATE	equ	2

GETMEM		macro
	IFEQ	REMOTE
		move.w	(\1)+,d0
	ENDC
	IFNE	REMOTE
		move.l	\1,-(sp)
		jsr	_GetMem
		addq.l	#4,sp
		addq.l	#2,\1
		and.l	#$0FFFF,d0
	ENDC
		endm


REFRESH		macro
		move.b	#1,_flags
		endm


************************************************************************
*
*	Show A Hexword Frame
*
************************************************************************

	xdef	_ShowFrame
_ShowFrame:
		REFRESH
		rts


************************************************************************
*
*	Show A Disassembled Frame
*
************************************************************************

	xdef	_DisAsm
_DisAsm:
		movem.l	d2/d3,-(sp)
		move.l	_DisasmAddr,d3
		move.l	_FrameSize,d2
		add.l	d3,d2
		bsr	newline
da_next:
		move.l	d3,-(sp)
		jsr	_do_decode
		addq.l	#4,sp
		add.l	d0,d3
		bsr	newline
		cmp.l	d3,d2
		bgt.s	da_next

		move.l	d3,_DisasmAddr
		movem.l	(sp)+,d2/d3
ts_exit:
		rts


************************************************************************
*
*	Asssignment States
*
************************************************************************

assign:
as_next:
		bsr	newline
		move.l	_CurrAddr,d0
		bsr	hex24
		move.l	d0,a0
		GETMEM	a0
		bsr	hex16
		moveq	#'=',d0
		bsr	putchar
*		lea	StateAssign,a0
		bsr	parse
		btst	#TERMSTATE,_flags
		beq.s	as_next
		clr.b	_flags
		bsr	newline
		rts

termState:
		bset	#TERMSTATE,_flags
		rts

store:
		move.l	_CurrAddr,d0
		moveq	#1,d1
		move.l	_LastNum,d2
		bsr	setMem
		bsr	fwdOne
		rts


************************************************************************
*
*	Breakpoint States
*
************************************************************************

breakpt:
bp_next:
		bsr	newline
		move.l	_CurrAddr,d0
		bsr	hex24
		move.l	d0,-(sp)
		jsr	_getBrkCnt
		addq.l	#4,sp
		bsr	hex16
		moveq	#'^',d0
		bsr	putchar
*		lea	StateBrkPt,a0
		bsr	parse
		clr.b	_flags
		bsr	newline
		rts

execute:
		bsr	newline
		jsr	_freeCPU
		clr.b	_flags
		rts

setBrkCnt:
		move.l	_LastNum,d0
		move.l	d0,-(sp)
		jsr	_setBrkPt
		addq.l	#4,sp
		rts


************************************************************************
*
*	Frame Position Movement
*
************************************************************************

	xdef	_NextWord
	xdef	fwdOne

_NextWord:
fwdOne:
		addq.l	#2,_CurrAddr
		REFRESH
		rts


	xdef	_BackWord
	xdef	backOne

_BackWord:
backOne:
		subq.l	#2,_CurrAddr
		REFRESH
		rts


	xdef	_NextFrame
	xdef	fwdFrame

_NextFrame:
fwdFrame:
		move.l	_FrameSize,d0
		add.l	d0,_CurrAddr
		REFRESH
		rts


	xdef	_BackFrame
	xdef	backFrame

_BackFrame:
backFrame:
		move.l	_FrameSize,d0
		sub.l	d0,_CurrAddr
		REFRESH
		rts


	xdef	indirect

indirect:
		move.l	_CurrAddr,a0
		move.l	a0,(a5)+
		bsr	getmeml
		move.l	d0,_CurrAddr
		moveq.l	#-2,d0
		and.l	d0,_CurrAddr
		REFRESH
		rts


	xdef	exdirect

exdirect:
		move.l	-(a5),_CurrAddr
		REFRESH
		rts


	xdef	plusCnt

plusCnt:
		move.l	_LastNum,d0
		add.l	d0,_CurrAddr
		REFRESH
		rts


	xdef	minusCnt

minusCnt:
		move.l	_LastNum,d0
		sub.l	d0,_CurrAddr
		REFRESH
		rts

setCurr:
		move.l	_LastNum,_CurrAddr
		REFRESH
		rts


************************************************************************
*
*	Frame Sizing
*
************************************************************************

	xdef	frameSize

frameSize:
		move.l	_LastNum,d0
		move.l	d0,_FrameSize
		REFRESH
		rts


************************************************************************
*
*	Memory Fetch
*
************************************************************************

getmeml:
	IFEQ	REMOTE
		moveq	#0,d0
		move.w	(a0),d0
	ENDC
	IFNE	REMOTE
		move.l	a0,-(sp)
		jsr	_GetMemL
		addq.l	#4,sp
	ENDC
		rts


************************************************************************
*
*	Calculator
*
************************************************************************

calc:
		rts


************************************************************************
*
*	Show Memory
*
************************************************************************
*
*	showMem (from, cnt)
*		 d0    d1
*
************************************************************************

showMem:
		movem.l	d0/d2/d3/a2/a3,-(sp)
		link	a6,#-34

		bsr	newline
		move.l	d0,a2
		move.l	d1,d2
		beq.s	shm_exit
sl0:
		move.l	a2,d0
		bsr	hex24
		moveq	#7,d3
		lea	-32(a6),a3
sl1:
		GETMEM	a2
		bsr	hex16
		move.l	d0,-(sp)
		lsr	#8,d0
		bsr	unctrl
		move.w	d0,(a3)+
		move.l	(sp)+,d0
		bsr	unctrl
		move.w	d0,(a3)+

		subq.l	#2,d2
		ble.s   shm
		dbf	d3,sl1
		clr.w	(a3)+
		lea	-32(a6),a0
		bsr	charline

		bra.s	sl0
shm:
		clr.w	(a3)+
		lea	-32(a6),a0
		bsr	charline

shm_exit:
		unlk	a6
		movem.l	(sp)+,d0/d2/d3/a2/a3
		rts


************************************************************************
*
*	Set Memory
*
************************************************************************
*
*	setMem ( first, count, value )
*	         d0     d1     d2
*
************************************************************************

setMem:
		move.l	a2,-(sp)
		move.l	d0,a2
		bra.s	sm1
sm0:
	IFEQ	REMOTE
		move.w	d2,(a2)+
	ENDC
	IFNE	REMOTE
		move.l	d1,-(sp)
		move.l	d2,-(sp)
		move.l	a2,-(sp)
		jsr	_PutMem
		addq.l	#8,sp
		move.l	(sp)+,d1
		addq.l	#2,a2
	ENDC
sm1:
		dbf	d1,sm0

		move.l	(sp)+,a2
		rts


************************************************************************
*
*	Poke Memory (no pre-read)
*
************************************************************************

pokeAddr:
		move.l	_LastNum,_HoldNum
		rts	
pokeVal:
		move.l	_HoldNum,d0
		moveq	#1,d1
		move.l	_LastNum,d2
		bsr	setMem
		clr.b	_flags
		bsr	newline
		rts


************************************************************************
*
*	Invalid Operation
*
************************************************************************

	xdef	il
il:
		move.l	_InputPtr,d4
		sub.l	#_stream,d4
		and.w	#$07F,d4
*		bsr	newline
il_space:	bsr	spc
		dbf	d4,il_space

		move.l	d3,-(sp)
		pea	ilm
		jsr	_printf
		addq.l	#8,sp
		move.l	a6,sp
		bra	errorExit

abortCmd:
		move.l	a6,sp
		bsr	newline
		bra	test1

spc:
		pea	spcStr
		jsr	_printf
		addq.l	#4,sp
		rts
		


************************************************************************
*
*	Hex Number Printing
*
************************************************************************

hex16:
		move.l	d0,-(sp)
		clr.w	(sp)
		pea	fh16
		bra.s	phex
hex24:		move.l	d0,-(sp)
		clr.b	(sp)
		pea	fh24
		bra.s	phex
hex32:		move.l	d0,-(sp)
		pea	fh32
phex:		jsr	_printf
		addq.l	#4,sp
		move.l	(sp)+,d0
		rts


************************************************************************
*
*	Various Sundry Functions
*
************************************************************************

FindSymbol:
		move.l	a0,-(sp)
		jsr	_FindSymbol
		addq.l	#4,sp
		rts

newline:
		movem.l	d0-d1/a0-a1,-(sp)
		pea	nlmsg
		jsr	_printf
		addq.l	#4,sp
		movem.l	(sp)+,d0-d1/a0-a1
		rts
prompt:
		pea	pmpt
		jsr	_printf
		addq.l	#4,sp
		rts
charline:
		pea	(a0)
		pea	cln
		jsr	_printf
		addq.l	#8,sp
		rts

sameline:
		moveq	#13,d0

putchar:
		move.l	d0,-(sp)
		jsr	_putchar
		addq.l	#4,sp
		rts

unctrl:
		move.l	d2,-(sp)
		move.w	#('.'<<8)!'.',d2
		tst.b	d0
		beq.s	uc_done
		btst	#7,d0
		bne.s	uc_done
		move.w	#$2000,d2
		move.b	d0,d1
		and.b	#$E0,d1
		bne.s	uc_norm
		move.w	#('^'<<8),d2
		or.b	#$40,d0
uc_norm:	move.b	d0,d2
uc_done:	move.l	d2,d0
		move.l	(sp)+,d2
		rts		


************************************************************************
*
*	State Machine Parser
*
************************************************************************

parse:
		movem.l	d2/d3/a2,-(sp)
		move.l	a0,a2
		bra.s	commence
next:		addq.l	#4,a2
match:		move.w	(a2)+,d2
		beq.s	conclude
		tst.b	d2
		beq.s	default
		cmp.b	d2,d3
		bne.s	next
default:	move.l	(a2)+,d1
		beq.s	noAction
		move.l	d1,a1
		jsr	(a1)
noAction:	lsr	#8,d2
		beq.s	conclude
		lea	0(a2,d2.w),a2
commence:	
*jsr	NextToken
		move.l	d0,d3
*		bsr	hex16
		bra.s	match
conclude:
		movem.l	(sp)+,d2/d3/a2
		rts


************************************************************************
*
*	Main Loop
*
************************************************************************

_test:
		movem.l	a5/a6,-(sp)

		move.l	sp,a6
		lea	_values,a5
test1:
		bsr	prompt
*		bsr	topState
		jsr	_evalBrkPts
		bra	test1
errorExit:
		movem.l	(sp)+,a5/a6
		rts	

_Execute:
		movem.l	a5/a6,-(sp)
		move.l	sp,a6
		lea	_values,a5
		move.b	#(1<<SHOWHEX),_flags	* !!! setable default
		clr.b	_flags
		move.l	12(sp),a0
		move.l  16(sp),-(sp)		; argStr parameter
		jsr	(a0)
		addq.l	#4,sp

		tst.b	_flags
		beq.s	dhd_exit

*		------- display hex dump:
		move.l	_CurrAddr,d0
		move.l	d0,_DisasmAddr
		move.l	_FrameSize,d1
		bsr	showMem
dhd_exit:
		jsr	_evalBrkPts
		movem.l	(sp)+,a5/a6
		rts	


************************************************************************

pmpt		dc.b	': ',0

fh16		dc.b	'%04x ',0
fh24		dc.b	'%06x ',0
fh32		dc.b	'%08x ',0

cln		dc.b	'%s',10,0
ilm		dc.b	'^ wrong',10,0

spcStr		dc.b	32,0
nlmsg		dc.b	10,0
		ds.w	0


	END
