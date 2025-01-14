	TTL    '$Id: library.asm,v 37.1 91/01/21 11:43:19 mks Exp $'
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
*   Source Control
*   --------------
*   $Id: library.asm,v 37.1 91/01/21 11:43:19 mks Exp $
*
*   $Locker:  $
*
*   $Log:	library.asm,v $
*   Revision 37.1  91/01/21  11:43:19  mks
*   V37 checkin for Make Internal
*   
*   Revision 36.3  90/04/08  01:29:35  dale
*   new RCS
*
*   Revision 36.2  89/12/07  00:19:10  dale
*   private mathlibrary stuff
*
*   Revision 36.1  89/11/17  21:38:06  dale
*   make internal wierding out
*
*   Revision 36.0  89/06/05  15:45:35  dale
*   *** empty log message ***
*
*
**********************************************************************

	SECTION		mathieeesingtrans

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
	INCLUDE		'exec/libraries.i'
	INCLUDE		'exec/initializers.i'
	INCLUDE		'exec/execbase.i'

	INCLUDE		'mathieeesingtrans_rev.i'
	INCLUDE		'macros.i'
	include "resources/mathresource.i"
	include	"privatemathlibrary.i"


DEBUG	EQU		0		* Turn on debug statements if set


*------ Imported Globals ---------------------------------------------

	    XREF	_AbsExecBase

	    XREF	MIName


*------ Imported Functions -------------------------------------------

	XREF_EXE	MakeLibrary	* Generates:  XREF	_LVOMakeLibrary
	XREF_EXE OpenResource		* for _LVOOpenResource
	XREF_EXE	OpenLibrary	* Generates:  XREF	_LVOOpenLibrary
	XREF_EXE	AddLibrary	* Generates:  XREF  _LVOAddLibrary
	XREF_EXE	FreeMem		* Generates:  XREF  _LVOFreeMem
	XREF_EXE	NoFunc		* Generates:  XREF  _LVONoFunc
	XREF_EXE	Forbid
	XREF_EXE	Permit

	IFNE	DEBUG
		XREF	_kprintf
	ENDC


*------ Exported Globals ---------------------------------------------

	XDEF	_MathSegList

*------ Exported Functions -------------------------------------------

	XDEF	MIInit

*------ Temporary Debug Macros Used ----------------------------------

PRINT	MACRO
	IFNE	DEBUG
		movem.l	d0-d7/a0-a6,-(sp)	* Save all processor registers
		pea		msg\1(pc)			* Push message address
		jsr		_kprintf			* Go display the message
		addq.l	#4,sp				* Clean up stack from C call
		movem.l	(sp)+,d0-d7/a0-a6	* Restore all processor registers
	ENDC
		ENDM

**  TITLE   "--Library-- Init"
*------ Transcendental Math Init -------------------------------------
*
*   NAME
*	Init - initialize the Transcendental Math library from nothing
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
*	accepts a pointer to the systemLib, not the
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
	xref	_LVOCloseLibrary
	xref	hardlibFuncInit
	xref	iolibFuncInit

	data
	xdef	_MathIeeeSingBasBase
	xdef	_MathIeeeSingTransBase
	xdef	_SysBase
_MathIeeeSingBasBase ds.l 1
_MathIeeeSingTransBase ds.l 1	; used by Pow function
_SysBase ds.l 1
	code

my_rts:	clr.l	d0
	rts

*	this needs to become a resource that appears during autoconfig.
io68881	dc.b	'MathIEEE.resource',0
	cnop	0,2
mathbase dc.b	'mathieeesingbas.library',0
	CNOP 0,2
MIInit:
	move.l	a0,_MathSegList
	move.l	a6,_SysBase
	movem.l	d2/d3/a2,-(sp)	* save registers used
	clr.l	d2
	clr.l	d3
*	find Basic library
	moveq	#0,d0
	lea	mathbase,a1
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,_MathIeeeSingBasBase	; store here
	if =				; null?
		movem.l	(sp)+,d2/d3/a2
		rts			; error return, need library
	endif
	lea	hardlibFuncInit,a0
	btst	#AFB_68881,AttnFlags+1(a6)
	bne.s	load_it
*	test for 68881 as i/o device
	moveq	#0,d0			* resource version
	lea	io68881,a1
	jsr	_LVOOpenResource(a6)	* for now
	move.l	d0,d3
	beq.s	use_soft
*               found 68881 as an i/o device
                move.l  d0,a1
                move.l  MathIEEEResource_BaseAddr(a1),d2        ; extract ptr
                tst.l   d2                      ; is it there?
                beq.s   use_soft
                lea     iolibFuncInit,a0
        bra.s   load_it
*	check for vendors own support
use_soft:
	lea	libFuncInit,a0
load_it:
	sub.l	a2,a2			* no MakeLibrary Init
	lea	libStructInit,a1	*
	move.l	#MathIEEEBase_SIZE,d0
	clr.l	d1
	jsr	_LVOMakeLibrary(a6)
	move.l	d0,a2
	move.l	d0,_MathIeeeSingTransBase

*	set up default open/close routines
	lea	my_rts,a1
	move.l	a1,MathIEEEBase_TaskOpenLib(a2)
	move.l	a1,MathIEEEBase_TaskCloseLib(a2)

* retest 68881 bit for 68020/68881 and reflect state in local flag
	move.b	AttnFlags+1(a6),d0
	and.b	#AFF_68881,d0
	if <>
		lea	init_68881,a1
		move.l	a1,MathIEEEBase_TaskOpenLib(a2)
	endif
	move.b	d0,MathIEEEBase_Flags(a2)
	move.l	d2,MathIEEEBase_68881(a2)
	move.l  a6,MathIEEEBase_SysLib(a2)
	move.l	d3,MathIEEEBase_Resource(a2)
	if <>		; check for alternate functions
		lea	init_io68881,a1
		move.l	a1,MathIEEEBase_TaskOpenLib(a2)
		move.l	d3,a1
		btst	#MATHIEEERESOURCEB_SGLTRANS,MathIEEEResource_Flags+1(a1)
		if <>
			move.l	MathIEEEResource_SglTransInit(a1),a0
			jsr (a0)
		endif
	endif
	move.l	a2,a1	* transfer library pointer
*	fill in the library node
	jsr	_LVOAddLibrary(a6)

	movem.l	(sp)+,d2/d3/a2	* restore register
	rts

MIExtFunc:
	clr.l	d0
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
*------ Transcendental Math Expunge ------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove the Transcendental Math library
*
*   SYNOPSIS
*	Result = Expunge()
*			    A6
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

*    ;-- this is really it.  Free up all the resources
	move.l	a6,-(sp)
	move.l	_MathIeeeSingBasBase,a1
	LINKEXE CloseLibrary
	move.l	(sp)+,a6
	MOVE.L	A6,A1
        REMOVE

*  ;------ deallocate Transcendental Math library data
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
	BSET    #LIBB_DELEXP,LIB_FLAGS(A6)	* set expunge request on close
	LINKEXE	Permit
	MOVEQ   #0,D0	    ;still in use
    RTS


**  TITLE   "--Library-- Open"
*------ Transcendental Math Open -------------------------------------
*
*   NAME
*	Open - a request to open the Transcendental Math library
*
*   SYNOPSIS
*	Open(), mathieeesingtranslib
*		A6
*
*   FUNCTION
*	The open routine grants access to the Transcendental Math library.	The
*	library open count will be incremented.	 The library cannot
*	be expunged unless this open is matched by a Close.
*
*---------------------------------------------------------------------
	xref	init_68881
	xref	init_io68881
MIOpen:
	move.l	MathIEEEBase_TaskOpenLib(a6),a0
	jsr	(a0)	; call any special open routine
	if d0.l
		clr.l	d0
		rts
	endif
	BCLR    #LIBB_DELEXP,LIB_FLAGS(A6)	* cancel expunge request
	ADDQ.W  #1,LIB_OPENCNT(A6)		* Bump library open counter
	MOVE.L  A6,D0
	RTS


**  TITLE   "--Library-- Close"
*------ Transcendental Math Close -------------------------------------
*
*   NAME
*	Close - terminate access to a library
*
*   SYNOPSIS
*	Close(), mathieeesingtranslib
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
			PRINT	8

	move.l	MathIEEEBase_TaskCloseLib(a6),a0
	jsr	(a0)

	SUBQ.W  #1,LIB_OPENCNT(A6)
	BTST    #LIBB_DELEXP,LIB_FLAGS(A6)
	BNE	MIExpunge		; checks opencnt itself
	clr.l	d0			; no expunge
	RTS

*
*	Data Definition Region
*

_MathSegList:
	dc.l	0

	END
