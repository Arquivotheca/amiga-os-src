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
* $Id: library.asm,v 40.7 93/03/16 12:22:47 mks Exp $
*
**********************************************************************

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
	INCLUDE		'exec/libraries.i'
	INCLUDE		'exec/initializers.i'
	INCLUDE		'exec/execbase.i'

	INCLUDE		'mathieeesingbas_rev.i'
	INCLUDE		'macros.i'

	include		'resources/mathresource.i'
	include		'privatemathlibrary.i'


*------ Imported Globals ---------------------------------------------

	XREF	MIName
	xref	_AbsExecBase
	xref	libFuncInit
	xref	init_68881
	xref	init_io68881

*------ Imported Functions -------------------------------------------

	XREF_EXE	MakeLibrary	* Generates:  XREF	_LVOMakeLibrary
	XREF_EXE	OpenResource	* Generates:  XREF	_LVOOpenLibrary
	XREF_EXE	AddLibrary	* Generates:  XREF	_LVOAddLibrary
	XREF_EXE	FreeMem		* Generates:  XREF	_LVOFreeMem
	XREF_EXE	Permit
	XREF_EXE	Forbid


*------ Exported Globals ---------------------------------------------

*------ Exported Functions -------------------------------------------

	XDEF	MIInit

	IFD	ON_DISK
	clr.l	d0
	rts
	ENDC

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

MIInit:	movem.l	d2/d3/a2,-(sp)	* save registers used
	move.l	a0,-(sp)	* save MathSegList here
	clr.l	d2
	clr.l	d3
	lea	hardlibFuncInit,a0
	btst	#AFB_68881,AttnFlags+1(a6)
	bne.s	load_it
*
* If FPU-Only build, we wish to exit here...  (Error)
*
	IFD	CPU_FPU
	move.l	(sp)+,a0	; Restore seglist from stack...
	moveq.l	#0,d0		; Return failure
	bra.s	init_Exit	; And exit...
	ENDC
*
* If not FPU-Only, we have software option...
*
	IFND	CPU_FPU
	lea	libFuncInit,a0
	ENDC

load_it:
	sub.l	a2,a2			* There is not MakeLibrary init
	lea	libStructInit,a1	*
	move.l	#MathIEEEBase_SIZE,d0
	clr.l	d1
	jsr	_LVOMakeLibrary(a6)
*	fill in the library node
	move.l	d0,a2
;	set up default task inits

********************************************************************************
*
* If we are building 68000-compatible, we should check for 68020 and up...
*
	IFD	CPU_68000
*
;	If we are an 020 or better without a 68881/2 then we
;	can probably do better than any 68881 peripheral processor
	btst	#AFB_68881,AttnFlags+1(a6)

	bne.s	_if0000	; if =
		btst #AFB_68020,AttnFlags+1(a6)

		beq.s	_if0001	; if <>
			xref	MIIEEESPMul020,_LVOIEEESPMul
			xref	MIIEEESPDiv020,_LVOIEEESPDiv
			lea	MIIEEESPMul020,a1
			move.l	a1,_LVOIEEESPMul+2(a2)
			lea	MIIEEESPDiv020,a1
			move.l	a1,_LVOIEEESPDiv+2(a2)
_if0001:			; endif

_if0000:		; endif
*
	ENDC
*
********************************************************************************

;	code for superfast fneg and fabs
;	just jam the instructions themselves into vectortable
;	removed by popular demand, sigh.
;	xref	MIIEEEDPNeg,MIIEEEDPAbs
;	xref	_LVOIEEEDPNeg,_LVOIEEEDPAbs
;	move.l	MIIEEEDPNeg,_LVOIEEEDPNeg(a2)
;	move.w	MIIEEEDPNeg+4,_LVOIEEEDPNeg+4(a2)
;	move.l	MIIEEEDPAbs,_LVOIEEEDPAbs(a2)
;	move.w	MIIEEEDPAbs+4,_LVOIEEEDPAbs+4(a2)

	lea	my_rts,a1	; Default...
	btst.b	#AFB_68881,AttnFlags+1(a6)
	beq.s	no_FPU
	lea	init_68881,a1	; FPU version...
no_FPU:	move.l	a1,MathIEEEBase_TaskOpenLib(a2)
;
	move.b  d0,MathIEEEBase_Flags(a2)
	move.l	d2,MathIEEEBase_68881(a2)
	move.l	a6,MathIEEEBase_SysLib(a2)

;	Get MathSegList saved on stack now
	move.l	(sp)+,MathIEEEBase_SegList(a2)
	move.l	a2,a1
	jsr	_LVOAddLibrary(a6)	; a1->Library
init_Exit:
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
*
* If we are not on-disk, we don't Expunge
*
	IFD	ON_DISK

*	;-- see if any one is using the library
	TST.W   LIB_OPENCNT(A6)
	BNE.S   Exp_InUse
;
	move.l	a6,-(sp)	; save library ptr on stack
	move.l	MathIEEEBase_SysLib(a6),a6
	jsr	_LVOForbid(a6)
	move.l	(sp)+,a6	; restore library ptr

*	;-- this is really it.  Free up all the resources
	MOVE.L	A6,A1
        REMOVE

*	    ;------ deallocate Math library data
	move.l	MathIEEEBase_SegList(a6),-(sp)	; Save return value
	MOVE.L  A6,A1
	MOVEQ	#0,D0
	MOVE.W  LIB_NEGSIZE(A6),D0
	SUB.W	D0,A1
	ADD.W	LIB_POSSIZE(A6),D0
; no longer need a6 as ptr to MathLibraryBase
	move.l	MathIEEEBase_SysLib(a6),a6
	jsr	_LVOFreeMem(a6)
	jsr	_LVOPermit(a6)

	move.l	(sp)+,d0			; get seglist from stack
	RTS

Exp_InUse:
	ENDC

	MOVEQ   #0,D0	    ;still in use
	RTS


**  TITLE   "--Library-- Open"
*------ Open -------------------------------------
*
*   NAME
*	Open - a request to open the library
*
*   SYNOPSIS
*	Open(), mathieeesingbaslib
*		A6
*
*   FUNCTION
*	The open routine grants access to the library.	The
*	library open count will be incremented.	 The library cannot
*	be expunged unless this open is matched by a Close.
*
*---------------------------------------------------------------------

MIOpen:	ADDQ.W  #1,LIB_OPENCNT(A6)		* Bump library open counter
	move.l	MathIEEEBase_TaskOpenLib(a6),a0
	jsr	(a0)
    	MOVE.L  A6,D0
    	RTS

**  TITLE   "--Library-- Close"
*------ Close -------------------------------------
*
*   NAME
*	Close - terminate access to a library
*
*   SYNOPSIS
*	Close(), mathieeesingbaslib
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
	SUBQ.W  #1,LIB_OPENCNT(A6)
my_rts:	moveq.l	#0,d0
closed:	RTS

	end
