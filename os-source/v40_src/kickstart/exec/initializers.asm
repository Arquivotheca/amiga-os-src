*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **  Cold Initialization (ROMTags & KickMem)   **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,90 Commodore-Amiga, Inc.			*
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
*   Source Control:
*
*	$Id: initializers.asm,v 39.1 92/02/19 15:54:28 mks Exp $
*
*	$Log:	initializers.asm,v $
* Revision 39.1  92/02/19  15:54:28  mks
* short branch optimizations taken
* 
* Revision 39.0  91/10/15  08:27:20  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

	NOLIST
	INCLUDE "assembly.i"
	INCLUDE "types.i"
	INCLUDE "calls.i"

	INCLUDE "nodes.i"
	INCLUDE "lists.i"
	INCLUDE "tasks.i"
	INCLUDE "libraries.i"
	INCLUDE "interrupts.i"
	INCLUDE "execbase.i"
	INCLUDE "memory.i"
	INCLUDE "resident.i"
	LIST

;****** Exported Functions *******************************************

	XDEF	FindCodeBefore
	XDEF	FindCodeAfter
	XDEF	SumKickData
	XDEF	FindResident
	XDEF	InitCode
	XDEF	InitResident
	XDEF	InitStruct

;****** Imported Functions *******************************************

    EXTERN_CODE Enqueue
    EXTERN_CODE Remove
    EXTERN_CODE FindName
    EXTERN_CODE RawPutStr

    EXTERN_SYS	AddDevice
    EXTERN_SYS	AddLibrary
    EXTERN_SYS	AddResource
    EXTERN_SYS	Alert
    EXTERN_SYS	AllocAbs
    EXTERN_SYS	CmpStr
    EXTERN_SYS	InitResident
    EXTERN_SYS	MakeLibrary
    EXTERN_SYS	AllocMem
    EXTERN_SYS	FreeMem
    EXTERN_SYS	AvailMem



*****i* exec.library/FindCode ********************************************
*
*   SYNOPSIS
*	lastTagEntry = FindCodeBefore(scanBounds)	;before expansion
*	D0			      A0
*
*	lastTagEntry = FindCodeAfter(scanBounds)	;after expansion
*	D0			     A0
*
*   DESCRIPTION
*	Memory regions will be scanned for resident tags.  Valid
*	tags will be stored in the tagArray.  Entries will be in
*	priority sorted order.	If two tags have the same name,
*	FindCode opts for the tag with the greater version number.
*	If equal version numbers are found, the tag with the higher
*	priority will be used.	Equal priority goes to the latest.
*
*   INPUTS
*	scanBounds - an array of lower/upper bounds pairs to
*		scan through.  Array is -1 terminated.
*
*   RESULT
*	base of ResidentArray, null terminated (last entry is always zero).
*
**************************************************************************
*
*	The common code below could be optimized.  No time now!
*	:OPTIMIZE:
*

 STRUCTURE TMP,LN_SIZE
    APTR    TMP_ADDR
    LABEL   TMP_SIZE

;Findcode including KickMemLists
FindCodeAfter:
	    MOVEM.L D3/D4/A2/A3/A4,-(SP)
	    LINK    A5,#-LH_SIZE
	    MOVE.L  SP,A3
	    NEWLIST A3
	    MOVE.L  A0,A2

fc_nextRegion:
	    MOVE.L  (A2)+,A4            * starting address
	    MOVEQ   #-1,D0
	    CMP.L   D0,A4
	    BEQ.S   fc_addkicktags	* fixed end-of-list test -Bryce
	    MOVE.L  (A2)+,D4            * ending address
	    BSR.S   searchForTags (a3,a4,d4)
	    BRA.S   fc_nextRegion

fc_addkicktags:
	    BSR     SumKickData
	    CMP.L   KickCheckSum(A6),D0
	    BNE.S   fc_buildarray

	    BSR     AllocKickMem
	    TST.L   D0
	    BEQ.S   fc_buildarray

	    BSR     SpliceRomTagList (a3)

fc_buildarray:
	    BSR     buildResidentArray (a3)

	    UNLK    A5
	    MOVEM.L (SP)+,D3/D4/A2/A3/A4
	    RTS


;Findcode excluding KickMemLists
FindCodeBefore:
	    MOVEM.L D3/D4/A2/A3/A4,-(SP)
	    LINK    A5,#-LH_SIZE
	    MOVE.L  SP,A3
	    NEWLIST A3
	    MOVE.L  A0,A2

fcb_nextRegion:
	    MOVE.L  (A2)+,A4            * starting address
	    MOVEQ   #-1,D0
	    CMP.L   D0,A4
	    BEQ.S   fcb_buildarray	* fixed end-of-list test -Bryce
	    MOVE.L  (A2)+,D4            * ending address
	    BSR.S   searchForTags (a3,a4,d4)
	    BRA.S   fcb_nextRegion

fcb_buildarray:
	    BSR     buildResidentArray (a3)

	    UNLK    A5
	    MOVEM.L (SP)+,D3/D4/A2/A3/A4
	    RTS



*** Support Routines *****

searchForTags:	; ( A3:list head, A4:starting addr, D4:end addr )

	    MOVEM.L D2/A5,-(SP) ; holds current node

	    MOVE.W  #RTC_MATCHWORD,D2

sft_calc:
	    ;------ compute size of region in bytes
	    MOVE.L  D4,D0		* end
	    SUB.L   A4,D0
	    BLS.S   sft_end

	    ;------ convert to number of words, in two separate words
	    LSR.L   #1,D0
	    SUBQ.L  #1,D0
	    MOVE.L  D0,D1
	    SWAP    D1
	    BRA.S   sft_searchentry

	    ;------ scan for matchword:
sft_search:
	    CMP.W   (A4)+,D2
sft_searchentry:
	    DBEQ    D0,sft_search
	    DBEQ    D1,sft_search

	    BNE.S   sft_end	* region exhausted

	    ;------ check for backpointer
	    LEA     -2(A4),A5
	    CMP.L   (A4),A5
	    BNE.S   sft_searchentry

	    bsr.s   addRomTag	(D0,A3)

	    ;------ skip this module
	    MOVE.L  RT_ENDSKIP(A5),A4   * continuation point
	    BRA.S   sft_calc

sft_end:
	    MOVEM.L (SP)+,D2/A5
	    RTS


*******************************

addRomTag:	; ( a3:list head, a5:current node )

	    ;------ scan sorted list for same name:
	    MOVE.L  A3,A0
	    MOVE.L  RT_NAME(A5),A1
	    BSR     FindName
	    TST.L   D0
	    BEQ.S   art_allocNode

	    ;------ the name already existed.  see if it is newer
	    MOVE.L  D0,A1
	    MOVE.L  TMP_ADDR(A1),A0
	    MOVE.B  RT_VERSION(A5),D0
	    CMP.B   RT_VERSION(A0),D0
	    blt.s   art_end		* lesser version
	    BGT.S   art_trade		* greater version

	    ;------ the version was equal.  check the priority
	    MOVE.B  RT_PRI(A5),D0
	    CMP.B   RT_PRI(A0),D0
	    BLT.S   art_end		* lesser priority

art_trade:
	    ;------ there was an older one.  replace it.
	    MOVE.L  A1,D0
	    BSR     Remove (A1)
	    MOVE.L  D0,A1
	    BRA.S   art_enqueue 	* reuse node

	    ;------ allocate a new node:
art_allocNode:
	    CLEAR   D1
	    MOVEQ   #TMP_SIZE,D0
	    JSRSELF AllocMem
	    TST.L   D0
	    BEQ.S   art_end
	    MOVE.L  D0,A1

	    ;------ save in a sorted list:
art_enqueue:
	    MOVE.B  RT_PRI(A5),LN_PRI(A1)
	    MOVE.L  RT_NAME(A5),LN_NAME(A1)

	    NPRINTF 600,<'ROMTag: $%lx,%s to $%lx'>,A5,LN_NAME(A1),A1

	    MOVE.L  A5,TMP_ADDR(A1)
	    MOVE.L  A3,A0
	    BSR     Enqueue (A0,A1)

art_end:
	    RTS


***************************

buildResidentArray:	; (a2:scratch, a3:residentList, d3/d4:scratch)

	    ;------ first size the list
	    MOVEQ   #4,D0	; cell for the null termination
	    MOVE.L  (A3),D4
bra_sizeloop:
	    MOVE.L  D4,A1
	    MOVE.L  (A1),D4
	    BEQ.S   bra_alloc

	    ADDQ.L  #4,D0
	    BRA.S   bra_sizeloop

bra_alloc:
	    ;------ get the memory
	    MOVE.L  #MEMF_PUBLIC!MEMF_CLEAR,D1
	    JSRSELF AllocMem
	    MOVE.L  D0,A2
	    MOVE.L  D0,D3

	    ;------ move sorted list pointers into array:
	    MOVE.L  (A3),D4
bra_loop:
	    MOVE.L  D4,A1
	    MOVE.L  (A1),D4
	    BEQ.S   bra_end

	    ;------ assign the resident tag pointer
	    MOVE.L  TMP_ADDR(A1),(A2)+

	    ;------ and free the memory
	    MOVEQ   #TMP_SIZE,D0
	    JSRSELF FreeMem

	    BRA.S   bra_loop

bra_end:
	    ;------ terminate the list
	    CLR.L   (A2)

	    ;------ result is base of resident array
	    MOVE.L  D3,D0

	    RTS




***************************************************************************
*
* SpliceRomTag()
*
* splice the KickTagPtr onto the beginning of the rom tag list.
* If KickTagPtr is null then this is a nop
*
***************************************************************************
SpliceRomTagList:	; (a3:residentList)
		MOVEM.L A2/A5,-(SP)

		MOVE.L	KickTagPtr(A6),D0
		BEQ.S	srtl_end

		MOVE.L	D0,A2

srtl_loop:
		MOVE.L	(A2)+,D0
		BEQ.S	srtl_end
		BMI.S	srtl_splice

		MOVE.L	D0,A5
		BSR	addRomTag

		BRA.S	srtl_loop

srtl_splice
		;------ negative: splice in the next chunk
		BCLR	#31,D0	;:BUG:Magic bit should be bit zero
		MOVE.L	D0,A2
		BRA.S	srtl_loop

srtl_end:
		MOVEM.L (SP)+,A2/A5
		RTS

*****o* exec.library/SumKickData ********************************************
*
*   NAME
*	SumKickData -- compute the checksum for the kickstart delta list
*
*   SYNOPSIS
*	checksum = SumKickData()
*
*	APTR SumKickData(void);
*
*
*   FUNCTION
*	The Amiga system has some ROM (or kickstart) resident code
*	that provides the basic functions for the machine.  This
*	code is unchangeable by the system software.  This routine
*	is part of a support system to modify parts of the ROM.
*
*	The ROM code is linked together at run time via rom-tags
*	(also known as Resident structures).  These tags tell
*	exec's low level boot code what subsystems exist in which
*	regions of memory.  The current list of rom-tags is
*	contained in the ResModules field of ExecBase.	By default
*	this list contains any rom-tags found in the address ranges
*	$F80000-$FFFFFF and $F00000-$F7FFFF.
*
*	There is also a facility to selectively add or replace
*	modules to the rom-tag list.  These modules can exist in
*	RAM, and the memory they occupy will be deleted from the
*	memory free list during the boot process.  SumKickData()
*	plays an important role in this run-time modification of
*	the rom-tag array.
*
*	Three variables in ExecBase are used in changing the
*	rom-tag array: KickMemPtr, KickTagPtr, and KickCheckSum.
*	KickMemPtr points to a linked list of MemEntry structures.
*	The memory that these MemEntries reference will be
*	allocated (via AllocAbs) at boot time.  The MemEntry
*	structure itself must also be in the list.
*
*	KickTagPtr points to a long-word array of the same format
*	as the ResModules array.  The array has a series of
*	pointers to rom-tag structures.  The array is either null
*	terminated, or will have an entry with the most significant
*	bit (bit 31) set.  The most significant bit being set says
*	that this is a link to another long-word array of rom-tag
*	entries.  This new array's address can be found by clearing
*	bit 31.
*
*	KickCheckSum has the result of SumKickData().  It is the
*	checksum of both the KickMemPtr structure and the
*	KickTagPtr arrays.  If the checksum does not compute
*	correctly then both KickMemPtr and KickTagPtr will be
*	ignored.
*
*	If all the memory referenced by KickMemPtr can't be
*	allocated then KickTagPtr will be ignored.
*
*	There is one more important caveat about adding rom-tags.
*	All this rom-tag magic is run very early on in the system
*	-- before expansion memory is added to the system.
*	Therefore any memory in this additional rom-tag area must
*	be addressible at this time.  This means that your rom-tag
*	code, MemEntries, and resident arrays cannot be in
*	expansion memory.  There are two regions of memory that are
*	acceptable:  one is chip memory, and the other is "Ranger"
*	memory (memory in the range between $C00000-$D80000).
*
*	Remember that changing an existing rom-tag entry falls into
*	the "heavy magic" category -- be very careful when doing
*	it.  The odd are that you will blow yourself out of the
*	water by doing it.
*
*   RESULTS
*	checksum - The value to be stuffed into ExecBase->KickCheckSum
*
*   NOTE
*	SumKickData was introduced in the 1.2 release
*
*   SEE ALSO
*	InitResident, FindResident
*
**********************************************************************
*
* SumKickData()
*
* Compute a checksum for the kick data, following from beginning
* to end.
*

SumKickData:
		MOVEM.L D2/D3/D4,-(SP)

		;------ break mutual recursion loop:
		LEA	KickMemPtr(A6),A0
		MOVEM.L (A0),D3/D4
		CLR.L	(A0)+
		CLR.L	(A0)+

		;------ set seed
		MOVEQ	#-1,D0

		MOVE.L	D3,D2		; KickMemPtr -> D2

skd_nextmemptr:
		TST.L	D2
		BEQ.S	skd_tagptr

		MOVE.L	D2,A0
		MOVE.L	(A0),D2

		;------ get length in long words
		MOVE.W	ML_NUMENTRIES(A0),D1
		ADD.W	D1,D1
		ADD.W	#(ML_SIZE>>2),D1
		bsr.s	sumKickBlock

		BRA.S	skd_nextmemptr

skd_tagptr:
		MOVE.L	D4,D2		; KickTagPtr -> D2
		BEQ.S	skd_end

		MOVE.L	D2,A0
		BRA.S	skd_tagentry
skd_tagloop:
		ADD.L	D2,D0
skd_tagentry:
		MOVE.L	(A0)+,D2
		BEQ.S	skd_end
		BPL.S	skd_tagloop

		;------ negative: splice in the next chunk
		BCLR	#31,D2
		MOVE.L	D2,A0
		BRA.S	skd_tagentry

skd_end:
		MOVEM.L D3/D4,KickMemPtr(A6)
		MOVEM.L (SP)+,D2/D3/D4
		RTS

*******************************************

skb_loop:
		ADD.L	(A0)+,D0
sumKickBlock:	; ( d0: running sum; d1: long word count; a0: ptr to block )
		DBRA	D1,skb_loop

		RTS



***************************************************************************
*
* success = AllocKickMem()
*
* attempt to AllocAbs all the memory in the KickMemPtr mem list
*
***************************************************************************
AllocKickMem:
		MOVE.L	KickMemPtr(A6),D4

akm_nextmemlist:
		TST.L	D4
		beq.s	akm_ok

		MOVE.L	D4,A2

		NPRINTF 50,<'Processing MemList-$%lx,%s'>,A2,LN_NAME(A2)

		MOVE.L	(A2),D4         ; get next mem list
		LEA	ML_NUMENTRIES(A2),A2
		MOVE.W	(A2)+,D3        ; get num entries
		MOVEQ	#1,D0		; make sure condition codes are nonzero
		BRA.S	akm_allocabs_entry

akm_allocabs_loop:
		MOVE.L	(A2)+,A1
		MOVE.L	(A2)+,D0
		NPRINTF  50,<'AllocAbs-$%lx,%lx'>,A1,D0
		JSRSELF AllocAbs
		NPRINTF  50,<'AllocAbs result %lx'>,D0
		;NOTE: ramkick allocabs does not fail on AllocAbs error


		;------ AllocAbs returns null on error
		TST.L	D0

akm_allocabs_entry:
		DBEQ	D3,akm_allocabs_loop	; loop until equal
		BEQ.S	akm_end

		BRA	akm_nextmemlist


akm_ok:
		MOVEQ	#1,D0
akm_end:
		RTS


*****o* exec.library/FindResident **************************************
*
*   NAME
*	FindResident - find a resident module by name
*
*   SYNOPSIS
*	resident = FindResident(name)
*	D0			A1
*
*   FUNCTION
*	Find the resident module with the given name.  If found
*	return a pointer to the resident tag structure, else
*	return zero.
*
*   INPUTS
*	name - pointer to name string
*
*    RESULT
*	resident - pointer to the resident tag structure or
*		zero if none found.
*
*    SEE ALSO
*	ResidentTag (RT) structure definition (resident.h)
*
*
**********************************************************************

FindResident:
	    MOVEM.L A2-A3,-(SP)
	    MOVE.L  ResModules(A6),A2
	    MOVE.L  A1,A3

*	    ------- fetch next pointer:
ft_next:
	    MOVE.L  (A2)+,D0
	    BEQ.S   ft_exit
	    BGT.S   ft_noExtend
	    BCLR    #31,D0		* extended table pointer
	    MOVE.L  D0,A2
	    BRA.S   ft_next

*	    ------- compare names
ft_noExtend:
	    MOVE.L  D0,A1
	    MOVE.L  A3,A0
	    MOVE.L  RT_NAME(A1),A1
ft_char:    CMP.B   (A0)+,(A1)+
	    BNE.S   ft_next
	    TST.B   -1(A0)
	    BNE.S   ft_char
ft_exit:
	    MOVEM.L (SP)+,A2-A3
	    RTS


*****o* exec.library/InitCode *************************************************
*
*   NAME
*	InitCode - initialize resident code modules
*
*   SYNOPSIS
*	InitCode(startClass, version)
*		 D0	     D1
*
*   FUNCTION
*	Call InitResident() for all resident modules in the ResModules array
*	with the given startClass and with versions equal or greater than
*	that specified.  The segList parameter is passed as zero.
*	(This function may be ignored by application programmers)
*
*	Resident modules are used by the system to pull all its parts
*	together at startup.  Modules are initialized in a prioritized order.
*
*   INPUTS
*	startClass - the class of code to be initialized:
*		BITDEF RT,COLDSTART,0
*		BITDEF RT,SINGLETASK,1	;ExecBase->ThisTask will be 0
*		BITDEF RT,AFTERDOS,2
*	version - a major version number
*
*    SEE ALSO
*	ResidentTag (RT) structure definition (resident.h)
*
*******************************************************************************
*
*	Conventions:
*		  0  if you don't care
*		-100 (or lower) for RTF_AFTERDOS modules
*		-120 (or lower) for modules that will not be auto-initialized
*
InitCode:
	    MOVEM.L D2-D3/A2,-(SP)
	    MOVE.L  ResModules(A6),A2
	    MOVE.B  D0,D2
	    MOVE.B  D1,D3

*	    ------- fetch next pointer:
ic_next:
	    MOVE.L  (A2)+,D0
	    beq.s   ic_exit		* .S
	    BGT.S   ic_noExtend
	    BCLR    #31,D0		* extended table pointer
	    MOVE.L  D0,A2
	    BRA.S   ic_next
ic_noExtend:
	    MOVE.L  D0,A1
	    CMP.B   RT_VERSION(A1),D3
	    BGT.S   ic_next
	    MOVE.B  RT_FLAGS(A1),D0
	    AND.B   D2,D0
	    BEQ.S   ic_next


	    IFGE    INFODEPTH-400

            MOVE.L  A1,-(SP)
	    moveq   #MEMF_CHIP,d1
	    JSRSELF AvailMem
	    NPRINTF 399,<'C=%ld '>,d0

	    moveq   #MEMF_FAST,d1
	    JSRSELF AvailMem
	    NPRINTF 399,<'F=%ld '>,d0
            MOVE.L  (SP)+,A1

	    NPRINTF 399,<': %s'>,RT_IDSTRING(A1)

	    ENDC


	    CLEAR   D1
	    JSRSELF InitResident
	    BRA     ic_next
;------------------------------------

ic_exit:
	    MOVEM.L (SP)+,D2-D3/A2
	    RTS


*****o* exec.library/InitResident *************************************
*
*   NAME
*	InitResident - initialize resident module
*
*   SYNOPSIS
*	InitResident(resident, segList)
*		     A1        D1
*
*   FUNCTION
*	Initialize a module.
*
*   INPUTS
*
*   SEE ALSO
*	ResidentTag (RT) structure definition (resident.h)
*
*
**********************************************************************

InitResident:
	    BTST.B  #RTB_AUTOINIT,RT_FLAGS(A1)
	    BNE.S   ir_autoInit

*	    ------- call initialization routine:
	    MOVE.L  RT_INIT(A1),A1
	    CLEAR   D0
	    MOVE.L  D1,A0
	    JMP     (A1)        ;(exit)
	;---------------------------------------------------------
	; Calling Sequence:
	;   library = InitLib (base, segList),ExecBase
	;   d0		       d0=0  a0       a6
	;---------------------------------------------------------


ir_autoInit:
	    MOVEM.L D1/A1/A2,-(SP)      ;SegList,ROMTag,Scratch
	    MOVE.L  RT_INIT(A1),A1
	    MOVEM.L (A1),D0/A0-A1       ; size,vectors,structure,<init>
	    SUBA.L  A2,A2		; nuke callback vector for now
	    JSRSELF MakeLibrary
	    MOVEM.L (SP)+,A2/A0/D1      ; note: A0 not A1

	    ;------ check for MakeLibrary failure.  If failed, exit
	    MOVE.L  D0,-(SP)
	    BEQ.S   ir_retval		;NULL on stack...
	    MOVE.L  D0,A1

	    ;------ V36 addition, copy over good stuff from ROMTag -----
	    ;------ this got ugly because of the desire to retain
	    ;------ MakeLibrary patchability.  We NULL out the
	    ;------ Init vector, MakeLibrary, copy over ROMTag info,
	    ;------ then call the delayed Init vector.	*phew*
	    ;A0-ROMTag
	    ;A1-Library
	    ;D1-SegList
	    MOVE.B  RT_TYPE(A0),LN_TYPE(A1)
	    MOVE.L  RT_NAME(A0),LN_NAME(A1)
	    MOVE.B  #LIBF_SUMUSED!LIBF_CHANGED,LIB_FLAGS(A1)
	    MOVE.B  RT_VERSION(A0),LIB_VERSION+1(A1) ;Note: Byte to word
	    MOVE.L  RT_IDSTRING(A0),LIB_IDSTRING(A1)

	    MOVE.L  A0,-(SP)
	    MOVE.L  RT_INIT(A0),A0
	    MOVE.L  12(A0),D0           ;get call vector
	    BEQ.S   ir_NoCall
	    EXG.L   A1,D0		;call vector to A1, libbase to D0
	    MOVE.L  D1,A0		;get seglist
	    JSR     (A1)
	;------------------------------------------------------
	; Calling Sequence
	;	library = InitLib (base, SegList),a6
	;	d0		   d0	 a0	  execbase
	;------------------------------------------------------
	    MOVE.L  D0,4(SP)            ;update library base value

ir_NoCall:  MOVE.L  (SP)+,A0            ;A0-ROMTag
	    MOVE.L  (SP),D0
	    BEQ.S   ir_retval		;NULL on stack...
	    MOVE.L  D0,A1		;A1-libbase
	    MOVE.B  RT_TYPE(A0),D0

	    CMP.B   #NT_DEVICE,D0 ;smaller with SUBQ (but hard to maintain)
	    BNE.S   ir_addLib
	    JSRSELF AddDevice
	    BRA.S   ir_retval

ir_addLib:  CMP.B   #NT_LIBRARY,D0
	    BNE.S   ir_addRes
	    JSRSELF AddLibrary
	    BRA.S   ir_retval

ir_addRes:  CMP.B   #NT_RESOURCE,D0
	    BNE.S   ir_retval
	    JSRSELF AddResource

ir_retval:  MOVE.L  (SP)+,D0
	    RTS 	;(exit)


*****o* exec.library/InitStruct ********************************************
*
*   NAME
*	InitStruct - initialize memory from a table
*
*   SYNOPSIS
*	InitStruct(initTable, memory, size);
*		   A1	      A2      D0-0:16
*
*   FUNCTION
*	Clear a memory area except those words whose data and offset
*	values are provided in the initialization table.  This
*	initialization table has byte commands to
*
*	     |a    ||byte|	|given||byte|	      |once	    |
*	load |count||word| into |next ||rptr| offset, |repeatitively|.
*		    |long|
*
*	Not all combinations are supported.  The offset, when
*	specified, is relative to the memory pointer provided
*	(Memory), and is initially zero.  The initialization data
*	(InitTable) contains byte commands whose 8 bits are
*	interpreted as follows:
*
*	ddssnnnn
*	    dd	the destination type (and size):
*		00  next destination, nnnn is count
*		01  next destination, nnnn is repeat
*		10  destination offset is next byte, nnnn is count
*		11  destination offset is next rptr, nnnn is count
*	    ss	the size and location of the source:
*		00  long, from the next two aligned words
*		01  word, from the next aligned word
*		10  byte, from the next byte
*		11  ERROR - will cause an ALERT (see below)
*	  nnnn	the count or repeat:
*	     count  the (number+1) of source items to copy
*	    repeat  the source is copied (number+1) times.
*
*	initTable commands are always read from the next even byte.
*	Given destination offsets are always relative to memory (A2).
*
*	The command 00000000 ends the InitTable stream: use 00010001
*	if you really want to copy one longword.
*
*	24 bit APTR not supported for 68020 compatibitity -- use long.
*
*   INPUTS
*	initTable - the beginning of the commands and data to init
*		Memory with.  Must be on an even boundary unless only
*		byte initialization is done.
*	memory - the beginning of the memory to initialize.  Must be
*		on an even boundary if size is specified.
*	size - the size of memory, which is used to clear it before
*		initializing it via the initTable.  If Size is zero,
*		memory is not cleared before initializing.  Size is
*		rounded down to the nearest even number before use.
*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION
*	D0  clear size, command, count and repeat
*	D1  destination offset, command type
*	A0  current Memory pointer
*	A1  current InitTable pointer
*
*	D0,D1,A0,A1 destroyed
*
**********************************************************************


isrbyte:
	    MOVE.B  (A1)+,D1
isrbytes:
	    MOVE.B  D1,(A0)+
	    DBF     D0,isrbytes
	    BRA.S   isloop

isrlong:
*	    ----------- Round up to a word boundary
	    MOVE.L  A1,D1
	    ADDQ.L  #1,D1
	    AND.B   #$0FE,D1
	    MOVE.L  D1,A1

	    MOVE.L  (A1)+,D1
isrlongr:
	    MOVE.L  D1,(A0)+
	    DBF     D0,isrlongr
	    BRA.S   isloop

isrword:
*	    ----------- Round up to a word boundary
	    MOVE.L  A1,D1
	    ADDQ.L  #1,D1
	    AND.B   #$0FE,D1
	    MOVE.L  D1,A1

	    MOVE.W  (A1)+,D1
isrwordr:
	    MOVE.W  D1,(A0)+
	    DBF     D0,isrwordr
	    BRA.S   isloop

*-------------------------------------------------------------------
InitStruct:
	    MOVE.L  A2,A0
	    LSR     #1,D0
	    BRA.S   iscdbf
isclear:
	    CLR     (A0)+
iscdbf:
	    DBF     D0,isclear

isinit:
	    MOVE.L  A2,A0	*initialize current offset

isloop:
	    CLR     D0		*ensure high byte clear
	    MOVE.B  (A1)+,D0
	    BEQ.S   isterminate *initialization is done
;;;;	    BCLR    #7,D0	*check for a given destination
	    BPL.S   issource	*  nope, use next destination
	    BCLR    #6,D0	*show count, not repeat
	    BEQ.S   isbytedest
	    SUBQ.L  #1,A1
	    MOVE.L  (A1)+,D1    *aptr destination
	    AND.L   #$FFFFFF,D1 *get positive offset
	    BRA.S   isoffdest	*go get offset
isbytedest:
	    CLEAR   D1
	    MOVE.B  (A1)+,D1    *get low byte
isoffdest:
	    MOVE.L  A2,A0	*get base
	    ADD.L   D1,A0	*add offset

issource:
	    MOVE.W  D0,D1	*get command type
	    LSR     #3,D1	* #4
	    AND.W   #$E,D1
	    MOVE.W  dss(PC,D1.W),D1
	    AND.W   #$000F,D0	*ensure just count/repeat
	    JMP     dss(PC,D1.W)

iscbyte:
*****	    ADDQ.L  #1,A1

isbytesc:
	    MOVE.B  (A1)+,(A0)+
	    DBF     D0,isbytesc
	    MOVE.L  A1,D0	*get word aligned next command address
	    ADDQ.L  #1,D0	*
	    BCLR    #0,D0	*
	    MOVE.L  D0,A1	*
	    BRA.S   isloop

isclong:
	    ADD.W   D0,D0	*make long count into word count
	    ADDQ.W  #1,D0	*

iscword:
*	    ----------- Round pointer up to a word boundary
	    MOVE.L  A1,D1
	    ADDQ.L  #1,D1
	    AND.B   #$0FE,D1
	    MOVE.L  D1,A1
iscwordc:
	    MOVE.W  (A1)+,(A0)+
	    DBF     D0,iscwordc
	    BRA.S   isloop

isterminate:
iscaptr:
israptr:	;------ illegal operation (APTR source not allowed)
	    RTS


dss	    DC.W    isclong-dss
	    DC.W    iscword-dss
	    DC.W    iscbyte-dss
	    DC.W    iscaptr-dss
	    DC.W    isrlong-dss
	    DC.W    isrword-dss
	    DC.W    isrbyte-dss
	    DC.W    israptr-dss

    END
