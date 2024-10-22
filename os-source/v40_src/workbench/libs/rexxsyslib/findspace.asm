* == cvs2d.asm =========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     GetSpace     ===========================
* Memory allocator for use outside the interpreter.  Disables the error
* trap and restores it after the allocation.
* Registers:   A0 -- RexxTask (global) pointer
*              D0 -- request size
* Return:      D0 -- allocated block
*              A0 -- same
GetSpace
         move.l   a3,-(sp)

         lea      rt_ErrTrap(a0),a3    ; trap slot
         move.l   (a3),-(sp)           ; error trap address
         clr.l    (a3)                 ; clear it
         bsr.s    FindSpace            ; D0=A0=block
         move.l   (sp)+,(a3)           ; restore trap
         tst.l    d0                   ; set CCR
         movea.l  (sp)+,a3
         rts

* =========================     FindSpace     ==========================
* Memory space allocator for the REXX interpreter.  Allocates memory from
* the system and parcels it out in quantized blocks.  Allocated blocks are
* linked into the global memory list, and excess amounts are passed to the
* FreeList manager.
* Registers:   A0 -- storage environment
*              D0 -- request size
* Return:      D0 -- address of block
*              A0 -- same as D0
FindSpace
         movea.l  ev_GlobalPtr(a0),a0  ; base environment

         ; Quantize the request ...

         moveq    #MEMQUANT-1,d1       ; rounding amount
         add.l    d1,d0                ; add it
         andi.b   #~(MEMQUANT-1),d0    ; quantized request

         moveq    #MEMQUANT*3,d1       ; three quanta
         cmp.l    d1,d0                ; "hot" request?
         bgt.s    FindSlowly           ; ... too big

         ; A "hot list" request ... compute the list address.

         move.l   d0,d1                ; quantized request
         lea      (gn_HotList1-4)(a0),a1
         lsr.w    #2,d0                ; selection index
         adda.w   d0,a1                ; selected list
         bra.s    FindQuickly

* =======================     FindQuickThree     =======================
* Fastest possible allocators for internal use ...
FindQuickThree
         movea.l  ev_GlobalPtr(a0),a0  ; base environment
         lea      gn_HotList3(a0),a1   ; third hot list
         moveq    #MEMQUANT*3,d1       ; three quanta
         bra.s    FindQuickly

FindQuickTwo
         movea.l  ev_GlobalPtr(a0),a0  ; base environment
         lea      gn_HotList2(a0),a1   ; second hot list
         moveq    #MEMQUANT*2,d1       ; two quanta
         bra.s    FindQuickly

FindQuickOne
         movea.l  ev_GlobalPtr(a0),a0  ; base environment
         lea      gn_HotList1(a0),a1   ; first hot list
         moveq    #MEMQUANT,d1         ; one quantum

         ; Attempt the "hot" list allocation

FindQuickly
         move.l   (a1),d0              ; any nodes?
         beq.s    1$                   ; no ...

         ; Success!  Update the free total and return ...

         sub.l    d1,gn_TotFree(a0)    ; decrement free total
         movea.l  d0,a0                ; "hot" node
         move.l   (a0),(a1)            ; update "hot" list
         move.l   a0,d0                ; set CCR
         rts

         ; Quick allocation failed ... restore size and continue search.

1$:      move.l   d1,d0                ; quantized size

FindSlowly
         movem.l  d2/a2/a3,-(sp)       ; delayed save
         movea.l  a0,a3                ; global data structure

         ; Search the freelist for a suitable node.  Abutting nodes are
         ; combined as the search progresses.

         move.l   FREELIST(a3),d1      ; first node

1$:      movea.l  d1,a2                ; current node
         move.l   (a2),d1              ; successor node?
         beq.s    6$                   ; no ... allocate more memory
         move.l   sn_Size(a2),d2       ; size of current node

2$:      cmp.l    d0,d2                ; node big enough?
         beq.s    3$                   ; yes -- just enough
         bhi.s    4$                   ; yes -- space left over

         movea.l  a2,a1                ; current node
         adda.l   d2,a1                ; high-memory extent
         cmpa.l   d1,a1                ; current node abuts successor?
         bne.s    1$                   ; no ...

         ; Abutting nodes ... unlink successor and combine sizes.

         movea.l  (a1),a0              ; successor's successor
         move.l   a2,sn_Pred(a0)
         move.l   a0,(a2)              ; new successor
         add.l    sn_Size(a1),d2       ; extended size
         move.l   d2,sn_Size(a2)       ; install it
         move.l   a0,d1                ; update successor
         bra.s    2$                   ; check node (extended) again ...

         ; Node was just big enough -- unlink it

3$:      movea.l  d1,a1                ; successor node
         movea.l  sn_Pred(a2),a0       ; predecessor node
         move.l   a0,sn_Pred(a1)
         move.l   a1,(a0)
         bra.s    5$

         ; Node larger than needed.  Adjust size and leave it in the list.
         ; All memory blocks are quantized, so the remaining size is at
         ; least equal to the memory quantum.

4$:      sub.l    d0,d2                ; remaining size
         move.l   d2,sn_Size(a2)       ; save it ...
         adda.l   d2,a2                ; advance past remainder

5$:      sub.l    d0,gn_TotFree(a3)    ; decrement free total
         bra.s    8$

         ; No free block available ... attempt to allocate more.  Memory is
         ; allocated in powers of two times the global chunk size and must
         ; be a multiple of the quantum.

6$:      movea.l  d0,a2                ; save quantized request
         moveq    #RRSIZEOF,d0         ; resource node size
         add.l    a2,d0                ; total request size

         move.l   rl_Chunk(a6),d2      ; chunk size (possibly dynamic)
         bra.s    62$                  ; jump in

61$:     add.l    d2,d2                ; double chunk
62$:     cmp.l    d0,d2                ; chunk >= request?
         bcs.s    61$                  ; no ... loop back

         ; Add a resource node to the allocation list.

7$:      lea      MEMLIST(a3),a0       ; list header
         suba.l   a1,a1                ; no name
         move.l   d2,d0                ; total size
         CALLSYS  AddRsrcNode          ; D0=A0=node
         beq.s    10$                  ; ... fatal error

         add.l    d2,gn_TotAlloc(a3)   ; update total allocated
         sub.l    a2,d2                ; amount leftover
         movea.l  a0,a2                ; allocated node
         adda.l   d2,a2                ; start of return block

         ; Check whether any remainder to be released to the freelist ...

         lea      RRSIZEOF(a0),a1      ; start of remainder
         move.l   a2,d0                ; start of return
         sub.l    a1,d0                ; anything left over?
         bls.s    8$                   ; no ...

         ; Add the remainder block to the freelist.

         movea.l  a3,a0                ; base environment
         bsr.s    FreeSpace            ; release it

8$:      move.l   a2,d0                ; allocated block

9$:      movea.l  d0,a0                ; return value
         movem.l  (sp)+,d2/a2/a3
         rts

         ; Allocation failure ... check if the error trap is set.

10$:     lea      rt_ErrTrap(a3),a1    ; trap address
         move.l   (a1),d0              ; trap address set?
         beq.s    9$                   ; no ... return normally

         ; Load the trap address/stack pointer and exit ...

         movem.l  (a1),a0/a7           ; load registers
         clr.l    (a1)                 ; clear trap
         jmp      (a0)                 ; jump to trap

* ======================     FreeQuickThree     ========================
* Fastest possible deallocators for internal use ...
FreeQuickThree
         movea.l  ev_GlobalPtr(a0),a0  ; base environment
         moveq    #MEMQUANT*3,d0       ; three quanta
         IFD      MEMCHECK
         bsr      FreeVerify
         ENDC
         add.l    d0,gn_TotFree(a0)    ; update free total
         lea      gn_HotList3(a0),a0   ; third hot list
         bra.s    FreeQuickly

FreeQuickTwo
         movea.l  ev_GlobalPtr(a0),a0  ; base environment
         moveq    #MEMQUANT*2,d0       ; two quanta
         IFD      MEMCHECK
         bsr      FreeVerify
         ENDC
         add.l    d0,gn_TotFree(a0)    ; update free total
         lea      gn_HotList2(a0),a0   ; second hot list
         bra.s    FreeQuickly

FreeQuickOne
         movea.l  ev_GlobalPtr(a0),a0  ; base environment
         moveq    #MEMQUANT,d0         ; one quantum
         IFD      MEMCHECK
         bsr      FreeVerify
         ENDC
         add.l    d0,gn_TotFree(a0)    ; update free total
         lea      gn_HotList1(a0),a0   ; first hot list

FreeQuickly
         move.l   (a0),(a1)            ; add list to node
         move.l   a1,(a0)              ; install node (set CCR)
         rts

* =========================     FreeSpace     ==========================
* Adds a memory block to the free list.  If the returning space follows
* another block, the two are coalesced.
* Registers:   A0 -- storage environment
*              A1 -- memory block
*              D0 -- size of block
* Return:      D0 -- boolean success flag
FreeSpace
         movea.l  ev_GlobalPtr(a0),a0  ; base environment
         IFD      MEMCHECK
         bsr      FreeVerify
         ENDC

         moveq    #MEMQUANT-1,d1       ; rounding offset       4
         add.l    d1,d0                ; ... add it            8
         andi.b   #~(MEMQUANT-1),d0    ; quantized size        8
         add.l    d0,gn_TotFree(a0)    ; update free total

         moveq    #MEMQUANT*3,d1       ; three quanta
         cmp.l    d1,d0                ; size <= three quanta?
         bgt.s    FreeSlowly           ; no ...

         ; A "hot list" return ... compute the slot index.

         lea      (gn_HotList1-4)(a0),a0
         lsr.w    #2,d0                ; selection index
         adda.w   d0,a0                ; selected list
         bra.s    FreeQuickly

         ; The freelist is ordered by memory address ... start search from
         ; the head (low-memory).

FreeSlowly
         move.l   a2,-(sp)             ; delayed save

         lea      FREELIST(a0),a2      ; list header
         movea.l  (a2),a0              ; first node

         ; Scan the freelist for the insertion point.  If the new node abuts
         ; the high-memory end of an existing node, they are joined.

2$:      tst.l    (a0)                 ; end of list?
         beq.s    3$                   ; yes -- insert new node
         cmpa.l   a0,a1                ; return node < current node?
         bls.s    3$                   ; yes -- insert it

         movea.l  a0,a2                ; current node
         adda.l   sn_Size(a0),a0       ; ... high-memory extent
         cmpa.l   a0,a1                ; return node abuts current node?
         movea.l  (a2),a0              ; (load successor node)
         bne.s    2$                   ; no ... loop back

         ; Nodes abut ... extend the current node.

         add.l    d0,sn_Size(a2)       ; update size
         bra.s    4$

         ; Link in the new node ... A1=node A2=previous

3$:      move.l   d0,sn_Size(a1)       ; size of new node
         movea.l  a2,a0                ; insertion point
         move.l   a1,d0                ; first and last node
         bsr      Relink               ; link the node

4$:      movea.l  (sp)+,a2

5$:      moveq    #-1,d0               ; boolean TRUE
         rts

         IFD      MEMCHECK
         XDEF     FreeVerify

         ; Verifies that the returning block is a valid memory blocks.

FreeVerify
         movem.l  d0/a0/a1/a3,-(sp)
         movea.l  a0,a3                ; global pointer

         lea      ZeroFree(pc),a0      ; load message
         tst.l    d0                   ; size > 0?
         ble.s    6$                   ; no

         movea.l  MEMLIST(a3),a0       ; first node
1$:      cmpa.l   a0,a1                ; block >= start?
         bcs.s    2$                   ; no
         move.l   a0,d1
         add.l    rr_Size(a0),d1       ; extent
         sub.l    a1,d1                ; block <= end?
         bcs.s    2$                   ; no
         cmp.l    d1,d0                ; length <= remainder?
         bls.s    3$                   ; yes

2$:      movea.l  (a0),a0              ; next node
         tst.l    (a0)                 ; successor?
         bne.s    1$                   ; yes

         lea      BadFree(pc),a0       ; bad free
         bra.s    6$

         ; Now make sure the block isn't already free ...

3$:      lea      gn_HotList1(a3),a0   ; first hot list
         bra.s    32$                  ; jump in
31$:     cmp.l    a1,d1                ; already free?
         beq.s    5$                   ; yes
         movea.l  d1,a0
32$:     move.l   (a0),d1              ; a node?
         bne.s    31$                  ; yes

         lea      gn_HotList1+4(a3),a0
         bra.s    34$                  ; jump in
33$:     cmp.l    a1,d1                ; already free?
         beq.s    5$                   ; yes
         movea.l  d1,a0
34$:     move.l   (a0),d1              ; a node?
         bne.s    33$                  ; yes

         lea      gn_HotList1+8(a3),a0
         bra.s    36$                  ; jump in
35$:     cmp.l    a1,d1                ; already free?
         beq.s    5$                   ; yes
         movea.l  d1,a0
36$:     move.l   (a0),d1              ; a node?
         bne.s    35$                  ; yes

         lea      FREELIST(a3),a0      ; first free
4$:      movea.l  (a0),a0              ; next node
         tst.l    (a0)                 ; a successor?
         beq.s    7$                   ; no ... all done

         move.l   a1,d1
         add.l    d0,d1                ; high-memory extent
         cmp.l    a0,d1                ; high end < free block?
         bls.s    4$                   ; yes

         move.l   a0,d1                ; free block
         add.l    sn_Size(a0),d1       ; high extent
         cmp.l    a1,d1                ; start >= high end?
         bls.s    4$                   ; yes ... loop back

5$:      lea      DupFree(pc),a0       ; error ... load message

         ; Report the error ... message in A0

6$:      move.l   d0,-(sp)             ; size
         move.l   a1,-(sp)             ; address
         movea.l  sp,a1                ; data stream
         move.l   gn_TBuff(a3),d0      ; buffer area
         bsr      FormatString         ; D0=length A0=buffer
         addq.w   #8,sp                ; restore stack

         movem.l  d0/a0,-(sp)          ; push length/buffer
         movea.l  a3,a0                ; global pointer
         move.l   rl_STDOUT(a6),a1     ; stream name
         bsr      FindLogical          ; D0=A0=logical
         movem.l  (sp)+,d0/a1          ; pop length/buffer
         beq.s    7$                   ; ... no stream
         CALLSYS  WriteF               ; display message

7$:      movem.l  (sp)+,d0/a0/a1/a3
         rts

         ; Error messages

ZeroFree dc.b     'Zero-length free at %8lx size %ld',10,0
BadFree  dc.b     'Bad free at %8lx size %ld',10,0
DupFree  dc.b     'Duplicate free at %8lx size %ld',10,0
         CNOP     0,2
         ENDC
