	TTL    '$Id: library.asm,v 37.1 91/01/21 12:02:05 mks Exp $'
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*   $Id: library.asm,v 37.1 91/01/21 12:02:05 mks Exp $
*
**********************************************************************

	SECTION		mathieeedoubbas

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
	INCLUDE		'exec/libraries.i'
	INCLUDE		'exec/initializers.i'
	INCLUDE		'exec/execbase.i'

	INCLUDE		'mathieeedoubbas_rev.i'
	INCLUDE		'macros.i'
	include	"resources/mathresource.i"
	include "privatemathlibrary.i"


*------ Imported Globals ---------------------------------------------

	XREF	MIName
	xref	_AbsExecBase

*------ Imported Functions -------------------------------------------

	XREF_EXE	MakeLibrary	* Generates:  XREF	_LVOMakeLibrary
	XREF_EXE	OpenResource	* Generates:  XREF	_LVOOpenLibrary
	XREF_EXE	AddLibrary	* Generates:  XREF  _LVOAddLibrary
	XREF_EXE	FreeMem		* Generates:  XREF  _LVOFreeMem
	XREF_EXE	NoFunc		* Generates:  XREF  _LVONoFunc
	XREF_EXE	Permit
	XREF_EXE	Forbid


*------ Exported Globals ---------------------------------------------

	XDEF	_MathSegList

*------ Exported Functions -------------------------------------------

	XDEF	MIInit

my_rts:
	clr.l	d0
	rts

**  TITLE   "--Library-- Init"
*------ Init -------------------------------------
*
*   NAME
*	Init - initialize the library from nothing
*
*   SYNOPSIS
*	error = Init(), systemLibrary
*	D0		A6
*
*   FUNCTION
*	This routine will initialize the library.  It should only be
*	used if the library is not available to be opened (i.e. it has
*	been successfully Expunged).
*
*	This entry point will remain valid for all time, since it
*	is in rom, and it is OK for it to be cached even when the
*	library is not open.
*
*   INPUTS
*	Note that this entry point differs from all others in that it
*	accepts a pointer to the systemLib, not the audioLib
*	(which should not yet exist).  This pointer is not cached for
*	later use by the library, which always gets to the system
*	library via SysBase.  The name that is passed to some library
*	Init routines in A1 is not used by this library.
*
*   RESULTS
*	error	-
*		if the Init succeeded, then Error will be zero.	 If
*		it failed then it will be non-zero.
*
*---------------------------------------------------------------------
	xref	_LVOMakeLibrary
	xref	_LVOAddLibrary
	xref	hardlibFuncInit
	xref	iolibFuncInit

;MAXIPLANBUG	equ 1

*	this needs to become a resource that appears during autoconfig.
io68881	dc.b	'MathIEEE.resource',0
	CNOP	0,2
MIInit:
	move.l	a0,_MathSegList
	movem.l	d2/d3/a2,-(sp)	* save registers used
	clr.l	d2
	clr.l	d3
	lea	hardlibFuncInit,a0
	btst	#AFB_68881,AttnFlags+1(a6)
	ifnd	MAXIPLANBUG
	bne.s	load_it
	endc
*	test for 68881 as i/o device
	moveq	#0,d0			* resource version
	lea	io68881,a1
	jsr	_LVOOpenResource(a6)	* for now
	move.l	d0,d3
	beq.s	use_soft
*		found 68881 as an i/o device
		move.l	d0,a1
		move.l	MathIEEEResource_BaseAddr(a1),d2	; extract ptr to 68881 base
		tst.l	d2			; is it there?
		beq.s	use_soft
		lea	iolibFuncInit,a0
	bra.s	load_it
use_soft:
	lea	libFuncInit,a0
load_it:
	sub.l	a2,a2			* There is not MakeLibrary init
	lea	libStructInit,a1	*
	move.l	#MathIEEEBase_SIZE,d0
	clr.l	d1
	jsr	_LVOMakeLibrary(a6)
*	fill in the library node
	move.l	d0,a2
;	set up default task inits


;	code for superfast fneg and fabs
;	just jam the instructions themselves into vectortable
;	removed by popular demand, sigh.
;	xref	MIIEEEDPNeg,MIIEEEDPAbs
;	xref	_LVOIEEEDPNeg,_LVOIEEEDPAbs
;	move.l	MIIEEEDPNeg,_LVOIEEEDPNeg(a2)
;	move.w	MIIEEEDPNeg+4,_LVOIEEEDPNeg+4(a2)
;	move.l	MIIEEEDPAbs,_LVOIEEEDPAbs(a2)
;	move.w	MIIEEEDPAbs+4,_LVOIEEEDPAbs+4(a2)

	lea	my_rts,a1
	move.l	a1,MathIEEEBase_TaskOpenLib(a2)
	move.l	a1,MathIEEEBase_TaskCloseLib(a2)
	move.b	AttnFlags+1(a6),d0
	and.b   #AFF_68881,d0
	ifd	MAXIPLANBUG
	bclr	#AFB_68881,d0
	btst	#AFB_68881,d0
	endc
	if <>

		lea	init_68881,a1
		move.l	a1,MathIEEEBase_TaskOpenLib(a2)
	endif
	move.b  d0,MathIEEEBase_Flags(a2)
	move.l	d2,MathIEEEBase_68881(a2)
	move.l	a6,MathIEEEBase_SysLib(a2)
	move.l	d3,MathIEEEBase_Resource(a2)
	if <>				; check for alternate routines
		lea	init_io68881,a1
		move.l	a1,MathIEEEBase_TaskOpenLib(a2)
		move.l	d3,a1
		btst	#MATHIEEERESOURCEB_DBLBAS,MathIEEEResource_Flags+1(a1)
		if <>
			move.l	MathIEEEResource_DblBasInit(a1),a0
*				call with a6->SysBase
*					  a1->Resource
*					  a2->Math Library
			jsr (a0)
		endif
	endif
	move.l	a2,a1
	jsr	_LVOAddLibrary(a6)	; a1->Library

	movem.l	(sp)+,d2/d3/a2	* restore register
	rts

MIExtFunc:
		moveq	#0,d0
		RTS

**  SKIP
*----------------------------------------------------------------------
*
* Definitions for Transcendental Math Library Initialization
*
*----------------------------------------------------------------------
libStructInit:
*	;------ Initialize the library
		INITBYTE    LN_TYPE,NT_LIBRARY
		INITLONG    LN_NAME,MIName
		INITWORD	LIB_REVISION,REVISION
		INITWORD	LIB_VERSION,VERSION
		INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_OPENCNT,0

		DC.W	    0


	xref	libFuncInit

	xdef	MIOpen
	xdef	MIClose
	xdef	MIExtFunc
	xdef	MIExpunge

**  TITLE   "--Library-- Expunge"
*------ Expunge ------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove the library
*
*   SYNOPSIS
*	Result = Expunge()
*
*   FUNCTION
*	The Expunge routine is called when a user issues a RemLibrary
*	call.  By the time it is called, the library has already been
*	removed from the library list, so no new opens will succeed.
*	The existance of any other users of the library, as determined
*	by the library open count being non-zero, will cause the
*	Expunge to be deferred.	 When the library is not in use, or no
*	longer in use, the Expunge is actually performed.
*
*   !!!
*	This will change, so expunged librarys are not in the nether
*	state of not being Open-able or Init-able.
*
*   INPUTS
*
*---------------------------------------------------------------------

MIExpunge:

*	    ;-- see if any one is using the library
	LINKEXE	Forbid
	TST.W   LIB_OPENCNT(A6)
	BNE.S   Exp_InUse

*	    ;-- this is really it.  Free up all the resources
	MOVE.L	A6,A1
        REMOVE

*	    ;------ deallocate Math library data
	MOVE.L  A6,A1
	MOVEQ	#0,D0
	MOVE.W  LIB_NEGSIZE(A6),D0
	SUB.W	D0,A1
	ADD.W	LIB_POSSIZE(A6),D0
	LINKEXE FreeMem

	LINKEXE	Permit
	MOVE.L   _MathSegList,D0
	RTS

Exp_InUse:

	BSET    #LIBB_DELEXP,LIB_FLAGS(A6)
	LINKEXE	Permit
	MOVEQ   #0,D0	    ;still in use
	RTS


**  TITLE   "--Library-- Open"
*------ Open -------------------------------------
*
*   NAME
*	Open - a request to open the library
*
*   SYNOPSIS
*	Open(), mathieeedoubbaslib
*		A6
*
*   FUNCTION
*	The open routine grants access to the library.	The
*	library open count will be incremented.	 The library cannot
*	be expunged unless this open is matched by a Close.
*
*---------------------------------------------------------------------

	xref init_68881
	xref init_io68881
MIOpen:
*	if the 68881 is being used then set the exceptions up
*	and initialize rounding
	move.l	MathIEEEBase_TaskOpenLib(a6),a0
	jsr	(a0)
	if d0.l
openFail:
		clr.l	d0
		rts
	endif
    	ADDQ.W  #1,LIB_OPENCNT(A6)		* Bump library open counter
	BCLR    #LIBB_DELEXP,LIB_FLAGS(A6)	* cancel pending expunge
    	MOVE.L  A6,D0
    	RTS

**  TITLE   "--Library-- Close"
*------ Close -------------------------------------
*
*   NAME
*	Close - terminate access to a library
*
*   SYNOPSIS
*	Close(), mathieeedoubbaslib
*		 A6
*
*   FUNCTION
*	The close routine notifies a library that it will no longer
*	be using the library.
*
*	The open count on the library will be decremented, and if it
*	falls to zero and an Expunge is pending, the Expunge will
*	take place.
*
*---------------------------------------------------------------------
MIClose:

	move.l	MathIEEEBase_TaskCloseLib(a6),a0
	jsr	(a0)
	SUBQ.W  #1,LIB_OPENCNT(A6)
	BTST    #LIBB_DELEXP,LIB_FLAGS(A6)
	BNE	MIExpunge	; Expunge will check opencount
	clr.l	d0		; don't expunge
closed:
	RTS

*
*	Data Definition Region
*

_MathSegList:
		DC.L	0

		    END
