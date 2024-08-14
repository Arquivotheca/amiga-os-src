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

	XREF	_SysBase
	XDEF	_main

;---------------------------------------------------------------------

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
		PRINTR	<'Release2: start.'>

		PRINTR	<'Release2: tst.b $9ffff5'>
		tst.b	$9ffff5

		PRINTR	<'Release2: end of test.'>
		movem.l (a7)+,d2-d7/a2-a6
		rts

	END