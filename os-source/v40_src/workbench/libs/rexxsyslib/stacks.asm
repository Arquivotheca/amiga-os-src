* == stacks.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     PushStack     ==========================
* Adds a node to the top (head) of a 'stack' list.
* Registers:   A0 -- stack header (list)
*              A1 -- node to be added
PushStack
         addq.w   #1,skNum(a0)

* ==========================     AddHead     ===========================
* Adds a node to the head of a list.
* Registers:   A0 -- list header
*              A1 -- node
AddHead
         ADDHEAD
         rts

* =========================     PushTail     ===========================
* Adds a node to the bottom (tail) of a 'stack' list.
* Registers:   A0 -- stack header (list)
*              A1 -- node to be added
PushTail
         addq.w   #1,skNum(a0)

* ===========================     AddTail     ==========================
* Adds a node to the tail of a list.
* Registers:   A0 -- list header
*              A1 -- new node
AddTail
         addq.w   #LH_TAIL,a0
         move.l   a0,(a1)
         addq.w   #LN_PRED,a0
         move.l   (a0),d0
         move.l   a1,(a0)
         MOVE.L   D0,LN_PRED(A1)
         movea.l  d0,a0
         MOVE.L   A1,(A0)
         rts

* ==========================     RemHead     ===========================
* Removes the head node from a list.  A0 is preserved.
* Registers:   A0 -- list header
* Return:      D0 -- removed node or 0 (sets CCR)
*              A0 -- list header
RemHead
         movea.l  (a0),a1           ; first node
         move.l   (a1),d0           ; a successor?
         beq.s    1$                ; no

         exg      d0,a1             ; node=>D0, successor=>A1
         move.l   a0,sn_Pred(a1)    ; predecessor link
         move.l   a1,(a0)           ; successor link

1$:      rts

* ==========================     RemTail     ===========================
* Removes the tail node from a list.
* Registers:   A0 -- list header
* Return:      D0 -- removed node or 0
*              A0 -- list header
RemTail
         movea.l  LH_TAILPRED(a0),a1; tail predecessor
         move.l   sn_Pred(a1),d0    ; a predecessor?
         beq.s    1$                ; no

         move.l   (a1),d1           ; tail pointer
         exg      d0,a1             ; node=>D0, predecessor=>A1
         move.l   a1,LH_TAILPRED(a0)
         move.l   d1,(a1)           ; successor (tail)

1$:      rts

* ===========================     Remove     ===========================
* Removes the tail node from a list.
* Registers:   A1 -- node
Remove
         REMOVE
         rts

* ==========================     PopStack     ==========================
* Removes the top (head) node from a "stack" list.
* Registers:   A0 -- stack header (list)
* Return:      D0 -- removed node or NULL
PopStack
         bsr.s    RemHead              ; D0=node A0=header
         beq.s    PS002                ; ... not found

PS001    subq.w   #1,skNum(a0)         ; decrement stack count
         tst.l    d0                   ; set CCR

PS002    rts

* ==========================     PopTail     ===========================
* Removes the bottom (tail) node from a "stack" list.
* Registers: A0 -- stack header (list)
* Return:    D0 -- removed node or NULL
PopTail
         bsr.s    RemTail              ; D0=node A0=header
         bne.s    PS001                ; ... node found
         rts

* ============================     Unlink     ==========================
* Unlinks a chain of nodes from a list.
* Registers:   A0 -- first node
*              A1 -- last node
* Return:      A1 -- last node
*              D0 -- first node
Unlink
         move.l   a0,d0                ; save front node
         move.l   sn_Pred(a0),d1       ; load front predecessor
         movea.l  (a1),a0              ; load back successor
         move.l   d1,sn_Pred(a0)       ; install back predecessor
         exg      d1,a0                ; successor=>D0, predecessor=>A0
         move.l   d1,(a0)              ; install front successor
         rts

* ===========================     Relink     ===========================
* Relinks a chain of nodes following the specified insertion node.
* Registers:   A0 -- insertion point
*              A1 -- last node
*              D0 -- first node
Relink
         move.l   (a0),(a1)            ; load successor
         move.l   d0,(a0)              ; install front successor
         exg      d0,a0                ; insertion=>D0, first=>A0
         move.l   d0,sn_Pred(a0)       ; install front predecessor
         movea.l  (a1),a0              ; load back successor
         move.l   a1,sn_Pred(a0)       ; install back predecessor
         rts

* ==========================     ClearMem     ==========================
* Clears a section of (long-word aligned) memory.  No memory is cleared
* if the count is 0.  Register A0 is preserved.
* Registers:      A0 -- starting address
*                 D0 -- byte count (must be a multiple of 4)
ClearMem
         add.l    a0,d0                ; upper limit
         movea.l  a0,a1                ; running index
         bra.s    2$                   ; jump right in

1$:      clr.l    (a1)+                ; clear
2$:      cmpa.l   d0,a1                ; limit reached?
         bcs.s    1$                   ; not yet ...
         rts

* =========================     SrcPosition     ========================
* Returns the source position corresponding to the given segemnt and offset.
* Registers A0 and A1 are preserved.
* Registers:      D0 -- segment
*                 D1 -- offset
* Return:         D0 -- position
SrcPosition
         IFEQ     SEGSHIFT-16          ; fast implementation
         swap     d0                   ; segment=>upper
         move.w   d1,d0                ; install offset
         ENDC

         IFNE     SEGSHIFT-16          ; must use slow implementation
         move.w   d1,-(sp)             ; push offset
         moveq    #SEGSHIFT,d1         ; load count
         lsl.l    d1,d0                ; make space
         or.w     (sp)+,d0             ; OR in the offset
         ENDC

         rts

         IFLT     SEGSHIFT-8
         FAIL     'SEGSHIFT too small!'
         ENDC
         IFGT     SEGSHIFT-16
         FAIL     'SEGSHIFT too large!'
         ENDC

* =========================     SrcSegment     =========================
* Splits the source position into a segment and offset.  A0/A1 are preserved.
* Registers:      D0 -- source position
* Return:         D0 -- segment
*                 D1 -- offset
SrcSegment
         IFEQ     SEGSHIFT-16          ; fast implementation
         moveq    #0,d1                ; clear offset
         move.w   d0,d1                ; load offset
         clr.w    d0                   ; clear lower
         swap     d0                   ; segment=>lower
         ENDC

         IFNE     SEGSHIFT-16          ; must use slow implementation
         move.w   d0,d1                ; offset to lower register
         move.b   #SEGSHIFT,d0         ; borrow lower byte
         lsr.l    d0,d0                ; shift it over!
         andi.l   #SEGMASK,d1          ; prepare the offset
         ENDC

         rts
