*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		  Memory Allocation		 **
*	   **						 **
*	   ************************************************


**********************************************************************
*								     *
*   Copyright 1984,85,88,89,90,91 Commodore-Amiga Inc.		     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga,Inc.  All rights reserved			     *
*								     *
**********************************************************************


**********************************************************************
*
* $Id: memory.asm,v 39.11 92/07/02 11:26:22 mks Exp $
*
* $Log:	memory.asm,v $
* Revision 39.11  92/07/02  11:26:22  mks
* Fixed autodoc typo...
* 
* Revision 39.10  92/07/02  10:31:27  mks
* Fixed some autodoc issues
*
* Revision 39.9  92/06/10  09:10:45  mks
* Minor autodoc fixups.
*
* Revision 39.8  92/05/28  19:03:24  mks
* Changed NEWLIST a0 to a bsr NewList to save ROM space
*
* Revision 39.7  92/05/21  11:46:51  mks
* Changed some docs on the way memory works.  Moved the autodocs into the
* source (where they should have been to start with)
*
* Revision 39.6  92/03/17  11:14:11  mks
* Fixed memory bug with MEMF_REVERSE
*
* Revision 39.5  92/03/16  17:04:01  mks
* Fixed pool handling of bubbles and non-8-byte allocation returns
* from AllocVec/AllocMem
*
* Revision 39.4  92/03/13  11:00:03  mks
* Moved the pool code into the memory module in prep for making them
* work together...  (Code savings)
*
* Revision 39.3  92/02/19  15:44:17  mks
* Changed ALERT macro to MYALERT
*
* Revision 39.2  92/02/03  17:05:23  mks
* Made the allocation path for correct allocations the fastest path...
*
* Revision 39.1  91/12/19  19:56:58  mks
* Added the routines for the Low Memory Handler
*
* Revision 39.0  91/10/15  08:28:03  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

	NOLIST
	INCLUDE	"assembly.i"
	INCLUDE	"constants.i"
	INCLUDE	"types.i"
	INCLUDE	"calls.i"

	INCLUDE	"nodes.i"
	INCLUDE	"lists.i"
	INCLUDE	"execbase.i"
	INCLUDE	"ables.i"
	INCLUDE	"alerts.i"
	INCLUDE	"memory.i"
	INCLUDE	"interrupts.i"

;* These two are for the ERROR_NO_FREE_STORE and the pr_Result2 symbols
	INCLUDE	"dos/dos.i"
	INCLUDE	"dos/dosextens.i"


;****** Imported Globals *********************************************

	TASK_ABLES

	EXTERN_BASE	MemList


;****** Imported Functions *******************************************

	EXTERN_SYS	Alert
	EXTERN_SYS	AllocMem
	EXTERN_SYS	FreeMem
	EXTERN_SYS	AllocVec
	EXTERN_SYS	FreeVec
	EXTERN_SYS	FreeEntry
	EXTERN_SYS	AllocAbs

	EXTERN_CODE	AddNode
	EXTERN_CODE	printit
	EXTERN_CODE	NewList


;****** Exported Functions *******************************************

	XDEF	Allocate
	XDEF	Deallocate
	XDEF	AllocMem
	XDEF	AllocVec
	XDEF	AllocAbs
	XDEF	AllocAbsInternal
	XDEF	FreeMem
	XDEF	FreeVec
	XDEF	TypeOfMem
	XDEF	AvailMem
	XDEF	AllocEntry
	XDEF	FreeEntry
	XDEF	AddMemList
	XDEF	AddMemListInternal	;exec internal
	XDEF	AddMemHandler
	XDEF	RemMemHandler
	; Pools...
	XDEF	CreatePool
	XDEF	DeletePool
	XDEF	AllocPooled
	XDEF	FreePooled
	LIST


******* exec.library/AddMemHandler ********************************************
*
*   NAME
*	AddMemHandler - Add a low memory handler to exec                 (V39)
*
*   SYNOPSIS
*	AddMemHandler(memHandler)
*	              A1
*
*	VOID AddMemHandler(struct Interrupt *);
*
*   FUNCTION
*	This function adds a low memory handler to the system.  The handler
*	is described in the Interrupt structure.  Due to multitasking
*	issues, the handler must be ready to run the moment this function
*	call is made.  (The handler may be called before the call returns)
*
*   NOTE
*	Adding a handler from within a handler will cause undefined
*	actions.  It is safe to add a handler to the list while within
*	a handler but the newly added handler may or may not be called
*	for the specific failure currently running.
*
*   EXAMPLE
*	struct Interrupt *myInt;  \* Assume it is allocated *\
*
*	myInt->is_Node.ln_Pri=50;  \* Relatively early; before RAMLIB *\
*
*	\* Please fill in the name field! *\
*	myInt->is_Node.ln_Name="Example Handler";
*
*	myInt->is_Data=(APTR)mydata_pointer;
*	myInt->is_Code=myhandler_code;
*
*	AddMemHandler(myInt);
*	... \* and so on *\
*
*	_myhandler_code:
*			; This is the handler code
*			; We are passed a pointer to struct MemHandlerData
*			; in a0, the value of is_Data in a1 and
*			; ExecBase in a6.
*			; We must not break forbid!!!
*	;
*	; Start off assuming we did nothing
*	;
*		moveq.l	#MEM_DID_NOTHING,d0
*		move.l	memh_RequestFlags(a0),d1
*		btst.l	#MEMB_CHIP,d1	; Did the failure happen in CHIP
*		beq.s	handler_nop	; If not, we have nothing to do
*		bsr	DoMyMagic	; Do the magic...
*		; DoMyMagic frees whatever we can and returns d0 set...
*	handler_nop:
*		rts			; Return with d0 set...
*
*   INPUTS
*	memHandler - A pointer to a completely filled in Interrupt structure
*	             The priority field determine the position of the handler
*	             with respect to other handlers in the system.  The higher
*	             the priority, the earlier the handler is called.
*	             Positive priorities will have the handler called before
*	             any of the library expunge vectors are called.  Negative
*	             priority handlers will be called after the library
*	             expunge routines are called.
*	             (Note:  RAMLIB is a handler at priority 0)
*
*   SEE ALSO
*	RemMemHandler, exec/interrupts.i
*
*******************************************************************************
*
* Do a protected enqueue into the memory handler list...
*
AddMemHandler:
	LEA	ex_MemHandlers(A6),A0	; Get list header
	BRA	AddNode			; (Protected Enqueue onto list)

******* exec.library/RemMemHandler ********************************************
*
*   NAME
*	RemMemHandler - Remove low memory handler from exec              (V39)
*
*   SYNOPSIS
*	RemMemHandler(memHandler)
*	              A1
*
*	VOID RemMemHandler(struct Interrupt *);
*
*   FUNCTION
*	This function removes the low memory handler from the system.
*	This function can be called from within a handler.  If removing
*	oneself, it is important that the handler returns MEM_ALL_DONE.
*
*   NOTE
*	When removing a handler, the handler may be called until this
*	function returns.  Thus, the handler must still be valid until
*	then.
*
*   INPUTS
*	memHandler - Pointer to a handler added with AddMemHandler()
*
*   SEE ALSO
*	AddMemHandler, exec/interrupts.i
*
*******************************************************************************
*
* We must do the "right thing" such that removing nodes while in the handler
* is safe and "works"
*
RemMemHandler:
	FORBID				; First FORBID (protection)
	cmp.l	ex_MemHandler(a6),a1	; Check if we are the handler
	bne.s	rmh_NotMe		; If not, we don't change it...
	move.l	LN_SUCC(a1),ex_MemHandler(a6)	; Set up new holding value
rmh_NotMe:
	REMOVE				; Remove the node...
	JMP_PERMIT			; and permit/exit


******* exec.library/Deallocate ***********************************************
*
*   NAME
*	Deallocate -- deallocate a block of memory
*
*   SYNOPSIS
*	Deallocate(memHeader, memoryBlock, byteSize)
*		   A0	      A1	   D0
*
*	void Deallocate(struct MemHeader *,APTR,ULONG);
*
*   FUNCTION
*	This function deallocates memory by returning it to the appropriate
*	private free memory pool.  This function can be used to free an
*	entire block allocated with the above function, or it can be used
*	to free a sub-block of a previously allocated block.  Sub-blocks
*	must be an even multiple of the memory chunk size (currently 8
*	bytes).
*
*	This function can even be used to add a new free region to an
*	existing MemHeader, however the extent pointers in the MemHeader
*	will no longer be valid.
*
*	If memoryBlock is not on a block boundary (MEM_BLOCKSIZE) then it
*	will be rounded down in a manner compatible with Allocate().  Note
*	that this will work correctly with all the memory allocation
*	functions, but may cause surprises if one is freeing only part of a
*	region.  The size of the block will be rounded up, so the freed
*	block will fill to an even memory block boundary.
*
*   INPUTS
*	memHeader - points to the memory header this block is part of.
*	memoryBlock - address of memory block to free.
*	byteSize - the size of the block in bytes. If NULL, nothing
*		   happens.
*
*   SEE ALSO
*	Allocate, exec/memory.h
*
*******************************************************************************
*
*    REGISTER USAGE
*	A0 -- mem header
*	A1 -- mem being freed
*	A2 -- ptr to current mem chunk being looked at (one after A1 eventually)
*

Deallocate:
	TST.L   D0
	BEQ.S   de_rts

	MOVEM.L D3/A2,-(SP)


	IFEQ	TORTURE_TEST_1
	;------ round the base of the block down.
	MOVE.L  A1,D1			* check validity of the
	MOVEQ   #~MEM_BLOCKMASK,D3
	AND.L   D3,D1			*   freed region address
	EXG     D1,A1			* and size up
	SUB.L   A1,D1
	ADD.L   D1,D0
	ENDC
	IFNE	TORTURE_TEST_1
	MOVEQ   #MEM_BLOCKMASK,D3
	MOVE.L	A1,D1
	AND.L	D3,D1
	BEQ.S	de_ok
	DEADALERT $DEAD0061
de_ok:
	ENDC


	;------ round size up to even block address:
	ADDQ.L  #MEM_BLOCKMASK,D0	* bias
	AND.L   D3,D0			* round
	BEQ.S   de_exit

	LEA	MH_FIRST(A0),A2		* pred of first node (head)
	MOVE.L  (A2),D3			* test for null list
	BEQ.S   at_head



	;------ scan for position in free list:
de_chunkloop:
	CMP.L   D3,A1			* went past returned region?
	BLS.S   de_inlist		* add region to free list

	;------ advance to the next node
	MOVE.L  D3,A2
	MOVE.L  (A2),D3			* get next node address
	BNE.S   de_chunkloop		* not end of list
	BRA.S	de_head_check



de_inlist:
	BEQ.S	de_freetwice		* already in the list

de_head_check:
	;------ check for first node on the list
	MOVEQ  #MH_FIRST,D1
	ADD.L  A0,D1
	CMP.L  A2,D1
	BEQ.S  at_head

	;------ join pred?
	MOVE.L  MC_BYTES(A2),D3
	ADD.L   A2,D3
	CMP.L   A1,D3			* curr = pred.size + pred ?
	BEQ.S   pred_join
	BHI.S   de_badfree		* freelist extends into new free mem

	;------ don't join with pred:
at_head:
	MOVE.L  (A2),(A1)		* pred.succ -> curr.succ
	MOVE.L  A1,(A2)			* curr -> pred.succ
	MOVE.L  D0,MC_BYTES(A1)		* size -> curr.size
	BRA.S   tail_check

	;------ join with pred:
pred_join:
	ADD.L   D0,MC_BYTES(A2)		* size + pred.size -> pred.size
	MOVE.L  A2,A1			* pred -> curr

tail_check:
	;------ at tail?
	TST.L   (A1)
	BEQ.S   de_done

	;------ join succ?
	MOVE.L  MC_BYTES(A1),D3
	ADD.L   A1,D3
	CMP.L   (A1),D3			* pred = curr.size + curr ?
	BHI.S   de_badfree		* interference
	BNE.S   de_done

	;------ join with succ:
	MOVE.L  (A1),A2
	MOVE.L  (A2),(A1)		* curr.succ.succ -> curr.succ
	MOVE.L  MC_BYTES(A2),D3
	ADD.L   D3,MC_BYTES(A1)		* curr.size+succ.size->curr.size

de_done:
	ADD.L   D0,MH_FREE(A0)
de_exit:
	MOVEM.L (SP)+,D3/A2
de_rts:	RTS

de_freetwice:
	;------ we tried to free a piece of memory that was already
	;------ in the free list.
	MOVEM.L	(SP)+,D3/A2
	MYALERT	AN_FreeTwice
	RTS

de_badfree:
	DEADALERT AN_MemCorrupt



******* exec.library/FreeMem **************************************************
*
*   NAME
*	FreeMem -- deallocate with knowledge
*
*   SYNOPSIS
*	FreeMem(memoryBlock, byteSize)
*		A1	     D0
*
*	void FreeMem(void *,ULONG);
*
*   FUNCTION
*	Free a region of memory, returning it to the system pool from which
*	it came.  Freeing partial blocks back into the system pool is
*	unwise.
*
*   NOTE
*	If a block of memory is freed twice, the system will Guru. The
*	Alert is AN_FreeTwice ($01000009).   If you pass the wrong pointer,
*	you will probably see AN_MemCorrupt $01000005.  Future versions may
*	add more sanity checks to the memory lists.
*
*   INPUTS
*	memoryBlock - pointer to the memory block to free
*	byteSize - the size of the desired block in bytes.  (The operating
*		system will automatically round this number to a multiple of
*		the system memory chunk size)
*
*   SEE ALSO
*	AllocMem
*
*******************************************************************************

******* exec.library/FreeVec **************************************************
*
*   NAME
*	FreeVec -- return AllocVec() memory to the system  (V36)
*
*   SYNOPSIS
*	FreeVec(memoryBlock)
*		A1
*
*	void FreeVec(void *);
*
*   FUNCTION
*	Free an allocation made by the AllocVec() call.  The memory will
*	be returned to the system pool from which it came.
*
*   NOTE
*	If a block of memory is freed twice, the system will Guru. The
*	Alert is AN_FreeTwice ($01000009).   If you pass the wrong pointer,
*	you will probably see AN_MemCorrupt $01000005.  Future versions may
*	add more sanity checks to the memory lists.
*
*   INPUTS
*	memoryBlock - pointer to the memory block to free, or NULL.
*
*   SEE ALSO
*	AllocVec
*
*******************************************************************************
*
*   IMPLEMENTATION NOTES
*
*	FreeMem will panic if the freed memory has no corresponding
*	free list
*

;FreeVec assumes the block size is at -4 from the passed address
;Note strange register to match FreeMem()
FreeVec:	MOVE.L	A1,D1		;Test for NULL
		BEQ.S	FreeMem_Rts
		MOVE.L	-(A1),D0
		JMPSELF	FreeMem



FreeMem:

	IFNE	TORTURE_TEST_1
		tst.b TDNestCnt(a6)	;free memory while Forbidden()?
		bge.s oldway

		;----Zap block before freeing----
noprob:		movem.l d0/a1,-(a7)
		 move.l  #MEMMUNG_COOKIE,d1  ;Wipe memory to this value
		 addq.l  #7,d0	;Convert length to
		 lsr.l   #3,d0	; nearest double-longword
		 bra.s   inlp
loop		 move.l  d1,(a1)+
		 move.l  d1,(a1)+
inlp		 dbra    d0,loop
		 sub.l   #$10000,d0
		 bpl.s   loop
		movem.l (a7)+,a1/d0
oldway:
	ENDC


		MOVE.L	A1,D1		;special kludge for people who free
		BEQ.S   FreeMem_Rts	;zero bytes @ addr 0

		;------ run through the lists looking for a match
		LEA	MemList(A6),A0     ; beginning of free list array

		;--- !! protect whole func for now (err... for-ever)
		FORBID

FreeMem_Next:	TSTNODE A0,A0
		;------ check for end of chain (should "never" happen)
		BEQ.S	badFreeM		; corrupt free list or bad args

		CMP.L   MH_LOWER(A0),A1
		BCS.S   FreeMem_Next		;BLO.S
		CMP.L   MH_UPPER(A0),A1
		BCC.S   FreeMem_Next		;BHI.S


		;------ we have found the correct free list
		BSR	Deallocate	; try to deallocate it

FreeMem_End:	JMP_PERMIT		;exit


badFreeM:	BSR_PERMIT
		MYALERT	AN_BadFreeAddr
FreeMem_Rts:	RTS			;exit





******* exec.library/Allocate *************************************************
*
*   NAME
*	Allocate - allocate a block of memory
*
*   SYNOPSIS
*	memoryBlock=Allocate(memHeader, byteSize)
*	D0		     A0 	D0
*
*	void *Allocate(struct MemHeader *, ULONG);
*
*   FUNCTION
*	This function is used to allocate blocks of memory from a given
*	private free memory pool (as specified by a MemHeader and its
*	memory chunk list).  Allocate will return the first free block that
*	is greater than or equal to the requested size.
*
*	All blocks, whether free or allocated, will be block aligned;
*	hence, all allocation sizes are rounded up to the next block even
*	value (e.g. the minimum allocation resolution is currently 8
*	bytes.  A request for 8 bytes will use up exactly 8 bytes.  A
*	request for 7 bytes will also use up exactly 8 bytes.).
*
*	This function can be used to manage an application's internal data
*	memory.  Note that no arbitration of the MemHeader and associated
*	free chunk list is done.  You must be the owner before calling
*	Allocate.
*
*   INPUTS
*	memHeader - points to the local memory list header.
*	byteSize - the size of the desired block in bytes.
*
*   RESULT
*	memoryBlock - a pointer to the just allocated free block.
*	       If there are no free regions large enough to satisfy the
*	       request, return zero.
*
*   EXAMPLE
*	#include <exec/types.h>
*	#include <exec/memory.h>
*	void *AllocMem();
*	#define BLOCKSIZE 4096L /* Or whatever you want */
*
*	void main()
*	{
*	struct MemHeader *mh;
*	struct MemChunk  *mc;
*	APTR   block1;
*	APTR   block2;
*
*	    /* Get the MemHeader needed to keep track of our new block */
*	    mh = (struct MemHeader *)
*		 AllocMem((long)sizeof(struct MemHeader), MEMF_CLEAR );
*	    if( !mh )
*		exit(10);
*
*	    /* Get the actual block the above MemHeader will manage */
*	    mc = (struct MemChunk *)AllocMem( BLOCKSIZE, 0L );
*	    if( !mc )
*		{
*		FreeMem( mh, (long)sizeof(struct MemHeader) ); exit(10);
*		}
*
*	    mh->mh_Node.ln_Type = NT_MEMORY;
*	    mh->mh_Node.ln_Name = "myname";
*	    mh->mh_First = mc;
*	    mh->mh_Lower = (APTR) mc;
*	    mh->mh_Upper = (APTR) ( BLOCKSIZE + (ULONG) mc );
*	    mh->mh_Free  = BLOCKSIZE;
*
*	    /* Set up first chunk in the freelist */
*	    mc->mc_Next  = NULL;
*	    mc->mc_Bytes = BLOCKSIZE;
*
*	    block1 = (APTR) Allocate( mh, 20L );
*	    block2 = (APTR) Allocate( mh, 314L );
*	    printf("mh=$%lx mc=$%lx\n",mh,mc);
*	    printf("Block1=$%lx, Block2=$%lx\n",block1,block2);
*
*	    FreeMem( mh, (long)sizeof(struct MemHeader) );
*	    FreeMem( mc, BLOCKSIZE );
*	}
*
*   NOTE
*	If the free list is corrupt, the system will panic with alert
*	AN_MemCorrupt, $01000005.
*
*   SEE ALSO
*	Deallocate, exec/memory.h
*
*******************************************************************************
*
*	add more validity checks?
*
*	Note:	must preserve A0 for AllocMem
*		must set cc's for AllocMem
*


	    CNOP    2,4		* Two past longword for best loop alignment

Allocate:
*	    ------- quick check for space:
	    CMP.L   MH_FREE(A0),D0
	    BHI.S   am_quickfail	* exit if not enough total space...
	    TST.L   D0
	    BEQ.S   am_rts		* idiot case; asking for zero bytes...


InternalAllocate:
	    MOVE.L  A2,-(SP)
*	    ------- round size up to even block address:
	    ADDQ.L  #MEM_BLOCKMASK,D0	* bias
	    AND.W   #~MEM_BLOCKMASK,D0	* and round

	    LEA	    MH_FIRST(A0),A2	* pred of first node (same as head)



*	    -------
*	    ------- Chunk loop is in two halves, alternating registers
*	    ------- (we must always have pointers to the current and
*	    -------- previous chunks.  This double-loop saves bookkeeping)
*	    -------
am_chunkloop:
	    MOVE.L  (A2),A1
	    MOVE.L  A1,D1		* test for zero
	    BEQ.S   am_none		* end of list
	    CMP.L   MC_BYTES(A1),D0
	    BLS.S   am_found		* A2=prev A1=current

	    MOVE.L  (A1),A2
	    MOVE.L  A2,D1		* test for zero
	    BEQ.S   am_none		* end of list
	    CMP.L   MC_BYTES(A2),D0
	    BHI.S   am_chunkloop	* A1=prev A2=current

*	    ------- exchange so both paths look the same
	    EXG.L   A1,A2		* cc's unaffected



*	    ------- A0=memheader A1=current chunk A2=previous chunk A3-split
am_found:   BEQ.S   no_split

*	    ------- split the region:
	    MOVE.L  A3,-(SP)
	     LEA.L  0(A1,D0.L),A3	* newly created chunk's address
	     MOVE.L A3,(A2)		* make previous chunk point to new
	     MOVE.L (A1),(A3)+		* new MC_NEXT  = current MC_NEXT
	     MOVE.L MC_BYTES(A1),D1
	     SUB.L  D0,D1		* remaining size
	     MOVE.L D1,(A3)		* new MC_BYTES = size of new chunk
	     SUB.L  D0,MH_FREE(A0)	* adjust memory header
	    MOVE.L  (SP)+,A3
	    MOVE.L  A1,D0		* set result
	    MOVE.L  (SP)+,A2
	    RTS				* exit.  cc's


*	    ------- exact fit.  just cut out this area.
no_split:   MOVE.L  (A1),(A2)		* make previous chunk point to next
	    SUB.L   D0,MH_FREE(A0)	* adjust memory header
	    MOVE.L  (SP)+,A2
	    MOVE.L  A1,D0		* set result
	    RTS				* exit.  cc's


am_none:    MOVE.L  (SP)+,A2
am_quickfail:
	    MOVEQ   #0,D0
am_rts:	    RTS				* exit.  cc's


******* exec.library/AllocMem *************************************************
*
*   NAME
*	AllocMem -- allocate memory given certain requirements
*
*   SYNOPSIS
*	memoryBlock = AllocMem(byteSize, attributes)
*	D0		       D0	 D1
*
*	void *AllocMem(ULONG, ULONG);
*
*   FUNCTION
*	This is the memory allocator to be used by system code and
*	applications.  It provides a means of specifying that the allocation
*	should be made in a memory area accessible to the chips, or
*	accessible to shared system code.
*
*	Memory is allocated based on requirements and options.	Any
*	"requirement" must be met by a memory allocation, any "option" will
*	be applied to the block regardless.  AllocMem will try all memory
*	spaces until one is found with the proper requirements and room for
*	the memory request.
*
*   INPUTS
*	byteSize - the size of the desired block in bytes.  (The operating
*		system will automatically round this number to a multiple of
*		the system memory chunk size)
*
*	attributes -
*	    requirements
*
*		If no flags are set, the system will return the best
*		available memory block.  For expanded systems, the fast
*		memory pool is searched first.
*
*		MEMF_CHIP:	If the requested memory will be used by
*				the Amiga custom chips, this flag *must*
*				be set.
*
*				Only certain parts of memory are reachable
*				by the special chip sets' DMA circuitry.
*				Chip DMA includes screen memory, images that
*				are blitted, audio data, copper lists, sprites
*				and Pre-V36 trackdisk.device buffers.
*
*
*		MEMF_FAST:	This is non-chip memory.  If no flag is set
*				MEMF_FAST is taken as the default.
*
*				DO NOT SPECIFY MEMF_FAST unless you know
*				exactly what you are doing!  If MEMF_FAST is
*				set, AllocMem() will fail on machines that
*				only have chip memory!  This flag may not
*				be set when MEMF_CHIP is set.
*
*
*		MEMF_PUBLIC:	Memory that must not be mapped, swapped,
*				or otherwise made non-addressable. ALL
*				MEMORY THAT IS REFERENCED VIA INTERRUPTS
*				AND/OR BY OTHER TASKS MUST BE EITHER PUBLIC
*				OR LOCKED INTO MEMORY! This includes both
*				code and data.
*
*
*		MEMF_LOCAL:	This is memory that will not go away
*				after the CPU RESET instruction.  Normally,
*				autoconfig memory boards become unavailable
*				after RESET while motherboard memory
*				may still be available.  This memory type
*				is now automatically set in V36.  Pre-V36
*				systems may not have this memory type
*				and AllocMem() will then fail.
*
*
*		MEMF_24BITDMA:	This is memory that is within the address
*				range of 24-bit DMA devices.  (Zorro-II)
*				This is required if you run a Zorro-II
*				DMA device on a machine that has memory
*				beyond the 24-bit addressing limit of
*				Zorro-II.  This memory type
*				is now automatically set in V36.  Pre-V36
*				systems may not have this memory type
*				and AllocMem() will then fail.
*
*
*		MEMF_KICK:	This memory is memory that EXEC was able
*				to access during/before the KickMem and
*				KickTags are processed.  This means that
*				if you wish to use these, you should allocate
*				memory with this flag.  This flag is
*				automaticly set by EXEC in V39.  Pre-V39
*				systems may not have this memory type and
*				AllocMem() will then fail.  Also, *DO NOT*
*				ever add memory the system with this flag
*				set.  EXEC will set the flag as needed
*				if the memory matches the needs of EXEC.
*
*
*	    options
*
*		MEMF_CLEAR:	The memory will be initialized to all
*				zeros.
*
*
*		MEMF_REVERSE:	This allocates memory from the top of
*				the memory pool.  It searches the pools
*				in the same order, such that FAST memory
*				will be found first.  However, the
*				memory will be allocated from the highest
*				address available in the pool.  This
*				option is new as of V36.  Note that this
*				option has a bug in pre-V39 systems.
*
*
*		MEMF_NO_EXPUNGE	This will prevent an expunge to happen on
*				a failed memory allocation.  This option is
*				new to V39 and will be ignored in V37.
*				If a memory allocation with this flag
*				set fails, the allocator will not cause
*				any expunge operations.  (See AddMemHandler())
*
*
*   RESULT
*	memoryBlock - a pointer to the newly allocated memory block.
*		If there are no free memory regions large enough to satisfy
*		the request, zero will be returned.  The pointer must be
*		checked for zero before the memory block may be used!
*		The memory block returned is long word aligned.
*
*   WARNING
*	The result of any memory allocation MUST be checked, and a viable
*	error handling path taken.  ANY allocation may fail if memory has
*	been filled.
*
*   EXAMPLES
*	AllocMem(64,0L)		- Allocate the best available memory
*	AllocMem(25,MEMF_CLEAR) - Allocate the best available memory, and
*				  clear it before returning.
*	AllocMem(128,MEMF_CHIP) - Allocate chip memory
*	AllocMem(128,MEMF_CHIP|MEMF_CLEAR) - Allocate cleared chip memory
*	AllocMem(821,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR) - Allocate cleared,
*		public, chip memory.
*
*   NOTE
*	If the free list is corrupt, the system will panic with alert
*	AN_MemCorrupt, $01000005.
*
*	This function may not be called from interrupts.
*
*	A DOS process will have its pr_Result2 field set to
*	ERROR_NO_FREE_STORE if the memory allocation fails.
*
*   SEE ALSO
*	FreeMem
*
*******************************************************************************

******* exec.library/AllocVec *************************************************
*
*   NAME
*	AllocVec -- allocate memory and keep track of the size  (V36)
*
*   SYNOPSIS
*	memoryBlock = AllocVec(byteSize, attributes)
*	D0		       D0	 D1
*
*	void *AllocVec(ULONG, ULONG);
*
*   FUNCTION
*	This function works identically to AllocMem(), but tracks the size
*	of the allocation.
*
*	See the AllocMem() documentation for details.
*
*   WARNING
*	The result of any memory allocation MUST be checked, and a viable
*	error handling path taken.  ANY allocation may fail if memory has
*	been filled.
*
*   SEE ALSO
*	FreeVec, AllocMem
*
*******************************************************************************
*
*   REGISTER USAGE
*	A6 - library pointer (SysLib)
*	A2 - Current memory block pointer
*	D2 - requirements
*	D3 - byte size requested
*
*
*   IMPLEMENTATION NOTES
*
*	!!! The whole memory management scheme is chock full
*	of race conditions.  It cannot be referenced from interrupt
*	time.
*
*	**** NOTE:  Compatibility
*	RAMLIB used to setfunction AllocMem.  With the new handlers
*	this will no longer be the case.  However, with this came
*	the need to make sure that the Z flag is set and that if
*	the calling task is a process the pr_Result2 field is set
*	to ERROR_NO_FREE_STORE on failure.
*

;
; AllocMem, but leave size at -4 from the returned address
;
AllocVec:	tst.l	d0
		beq.s	ac_exit	;Bozo case.  Carolyn made me do it.
		addq.l	#4,d0
		move.l	d0,-(sp)
		JSRSELF	AllocMem
		tst.l	d0
		beq.s	ac_fail
		move.l	d0,a0
		move.l	(sp),(a0)+
		move.l	a0,d0
ac_fail:	addq.l	#4,sp
ac_exit:	rts


**********
* New!  This is needed for the low memory handler
**********
Alloc_Failed:
*
* Ok, so we are in a possible expunge situation...
* We need to go into FORBID state now!
*
	movem.l	a2/a3/a5,-(sp)	; Save for later use...
	lea	3*4(sp),a3	; Get pointer to MemHandlerData...
*
* Now, we *MUST* be in FORBID for the rest of this
*
	FORBID
	;*
	;* This following is to go into no-expunge if we are in expunge already
	;* Could be removed later...
	;* (If not buggy software, it should always be 0, but some software
	;* is buggy...)
	tst.l	ex_MemHandler(a6)
	bne.s	AllocMemExit
**********
* Ok, so we are a failed allocation that can do expunge...
**********
	lea	ex_MemHandlers(a6),a0		; Get memory handlers
	move.l	MLH_HEAD(a0),ex_MemHandler(a6)	; Get memory handler ready
**********
* Do the next handler
**********
Next_Handler:
	move.l	ex_MemHandler(a6),a2	; Get structure...
	move.l	LN_SUCC(a2),d0		; Get next handler...
	move.l	d0,ex_MemHandler(a6)	; ...and store it...
	beq.s	AllocMemExit		; If NULL, end of handler list
	clr.l	memh_Flags(a3)		; Clear the flags...
**********
* Here we call the handler...
**********
Do_Handler:
	move.l	a3,a0			; Get pointer to MemHandlerData
	movem.l	IS_DATA(a2),a1/a5	; Get data & code field...
	jsr	(a5)			; Call the routine...
*
* Ok, so we now called the routine.  Lets check the results
* and do as needed...
*
	move.l	d0,a5		; Use a5 for temp storage...
	tst.l	d0		; Check it...
	beq.s	Next_Handler	; The handler returned MEM_DID_NOTHING
*
* Now, try the allocation again
*
	movem.l	(a3),d0/d1	; Get requirements
	bsr.s	OldAllocMem	; Do the basic AllocMem (sets flags)
	bne.s	AllocMemDone	; If it worked, we are done...
*
* Now, check to see if we need to do the same handler again or not...
*
	move.l	a5,d0		; Get return result again...
	bmi.s	Next_Handler	; If it returned MEM_ALL_DONE, we do next
*
* Set the non-first time flag
*
	moveq.l	#MEMHF_RECYCLE,d0	; Get flag
	move.l	d0,memh_Flags(a3)	; Set flag
	bra.s	Do_Handler
*
* Exit from routine
*
AllocMemDone:
	clr.l	ex_MemHandler(a6)	; Make sure it is cleared...
AllocMemExit:
	BSR_PERMIT		; Let the OS continue...
	movem.l	(sp)+,a2/a3/a5	; Restore a2/a3/a5 now...
*
* Once here, check if there was a allocation and set bits as needed
*
AllocMemCheck:
	tst.l	d0			; Check if we worked...
	bne.s	AllocMemOK		; If we got some memory, we continue
	move.l	ThisTask(a6),a0		; Get the current task
	cmp.b	#NT_PROCESS,LN_TYPE(a0)	; Are we a process?
	bne.s	AllocMemOK		; If not, we don't set IoErr()
	moveq.l	#ERROR_NO_FREE_STORE,d1	; Get Error code
	move.l	d1,pr_Result2(a0)	; Set IoErr()...
	bra.s	AllocMemOK		; Do cleanup & return...

*********
*
* We did not get an allocation, check what we should do...
*
Alloc_Empty:
*
* Check if we are in SingleTask and if so, skip this...
*
	tst.l	ThisTask(a6)	; Check ThisTask
	beq.s	AllocMemOK	; If no task, we are not expunge ready
*
* Ok, so the allocation did not work, check if we care
*
	IFNE	MEMB_NO_EXPUNGE-31
	fail
	ENDC
*
	tst.l	4(sp)		; Check for MEMB_NO_EXPUNGE
	bmi.s	AllocMemCheck	; (It is bit 31)
	bra.s	Alloc_Failed	; Otherwise do expunge...

***
*** NOTE:  This code is designed to be fastest path when the allocation
*** works.  When an allocation fails, sooo much CPU will be used that it
*** does not matter much if a few extra cycles are used.  Since most
*** allocations work, it is best to make those the fastest.
***
AllocMem:
**********
* New!  This is needed for the low memory handler
**********
*
* Ok, so now try the AllocMem.   We want the basic AllocMem to be fast...
* We also need to have some stuff saved, so save it first.
*
	subq.l	#4,sp		; Space for the flags field
	movem.l	d0/d1,-(sp)	; Save the requirements...
	bsr.s	OldAllocMem	; Call the allocmem routine...
	beq.s	Alloc_Empty	; Do failure code if not OK (flags already set)
*
AllocMemOK:
	addq.l	#8,sp		; Release the allocation requirements
	addq.l	#4,sp		; Release the flags field
****
**** WARNING:  People depend on the Z flag being set!
****
	tst.l	d0		; Set the flags before we return...
	rts			; Return with result...
*
**********************************************
* Original allocmem routine
*
OldAllocMem:
	MOVEM.L D2/D3,-(SP)
	MOVE.L  D0,D3		;D3=Raw number of bytes to allocate
	BEQ.S	AllocMem_End	;Bozo case...
	MOVE.L  D1,D2		;D2=Flags

	;------ find a free list that matches the requirements
	LEA	MemList(A6),A0

	;---!!! protect whole func for now
	FORBID

AllocMem_NextRegion:
	MOVE.L	(A0),A0
	TST.L	(A0)
	BEQ.S   AllocMem_NoMore

	;------ check for exact match on attributes (low word of paramter)
	MOVE.W  MH_ATTRIBUTES(A0),D0
	AND.W   D2,D0
	CMP.W   D2,D0
	BNE.S   AllocMem_NextRegion

		;------ we have satisfied the requirements
		CMP.L	MH_FREE(A0),D3
		BHI.S	AllocMem_NextRegion	;not enough space left...
		BTST	#MEMB_REVERSE,D2
		BNE.S	AllocMem_FromEnd
		MOVE.L  D3,D0
		BSR	InternalAllocate	;(A0,D0); A0 saved.  cc's set

	BEQ.S   AllocMem_NextRegion


AllocMem_GotRev:
	;------ we have our memory block.  turn everyone else back on
	BSR_PERMIT

	;------ see if we need to clear memory
	IFEQ	TORTURE_TEST_1
		BTST	#MEMB_CLEAR,D2
		BEQ.S	AllocMem_End
		CLEAR	D1
	ENDC
	IFNE	TORTURE_TEST_1
		CLEAR	D1
		BTST	#MEMB_CLEAR,D2
		BNE.S	AllocMem_Clear
		MOVE.L	#MEMMUNG_CRACKER,D1
AllocMem_Clear:
	ENDC

	;------ round off
	MOVE.L  D0,A0
	ADDQ.L  #7,D3		; bias
	LSR.L   #3,D3		; and round

	;------ split D3 into high and low words
	MOVE.W	D3,D2		; low order word in D2
	SWAP	D3		; high order word in D3
	BRA.S   AllocMem_dbraclear

AllocMem_topclear:
	MOVE.L  D1,(A0)+
	MOVE.L  D1,(A0)+
AllocMem_dbraclear:
	DBF	D2,AllocMem_topclear
	DBF	D3,AllocMem_topclear

AllocMem_End:
	MOVEM.L (SP)+,D2/D3
	NPRINTF	999,<'AllocMemed at $%lx'>,D0
	tst.l	d0		; Set flags so we don't have to...
	RTS

AllocMem_NoMore:
	;------ no more regions
	BSR_PERMIT
	CLEAR   D0
	BRA.S   AllocMem_End

***************
	;------ Alloc from end.  Must preserve A0
AllocMem_FromEnd:
	moveq	#0,d1			;Clear target
	move.l	MH_FIRST(a0),d0
	beq.s	AllocMem_NextRegion
AllocMem_NextChunk:
	move.l	d0,a1
	cmp.l	MC_BYTES(a1),d3
	bhi.s	AllocMem_TooSmall
	move.l	a1,d1			;Possible target
AllocMem_TooSmall:
	move.l	(a1),d0			;MC_NEXT
	bne.s	AllocMem_NextChunk
	tst.l	d1			;Check target
	beq.s	AllocMem_NextRegion	; If no target, get out...

	move.l	d1,a1			;Target to address register
	move.l	MC_BYTES(a1),d0		;Target size to data register
	sub.l	d3,d0			;avail-asking=remaining
	and.w   #~MEM_BLOCKMASK,d0	;Round down to chunk size
	add.l	d0,a1			;End-jusify allocation
	move.l	d3,d0
	JSRSELF	AllocAbs
	bra.s	AllocMem_GotRev		;Got the memory

******* exec.library/TypeOfMem ************************************************
*
*   NAME
*	TypeOfMem -- determine attributes of a given memory address
*
*   SYNOPSIS
*	attributes = TypeOfMem(address)
*	D0		       A1
*
*	ULONG TypeOfMem(void *);
*
*   FUNCTION
*	Given a RAM memory address, search the system memory lists and
*	return its memory attributes.  The memory attributes are similar to
*	those specified when the memory was first allocated: (eg. MEMF_CHIP
*	and MEMF_FAST).
*
*	This function is usually used to determine if a particular block of
*	memory is within CHIP space.
*
*	If the address is not in known-space, a zero will be returned.
*	(Anything that is not RAM, like the ROM or expansion area, will
*	return zero.  Also the first few bytes of a memory area are used up
*	by the MemHeader.)
*
*   INPUT
*	address - a memory address
*
*   RESULT
*	attributes - a long word of memory attribute flags.
*	If the address is not in known RAM, zero is returned.
*
*   SEE ALSO
*	AllocMem()
*
*******************************************************************************
TypeOfMem:
	FORBID
	LEA	MemList(A6),A0     * beginning of free list array
	CLEAR   D0

	;------ run through the lists looking for a match
1$:	TSTNODE A0,A0
	BEQ.S   2$			* end of list
	CMP.L   MH_LOWER(A0),A1
	BCS.S   1$
	CMP.L   MH_UPPER(A0),A1
	BCC.S   1$

	MOVE.W  MH_ATTRIBUTES(A0),D0

2$:
	JMP_PERMIT


******* exec.library/AllocAbs *************************************************
*
*   NAME
*	AllocAbs -- allocate at a given location
*
*   SYNOPSIS
*	memoryBlock = AllocAbs(byteSize, location)
*	D0		       D0	 A1
*
*	void *AllocAbs(ULONG, APTR);
*
*   FUNCTION
*	This function attempts to allocate memory at a given absolute
*	memory location.  Often this is used by boot-surviving entities
*	such as recoverable ram-disks.	If the memory is already being
*	used, or if there is not enough memory to satisfy the request,
*	AllocAbs will return NULL.
*
*	This block may not be exactly the same as the requested block
*	because of rounding, but if the return value is non-zero, the block
*	is guaranteed to contain the requested range.
*
*   INPUTS
*	byteSize - the size of the desired block in bytes
*		   This number is rounded up to the next larger
*		   block size for the actual allocation.
*	location - the address where the memory MUST be.
*
*
*   RESULT
*	memoryBlock - a pointer to the newly allocated memory block, or
*		      NULL if failed.
*
*   NOTE
*	If the free list is corrupt, the system will panic with alert
*	AN_MemCorrupt, $01000005.
*
*	The 8 bytes past the end of an AllocAbs will be changed by Exec
*	relinking the next block of memory.  Generally you can't trust
*	the first 8 bytes of anything you AllocAbs.
*
*   SEE ALSO
*	AllocMem, FreeMem
*
*******************************************************************************
*
*   ignore the Chipmem and system bits of the req mask?
*	D0-bytesize
*	A1-location
*
*	Note:  The first 8 bytes of the block are probably trashed.

AllocAbs:
	;------ protect whole func for now
	FORBID

	;------ round address to block boundary add delta to size
	MOVE.L  A1,D1
	AND.L   #MEM_BLOCKMASK,D1
	SUB.L   D1,A1
	ADD.L   D1,D0	;inc byteSize

	;------ round size up to next block
	ADDQ.L  #MEM_BLOCKMASK,D0
	AND.W   #~MEM_BLOCKMASK,D0

	;------ find the correct free list
	LEA.L	MemList(A6),A0	;beginning of free list array
	MOVE.L  D0,D1
	;[D1=byteSize]
	MOVEQ	#0,D0		;Clear out result

	;------ run through the lists looking for a match
aaa_nextheader:
	TSTNODE A0,A0
	BEQ.S   aaa_fail ;------ check for end of chain
	CMP.L   MH_LOWER(A0),A1
	BCS.S   aaa_nextheader
	CMP.L   MH_UPPER(A0),A1
	BCC.S   aaa_nextheader

	;------ found the correct header, try and get it
	BSR.S	AllocAbsInternal
aaa_fail:
	JMP_PERMIT


*****i* exec.library/AllocAbsInternal *************************************
*
*   SYNOPSIS
*	memoryBlock = AllocAbsInternal(MemHeader,byteSize,location)
*	D0		               A0        D1*      A1
*
*	*unusual location
*
*************************************************************************
*
*	A0-MemHeader
*	A1-startcurrent (D3)
*	A2-prevchunk
*	A3-startwant
*	D0-byteSize
*	D1-***UNUSED***
*	D2-endwant
*	D3-startcurrent (D3)
*	D4-endcurrent
*
AllocAbsInternal:

	MOVEM.L D2-D4/A2-A3,-(SP)
	MOVE.L	D1,D0

	;------ Do a quick check for space:
	CMP.L   MH_FREE(A0),D0
	BHI.S    aa_fail

	MOVE.L  A1,A3		;startwant
	MOVE.L  A1,D2
	ADD.L   D0,D2		;loc+size=endwant

	LEA	MH_FIRST(A0),A2	;prevchunk

	;------ scan list until chunk+chunklen >= endwant
aa1:
	MOVE.L  (A2),D3
	BEQ.S   aa_fail		* end of list
	MOVE.L  D3,A1
	MOVE.L  MC_BYTES(A1),D4
	ADD.L   D3,D4
	CMP.L   D2,D4
	BCC.S   aa_found	;BHS
	MOVE.L  A1,A2	;------ bump next pointer
	BRA.S   aa1

	;------ Found it!
aa_found:
	;------ see if the chunk is large enough
	CMP.L   A3,D3
	BHI.S   aa_fail

	;------ we found the right region.  update the free mem count
	SUB.L   D0,MH_FREE(A0)

	;------ see if we need to split the end of the chunk
	SUB.L   D2,D4		;endcurrent-endwant
	BNE.S   aa_splitsucc

	;------ the ends are the same.  Get the next ptr.
	MOVE.L  (A1),A0
	BRA.S   aa_checkhead

aa_splitsucc:
	;------ get the address of the new node
	LEA     0(A3,D0.L),A0	;just past end of new block
	MOVE.L  (A1),(A0)	;(startcurrent),(newblock)
	MOVE.L  A0,(A1)		;newblock,(startcurrent)
	MOVE.L  D4,MC_BYTES(A0)	;difference is newsize

aa_checkhead:
	CMP.L   A3,D3		;startwant vs. startcurrent
	BEQ.S   aa_removenode

	SUB.L   A3,D3
	NEG.L   D3
	MOVE.L  D3,MC_BYTES(A1)
	BRA.S   aa_done

aa_removenode:
	;------ update the previous pointer
	MOVE.L  A0,(A2)

aa_done:
	MOVE.L  A3,D0
aa_end:
	MOVEM.L (SP)+,D2-D4/A2-A3
	RTS

aa_fail:
	CLEAR   D0
	BRA.S   aa_end


******* exec.library/AvailMem *************************************************
*
*   NAME
*	AvailMem -- memory available given certain requirements
*
*   SYNOPSIS
*	size = AvailMem(attributes)
*	D0		D1
*
*	ULONG AvailMem(ULONG);
*
*   FUNCTION
*	This function returns the amount of free memory given certain
*	attributes.
*
*	To find out what the largest block of a particular type is, add
*	MEMF_LARGEST into the requirements argument.  Returning the largest
*	block is a slow operation.
*
*   WARNING
*	Due to the effect of multitasking, the value returned may not
*	actually be the amount of free memory available at that instant.
*
*   INPUTS
*	requirements - a requirements mask as specified in AllocMem.  Any
*		       of the AllocMem bits are valid, as is MEMF_LARGEST
*		       which returns the size of the largest block matching
*		       the requirements.
*
*   RESULT
*	size - total free space remaining (or the largest free block).
*
*   NOTE
*	For V36 Exec, AvailMem(MEMF_LARGEST) does a consistency check on
*	the memory list.  Alert AN_MemoryInsane will be pulled if any mismatch
*	is noted.
*
*   EXAMPLE
*	AvailMem(MEMF_CHIP|MEMF_LARGEST);
*	/* return size of largest available chip memory chunk */
*
*   SEE ALSO
*	exec/memory.h
*
*******************************************************************************
*
*	For MEMF_LARGEST requests, we must scan the entire memory list.
*	AvailMem now adds up the sizes of all chunks, and compares it to
*	the expected total memory free.  If different, a recoverable
*	alert is pulled.
*
*	Arp avail was broken at one point.  It got confused by MEMF_LOCAL.
*

REQ	EQUR    D1
RESULT	EQUR    D2
TOTAL	EQUR	D3

AvailMem:	MOVEM.L RESULT/TOTAL,-(SP)
		CLEAR   RESULT

		;------ find a free list that matches the requirements
		LEA.L	MemList(A6),A1
		FORBID

av_next:	TSTNODE A1,A1
		BEQ.S   av_exit
		MOVE.W  MH_ATTRIBUTES(A1),D0	; ONLY LOWER WORD ATTR!
		AND.W   REQ,D0
		CMP.W   REQ,D0
		BNE.S   av_next

		BTST.L  #MEMB_LARGEST,REQ
		BNE.S   av_largest
		BTST.L  #MEMB_TOTAL,REQ
		BNE.S	av_total
		;------------------------
		ADD.L   MH_FREE(A1),RESULT	;Case 1: available
		BRA.S   av_next


;----------------------------------------
av_total:	MOVEQ	#MH_SIZE,D0		;Case 2: total
		ADD.L	MH_UPPER(A1),D0
		SUB.L	MH_LOWER(A1),D0
		ADD.L	D0,RESULT
		BRA.S	av_next

;----------------------------------------
av_largest:	MOVE.L  MH_FIRST(A1),D0		;Case 3: largest
		BEQ.S   av_next
		CLEAR	TOTAL

av_scan:	MOVE.L  D0,A0
		MOVE.L	MC_BYTES(A0),D0
		ADD.L	D0,TOTAL
		CMP.L   D0,RESULT
		BGE.S   av_smaller
		MOVE.L  D0,RESULT	;Set new largest chunk
av_smaller:	MOVE.L  (A0),D0		;Next chunk
		BNE.S   av_scan

		CMP.L	MH_FREE(A1),TOTAL
		BEQ.S	av_next

		;Hold on!  The count of all chunks does not match the
		;total memory free.
		AZ_TRIG	2
		MYALERT   AN_MemoryInsane
		BRA.S   av_next
;-----------------------------------------

av_exit:	BSR_PERMIT
		MOVE.L  RESULT,D0
		MOVEM.L (SP)+,RESULT/TOTAL
		RTS


******* exec.library/AllocEntry ***********************************************
*
*   NAME
*	AllocEntry -- allocate many regions of memory
*
*   SYNOPSIS
*	memList = AllocEntry(memList)
*	D0		     A0
*
*	struct MemList *AllocEntry(struct MemList *);
*
*   FUNCTION
*	This function takes a memList structure and allocates enough memory
*	to hold the required memory as well as a MemList structure to keep
*	track of it.
*
*	These MemList structures may be linked together in a task control
*	block to keep track of the total memory usage of this task. (See
*	the description of TC_MEMENTRY under RemTask).
*
*   INPUTS
*	memList -- A MemList structure filled in with MemEntry structures.
*
*   RESULTS
*	memList -- A different MemList filled in with the actual memory
*	    allocated in the me_Addr field, and their sizes in me_Length.
*	    If enough memory cannot be obtained, then the requirements of
*	    the allocation that failed is returned and bit 31 is set.
*
*	    WARNING: The result is unusual!  Bit 31 indicates failure.
*
*   EXAMPLES
*	The user wants five regions of 2, 4, 8, 16, and 32 bytes in size
*	with requirements of MEMF_CLEAR, MEMF_PUBLIC, MEMF_CHIP!MEMF_CLEAR,
*	MEMF_CLEAR, and MEMF_PUBLIC!MEMF_CLEAR respectively.  The
*	following code fragment would do that:
*
*	    MemListDecl:
*		DS.B	LN_SIZE 	    * reserve space for list node
*		DC.W	5		    * number of entries
*		DC.L	MEMF_CLEAR		    * entry #0
*		DC.L	2
*		DC.L	MEMF_PUBLIC		    * entry #1
*		DC.L	4
*		DC.L	MEMF_CHIP!MEMF_CLEAR	    * entry #2
*		DC.L	8
*		DC.L	MEMF_CLEAR	   	    * entry #3
*		DC.L	16
*		DC.L	MEMF_PUBLIC!MEMF_CLEAR	    * entry #4
*		DC.L	32
*
*	    start:
*		LEA.L	MemListDecl(PC),A0
*		JSR	_LVOAllocEntry(a6)
*		BCLR.L	#31,D0
*		BEQ.S	success
*
*		------- Type of memory that we failed on is in D0
*
*   BUGS
*	If any one of the allocations fails, this function fails to back
*	out fully.  This is fixed by the "SetPatch" program on V1.3
*	Workbench disks.
*
*   SEE ALSO
*	exec/memory.h
*
*******************************************************************************
*
*   REGISTER USAGE
*	D2 -- Index of current MemEntry
*	D3 -- Count of total MemEntries
*	A2 -- Ptr to source MemEntry
*	A3 -- Ptr to dest MemEntry
*	A4 -- Ptr to source MemEntry
*
*	:OPTIMIZE: Error bailout could be made smaller by calling FreeEntry.
*

AllocEntry: MOVEM.L D2/D3/A2/A3/A4,-(SP)

	    MOVE.L  A0,A2
	    CLEAR   D3
	    MOVE.W  ML_NUMENTRIES(A2),D3

	    MOVE.L  D3,D0
	    LSL.L   #3,D0		    * multipy by ME_SIZE
	    ADD.L   #ML_SIZE,D0
	    MOVE.L  #MEMF_CLEAR,D1
	    JSRSELF AllocMem
	    MOVE.L  D0,A3
	    MOVE.L  D0,A4
	    TST.L   D0
	    BEQ.S   AllocEntry_Error		*!!CES 22-Oct

*	    ------- Now walk down the source MemEntry, AllocMem'ing as
*	    ------- we go
	    MOVE.W  D3,ML_NUMENTRIES(A3)
	    LEA     ML_ME(A2),A2
	    LEA     ML_ME(A3),A3
	    CLEAR   D2

AllocEntry_ME:
	    MOVE.L  ME_REQS(A2),D1
	    MOVE.L  ME_LENGTH(A2),D0
	    MOVE.L  D0,ME_LENGTH(A3)
	    BEQ.S   AllocEntry_PostAM
	    JSRSELF AllocMem
	    TST.L   D0
	    BEQ.S   AllocEntry_Free
AllocEntry_PostAM:
	    MOVE.L  D0,ME_ADDR(A3)
	    ADDQ.L  #ME_SIZE,A2
	    ADDQ.L  #ME_SIZE,A3
	    ADDQ.W  #1,D2
	    SUBQ.L  #1,D3
	    BNE.S   AllocEntry_ME


	    MOVE.L  A4,D0
AllocEntry_End:
	    MOVEM.L (SP)+,D2/D3/A2/A3/A4
	    RTS


*	    ------- Free all the memory we have collected
AllocEntry_Free:

AllocEntry_FLoop:
	    SUBQ.W  #1,D2			;Free collected memory
	    BMI.S   AllocEntry_FEnd
	    SUBQ.L  #ME_SIZE,A3
	    MOVE.L  ME_ADDR(A3),A1
	    MOVE.L  ME_LENGTH(A3),D0
	    JSRSELF FreeMem
	    BRA.S   AllocEntry_FLoop

AllocEntry_FEnd:
	    CLEAR   D0				;Free MemHeader
	    MOVE.W  ML_NUMENTRIES(A4),D0
	    LSL.L   #3,D0		    * multiply by ME_SIZE
	    ADD.L   #ML_SIZE,D0
	    MOVE.L  A4,A1
	    JSRSELF FreeMem
	    MOVE.L  ME_REQS(A2),D0          * Reqs the failure occured on

AllocEntry_Error:
	    BSET.L  #31,D0			    *!!CES 22-Oct
	    BRA.S   AllocEntry_End


******* exec.library/FreeEntry ************************************************
*
*   NAME
*	FreeEntry -- free many regions of memory
*
*   SYNOPSIS
*	FreeEntry(memList)
*		  A0
*	void FreeEntry(struct MemList *);
*
*   FUNCTION
*	This function takes a memList structure (as returned by AllocEntry)
*	and frees all the entries.
*
*   INPUTS
*	memList -- pointer to structure filled in with MemEntry
*		   structures
*
*   SEE ALSO
*	AllocEntry
*
*******************************************************************************
*
*   REGISTER USAGE
*
*	A2 -- Ptr to MemList
*	A3 -- Ptr to MemEntry
*	D2 -- numelements
*
*

FreeEntry:
	MOVEM.L D2/A2/A3,-(SP)

	MOVE.L  A0,A2

FreeEnt_MLloop:
	LEA	ML_ME(A2),A3
	MOVE.W  ML_NUMENTRIES(A2),D2
	BRA.S   FreeEnt_MEstart

FreeEnt_MEloop:
	MOVE.L  ME_ADDR(A3),A1
	MOVE.L  ME_LENGTH(A3),D0
	BEQ.S   FreeEnt_MEpostfree

	JSRSELF FreeMem

FreeEnt_MEpostfree:
	ADDQ.L  #ME_SIZE,A3
FreeEnt_MEstart:
	DBF	D2,FreeEnt_MEloop

	;------ Free the list itself
	CLEAR   D0
	MOVE.W  ML_NUMENTRIES(A2),D0
	LSL.L   #3,D0			* multiply by ME_SIZE
	ADD.L   #ML_SIZE,D0
	MOVE.L  A2,A1
	JSRSELF FreeMem

FreeEnt_End:
	MOVEM.L (SP)+,D2/A2/A3

	RTS


******* exec.library/AddMemList ***********************************************
*
*   NAME
*	AddMemList - add memory to the system free pool
*
*   SYNOPSIS
*	AddMemList( size, attributes, pri, base, name )
*                     D0      D1        D2   A0    A1
*
*	void AddMemList(ULONG, ULONG, LONG, APTR, STRPTR);
*
*   FUNCTION
*	Add a new region of memory to the system free pool.  The first few
*	bytes will be used to hold the MemHeader structure.  The remainder
*	will be made available to the rest of the world.
*
*   INPUTS
*	size - the size (in bytes) of the memory area
*	attributes - the attributes word that the memory pool will have
*	pri  - the priority for this memory.  CHIP memory has a pri of -10,
*	       16 bit expansion memory has a priority of 0.  The higher the
*	       priority, the closer to the head of the memory list it will
*	       be placed.
*	base - the base of the new memory area
*	name - the name that will be used in the memory header, or NULL
*	       if no name is to be provided.  This name is not copied, so it
*	       must remain valid for as long as the memory header is in the
*	       system.
*
*   NOTES
*	*DO NOT* add memory to the system with the attribute of MEMF_KICK.
*	EXEC will mark your memory as such if it is of the right type.
*
*   SEE ALSO
*	AllocMem, exec/memory.h
*
*******************************************************************************
AddMemList:	bsr.s	AddMemListInternal

		;------ and add the list to the system
		move.l	a0,a1
		lea.l	MemList(a6),a0
		bra	AddNode			;exit

*****i* Exec/Internal/AddMemListInternal ***************************************
*
* MemHeader=AddMemListInternal( size, attributes, pri, base, name )
*	A0                      D0    D1          D2   A0    A1
*
********************************************************************************

AddMemListInternal:

;------ clear out links
	clr.l	(A0)
	clr.l	LN_PRED(A0)

;------ set up node
	move.b	#NT_MEMORY,LN_TYPE(a0)
	move.b	d2,LN_PRI(a0)
	move.l  a1,LN_NAME(a0)

;------ set up memheader
	move.w	d1,MH_ATTRIBUTES(a0)

;------ a0 is the base of the mem header, a1 is the
;------ base of the first mem chunk.  The memheader is carved from
;------ the start of the new memory area.
	lea	MH_SIZE(a0),a1

;------ make sure free block is long aligned
	move.l	a1,d1
	addq.l	#MEM_BLOCKMASK,d1
	and.w	#~MEM_BLOCKMASK,d1
	exg     d1,a1			; a1 is now rounded up

;------ adjust size by the amount we rounded up
	sub.l	a1,d1			; a1 <= d1, so 0 >= d1 > -7
	add.l   d1,d0

;------ and that the size is long aligned
	and.w	#~MEM_BLOCKMASK,d0
	sub.l	#MH_SIZE,d0

;------ finish with the mem headers
	move.l	a1,MH_FIRST(a0)
	move.l	a1,MH_LOWER(a0)
	move.l	a1,d1
	add.l	d0,d1
	move.l	d1,MH_UPPER(a0)
	move.l	d0,MH_FREE(a0)

;------ set up the first mem chunk
	clr.l	(a1)			;MC_NEXT
	move.l	d0,MC_BYTES(a1)
	rts				;Return MemHeader in A0

*******************************************************************************
*  ****************************              *******************************  *
* ****************************** EXEC Pools ********************************* *
*  ****************************              *******************************  *
*******************************************************************************

 STRUCTURE PrivatePool,0
	STRUCT	PP_List,MLH_SIZE	; The pool's free list
	ULONG	PP_Flags		; The pool's memory type
	ULONG	PP_Size			; The pool's puddle size
	ULONG	PP_Thresh		; The pool's threshhold size
	LABEL	PP_SIZEOF

 STRUCTURE LargePuddle,0
	STRUCT	LP_Node,MLN_SIZE	; This puddle's node
	ULONG	LP_Flag			; NULL if node is large puddle
	LABEL	LP_SIZEOF

******* exec.library/CreatePool ***********************************************
*
*    NAME
*	CreatePool -- Generate a private memory pool header (V39)
*
*    SYNOPSIS
*	newPool=CreatePool(memFlags,puddleSize,threshSize)
*	a0                 d0       d1         d2
*
*	void *CreatePool(ULONG,ULONG,ULONG);
*
*    FUNCTION
*	Allocate and prepare a new memory pool header.	Each pool is a
*	separate tracking system for memory of a specific type.  Any number
*	of pools may exist in the system.
*
*	Pools automatically expand and shrink based on demand.	Fixed sized
*	"puddles" are allocated by the pool manager when more total memory
*	is needed.  Many small allocations can fit in a single puddle.
*	Allocations larger than the threshSize are allocation in their own
*	puddles.
*
*	At any time individual allocations may be freed.  Or, the entire
*	pool may be removed in a single step.
*
*    INPUTS
*	memFlags - a memory flags specifier, as taken by AllocMem.
*	puddleSize - the size of Puddles...
*	threshSize - the largest allocation that goes into normal puddles
*	             This *MUST* be less than or equal to puddleSize
*	             (CreatePool() will fail if it is not)
*
*    RESULT
*	The address of a new pool header, or NULL for error.
*
*    SEE ALSO
*	DeletePool(), AllocPooled(), FreePooled(), exec/memory.i
*
*******************************************************************************

CreatePool:	sub.l	a0,a0		; Clear a0...
		cmp.l	d2,d1		; Check threshold for too large...
		bcs.s	cp_BadThresh	; Threshold is larger than puddle?!!
		move.l	d0,-(sp)	; Save the flags...
		moveq.l	#MEM_BLOCKMASK,d0	; Get size mask...
		add.l	d0,d1		; Round to the
		not.b	d0		; ... memory block size
		and.b	d0,d1		; ... before we save it
		move.l	d1,-(sp)	; Save the puddle size...
		moveq.l	#0,d1		; Get *ANY* memory for the header
		moveq.l	#PP_SIZEOF,d0	; Size of the pool header...
		JSRSELF	AllocMem	; Get the memory...
		move.l	(sp)+,d1	; Get back from the stack...
		move.l	(sp)+,a0	; the data needed...
		tst.l	d0		; Check it out
		beq.s	cp_NoMem	; If no memory, exit...
		exg	d0,a0		; Swap A0/D0
		bsr	NewList		; (a0 - Preserved)
		lea	PP_Flags(a0),a1	; Get a pointer to the flags...
		move.l	d0,(a1)+	; Set up the flags
		move.l	d1,(a1)+	; Set up the size
		move.l	d2,(a1)+	; Set up the thresh
cp_BadThresh:	move.l	a0,d0		; Get the return result...
cp_NoMem:	rts

******* exec.library/DeletePool ***********************************************
*
*    NAME
*	DeletePool --  Drain an entire memory pool (V39)
*
*    SYNOPSIS
*	DeletePool(poolHeader)
*	           a0
*
*	void DeletePool(void *);
*
*    FUNCTION
*	Frees all memory in all pudles of the specified pool header, then
*	deletes the pool header.  Individual free calls are not needed.
*
*    INPUTS
*	poolHeader - as returned by CreatePool().
*
*    SEE ALSO
*	CreatePool(), AllocPooled(), FreePooled()
*
*******************************************************************************

DeletePool:	move.l	a0,d0		; Check for death
		beq.s	dp_NoPool	; If no pool...
		movem.l	a2/d2,-(sp)	; Save on the stack...
		move.l	(a0),a2		; Point at first node
dp_puddles:	move.l	(a2),d2		; Check if next puddle is NULL
		beq.s	dp_header	; If NULL, we are at the header...
		move.l	a2,a1		; Get ready for FreeVec
		move.l	d2,a2		; Get my next pointer
		JSRSELF	FreeVec		; Free the memory
		bra.s	dp_puddles	; And loop for more...
dp_NoPool:	rts
*
* Ok, we went through the whole list and a2 is now pointing at MLH_TAIL
* so subtract 4 and then free it too.
*
dp_header:	move.l	a2,a1		; Get into register...
		subq.l	#4,a1		; Back to top of header...
		movem.l	(sp)+,a2/d2	; Get registers back from the stack
		moveq.l	#PP_SIZEOF,d0	; Get size of pool header...
		JMPSELF	FreeMem		; Release the header

******* exec.library/AllocPooled **********************************************
*
*    NAME
*	AllocPooled -- Allocate memory with the pool manager (V39)
*
*    SYNOPSIS
*	memory=AllocPooled(poolHeader,memSize)
*	d0                 a0         d0
*
*	void *AllocPooled(void *,ULONG);
*
*    FUNCTION
*	Allocate memSize bytes of memory, and return a pointer. NULL is
*	returned if the allocation fails.
*
*	Doing a DeletePool() on the pool will free all of the puddles
*	and thus all of the allocations done with AllocPooled() in that
*	pool.  (No need to FreePooled() each allocation)
*
*    INPUTS
*	memSize - the number of bytes to allocate
*	poolHeader - a specific private pool header.
*
*    RESULT
*	A pointer to the memory, or NULL.
*	The memory block returned is long word aligned.
*
*    NOTES
*	To track sizes yourself, the following code can be used:
*
*	;
*	; Function to do AllocVecPooled(Pool,memSize)
*	;
*	AllocVecPooled:	addq.l	#4,d0		; Get space for tracking
*			move.l	d0,-(sp)	; Save the size
*			jsr	_LVOAllocPooled(a6)	; Call pool...
*			move.l	(sp)+,d1	; Get size back...
*			tst.l	d0		; Check for error
*			beq.s	avp_fail	; If NULL, failed!
*			move.l	d0,a0		; Get pointer...
*			move.l	d1,(a0)+	; Store size
*			move.l	a0,d0		; Get result
*	avp_fail:	rts			; return
*
*	;
*	; Function to do FreeVecPooled(pool,memory)
*	;
*	FreeVecPooled:	move.l	-(a1),d0	; Get size / ajust pointer
*			jmp	_LVOFreePooled(a6)
*
*    SEE ALSO
*	FreePooled(), CreatePool(), DeletePool()
*
*******************************************************************************

AllocPooled:	move.l	d0,d1		; Save for a moment (and check)
		beq.s	ap_NoPool	; If NULL size, silly call ;^)
		move.l	a0,d0		; Get pool header
		beq.s	ap_NoPool	; If NULL, we exit
*
* Ok, so we look like it will actually fly.  So set up our registers as needed
*
		movem.l	a2/a3/d2/d3,-(sp)	; Save for later...
		move.l	d1,d2			; Save of the needed allocation
		move.l	a0,a2			; Pool for the allocation
		cmp.l	PP_Thresh(a2),d2	; Check against threshold
		bcc.s	ap_largeAlloc		; If larger, we do large alloc
*
* Ok, now we need to do a search for space...
*
ap_LookAgain:	move.l	(a2),d3			; Get first free list...
ap_Looking:	move.l	d3,a3			; Next node...
		move.l	(a3),d3			; Get next node...
		beq.s	ap_newPuddle		; If end, we need a new puddle
		move.l	a3,a0			; Get it ready...
		tst.l	LP_Flag(a0)		; If large puddle, end of list
		beq.s	ap_newPuddle		; (large puddle are at tail...)
		move.l	d2,d0			; Get the allocation size
		bsr	Allocate		; Try to allocate
		;tst.l	d0	;cc's set by Allocate
		beq.s	ap_Looking		; We are still looking...
*
* Now, we need to do the clearing if we have MEMF_CLEAR set...
*
		move.l	d0,a0			; Get into a0
		move.l	PP_Flags(a2),d0		; Get flags
		btst.l	#MEMB_CLEAR,d0		; Are we MEMF_CLEAR?
		beq.s	ap_Exit			; If not, exit
		move.l	a0,a1			; Get data block...
		move.l	d2,d1			; Get size...
		addq.l	#7,d1			; To block size rounding
		lsr.l	#3,d1			; Number of 8-byte chunks
		subq.l	#1,d1			; minus 1...
		move.w	d1,d0			; Get low word of size
		swap	d1			; Get high word of size
		moveq.l	#0,d3			; Get a NULL...
ap_ClearLoop:	move.l	d3,(a1)+		; Clear 4 bytes
		move.l	d3,(a1)+		; and the next 4...
		dbra.s	d0,ap_ClearLoop		; Clear loop...
		dbra.s	d1,ap_ClearLoop		; outer (large) loop
*
* Now return pointer to data...
*
ap_Exit:	move.l	a0,d0			; Point at memory available
ap_Restore:	movem.l	(sp)+,a2/a3/d2/d3	; Restore
ap_NoPool:	rts
*
* For allocations larger than the threshold...
*
ap_largeAlloc:	move.l	d2,d0			; Size of allocation
		addq.l	#8,d0			; My list node (minnode)
		addq.l	#4,d0			; My flag (That this is large)
		move.l	PP_Flags(a2),d1		; Get the flags
		JSRSELF	AllocVec		; Allocate it
		tst.l	d0			; Check for error
		beq.s	ap_Restore		; If no pool...
		move.l	d0,a1			; Get the node
		move.l	a2,a0			; Get the list
		ADDTAIL				; Place it onto the list
		addq.l	#8,a1			; Point past the node
		clr.l	(a1)+			; Clear my flag...
		move.l	a1,d0			; Put it into d0...
		bra.s	ap_Restore		; Go and exit with memory
*
* Ok, so we did not find space in the current puddles, so try to make a new
* new puddle...
*
ap_newPuddle:	move.l	PP_Flags(a2),d1		; Memory flags
		move.l	PP_Size(a2),d0		; Get size of puddle to make
		add.l	#MH_SIZE+4,d0		; Size of puddle header
		JSRSELF	AllocVec		; Allocate the memory
		tst.l	d0			; Did we get it?
		beq.s	ap_Restore		; If no memory...
		move.l	d0,a3			; Set up puddle pointer...
		move.l	d0,a1			; Get into a1
		move.l	a2,a0			; Pool header
		ADDHEAD				; Add it to the pool
		moveq.l	#NT_MEMORY,d0		; Get memory type...
		move.b	d0,LN_TYPE(a3)		; Set up the type
		move.b	d0,LN_PRI(a3)		; (Cheat: Priority field ;^)
		lea	PoolName(pc),a0		; Get string "Pool"
		move.l	a0,LN_NAME(a3)		; Name the pool...
		move.l	PP_Flags(a2),d1		; Get flags
		move.w	d1,MH_ATTRIBUTES(a3)	; Set the attributes
		lea	MH_SIZE+4(a3),a0	; Point at first free...
***
*** Now need to check to make sure address is double-long alligned
***
		moveq.l	#7,d1			; Get d1 to be 7...
		not.l	d1			; Invert... (bottom 3 bits 0)
		move.l	a0,d0			; Get address into register...
		and.l	d0,d1			; Mask off the bottom 3
		move.l	d1,a0			; Address now double-long
***
		move.l	PP_Size(a2),d0		; Chunk byte size
		move.l	a0,MH_FIRST(a3)		; Point at the first free
		move.l	a0,MH_LOWER(a3)		; Point at the lower bound
		move.l	d0,MH_FREE(a3)		; Set the FREE space
		clr.l	MC_NEXT(a0)		; Clear next pointer
		move.l	d0,MC_BYTES(a0)		; Set the chunk size
		add.l	d0,a0			; Add in the size
		move.l	a0,MH_UPPER(a3)		; Point at end of puddle+1
		bra	ap_LookAgain		; Look again (new puddle ;^)

******* exec.library/FreePooled ***********************************************
*
*    NAME
*	FreePooled -- Free pooled memory  (V39)
*
*    SYNOPSIS
*	FreePooled(poolHeader,memory,memSize)
*		   a0         a1     d0
*
*	void FreePooled(void *,void *,ULONG);
*
*    FUNCTION
*	Deallocates memory allocated by AllocPooled().  The size of the
*	allocation *MUST* match the size given to AllocPooled().
*	The reason the pool functions do not track individual allocation
*	sizes is because many of the uses of pools have small allocation
*	sizes and the tracking of the size would be a large overhead.
*
*	Only memory allocated by AllocPooled() may be freed with this
*	function!
*
*	Doing a DeletePool() on the pool will free all of the puddles
*	and thus all of the allocations done with AllocPooled() in that
*	pool.  (No need to FreePooled() each allocation)
*
*    INPUTS
*	memory - pointer to memory allocated by AllocPooled.
*	poolHeader - a specific private pool header.
*
*    NOTES
*	To track sizes yourself, the following code can be used:
*
*	;
*	; Function to do AllocVecPooled(Pool,memSize)
*	;
*	AllocVecPooled:	addq.l	#4,d0		; Get space for tracking
*			move.l	d0,-(sp)	; Save the size
*			jsr	_LVOAllocPooled(a6)	; Call pool...
*			move.l	(sp)+,d1	; Get size back...
*			tst.l	d0		; Check for error
*			beq.s	avp_fail	; If NULL, failed!
*			move.l	d0,a0		; Get pointer...
*			move.l	d1,(a0)+	; Store size
*			move.l	a0,d0		; Get result
*	avp_fail:	rts			; return
*
*	;
*	; Function to do FreeVecPooled(pool,memory)
*	;
*	FreeVecPooled:	move.l	-(a1),d0	; Get size / ajust pointer
*			jmp	_LVOFreePooled(a6)
*
*    SEE ALSO
*	AllocPooled(), CreatePool(), DeletePool()
*
*******************************************************************************

FreePooled:	move.l	a0,d1			; Check if NULL pool
		beq.s	fp_NoMem		; If NULL, we exit...
		move.l	a1,d1			; Check if NULL memory
		beq.s	fp_NoMem		; If NULL, we exit...
		cmp.l	PP_Thresh(a0),d0	; Now check threshold
		bcc.s	fp_largeFree		; If large one, do that...
		move.l	a3,-(sp)		; Save some registers...
		move.l	(a0),d1			; Get ready for loop
fp_Looking:	move.l	d1,a3			; Get node
		move.l	(a3),d1			; Get next node
		beq.s	fp_Error		; If not found we be in trouble
		tst.l	LP_Flag(a3)		; Is it a large puddle
		beq.s	fp_Error		; (They are at tail)
		cmp.l	MH_LOWER(a3),a1		; Are we in this one
		bcs.s	fp_Looking		; If not...
		cmp.l	MH_UPPER(a3),a1		; ...just keep
		bcc.s	fp_Looking		; ...on looking
		move.l	a3,a0			; Get memory header
		bsr	Deallocate		; Free this memory...
*******
* It may be usefull to bubble up pools that seem to be
* getting used more...
*******
*
* Now, bubble the node up one on the list (a performance tuning issue)
* We buble the node in a3
* We trash a0, a1, and d1
*
bubble:		move.l	LN_PRED(a3),a1		; Get previous node
		move.l	LN_PRED(a1),d1		; Get node before that
		beq.s	nobubble		; If NULL, no bubble...
*
* This code here moves the node in a3 above the node in a1
* The node a1 is the node just above the node a3 before this.
*
		move.l	d1,a0			; Get node-2
		move.l	a3,LN_SUCC(a0)		; Point down...
		move.l	a0,LN_PRED(a3)		; Point up...
		move.l	a3,LN_PRED(a1)		; Swap it...
		move.l	LN_SUCC(a3),a0		; Get next node
		move.l	a0,LN_SUCC(a1)		; Point down
		move.l	a1,LN_PRED(a0)		; Point up...
		move.l	a1,LN_SUCC(a3)		; Into both nodes
*
* Ok, a3 is now one higher
*
nobubble:
*
		move.l	a3,a1			; Get node into a1
		move.l	(sp)+,a3		; Restore a3
		move.l	MH_SIZE(a1),d0		; Get size...
		add.l	MH_LOWER(a1),d0		; Add in lower bound
		sub.l	MH_UPPER(a1),d0		; Subtract upper bound
		beq.s	fp_FreeNode		; If NULL, it is empty
fp_NoMem:	rts
fp_largeFree:	subq.l	#4,a1		; The special flag	:speed reasons
		subq.l	#8,a1		; The "node" structure
*
* So free the node in a1 (after removing it first!
*
fp_FreeNode:	move.l	a1,d0		; Save the value of a1
		REMOVE			; Remove (a1) from list (a0 trashed)
		move.l	d0,a1		; Get pointer back
		JMPSELF	FreeVec		; Free it...
***************
fp_Error:	move.l	(sp)+,a3	; Restore a3
		MYALERT	AN_BadFreeAddr	; Bad boy...
		rts

		dc.b	0		; Make the name ODD
PoolName:	dc.b	'Pool',0	; Now word aligned again...


	END
