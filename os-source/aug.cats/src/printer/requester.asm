**
**	$Id: requester.asm,v 1.1 91/07/15 17:03:48 darren Exp $
**
**      Localized printer.device requester
**
**      (C) Copyright 1991 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/strings.i"
	INCLUDE		"exec/macros.i"
	INCLUDE		"exec/libraries.i"

	INCLUDE		"libraries/locale.i"

	INCLUDE		"intuition/intuition.i"

	INCLUDE		"macros.i"
	INCLUDE		"prtbase.i"
	INCLUDE		"printer.i"

*------ Imported Names -----------------------------------------------

	XREF_EXE	OpenLibrary
	XREF_EXE	CloseLibrary

	XREF_ITU	EasyRequestArgs

	XREF		_SysBase
	XREF		_IntuitionBase

*------ Exported Functions -------------------------------------------

	XDEF		_Localize
	XDEF		_CloseLocale
	XDEF		_TroubleNotify

*------ Exported storage ---------------------------------------------

*------ Equates ------------------------------------------------------

LOCALEV			EQU	38	; minimum version released

** Arg - amiga.lib needs to be remade with the locale LVO's!

_LVOCloseCatalog	EQU	-36
_LVOOpenCatalogA	EQU	-150
_LVOGetCatalogStr	EQU	-72

*------ Default strings/table from catalog generator -----------------

	SECTION		printer,DATA

_LocaleBase:		dc.l	0
_Catalog:		dc.l	0


PRINTER_DEVICE		EQU	1		;printer.device strings only
STRINGARRAY		EQU	1		;include strings

	INCLUDE		"localestr/devs.i"


MSG_COUNT		EQU	MSG_PRI_DEV_MAYBEPAPER-MSG_PRI_DEV_TROUBLE+1

	IFNE		MSG_COUNT-6
	FAIL		"Expected 6 enumerated equates in catalog, recode!"
	ENDC

	IFNE		MSG_PRI_DEV_UNKNOWN-MSG_PRI_DEV_OFFLINE-2
	FAIL		"Error message UNKNOWN error not equal 2, recode!"
	ENDC

			ds.w	0		;align

	;-- ptrs to strings to use

usestrings:
trouble_string:		ds.l	1
query_string:		ds.l	1
message_strings:	ds.l	4

	;-- name of things to open

LOCALENAME:
			dc.b	'locale.library',0
CATALOGNAME:
			dc.b	'sys/devs.catalog',0

	SECTION		printer,CODE
	CNOP		0,4

*------ Localize -----------------------------------------------------
*
*   NAME
*	Localize -- localize printer.device text strings
*
*   SYNOPSIS
*	void Localize(void)
*
*---------------------------------------------------------------------

_Localize:
		movem.l	d2-d3/a2-a4/a6,-(sp)

	;-- set-up default strings (assume no locale, or catalog)

		lea	AppStrings,a0
		lea	usestrings,a1

		moveq	#MSG_COUNT-1,d0
setstrings:
		move.l	as_Str(a0),(a1)+
		addq.l	#AppString_SIZEOF,a0
		dbf	d0,setstrings

	;-- Open locale.library

		lea	LOCALENAME,a1
		moveq	#LOCALEV,d0
		LINKEXE	OpenLibrary

		move.l	d0,_LocaleBase
		beq	usedefaults		; use default strings?

		move.l	d0,a6
		suba.l	a0,a0			; NULL (default catalog)
		lea	CATALOGNAME,a1
		suba.l	a2,a2			; no tags

	;-- Open catalog file

		CALLLIB	_LVOOpenCatalogA

		move.l	d0,_Catalog
		beq.s	usedefaults
		movea.l	d0,a2

	;-- Obtain ptrs to all strings

		moveq	#MSG_COUNT-1,d2

	;-- code such that first string can start with any number
		move.l	#MSG_PRI_DEV_TROUBLE,d3

		lea	AppStrings,a3
		lea	usestrings,a4

getstrings:
		movea.l	a2,a0		;catalog ptr
		move.l	d3,d0		;string number
		move.l	as_Str(a3),a1	;default string
		CALLLIB	_LVOGetCatalogStr
		move.l	d0,(a4)+	;cache ptr
		addq.l	#1,d3		;enumerated
		addq.l	#AppString_SIZEOF,a3
		dbf	d2,getstrings

usedefaults:
		movem.l	(sp)+,d2-d3/a2-a4/a6
		rts

*------ CloseLocale --------------------------------------------------
*
*   NAME
*	CloseLocale - close locale library, and catalog 
*
*   SYNOPSIS
*	void CloseLocale(void)
*
*---------------------------------------------------------------------
_CloseLocale:
		move.l	a6,-(sp)
		move.l	_LocaleBase,d0
		beq.s	cl_exit

		move.l	d0,a6
		move.l	_Catalog,d0
		beq.s	closelocale

		move.l	d0,a0
		CALLLIB	_LVOCloseCatalog

closelocale
		move.l	a6,a1
		LINKEXE	CloseLibrary

cl_exit:
		moveq	#00,d0
		move.l	d0,_LocaleBase
		move.l	d0,_Catalog
		move.l	(sp)+,a6
		rts


*------ TroubleNotify ------------------------------------------------
*
*   NAME
*	TroubleNotify -- display printer trouble requester
*
*   SYNOPSIS
*	result = TroubleNotify(struct Window *window,ULONG message_number)
*	d0			a0			d1
*
*   FUNCTION
*	Takes a message number as an argument, and window ptr
*	as args - calls EasyRequest() asking if the user wants to resume
*	printing, or cancel.
*
*   RESULT
*	0 indicating CANCEL, or 1 indicating RESUME.
*
*---------------------------------------------------------------------
_TroubleNotify:
		movem.l	a2-a3,-(sp)

		sub.l	#es_SIZEOF,sp
		movea.l	sp,a1

	;-- set-up EasyStruct

		move.l	#es_SIZEOF,es_StructSize(a1)
		clr.l	es_Flags(a1)

	;-- Use default (localized) System Request string for title
		
		clr.l	es_Title(a1)

		move.l	query_string,es_GadgetFormat(a1)
		move.l	trouble_string,es_TextFormat(a1)

		lea	message_strings,a2
		lsl.w	#2,d1		;table offset
		lea	0(a2,d1.w),a3	;ptr to arguments

	;-- and call it

		suba.l	a2,a2		;NULL IDCMP ptr

		LINKITU	EasyRequestArgs

		add.l	#es_SIZEOF,sp

		movem.l	(sp)+,a2-a3
		rts


		END
