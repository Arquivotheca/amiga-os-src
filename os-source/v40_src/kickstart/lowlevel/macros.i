******************************************************************************
*
*	$Id: Macros.i,v 39.2 93/01/07 14:29:19 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	Macros.i,v $
* Revision 39.2  93/01/07  14:29:19  Jim2
* *** empty log message ***
* 
* Revision 39.1  92/12/17  18:12:05  Jim2
* *** empty log message ***
*
* Revision 39.0  92/12/07  16:06:40  Jim2
* Initial release prior to testing.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************





dbug	macro	r
	xref	_kprintf
	movem.l	d0/d1/a0/a1,-(a7)
	move.l \1,-(a7)
	pea	a\@(pc)
	jsr	_kprintf
	lea	8(a7),a7
	bra.s	b\@
a\@: dc.b	'%lx ',0,0
b\@:
	movem.l	(a7)+,d0/d1/a0/a1
	endm

dbugw	macro	r
	xref	_kprintf
	movem.l	d0/d1/a0/a1,-(a7)
	move.w \1,-(a7)
	pea	a\@(pc)
	jsr	_kprintf
	lea	6(a7),a7
	bra.s	b\@
a\@: dc.b	'%x ',0
b\@:
	movem.l	(a7)+,d0/d1/a0/a1
	endm

print	macro	string
	xref	KPutStr
	movem.l	a0/a1/d0/d1,-(a7)
	lea	mystring\@(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0/a1/d0/d1
	bra.s	skip\@
mystring\@:
	dc.b	\1,0
	cnop	0,2
skip\@:
	endm

WORDVAR	macro	vname
TEMP_SIZE	set	TEMP_SIZE+(TEMP_SIZE&1)	; even up
\1_w		set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+2
	endm

LONGVAR	macro	vname
TEMP_SIZE	set	TEMP_SIZE+((4-(TEMP_SIZE&3))&3)	; lword align
\1_l		set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+4
	endm

BVAR	macro	vname
\1_b	set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+1
	endm

ARRAYVAR	macro	vname,size
TEMP_SIZE	set	TEMP_SIZE+((4-(TEMP_SIZE&3))&3)	; lword align
\1	set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+\2
	endm

ALLOCLOCALS	macro	offthis
; align temp_size and sub from sp
TEMP_SIZE	set	TEMP_SIZE+((4-(TEMP_SIZE&3))&3)	; lword align
	lea	-TEMP_SIZE(\1),\1
	endm

DONELOCALS	macro	offthis
	lea	TEMP_SIZE(\1),\1
	endm
