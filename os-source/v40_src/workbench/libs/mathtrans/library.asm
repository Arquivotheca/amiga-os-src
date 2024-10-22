	TTL    '$Id: library.asm,v 37.1 91/01/22 14:09:21 mks Exp $'
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
*   $Id: library.asm,v 37.1 91/01/22 14:09:21 mks Exp $
*
**********************************************************************

	SECTION		mathtrans

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
	INCLUDE		'exec/nodes.i'
	INCLUDE		'exec/lists.i'
	INCLUDE		'exec/memory.i'
	INCLUDE		'exec/ports.i'
	INCLUDE		'exec/libraries.i'
	INCLUDE		'exec/devices.i'
	INCLUDE		'exec/tasks.i'
	INCLUDE		'exec/io.i'
	INCLUDE		'exec/strings.i'
	INCLUDE		'exec/initializers.i'

	INCLUDE		'mathtrans_rev.i'
	INCLUDE		'macros.i'
	INCLUDE		'library.i'


DEBUG	EQU		0							* Turn on debug statements if set


*------ Imported Globals ---------------------------------------------

	    XREF	_AbsExecBase

	    XREF	MFName


*------ Imported Functions -------------------------------------------

	    XREF_EXE	MakeLibrary				* Generates:  XREF	_LVOMakeLibrary
	    XREF_EXE	AddLibrary				* Generates:  XREF  _LVOAddLibrary
	    XREF_EXE	FreeMem					* Generates:  XREF  _LVOFreeMem
	    XREF_EXE	NoFunc					* Generates:  XREF  _LVONoFunc

	    XREF	_CLInit

		XREF	_OpenLibrary
		XREF	_CloseLibrary

	IFNE	DEBUG
		XREF	_kprintf
	ENDC


*------ Exported Globals ---------------------------------------------

	    XDEF	_SysBase
	    XDEF	_MathBase
	    XDEF	_MathSegList


*------ Exported Functions -------------------------------------------

	    XDEF	MFInit


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
MFInit:
		    DC.L    MF_SIZE
		    DC.L    libFuncInit
		    DC.L    libStructInit
		    DC.L    libInit2

libInit2:
*           ;------ This is called from within MakeLibrary, after
*           ;------ all the memory has been allocated

MFExtFunc:
		PRINT	1

		MOVE.L	A0,_MathSegList
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
		INITLONG    LN_NAME,MFName
		INITWORD	LIB_REVISION,REVISION
		INITWORD	LIB_VERSION,VERSION
		INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_OPENCNT,0

		DC.W	    0

FUNCDEF		MACRO
	XREF	MF\1
		DC.L    MF\1
		ENDM


libFuncInit:
		DC.L	MFOpen
		DC.L	MFClose
		DC.L	MFExpunge
		DC.L	MFExtFunc	; currently an RTS

	INCLUDE		'mathtrans_lib.i'

		DC.L	-1


**  TITLE   "--Library-- Expunge"
*------ Transcendental Math Expunge ------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove the Transcendental Math library
*
*   SYNOPSIS
*	Result = Expunge(), audioLib
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

MFExpunge:
			PRINT	2

*	    ;-- see if any one is using the library
		    TST.W   LIB_OPENCNT(A6)
		    BNE.S   Exp_InUse

			PRINT	3

*	    ;-- this is really it.  Free up all the resources
		    MOVE.L	A6,A1
            REMOVE

*	    ;------ deallocate Transcendental Math library data
		    MOVE.L  A6,A1
			MOVEQ	#0,D0
		    MOVE.W  LIB_NEGSIZE(A6),D0
			SUB.W	D0,A1
			ADD.W	LIB_POSSIZE(A6),D0
		    LINKEXE FreeMem

		    MOVE.L   _MathSegList,D0
		    RTS

Exp_InUse:
			PRINT 4

		    BSET    #MFB_EXPUNGED,LIB_FLAGS(A6)
		    MOVEQ   #0,D0	    ;still in use
		    RTS


**  TITLE   "--Library-- Open"
*------ Transcendental Math Open -------------------------------------
*
*   NAME
*	Open - a request to open the Transcendental Math library
*
*   SYNOPSIS
*	Open(), audioLib
*		A6
*
*   FUNCTION
*	The open routine grants access to the Transcendental Math library.	The
*	library open count will be incremented.	 The library cannot
*	be expunged unless this open is matched by a Close.
*
*---------------------------------------------------------------------
MFOpen:
			PRINT	5

		    BTST    #MFB_EXPUNGED,LIB_FLAGS(A6)
		    BNE.S   openFail

			PRINT	6

		    ADDQ.W  #1,LIB_OPENCNT(A6)		* Bump library open counter

			CMP.W	#1,LIB_OPENCNT(A6)		* Check for first RAM library open
			BNE.S	NoROMOpen

			PRINT	7

			MOVE.L	_AbsExecBase,_SysBase	* Set up _SysBase before open

			CLR.L	-(SP)					* Push 2nd arg = 0
			PEA		MLIBNAM					* Push 1st arg = pointer to basic math library name
			JSR		_OpenLibrary			* Go open the basic math library (maybe already opened)
			ADDQ.L	#8,SP					* Clean up stack after return
			MOVE.L	D0,_MathBase			* Set up the _MathBase pointer

NoROMOpen:
			PRINT	8

		    MOVE.L  A6,D0
		    RTS

openFail:
			PRINT	9

		    MOVEQ   #-1,D0
		    RTS


**  TITLE   "--Library-- Close"
*------ Transcendental Math Close -------------------------------------
*
*   NAME
*	Close - terminate access to a library
*
*   SYNOPSIS
*	Close(), mathlibrary
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
MFClose:
			PRINT	10

			MOVEQ.L	#0,D0					* Clear D0 for possible return
		    SUBQ.W  #1,LIB_OPENCNT(A6)

*	    ;-- check if this library should now be expunged
		    BNE.S   closed

			PRINT	11

		    BTST    #MFB_EXPUNGED,LIB_FLAGS(A6)
		    BEQ.S   closed

			PRINT	12

		    BSR	    MFExpunge

closed:
			PRINT	13

			MOVE.L	D0,-(SP)				* Save D0 (from Expunge) for CloseLibrary call
			TST.W	LIB_OPENCNT(A6)			* Test for last close call this library
			BNE.S	NoROMClose

			PRINT	14

			MOVE.L	_MathBase,-(SP)			* Push _MathBase pointer for close call
			JSR		_CloseLibrary			* Go close the basic math library
			ADDQ.L	#4,SP					* Clean up stack
NoROMClose:
			PRINT	15

			MOVE.L	(SP)+,D0				* Restore D0 from Expunge call
		    RTS

*
*	Data Definition Region
*

_SysBase:
		DC.L	0

_MathBase:
		DS.L	1							* int MathBase for ROM library access

_MathSegList:
		DC.L	0

MLIBNAM:
		DC.B	'mathffp.library'			* Basic math library name
		DC.B	0							* Null terminator for C

	IFNE	DEBUG
msg1:	DC.B	'Message # 1  ',13,10,0
msg2:	DC.B	'Message # 2  ',13,10,0
msg3:	DC.B	'Message # 3  ',13,10,0
msg4:	DC.B	'Message # 4  ',13,10,0
msg5:	DC.B	'Message # 5  ',13,10,0
msg6:	DC.B	'Message # 6  ',13,10,0
msg7:	DC.B	'Message # 7  ',13,10,0
msg8:	DC.B	'Message # 8  ',13,10,0
msg9:	DC.B	'Message # 9  ',13,10,0
msg10:	DC.B	'Message # 10  ',13,10,0
msg11:	DC.B	'Message # 11  ',13,10,0
msg12:	DC.B	'Message # 12  ',13,10,0
msg13:	DC.B	'Message # 13  ',13,10,0
msg14:	DC.B	'Message # 14  ',13,10,0
msg15:	DC.B	'Message # 15  ',13,10,0
	ENDC



		    END
