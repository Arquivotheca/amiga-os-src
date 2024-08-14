**
**	$Id: residenttag.asm,v 36.5 90/05/16 11:35:07 kodiak Exp $
**
**      console resident tag
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"
	INCLUDE	"console_rev.i"

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/resident.i"


**	Exports

	XDEF	CDName
	XDEF	VERSION
	XDEF	REVISION


**	Imports

	XREF	CDFuncInit
	XREF	CDStructInit
	XREF	CDInit
	XREF	EndModule


**********************************************************************

residentCD:
		dc.w	RTC_MATCHWORD
		dc.l	residentCD
		dc.l	EndModule
		dc.b	RTF_AUTOINIT!RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_DEVICE
		dc.b	5
		dc.l	CDName
		dc.l	identCD
		dc.l	autoTable

autoTable:
		dc.l	cd_SIZEOF
		dc.l	CDFuncInit
		dc.l	CDStructInit
		dc.l	CDInit

CDName:
		dc.b	'console.device',0
identCD:
		VSTRING

	END
