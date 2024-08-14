**********************************************************************
*
* Init.asm
*
* Copyright 1991 Raymond S. Brand, All rights reserved.
*
* 19910210
*
* $Id$
*
**********************************************************************

	INCLUDE	"internal.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"netbuff_rev.i"


	SECTION	NetBuff

	XDEF	Init

	XSYS	MakeLibrary
	XSYS	AddLibrary

	XREF	Open
	XREF	Close
	XREF	Expunge
	XREF	Null

	XREF	IntAllocSegments
	XREF	AllocSegments
	XREF	IntFreeSegments
	XREF	FreeSegments
	XREF	SplitNetBuff
	XREF	TrimNetBuff
	XREF	CopyToNetBuff
	XREF	CopyFromNetBuff
	XREF	CopyNetBuff
	XREF	CompactNetBuff
	XREF	ReadyNetBuff
	XREF	IsContiguous
	XREF	NetBuffAppend
	XREF	PrependNetBuff
	XREF	ReclaimSegments

	XREF	EndCode


	moveq.l	#-1,d0
	rts

initDescrip:
			     	;STRUCTURE RT,0
	dc.w	RTC_MATCHWORD	; UWORD RT_MATCHWORD
	dc.l	initDescrip	; APTR  RT_MATCHTAG
	dc.l	EndCode		; APTR  RT_ENDSKIP
	dc.b	0		; UBYTE RT_FLAGS
	dc.b	VERSION		; UBYTE RT_VERSION
	dc.b	NT_LIBRARY	; UBYTE RT_TYPE
	dc.b	50		; BYTE  RT_PRI
	dc.l	NetBuffName	; APTR  RT_NAME
	dc.l	VERSTRING 	; APTR  RT_IDSTRING
	dc.l	Init		; APTR  RT_INIT
				; LABEL RT_SIZE


NetBuffName:
	NETBUFFNAME

VERSTRING:
	VERTAG

VERNUM:	EQU	VERSION

REVNUM:	EQU	REVISION

	ds.w	0
devFuncInit:
	dc.l	Open			; - 6	Open
	dc.l	Close			; - C	Close
	dc.l	Expunge			; -12	Expunge
	dc.l	Null			; -18	reserved
	dc.l	IntAllocSegments	; -1E	
	dc.l	AllocSegments		; -24   
	dc.l	IntFreeSegments		; -2A
	dc.l	FreeSegments		; -30
	dc.l	SplitNetBuff		; -36
	dc.l	TrimNetBuff		; -3C
	dc.l	CopyToNetBuff		; -42
	dc.l	CopyFromNetBuff		; -48
	dc.l	CopyNetBuff		; -4E
	dc.l	CompactNetBuff		; -54
	dc.l	ReadyNetBuff		; -5A
	dc.l	IsContiguous		; -60
	dc.l	NetBuffAppend		; -66
	dc.l	PrependNetBuff		; -6c
	dc.l	ReclaimSegments		; -72

	dc.l	-1			; end of table


*****i* netbuff.library/Init() ***************************************
*
*   NAME
*	Init -- Initialize NetBuff library from nothing.
*
*   SYNOPSIS
*	Init( SegList )
*
*	void Init( void * );
*
*   FUNCTION
*	This routine initializes the NetBuff library from nothing.
*
*   INPUTS
*	SegList --      Pointer to the SegList for the library code.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	?????	scratch	munged
* D1 --	?????	scratch	munged
* D2 --	?????	scratch	restored
* A0 --	seglist	scratch	munged
* A1 -- ?????	scratch	munged
* A2 --	?????	library	restored
* A3 --	?????	scratch	restored
* A5 --	?????	frame	restored
* A6 --	exec	exec	exec
*

Init:
*!* debug
*	movem.l	d0-d7/a0-a6,-(sp)
*	CALLSYS	Debug
*	movem.l	(sp)+,d0-d7/a0-a6
*!* debug
*!* debug
	ERRMSG	" "
	ERRMSG	"Init"
*!* debug
	movem.l	a2/a3/a5/d2,-(sp)
	move.l	sp,a5
	move.l	a0,a3

	lea	devFuncInit,a0
	lea	NBLinitStruct,a1
	suba.l	a2,a2
	move.l	#NBL_SIZE,d0
	CALLSYS	MakeLibrary		; make us a playpen

	tst.l	d0
	beq	Init_End

	move.l	d0,a2			; store some important things
	move.l	a6,NBL_SYSLIB(a2)
	move.l	a3,NBL_SEGLIST(a2)

	lea	NBL_FREEPOOL(a2),a0	; clean the pool before use
	NEWLIST	a0

*!* debug
*	ERRMSG	"AddLibrary"
*!* debug

	move.l	a2,a1
	CALLSYS	AddLibrary		; it's official now

*!* debug
	ERRMSG	"InitDone"
*!* debug

Init_End:
	move.l	a5,sp
	movem.l	(sp)+,a2/a3/a5/d2
	rts


NBLinitStruct:
	INITBYTE	LN_TYPE,NT_LIBRARY
	INITLONG	LN_NAME,NetBuffName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERNUM
	INITWORD	LIB_REVISION,REVNUM
	INITLONG	LIB_IDSTRING,VERSTRING

	INITLONG	NBL_DEFERED,0

	dc.l		0


	END
