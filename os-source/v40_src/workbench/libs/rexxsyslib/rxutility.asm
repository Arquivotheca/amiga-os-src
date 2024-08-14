* == cvs2d.asm =========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =======================     CreateArgstring     ======================
* Allocates a RexxArg structure and copies a string into it.
* Registers:      A0 -- string pointer or NULL
*                 D0 -- string length (65535 maximum)
* Return:         D0 -- allocated structure or 0
*                 A0 -- same
CreateArgstring
         movem.l  d2/d3/a2,-(sp)
         movea.l  a0,a2                ; save string

         ; Allocate the structure ... use string length to compute size.

         moveq    #0,d2
         move.w   d0,d2                ; string length
         moveq    #NXADDLEN,d0         ; structure offset
         add.l    d2,d0                ; total structure length
         move.l   d0,d3                ; save it
         bsr.s    GetMem               ; D0=A0=block
         beq.s    2$                   ; failure ...

         exg      d0,a2                ; string=>D0, structure=>A2
         move.l   d3,(a0)+             ; (ra_Size) allocated length
         move.w   d2,(a0)+             ; (ra_Length) string length
         move.w   #(NSF_STRING!NSF_NOTNUM!NSF_EXT)<<8,(a0)+

         ; Copy the string into the allocated structure.

         tst.l    d0                   ; string to copy?
         beq.s    1$                   ; no
         movea.l  d0,a1                ; source string
         move.l   d2,d0                ; length
         CALLSYS  StrcpyN              ; D0=hash
         move.b   d0,ra_Hash(a2)       ; store the hash byte

1$:      move.l   a2,d0                ; structure
         addq.l   #ra_Buff,d0          ; ... offset to string

2$:      movea.l  d0,a0
         movem.l  (sp)+,d2/d3/a2
         rts

* ===========================     GetMem     ===========================
* Allocates a block of public memory (cleared) from the system pool.
* Registers:      D0 -- length
* Return:         D0 -- allocated block
*                 A0 -- same
GetMem
         move.l   a6,-(sp)
         movea.l  rl_SysBase(a6),a6
         move.l   #MEMQUICK!MEMCLEAR,d1; cleared public
         CALLSYS  AllocMem             ; D0=memory block
         tst.l    d0                   ; set CCR
         movea.l  d0,a0
         movea.l  (sp)+,a6
         rts

* =========================     AddRsrcNode     ========================
* Creates and links a new resource node at the tail of the specified list.
* Registers:   A0 -- list header
*              A1 -- name string (null-terminated)
*              D0 -- length of structure
* Return:      D0 -- allocated node (or 0)
*              A0 -- same
AddRsrcNode
         movem.l  d2/a2/a3,-(sp)
         movea.l  a0,a2                ; save header
         move.l   d0,d2                ; save length

         ; Create an argument string for the name ...

         movea.l  a1,a0                ; name string
         move.l   a0,d0                ; name supplied?
         beq.s    1$                   ; no ...
         CALLSYS  Strlen               ; D0=D1=length A0=string
         CALLSYS  CreateArgstring      ; D0=A0=string
         beq.s    3$                   ; failure ...

1$:      movea.l  a0,a3                ; save name
         move.l   d2,d0                ; length
         bsr.s    GetMem               ; D0=A0=block
         exg      a0,a3                ; name=>A0, node=>A3
         beq.s    2$                   ; failure
         move.l   a0,RRNAME(a3)        ; install name
         move.l   d2,RRSIZE(a3)        ; install size

         ; Link the new node at the end of the list.  Since the list may be
         ; shared, we Forbid() other tasks ...

         bsr.s    Forbid
         movea.l  a2,a0                ; list header
         movea.l  a3,a1                ; new node
         bsr      AddTail              ; link it
         bsr.s    Permit
         move.l   a3,d0                ; return the node
         bra.s    3$

         ; Node allocation failure -- release the argstring.

2$:      CALLSYS  DeleteArgstring      ; release it
         moveq    #0,d0                ; clear return

3$:      movea.l  d0,a0
         movem.l  (sp)+,d2/a2/a3
         rts

* =======================     LengthArgstring     ======================
* Returns the length of an argstring.  A0 is preserved.
* Registers:      A0 -- argstring pointer
* Return:         D0 -- length
*                 A0 -- argstring pointer
LengthArgstring
         moveq    #0,d0
         move.w   (ra_Length-ra_Buff)(a0),d0
         rts

* =======================     DeleteArgstring     ======================
* Releases a RexxArg structure.  The pointer is to the string buffer of
* the structure.
* Registers:      A0 -- argstring (may be NULL)
DeleteArgstring
         move.l   a0,d0                ; string given?
         beq.s    GM002                ; no

         subq.w   #ra_Buff,a0          ; start of structure
         move.l   (a0),d0              ; (ra_Size) total length

* ==========================     GiveMem     ===========================
* Returns allocated memory to the system pool.
* Registers:   A0 -- memory block
*              D0 -- length
GiveMem:
         move.l   a6,-(sp)
         movea.l  rl_SysBase(a6),a6
         movea.l  a0,a1                ; memory block
         CALLSYS  FreeMem

GM001    movea.l  (sp)+,a6

GM002    rts

* ===========================     Forbid     ===========================
* Disables task switching.  All registers are preserved.
Forbid
         move.l   a6,-(sp)
         movea.l  rl_SysBase(a6),a6
         addq.b   #1,TDNestCnt(a6)     ; bump count
         bra.s    GM001

* ===========================     Permit     ===========================
* Reenables task-switching.  All registers are preserved.
Permit
         movem.l  d0/d1/a0/a1/a6,-(sp)
         movea.l  rl_SysBase(a6),a6
         CALLSYS  Permit
         movem.l  (sp)+,d0/d1/a0/a1/a6
         rts

* =========================     RemRsrcNode     ========================
* Unlinks and releases a resource node.  The name string is also released.
* If a deletion function has been specified, it is called before the node
* is released.
* Registers:      A0 -- resource node
RemRsrcNode
         move.l   a2,-(sp)
         movea.l  a0,a2                ; save node

         ; Forbid() other tasks and unlink the node.

         bsr.s    Forbid               ; forbid switching
         movea.l  a2,a1
         bsr      Remove               ; unlink the node
         bsr.s    Permit               ; reenable switching

         ; Check whether a "cleanup" function was given.  If so, it is
         ; called with the node in A0.  Functions may be specified either
         ; as absolute addresses or as library (Base,Offset) pairs.
         ; The "cleanup" base is cleared to prevent recursive calls.

         move.l   rr_Base(a2),d0       ; auto-deletion function?
         beq.s    1$                   ; no

         move.l   a6,-(sp)             ; save REXX base
         clr.l    rr_Base(a2)          ; clear it
         movea.l  a2,a0                ; load resource node
         movea.l  d0,a6                ; base address
         move.w   rr_Func(a2),d1       ; load offset

         ; Call the "auto-delete" function

         jsr      0(a6,d1.w)           ; library offset
         movea.l  (sp)+,a6             ; restore REXX base

         ; Release the name string (OK if no string present ...)

1$:      movea.l  RRNAME(a2),a0        ; name string
         CALLSYS  DeleteArgstring      ; release it

         ; Release the node, using the stored size field.

         movea.l  a2,a0                ; node to release
         move.l   RRSIZE(a0),d0        ; total length
         movea.l  (sp)+,a2             ; restore register
         bra.s    GiveMem

* ========================     RemRsrcList     =========================
* Deletes an entire resource list.
* Registers:   A0 -- list header
* Return:      A0 -- list header
RemRsrcList
         move.l   a0,-(sp)             ; push header
         bra.s    2$                   ; jump in

1$:      CALLSYS  RemRsrcNode          ; release node

2$:      movea.l  (sp),a0              ; list header
         movea.l  (a0),a0              ; first node
         tst.l    (a0)                 ; a successor?
         bne.s    1$                   ; yes

         movea.l  (sp)+,a0             ; pop header
         rts

* ========================     FindRsrcNode     ========================
* Locates the (first) resource node with the given name and type.
* Registers:   A0 -- list header (or node)
*              A1 -- name string (NULL-terminated)
*              D0 -- node type or 0
* Return:      D0 -- located node or 0
*              A0 -- same
FindRsrcNode
         movem.l  d2/a2/a6,-(sp)
         movea.l  a0,a2
         move.b   d0,d2                ; node type
         movea.l  rl_SysBase(a6),a6    ; EXEC base

1$:      movea.l  a2,a0                ; list header
         CALLSYS  FindName             ; D0=node
         movea.l  d0,a0
         tst.l    d0                   ; node found?
         beq.s    2$                   ; no

         ; Check if the node type was correct ...

         movea.l  RRNAME(a0),a1        ; node name
         tst.b    d2                   ; a particular type?
         beq.s    2$                   ; no
         cmp.b    RRTYPE(a0),d2        ; type correct?
         bne.s    1$                   ; no ... keep looking

2$:      tst.l    d0                   ; set CCR
         movem.l  (sp)+,d2/a2/a6
         rts

* ========================     AddClipNode     =========================
* Allocates and links a Clip List node into the specified list.
* Registers:   A0 -- list header
*              A1 -- node name
*              D0 -- value length
*              D1 -- value pointer
* Return:      D0 -- node or 0
*              A0 -- same
AddClipNode
         move.l   a2,-(sp)

         ; Allocate the resource node

         movem.l  d0/d1,-(sp)          ; push length/pointer
         moveq    #RRSIZEOF,d0         ; node size
         CALLSYS  AddRsrcNode          ; D0=A0=node
         movea.l  d0,a2                ; resource node
         movem.l  (sp)+,d0/a0          ; pop length/pointer
         beq.s    1$                   ; failure

         ; Fill in the node fields

         move.b   #RRT_CLIP,RRTYPE(a2)
         move.w   #_LVORemClipNode,rr_Func(a2)
         move.l   a6,rr_Base(a2)

         ; Allocate the value string

         CALLSYS  CreateArgstring      ; D0=A0=argstring

         ; Install the value string in the node ...

         move.l   d0,CLVALUE(a2)       ; a value?
         bne.s    1$                   ; yes

         ; Value allocation failure ... release the node.

         movea.l  a2,a0                ; resource node
         CALLSYS  RemRsrcNode          ; release it
         suba.l   a2,a2                ; clear return

1$:      move.l   a2,d0                ; set CCR
         movea.l  d0,a0
         movea.l  (sp)+,a2
         rts

* =========================     RemClipNode     ========================
* The "auto-delete" function for Clip List nodes.
* Registers:      A0 -- node
RemClipNode
         tst.l    rr_Base(a0)          ; "auto-delete" call?
         beq.s    1$                   ; no
         jmp      _LVORemRsrcNode(a6)

         ; Auto-delete ... release value argstring.

1$:      lea      CLVALUE(a0),a1       ; slot address
         movea.l  (a1),a0              ; value argstring
         clr.l    (a1)                 ; clear slot
         jmp      _LVODeleteArgstring(a6)

* ==========================     ListNames     =========================
* Scans an EXEC-style list with task switching (or interrupts) disabled
* and builds a string of the node names, separated by a space or by the
* specified character.  If the name is NULL, a '?' is used instead.
* Registers:   A0 -- list header
*              D0 -- separator character
*              D1 -- flag: 0=Forbid -1=Disable
* Return:      D0 -- argstring or 0
*              A0 -- same
ListNames
         movem.l  d2-d5/a2-a6,-(sp)
         movea.l  a0,a2                ; list header
         movea.l  rl_SysBase(a6),a5    ; EXEC base

         moveq    #250>>1,d2           ; block size/2
         move.b   d0,d4                ; separator character
         move.w   #_LVOForbid,d5       ; default offset
         tst.w    d1                   ; disable?
         beq.s    LNForbid             ; no
         move.w   #_LVODisable,d5      ; ... use disable entry

         ; Forbid task switching or disable interrupts, in case we're
         ; scanning a shared system list.

LNForbid exg      a5,a6
         jsr      0(a6,d5.w)           ; Forbid (or Disable)
         exg      a5,a6
         bra.s    2$                   ; jump in

         ; Buffer filled up ... increase size and try again.

1$:      movea.l  a3,a0                ; current argstring
         CALLSYS  DeleteArgstring      ; release it ...

         ; Allocate a block to hold the names.  If the block overflows,
         ; its size is doubled and the process repeated.

2$:      suba.l   a3,a3                ; clear return
         add.w    d2,d2                ; double the block
         bvs.s    LNPermit             ; ... overflow

         move.l   d2,d0                ; block size
         suba.l   a0,a0                ; no string
         CALLSYS  CreateArgstring      ; D0=A0=string
         beq.s    LNPermit             ; ... failure

         ; Initialize pointers and get the first list node.

         movea.l  a0,a3                ; save argstring
         movea.l  a0,a4                ; scan pointer
         moveq    #0,d3                ; clear count
         move.l   (a2),d0              ; first node
         bra.s    5$                   ; jump in

         ; Scan the list and copy the name strings ...

3$:      movea.l  LN_NAME(a0),a0       ; node name
         move.l   a0,d1                ; a name?
         bne.s    4$                   ; yes
         lea      QMark(pc),a0         ; no -- use '?'

4$:      cmp.l    d2,d3                ; buffer full?
         bge.s    1$                   ; yes ... loop back
         addq.l   #1,d3
         move.b   (a0)+,(a4)+          ; install character
         bne.s    4$                   ; loop back
         move.b   d4,-1(a4)            ; install separator

         ; Get the next node from the list

5$:      movea.l  d0,a0                ; current node
         move.l   (a0),d0              ; a successor?
         bne.s    3$                   ; yes ... loop back

         ; List of names complete ... back up over last separator.

         tst.l    d3                   ; any characters?
         beq.s    6$                   ; no
         subq.l   #1,d3                ; back up one
         clr.b    -(a4)                ; install a null

6$:      move.w   d3,(ra_Length-ra_Buff)(a3)

         ; Reenable task switching (or enable interrupts) and exit ...

LNPermit movea.l  a5,a6                ; EXEC=>A6
         jsr      -6(a6,d5.w)          ; Permit (or Enable)

         move.l   a3,d0                ; return string
         movea.l  d0,a0
         movem.l  (sp)+,d2-d5/a2-a6
         rts
