* == rxinsdo.asm =======================================================
*
* Copyright © 1986, 1987-1991 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     rxInsDO     ===========================
* Implements the 'DO' instruction.
* [instruction module environment]
SKIPMODE SET      31
rxInsDO
         movea.l  a0,a3                ; first token
         moveq    #0,d2                ; clear value

         moveq    #KT4_OPTMASK,d0      ; options mask
         and.l    d0,d5                ; secondary options
         btst     #EXB_SKIP,d4         ; 'SKIP' mode?
         bne.s    1$                   ; yes
         subq.b   #TRACE_S,d7          ; trace 'SCAN' mode?
         bne.s    2$                   ; no
1$:      bset     #SKIPMODE,d5         ; set flag

         ; Check whether the loop is already active.

2$:      move.l   c_SrcPos(a5),d0      ; position of clause
         moveq    #NRANGE_DO,d1        ; range type
         tst.w    skNum(a2)            ; any ranges?
         beq.s    3$                   ; no
         movea.l  (a2),a2              ; top (innermost) range
         cmp.b    r_Type(a2),d1        ; a 'DO' range?
         bne.s    3$                   ; no
         cmp.l    r_SrcPos(a2),d0      ; loop matches clause?
         beq.s    6$                   ; yes -- already activated

         ; Allocate and initialize a new 'DO' range.

3$:      movea.l  a4,a0                ; environment
         bsr      GetRange             ; D0=A0=range
         movea.l  d0,a2                ; save range

         ; Install the "next position" from the clause.

         move.l   c_NextPos(a5),r_NextPos(a2)

         ; Check whether the range is 'active'.

         movea.l  a3,a1                ; first token
         tst.l    d5                   ; (SKIPMODE) skipping?
         bmi.s    4$                   ; yes
         bset     #NRFB_ACTIVE,r_Flags(a2)
         not.w    r_Test(a2)           ; set 'test' flag
         movea.l  cTL_TailPred(a5),a1  ; assignment token

         ; An iterative range?  A range is iterative if explicit options
         ; were specified or if the first token carries a value.

4$:      tst.w    d5                   ; any options?
         bne.s    41$                  ; yes
         move.l   TSTRING(a3),d2       ; a value string?
         beq.s    6$                   ; no

         ; An iterative 'DO' ... set flag and install "next position".

41$:     bset     #NRFB_ITERATE,r_Flags(a2)
         move.l   c_NextPos(a5),r_NextPos(a2)

         ; Check for an initializer expression and set defaults.

         moveq    #(KT4_OPTMASK&KT4_INIT),d0
         and.w    d5,d0                ; initializer expression?
         beq.s    5$                   ; no

         ; Save the index symbol from the assignment token ... at the head
         ; of the token list if 'SKIPping', otherwise the last token.

         movea.l  a4,a0                ; environment
         movea.l  TSTRING(a1),a1       ; symbol name
         bsr      KeepString           ; D0=A1=string
         move.l   a1,r_Index(a2)       ; save index symbol

         move.l   rl_TRUE(a6),r_BY(a2) ; default 'BY' value (global '1')
         move.b   #OP_LE,RCOMP(a2)     ; comparison opcode
         bra.s    6$

         ; Check for invalid options: 'TO' or 'BY' without initializer.

5$:      moveq    #(KT4_OPTMASK&(KT4_TO!KT4_BY)),d0
         and.w    d5,d0                ; a 'TO' or 'BY' expression?
         bne      rxErr28              ; yes -- error

         ; Check for an implicit 'FOR' count ...

         moveq    #(KT4_OPTMASK&KT4_FOR),d0
         and.w    d5,d0                ; explicit 'FOR'?
         bne.s    6$                   ; yes
         move.l   TSTRING(a3),d2       ; value string or NULL

         ; Check for active loop processing.

6$:      moveq    #NRFF_ACTIVE!NRFF_ITERATE,d0
         and.b    r_Flags(a2),d0       ; select flags
         cmpi.b   #NRFF_ACTIVE,d0      ; active non-iterative?
         beq      rDOOut               ; yes

         btst     #NRFB_ACTIVE,d0      ; active?
         beq      rDOTerm              ; no

         tst.l    d2                   ; counter value?
         beq.s    rDOFast              ; no
         movea.l  d2,a1                ; value string

         ; Implicit loop count or 'FOR' token ... set the loop counter.

rDOSet   movea.l  a1,a0                ; string
         bsr      CVs2i                ; D0=error D1=value
         bne      rDOError             ; ... error
         move.l   d1,r_Count(a2)       ; loop count >= 0?
         bmi      rxErr44              ; no ... error
         not.w    r_Pad(a2)            ; set flag

         ; Fast track for implicit count ... no scan required.

rDOFast  tst.w    d5                   ; any options?
         beq      rDOCount             ; no ... update count

         ; Scan the token list to pick up expression results.  If an 'UNTIL'
         ; expression was specified, it will be preceded by a STOPTOKEN.

rDOScan  movea.l  a3,a0                ; current token
         movea.l  (a3),a3              ; next token

         move.b   t_Type(a0),d0        ; token type
         subq.b   #T_END,d0            ; the STOPTOKEN?
         beq.s    rDOCheck             ; yes ...
         subq.b   #T_TERM-T_END,d0     ; a TERM token?
         bne      rxErr35              ; no ... error

         ; Check for keyword tokens ... successor may have value.

         move.w   TKEYCODE(a0),d0      ; subkeyword
         moveq    #KT4_EXPRMASK,d1     ; "expression required" mask
         and.w    d0,d1                ; expression required?
         beq.s    rDOScan              ; no ... loop back

         ; Keyword token is a 'TO', 'BY', 'FOR', or 'WHILE' ... next token
         ; must have an expression result!

         move.l   TSTRING(a3),d1       ; a value string?
         beq      rxErr45              ; no ... error
         movea.l  d1,a1                ; value string

         ; Decode the token keyword and make a private copy of the result
         ; as necessary.

         movea.l  a4,a0                ; storage environment
         subq.w   #KT4_TO,d0           ; 'TO' token?
         beq.s    1$                   ; yes ...
         subq.w   #KT4_BY-KT4_TO,d0    ; 'BY' token?
         beq.s    2$                   ; yes ...
         subq.w   #KT4_FOR-KT4_BY,d0   ; 'FOR' token?
         beq.s    rDOSet               ; yes ... loop back
*         cmpi.w   #KT4_WHILE-KT4_FOR,d0; 'WHILE' token?
*         bne.s    rDOScan              ; no ... loop back

         ; A 'WHILE' token ... make sure it's a Boolean value.

         bsr      CVs2bool             ; D0=error D1=boolean
         bne      rDOError             ; ... error
         move.w   d1,r_Test(a2)        ; set test flag
         bra.s    rDOScan              ; loop back

         ; A 'TO' token ... save the value.

1$:      bsr      KeepString           ; D0=A1=string
         move.l   a1,r_TO(a2)          ; install it
         bra.s    rDOScan              ; loop back

         ; A 'BY' token ... save the value.

2$:      bsr      KeepString           ; D0=A1=string
         move.l   a1,r_BY(a2)          ; install it

         ; Check whether the 'BY' result is >= 0

         movea.l  a4,a0                ; environment
         moveq    #OP_GE,d0            ; comparison operator
         move.l   rl_FALSE(a6),d1      ; use 'FALSE' as 0
         CALLSYS  EvalOp               ; D0=error D1=value
         bne      rDOError             ; ... error

         ; Make sure the 'BY' value was numeric.

         movea.l  r_BY(a2),a1          ; 'BY' result
         btst     #NSB_NOTNUM,ns_Flags(a1)
         bne      rxErr44              ; error ... not numeric

         ; Value is numeric ... check the algebraic sign.

         cmp.l    rl_TRUE(a6),d1       ; value >= 0? ('TRUE' result?)
         beq.s    rDOScan              ; yes ... loop back
         move.b   #OP_GE,RCOMP(a2)     ; no -- use '>=' operator
         bra.s    rDOScan              ; loop back

         ; Check for loop completion conditions.

rDOCheck move.l   a3,d3                ; save current token
         tst.w    r_Test(a2)           ; flag still set?
         beq      rDOTerm              ; no -- termination processing

         ; Check whether the 'TO' limit has been reached.

         moveq    #(KT4_OPTMASK&KT4_TO),d0
         and.w    d5,d0                ; a 'TO' expression?
         beq.s    rDOUntil             ; no

         ; Attempt a fast lookup using the symbol table directly ...

         movea.l  r_Value(a2),a0       ; symbol node
         move.l   a0,d0                ; node known?
         bne.s    3$                   ; yes

         ; Look up the current value of the index symbol.

         movea.l  a4,a0                ; environment
         movea.l  cTL_TailPred(a5),a1  ; index token
         movea.l  a1,a3                ; save token
         CALLSYS  LookUpValue          ; D0=error D1=literal A0=node A1=value

         tst.w    d1                   ; literal value?
         beq.s    2$                   ; no

         ; Value is a literal ... release it and return error.

         cmpa.l   TSTRING(a3),a1       ; symbol name?
         beq.s    1$                   ; yes ... don't release
         movea.l  a4,a0                ; environment
         bsr      FreeString           ; release string

1$:      bra      rxErr44              ; error ... "invalid result"

         ; Cache the node pointer if the index isn't a compound symbol.

2$:      cmpi.b   #T_SYMCMPD,t_Type(a3); compound?
         beq.s    3$                   ; yes
         move.l   a0,r_Value(a2)       ; save node

         ; Get the index value from the symbol table node ...

3$:      movea.l  btPst(a0),a0         ; symbol entry
         movea.l  (a0),a1              ; (be_PValue) value
         move.l   a1,d1                ; a value?
         beq.s    1$                   ; no ... error
         movea.l  a1,a3                ; save operand

         ; Compare the index variable to the 'TO' limit.

         movea.l  a4,a0                ; environment
         move.b   RCOMP(a2),d0         ; comparison operator
         move.l   r_TO(a2),d1          ; 'TO' limit
         CALLSYS  EvalOp               ; D0=error D1=value
         bne.s    rDOError             ; ... error

         ; Make sure a numeric comparison was done ...

         movea.l  r_TO(a2),a0          ; 'TO' string
         move.b   ns_Flags(a0),d0      ; 'TO' attributes
         or.b     ns_Flags(a3),d0      ; OR index attributes
         btst     #NSB_NOTNUM,d0       ; either non-numeric?
         bne.s    1$                   ; yes ... error

         cmp.l    rl_TRUE(a6),d1       ; 'TRUE' result?
         bne.s    rDOTerm              ; no

         ; Check for an 'UNTIL' expression.

rDOUntil moveq    #(KT4_OPTMASK&KT4_UNTIL),d0
         and.w    d5,d0                ; 'UNTIL' given?
         beq.s    rDOFor               ; no

         ; Create the 'UNTIL' clause and post it to the range.  All tokens
         ; after the current token are transferred to the new clause.

         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; parent clause
         move.l   d3,d0                ; current token
         bsr      CopyClause           ; D0=A1=clause
         move.w   c_Type(a5),c_Type(a1); c_Type/c_Flags
         andi.b   #CFF_ANYFUNC!CFF_PARSED,c_Flags(a1)
         move.l   #KTF_UNTIL,c_Keys(a1)
         move.l   a1,r_Segment(a2)     ; post it to the range

         ; Check whether the loop counter was initialized ...

rDOFor   tst.w    r_Pad(a2)            ; loop counter?
         beq.s    rDOOut               ; no

rDOCount subq.l   #1,r_Count(a2)       ; decrement count
         bpl.s    rDOOut               ; ... still active

         ; Loop termination processing ... clear 'active' flag and move
         ; to 'END' clause if position is known.

rDOTerm  clr.w    r_Test(a2)           ; clear flag
         bclr     #NRFB_ACTIVE,r_Flags(a2)
         move.l   r_NextPos(a2),d0     ; 'END' position known?
         beq.s    rDOOut               ; no ...
         bra      NewPosition          ; update PC

         ; An error ... save the error code.

rDOError move.l   d0,d6                ; save error

rDOOut   rts

* =========================     rxInsSELECT     ========================
* Implements the 'SELECT' instruction.
rxInsSELECT
         movea.l  a4,a0                ; environment
         move.l   c_SrcPos(a5),d0      ; source position
         moveq    #NRANGE_SELECT,d1    ; range type
         bsr      GetRange             ; D0=A0=range

         btst     #EXB_SKIP,d4         ; in 'SKIP' mode?
         bne.s    1$                   ; yes 

         ; Range active ... set the 'ACTIVE' flag.  The "next position"
         ; is not installed in the range until a branch has been taken.

         bset     #NRFB_ACTIVE,r_Flags(a0)
         rts

         ; In 'skip' mode ... see if the end position is known.

1$:      move.l   c_NextPos(a5),d0     ; next position?
         beq.s    rDOOut               ; no ...

         ; Install the next position and update the PC.

         move.l   d0,r_NextPos(a0)     ; install position
         bra      NewPosition          ; update PC

* ==========================     rxInsEND     ==========================
* Implements the 'END' instruction.  Valid in 'DO' and 'SELECT' ranges.
* [instruction module environment]
rxInsEND
         movea.l  a0,a3                ; check token
         tst.w    skNum(a2)            ; any ranges?
         beq      rxErr26              ; error ... "unexpected END"

         movea.l  (a2),a2              ; topmost range
         move.b   r_Type(a2),d0        ; range type
         move.b   r_Flags(a2),d2       ; range flags
         subq.b   #NRANGE_DO,d0        ; a 'DO' range?
         beq.s    2$                   ; yes
         subq.b   #NRANGE_SELECT-NRANGE_DO,d0
         bne      rxErr26              ; ... error

         ; A 'SELECT' range ... check that the syntax was correct.

         btst     #NRFB_ELSE,d2        ; an OTHERWISE?
         bne.s    1$                   ; yes
         btst     #NRFB_ACTIVE,d2      ; still active?
         bne      rxErr25              ; yes ... "missing OTHERWISE"

         ; If the range was skipped using the "next position", skip over
         ; the syntax checking.

         tst.l    r_NextPos(a2)        ; next position known?
         bne.s    1$                   ; yes
         btst     #NRFB_THEN,d2        ; a THEN clause?
         beq      rxErr24              ; no ... "missing THEN"
         btst     #EXB_COND,d4         ; still conditional?
         bne      rxErr29              ; yes ... error

1$:      bset     #NRFB_FINISH,r_Flags(a2)
         bra.s    6$                   ; check for cache

         ; A 'DO' range ... check if it's active and iterative.

2$:      btst     #EXB_SKIP,d4         ; 'SKIP' mode?
         bne.s    4$                   ; yes
         cmpi.b   #TRACE_S,d7          ; trace 'SCAN' mode?
         beq.s    4$                   ; yes

         moveq    #NRFF_ACTIVE!NRFF_ITERATE,d1
         and.b    d1,d2                ; selected flags
         cmp.b    d1,d2                ; active and iterative?
         bne.s    4$                   ; no

         ; An active iterative range.  Update PC for next iteration.  

         move.l   c_SrcPos(a5),r_NextPos(a2)
         move.l   r_SrcPos(a2),d0      ; starting PC
         move.l   r_Segment(a2),d1     ; an 'UNTIL' clause?
         bne.s    3$                   ; yes

         ; Update the PC and set 'MOVE' flag.

         bsr      NewPosition          ; update PC
         bra.s    5$

         ; Stack the 'UNTIL' clause for execution.

3$:      bsr.s    StackUNTIL           ; stack clause
         bra.s    5$

         ; Range all done ... set finish flag.

4$:      bset     #NRFB_FINISH,r_Flags(a2)

         ; Check for an index symbol on the first execution ... 'INIT' flag
         ; used to prevent subsequent checks.

5$:      bset     #NRFB_INIT,r_Flags(a2)
         bne.s    7$                   ; ... no more checking

         tst.w    c_Count(a5)          ; a symbol?
         beq.s    6$                   ; no
         move.l   r_Index(a2),d1       ; an index variable?
         beq      rxErr27              ; no -- error

         ; Compare the index variable to the symbol name.

         movea.l  TSTRING(a3),a0       ; symbol string
         movea.l  d1,a1                ; index variable
         CALLSYS  CmpString            ; symbols match?
         beq      rxErr27              ; no -- error
         movea.l  (a3),a3              ; advance check token

         ; Check whether the range clause is cached ... if so, install the
         ; current source position in the cached clause "next position" slot.
         ; This will enable a fast exit on future executions of the range.

6$:      movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      SetCachePosition     ; install position

         ; Check that the clause was terminated properly.

         movea.l  a3,a0                ; token to check
         bra      rxCheckSTOP

7$:      rts

* ========================      StackUNTIL     =========================
* Stacks an 'UNTIL' clause and clears the segment slot.
* Registers:      D1 -- 'UNTIL' clause
StackUNTIL
         clr.l    r_Segment(a2)        ; clear segment
         lea      ev_EStack(a4),a0     ; execution stack
         movea.l  d1,a1                ; 'UNTIL' clause
         bra      PushTail             ; stack it

* ========================      rxInsUNTIL     =========================
* Implements the 'UNTIL' instruction.  This instruction is created from
* a 'DO' instruction, and is inserted into the pipeline after the 'END'.
* [instruction module environment]
rxInsUNTIL
         btst     #CFB_NULLEXPR,c_Flags(a5)
         bne      rxErr45              ; ... expression required

         ; Make sure there's a 'DO' range on top.

         tst.w    skNum(a2)            ; any ranges?
         beq      rxErr28              ; no
         movea.l  (a2),a2              ; top (innermost) range
         cmpi.b   #NRANGE_DO,r_Type(a2); 'DO' range?
         bne      rxErr28              ; no ... error

         ; Convert the expression result to a boolean flag.

         movea.l  a4,a0                ; environment
         bsr      CVs2bool             ; D0=error  D1=boolean
         move.l   d0,d6                ; error?
         bne.s    NP001                ; yes ...

         move.l   r_SrcPos(a2),d0      ; 'DO' position
         tst.w    d1                   ; TRUE result?
         beq.s    NewPosition          ; no

         ; Deactivate the 'DO' range and go to the 'END' clause.

         move.l   r_NextPos(a2),d0     ; 'END' position
         bclr     #NRFB_ACTIVE,r_Flags(a2)

         ; Install the new PC and set the 'MOVE' flag.

* ========================     NewPosition     =========================
* Registers:      D0 -- source position
NewPosition
         move.l   d0,ev_PC(a4)         ; new PC
         bset     #(EXB_MOVE-8),(STATUS+2)(a4)

NP001    rts

* ========================     rxInsITERATE     ========================
* Implements the 'ITERATE' and 'LEAVE' instructions.
rxInsITERATE
         movea.l  a0,a3                ; first token
         move.l   (a2),d2              ; top (inner) range
         moveq    #0,d5                ; clear flag
         subq.b   #TRACE_S,d7          ; 'SCAN' mode?

1$:      movea.l  d2,a2                ; current range
         move.l   (a2),d2              ; a successor?
         beq.s    2$                   ; no -- end scan

         move.b   r_Type(a2),d0        ; range type
         subq.b   #NRANGE_DO,d0        ; a 'DO' range?
         beq.s    3$                   ; yes
         subq.b   #NRANGE_INTERP-NRANGE_DO,d0
         bne.s    4$                   ; deactivate

         ; Unsuccessful scan termination ... determine the error code.

2$:      tst.w    d5                   ; 'DO' range found?
         bne      rxErr27              ; yes ... symbol mismatch
         bra      rxErr22

         ; A 'DO' range found ... see if it's iterative.

3$:      btst     #NRFB_ITERATE,r_Flags(a2)
         beq.s    4$                   ; ... not iterative
         moveq    #-1,d5               ; set 'found' flag
         tst.w    c_Count(a5)          ; a match symbol?
         beq.s    6$                   ; no

         ; Check if the symbols match

         move.l   r_Index(a2),d1       ; an index symbol?
         beq.s    4$                   ; no
         movea.l  TSTRING(a3),a0       ; symbol string
         movea.l  d1,a1                ; index string
         CALLSYS  CmpString            ; D0=booelan
         bne.s    5$                   ; ... matched

         ; Deactivate the range ...

4$:      tst.b    d7                   ; 'SCAN' mode?
         beq.s    1$                   ; yes -- do nothing
         bclr     #NRFB_ACTIVE,r_Flags(a2)
         bra.s    1$                   ; loop back

5$:      movea.l  (a3),a3              ; advance the "check" token

         ; Scan completed.  Check for errors.

6$:      cmpi.b   #T_END,t_Type(a3)    ; STOPTOKEN?
         bne      rxErr35              ; no

         tst.b    d7                   ; 'SCAN' mode?
         beq.s    RIBOut               ; yes ... all done

         cmpi.l   #KC_LEAVE,d3         ; 'LEAVE' instruction?
         beq.s    7$                   ; yes

         ; Processing for 'ITERATE' instruction.

         move.l   r_SrcPos(a2),d0      ; start of loop
         move.l   r_Segment(a2),d1     ; an 'UNTIL' instruction?
         beq.s    RIIMove              ; no

         ; An 'UNTIL' clause ... stack it for execution.

         bsr      StackUNTIL           ; stack clause
         bra.s    RIIPred              ; flag inner ranges

         ; Processing for 'LEAVE' instruction.

7$:      bclr     #NRFB_ACTIVE,r_Flags(a2)

         ; Check whether the end position is known ...

RIICheck move.l   r_NextPos(a2),d0     ; 'END' known?
         beq.s    RIBOut               ; no ...

RIIMove  bsr      NewPosition

         ; Set the 'FINISH' flag for all preceding ranges ...

RIIPred  movea.l  r_Pred(a2),a2        ; previous range
         tst.l    r_Pred(a2)           ; a predecessor?
         beq.s    RIBOut               ; no ...

         bset     #NRFB_FINISH,r_Flags(a2)
         bra.s    RIIPred              ; loop back

* ===========================     rxInsBREAK     ========================
* Implements the 'BREAK' instruction.
* [instruction module environment]
rxInsBREAK
         move.l   (a2),d2              ; top (innermost) range

         ; Scan through the control ranges,  deactivating as we go ...
         ; terminate if a 'DO' or 'INTERPRET' range is found.

1$:      movea.l  d2,a2                ; range to check
         move.l   (a2),d2              ; a successor?
         beq      rxErr22              ; no ... error

         ; Clear the 'ACTIVE' flag (unless in trace 'SCAN' mode).

         cmpi.b   #TRACE_S,d7          ; trace 'SCAN' mode?
         beq.s    2$                   ; yes
         bclr     #NRFB_ACTIVE,r_Flags(a2)

2$:      move.b   r_Type(a2),d0        ; range type
         subq.b   #NRANGE_DO,d0        ; a 'DO' range?
         beq.s    RIICheck             ; yes ... loop back
         subq.b   #NRANGE_INTERP-NRANGE_DO,d0
         bne.s    1$                   ; loop back

RIBOut   rts
