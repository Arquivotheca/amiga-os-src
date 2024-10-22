* == struct.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     GetRange     ==========================
* Allocates a control range structure and pushes it onto the range stack.
* Registers:   A0 -- storage environment
*              D0 -- source position
*              D1 -- range type
* Return:      D0 -- address of allocated structure
*              A0 -- same as D0
GetRange
         move.l   a4,-(sp)
         movea.l  a0,a4                ; save environment
         move.l   d0,-(sp)             ; push position
         move.b   d1,-(sp)             ; push type

         ; Allocate and clear a range structure (three quanta).

         bsr      FindQuickThree       ; D0=A0=range
         moveq    #r_SIZEOF,d0         ; size of structure
         CALLSYS  ClearMem             ; (A0 preserved)

         move.b   (sp)+,r_Type(a0)     ; install type
         move.l   (sp)+,r_SrcPos(a0)   ; install position

         ; Push the range onto the control stack.

         movea.l  a0,a1                ; range
         lea      ev_RStack(a4),a0     ; control stack
         movea.l  a1,a4                ; save range
         bsr      PushStack            ; stack it

         movea.l   a4,a0               ; return range
         move.l   a0,d0
         movea.l  (sp)+,a4
         rts

* =========================     CopyClause     =========================
* Allocates a new clause and moves tokens from the parent clause.
* Registers:   A0 -- storage environment
*              A1 -- parent clause
*              D0 -- start token
* Return:      D0 -- new clause
*              A1 -- same
CopyClause
         move.l   a3,-(sp)
         movea.l  a1,a3                ; save clause

         ; Allocate the new clause ...

         move.l   d0,-(sp)             ; push token
         bsr.s    NewClause            ; D0=A1=clause
         movea.l  (sp)+,a0             ; pop token
         exg      a1,a3                ; old clause=>A1, new clause=>a3

         ; Unlink from the old clause and relink in the new.

         movea.l  cTL_TailPred(a1),a1  ; last token
         bsr      Unlink               ; D0=first A1=last
         lea      cTL_Head(a3),a0      ; insertion point
         bsr      Relink               ; ... relink tokens

         move.l   a3,d0                ; new clause
         movea.l  d0,a1
         movea.l  (sp)+,a3
         rts

* ==========================     NewClause     =========================
* Allocates a new clause, copying selected fields.
* Registers:   A0 -- storage environment
*              A1 -- parent clause
* Return:      D0 -- new clause
*              A1 -- same
NewClause
         move.l   a1,-(sp)             ; push parent
         bsr.s    GetClause            ; D0=A0=clause
         movea.l  (sp)+,a0             ; pop parent
         movea.l  d0,a1                ; new clause

         ; Copy selected fields from the old to the new clause.

         move.l   c_SrcLine(a0),c_SrcLine(a1); c_SrcLine/c_Len
         move.l   c_SrcPos(a0),c_SrcPos(a1)  ; c_SrcPos
         rts

* =========================     GetClause     ==========================
* Allocates a clause structure.
* Registers:   A0 -- storage environment
* Return:      A0 -- pointer to allocated structure
*              D0 -- same as A0
GetClause
         bsr      FindQuickThree       ; D0=A0=clause

         ; Initialize the clause ... 

         addq.w   #c_Type,a0           ; data area
         clr.l    (a0)+                ; (c_Type/c_Flags/c_Count)
         clr.l    (a0)+                ; (c_Link)

         clr.l    (a0)+                ; (c_SrcLine/c_Len)
         addq.w   #8,a0                ; (skip c_Keys/c_SrcPos)
         clr.l    (a0)+                ; (c_NextPos)

         ; Initialize the token list header.

         lea      (cTL_Tail-cTL_Head)(a0),a1
         move.l   a1,(a0)              ; (cTL_Head)
         clr.l    (a1)+                ; (cTL_Tail)
         move.l   a0,(a1)              ; (cTL_TailPred)

         movea.l  d0,a0
         rts

* ==========================     EndToken     ==========================
* Private routine to link a STOPTOKEN into a clause.
* Registers:   A4 -- storage environment
*              A5 -- clause
EndToken
         movea.l  a4,a0
         movea.l  a5,a1                ; new clause
         moveq    #0,d0
         moveq    #T_END,d1            ; fall through

* =========================     AddToken     ===========================
* Allocates a token structure and intializes the fields.  If the clause
* is specified, the token is linked into the list and the count incremented.
* Registers:   A0 -- storage environment
*              A1 -- clause pointer
*              D0 -- token data
*              D1 -- token type byte
* Return:      D0 -- address of token structure
*              A0 -- same
AddToken
         move.l   a1,-(sp)             ; push clause

         ; Allocate a new token ... one quantum of memory.

         move.l   d0,-(sp)             ; push token data
         move.b   d1,-(sp)             ; push token type
         bsr      FindQuickOne         ; D0=A0=token

         addq.w   #t_Type,a0           ; data area
         move.b   (sp)+,(a0)+          ; (t_Type) pop type
         clr.b    (a0)+                ; (t_Flags) clear flags
         addq.w   #2,a0                ; (t_Offset) skip offset
         move.l   (sp)+,(a0)           ; (t_Data) pop data

         ; Check whether the token is to be added to a clause list.

1$:      move.l   (sp)+,d1             ; a clause?
         beq.s    2$                   ; no ...

         ; Link the token to the end of list ...

         movea.l  d1,a0                ; clause
         movea.l  cTL_TailPred(a0),a1  ; tail predecessor
         move.l   d0,cTL_TailPred(a0)  ; new tail predecessor
         movea.l  d0,a0                ; new node
         move.l   (a1),(a0)+           ; install successor
         move.l   a1,(a0)              ; install predecessor
         move.l   d0,(a1)

2$:      movea.l  d0,a0                ; return token
         rts

* =========================     AddString     ==========================
* Allocates a string structure and copies the given string
* Registers:   A0 -- storage environment
*              A1 -- string buffer
*              D0 -- attribute flags
*              D1 -- length of string (bytes)
* Return:      D0 -- address of allocated structure
*              A0 -- same
AddString
         move.l   a1,-(sp)             ; save string pointer
         bsr.s    GetString            ; D0=A0=string
         move.l   (sp)+,d1             ; string pointer supplied?
         beq.s    1$                   ; no ...

         ; Copy the given string ...

         move.l   a0,-(sp)             ; push string
         move.w   ns_Length(a0),d0     ; string length
         addq.w   #ns_Buff,a0          ; destination buffer
         movea.l  d1,a1                ; source buffer
         CALLSYS  StrcpyN              ; D0=hash code
         move.l   (sp)+,a0             ; pop string
         move.b   d0,ns_Hash(a0)       ; install hash code

1$:      move.l   a0,d0                ; return string
         rts

* ========================     GetString     ===========================
* Allocates a string structure of the given length.  The length and flag
* bytes are set as given.
* Registers:   A0 -- storage environment
*              D0 -- attribute flag byte
*              D1 -- length of string
* Return:      D0 -- string structure
*              A0 -- same
GetString
         move.l   d2,-(sp)
         move.b   d0,-(sp)             ; push flag byte

         moveq    #NXADDLEN,d0         ; structure offset
         moveq    #0,d2                ; clear length
         move.w   d1,d2                ; string length
         add.l    d2,d0                ; ... total length
         bsr      FindSpace            ; D0=A0=structure

         ; Initialize the string ...

         clr.l    (a0)+                ; (IVALUE) clear value
         move.w   d2,(a0)+             ; ns_Length
         move.b   (sp)+,(a0)+          ; ns_Flags
         clr.b    (a0)+                ; ns_Hash
         adda.l   d2,a0                ; end of string
         clr.b    (a0)                 ; ... install a null

         movea.l  d0,a0
         move.l   (sp)+,d2
         rts

* =========================     UpperString     ========================
* Converts a string to UPPERcase.  If the string is owned, a copy is made
* before the conversion is performed.
* Registers:   A0 -- environment
*              A1 -- string
* Return:      D0 -- converted string
*              A1 -- same as D0
UpperString
         move.l   a2,-(sp)
         movea.l  a1,a2                ; source string

         ; Check whether the string is owned ...

         moveq    #$FF&~(NSF_SOURCE!NSF_KEEP),d0
         move.b   ns_Flags(a2),d1      ; string attributes
         and.b    d1,d0                ; selected attributes
         cmp.b    d1,d0                ; 'SOURCE' or 'KEEP'?
         beq.s    1$                   ; no ...

         ; String owned or static ... allocate a new string structure.

         move.w   ns_Length(a2),d1     ; length
         bsr.s    GetString            ; D0=A0=string
         movea.l  a2,a1                ; restore source
         movea.l  d0,a2                ; save new string

         ; Copy the string, converting to uppercase.

1$:      lea      ns_Buff(a2),a0       ; destination
         addq.w   #ns_Buff,a1          ; source string
         move.w   ns_Length(a2),d0     ; length
         CALLSYS  StrcpyU              ; D0=hash
         move.b   d0,ns_Hash(a2)       ; install hash

         movea.l  a2,a1                ; return
         move.l   a1,d0
         movea.l  (sp)+,a2
         rts

* ========================     KeepString     ==========================
* Marks a string for ownership by a symbol table.  If necessary, a copy
* of the string is made.
* Registers:   A0 -- environment
*              A1 -- string (may be NULL)
* Return:      D0 -- marked string
*              A1 -- same as D0
KeepString
         move.l   a1,d0                ; a string?
         beq.s    1$                   ; no ...

         ; Check whether the string is permanent ... no copy required.

         move.b   ns_Flags(a1),d1      ; attribute flags
         bmi.s    1$                   ; ... 'SOURCE' string

         ; May need to make a copy ...

         bsr.s    MayCopyString        ; D0=A1=string
         ori.b    #NSF_KEEP,ns_Flags(a1)

1$:      rts

* =======================     MayCopyString     ========================
* Conditionally copies a string structure if the KEEP flag is set.
* Registers:   A0 -- storage environment
*              A1 -- string structure
* Return:      A1 -- string structure
MayCopyString
         move.l   a1,d0
         btst     #NSB_KEEP,ns_Flags(a1)
         beq.s    CS001                ; ... no copy needed.

* ========================     CopyString     ==========================
* Copies a string structure, which is ALWAYS long-word aligned and a
* multiple of 4 bytes long.  The NSF_OWNED flags are cleared so that the
* copied string is uncommitted.
* Registers:   A0 -- storage environment
*              A1 -- string structure to be copied
* Return:      D0 -- copy of structure
*              A1 -- same as D0
CopyString
         moveq    #0,d0                ; clear length
         move.w   ns_Length(a1),d0     ; string length
         moveq    #NXADDLEN,d1         ; structure offset
         add.l    d1,d0                ; structure length
         movem.l  d0/a1,-(sp)          ; push length/string
         bsr      FindSpace            ; D0=A0=structure
         movem.l  (sp)+,d1/a1          ; pop length/string

1$:      move.l   (a1)+,(a0)+          ; longword copy
         subq.l   #4,d1                ; any left?
         bgt.s    1$                   ; loop back

         ; Clear the ownership flags so structure is uncommitted ...

         movea.l  d0,a1                ; return pointer
         andi.b   #($FF&~NSF_OWNED),ns_Flags(a1)

CS001    rts

* =========================     FreeRange     ==========================
* Releases the topmost control range and any associated structures.
* Registers:   A0 -- storage environment
* Return:      D0 -- boolean success flag
FreeRange
         move.l   a4,-(sp)
         movea.l  a0,a4

         lea      ev_RStack(a4),a0     ; the range stack
         bsr      PopStack             ; D0=topmost range
         beq.s    5$                   ; no ranges ...

         move.l   a5,-(sp)             ; delayed save
         movea.l  d0,a5                ; save range

         ; Check for an 'INTERPRET' range.

         move.b   r_Type(a5),d0        ; range type
         subq.b   #NRANGE_INTERP,d0    ; an INTERPRET range?
         bne.s    3$                   ; no ...

         ; An 'INTERPRET' ... release the source segment array.

         move.l   ev_Segment(a4),d1    ; segment array?
         beq.s    1$                   ; no
         movea.l  a4,a0                ; environment
         movea.l  d1,a1
         bsr.s    FreeSrcSeg           ; release the array

         ; Restore previous fields.

1$:      move.l   r_Segment(a5),ev_Segment(a4)  ; restore segment array
         move.l   r_NextPos(a5),ev_PC(a4)       ; restore PC
         move.l   r_SrcPos(a5),SRCPOS(a4)       ; restore source position

         ; Check if leaving 'DEBUG' mode ... reset 'PAUSE' flag.

         btst     #NRFB_DEBUG,r_Flags(a5) ; 'DEBUG' source?
         beq.s    2$                      ; no ...
         bclr     #(EXB_DEBUG-16),(STATUS+1)(a4)
         bset     #(EXB_PAUSE-24),(STATUS+0)(a4)

2$:      bset     #(EXB_FLUSH-24),(STATUS+0)(a4)
         bra.s    4$

         ; Check for a 'DO'-range.

3$:      addq.b   #NRANGE_INTERP-NRANGE_DO,d0
         bne.s    4$                   ; not a 'DO'

         ; Release the private strings saved in the range structure.

         movea.l  a4,a0                ; environment
         movea.l  r_Index(a5),a1       ; index variable
         bsr.s    FreeKeepStr          ; release it

         movea.l  a4,a0                ; environment
         movea.l  r_TO(a5),a1          ; 'TO' result
         bsr.s    FreeKeepStr

         movea.l  a4,a0                ; environment
         movea.l  r_BY(a5),a1          ; 'BY' result
         bsr.s    FreeKeepStr

         move.l   r_Segment(a5),d1     ; an 'UNTIL' clause?
         beq.s    4$                   ; no ...
         movea.l  a4,a0                ; environment
         movea.l  d1,a1                ; clause
         bsr.s    FreeClause           ; release it

         ; Release the range structure.

4$:      movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; range
         bsr      FreeQuickThree       ; D0=boolean
         movea.l  (sp)+,a5

5$:      movea.l  (sp)+,a4
         rts

* =======================     FreeSrcSeg     ===========================
* Releases a source segment array and the associated string structures.
* Registers:   A0 -- storage environment
*              A1 -- pointer to the segment array
FreeSrcSeg
         movem.l  d2/a1/a2/a4,-(sp)
         movea.l  a0,a4                ; storage environment

         move.l   (a1)+,d2             ; segment count
         movea.l  a1,a2                ; segment array
         bra.s    2$                   ; jump in

1$:      movea.l  (a2)+,a1             ; string structure
         bsr.s    FreeString           ; release it
2$:      movea.l  a4,a0                ; storage environment
         subq.l   #1,d2                ; any segments left?
         bcc.s    1$                   ; yes ...

         ; Restore registers and fall through ...

         movem.l  (sp)+,d2/a1/a2/a4

* ==========================     FreeSeg     ===========================
* Releases the storage asociated with a segment array
* Registers:   A0 -- storage environment
*              A1 -- pointer to the segment array
FreeSeg
         move.l   (a1),d0                 ; array count
         lsl.l    #2,d0                   ; multiply by 4
         addq.l   #4,d0                   ; ... plus count
         bra      FreeSpace

* ========================     FreeKeepStr     =========================
* Releases the space associated with a string structure, which may be
* specified as NULL.  The string is not released if the NSF_SOURCE or
* NSF_EXT attributes are set.
* Registers:   A0 -- storage environment
*              A1 -- string structure to be released
* Return:      D0 -- boolean
FreeKeepStr
         move.l   a1,d0                ; a string given?
         beq.s    FS002                ; no

         move.b   #NSF_SOURCE!NSF_EXT,d1
         bra.s    FS001

* ========================     FreeString     ==========================
* Releases the space associated with a string structure.  If the NSF_OWNED
* flags are set, the string is not released.
* Registers:   A0 -- storage environment
*              A1 -- string structure to be released
* Return:      D0 -- boolean success flag
FreeString
         move.b   #NSF_OWNED,d1        ; ownership flags

FS001    moveq    #0,d0                ; clear return
         and.b    ns_Flags(a1),d1      ; structure currently owned?
         bne.s    FS002                ; yes

         move.w   ns_Length(a1),d0     ; string length
         moveq    #NXADDLEN,d1         ; structure offset
         add.l    d1,d0                ; ... total length
         bsr      FreeSpace            ; D0=boolean

FS002    tst.l    d0                   ; set CCR
         rts

* ========================     FreeClause     ==========================
* Releases a clause and any associated tokens.
* Registers:   A0 -- storage environment
*              A1 -- pointer to clause
* Return:      D0 -- boolean success flag
FreeClause
         movem.l  d2/a4/a5,-(sp)       ; delayed save ...
         movea.l  a0,a4                ; save environment
         movea.l  a1,a5                ; save clause

         ; Release the tokens ... no need to unlink first!

         move.l   cTL_Head(a5),d2      ; first token
         bra.s    2$                   ; jump in

1$:      bsr.s    FreeLooseToken       ; release token
2$:      movea.l  a4,a0                ; environment
         movea.l  d2,a1                ; token
         move.l   (a1),d2              ; a successor?
         bne.s    1$                   ; yes ... loop back

         ; Release the clause (three quanta).

         movea.l  a5,a1                ; clause
         movem.l  (sp)+,d2/a4/a5       ; restore registers
         bra      FreeQuickThree       ; D0=boolean

* ========================     FlushClause     =========================
* Flushes the execution stack.
* Registers:   A0 -- environment
FlushClause
         move.l   a4,-(sp)
         movea.l  a0,a4                ; save environment

1$:      lea      ev_EStack(a4),a0     ; list header
         bsr      RemHead              ; D0=clause A0=header
         beq.s    2$                   ; ... all done

         move.l   a4,a0                ; environment
         movea.l  d0,a1                ; clause
         bsr.s    FreeClause           ; release it
         bra.s    1$                   ; loop back

2$:      clr.w    skNum(a0)            ; clear count
         movea.l  (sp)+,a4
         rts

* ========================     QFreeToken     ==========================
* "Quick" routine to free a token.
* Registers:   A4 -- environment
*              A5 -- clause
*              D0 -- token
QFreeToken
         movea.l  a4,a0
         movea.l  a5,a1

* ========================     FreeToken     ===========================
* Releases a token structure.  If the clause pointer is given, the token
* is unlinked from the Token List; otherwise, it is assumed to be free.
* Registers:   A0 -- storage environment pointer
*              A1 -- address of clause (or 0 if token not linked)
*              D0 -- address of token
FreeToken
         move.l   a1,d1                ; clause pointer given?
         beq.s    1$                   ; no ...

         ; Remove the token ... must preserve A0/D0!

         movea.l  d0,a1                ; token
         move.l   (a1)+,d1             ; successor
         movea.l  (a1),a1              ; predecessor
         move.l   d1,(a1)              ; install successor
         exg      d1,a1                ; exchange
         move.l   d1,sn_Pred(a1)       ; install predecessor

1$:      movea.l  d0,a1                ; token

* Frees a loose (unlinked) token ...
* Registers:   A0 -- environment
*              A1 -- token
FreeLooseToken
         move.l   TDATA(a1),d1         ; a string?
         beq.s    2$                   ; no ...
         tst.b    t_Flags(a1)          ; (NOSTR) suppress recycling?
         bmi.s    2$                   ; yes

         ; Check whether the token carries a string.  Only SYMBOL, STRING,
         ; TERM, or STOPTOKENs can have a string structure; the bits in the
         ; test mask indicate whether the token can carry a string.

         move.l   a1,-(sp)             ; delayed save
         move.b   t_Type(a1),d0        ; token type
         movea.l  d1,a1                ; string (maybe)
         move.l   #TSTMASK,d1          ; test mask
         btst     d0,d1                ; token carries a string?
         beq.s    1$                   ; no

         tst.b    ns_Flags(a1)         ; 'SOURCE' attribute?
         bmi.s    1$                   ; yes

         move.l   a0,-(sp)             ; push environment
         bsr      FreeString           ; release string
         movea.l  (sp)+,a0             ; pop environment

1$:      movea.l  (sp)+,a1             ; pop token

         ; Free the token structure ... one quantum.

2$:      bra      FreeQuickOne          ; D0=boolean
