**********************************************************************
*
* Library.asm
*
* Copyright 1991 Raymond S. Brand, All rights reserved.
*
* 19910208
*
* $Id$
*
**********************************************************************

	INCLUDE	"internal.i"


	SECTION	NetBuff

	XDEF	Open
	XDEF	Close
	XDEF	Expunge
	XDEF	Null

	XSYS	FreeMem

	XREF	DeferedFree


*****i* netbuff.library/Open() ***************************************
*
*   NAME
*	Open -- Open the NetBuff library for use.
*
*   SYNOPSIS
*	Library Open( Version ) NBL_lib
*	D0            D0        A6
*
*	struct Library * Open( ULONG );
*
*   FUNCTION
*	This function opens the library for use.
*
*   INPUTS
*	Version         Version level desired.
*
*   RESULTS
*	Library         Pointer to the library base or NULL.
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	version	scratch	library
* A6 --	library	library	library
*

Open:
*!* debug
	ERRMSG	"Open"
*!* debug
	bsr.w	DeferedFree

	addq.w	#1,LIB_OPENCNT(a6)
	move.l	a6,d0
	rts


*****i* netbuff.library/Close() **************************************
*
*   NAME
*	Close -- Close the library.
*
*   SYNOPSIS
*	SegList = Close( ) NBL_lib
*	                   A6
*
*	void *Close( void );
*
*   FUNCTION
*	This routine closes the library.
*
*   RESULTS
*	SegList         SegList pointer stored at Init() if library is
*	                    Expunged(), else zero.
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	?????	scratch	SegList
* A6 --	library	library	library
*

Close:
*!* debug
	ERRMSG	"Close"
*!* debug
	bsr.w	DeferedFree

	subq.w	#1,LIB_OPENCNT(a6)

	; fall into expunge - convenient for testing
	; rft


*****i* netbuff.library/Expunge() ************************************
*
*   NAME
*	Expunge -- Expunge the library.
*
*   SYNOPSIS
*	SegList = Expunge( ) NBL_lib
*	                     A6
*
*	void *Expunge( void );
*
*   FUNCTION
*	This routine removes the library from the system if the open
*	count is zero.
*
*   INPUTS
*
*   RESULTS
*	SegList         SegList pointer stored at Init() if library is
*	                    Expunged(), else zero.
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	?????	scratch	SegList
* D1 --	?????	scratch	munged
* D2 --	?????	scratch	restored
* A0 --	?????	scratch	munged
* A1 --	?????	scratch	munged
* A2 --	?????	library	restored
* A6 --	library	exec	library
*

Expunge:
*!* debug
	ERRMSG	"Expunge"
*!* debug
	bsr.w	DeferedFree

	tst.w	LIB_OPENCNT(a6)		; test if still open
	bne.w	Null

	lea.l	NBL_FREEPOOL(a6),a0
	TSTLIST	a0
	bne.w	Null

*!* debug
	ERRMSG	"Expunging"
*!* debug
	movem.l	d2/a2,-(sp)

	move.l	a6,a2
	move.l	NBL_SYSLIB(a2),a6

	move.l	a2,a1			; remove library from exec's list
	REMOVE

	move.l	NBL_SEGLIST(a2),d2	; save for return

	move.l	a2,a1			; remove the library base
	moveq.l	#0,d0
	moveq.l	#0,d1
	move.w	LIB_NEGSIZE(a2),d1
	sub.l	d1,a1
	move.w	LIB_POSSIZE(a2),d0
	add.l	d1,d0
	CALLSYS	FreeMem

	move.l	d2,d0
	move.l	a2,a6
	movem.l	(sp)+,d2/a2
*!* debug
	ERRMSG	"Expunged"
*!* debug
	rts


*****i* netbuff.library/Null() ***************************************
*
*   NAME
*	Null -- Do nothing
*
*   SYNOPSIS
*	null = Null() NBL_lib
*	D0            A6
*
*	LONG Null( );
*
*   FUNCTION
*	This routine returns null.
*
*   INPUTS
*
*   RESULTS
*	null -- always zero.
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	?????	scratch	zero
* A6 --	library	library	library
*

Null:
*!* debug
	ERRMSG	"Null"
*!* debug
	moveq.l	#0,d0
	rts


	END
