**
**	$Id: library.asm,v 39.2 93/03/16 12:25:08 mks Exp $
**
**      mathffp library glue
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

**	Included Files

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/devices.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/strings.i"
	INCLUDE	"exec/initializers.i"


**	Imported Names

	XREF	MFName
	XREF	MFID
	XREF	REVISION
	XREF	VERSION

	XREF	libFuncInit


ABSEXECBASE	EQU	4
	XREF	_LVOFreeMem


*------ Exported Functions -------------------------------------------

	XDEF	MFInitTable
	XDEF	MFOpen
	XDEF	MFExtFunc
	XDEF	MFExpunge
	XDEF	MFClose


MFInitTable:
		dc.l    LIB_SIZE
		dc.l    libFuncInit
		dc.l    libStructInit
		dc.l    0


*----------------------------------------------------------------------
*
* Definitions for MathFFP Library Initialization
*
*----------------------------------------------------------------------
libStructInit:
*	;------ Initialize the library
		INITBYTE    LN_TYPE,NT_LIBRARY
		INITLONG    LN_NAME,MFName
		INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD    LIB_VERSION,VERSION
		INITWORD    LIB_REVISION,REVISION
		INITLONG    LIB_IDSTRING,MFID
		dc.w	    0



;------ MFOpen -------------------------------------------------------
MFOpen:		addq.w  #1,LIB_OPENCNT(a6)
		move.l  a6,d0
		rts


;------ MFClose ------------------------------------------------------
;------ MFExpunge ----------------------------------------------------
MFClose:	subq.w  #1,LIB_OPENCNT(a6)
MFExpunge:
MFExtFunc:	moveq   #0,d0
		rts

	END
