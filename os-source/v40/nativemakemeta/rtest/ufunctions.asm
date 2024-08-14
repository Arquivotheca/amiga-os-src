*************************************************************************
*                                                                       *
*   ufuncs.asm -- utility.library function table
*                                                                       *
*   Copyright (C) 1985, 1989 Commodore Amiga Inc.  All rights reserved. *
*                                                                       *
*   $Id: ufunctions.asm,v 36.2 90/11/05 18:54:56 peter Exp $
*************************************************************************

   SECTION   section



	xdef	uFuncTable

	xref	Open
	xref	Close
	xref	Expunge
	xref	Null
	xref	_findTagItem
	xref	_getTagData
	xref	_packBoolTags
	xref	_nextTagItem

	xref	_filterTagChanges
	xref	_mapTags
	xref	_allocateTagItems
	xref	_cloneTagItems
	xref	_freeTagItems
	xref	_refreshTagItemClones
	xref	_tagInArray
	xref	_filterTagItems

	xref	CallHook
*	xref	CalcDate
	xref	Amiga2DateS
	xref	Date2AmigaS
	xref	CheckDateS
	xref	SMult32S
	xref	UMult32S
	xref	SDivMod32S
	xref	UDivMod32S

	xref	stricmp
	xref	strnicmp


MCCASM	    EQU     1

CMDDEF	    MACRO   * function
    IFEQ MCCASM
	    DC.W    \1-uFuncTable
    ENDC
    IFNE MCCASM
	    DC.W    \1+(*-uFuncTable)
    ENDC
	    ENDM


uFuncTable:
	dc.w	-1			; word table
	CMDDEF   Open
	CMDDEF   Close
	CMDDEF   Expunge
	CMDDEF   Null

	CMDDEF	_findTagItem
	CMDDEF	_getTagData
	CMDDEF	_packBoolTags
	CMDDEF	_nextTagItem
	CMDDEF	_filterTagChanges
	CMDDEF	_mapTags
	CMDDEF	_allocateTagItems
	CMDDEF	_cloneTagItems
	CMDDEF	_freeTagItems
	CMDDEF	_refreshTagItemClones
	CMDDEF	_tagInArray
	CMDDEF	_filterTagItems

	CMDDEF	CallHook
	CMDDEF	Null

	; date functions
	CMDDEF	Null		; we aren't going with CalcDate
	CMDDEF	Amiga2DateS
	CMDDEF	Date2AmigaS
	CMDDEF	CheckDateS

	; math functions
	CMDDEF	SMult32S
	CMDDEF	UMult32S
	CMDDEF	SDivMod32S
	CMDDEF	UDivMod32S

	; new functions in V2.04 and later
	CMDDEF	stricmp
	CMDDEF	strnicmp

	; empty functions for setpatch!
	CMDDEF	Null
	CMDDEF	Null
	CMDDEF	Null
	CMDDEF	Null

	dc.w	-1


	end
