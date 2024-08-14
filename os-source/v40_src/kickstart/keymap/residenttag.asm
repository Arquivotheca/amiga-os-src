**
**	$Id: residenttag.asm,v 36.4 90/04/13 11:27:51 kodiak Exp $
**
**      keymap residenttag
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	keymap

**	Included Files

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/resident.i"

	INCLUDE	"keymap_rev.i"


**	Exported Names

	XDEF	KLName
	XDEF	KLID
	XDEF	KRName
	XDEF	REVISION
	XDEF	VERSION


**	Imported Names

	XREF	KLInitTable
	XREF	KLEndTag


**	Resident Tag

residentTag:
		dc.w	RTC_MATCHWORD
		dc.l	residentTag
		dc.l	KLEndTag
		dc.b	RTF_AUTOINIT!RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_LIBRARY
		dc.b	40
		dc.l	KLName
		dc.l	KLID
		dc.l	KLInitTable

KLName:
		dc.b	'keymap.library',0
KRName:
		dc.b	'keymap.resource',0

KLID:
		VSTRING

	END
