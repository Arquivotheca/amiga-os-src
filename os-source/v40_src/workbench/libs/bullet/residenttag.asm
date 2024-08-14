**
**	$Id: residenttag.asm,v 5.0 91/02/26 10:53:48 kodiak Exp $
**
**      bullet residenttag
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
**
	SECTION	bullet

**	Included Files

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/resident.i"

	INCLUDE	"bullet_rev.i"


**	Exported Names

	XDEF	BLName
	XDEF	BLID
	XDEF	REVISION
	XDEF	VERSION


**	Imported Names

	XREF	BLInitTable
	XREF	BLEndTag


**	Resident Tag

residentTag:
		dc.w	RTC_MATCHWORD
		dc.l	residentTag
		dc.l	BLEndTag
		dc.b	RTF_AUTOINIT!RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_LIBRARY
		dc.b	-120
		dc.l	BLName
		dc.l	BLID
		dc.l	BLInitTable

BLName:
		dc.b	'bullet.library',0

BLID:
		VSTRING

	END
