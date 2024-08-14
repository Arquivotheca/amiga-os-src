**
**	$Id: residenttag.asm,v 36.0 90/04/24 13:47:09 kodiak Exp $
**
**      translator residenttag
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	translator

**	Included Files

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/resident.i"

	INCLUDE	"translator_rev.i"


**	Exported Names

	XDEF	TLName
	XDEF	TLID
	XDEF	REVISION
	XDEF	VERSION


**	Imported Names

	XREF	TLInitTable
	XREF	TLEndTag


**	Resident Tag

residentTag:
		dc.w	RTC_MATCHWORD
		dc.l	residentTag
		dc.l	TLEndTag
		dc.b	RTF_AUTOINIT!RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_LIBRARY
		dc.b	40
		dc.l	TLName
		dc.l	TLID
		dc.l	TLInitTable

TLName:
		dc.b	'translator.library',0

TLID:
		VSTRING

	END
