**
**	$Header: V38:src/workbench/libs/diskfont/RCS/residenttag.asm,v 36.4 90/04/09 18:04:53 kodiak Exp $
**
**      diskfont.library resident tag
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		diskfont

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/resident.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"diskfont_rev.i"


*------ Imported Names -----------------------------------------------

	XREF		DFInit
	XREF		DFEndModule


*------ Exported Names -----------------------------------------------

	XDEF		DFName
	XDEF		VERSION
	XDEF		REVISION


**********************************************************************
		moveq	#-1,d0
		rts

residentDF:
		dc.w	RTC_MATCHWORD
		dc.l	residentDF
		dc.l	endTag
		dc.b	RTF_AUTOINIT+RTF_AFTERDOS
		dc.b	VERSION
		dc.b	NT_LIBRARY
		dc.b	-120
		dc.l	DFName
		dc.l	identDF
		dc.l	DFInit

DFName:
		dc.b	'diskfont.library',0

identDF:
		VSTRING

endTag:

	END
