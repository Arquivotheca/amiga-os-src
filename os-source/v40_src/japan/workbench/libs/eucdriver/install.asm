**
**	$Id: install.asm,v 1.1 93/02/01 12:17:59 darren Exp $
**
**      EUC bitmap font driver - install procedure
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**


	SECTION		eucdriver

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/initializers.i"
	INCLUDE		"exec/alerts.i"

	INCLUDE		"utility/tagitem.i"

	INCLUDE		"driver.i"
	INCLUDE		"debug.i"

	INCLUDE 	"/diskfont/vecfont.i"

*------ Imported Globals ---------------------------------------------

	XREF	_EUCLoaderSeg
	XREF	_EUCLoaderName
	XREF	_EUCText
	XREF	_EUCCloseFont
	XREF	_EUCTextLength
	XREF	_EUCTextExtent
	XREF	_EUCTextFit

*------ Imported Functions -------------------------------------------

*------ Exported Functions -------------------------------------------

	XDEF	_EUCInstall

*------ Assumptions --------------------------------------------------

	IFNE	TAG_DONE
	FAIL	"TAG_DONE <> NULL; recode!"
	ENDC

	IFNE	ti_SIZEOF-8
	FAIL	"ti_SIZEOF <> 8; recode!"
	ENDC

*------ temps --------------------------------------------------------

_LVOSetFontVectors	EQU	-$42

****** vecfont.library/InstallVectoredFont **************************
*
*   NAME
*       InstallVectoredFont -- Install a vectored font
*
*   SYNOPSIS
*       success=InstallVectoredFont( ttextattr, textfont, custom, vecpath )
*       D0                              A0       A1         A2     A3
*
*       ULONG InstallVectoredFonts(
*				struct TTextAttr *, 
*				struct TextFont *, 
*				APTR custom,
*				char *vecpath );
*
*   FUNCTION
*       This is a standard function which must be provided by all
*	vectored font libraries.  It is this function which is
*	called by diskfont.library to actually install a vectored
*	font.
*
*	
*   INPUTS
*       ttextattr - Pointer to a TTextAttr structure.  This is
*	the same structure provided to diskfont library when
*	OpenDiskFont() was called.
*
*	textfont - Pointer to the TextFont structure created
*	by diskfont library (LoadSeg()ed from disk).  The
*	TextFFont structure has already been extended.
*
*	custom - Pointer to a custom structure; contains any
*	additional arguments required by this vectored font
*	library.
*
*	vecpath - Pointer to the full path/file name used to LoadSeg()
*	the font extension file for this font.  For scaled fonts,
*	the path/font name will be the font used as the source data for
*	this font.  This is useful for loading disk data which is stored
*	relative to the font extension file.  The string pointed
*	to by the function argument must be copied if required
*	by the driver.  Use dos.library/PathPart() to extract the path
*	from the string if needed.
*
*   RESULTS
*       success - Must be 1 indicating success, or 0 indicating
*	failure.  In the future this function may return a pointer.
*
*
*********************************************************************

_EUCInstall:
		movem.l	d4-d7/a4-a6,-(sp)
		move.l	a0,d6			;cache
		move.l	a1,d7			;cache

	PRINTF	DBG_ENTRY,<'EUC--InstallVectoredFont($%lx,$%lx,$%lx,$%lx)'>,A0,A1,A2,A3

	; allocate, and clear a new EUCFontHandle structure - all font
	; pointers are cleared as well

		move.l	a3,a0
		move.l	#EUCFontHandle_SIZEOF,d0	;size of structure
euci_pathlen:
		addq.l	#1,d0				;+ length of string
		tst.b	(a0)+
		bne.s	euci_pathlen

		move.l	a6,a5
		move.l	edvr_SysBase(a5),a6
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		JSRLIB	AllocVec

		tst.l	d0
		beq	eui_failed

	PRINTF	DBG_FLOW,<'EUC--EUCFontHandle @ $%lx'>,D0

	; prepare EUCFontHandle structure

		move.l	d0,a4

	PUSHWORD	(a2)
	PRINTF	DBG_FLOW,<'EUC--[efh_FontFlags=$%lx]'>
	POPLONG		1

		move.w	(a2)+,efh_FontFlags(a4)			;private flag info

	PRINTF	DBG_FLOW,<'EUC--Font LoadName: %s'>,A2

		lea	efh_FontLoadName(a4),a0
euci_copyloadname:
		move.b	(a2)+,(a0)+
		bne.s	euci_copyloadname

		move.l	a5,efh_DriverBase(a4)
		move.l	edvr_GfxBase(a5),efh_GfxBase(a4)
		move.l	edvr_SysBase(a5),efh_SysBase(a4)
		move.l	d7,efh_TextFont(a4)			;cache TextFont structure
		
		lea	efh_HandleSemaphore(a4),a0
		JSRLIB	InitSemaphore

		lea	efh_ReplyPort(a4),a0
		move.b	#NT_MSGPORT,LN_TYPE(a0)
		move.b	#PA_SIGNAL,MP_FLAGS(a0)
		move.b	#SIGB_SINGLE,MP_SIGBIT(a0)
		lea	MP_MSGLIST(a0),a0
		NEWLIST	a0

	; and copy path part of extension file

		lea	efh_ExtensionPath(a4),a0
		move.l	a3,a1
		move.l	a0,d1				;cache for PathPart()
euci_copypath:
		move.b	(a1)+,(a0)+
		bne.s	euci_copypath

	PRINTF	DBG_FLOW,<'EUC--Extension path: %s'>,D1

	; and truncate file part

		move.l	edvr_DOSBase(a5),a6
		JSRLIB	PathPart
		move.l	d0,a0
		move.b	#0,(a0)				;truncate
	
	; and strip last two chars if white space font

		btst.b	#FFB_WHITESPACED,efh_FontFlags+1(a4)
		beq.s	euci_notwhitespc

	PRINTF	DBG_FLOW,<'EUC--White Spaced font'>

		move.b	#0,-2(a0)			;remove "_e"

euci_notwhitespc:

	; finally, install vectors for this font

		move.l	edvr_SysBase(a5),a6
		move.l	#EUCTagItems_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		JSRLIB	AllocVec

		tst.l	d0
		beq	eui_failed2

		move.l	d0,a1
		move.l	d0,d3

		move.l	#SFV_UserData,eti_UserData+ti_Tag(a1)
		move.l	a4,eti_UserData+ti_Data(a1)

		move.l	#SFV_Library,eti_Library+ti_Tag(a1)
		move.l	a5,eti_Library+ti_Data(a1)

		move.l	#SFV_CodeSet,eti_CodeSet+ti_Tag(a1)
		move.l	#CODESET_JAPANESE_EUC,eti_CodeSet+ti_Data(a1)

		move.l	#SFV_TextFunc,eti_TextFunc+ti_Tag(a1)
		move.l	#_EUCText,eti_TextFunc+ti_Data(a1)

		move.l	#SFV_CloseFunc,eti_CloseFunc+ti_Tag(a1)
		move.l	#_EUCCloseFont,eti_CloseFunc+ti_Data(a1)

		move.l	#SFV_TextLengthFunc,eti_TextLengthFunc+ti_Tag(a1)
		move.l	#_EUCTextLength,eti_TextLengthFunc+ti_Data(a1)

		move.l	#SFV_TextExtentFunc,eti_TextExtentFunc+ti_Tag(a1)
		move.l	#_EUCTextExtent,eti_TextExtentFunc+ti_Data(a1)

		move.l	#SFV_TextFitFunc,eti_TextFitFunc+ti_Tag(a1)
		move.l	#_EUCTextFit,eti_TextFitFunc+ti_Data(a1)

		move.l	d7,a0			;pass in same TextFont
		move.l	edvr_DiskFontBase(a5),a6

	PRINTF	DBG_OSCALL,<'EUC--SetFontVectors($%lx,$%lx)'>,A0,A1

		jsr	_LVOSetFontVectors(a6)
	;;	JSRLIB	SetFontVectors		;in diskfont.library

		move.l	d0,d4			;cache return - free tags

		move.l	edvr_SysBase(a5),a6
		move.l	d3,a1
		JSRLIB	FreeVec

		tst.l	d4
		beq.s	eui_failed2

	; increment font count by 1 - lock-out expunge (font driver
	; is responsible for expunge)

		move.l	d7,a0
		move.w	#1,tf_Accessors(a0)

	; add to internal list of vectored fonts

		lea	edvr_EUCFontList(a5),a0
		move.l	a4,a1
		JSRLIB	AddHead

		move.l	d7,d0			;return TextFont structure

eui_failed:

	PRINTF	DBG_EXIT,<'EUC--$%lx=InstallVectoredFont()'>,D0

		movem.l	(sp)+,d4-d7/a4-a6
		rts


eui_failed2:
		move.l	a4,a1
		JSRLIB	FreeVec
		moveq	#00,d0			;failed
		bra.s	eui_failed

