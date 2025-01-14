* == rxinsif.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     rxInsIF     ===========================
* Implements the 'IF' and 'WHEN' instructions.
* [Instruction module environment]
rxInsIF
rxInsWHEN
         btst     #EXB_SKIP,d4         ; 'SKIP' mode?
         bne.s    1$                   ; yes
         cmpi.b   #TRACE_S,d7          ; 'SCAN' mode?
         bne.s    2$                   ; no

         ; 'SKIP' or 'SCAN' mode ... use last token for check.

1$:      move.l   cTL_TailPred(a5),d2  ; last token
         bra.s    6$                   ; skip conversion

2$:      move.l   a0,d2                ; first token
         andi.w   #KTF_SECMASK,d5      ; inline 'THEN'?
         beq.s    3$                   ; no
         move.l   (a0),d2              ; use next token

         ; Check whether an expression was supplied.

3$:      btst     #CFB_NULLEXPR,c_Flags(a5)
         beq.s    5$                   ; a result ...
         moveq    #ERR10_045,d0        ; "expression required"

4$:      move.l   d0,d6                ; error code
         rts

         ; Convert the expression result to a boolean flag.

5$:      movea.l  a4,a0                ; environment
         bsr      CVs2bool             ; D0=error D1=boolean
         bne.s    4$                   ; error ...

         ; Check whether this is an 'IF' or 'WHEN' instruction.

6$:      btst     #IPB_SELECT,d5       ; 'WHEN' instruction?
         beq.s    rIF                  ; no

         ; A 'WHEN' instruction ... check for valid context.

         tst.w    skNum(a2)            ; any ranges?
         beq      rxErr20              ; no -- error
         movea.l  (a2),a2              ; top (innermost) range
         cmpi.b   #NRANGE_SELECT,r_Type(a2)
         bne      rxErr20
         move.w   d1,r_Test(a2)        ; save result

         ; Check whether the status is still 'CONDitional'.

         btst     #EXB_COND,d4         ; still conditional?
         bne      rxErr20              ; yes

         bset     #NRFB_INIT,r_Flags(a2)  ; initialized?
         bne      rxErr24                 ; yes -- error
         btst     #NRFB_ELSE,r_Flags(a2)  ; an 'OTHERWISE' clause?
         bne      rxErr20                 ; yes -- error
         bclr     #NRFB_THEN,r_Flags(a2)  ; clear 'THEN' after each 'WHEN'
         bra.s    rIFTest

         ; A 'IF' instruction ... check test result.

rIF      move.w   d1,d3                ; save boolean
         bne.s    1$                   ; TRUE test ...

         ; Test FALSE ... don't create a range if the next position is known.

         move.l   c_NextPos(a5),d0     ; position known?
         bne      rIFMove              ; yes

         ; Allocate and stack an 'IF' range structure.

1$:      movea.l  a4,a0                ; environment
         move.l   c_SrcPos(a5),d0      ; source position
         moveq    #NRANGE_IF,d1        ; range type
         bsr      GetRange             ; D0=A0=range
         movea.l  a0,a2                ; save range
         move.w   d3,r_Test(a2)        ; install test

         ; Install the "next position" from the clause into the range.

         move.l   c_NextPos(a5),r_NextPos(a2)

         ; Set range 'INIT' and check whether to set range 'ACTIVE'.

         bset     #NRFB_INIT,r_Flags(a2)
         btst     #EXB_SKIP,d4         ; 'SKIP' mode?
         bne.s    rIFTest              ; yes
         bset     #NRFB_ACTIVE,r_Flags(a2)

         ; Check whether an inline "THEN' was included ...

rIFTest  tst.w    d5                   ; an inline 'THEN'?
         bne.s    rTHENTest            ; yes
         beq      rIFCheck             ; no

* ==========================     rxInsTHEN     ==========================
* Implements the 'THEN' instruction.  Valid in 'IF' or 'SELECT' ranges.
rxInsTHEN
         move.l   a0,d2                ; check token
         tst.w    skNum(a2)            ; any ranges?
         beq      rxErr20              ; no -- error

         movea.l  (a2),a2              ; top (innermost) range
         move.b   r_Type(a2),d0        ; range type
         subq.b   #NRANGE_IF,d0        ; an 'IF' range?
         beq.s    rTHENTest            ; yes
         subq.b   #NRANGE_SELECT-NRANGE_IF,d0
         bne      rxErr20              ; error ... not 'IF' or SELECT

         ; Get the skip flag for a 'THEN' clause.

rTHENTest
         bclr     #NRFB_INIT,r_Flags(a2)  ; clear 'INITialized'
         beq      rxErr20                 ; not set -- error
         bset     #NRFB_THEN,r_Flags(a2)  ; set 'THEN'
         bne      rxErr24                 ; already set -- error
         move.w   r_Test(a2),d0           ; test result
         not.w    d0                      ; invert it ...
         bra.s    rItest

* ==========================      rxInsOTHRW     ========================
* Implements the 'OTHERWISE' instruction.  Valid only in a 'SELECT' range.
rxInsOTHRW
         moveq    #NRANGE_SELECT,d0    ; valid range
         bra.s    rELSE

* ==========================     rxInsELSE     ==========================
* Implements the 'ELSE' instruction.  Valid only in an 'IF' range.
rxInsELSE
         moveq    #NRANGE_IF,d0        ; valid range

rELSE    move.l   a0,d2                ; check token
         tst.w    skNum(a2)            ; any ranges?
         beq      rxErr21              ; no -- error
         movea.l  (a2),a2              ; top (innermost) range
         cmp.b    r_Type(a2),d0        ; valid range?
         bne      rxErr21              ; no

         ; Check whether the status is still 'CONDitional'.

         btst     #EXB_COND,d4         ; still 'CONDitional'?
         bne      rxErr21              ; yes -- error
         btst     #NRFB_THEN,r_Flags(a2)
         beq      rxErr24              ; error ... no 'THEN' clause

         ; Get the skip flag for an 'ELSE' clause.

         bset     #NRFB_ELSE,r_Flags(a2)
         bne      rxErr21              ; ... already 'ELSE' or 'OTHERWISE'
         move.w   r_Test(a2),d0        ; test result

         ; Decide whether to skip the next statement ... set 'BRANCH' flag
         ; when a branch is taken.

rItest   bset     #(EXB_NEXTC-8),(STATUS+2)(a4)
         subq.b   #TRACE_S,d7          ; trace 'SCAN' mode?
         beq.s    1$                   ; yes
         tst.w    d0                   ; skip next statement?
         bne.s    2$                   ; yes

         ; Branch to be taken ... set flag and look up "next position".

1$:      bset     #NRFB_BRANCH,r_Flags(a2)
         tst.w    r_NextPos(a2)        ; position known?
         bne.s    rIFCheck             ; yes

         ; Check whether the range clause has been cached ...

         movea.l  a4,a0                ; environment
         move.l   r_SrcPos(a2),d0      ; range clause position
         bsr      FindCachedClause     ; D0=A0=cached clause
         beq.s    rIFCheck             ; ... not found
         move.l   c_NextPos(a0),r_NextPos(a2)
         bra.s    rIFCheck

         ; Branch not taken ... clear flag and check "next position".

2$:      bclr     #NRFB_BRANCH,r_Flags(a2)
         move.l   r_NextPos(a2),d0     ; next position known?
         bne.s    rIFMove              ; yes

         ; Next position not known ... set skip flag.

         bset     #(EXB_SKIP-8),(STATUS+2)(a4)

         ; Check that the clause was terminated properly.

rIFCheck movea.l  d2,a0                ; token to check
         bra      rxCheckSTOP

         ; Next position known ... update PC and set 'MOVE'.  No need to
         ; check last token, since syntax has to be correct to set "next".

rIFMove  bsr      NewPosition          ; update PC
         rts
