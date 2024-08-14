*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		   Library Support		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
* $Id: libraries.asm,v 39.9 92/05/20 11:57:22 mks Exp $
*
* $Log:	libraries.asm,v $
* Revision 39.9  92/05/20  11:57:22  mks
* Changed autodoc for TaggedOpenLibrary()
* 
* Revision 39.8  92/05/01  16:16:29  mks
* Added FindResident() InitResident() to the chain of TaggedOpenLibrary()
* such that graphics and bootmenu can get smaller.
*
* Revision 39.7  92/04/16  08:46:30  mks
* Changed the way the copyright tags are done
*
* Revision 39.6  92/04/15  13:41:30  mks
* Added the support for getting the system copyright string
*
* Revision 39.5  92/04/13  12:59:01  mks
* Added workbench to the tags and now uses a single global '.library'
* from part of the name 'exec.library'  (<*grin*>)
*
* Revision 39.4  92/04/13  12:14:47  mks
* Added workbench to the list of tagged libraries
*
* Revision 39.3  92/04/10  15:30:56  mks
* Change the library numbers to start with 1
*
* Revision 39.2  92/04/09  09:11:51  mks
* Now has tagged openlibrary (for ROM space...)
*
* Revision 39.1  92/02/19  15:44:57  mks
* Changed ALERT macro to MYALERT
*
* Revision 39.0  91/10/15  08:27:41  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"
    INCLUDE	"constants.i"

    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"lists.i"
    INCLUDE	"libraries.i"
    INCLUDE	"memory.i"
    INCLUDE	"alerts.i"
    INCLUDE	"interrupts.i"
    INCLUDE	"execbase.i"
    INCLUDE	"ables.i"
    INT_ABLES

    INCLUDE	"calls.i"
    LIST


;****** Imported Globals *********************************************

    TASK_ABLES

    EXTERN_BASE LibList


;****** Exported Functions *******************************************

    XDEF	AddLibrary
    XDEF	RemLibrary
    XDEF	RemDevice	;echo of RemLibrary
    XDEF	OpenLibrary
    XDEF	OldOpenLibrary
    XDEF	CloseLibrary
    XDEF	SetFunction
    XDEF	SetFunction8
    XDEF	SumLibrary
    XDEF	MakeLibrary
    XDEF	MakeFunctions


;****** Imported Functions *******************************************

    EXTERN_CODE Remove
    EXTERN_CODE AddNode
    EXTERN_CODE FindName

    EXTERN_SYS	AllocMem
    EXTERN_SYS	Alert
    EXTERN_SYS	FreeMem
    EXTERN_SYS	AddLibrary
    EXTERN_SYS	OpenLibrary
    EXTERN_SYS	FindName
    EXTERN_SYS	SumLibrary
    EXTERN_SYS	InitStruct
    EXTERN_SYS	FindResident
    EXTERN_SYS	InitResident

    EXTERN_SYS	CacheClearU


*****o* exec.library/RemLibrary ********************************************
*
*   NAME
*	RemLibrary -- remove a library from the system
*
*   SYNOPSIS
*	error = RemLibrary(library)
*	D0		   A1
*
*   FUNCTION
*	This function removes an existing library from the system.
*	It will delete it from the system library name list, so no
*	new opens may be performed.
*
*   INPUTS
*	library - pointer to a library node structure
*
*   RESULTS
*	error - zero if successful, else an error number
*
*   SEE ALSO
*	AddLibrary
*
**********************************************************************
*
*   IMPLEMENTATION
*	Two options are recommended -- either delaying the actual
*	expunging of the library until all users have closed it, or
*	replacing all entry points will "null" routines that always
*	return errors.
*
*	The DISABLE/ENABLE pair is essential, because the library
*	will free up its memory.  This is safe ONLY because we are
*	assured that no one may allocate that memory before we exit
*	from the library proper.
*

RemDevice:
RemLibrary:
	    MOVE.L  A1,D0
	    FORBID

******o*    BSR     Remove		* does not affect D0
;------- call the library's expunge point:
	    LINKLIB LIB_EXPUNGE,D0
;------- return whatever the expunge routine did

	    JMP_PERMIT


*****o* exec.library/OpenLibrary *******************************************
*
*   NAME
*	OpenLibrary -- gain access to a library
*
*   SYNOPSIS
*	library = OpenLibrary(libName, version)
*	D0		      A1       D0
*
*   FUNCTION
*	This function returns a pointer to a library that was
*	previously installed into the system.  If the requested
*	library is exists, and if the library version is greater
*	than or equal to the requested version, then the open
*	will succeed.
*
*   INPUTS
*	libName - the name of the library to open
*
*	version - the version of the library required.
*
*   RESULTS
*	library - a library pointer for a successful open, else zero
*
*   SEE ALSO
*	CloseLibrary
*
**********************************************************************

*****o* exec.library/OldOpenLibrary *******************************************
*
*   NAME
*	OldOpenLibrary -- obsolete OpenLibrary
*
*   SYNOPSIS
*	library = OldOpenLibrary(libName)
*	D0			 A1
*
*   FUNCTION
*	The 1.0 release of the Amiga system had an incorrect
*	version of OpenLibrary that did not check the version
*	number during the library open.  This obsolete function
*	is provided so that object code compiled using a 1.0 system
*	will still run.
*
*   INPUTS
*	libName - the name of the library to open
*
*   RESULTS
*	library - a library pointer for a successful open, else zero
*
*   SEE ALSO
*	CloseLibrary
*
**********************************************************************

*****i* exec.library/TaggedOpenLibrary ****************************************
*
*   NAME
*	TaggedOpenLibrary -- Special ROM OpenLibrary (ROM Module use only)
*
*   SYNOPSIS
*	library = TaggedOpenLibrary(libTag)
*	d0                          d0
*
*   FUNCTION
*	This function is for system ROM modules such that they can open
*	libraries without each needing to have the name string and
*	version number.  The only libraries supported are system ROM
*	libraries...
*
*	If the library does not open by just calling OpenLibrary(),
*	this function will then FindResident(), InitResident(), and
*	then calls OpenLibrary() again.  (Again, a ROM space system-only
*	feature.)
*
*	Special case:  If d0 is < 0, this function will return a pointer
*	to the Copyright strings in ROM.
*
*   INPUTS
*	libTag - Magic tag values for the various libraries...
*
*   RESULTS
*	library - a library pointer for a successful open, else zero
*	          or the EXEC Copyright string.
*
*   SEE ALSO
*	CloseLibrary
*
*******************************************************************************
TaggedNames:	dc.b	0,0		; Skip tag=0...
		dc.b	'graphics',0	; OLTAG_GRAPHICS=1
		dc.b	'layers',0	; OLTAG_LAYERS=2
		dc.b	'intuition',0	; OLTAG_INTUITION=3
		dc.b	'dos',0		; OLTAG_DOS=4
		dc.b	'icon',0	; OLTAG_ICON=5
		dc.b	'expansion',0	; OLTAG_EXPANSION=6
		dc.b	'utility',0	; OLTAG_UTILITY=7
		dc.b	'keymap',0	; OLTAG_KEYMAP=8
		dc.b	'gadtools',0	; OLTAG_GADTOOLS=9
		dc.b	'workbench',0	; OLTAG_WORKBENCH=10
;
; -1 to -4 ar the Copyright strings...
;
		cnop	0,2

TaggedMaxName:	equ	24		; Maximum total library name length...
		xref	LibraryStr	; '.library'
		xref	TitleStr	; Copyright string...

TaggedOpenLibrary:	xdef	TaggedOpenLibrary
			; Note that this ROM private...
		tst.l	d0			; Check for negative
		bpl.s	Tagged_Normal		; If not negative, we do normal
		lea	TitleStr(pc),a0		; Set up pointer
		not.l	d0			; Invert the number...
Tagged_Search:	move.b	(a0)+,d1		; Check the next byte
		bne.s	Tagged_Search		; Keep looking...
		dbra.s	d0,Tagged_Search	; Next string...
		move.l	a0,d0			; Set result...
		rts				; Exit...
*
Tagged_Normal:	lea	TaggedNames(pc),a0	; Get string table...
		bsr.s	Tagged_Search		; Look for the string...
		lea	-TaggedMaxName(sp),sp	; Space for any of the strings
		move.l	sp,a1			; Get string buffer...
Tagged_Copy:	move.b	(a0)+,(a1)+		; Copy library name
		bne.s	Tagged_Copy
		subq.l	#1,a1			; Back up one...
		lea	LibraryStr(pc),a0	; get '.library'
Tagged_Copy2:	move.b	(a0)+,(a1)+
		bne.s	Tagged_Copy2		; Add on '.library'
		move.l	sp,a1			; Get string buffer again...
		;
		; We use OldOpenLibrary() get us into OpenLibrary via the stub
		;
		bsr.s	OldOpenLibrary		; Open it...
		tst.l	d0			; Did it open?
		bne.s	Tagged_Open		; Yes, so we are done!
		move.l	sp,a1			; Get name again
		JSRSELF	FindResident		; Find it...
		tst.l	d0			; Did we find it?
		beq.s	Tagged_Open		; Fail the open if not found...
		move.l	d0,a1			; Get resident structure
		moveq.l	#0,d1			; Clear segList...
		JSRSELF	InitResident		; Do it again...
		move.l	sp,a1			; Get name again...
		bsr.s	OldOpenLibrary		; Try again to open it...
Tagged_Open:	lea	TaggedMaxName(sp),sp	; Restor stack
		rts				; Return with d0 set...

OldOpenLibrary:
	    CLEAR   D0
	    JMPSELF OpenLibrary ;Go via. OpenLibrary vector

OpenLibrary:
	    MOVE.L  D2,-(SP)
	    MOVE.L  D0,D2

	    NPRINTF 990,<'OpenLibrary of %s - ver %ld'>,a1,d0
	    ;------ search for library name
	    LEA.L    LibList(A6),A0

	    FORBID

ol_next:    MOVE.L  A1,-(SP)
	    JSRSELF FindName	; Use vector for case-insensitive patch!
	    MOVE.L  (SP)+,A1

	    TST.L   D0
	    BEQ.S   ol_exit		* D0 is 0 when exiting
	    MOVE.L  D0,A0

	    CMP.W   LIB_VERSION(A0),D2
	    BGT.S   ol_next

	    ;------ call open entry point
	    ;------ we will return whatever the open returns:
	    LINKLIB LIB_OPEN,A0 	* (version is in D2)

ol_exit:    MOVE.L  (SP)+,D2
	    JMP_PERMIT


*****o* exec.library/CloseLibrary ******************************************
*
*   NAME
*	CloseLibrary -- conclude access to a library
*
*   SYNOPSIS
*	CloseLibrary(library)
*		     A1
*
*   FUNCTION
*	This function informs the system that access to the given
*	library has been concluded.  The user must not reference
*	the library or any routine in the library after this close.
*
*	Starting with V36, it is safe to pass a NULL instead of
*	a library pointer.
*
*   INPUTS
*	library - pointer to a library node
*
*   SEE ALSO
*	OpenLibrary
*
**********************************************************************

CloseLibrary:
*	    ------- call library close entry point
	    MOVE.L  A1,D0		;:bryce
	    BEQ.S   cl_RTS
	    FORBID
	    LINKLIB LIB_CLOSE,A1
	    JMP_PERMIT


*****o* exec.library/SetFunction ******************************************
*
*   NAME
*	SetFunction -- change a function vector in a library
*
*   SYNOPSIS
*	oldFunc = SetFunction(library, funcOffset, funcEntry)
*	D0		      A1       A0.W	   D0
*
*   FUNCTION
*	SetFunction is a functional way of changing those parts of
*	a library that are checksummed.  They are changed in such a
*	way that the summing process will never falsely declare a
*	library to be invalid.
*
*   INPUTS
*	library - a pointer to the library to be changed
*
*	funcOffset - the offset that FuncEntry should be put at.
*
*	funcEntry - pointer to new function
*
*********************************************************************

SetFunction8:	;A relic that was never needed.
SetFunction:
	    FORBID
	    BSET    #LIBB_CHANGED,LIB_FLAGS(A1)
	    LEA     0(A1,A0.W),A0
	    MOVE.L  2(A0),-(SP)		;Old vector
	    MOVE.W  #JMPINSTR,(A0)
	    MOVE.L  D0,2(A0)
	    MOVE.L  A1,-(SP)
	    JSRSELF CacheClearU		;Flush cache IN SAME DISABLE
	    BSR_PERMIT
	    MOVE.L  (SP)+,A1
	    JSRSELF SumLibrary	;(A1)
	    MOVE.L  (SP)+,D0		;Old vector
cl_RTS:     RTS


*****o* exec.library/AddLibrary ********************************************
*
*   NAME
*	AddLibrary -- add a library to the system
*
*   SYNOPSIS
*	AddLibrary(library)
*		   A1
*
*   FUNCTION
*	This function adds a new library to the system making it
*	available to other programs.  The library should be ready to be
*	opened at this time.  It will be added to the system
*	library name list, and the checksum on the library entries
*	will be calculated.
*
*   INPUTS
*	library - pointer to a properly initialized library structure
*
*   SEE ALSO
*	RemLibrary, CloseLibrary, OpenLibrary, MakeLibrary
*
**********************************************************************

AddLibrary: LEA     LibList(A6),A0
	    BSR     AddNode
	    JMPSELF SumLibrary		;(A1)

*****o* exec.library/SumLibrary *******************************************
*
*   NAME
*	SumLibrary -- compute and check the checksum on a library
*
*   SYNOPSIS
*	SumLibrary(library)
*		   A1
*
*   FUNCTION
*	SumLibrary computes a new checksum on a library.  It can
*	also be used to check an old checksum.	If an old checksum
*	does not match and the library has not been marked as
*	changed then the system will alert the user.
*
*   INPUTS
*	library - a pointer to the library to be changed
*
*   EXCEPTIONS
*	An alert will occur if the checksum fails.
*
*********************************************************************

SumLibrary:
*	    ------- see if this library should be checksummed:
	    BTST    #LIBB_SUMUSED,LIB_FLAGS(A1)
	    BEQ.S   sl_exit

	    FORBID

*	    ------- do we know we've changed?
	    BCLR    #LIBB_CHANGED,LIB_FLAGS(A1)
	    BEQ.S   sl_top
	    CLR.W   LIB_SUM(A1)         * used as an indicator

*	    ------- calculate sum:
sl_top:
	    MOVE.L  A1,A0		* pointer to entry to sum
	    MOVE.W  LIB_NEGSIZE(A1),D0
	    LSR.W   #1,D0		* loop counter
	    CLEAR   D1
	    BRA.S   sl_start
sl_loop:
	    ADD.W   -(A0),D1
sl_start:   DBF     D0,sl_loop

	    MOVE.W  LIB_SUM(A1),D0
	    BEQ.S   sl_newSum

*	    ------- compare the sum to what we got:
	    CMP.W   D0,D1
	    BEQ.S   sl_permit
	    MYALERT   AN_LibChkSum

sl_newSum:
	    MOVE.W  D1,LIB_SUM(A1)

sl_permit:
	    BSR_PERMIT
sl_exit:
	    RTS


*****o* exec.library/MakeLibrary *******************************************
*
*   NAME
*	MakeLibrary -- construct a library
*
*   SYNOPSIS
*	library = MakeLibrary(vectors, structure, init, dataSize, segList)
*	D0		      A0       A1	  A2	D0	  D1
*
*   FUNCTION
*	This function is used for constructing a library vector and
*	data area.  Space for the library is allocated from the
*	system's free memory pool. The size fields of the library
*	are filled.  The data portion of the library is
*	initialized.  A library specific entrypoint is called
*	(init) if present.
*
*   INPUTS
*	vectors - pointer to an array of function pointers or
*		function displacements.  If the first word of the
*		array is -1, then the array contains relative word
*		displacements (based off of vectors); otherwise,
*		the array contains absolute function pointers.
*		The vector list is terminated by a -1 (of the same
*		size as the pointers).
*
*	structure - points to an "InitStruct" data region.  If null,
*		then it will not be called.
*
*	init - an entry point that will be called before adding
*		the library to the system.  If null, it will not be
*		called.  When it is called, it will be called with:
*			d0 = libAddr	;Your Library Address
*			a0 = segList	;Your AmigaDOS segment list
*			a6 = ExecBase	;Address of exec.library
*		The result of the init function will be the result
*		returned by MakeLibrary.
*
*	dSize - the size of the library data area, including the
*		standard library node data.
*
*	segList - pointer to a memory segment list (used by DOS)
*		This is passed to a library's init code.
*
*   RESULT
*	library - the reference address of the library.  This is
*		the address used in references to the library, not
*		the beginning of the memory area allocated.
*
*   EXCEPTION
*	If the library vector table require more system memory
*	than is available, this function will cause a system panic.
*
*   SEE ALSO
*	InitStruct
*
**********************************************************************
*	vSize - the size of the library vector area.  This is the
*		actual size in bytes, not longwords.  If null,
*		then the size of this area is calculated from the
*		vector initialization array ("vectors" above).
*
*	library = MakeLibrary(vectors, structure, init, dataSize, segList)
*	D0		      A0       A1	  A2	D0	  D1
*		D2-dataSize
*		D4-vectors
*		D5-structure
*		D6-init
*		D7-segList

MakeLibrary:
	    MOVEM.L D2-D7/A2-A3,-(SP)

	    MOVE.L  D0,D2
	    MOVE.L  A0,D4
	    MOVE.L  A1,D5
	    MOVE.L  A2,D6
	    MOVE.L  D1,D7

	    MOVE.L  A0,D3
	    BEQ.S   ml_noAdjust 	* function array not provided

	    MOVE.L  A0,A3
	    MOVEQ   #-1,D3
	    MOVE.L  D3,D0
	    CMP.W   (A3),D0
	    BNE.S   ml_longCnt

	    ADDQ.L  #2,A3
ml_lenCnt:  CMP.W   (A3)+,D0
	    DBEQ    D3,ml_lenCnt
	    BRA.S   ml_calcSize

ml_longCnt: CMP.L   (A3)+,D0
	    DBEQ    D3,ml_longCnt
ml_calcSize:
	    NOT.W   D3
	    MULU    #6,D3
	    ADDQ.L  #3,D3	;Round to longword for 68020
	    ANDI.W  #~3,D3

ml_noAdjust:
*	    ------- allocate system global memory
	    MOVE.L  D2,D0
	    ADD.L   D3,D0		* get total memory required
	    MOVE.L  #MEMF_PUBLIC!MEMF_CLEAR,D1
	    JSRSELF AllocMem
*	------- changed from ALERT to error return. 8 sep 85, Neil
	    TST.L   D0
	    beq.s   ml_done

*	    ------- initialize library data size entries
ml_memOK:
	    MOVE.L  D0,A3		* get library memory
	    ADD.L   D3,A3		* adjust to reference address

	    MOVE.W  D3,LIB_NEGSIZE(A3)  * set library function size
	    MOVE.W  D2,LIB_POSSIZE(A3)  *   and library data size

*	    ------- initialize the library function memory
	    MOVE.L  A3,A0		* target
	    SUB.L   A2,A2		* clear relative Base
	    MOVE.L  D4,A1		* functions
	    CMP.W   #-1,(A1)
	    BNE.S   ml_longs
	    ADDQ.L  #2,A1		* skip "-1" word
	    MOVE.L  D4,A2		* relative base
ml_longs:
	    BSR.S   MakeFunctions	;(A0,A1,A2)

*	    ------- Initialize the structures
	    TST.L   D5
	    BEQ.S   ml_callInit
	    MOVE.L  A3,A2		* target
	    MOVE.L  D5,A1		* table
	    CLEAR   D0			* don't clear (we allocated cleared)
	    JSRSELF InitStruct

ml_callInit:
*	    ------- Call the library initialization entry point
	    MOVE.L  A3,D0		* default return value
	    TST.L   D6
	    BEQ.S   ml_done
	    MOVE.L  D6,A1
	    MOVE.L  D7,A0
	    JSR     (A1)
	;---------------------------------------------------------
	; Calling Sequence:
	;   library = InitLib (base, segList),a6
	;   d0		       d0    a0       execbase
	;---------------------------------------------------------

*	    ------- Return whatever the libInit returns

ml_done:
	    MOVEM.L (SP)+,D2-D7/A2-A3
	    RTS


*****o* exec.library/MakeFunctions ****************************************
*
*   NAME
*	MakeFunctions -- construct a function jump table
*
*   SYNOPSYS
*	tableSize = MakeFunctions(target, functionArray, funcDispBase)
*	D0			  A0	  A1		 A2
*
*   FUNCTION
*	This function constructs a function jump table of the
*	type used by resources, libraries, and devices.  It
*	allows the table to be built anywhere in memory, and
*	can be used both for initialization and replacement.
*	This function also supports function pointer compression
*	by expanding relative displacements into absolute pointers.
*
*   INPUT
*	destination - the target address for the function jump
*		table.
*
*	functionArray - pointer to an array of function pointers
*		or function displacements.  If funcDispBase is
*		zero, the array is assumed to contain absolute
*		pointers to functions.	If funcDispBase is not
*		zero, then the array is assumed to contain word
*		displacements to functions.  In both cases, the
*		array is terminated by a -1 (of the same size
*		as the actual entry.
*
*	funcDispBase - pointer to the base about which all function
*		displacements are relative.  If zero, then the
*		function array contains absolute pointers.
*
*   RESULT
*	tableSize - size of the new table in bytes.
*
********************************************************************

MakeFunctions:
	    MOVE.L  A3,-(SP)
	    CLEAR   D0
	    MOVE.L  A2,D1
	    BEQ.S   mf_nextAbs

mf_nextRel:
	    MOVE.W  (A1)+,D1
	    CMP.W   #-1,D1
	    BEQ.S   mf_exit
	    LEA     0(A2,D1.W),A3
	    MOVE.L  A3,-(A0)
	    MOVE.W  #JMPINSTR,-(A0)
	    ADDQ.L  #6,D0
	    BRA.S   mf_nextRel

mf_nextAbs:
	    MOVE.L  (A1)+,D1
	    CMP.L   #-1,D1
	    BEQ.S   mf_exit
	    MOVE.L  D1,-(A0)
	    MOVE.W  #JMPINSTR,-(A0)
	    ADDQ.L  #6,D0
	    BRA.S   mf_nextAbs

mf_exit:    MOVE.L  D0,A3
	    JSRSELF CacheClearU
            MOVE.L  A3,D0
	    MOVE.L  (SP)+,A3
	    RTS

    END
