**
**	$Id: driver.i,v 1.2 93/05/06 12:23:01 darren Exp $
**
**      EUC bitmap font driver includes
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/semaphores.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/interrupts.i"

	INCLUDE "graphics/text.i"

	INCLUDE	"utility/tagitem.i"

** equates
ROM_MODULE	EQU	0				;set true for ROM module
FAST_PRECHAR	EQU	1				;set true for direct prechar
							;access vs subroutine

EUC_HICHAR	EQU	$FF
EUC_LOCHAR	EQU	$A1

INVALID_FONT	EQU	-1
RETRY_FONT	EQU	(10-1)				;must be a small number
							;never a valid TextFont address

MAXMUSTMEM	EQU	4096				;pre-alloced CHIP memory
							;for Text()

**
** Message port structure / used by process to find driver base and reply port
**
  STRUCTURE EUCLibPort,MP_SIZE
	APTR	elp_LibBase
	LABEL	EUCLibPort_SIZEOF

**
** Driver library base
**

  STRUCTURE EUCDriver,LIB_SIZE
	UWORD	edvr_reserved
	APTR	edvr_SysBase
	APTR	edvr_GfxBase
	APTR	edvr_DOSBase
	APTR	edvr_DiskFontBase
	APTR	edvr_UtilityBase
	APTR	edvr_MemList				; memlist for process
	APTR	edvr_LibMsgPort				; pointer to EUCLibPort
	APTR	edvr_MustMem				; CHIP RAM MUST MEMORY
	ULONG	edvr_SegList
	STRUCT	edvr_EUCFontList,LH_SIZE		; list of EUC fonts
	STRUCT	edvr_OpenSemaphore,SS_SIZE		; Open/Close lib semaphore
	STRUCT	edvr_MustMemSemaphore,SS_SIZE		; MustMem semaphore
	STRUCT	edvr_LoaderPort,MP_SIZE			; 
	STRUCT	edvr_LowMemHandler,IS_SIZE		; for low memory handler
	LABEL	EUCDriver_SIZEOF

**
** Tag item structure for calling SetFontVectors()
**

  STRUCTURE EUCTagItems,0
	STRUCT	eti_UserData,ti_SIZEOF
	STRUCT	eti_Library,ti_SIZEOF
	STRUCT	eti_CodeSet,ti_SIZEOF
	STRUCT	eti_TextFunc,ti_SIZEOF
	STRUCT	eti_CloseFunc,ti_SIZEOF
	STRUCT	eti_TextLengthFunc,ti_SIZEOF
	STRUCT	eti_TextExtentFunc,ti_SIZEOF
	STRUCT	eti_TextFitFunc,ti_SIZEOF
	STRUCT	eti_TagDone,ti_SIZEOF
	LABEL	EUCTagItems_SIZEOF


**
** Message for process structure (requesting font to OpenDiskFont)
**
  STRUCTURE EUCOpenFont,MN_SIZE
	STRUCT	eof_TTextAttr,tta_SIZEOF		; font to open
	STRUCT	eof_DPITag,ti_SIZEOF			; clone of DPI tag
	STRUCT	eof_TAGDONE,ti_SIZEOF			; end of tag list
	APTR	eof_TextFont				; return font, or NULL
	LABEL	EUCOpenFont_SIZEOF

**
** Font array structure element (lock, retry count, and pointer to TextFont)
**
  STRUCTURE EUCFontElement,0
	UWORD	efe_Lock
	UWORD	efe_Retry
	APTR	efe_TextFont
	LABEL	EUCFontElement_SIZEOF

FONTARRAY	EQU	(((EUC_HICHAR-EUC_LOCHAR)+1)*EUCFontElement_SIZEOF)

**
** Font flags
**

	BITDEF	FF,WHITESPACED,0

**
** This is the handle for a font (UserData)
**
  STRUCTURE EUCFontHandle,MLN_SIZE		; maintain private list of my fonts
	APTR	efh_DriverBase			; driver library base
	APTR	efh_GfxBase			; quick lookup of GfxBase
	APTR	efh_SysBase			; cach in fast (?) ram
	APTR	efh_TextFont			; TextFont structure for this handle

	; cache pointers to TextFont structures of opened fonts for quick access

	STRUCT	efh_FontLookup,FONTARRAY

	; semaphore for this fonts private font list

	STRUCT	efh_HandleSemaphore,SS_SIZE

	; message for process

	STRUCT	efh_Message,EUCOpenFont_SIZEOF		; message for process

	; reply port for process message

	STRUCT	efh_ReplyPort,MP_SIZE			; 

	; flag to indicate if this is an algorithmic white space font

	UWORD	efh_FontFlags


	STRUCT	efh_PFontPath,256			; path of private font

	; font file name for loading subfonts

	STRUCT	efh_FontLoadName,32

	; variable length, complete path to directory of private files

	UBYTE	efh_ExtensionPath

	ALIGNWORD

	LABEL	EUCFontHandle_SIZEOF

