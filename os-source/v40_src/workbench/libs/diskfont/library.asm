**
**	$Id: library.asm,v 38.4 92/01/29 13:54:20 davidj Exp $
**
**      diskfont.library initialization and access
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

;DEBUG	EQU	1

	SECTION		diskfont

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/strings.i"
	INCLUDE		"exec/initializers.i"
	INCLUDE		"exec/alerts.i"


	INCLUDE		"macros.i"
	INCLUDE		"dfdata.i"
	INCLUDE		"diskfont.i"
	INCLUDE		"debug.i"

*------ Imported Globals ---------------------------------------------

	XREF	DFName
	XREF	VERSION
	XREF	REVISION


*------ Imported Functions -------------------------------------------

	XLVO	Alert
	XLVO	AddLibrary
	XLVO	CloseLibrary
	XLVO	FreeMem
	XLVO	GetMsg
	XLVO	InitSemaphore
	XLVO	MakeLibrary
	XLVO	OpenLibrary

	XLVO	UnLoadSeg

	XLVO	StripFont

	XREF	_MyGetEnv

*------ Exported Functions -------------------------------------------

	XDEF	DFInit

	XDEF	_SysBase
	XDEF	_UtilityBase
	XDEF	_GfxBase
	XDEF	_DOSBase
	XDEF	_DiskfontBase


**********************************************************************
	DATA
_SysBase	ds.l	1
_UtilityBase	ds.l	1
_GfxBase	ds.l	1
_DOSBase	ds.l	1
_DiskfontBase	ds.l	1

**********************************************************************
	SECTION		diskfont

DFInit:
		dc.l	dfl_SIZEOF
		dc.l	dfFuncInit
		dc.l	dfStructInit
		dc.l	dfInit

dfInit:
		movem.l	a5/a6,-(a7)
		move.l	d0,a5
		move.l	a0,dfl_Segment(a5)
		move.l	a6,dfl_SysBase(a5)
		move.l	a6,_SysBase
		move.l	d0,dfl_DiskfontBase(a5)
		move.l	d0,_DiskfontBase

	    ;------ open the utility library
		lea	ULName(pc),a1
		moveq	#0,d0
		CALLLVO OpenLibrary
		move.l	d0,dfl_UtilityBase(a5)
		beq	dfiOpenULErr
		move.l	d0,_UtilityBase

	    ;------ open the graphics library
		lea	GLName(pc),a1
		moveq	#36,d0
		CALLLVO OpenLibrary
		move.l	d0,dfl_GfxBase(a5)
		beq	dfiOpenGLErr
		move.l	d0,_GfxBase

	    ;------ open the dos library
		lea	DLName(pc),a1
		moveq	#36,d0
		CALLLVO OpenLibrary
		move.l	d0,dfl_DOSBase(a5)
		beq.s	dfiOpenDLErr
		move.l	d0,_DOSBase

	    ;------ initialize the font list
		lea	dfl_DiskFonts(a5),a0
		NEWLIST	a0

	    ;------ initialize the bullet semaphore
		lea	dfl_BSemaphore(a5),a0
		CALLLVO	InitSemaphore

		move.l	a5,a0
		bsr	_MyGetEnv

		move.l	a5,d0
dfiRts:
		movem.l	(a7)+,a5/a6
		rts

dfiInitBLErr:
		ALERT	AN_DiskfontLib!AG_MakeLib
		bra.s	dfiCloseDOS

dfiOpenBLErr:
		ALERT	AN_DiskfontLib!AG_OpenLib
dfiCloseDOS:
		move.l	dfl_DOSBase(a5),a1
		CALLLVO	CloseLibrary
		bra.s	dfiCloseGfx

dfiOpenDLErr:
		ALERT	AN_DiskfontLib!AG_OpenLib!AO_DOSLib
dfiCloseGfx:
		move.l	dfl_GfxBase(a5),a1
		CALLLVO	CloseLibrary
		bra.s	dfiCloseUtil

dfiOpenGLErr:
		ALERT	AN_DiskfontLib!AG_OpenLib!AO_GraphicsLib

dfiCloseUtil:
		move.l	dfl_UtilityBase(a5),a1
		CALLLVO	CloseLibrary
		bra.s	dfiFreeMem

dfiOpenULErr:
		ALERT	AN_DiskfontLib!AG_OpenLib!AO_UtilityLib

dfiFreeMem:
		move.l	a5,a1
		move.w	LIB_NEGSIZE(a5),d0
		suba.w	d0,a1
		add.w	LIB_POSSIZE(a5),d0
		ext.l	d0
		CALLLVO FreeMem
		moveq	#0,d0
		bra	dfiRts

ULName:
		dc.b	'utility.library'
		dc.b	0
GLName:
		dc.b	'graphics.library'
		dc.b	0
DLName:
		dc.b	'dos.library'
		dc.b	0
		ds.w	0



*----------------------------------------------------------------------
*
* Definitions for Diskfont Library Initialization
*
*----------------------------------------------------------------------
dfStructInit:
*	;------ Initialize the library
		INITBYTE    LN_TYPE,NT_LIBRARY
		INITLONG    LN_NAME,DFName
		INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD    LIB_VERSION,VERSION
		INITWORD    LIB_REVISION,REVISION

		INITBYTE    dfl_DiskFonts+LH_TYPE,NT_FONT
		INITWORD    dfl_XDPI,72
		INITWORD    dfl_YDPI,72
		INITWORD    dfl_DotSizeX,100		; 1.0
		INITWORD    dfl_DotSizeY,100		; 1.0
		dc.w	    0


FUNCDEF		MACRO
	XREF	_DF\1
		dc.l    _DF\1
		ENDM


dfFuncInit:
		dc.l	Open
		dc.l	Close
		dc.l	Expunge
		dc.l	ExtFunc		; currently an RTS

	INCLUDE		"diskfont_lib.i"

		dc.l	-1


*------ Expunge ------------------------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove this library
*
*   SYNOPSIS
*	segment = Expunge(), diskfontLib
*	d0		     a6
*
*   FUNCTION
*	The Expunge routine is called when a user issues a RemLibrary
*	call.  The existance of any other users of the library, as
*	determined by the library open count being non-zero, will
*	cause the Expunge to be deferred.  When the library is not
*	in use, or no longer in use, the Expunge is actually
*	performed, and the library removed from the system list.
*
*	This library keeps the library access count at zero so that it
*	can use the Expunge call to free unused fonts.
*
*---------------------------------------------------------------------
Expunge:
		movem.l	a2/a5/a6,-(a7)
		move.l	a6,a5
		move.l	dfl_DiskFonts(a5),a2
exFontLoop:
		tst.l	(a2)
		beq.s	tryExpunge
		kprintf	<'Expunge candidate accessors $%lx name "%s"',13,10>,dfh_TF+tf_BoldSmear(a2),dfh_TF+LN_NAME(a2)
		tst.w	dfh_TF+tf_Accessors(a2)
		bgt.s	exNextFont

		;--	remove from the DiskfontBase fonts list
		move.l	a2,a1
		REMOVE

		;--	remove from the GfxBase fonts list
		lea	dfh_TF(a2),a1
		move.l	(a1),a0		; check to ensure the font is still
		cmp.l	LN_PRED(a0),a1	;   on the graphics font list (in
		bne.s	exUncompile	;   case it's already been removed
		move.l	LN_PRED(a1),a0	;   as the workaround to a bug here
		cmp.l	(a0),a1		;   in V1.0 & V1.1
		bne.s	exUncompile	;
		REMOVE			; from the GfxBase fonts list

exUncompile:
		;--	un-compile the font
		lea	dfh_TF(a2),a0
		move.l	dfl_GfxBase(a5),a6
		CALLLVO	StripFont

		;--	unload the font memory
		move.l	dfh_Segment(a2),d1
		kprintf	<'  UnLoadSeg $%lx',13,10>,d1
		move.l	dfl_DOSBase(a5),a6
		CALLLVO	UnLoadSeg

		subq.w	#1,dfl_NumLFonts(a5)

exNextFont:
		move.l	(a2),a2
		bra	exFontLoop

tryExpunge:
	    ;-- see if any one is using the library
		tst.w	dfl_PrivOpenCnt(a5)
		bne.s	deferExpunge
		tst.w	dfl_NumLFonts(a5)
		bgt.s	deferExpunge

	    ;-- this is really it.  Free up all the resources
		move.l  dfl_DOSBase(a5),a1
		move.l	dfl_SysBase(a5),a6
		CALLLVO CloseLibrary

		move.l  dfl_GfxBase(a5),a1
		CALLLVO CloseLibrary

		move.l  dfl_UtilityBase(a5),a1
		CALLLVO CloseLibrary

		move.l	a5,a1
		REMOVE

	    ;------ deallocate library data
		move.l	dfl_Segment(a5),a2
		move.l	a5,a1
		move.w	LIB_NEGSIZE(a5),d0
		suba.w	d0,a1
		add.w	LIB_POSSIZE(a5),d0
		ext.l	d0
		CALLLVO FreeMem

		move.l	a2,d0

exDone:
		movem.l	(a7)+,a2/a5/a6
ExtFunc:
		rts

deferExpunge:
		bset	#LIBB_EXPUNGED,LIB_FLAGS(a5)
		moveq	#0,d0		    ;still in use
		bra.s	exDone


*------ Open ---------------------------------------------------------
*
*   NAME
*	Open - a request to open this library
*
*   SYNOPSIS
*	Open(), diskfontLib
*		a6
*
*   FUNCTION
*	The open routine grants access to the library.
*
*	The standard library open count is not incremented.  This
*	library maintains a private open count, and always appears to
*	be ready to be expunged so that it can free unused font memory
*	during the expunge call.
*
*---------------------------------------------------------------------
Open:
		bclr	#LIBB_EXPUNGED,LIB_FLAGS(a6)
		addq.w	#1,dfl_PrivOpenCnt(a6)
		move.l	a6,d0
		rts


*------ Close --------------------------------------------------------
*
*   NAME
*	Close - terminate access to a library
*
*   SYNOPSIS
*	Close(), diskfontLib
*		 a6
*
*   FUNCTION
*	The close routine notifies a library that it will no longer
*	be used.
*
*	The private open count on the library will be decremented, and
*	if it falls to zero, and there are no disk fonts still loaded,
*	and an Expunge is pending, the Expunge will take place.
*
*---------------------------------------------------------------------
Close:
		subq.w	#1,dfl_PrivOpenCnt(a6)

	    ;-- check if this should now be expunged
		bne.s	closeRts
		btst	#LIBB_EXPUNGED,LIB_FLAGS(a6)
		beq.s	closeRts
		jmp	LIB_EXPUNGE(a6)

*	    ;-- report not expunged
closeRts:
		moveq	#0,d0
		rts

	END
