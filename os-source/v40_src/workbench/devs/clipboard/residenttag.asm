**
**	$Header: /usr.MC68010/ghostwheel/darren/V38/clipboard/RCS/residenttag.asm,v 36.1 90/11/02 14:02:45 darren Exp $
**
**      clipboard resident tag
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	CODE

**	Includes

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/initializers.i"

	INCLUDE	"clipboard_rev.i"


**	Imports

	XREF	AInit
	XREF	AOpen
	XREF	AClose
	XREF	AExpunge
	XREF	AExtFunc
	XREF	ABeginIO
	XREF	AAbortIO


**********************************************************************
		;--	if executing here, then this isn't ramlib calling
		moveq	#-1,d0
		rts

residentTag:
		dc.w	RTC_MATCHWORD
		dc.l	residentTag
		dc.l	localEnd
		dc.b	RTF_AUTOINIT
		dc.b	VERSION
		dc.b	NT_DEVICE
		dc.b	-120
		dc.l	name
		dc.l	ident
		dc.l	autoTable

autoTable:
		dc.l	LIB_SIZE
		dc.l	funcInit
		dc.l	structInit
		dc.l	AInit

funcInit:
		dc.l	AOpen
		dc.l	AClose
		dc.l	AExpunge
		dc.l	AExtFunc
		dc.l	ABeginIO
		dc.l	AAbortIO
		dc.l	-1

structInit:
	    INITBYTE	LN_TYPE,NT_DEVICE
	    INITLONG	LN_NAME,name
	    INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	    INITWORD    LIB_VERSION,VERSION
	    INITWORD    LIB_REVISION,REVISION
	    INITLONG	LIB_IDSTRING,ident
		dc.w	0

name:
		dc.b	'clipboard.device',0

ident:
		VSTRING

localEnd:

	END
