* == assignvalue.asm ===================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ========================     AssignValue     =========================
* Assigns a value to a symbol token.  Compound names are expanded prior
* to the assignment.  If the symbol table node is provided (non-zero),
* no checking is performed and the value is assigned directly.
* Registers:   A0 -- environment
*              A1 -- token
*              D0 -- value string
*              D1 -- symbol table node
* Return:      D0 -- error code
*              A0 -- symbol table node
*              A1 -- value string
AssignValue
         movem.l  d2/a2-a4,-(sp)
         movea.l  a0,a4                ; environment
         movea.l  d0,a3                ; value string

         move.l   d1,d0                ; node supplied?
         bne.s    3$                   ; yes

         ; Check whether to expand the compound symbol name ...

         movea.l  TSTRING(a1),a2       ; symbol name string
         cmpi.b   #T_SYMCMPD,t_Type(a1); a compound symbol?
         movea.l  a2,a1                ; ... name string
         bne.s    1$                   ; no

         ; Expand the compound name.  The stem string is always new,
         ; but the compound string may be the original symbol.

         CALLSYS  SymExpand            ; D0=error D1=compound A1=stem
         bne.s    4$                   ; ... error

         ; Check whether to protect the symbol name from deletion.  This
         ; may force the symbol table routine to copy the name string.

1$:      move.b   ns_Flags(a2),d2      ; protected?
         bmi.s    2$                   ; yes ... 'SOURCE' attribute
         bset     #NSB_KEEP,ns_Flags(a2)

         ; Enter the name in the symbol table ... stem and compound name
         ; strings are conditionally released.

2$:      movea.l  a4,a0                ; environment
         move.l   d1,d0                ; compound string
         CALLSYS  EnterSymbol          ; D0=A0=node

         ; Restore the original string attributes, if necessary.

         tst.b    d2                   ; 'SOURCE'?
         bmi.s    3$                   ; yes
         move.b   d2,ns_Flags(a2)      ; ... restore flags

         ; Store the value string in the new node.

3$:      move.l   d0,d2                ; save node
         movea.l  a4,a0                ; environment
         movea.l  a3,a1                ; value string
         CALLSYS  SetValue             ; install it

         movea.l  d2,a0                ; symbol table node
         movea.l  a3,a1                ; value string
         moveq    #0,d0                ; return code

4$:      movem.l  (sp)+,d2/a2-a4
         rts

* ========================     AssignSpecial     =======================
* Assigns a value to a REXX special variable.  The value supplied must be
* a string structure if the assignment is to 'RESULT' (index 0), and an
* integer if the assignment is to 'RC' or 'SIGL'.
* receive integer value
* Registers:   A0 -- environment
*              D0 -- string or integer
*              D1 -- variable index
AssignSpecial
         move.l   a4,-(sp)
         movea.l  a0,a4                ; environment

         ; Check whether the assignment is to RESULT (STRESULT = 0) ...

         move.w   d1,-(sp)             ; push index
         beq.s    1$                   ; ... no conversion

         ; 'RC' or 'SIGL' ... convert the integer value to a string.

         bsr      CVi2s                ; D0=string

         ; Install the string in the specified special variable.

1$:      move.w   (sp)+,d1             ; pop index
         movea.l  a4,a0                ; environment
         movea.l  ev_ST(a0),a1         ; symbol table
         lea      st_RESULT(a1,d1.w),a4; special variable slot
         lea      gtRESULT(pc),a1      ; token array
         lsl.w    #2,d1                ; scale for 16 bytes
         adda.w   d1,a1                ; selected token
         move.l   (a4),d1              ; cached node
         CALLSYS  AssignValue          ; A0=node A1=value
         move.l   a0,(a4)              ; ... cache the node

         movea.l   (sp)+,a4
         rts

* ========================     LookUpValue     =========================
* Retrieves the value currently assigned to a symbol.  If the symbol table
* node is supplied, the table search is skipped (but compound names are
* still expanded, since the symbol may be a literal).
* Registers:   A0 -- environment
*              A1 -- token
*              D0 -- symbol table node (or 0)
* Return:      D0 -- error code
*              D1 -- literal flag
*              A0 -- symbol table node
*              A1 -- value string
LookUpValue
         movem.l  d2/d3/a2-a4,-(sp)
         movea.l  a0,a4                ; environment
         movea.l  TSTRING(a1),a3       ; symbol name

         moveq    #0,d1                ; clear compound string
         move.l   d0,d2                ; save node
         moveq    #-1,d3               ; literal flag

         move.b   t_Type(a1),d0        ; token type
         subi.b   #T_SYMFIXED,d0       ; a fixed symbol?
         beq.s    4$                   ; yes -- skip lookup

         movea.l  a3,a1                ; stem string
         subq.b   #T_SYMCMPD-T_SYMFIXED,d0
         bne.s    1$                   ; ... not compound

         ; Expand the compound symbol name.

         CALLSYS  SymExpand            ; D0=error D1=compound A1=stem
         bne.s    7$                   ; ... error

1$:      movea.l  a1,a2                ; save stem
         exg      d1,d2                ; node=>D1, compound=>D2
         movea.l  a4,a0                ; environment
         move.l   d2,d0                ; compound string
         bsr      GetValue             ; D0=node D1=flag A1=value
         exg      a1,a3                ; name=>A1, value=>A3
         move.l   d1,d3                ; save flag
         exg      d0,d2                ; compound=>D0, node=>D2

         ; Check whether to recycle the compound and stem strings.

         tst.l    d0                   ; compound name?
         beq.s    3$                   ; no -- nothing to release

         ; Check whether the compound name is the symbol or value string.

         cmp.l    a3,d0                ; the value string?
         beq.s    2$                   ; yes ... don't release
         cmp.l    a1,d0                ; the symbol name?
         beq.s    2$                   ; yes ... don't release

         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; compound name
         bsr      FreeString           ; release it

         ; Always release the stem string for a compound look-up.

2$:      movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; stem name
         bsr      FreeString           ; release it

         ; Check whether the 'NOVALUE' signal is set.

3$:      tst.w    d3                   ; a literal?
         beq.s    4$                   ; no
         movea.l  a4,a0                ; environment
         moveq    #ERR10_039,d0        ; "uninitialized variable"
         moveq    #INB_NOVALUE,d1      ; signal flag
         bsr      rxSignal             ; D0=error d1=flag
         bne.s    7$                   ; ... error

         ; Check if tracing 'INTERMEDIATES'.  If so, trace the value string.

4$:      moveq    #TRACEMASK,d0        ; load mask
         and.b    TRACEOPT(a4),d0      ; trace option
         subq.b   #TRACE_I,d0          ; 'INTERMEDIATES'?
         bne.s    6$                   ; no

         moveq    #ACT_INTL,d1         ; 'LITERAL' mode
         tst.w    d3                   ; a literal?
         bne.s    5$                   ; yes
         moveq    #ACT_INTV,d1         ; 'VALUE' mode
5$:      movea.l  a4,a0                ; environment
         movea.l  a3,a1                ; result string
         bsr      rxTrace

         ; Load return values 

6$:      movea.l  d2,a0                ; symbol table node
         movea.l  a3,a1                ; value string
         move.l   d3,d1                ; literal flag
         moveq    #0,d0                ; clear return

7$:      movem.l  (sp)+,d2/d3/a2-a4
         rts

* ==========================     SymExpand     =========================
* Expands a compound symbol name.  The expansion uses the global work area.
* Registers:      A0 -- storage environment
*                 A1 -- symbol (stem) string
* Return:         D0 -- error code or 0
*                 D1 -- compound string
*                 A1 -- stem string

         STRUCTURE SymbolFrame,0
         UBYTE    Ctype                ; type of entry
         UBYTE    Cpad
         UWORD    Clen                 ; length
         APTR     Cpss                 ; string pointer
         LABEL    Csize                ; size: 8 bytes

VALFRAME SET      0                    ; a value frame
TXTFRAME SET      1                    ; a text frame
SYMFRAME SET      2                    ; a symbol frame

SYMBMODE SET      15
TEXTMODE SET      16
NOTLIT   SET      31                   ; not literal (sign bit)
SymExpand
         movem.l  d2-d7/a2-a5,-(sp)
         movea.l  a0,a4                ; environment
         movea.l  a1,a5                ; symbol string
         movea.l  ev_GlobalPtr(a4),a1  ; global pointer
         movea.l  gn_TBuff(a1),a3      ; global buffer (frame base)

         moveq    #'.',d2              ; load period
         moveq    #0,d3                ; frame offset
         moveq    #ERR10_040,d6        ; default error ("invalid variable")

         ; Count the characters in the stem ...

         lea      ns_Buff(a5),a1       ; name string
         move.w   ns_Length(a5),d0     ; length
         move.l   a1,d1                ; save start

1$:      cmp.b    (a1)+,d2             ; a period?
         dbeq     d0,1$                ; loop back

         exg      d1,a1                ; start=>A1, end=>D1
         sub.l    a1,d1                ; stem length
         move.l   d1,d4                ; initial total length
         move.l   d1,d5                ; initial scan index

         ; Create the first frame for the stem.

         movem.l  d1/a1,(a3)           ; type/length/pointer
         addq.b   #TXTFRAME,(a3)       ; ... set type

         ; Create the stem string structure.

         moveq    #NSF_STRING,d0       ; string type
         bsr      AddString            ; D0=A0=string structure
         move.l   d0,d7                ; save stem string

         ; Make sure there's room in the global work area.

SEScan   moveq    #-(Csize*2),d0       ; two frames
         add.w    (ra_Length-ra_Buff)(a3),d0
         cmp.w    d3,d0                ; frame index <= space?
         bcc.s    0$                   ; yes

         ; No space ... attempt the increase the work buffer.

         movea.l  ev_GlobalPtr(a4),a0  ; global pointer
         bsr      DoubleWorkBuffer     ; D0=A0=buffer
         beq      SEOut                ; ... failure
         movea.l  d0,a3                ; new frame buffer

0$:      lea      ns_Buff(a5,d5.l),a1  ; current scan position
         addq.w   #1,d5                ; advance scan index
         moveq    #0,d1                ; clear length
         cmp.b    (a1),d2              ; 'TEXT' character?
         beq.s    2$                   ; yes ...

         ; Not a period ... assume a symbol character.

         bclr     #TEXTMODE,d2         ; clear 'TEXT' flag
         bset     #SYMBMODE,d2         ; entering 'SYMBOL' mode?
         bne.s    1$                   ; no ...

         addq.w   #Csize,d3            ; advance frame
         lea      0(a3,d3.l),a0        ; symbol frame pointer
         movem.l  d1/a1,(a0)           ; type/length/position
         addq.b   #SYMFRAME,(a0)       ; set type
         movea.l  d3,a2                ; save frame index

1$:      addq.w   #1,Clen(a3,d3.l)     ; increment name length
         bra.s    6$

         ; A period found ... check if already in 'TEXT' mode

2$:      bset     #TEXTMODE,d2         ; entering 'TEXT' mode?
         bne.s    3$                   ; no -- already in it

         addq.w   #Csize,d3            ; advance frame
         lea      0(a3,d3.l),a0        ; current frame pointer
         movem.l  d1/a1,(a0)           ; type/length/position
         addq.b   #TXTFRAME,(a0)       ; set type

3$:      addq.w   #1,Clen(a3,d3.l)     ; advance length
         addq.l   #1,d4                ; increment total length

         ; Check whether 'SYMBOL' mode is ending ...

4$:      bclr     #SYMBMODE,d2         ; SYMBOL mode?
         beq.s    6$                   ; no ...

         ; Allocate a name string and reclassify as a 'value' frame.

         adda.l   a3,a2                ; symbol frame
         clr.b    (a2)                 ; reclassify as VALFRAME
         movea.l  a4,a0                ; environment
         moveq    #NSF_STRING,d0
         movem.l  (a2),d1/a1           ; length/string
         bsr      AddString            ; D0=A0=string
         move.l   d0,Cpss(a2)          ; save name string

         ; Look up the current value for the string.

         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; symbol name
         moveq    #0,d0                ; (not compound)
         moveq    #0,d1                ; no node
         bsr      GetValue             ; D0=node D1=flag A1=value

         ; Check if it's a literal string.  If so, the length doesn't need
         ; to be updated.  If not, the NAME string is released.

         tst.w    d1                   ; a literal?
         bne.s    5$                   ; yes ... keep it

         bset     #NOTLIT,d2           ; no ... set flag
         lea      Clen(a2),a0
         move.w   ns_Length(a1),(a0)+  ; value string length (A0 => Cpss())
         move.l   (a0),d1              ; load NAME string
         move.l   a1,(a0)              ; install VALUE string
         movea.l  a4,a0                ; environment
         movea.l  d1,a1                ; name string
         bsr      FreeString           ; release it ...

         ; Increment the total length.

5$:      add.l    (a2),d4              ; add length (upper word 0)

         ; Check whether we're done with the scan.

6$:      cmp.w    ns_Length(a5),d5     ; scan done?
         bcs      SEScan               ; no ... loop back

         tst.w    d2                   ; last symbol?
         bmi.s    4$                   ; yes ... loop back

         ; Scan completed ... check for a literal value.

SECheck  tst.l    d2                   ; a literal?
         bpl.s    SEDone               ; yes ... no string needed

         ; Not a literal ... make sure the final length is not too long.

         cmp.l    MaxLen(pc),d4        ; too long?
         bgt.s    SEOut                ; yes ... error

         ; Allocate a string for the compound name.

         movea.l  a4,a0                ; environment
         moveq    #NSF_STRING!NSF_NOTNUM,d0
         move.w   d4,d1                ; expanded length
         bsr      GetString            ; D0=A0=string
         movea.l  d0,a5                ; don't need original name

         ; Scan through the node frames, copying into the final string.

         movea.l  a3,a0                ; frame pointer
         lea      ns_Buff(a5),a2       ; destination
         moveq    #0,d1                ; clear hash
         move.w   d3,d4                ; frame index

1$:      tst.b    (a0)                 ; a value frame?
         movem.l  (a0)+,d0/a1          ; (load length/value)
         bne.s    3$                   ; no ... jump in
         addq.w   #ns_Buff,a1          ; offset to string
         bra.s    3$                   ; jump in

2$:      move.b   (a1)+,(a2)           ; copy character
         add.b    (a2)+,d1             ; increment hash
3$:      dbf      d0,2$                ; loop back

         subq.w   #Csize,d4            ; another frame?
         bcc.s    1$                   ; yes ... loop back
         move.b   d1,ns_Hash(a5)       ; install hash

SEDone   moveq    #0,d6                ; clear error

         ; Cleanup operations ... release strings in value frames.

SEOut    movea.l  a3,a2                ; frame base

1$:      tst.b    (a2)                 ; value frame?
         movem.l  (a2)+,d0/a1          ; (load string)
         bne.s    2$                   ; no
         movea.l  a4,a0                ; environment
         bsr      FreeString           ; release it

2$:      subq.w   #Csize,d3            ; any nodes left?
         bcc.s    1$                   ; yes ... loop back

         ; Check whether to trace the expanded name.

         moveq    #TRACEMASK,d0        ; trace mask
         and.b    TRACEOPT(a4),d0      ; trace option
         subq.b   #TRACE_I,d0          ; 'INTERMEDIATES'?
         bne.s    3$                   ; no

         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; expanded name
         moveq    #ACT_INTC,d1         ; (expanded) compound name
         bsr      rxTrace

3$:      movea.l  d7,a1                ; stem string
         move.l   a5,d1                ; return compound string
         move.l   d6,d0                ; return code
         movem.l  (sp)+,d2-d7/a2-a5
         rts
