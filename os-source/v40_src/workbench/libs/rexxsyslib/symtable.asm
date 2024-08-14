* == symtable.asm ======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ========================     GetSymTable     =========================
* Allocates and clears a symbol table structure.
* Registers:   A0 -- storage environment
* Return:      D0 -- allocated table
*              A0 -- same
GetSymTable
         moveq    #st_SIZEOF,d0
         bsr      FindSpace            ; D0=A0=allocated space
         moveq    #st_SIZEOF,d0
         CALLSYS  ClearMem             ; (A0 preserved)
         move.l   a0,d0
         rts

* ==========================     FetchValue     ========================
* Retrieves the current value for a symbol.  The stem and compound strings
* are released.
* Registers:   A0 -- environment
*              A1 -- stem name
*              D0 -- compound name
*              D1 -- node pointer or 0
* Return:      D0 -- node
*              D1 -- literal flag
*              A1 -- value
FetchValue
         movem.l  d2/a2/a4,-(sp)
         movea.l  a0,a4
         movea.l  a1,a2                ; save stem
         move.l   d0,d2                ; save compound string

         ; Look up the value ...

         bsr.s    GetValue             ; D0=node D1=literal A1=value
         movem.l  d0/d1/a1,-(sp)       ; save return

         ; Release the stem and compound strings ...

         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; stem string
         bsr      FreeString           ; release it

         tst.l    d2                   ; compound string?
         beq.s    1$                   ; no
         movea.l  a4,a0                ; environment
         movea.l  d2,a1                ; compound string
         bsr      FreeString           ; release it ...

1$:      movem.l  (sp)+,d0/d1/a1       ; restore return
         movem.l  (sp)+,d2/a2/a4
         rts

* ==========================     GetValue     ==========================
* Retrieves the current value for a symbol from the symbol table.
* Registers:   A0 -- environment
*              A1 -- stem string
*              D0 -- compound string or 0
*              D1 -- symbol table node or 0
* Return:      D0 -- symbol table node
*              D1 -- boolean literal flag
*              A0 -- symbol table node
*              A1 -- value string
GetValue
         movem.l  d3-d5,-(sp)
         move.l   d0,d5                ; save compound string
         moveq    #-1,d3               ; assume a literal

         ; Determine the default value.

         move.l   d5,d4                ; compound lookup?
         bne.s    0$                   ; yes ...
         move.l   a1,d4                ; no -- use stem name

0$:      tst.l    d1                   ; node specified?
         beq.s    1$                   ; no -- proceed with lookup
         movea.l  d1,a0                ; load symbol table node
         bra.s    3$                   ; branch to get value

         ; First-level look-up:  get the root node in the symbol table.

1$:      move.b   ns_Hash(a1),d0       ; hash code for stem
         bsr      BTIndex              ; root index=>D0  (A0/A1 unchanged)
         movea.l  ev_ST(a0),a0         ; symbol table
         adda.w   d0,a0                ; index to root
         movea.l  (a0),a0              ; load root node
         bsr.s    LocateBTNode         ; D0=node  A0=last checked
         beq.s    4$                   ; not found ...

         ; First-level node located ... check whether to continue

         tst.l    d5                   ; compound look-up?
         beq.s    3$                   ; no ...

         ; Get first level value to update the default return.  Since a
         ; match occurred, the last node (A0) is the located node (D0).

         movea.l  btPst(a0),a1         ; table entry
         move.l   (a1)+,d1             ; (be_PValue) value assigned?
         beq.s    2$                   ; no ...
         moveq    #0,d3                ; yes -- not a literal
         move.l   d1,d4                ; new default value

2$:      movea.l  (a1),a0              ; (be_Next) second-level root
         movea.l  d5,a1                ; compound string
         bsr.s    LocateBTNode         ; compound symbol found?
         beq.s    4$                   ; no ...

         ; Compound symbol node found, so compound name becomes the default.

         move.l   d5,d4                ; compound name

         ; Final value look-up ...

3$:      movea.l  btPst(a0),a1         ; table entry
         move.l   (a1),d1              ; (be_PValue) value assigned?
         beq.s    4$                   ; no ...
         move.l   d1,d4                ; new value
         moveq    #0,d3                ; ... not a literal

4$:      move.l   a0,d0                ; node
         movea.l  d4,a1                ; value string
         move.l   d3,d1                ; literal flag (set CCR)
         movem.l  (sp)+,d3-d5
         rts

* =========================     LocateBTNode     =======================
* Searches a binary tree for the specified string.  No search is performed  
* if the root is NULL.  The condition codes are always set for the return.
* Registers:   A0 -- root of tree
*              A1 -- string structure
* Return:      D0 -- located node or 0
*              A0 -- last node checked
LocateBTNode
         move.l   a0,d0                ; first test node
         beq.s    6$                   ; no root ...

         movem.l  d2/d3/a2-a4,-(sp)    ; delayed save ...
         movea.l  a1,a3                ; search string
         move.w   ns_Length(a3),d2     ; search string length
         move.b   ns_Hash(a3),d3       ; search string hash code

1$:      move.l   d0,a2                ; current test node
         movea.l  a2,a4                ; (HIGHMODE) default branch
         movea.l  BTNAME(a2),a1        ; test name string
         cmp.w    ns_Length(a1),d2     ; lengths match?
         bcs.s    2$                   ; no -- shorter
         bhi.s    3$                   ; no -- longer

         ; Equal lengths ... check whether the hash codes match.

         cmp.b    ns_Hash(a1),d3       ; hash codes match?
         beq.s    4$                   ; yes

         ; HIGH/LOW branch selected by even/odd hash codes, to randomize
         ; sequences of strings differing by only one character.

         btst     #0,d3                ; odd hash code?
         bne.s    3$                   ; yes ... "HIGH" branch

2$:      addq.w   #LOWMODE,a4          ; low branch

3$:      move.l   (a4),d0              ; next node?
         bne.s    1$                   ; yes ... loop back

         ; Node not found ... save branch offset.

         suba.l   a2,a4                ; branch offset
         move.w   a4,BTMODE(a2)        ; install branch mode
         tst.l    d0                   ; set CCR
         bra.s    5$

         ; Length and hash codes match ... compare the name strings.

4$:      lea      ns_Buff(a3),a0       ; first string
         addq.w   #ns_Buff,a1          ; second string
         move.w   d2,d0                ; comparison length
         CALLSYS  StrcmpN              ; D0=match flag
         bmi.s    2$                   ; no match ... lower
         bgt.s    3$                   ; no match ... higher
         move.l   a2,d0                ; exact match

5$:      movea.l  a2,a0                ; last node checked
         movem.l  (sp)+,d2/d3/a2-a4

6$:      rts

* =========================     EnterSymbol     ==========================
* Enters the specified symbol in the symbol table, allocating a node if
* necessary.  The stem and compound strings are (conditionally) released.
* Registers:   A0 -- storage environment
*              A1 -- stem string
*              D0 -- compound string
* Return:      D0 -- symbol table node
*              A0 -- same
EnterSymbol
         movem.l  d2/d5/a2-a4,-(sp)
         movea.l  a0,a4                ; environment
         move.l   d0,d5                ; compound string

         move.b   ns_Hash(a1),d0       ; hash code for stem
         bsr.s    BTIndex              ; D0=index
         movea.l  ev_ST(a4),a0         ; symbol table
         lea      st_Root(a0,d0.w),a3  ; root address
         movea.l  (a3),a0              ; first-level root
         bsr.s    BTLink               ; A0=node

         ; Check if second-level look-up required ...

         tst.l    d5                   ; compound symbol given?
         beq.s    1$                   ; no ...
         movea.l  btPst(a0),a3         ; symbol table entry
         addq.w   #be_Next,a3          ; second-level root address
         movea.l  (a3),a0              ; root node
         movea.l  d5,a1                ; compound string
         bsr.s    BTLink               ; A0=node

1$:      move.l   a0,d0                ; final node
         movem.l  (sp)+,d2/d5/a2-a4
         rts

* ==========================      BTIndex     ==========================
* Creates the symbol table index from the string hash code.  Registers A0/A1
* are preserved.
* Registers:   D0 -- hash code
* Return:      D0 -- table index
BTIndex
         move.b   d0,d1                ; save code
         moveq    #$0F,d0              ; nibble mask
         and.b    d1,d0                ; low-order nibble
         lsr.b    #4,d1                ; high-order nibble
         eor.b    d1,d0                ; ... folded index
         lsl.b    #2,d0                ; scale for 4-byte entries
         rts

* ===========================      BTLink     ==========================
* Private routine to locate a symbol table node.  If not found, a new node
* is allocated and linked.
* Registers:   A0 -- root node (may be NULL)
*              A1 -- name string
*              A3 -- root address
*              A4 -- environment
* Return:      A0 -- new node
* Scratch:     A2/D2
BTLink
         movea.l  a1,a2                ; save name string ...
         bsr      LocateBTNode         ; D0=node  A0=last node
         exg      a0,d2                ; node or parent=>D2
         movea.l  a4,a0                ; environment
         beq.s    1$                   ; ... not found

         ; Symbol node already exists ... release the name string.

         movea.l  a2,a1                ; name string
         bsr      FreeString           ; release it
         movea.l  d2,a0                ; return node
         rts                           ; quick exit

         ; Symbol node not found, so we have to allocate a new node.

1$:      bsr.s    GetBTNode            ; D0=A0=new node
         exg      a0,d2                ; parent=>A0, new node=>D2
         move.l   a0,d0                ; parent exists?
         beq.s    2$                   ; no ...

         ; Existing binary tree ... link in the new node.  The linkage mode
         ; has been set by the call to 'LocateBTNode'.

         movea.l  d2,a1                ; new node
         move.w   BTMODE(a0),d0        ; linkage mode
         bsr.s    LinkBTNode           ; link it
         bra.s    3$

         ; Add new node as a root 

2$:      move.l   d2,(a3)              ; save as new root node

         ; Prepare and store the name string ...

3$:      movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; name string
         bsr      KeepString           ; D0=A1=string
         movea.l  d2,a0                ; new node
         move.l   a1,BTNAME(a0)        ; install name
         rts

* =========================     LinkBTNode     =========================
* Links a new node into a binary tree at the specified branch.  The parent
* node is assumed not to have any descendants at the linked branch.
* Registers:   A0 -- root of tree
*              A1 -- new node
*              D0 -- mode switch (HIGH/LOW offset)
LinkBTNode
         move.l   a0,BTPARENT(a1)      ; install parent
         adda.w   d0,a0                ; index to slot
         move.l   a1,(a0)              ; install node
         rts

* =========================     GetBTNode     ==========================
* Allocates a node for a binary tree, used for the symbol table and for
* building expression parse trees.
* Registers:   A0 -- storage environment
* Return:      D0 -- allocated node
*              A0 -- same
GetBTNode
         bsr      FindQuickTwo         ; D0=A0=node

         ; Initialize the node ... unrolled for speed.

         clr.l    (a0)+                ; (bt_HiKid)
         clr.l    (a0)+                ; (bt_LoKid)
         clr.l    (a0)+                ; (pt_Parent)
         clr.l    (a0)+                ; (bt_Flags/bt_Mode)
         clr.l    (a0)+                ; (bt_Name)
         move.l   a0,(btPst-BTENTRY)(a0)
         clr.l    (a0)+                ; (BTENTRY+be_PValue)
         clr.l    (a0)                 ; (BTENTRY+be_Next)

         movea.l  d0,a0
         rts

* ========================     FreeSymTable     ==========================
* Releases a symbol table structure and any binary trees.
* Registers:   A0 -- storage environment
FreeSymTable
         move.l   ev_ST(a0),d0         ; a symbol table?
         beq.s    2$                   ; no -- nothing to do

         movem.l  d5/a2/a4/a5,-(sp)    ; delayed save ...
         movea.l  a0,a4                ; environment
         movea.l  d0,a5                ; symbol table

         ; Loop through the binary tree root array, releasing all trees.

         movea.l  a5,a2                ; symbol table
         moveq    #ST_SIZE-1,d5        ; entries - 1

1$:      movea.l  (a2)+,a1             ; root node
         bsr.s    FreeBTree            ; release tree
         movea.l  a4,a0                ; restore environment
         dbf      d5,1$                ; loop back

         movea.l  a5,a1                ; symbol table
         moveq    #st_SIZEOF,d0        ; size 
         bsr      FreeSpace            ; recycle it
         movem.l  (sp)+,d5/a2/a4/a5

2$:      rts

* ===========================     SetValue     =========================
* Assigns a value to a symbol table node, releasing any pre-existing value
* strings.  If a second-level tree exists, it is released.
* Registers:   A0 -- environment
*              A1 -- value string
*              D0 -- symbol table node
SetValue
         move.l   a5,-(sp)
         move.l   a0,-(sp)             ; push environment
         movea.l  d0,a5
         movea.l  btPst(a5),a5         ; symbol table entry

         ; Mark the string as 'private', then recycle the old value.

         bsr      KeepString           ; D0=A1=string
         movea.l  (a5),a1              ; load old value (may be NULL)
         move.l   d0,(a5)+             ; install new value

         movea.l  (sp),a0              ; environment
         bsr      FreeKeepStr          ; release the old value

         ; Assignment to a node releases the secondary tree, if any.

         movea.l  (a5),a1              ; second-level tree
         clr.l    (a5)                 ; clear slot
         movea.l  (sp)+,a0             ; pop environment
         movea.l  (sp)+,a5             ; restore register

* =========================     FreeBTree     ==========================
* Releases the nodes in a (possibly multi-level) binary tree.
* Registers:   A0 -- storage environment
*              A1 -- root of the tree
FreeBTree
         move.l   a1,d1                ; root given?
         beq.s    FBN001               ; no ... quick exit

         movem.l  d2/a2/a4,-(sp)       ; delayed save
         movea.l  a0,a4                ; save environment
         clr.l    BTPARENT(a1)         ; just to be sure ...

1$:      movea.l  d1,a2                ; test node
         moveq    #LOWMODE,d2          ; check LOW branch first

2$:      movea.l  a2,a0                ; node
         adda.w   d2,a0                ; selected slot
         move.l   (a0),d1              ; descendant at this branch?
         beq.s    3$                   ; no ...

         movea.l  d1,a1
         move.l   BTLOW(a1),d0         ; low descendant?
         or.l     (a1),d0              ; high descendent?
         bne.s    1$                   ; yes ... loop back

         clr.l    (a0)                 ; close this path
         movea.l  a4,a0                ; environment
         bsr.s    FreeBTNode           ; release node

3$:      subq.w   #LOWMODE,d2          ; next mode?
         bcc.s    2$                   ; yes ... loop back
         move.l   BTPARENT(a2),d1      ; move up a level?
         bne.s    1$                   ; yes ...

         ; Finally, fall through and release the root node ...

         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; node
         movem.l  (sp)+,d2/a2/a4       ; restore registers

* =========================     FreeBTNode     =========================
* Releases a binary-tree node.
* Registers:   A0 -- storage environment
*              A1 -- node to be released
FreeBTNode
         move.l   a2,-(sp)
         move.l   a0,-(sp)             ; push environment
         movea.l  a1,a2

         ; Release the name string

         movea.l  BTNAME(a2),a1        ; the name string
         bsr      FreeKeepStr          ; release it

         ; Release the value string stored in this node ...

         movea.l  (BTENTRY+be_PValue)(a2),a1
         movea.l  (sp),a0              ; environment
         bsr      FreeKeepStr          ; release string

         ; Recursive call to release the second-level tree ...

         move.l   (BTENTRY+be_Next)(a2),d1
         beq.s    1$                   ; ... no root
         movea.l  (sp),a0              ; environment
         movea.l  d1,a1                ; root node
         bsr.s    FreeBTree            ; release tree

         ; ... and then release the node itself.

1$:      movea.l  (sp)+,a0             ; pop environment
         movea.l  a2,a1                ; node
         bsr      FreeQuickTwo         ; D0=boolean
*         moveq    #bt_SIZEOF,d0        ; size
*         CALLSYS  FreeSpace            ; recycle it
         movea.l  (sp)+,a2

FBN001   rts
