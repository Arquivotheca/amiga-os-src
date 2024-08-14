**
**	$Id: residenttag.asm,v 39.1 92/04/20 13:28:53 mks Exp $
**
**      mathffp resident tag
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

**	Included Files

	INCLUDE	"exec/types.i"

	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/strings.i"

	INCLUDE	"mathffp_rev.i"


**	Imported Names

	XREF		MFInitTable
	XREF		MFEndModule


**	Exported Names

	XDEF		MFName
	XDEF		MFID
	XDEF		VERSION
	XDEF		REVISION


**********************************************************************

residentMF:
		DC.W	RTC_MATCHWORD	; word to match on (ILLEGAL)
		DC.L	residentMF	; pointer to the above
		DC.L	MFEndModule	; address to continue scan
		DC.B	RTF_AUTOINIT	; various tag flags
		DC.B	VERSION		; release version number
		DC.B	NT_LIBRARY	; type of module (NT_mumble)
		DC.B	-120		; initialization priority
		DC.L	MFName		; pointer to node name
		DC.L	MFID		; pointer to ident string
		DC.L	MFInitTable	; pointer to init cod

MFName:
		dc.b	'mathffp.library',0
MFID:
		VSTRING

		END
