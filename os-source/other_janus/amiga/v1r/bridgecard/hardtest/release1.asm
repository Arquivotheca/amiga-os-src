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
	INCLUDE "janus/janusreg.i"
	LIST

	XREF	KPutFmt
	XREF	KGetChar

	XDEF	_main

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
		PRINTR	<'Release1: Wait for 8088 signal.'>

		move.b	#$ff,$998000
1$
		tas	$998000
		beq.s	2$
		move.b	#$ff,$998001
		bra.s	1$

2$
		PRINTR	<'Release1: Got 8088 signal -- hit a key to handshake back'>

		jsr	KGetChar

		clr.b	$998001

		PRINTR	<'Release1: hit a key to release lock'>

		clr.b	$998000

		PRINTR	<'Release1: End of test.'>
		movem.l (a7)+,d2-d7/a2-a6
		rts

	END