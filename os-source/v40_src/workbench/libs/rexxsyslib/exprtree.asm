* == exprtree.asm ======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     ExprTree     ==========================
* Builds a parse tree for an expression.  Private to 'ScanExpr'.
* Registers:   A4 -- environment
*              A5 -- clause
* Scratch:     D2-D6
ExprTree
         movem.l  a2/a3,-(sp)
         clr.l    -(sp)                ; clear slot
         movea.l  sp,a3                ; "last node"

         movea.l  CLINK(a5),a0         ; prior token
         movea.l  (a0),a1              ; first token
         btst     #TFB_ASSIGN,t_Flags(a1)
         beq.s    1$                   ; no assignment ...

         ; Unlink the assignment token and update the token chain ...

         move.l   a1,(a3)              ; save token
         movea.l  a1,a3                ; update "last"
         bsr      Remove               ; unlink it

         ; Build a parse tree for the expression.

1$:      moveq    #0,d0                ; nesting level = 0
         bsr.s    BuildTree            ; D0=error A0=root
         move.l   d0,d6                ; save error

         ; Rebuild the clause in postfix order ...

         move.l   a0,a2                ; root node or 0
         bra.s    5$                   ; jump in

2$:      clr.l    (a1)                 ; (BTRIGHT) clear slot

3$:      movea.l  d0,a1                ; current node
         move.l   BTLEFT(a1),d0        ; LEFT descendant?
         beq.s    4$                   ; no ...
         clr.l    BTLEFT(a1)           ; clear slot
         bra.s    3$                   ; loop back

4$:      move.l   (a1),d0              ; (BTRIGHT) RIGHT descendant?
         bne.s    2$                   ; yes

         ; Add the token to the end of the chain ...

         move.l   EXPTOKEN(a1),a0      ; load token
         move.l   a3,sn_Pred(a0)       ; predecessor
         move.l   a0,(a3)              ; successor
         movea.l  a0,a3                ; update last node

         ; Release the expression tree node

         movea.l  a4,a0                ; environment
         movea.l  BTPARENT(a1),a2      ; load parent
         bsr      FreeBTNode           ; release node

5$:      move.l   a2,d0                ; parent node exists?
         bne.s    3$                   ; yes -- loop back

         ; Relink the chain of tokens to the clause ...

         move.l   (sp)+,d0             ; pop first node
         beq.s    6$                   ; ... nothing to do
         movea.l  a3,a1                ; last node
         movea.l  CLINK(a5),a0         ; insertion point
         bsr      Relink               ; relink

6$:      move.l   d6,d0
         movem.l  (sp)+,a2/a3
         rts

* ==========================     BuildTree     =========================
* Builds a parse tree for an exression.  The tokens are unlinked from the
* clause and built into the tree.  Prefix operators are recognized at this
* point.  Private to 'ExprTree'.
* Registers:   A4 -- storage environment
*              A5 -- clause
*              D0 -- nesting level
* Return:      D0 -- error code or 0
*              A0 -- root of tree
PREVTERM SET      15
BuildTree
         movem.l  d2-d7/a2/a3,-(sp)
         move.l   a4,d2                ; environment

         suba.l   a4,a4                ; clear root node

         move.l   d0,d3                ; nesting level
         move.b   #NE_MAXP,d4          ; root priority
         move.b   d4,d5                ; branch priority
         moveq    #0,d7                ; flags register

         moveq    #ERR10_043,d0        ; "nested too deeply"
         cmp.l    rl_MaxNest(a6),d3    ; nesting level ok?
         bhi      ETOut                ; no ... error

ETScan   movea.l  CLINK(a5),a0         ; token fence
         movea.l  (a0),a3              ; first token
         move.b   t_Type(a3),d0        ; token type

         subq.b   #T_STRING,d0         ; a string?
         beq.s    1$                   ; yes
         subq.b   #T_OPERATOR-T_STRING,d0
         beq.s    2$                   ; an operator
         subq.b   #T_OPEN-T_OPERATOR,d0; start of sub-expression?
         beq.s    ETSubExpr            ; yes ...
         subq.b   #T_CLOSE-T_OPEN,d0   ; end of sub-expression?
         beq      ETClose              ; yes ... all done
         bra      ETEnd

         ; A string token ... flag as a term.

1$:      moveq    #NE_TERM,d6          ; a term
         move.w   #(1<<PREVTERM)!NE_SYMP,d7
         bra.s    5$                   ; get a new node

         ; An operator token

2$:      moveq    #NE_OPER,d6          ; an operator token
         tst.w    d7                   ; previous token a term?
         bmi.s    4$                   ; yes ...

         ; A prefix operation ... transform opcode as required.

         move.b   TOPCODE(a3),d0       ; load opcode
         subi.b   #OP_SUB,d0           ; subtraction operation?
         beq.s    3$                   ; yes ...
         subq.b   #(OP_ADD-OP_SUB),d0  ; addition operation?
         bne.s    4$                   ; no

3$:      addi.b   #(OP_NEG-OP_SUB),TOPCODE(a3)

4$:      clr.w    d7                   ; clear flag
         move.b   TOPCODE(a3),d7       ; install opcode
         lsr.b    #OPERSHFT,d7         ; ... operator priority

         ; Allocate a new node for the tree

5$:      movea.l  d2,a0                ; environment
         bsr      GetBTNode            ; D0=node
         movea.l  a3,a1                ; token
         movea.l  d0,a3                ; new node
         move.b   d6,EXPTYPE(a3)       ; node type
         move.b   d7,EXPPRIOR(a3)      ; node priority
         move.l   a1,EXPTOKEN(a3)      ; token
         bsr      Remove               ; unlink token
         bra.s    ET_Link

         ; Start of a sub-expression ... release the token.

ETSubExpr
         moveq    #NE_SUBT,d6          ; a subtree
         move.w   #(1<<PREVTERM)!NE_SYMP,d7

         movea.l  d2,a0                ; environment
         movea.l  a5,a1                ; clause
         move.l   a3,d0                ; token
         bsr      FreeToken            ; release it

         ; Recursive call for sub-expression ...

         move.l   d3,d0                ; nesting level 
         addq.w   #1,d0                ; ... plus one
         exg      d2,a4                ; environment=>A4
         bsr      BuildTree            ; D0=error A0=root
         exg      d2,a4                ; environment=>D2, root=>A4
         bne      ETOut                ; error ...
         movea.l  a0,a3                ; save root node

         ; Now to determine the linkage ...

ET_Link  cmp.b    d4,d7                ; add at the top?
         bhi.s    2$                   ; no ...
         move.l   a4,d0                ; root defined yet?
         beq.s    1$                   ; no ...
         movea.l  a3,a0                ; yes -- link root to new node
         movea.l  a4,a1                ; root node
         moveq    #EXPLEFT,d0          ; mode switch
         bsr      LinkBTNode

1$:      move.b   d7,d4                ; new root priority
         movea.l  a3,a4                ; new root node
         bra.s    6$

2$:      cmp.b    d5,d7                ; greater than branch priority?
         bhi.s    4$                   ; yes ...

         ; Scan up the tree for a lower priority node

3$:      move.l   BTPARENT(a2),d0      ; scan up for lower priority
         beq.s    ETErr41              ; no parent? ... error
         movea.l  d0,a2                ; next branch
         cmp.b    EXPPRIOR(a2),d7
         bls.s    3$                   ; loop back?

4$:      move.l   (a2),d0              ; (BTRIGHT) descendant node?
         beq.s    5$                   ; no ...
         cmpi.b   #NE_OPER,EXPTYPE(a2) ; an operator?
         bne.s    ETErr41              ; no -- error

         ; Move current descendant node to LEFT branch of new node

         clr.l    (a2)                 ; (BTRIGHT)
         movea.l  a3,a0                ; new node
         movea.l  d0,a1                ; current descendent
         moveq    #EXPLEFT,d0          ; left branch
         bsr      LinkBTNode

         ; Link new node to RIGHT branch of current node

5$:      move.l   a2,a0                ; current branch
         movea.l  a3,a1                ; new node
         moveq    #EXPRIGHT,d0         ; right branch
         bsr      LinkBTNode           ; link it in

         ; An operator?  Update branch node and priority ...

6$:      cmpi.b   #NE_OPER,d6          ; an operator?
         bne.s    7$                   ; no ...
         move.b   d7,d5                ; new branch priority
         movea.l  a3,a2                ; new branch node

7$:      bra      ETScan               ; loop back for next token

         ; Closing parenthesis ... make sure it's a nested call.

ETClose  tst.w    d3                   ; nested call?
         beq.s    ETErr42              ; no ... error

         movea.l  d2,a0                ; environment
         movea.l  a5,a1                ; clause
         move.l   a3,d0                ; this token
         bsr      FreeToken            ; release it
         bra.s    ETDone

         ; A 'TERM' or 'END' token ... make sure we're not nested.

ETEnd    subq.b   #T_END-T_CLOSE,d0    ; a STOPTOKEN?
         beq.s    1$                   ; yes -- all done
         subq.b   #T_TERM-T_END,d0     ; a separator?
         bne.s    ETErr41              ; no ... invalid token

1$:      tst.w    d3                   ; nested call?
         beq.s    ETDone               ; no

         ; Error return codes ...

ETErr42  moveq    #ERR10_042,d0        ; unbalanced parentheses
         bra.s    ETOut

ETErr41  moveq    #ERR10_041,d0        ; invalid expression
         bra.s    ETOut

ETDone   move.l   a4,d0                ; a root node?
         bne.s    1$                   ; yes
         bset     #CFB_NULLEXPR,c_Flags(a5)

1$:      moveq    #0,d0                ; clear error

ETOut    movea.l  a4,a0                ; root node
         movea.l  d2,a4                ; restore environment
         movem.l  (sp)+,d2-d7/a2/a3
         rts

* ===========================     EvalExpr     =========================
* Evaluates an ordered expression.  The result remains in the terminating
* token or is installed in the symbol table.  Private to 'ScanExpr'.
* Registers:      A0 -- first token
*                 A4 -- environment
*                 A5 -- clause
*                 D7 -- trace option
* Return:         D0 -- error
* Scratch:        D2-D6
SCANBIT  SET      15                   ; scan mode? (sign bit)
TRACEBIT SET      31                   ; trace result? (sign bit)
EvalExpr
         movem.l  a2/a3,-(sp)
         moveq    #0,d3                ; operand count
         moveq    #0,d5                ; trace flags

         ; Determine the tracing modes ...

         moveq    #TRACEBIT,d1         ; load flag
         cmpi.b   #TRACE_I,d7          ; trace 'Intermediates'?
         beq.s    1$                   ; yes

         moveq    #SCANBIT,d1          ; load flag
         cmpi.b   #TRACE_S,d7          ; trace 'SCAN'?
         bne.s    2$                   ; no

1$:      bset     d1,d5                ; set flag

         ; Check whether the first token is an assignment ...

2$:      move.l   a0,d2                ; first token
         btst     #TFB_ASSIGN,t_Flags(a0)
         sne      d5                   ; set flag
         beq.s    EEScan               ; ... no assignment
         move.l   (a0),d2              ; advance to next token

         ; Scan the clause.  Valid token types are STRING, OPERATOR, and
         ; STOPTOKEN.

EEScan   movea.l  d2,a2                ; scan token
         move.l   (a2),d2              ; next token

         move.b   t_Type(a2),d0        ; token type
         addq.w   #1,d3                ; preincrement count
         subq.b   #T_STRING,d0         ; an operand?
         beq.s    EEScan               ; yes

         suba.l   a3,a3                ; clear operand token
         subq.b   #T_OPERATOR-T_STRING,d0
         beq.s    2$                   ; ... an operator

         subq.w   #1,d3                ; restore count
         subq.b   #T_END-T_OPERATOR,d0 ; the STOPTOKEN?
         beq.s    0$                   ; yes

         subq.b   #T_TERM-T_END,d0     ; a separator?
         bne      EEErr41              ; no -- invalid token

         ; STOPTOKEN or terminator found ... load the final result and
         ; make sure that the operand count is zero.

0$:      move.l   d3,d6                ; any operands?
         beq.s    1$                   ; no
         subq.w   #1,d3                ; one operand?
         bne      EEErr41              ; no -- error

         movea.l  t_Pred(a2),a3        ; previous token
         move.l   TSTRING(a3),d6       ; load result
         clr.l    TSTRING(a3)          ; clear slot

1$:      moveq    #ACT_RESULT,d4       ; trace action
         cmpi.b   #TRACE_R,d7          ; trace (final) results?
         bne.s    5$                   ; no
         bset     #TRACEBIT,d5         ; yes  -- set flag
         bra.s    5$

         ; An operator found.  Load the operands and evaluate it.

2$:      subq.w   #1,d3                ; restore count
         beq      EEErr41              ; error ... no operands

         moveq    #ACT_INTP,d4         ; assume a monadic operator
         movea.l  t_Pred(a2),a0        ; right operand token
         move.l   TSTRING(a0),d1       ; right operand
         move.b   TOPCODE(a2),d0       ; load opcode
         cmpi.b   #OP_NEG,d0           ; prefix operator?
         bcc.s    3$                   ; yes

         ; A dyadic operation ... fetch the left operand and decrement the
         ; operand count.

         subq.w   #1,d3                ; at least two operands?
         beq      EEErr41              ; no -- error
         moveq    #ACT_INTO,d4         ; trace (dyadic) operation
         movea.l  t_Pred(a0),a3        ; left operand token
         movea.l  TSTRING(a3),a1       ; ... left operand

         ; Transform the operator into a 'STRING' token and clear slot.

3$:      move.w   #T_STRING<<8!0,t_Type(a2)
         clr.l    TSTRING(a2)          ; clear slot

         ; Evaluate the operation, if not in 'SCAN' mode.

         tst.w    d5                   ; scan mode?
         bmi.s    4$                   ; yes
         movea.l  a4,a0                ; environment
         CALLSYS  EvalOp               ; D0=error  D1=result
         bne      EEOut                ; ... error
         move.l   d1,d6                ; save result

         ; Recycle the right operand token ...

4$:      move.l   t_Pred(a2),d0        ; right operand
         bsr      QFreeToken           ; release it

         ; Check whether we're tracing (intermediates).  If so, make sure
         ; the token is in string format.

         tst.l    d5                   ; tracing?
         bpl.s    6$                   ; no ...

         ; Check whether the result string needs to be converted.

5$:      tst.l    d6                   ; a result?
         beq.s    6$                   ; no
         movea.l  d6,a1                ; operand string
         btst     #NSB_STRING,ns_Flags(a1)
         bne.s    6$                   ; ... already a string

         ; Convert the result to a string ... always returns a new string.

         movea.l  a4,a0                ; environment
         bsr      CVd2s                ; D0=A0=string
         exg      d0,d6                ; old=>D0, new=>D6

         ; Release the intermediate string structure.

         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; old string
         bsr      FreeString           ; release it

         ; Install the result string and check whether to trace result.

6$:      move.l   d6,TSTRING(a2)       ; install result

         tst.l    d5                   ; trace the result?
         bpl.s    7$                   ; no
         movea.l  a4,a0                ; environment
         movea.l  d6,a1                ; string
         move.l   d4,d1                ; trace action
         bsr      rxTrace

         ; Release the (optional) operand token ...

7$:      move.l   a3,d0                ; optional operand?
         beq.s    8$                   ; no
         bsr      QFreeToken           ; ... release it

8$:      tst.w    d3                   ; any more operands?
         bne      EEScan               ; yes -- loop back

         ; Scan completed ... check for errors and return the result.

EECheck  move.l   d6,d0                ; a result?
         beq.s    1$                   ; no

         ; Make sure the result is not volatile, making a copy if necessary.

         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; result string
         bsr      MayCopyString        ; D0=A1=string

1$:      move.l   d0,TSTRING(a2)       ; store result

         tst.b    d5                   ; an assignment?
         beq.s    3$                   ; no

         ; Install the result in the assignment symbol ...

         lea      cTL_Head(a5),a3      ; list header
         movea.l  a4,a0                ; environment
         movea.l  (a3),a1              ; assignment token
         moveq    #0,d1                ; (no node)
         CALLSYS  AssignValue          ; D0=error A0=node
         bne.s    EEOut                ; ... error

         clr.l    TSTRING(a2)          ; clear slot

         ; Check the instruction 'EMPTY' bit ...

         btst     #(IPB_EMPTY-16),(c_Keys+1)(a5)
         bne.s    2$                   ; ... bit set

         ; 'EMPTY' not set ... move the token to the end of the clause.

         movea.l  a3,a0                ; list header
         bsr      RemHead              ; D0=assignment token
         movea.l  a3,a0                ; list header
         movea.l  d0,a1                ; token
         bsr      AddTail              ; ... relink it
         bra.s    3$

         ; Instruction 'EMPTY' bit is set ... free the token.

2$:      move.l   (a3),d0              ; assignment token
         bsr      QFreeToken           ; release it

3$:      moveq    #0,d0                ; clear error
         bra.s    EEOut

         ; Error return ...

EEErr41  moveq    #ERR10_041,d0        ; invalid expression

EEOut    movem.l  (sp)+,a2/a3
         rts
