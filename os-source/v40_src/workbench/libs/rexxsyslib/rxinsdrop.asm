* == rxinsdrop.asm =====================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ========================     rxInsDROP     ===========================
* Implements the DROP instruction.
* [instruction module environment]
rxInsDROP
         tst.w    c_Count(a5)          ; any symbols?
         beq      rxErr31              ; no
         subq.b   #TRACE_S,d7          ; 'SCAN' mode?
         beq.s    2$                   ; yes

         ; Loop through the symbols assigning null values.

         move.l   a0,a2                ; first token

1$:      movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; token
         moveq    #0,d0                ; no value
         moveq    #0,d1                ; (no node)
         CALLSYS  AssignValue          ; D0=error A0=node
         move.l   d0,d6                ; an error?
         bne.s    2$                   ; yes

         movea.l  (a2),a2              ; next token
         cmpi.b   #T_END,t_Type(a2)    ; the STOPTOKEN?
         bne.s    1$                   ; no ... loop back

2$:      rts

* =========================     rxInsPROC     ==========================
* Implements the 'PROCEDURE' instruction.
* [instruction module environment]
rxInsPROC
         movea.l  a0,a2                ; first (check) token
         bset     #EFB_NEWST,EVFLAGS(a4)
         bne      rxErr19              ; error ... table already exists

         ; Allocate the new symbol table.

         movea.l  a4,a0                ; environment
         bsr      GetSymTable          ; D0=table
         move.l   d0,ev_ST(a4)         ; install it

         ; Check for an 'EXPOSE' list ...

         andi.w   #KTF_SECMASK,d5      ; 'EXPOSE' list?
         beq.s    1$                   ; no
         tst.w    c_Count(a5)          ; any symbols?
         beq      rxErr31              ; no -- error
         subq.b   #TRACE_S,d7          ; trace 'SCAN' mode?
         bne.s    2$                   ; no
         movea.l  cTL_TailPred(a5),a2  ; ... use last token

1$:      movea.l  a2,a0                ; check token
         bra      rxCheckSTOP

         ; An 'EXPOSE' keyword ... scan the list of symbols.

2$:      movea.l  TSTRING(a2),a1       ; name string

         ; Check whether the symbol name must be protected from deletion.

         move.b   ns_Flags(a1),d4      ; save attributes
         move.b   #NSF_OWNED,d0        ; ownership mask
         and.b    d4,d0                ; protected?
         bne.s    3$                   ; yes
         bset     #NSB_KEEP,ns_Flags(a1)

3$:      moveq    #0,d1                ; clear compound string
         cmpi.b   #T_SYMCMPD,t_Type(a2); compound?
         bne.s    4$                   ; no

         ; Expand the compound symbol (in the NEW environment)

         movea.l  a4,a0                ; new environment
         CALLSYS  SymExpand            ; D0=error D1=compound A1=stem
         move.l   d0,d6                ; error?
         bne.s    8$                   ; yes

         ; Check whether to copy the stem ...

4$:      move.l   a1,d2                ; save stem
         move.l   d1,d3                ; save compound
         beq.s    5$                   ; ... not compound

         ; Compound symbol entry ... make a copy of the stem symbol.

         movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=string

         ; Enter the symbol in the OLD environment's table.

5$:      movea.l  EVPRED(a4),a0        ; old environment
         move.l   d3,d0                ; compound
         CALLSYS  EnterSymbol          ; D0=A0=node
         movea.l  a0,a3                ; save node

         ; If a compound entry, use name in old node as compound name.

         move.l   d3,d0                ; compound symbol?
         beq.s    6$                   ; no
         move.l   BTNAME(a3),d0        ; ... reuse name

         ; Enter the symbol in the NEW environment's table.

6$:      movea.l  a4,a0                ; environment
         movea.l  d2,a1                ; stem
         CALLSYS  EnterSymbol          ; D0=A0=node

         ; Now for the trick:  change the new node's symbol entry pointer
         ; to point at the old symbol entry.

         move.l   btPst(a3),btPst(a0)  ; change pointer

         ; Check whether to restore the symbol name attribute flags ...

         movea.l  TSTRING(a2),a1       ; symbol name
         cmp.b    ns_Flags(a1),d4      ; changed?
         beq.s    7$                   ; no
         move.b   d4,ns_Flags(a1)      ; ... restore flags

7$:      movea.l  (a2),a2              ; next token
         cmpi.b   #T_END,t_Type(a2)    ; the STOPTOKEN?
         bne.s    2$                   ; no -- loop back

8$:      rts

* ==========================     rxInsUPPER    =========================
* Implements the UPPER instruction.
* [instruction module environment]
rxInsUPPER
         tst.w    c_Count(a5)          ; any symbols?
         beq      rxErr31              ; no
         subq.b   #TRACE_S,d7          ; 'SCAN' mode?
         beq.s    4$                   ; yes -- nothing to do

         ; Look up the current value of the symbol ...

         movea.l  a0,a2                ; first (symbol) token

1$:      movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; token
         moveq    #0,d0                ; (no node)
         CALLSYS  LookUpValue          ; D0=error D1=literal A0=node A1=value
         move.l   d0,d6                ; error?
         bne.s    4$                   ; yes
         move.l   a0,d3                ; symbol node
         movea.l  a4,a0                ; environment

         ; Check for a literal value.  No change is made if the symbol is
         ; a literal, even though an expanded compound name may include
         ; lowercase characters.

         tst.w    d1                   ; a literal?
         beq.s    2$                   ; no

         ; Release the value string unless it's the token string.

         cmpa.l   TSTRING(a2),a1       ; token symbol?
         beq.s    3$                   ; yes
         bsr      FreeString           ; release string
         bra.s    3$

         ; Convert the string to UPPERCASE.  Return string will always be
         ; uncommitted, since the string value may have been changed.

2$:      bsr      UpperString          ; D0=A1=string

         ; Install the new value in the symbol node ...

         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; token
         move.l   d3,d1                ; symbol table node
         CALLSYS  AssignValue          ; D0=node

         ; Check for the terminating token ...

3$:      movea.l  (a2),a2              ; next token
         cmpi.b   #T_END,t_Type(a2)    ; the STOPTOKEN?
         bne.s    1$                   ; no -- loop back

4$:      rts
