
                include     "exec/types.i"
                include     "exec/memory.i"
                include     "exec/lists.i"
                include     "exec/alerts.i"
                include     "nipcbase.i"

USE_POOLS_CODE	EQU	1
**
** SYS macro pulls ExecBase out of the nipc library base, and calls
** the appropriate exec.library function.
**
SYS             macro
                move.l      a6,-(sp)
                move.l      nb_SysBase(a6),a6
                jsr         _LVO\1(a6)
                move.l      (sp)+,a6
                endm

XSYS            macro
                xref        _LVO\1
                endm

                xdef        _ECreatePool,_EDeletePool,_EAllocPooled,_EFreePooled
                XSYS        AllocMem
                XSYS        FreeMem
                XSYS        AllocVec
                XSYS        FreeVec
                XSYS        Deallocate
                XSYS        Allocate
                XSYS        ObtainSemaphore
                XSYS        ReleaseSemaphore
		XSYS	    AddHead
		XSYS	    Remove

;TRACK_POOLS	SET	1

		IFD	TRACK_POOLS

		STRUCTURE   PoolAlloc,0
		    STRUCT	PA_Node,MLN_SIZE
		    ULONG	PA_Size
		    LABEL	PA_SIZEOF

		ENDC
*******************************************************************************
*  ****************************              *******************************  *
* ****************************** EXEC Pools ********************************* *
*  ****************************              *******************************  *
*******************************************************************************

** Ahem, stolen from V39's EXEC, as we're required to run with V37 ...
** The little editing I had to do was:
** Add a routine for NewList.
** Call AllocVec/FreeVec through the LVOs.
**      -- Added a special macro for SYS
**      -- Added XREF's for each Exec function (added XSYS macro)
** Add 'C' style names, and XDEF them.
** dEleted MYALERT

 STRUCTURE PrivatePool,0
        STRUCT  PP_List,MLH_SIZE        ; The pool's free list
        ULONG   PP_Flags                ; The pool's memory type
        ULONG   PP_Size                 ; The pool's puddle size
        ULONG   PP_Thresh               ; The pool's threshhold size

        IFD	TRACK_POOLS
        STRUCT  PP_Track,MLH_SIZE	; Tracking list
        ENDC

        LABEL   PP_SIZEOF

 STRUCTURE LargePuddle,0
        STRUCT  LP_Node,MLN_SIZE        ; This puddle's node
        ULONG   LP_Flag                 ; NULL if node is large puddle
        LABEL   LP_SIZEOF

******* exec.library/CreatePool ***********************************************
*
*    NAME
*       CreatePool -- Generate a private memory pool header (V39)
*
*    SYNOPSIS
*       newPool=CreatePool(memFlags,puddleSize,threshSize)
*       a0                 d0       d1         d2
*
*       void *CreatePool(ULONG,ULONG,ULONG);
*
*    FUNCTION
*       Allocate and prepare a new memory pool header.  Each pool is a
*       separate tracking system for memory of a specific type.  Any number
*       of pools may exist in the system.
*
*       Pools automatically expand and shrink based on demand.  Fixed sized
*       "puddles" are allocated by the pool manager when more total memory
*       is needed.  Many small allocations can fit in a single puddle.
*       Allocations larger than the threshSize are allocation in their own
*       puddles.
*
*       At any time individual allocations may be freed.  Or, the entire
*       pool may be removed in a single step.
*
*    INPUTS
*       memFlags - a memory flags specifier, as taken by AllocMem.
*       puddleSize - the size of Puddles...
*       threshSize - the largest allocation that goes into normal puddles
*                    This *MUST* be less than or equal to puddleSize
*                    (CreatePool() will fail if it is not)
*
*    RESULT
*       The address of a new pool header, or NULL for error.
*
*    SEE ALSO
*       DeletePool(), AllocPooled(), FreePooled(), exec/memory.i
*
*******************************************************************************

_ECreatePool:   sub.l   a0,a0           ; Clear a0...
                cmp.l   d2,d1           ; Check threshold for too large...
                bcs.s   cp_BadThresh    ; Threshold is larger than puddle?!!
                move.l  d0,-(sp)        ; Save the flags...
                moveq.l #MEM_BLOCKMASK,d0       ; Get size mask...
                add.l   d0,d1           ; Round to the
                not.b   d0              ; ... memory block size
                and.b   d0,d1           ; ... before we save it
                move.l  d1,-(sp)        ; Save the puddle size...
                moveq.l #0,d1           ; Get *ANY* memory for the header
                moveq.l #PP_SIZEOF,d0   ; Size of the pool header...
                SYS     AllocMem        ; Get the memory...
                move.l  (sp)+,d1        ; Get back from the stack...
                move.l  (sp)+,a0        ; the data needed...
                tst.l   d0              ; Check it out
                beq.s   cp_NoMem        ; If no memory, exit...
                exg     d0,a0           ; Swap A0/D0
                bsr     NewList         ; (a0 - Preserved)

                IFD	TRACK_POOLS
                move.l	a0,-(sp)
                lea.l	PP_Track(a0),a0
                bsr	NewList
                movea.l	(sp)+,a0
                ENDC

                lea     PP_Flags(a0),a1 ; Get a pointer to the flags...
                move.l  d0,(a1)+        ; Set up the flags
                move.l  d1,(a1)+        ; Set up the size
                move.l  d2,(a1)+        ; Set up the thresh
cp_BadThresh:   move.l  a0,d0           ; Get the return result...
cp_NoMem:       rts

******* exec.library/DeletePool ***********************************************
*
*    NAME
*       DeletePool --  Drain an entire memory pool (V39)
*
*    SYNOPSIS
*       DeletePool(poolHeader)
*                  a0
*
*       void DeletePool(void *);
*
*    FUNCTION
*       Frees all memory in all pudles of the specified pool header, then
*       deletes the pool header.  Individual free calls are not needed.
*
*    INPUTS
*       poolHeader - as returned by CreatePool().
*
*    SEE ALSO
*       CreatePool(), AllocPooled(), FreePooled()
*
*******************************************************************************

_EDeletePool:   move.l  a0,d0           ; Check for death
                beq.s   dp_NoPool       ; If no pool...
                movem.l a2/d2,-(sp)     ; Save on the stack...
                move.l  (a0),a2         ; Point at first node
dp_puddles:     move.l  (a2),d2         ; Check if next puddle is NULL
                beq.s   dp_header       ; If NULL, we are at the header...
                move.l  a2,a1           ; Get ready for FreeVec
                move.l  d2,a2           ; Get my next pointer
                SYS     FreeVec         ; Free the memory
                bra.s   dp_puddles      ; And loop for more...
dp_NoPool:      rts
*
* Ok, we went through the whole list and a2 is now pointing at MLH_TAIL
* so subtract 4 and then free it too.
*
dp_header:      move.l  a2,a1           ; Get into register...
                subq.l  #4,a1           ; Back to top of header...
                movem.l (sp)+,a2/d2     ; Get registers back from the stack
                moveq.l #PP_SIZEOF,d0   ; Get size of pool header...
                SYS     FreeMem         ; Release the header
                rts

******* exec.library/AllocPooled **********************************************
*
*    NAME
*       AllocPooled -- Allocate memory with the pool manager (V39)
*
*    SYNOPSIS
*       memory=AllocPooled(poolHeader,memSize)
*       d0                 a0         d0
*
*       void *AllocPooled(void *,ULONG);
*
*    FUNCTION
*       Allocate memSize bytes of memory, and return a pointer. NULL is
*       returned if the allocation fails.
*
*       Doing a DeletePool() on the pool will free all of the puddles
*       and thus all of the allocations done with AllocPooled() in that
*       pool.  (No need to FreePooled() each allocation)
*
*    INPUTS
*       memSize - the number of bytes to allocate
*       poolHeader - a specific private pool header.
*
*    RESULT
*       A pointer to the memory, or NULL.
*       The memory block returned is long word aligned.
*
*    NOTES
*       To track sizes yourself, the following code can be used:
*
*       ;
*       ; Function to do AllocVecPooled(Pool,memSize)
*       ;
*       AllocVecPooled: addq.l  #4,d0           ; Get space for tracking
*                       move.l  d0,-(sp)        ; Save the size
*                       jsr     _LVOAllocPooled(a6)     ; Call pool...
*                       move.l  (sp)+,d1        ; Get size back...
*                       tst.l   d0              ; Check for error
*                       beq.s   avp_fail        ; If NULL, failed!
*                       move.l  d0,a0           ; Get pointer...
*                       move.l  d1,(a0)+        ; Store size
*                       move.l  a0,d0           ; Get result
*       avp_fail:       rts                     ; return
*
*       ;
*       ; Function to do FreeVecPooled(pool,memory)
*       ;
*       FreeVecPooled:  move.l  -(a1),d0        ; Get size / ajust pointer
*                       jmp     _LVOFreePooled(a6)
*
*    SEE ALSO
*       FreePooled(), CreatePool(), DeletePool()
*
*******************************************************************************

_EAllocPooled:
		IFEQ	USE_POOLS_CODE
		moveq.l	#0,d1
		SYS	AllocMem
		rts
		ENDC
* Lock the pool
                movem.l d0/d1/a0/a1,-(sp)
                lea.l   PoolLock(a6),a0
                SYS     ObtainSemaphore
                movem.l (sp)+,d0/d1/a0/a1

		IFD	TRACK_POOLS
		add.l	#PA_SIZEOF,d0
		ENDC

                move.l  d0,d1           ; Save for a moment (and check)
                beq     ap_NoPool       ; If NULL size, silly call ;^)
                move.l  a0,d0           ; Get pool header
                beq     ap_NoPool       ; If NULL, we exit
*
* Ok, so we look like it will actually fly.  So set up our registers as needed
*
                movem.l a2/a3/d2/d3,-(sp)       ; Save for later...
                move.l  d1,d2                   ; Save of the needed allocation
                move.l  a0,a2                   ; Pool for the allocation
                cmp.l   PP_Thresh(a2),d2        ; Check against threshold
                bcc     ap_largeAlloc           ; If larger, we do large alloc
*
* Ok, now we need to do a search for space...
*
ap_LookAgain:   move.l  (a2),d3                 ; Get first free list...
ap_Looking:     move.l  d3,a3                   ; Next node...
                move.l  (a3),d3                 ; Get next node...
                beq     ap_newPuddle            ; If end, we need a new puddle
                move.l  a3,a0                   ; Get it ready...
                tst.l   LP_Flag(a0)             ; If large puddle, end of list
                beq     ap_newPuddle            ; (large puddle are at tail...)
                move.l  d2,d0                   ; Get the allocation size
                SYS     Allocate                ; Try to allocate
                tst.l   d0
                beq.s   ap_Looking              ; We are still looking...
*
* Now, we need to do the clearing if we have MEMF_CLEAR set...
*
                move.l  d0,a0                   ; Get into a0
                move.l  PP_Flags(a2),d0         ; Get flags
                btst.l  #MEMB_CLEAR,d0          ; Are we MEMF_CLEAR?
                beq.s   ap_Exit                 ; If not, exit
                move.l  a0,a1                   ; Get data block...
                move.l  d2,d1                   ; Get size...
                addq.l  #7,d1                   ; To block size rounding
                lsr.l   #3,d1                   ; Number of 8-byte chunks
                subq.l  #1,d1                   ; minus 1...
                move.w  d1,d0                   ; Get low word of size
                swap    d1                      ; Get high word of size
                moveq.l #0,d3                   ; Get a NULL...
ap_ClearLoop:   move.l  d3,(a1)+                ; Clear 4 bytes
                move.l  d3,(a1)+                ; and the next 4...
                dbra.s  d0,ap_ClearLoop         ; Clear loop...
                dbra.s  d1,ap_ClearLoop         ; outer (large) loop
*
* Now return pointer to data...
*
ap_Exit:        move.l  a0,d0                   ; Point at memory available

		; If we're tracking, then this allocation to our
		; tracking list and bump the returned pointer.

ap_Restore:
		IFD	TRACK_POOLS
		tst.l	d0			; Don't add a failed alloc!
		beq.b	1$
		move.l	d0,-(sp)		; Save a0
		movea.l	d0,a1			; Node
		move.l	d2,PA_Size(a1)		; Store allocation size
		lea.l	PP_Track(a2),a0		; List
		SYS	AddHead
		move.l	(sp)+,d0		; Restore to d0
		add.l	#PA_SIZEOF,d0		; Skip over header
1$
		ENDC

		movem.l (sp)+,a2/a3/d2/d3       ; Restore
ap_NoPool:

                movem.l d0/d1/a0/a1,-(sp)
                lea.l   PoolLock(a6),a0
                SYS     ReleaseSemaphore
                movem.l (sp)+,d0/d1/a0/a1

                rts
*
* For allocations larger than the threshold...
*
ap_largeAlloc:  move.l  d2,d0                   ; Size of allocation
                addq.l  #8,d0                   ; My list node (minnode)
                addq.l  #4,d0                   ; My flag (That this is large)
                move.l  PP_Flags(a2),d1         ; Get the flags
                SYS     AllocVec                ; Allocate it
                tst.l   d0                      ; Check for error
                beq.s   ap_Restore              ; If no pool...
                move.l  d0,a1                   ; Get the node
                move.l  a2,a0                   ; Get the list
                ADDTAIL                         ; Place it onto the list
                addq.l  #8,a1                   ; Point past the node
                clr.l   (a1)+                   ; Clear my flag...
                move.l  a1,d0                   ; Put it into d0...
                bra.s   ap_Restore              ; Go and exit with memory
*
* Ok, so we did not find space in the current puddles, so try to make a new
* new puddle...
*
ap_newPuddle:   move.l  PP_Flags(a2),d1         ; Memory flags
                move.l  PP_Size(a2),d0          ; Get size of puddle to make
                add.l   #MH_SIZE+4,d0           ; Size of puddle header
                SYS     AllocVec                ; Allocate the memory
                tst.l   d0                      ; Did we get it?
                beq     ap_Restore              ; If no memory...
                move.l  d0,a3                   ; Set up puddle pointer...
                move.l  d0,a1                   ; Get into a1
                move.l  a2,a0                   ; Pool header
                ADDHEAD                         ; Add it to the pool
                moveq.l #NT_MEMORY,d0           ; Get memory type...
                move.b  d0,LN_TYPE(a3)          ; Set up the type
                move.b  d0,LN_PRI(a3)           ; (Cheat: Priority field ;^)
                lea     PoolName(pc),a0         ; Get string "Pool"
                move.l  a0,LN_NAME(a3)          ; Name the pool...
                move.l  PP_Flags(a2),d1         ; Get flags
                move.w  d1,MH_ATTRIBUTES(a3)    ; Set the attributes
                lea     MH_SIZE+4(a3),a0        ; Point at first free...
***
*** Now need to check to make sure address is double-long alligned
***
                moveq.l #7,d1                   ; Get d1 to be 7...
                not.l   d1                      ; Invert... (bottom 3 bits 0)
                move.l  a0,d0                   ; Get address into register...
                and.l   d0,d1                   ; Mask off the bottom 3
                move.l  d1,a0                   ; Address now double-long
***
                move.l  PP_Size(a2),d0          ; Chunk byte size
                move.l  a0,MH_FIRST(a3)         ; Point at the first free
                move.l  a0,MH_LOWER(a3)         ; Point at the lower bound
                move.l  d0,MH_FREE(a3)          ; Set the FREE space
                clr.l   MC_NEXT(a0)             ; Clear next pointer
                move.l  d0,MC_BYTES(a0)         ; Set the chunk size
                add.l   d0,a0                   ; Add in the size
                move.l  a0,MH_UPPER(a3)         ; Point at end of puddle+1
                bra     ap_LookAgain            ; Look again (new puddle ;^)

******* exec.library/FreePooled ***********************************************
*
*    NAME
*       FreePooled -- Free pooled memory  (V39)
*
*    SYNOPSIS
*       FreePooled(poolHeader,memory,memSize)
*                  a0         a1     d0
*
*       void FreePooled(void *,void *,ULONG);
*
*    FUNCTION
*       Deallocates memory allocated by AllocPooled().  The size of the
*       allocation *MUST* match the size given to AllocPooled().
*       The reason the pool functions do not track individual allocation
*       sizes is because many of the uses of pools have small allocation
*       sizes and the tracking of the size would be a large overhead.
*
*       Only memory allocated by AllocPooled() may be freed with this
*       function!
*
*       Doing a DeletePool() on the pool will free all of the puddles
*       and thus all of the allocations done with AllocPooled() in that
*       pool.  (No need to FreePooled() each allocation)
*
*    INPUTS
*       memory - pointer to memory allocated by AllocPooled.
*       poolHeader - a specific private pool header.
*
*    NOTES
*       To track sizes yourself, the following code can be used:
*
*       ;
*       ; Function to do AllocVecPooled(Pool,memSize)
*       ;
*       AllocVecPooled: addq.l  #4,d0           ; Get space for tracking
*                       move.l  d0,-(sp)        ; Save the size
*                       jsr     _LVOAllocPooled(a6)     ; Call pool...
*                       move.l  (sp)+,d1        ; Get size back...
*                       tst.l   d0              ; Check for error
*                       beq.s   avp_fail        ; If NULL, failed!
*                       move.l  d0,a0           ; Get pointer...
*                       move.l  d1,(a0)+        ; Store size
*                       move.l  a0,d0           ; Get result
*       avp_fail:       rts                     ; return
*
*       ;
*       ; Function to do FreeVecPooled(pool,memory)
*       ;
*       FreeVecPooled:  move.l  -(a1),d0        ; Get size / ajust pointer
*                       jmp     _LVOFreePooled(a6)
*
*    SEE ALSO
*       AllocPooled(), CreatePool(), DeletePool()
*
*******************************************************************************

_EFreePooled:
		IFEQ	USE_POOLS_CODE
		SYS	FreeMem
		rts
		ENDC

* Lock the pool
                movem.l d0/d1/a0/a1,-(sp)
                lea.l   PoolLock(a6),a0
                SYS     ObtainSemaphore
                movem.l (sp)+,d0/d1/a0/a1

                move.l  a0,d1                   ; Check if NULL pool
                beq     fp_NoMem                ; If NULL, we exit...
                move.l  a1,d1                   ; Check if NULL memory
                beq     fp_NoMem                ; If NULL, we exit...

		IFD	TRACK_POOLS
		sub.w	#PA_SIZEOF,a1		; Adjust pointer
		add.l	#PA_SIZEOF,d0		; Adjust size
		movem.l	a0/a1/d0/d1,-(sp)	; Preserve registers
		SYS	Remove			; Remove from tracking list
		movem.l	(sp)+,a0/a1/d0/d1	; Restore registers
		ENDC

                cmp.l   PP_Thresh(a0),d0        ; Now check threshold
                bcc.s   fp_largeFree            ; If large one, do that...
                move.l  a3,-(sp)                ; Save some registers...
                move.l  (a0),d1                 ; Get ready for loop
fp_Looking:     move.l  d1,a3                   ; Get node
                move.l  (a3),d1                 ; Get next node
                beq     fp_Error                ; If not found we be in trouble
                tst.l   LP_Flag(a3)             ; Is it a large puddle
                beq.s   fp_Error                ; (They are at tail)
                cmp.l   MH_LOWER(a3),a1         ; Are we in this one
                bcs.s   fp_Looking              ; If not...
                cmp.l   MH_UPPER(a3),a1         ; ...just keep
                bcc.s   fp_Looking              ; ...on looking
                move.l  a3,a0                   ; Get memory header
                SYS     Deallocate              ; Free this memory...
*******
* It may be usefull to bubble up pools that seem to be
* getting used more...
*******
*
* Now, bubble the node up one on the list (a performance tuning issue)
* We buble the node in a3
* We trash a0, a1, and d1
*
bubble:         move.l  LN_PRED(a3),a1          ; Get previous node
                move.l  LN_PRED(a1),d1          ; Get node before that
                beq.s   nobubble                ; If NULL, no bubble...
*
* This code here moves the node in a3 above the node in a1
* The node a1 is the node just above the node a3 before this.
*
                move.l  d1,a0                   ; Get node-2
                move.l  a3,LN_SUCC(a0)          ; Point down...
                move.l  a0,LN_PRED(a3)          ; Point up...
                move.l  a3,LN_PRED(a1)          ; Swap it...
                move.l  LN_SUCC(a3),a0          ; Get next node
                move.l  a0,LN_SUCC(a1)          ; Point down
                move.l  a1,LN_PRED(a0)          ; Point up...
                move.l  a1,LN_SUCC(a3)          ; Into both nodes
*
* Ok, a3 is now one higher
*
nobubble:
*
                move.l  a3,a1                   ; Get node into a1
                move.l  (sp)+,a3                ; Restore a3
                move.l  MH_FREE(a1),d0          ; Get size...
                add.l   MH_LOWER(a1),d0         ; Add in lower bound
                sub.l   MH_UPPER(a1),d0         ; Subtract upper bound
                beq.s   fp_FreeNode             ; If NULL, it is empty
fp_NoMem:       bra     fp_Out
fp_largeFree:   subq.l  #4,a1           ; The special flag      :speed reasons
                subq.l  #8,a1           ; The "node" structure
*
* So free the node in a1 (after removing it first!
*
fp_FreeNode:    move.l  a1,d0           ; Save the value of a1
                REMOVE                  ; Remove (a1) from list (a0 trashed)
                move.l  d0,a1           ; Get pointer back
                SYS     FreeVec         ; Free it...
                bra     fp_Out
***************
fp_Error:       move.l  (sp)+,a3        ; Restore a3

fp_Out:

                movem.l d0/d1/a0/a1,-(sp)
                lea.l   PoolLock(a6),a0
                SYS     ReleaseSemaphore
                movem.l (sp)+,d0/d1/a0/a1

                rts


NewList         NEWLIST a0
                rts

                dc.b    0               ; Make the name ODD
PoolName:       dc.b    'Pool',0        ; Now word aligned again...


                end

