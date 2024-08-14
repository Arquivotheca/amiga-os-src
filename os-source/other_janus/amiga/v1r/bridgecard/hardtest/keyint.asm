;*********************************************************************
;
; keyint.asm -- keyboard interrupt test for Janus
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
;
;*********************************************************************
	    
	SECTION section,code

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "hardware/intbits.i"
	INCLUDE "janus/janusreg.i"
	LIST

	XREF	KGetChar
	XREF	KPutFmt
	XREF	KPutChar

	XREF	_Config8088

	XREF	_SysBase
	XREF	_LVOAddIntServer
	XREF	_LVODisable
	XREF	_LVOEnable
	XREF	_LVORemIntServer

	XDEF	_main

;---------------------------------------------------------------------
	SECTION section,data

intstruct	ds.b	IS_SIZE
keystring	ds.b	7
keypointer	ds.b	1
lastcode	ds.b	1
		ds.w	0
hungcount	ds.l	1

ASKEDFORKEY	EQU	-1000
HUNGTHRESH	EQU	500000
BACKSPACE	EQU	14
;---------------------------------------------------------------------
	SECTION section,code

PRINTR	MACRO
		movem.l d0/d1/a0/a1,-(a7)
		lea	MSG\@,a0
		lea	16(a7),a1
		jsr	KPutFmt
		movem.l (a7)+,d0/d1/a0/a1
	SECTION section,data
MSG\@		dc.b	\1,10,0
		ds.w	0
	SECTION section,code
	ENDM

_main:
		movem.l d2-d7/a2-a6,-(a7)
		PRINTR	<'start of KeyInt test.'>

		move.l	_SysBase,a6
		bsr	_Config8088

		move.l	d0,a2
		add.l	#IoRegOffset+IoAccessOffset,a2

		lea	intstruct,a1
		move.l	a2,IS_DATA(a1)
		move.l	#intcode,IS_CODE(a1)
		move.b	#10,LN_PRI(a1)
		moveq	#INTB_PORTS,d0
		move.l	_SysBase,a6
		jsr	_LVOAddIntServer(a6)


		move.l	#$ffffffff,keystring
		move.l	#$ffffff00,keystring+4
		move.b	#$7f,lastcode
		clr.l	hungcount

		PRINTR	<'hit a key for F1'>
		jsr	KGetChar
		
		move.b	#$ef,jio_IntEna(a2)
		move.b	#$fe,jio_Control(a2)
		move.b	#$3b,jio_KeyboardData(a2)	; f1
		move.b	#JPCKEYINT,jio_PcIntGen(a2)

		PRINTR	<'hit a key to start the test'>
		jsr	KGetChar

		move.l	a2,a0
		add.l	#ParameterOffset-IoRegOffset-IoAccessOffset,a0
		clr.w	(a0)


endlessLoop:
		;------ check here to see if key has been read
		btst	#JPCINTB_IRQ1,jio_PcIntReq(a2)
		bne.s	endlessLoop
			 
		addq.l	#1,hungcount
		beq.s	intOutput
		bmi.s	endlessLoop
		cmp.l	#HUNGTHRESH,hungcount
		blt.s	endlessLoop
		PRINTR	13
		move.l	#$ffffffff,keystring
		move.l	#$ffffff00,keystring+4
		move.b	#$7f,lastcode
		move.b	#$01,jio_KeyboardData(a2)	; ESC
		move.b	#JPCKEYINT,jio_PcIntGen(a2)
		clr.l	hungcount
		bra.s	endlessLoop

intOutput:
		moveq	#0,d0
		move.b	lastcode,d0
		bmi.s	nextCode
		moveq	#-1,d2
		or.b	#$80,d0

outputCode:
		move.b	d0,lastcode
		move.b	d0,jio_KeyboardData(a2)
		move.b	#JPCKEYINT,jio_PcIntGen(a2)
		move.l	d2,d0
		bmi	endlessLoop
		jsr	KPutChar
		bra	endlessLoop
nextCode:
		moveq	#0,d2
		lea	keystring,a0
		move.b	keypointer,d0
		bmi.s	backSp

incrementDigit:
		addq.b	#1,0(a0,d0.w)
		cmp.b	#9,0(a0,d0.w)
		bgt.s	overflow

		;------ output digit
		move.w	d0,d1
		cmp.b	#6,d0
		beq.s	endOfString
		addq.b	#1,d0
		bra.s	outChar
endOfString:
		neg.b	d0		    ; show backspace is next
outChar:
		move.b	d0,keypointer
		move.b	0(a0,d1.w),d0
		move.b	d0,d2
		add.b	#'0',d2
		move.b	codeLookup(pc,d0.w),d0
		bra.s	outputCode 

backSp: 						       
		;------ output backspace			    
		neg.b	d0		    ; show digit incr output is next

cachePointer:
		move.b	d0,keypointer
		move.b	#08,d2
		move.b	#BACKSPACE,d0
		bra.s	outputCode

overflow:
		;------ fix up this digit for later
		move.b	#-1,0(a0,d0.w)
		sub.b	#1,d0		    ; go to 10x digit
		bpl.s	cachePointer
		bra.s	incrementDigit


theEnd:
		lea	intstruct,a1
		moveq	#INTB_PORTS,d0
		move.l	_SysBase,a6
		jsr	_LVORemIntServer(a6)

		PRINTR	<'End of KeyInt test.'>
		movem.l (a7)+,d2-d7/a2-a6
		rts
		       
codeLookup:
		dc.b	11,2,3,4,5,6,7,8,9,10

gotIntStr:
		dc.b	'.%02lx.',8,8,8,8,0
gotNZJStr:
		dc.b	'.%02lx:%02lx:',8,8,8,8,8,8,8,0
gotKeyStr:
		dc.b	'.%02lx:%02lx|%02lx|',8,8,8,8,8,8,8,8,8,8,0

		ds.w	0


intcode:
		moveq	#0,d0
		move.b	jio_IntReq(a1),d0
		move.l	d0,-(a7)
		move.l	d0,-(a7)		      
		move.l	d0,-(a7) 
		move.l	a7,a1

		btst	#JINTB_ENBKB,d0
		bne.s	gotKey
		tst.b	d0
		bne.s	gotNZJ
		lea	gotIntStr(pc),a0
printInt:
		jsr	KPutFmt
		add.w	#12,a7
		moveq	#0,d0
		rts

gotKey:
		move.l	#ASKEDFORKEY,hungcount
		lea	gotKeyStr(pc),a0
		bra.s	printInt

gotNZJ:
		lea	gotNZJStr(pc),a0
		bra.s	printInt

	END