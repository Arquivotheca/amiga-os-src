;*********************************************************************
;
; mount.asm -- mount command for jdisk device
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

	XREF	KPutFmt
	XREF	KGetChar

	XREF	_SysBase
	XREF	_LVOAddIntServer
	XREF	_LVORemIntServer
	XDEF	_main

;---------------------------------------------------------------------
	SECTION section,data

intstruct	ds.b	IS_SIZE
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
		PRINTR	<'start of SInt test.'>

		lea	intstruct,a1
		move.l	#intcode,IS_CODE(a1)
		move.b	#-1,LN_PRI(a1)
		moveq	#INTB_PORTS,d0
		move.l	_SysBase,a6
		jsr	_LVOAddIntServer(a6)

		move.l	#$980000+IoAccessOffset+IoRegOffset,a0
		move.b	#$bf,jio_IntEna(a0)
		move.b	#$fe,jio_Control(a0)

		move.b	#$ff,$998000
1$
		tas	$998000
		beq.s	2$
		move.b	#$ff,$998001
		tst.b	jio_ReleasePcReset(a0)
		bra.s	1$

2$
		clr.w	$998000
		PRINTR	<'hit any key to terminate test.'>

		jsr	KGetChar

		lea	intstruct,a1
		moveq	#INTB_PORTS,d0
		move.l	_SysBase,a6
		jsr	_LVORemIntServer(a6)

		PRINTR	<'End of SInt test.'>
		movem.l (a7)+,d2-d7/a2-a6
		rts

intcode:
		move.l	#$980000+IoAccessOffset+IoRegOffset,a0
		btst	#JINTB_SYSINT,jio_IntReq(a0)
		beq.s	iEnd
		PRINTR	<'SYSINT'>
		move.b	#JPCSENDINT,jio_PcIntGen(a0)
iEnd:
		rts

	END