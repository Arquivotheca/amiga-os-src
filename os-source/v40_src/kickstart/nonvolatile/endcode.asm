******************************************************************************
*
*	$Id: endcode.asm,v 40.3 93/04/16 12:11:15 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	endcode.asm,v $
* Revision 40.3  93/04/16  12:11:15  brummer
* Add CR to last line of file.
* 
* Revision 40.2  93/04/16  11:44:44  brummer
* Fix rom tag in EndCode.asm
*
* Revision 40.1  93/02/25  16:12:30  Jim2
* When not using RTF_AUTOINIT using the init structure is not a good idea.
*
* Revision 40.0  93/02/19  15:27:33  Jim2
* Added a second ROM tag for the library.  This ROM tag is AfterDOS and
* will result in nonvolatile library being opened and closed.  This
* will place DOSBase in the nonvolatile library base and allow the
* library to now access any nonvolatile information stored on the disk.
*
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

	INCLUDE	'exec/macros.i'
        INCLUDE "exec/types.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/libraries.i"

		;Startup.asm
	XREF	ROMTagName
	XREF	subsysName



        XDEF    __EndCode

        SECTION NonVolatileLibrary

__EndCode:

initLDescrip:
		dc.w	RTC_MATCHWORD	; RT_MATCHWORD
		dc.l	initLDescrip	; RT_MATCHTAG
		dc.l	EndCode
		dc.b	RTF_AFTERDOS
		dc.b	0
		dc.b	NT_UNKNOWN	; RT_TYPE
		dc.b	0		; RT_PRI
		dc.l	ROMTagName
		dc.l	ROMTagName
		dc.l	initFunc

*****l* Startup.asm/initFunc *************************************************
*
*   NAME
*	initFunc - Cause Nonvolatile.library to open DOSBase.
*
*   SYNOPSIS
*	InitLib = SystemControlA (LibBase, SegList, ExecBase)
*	D0			  D0	   A1	    A6
*
*   FUNCTION
*	Open nonvolatile.library then close it.  Whenever the nonvolatile
*	library is opened it checks to see if DOS library is open.  If not
*	it will be opened.
*
*   INPUTS
*	LibBase - Pointer to the memory allocated for the library base.
*	SegList	- Pointer to the memory allocated and loaded with the
*		  library code.
*	ExecBase - Pointer to the base of exec.library.
*
*   RESULT
*	Always returns NULL
*
******************************************************************************
initFunc
		lea	subsysName(pc),a1
		CLEAR	d0
		JSRLIB	OpenLibrary
		move.l	d0,a1
		JSRLIB	CloseLibrary
		CLEAR	d0
		rts

EndCode:
		end
