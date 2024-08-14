* == cvs2d.asm =========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ===========================     ScanExpr     =========================
* Analyzes expressions and creates FUNCtion clauses.  Private to 'rxParse'.
* Registers:   A4 -- environment
*              A5 -- clause
*              D7 -- trace option
SYMBMASK SET      1<<T_SYMSTEM!1<<T_SYMCMPD!1<<T_SYMVAR!1<<T_SYMFIXED
TESTMASK SET      SYMBMASK!1<<T_FUNCTION!1<<T_END!1<<T_COMMA!1<<T_STRING
ScanExpr
         movem.l  d2-d6/a2/a3,-(sp)

         ; Check whether to skip the first token ...

SERestart
         movea.l  cTL_Head(a5),a3      ; first token
         btst     #(ISB_SKIP1ST-8),(c_Keys+2)(a5)
         beq.s    SEPreScan            ; no skip
         movea.l  (a3),a3              ; ... advance token

         ; Find the first token of an expression.  Preceding TERM tokens
         ; are skipped and blank tokens are removed.  After the starting
         ; token is located, the remaining tokens are scanned for function
         ; definitions.

SEPreScan
         moveq    #0,d2                ; clear start token
         moveq    #0,d3                ; clear 'function' flag
         move.w   #1<<T_END!1<<T_COMMA,d5
         bra.s    2$                   ; jump in

         ; Token not needed ... release it.

1$:      move.l   a2,d0                ; token
         bsr      QFreeToken           ; release it

         ; Start the prescan of the token list ...

2$:      movea.l  a3,a2                ; token to check
         movea.l  (a2),a3              ; next token
         move.b   t_Type(a2),d0        ; load type
         move.b   t_Flags(a2),d1       ; load flags

         tst.l    d2                   ; expression started yet?
         bne.s    5$                   ; yes ...

         ; Check for pre-expression tokens ...

         cmpi.b   #T_TERM,d0           ; a TERM token?
         beq.s    2$                   ; yes -- skip it
         btst     #TFB_BLANK,d1        ; a blank?
         bne.s    1$                   ; yes ... release it

         ; Start of expression ... save first token.  If the scan is being
         ; resumed after a function evaluation, skip ahead to scan token.

         move.l   a2,d2                ; first token of expression
         move.l   CLINK(a5),d6         ; resume scan?
         beq.s    3$                   ; no
         movea.l  d6,a2                ; scan token
         movea.l  (a2),a3              ; next token
         bra.s    2$                   ; loop back

3$:      move.l   a2,d6                ; first token to scan
         btst     #TFB_ASSIGN,d1       ; an assignment symbol?
         beq.s    4$                   ; no
         move.l   a3,d6                ; ... use next token

         ; Check the clause 'ANYFUNC' flag ... if there are no functions,
         ; proceed directly to symbol resolution.

4$:      btst     #CFB_ANYFUNC,c_Flags(a5)
         beq.s    SEResolve            ; ... no functions

         ; Expression started ... scan until FUNCDEF, COMMA, or STOPTOKEN.

5$:      btst     #TFB_FUNCDEF,d1      ; a function token?
         sne      d3                   ; set flag
         bne.s    SEResolve            ; end prescan

         btst     d0,d5                ; 'COMMA' or 'END'?
         beq.s    2$                   ; no ... loop back

         ; Scan the expression to resolve symbols and function calls.
         ; Resolution proceeds from left-to-right and terminates if
         ; a function call is found.

SEResolve
         movea.l  d6,a3                ; start of scan
         move.l   #TESTMASK,d5         ; load selection mask
         moveq    #0,d6                ; clear error

1$:      movea.l  a3,a2                ; scan token
         move.l   (a2),a3              ; next token
         move.b   t_Type(a2),d0        ; token type
         btst     d0,d5                ; selected token?
         beq.s    1$                   ; no

         btst     #TFB_FUNCDEF,t_Flags(a2)
         bne      SEFunc               ; ... function call

         btst     #TTB_SYMBOL,d0       ; a (variable) symbol?
         bne.s    3$                   ; yes
         subq.b   #T_STRING,d0         ; a string?
         beq.s    2$                   ; yes
         subq.b   #T_COMMA-T_STRING,d0 ; a separator token?
         beq      7$                   ; yes
         subq.b   #T_END-T_COMMA,d0    ; the STOPTOKEN?
         beq      SELast               ; yes

         ; "Placeholder" token found ... install function result.

         moveq    #ACT_INTF,d1         ; "function result" action
         movea.l  ev_GlobalPtr(a4),a0  ; global pointer
         movea.l  EVNAME(a0),a1        ; result string
         clr.l    EVNAME(a0)           ; clear slot
         move.l   a1,TSTRING(a2)       ; a result?
         bne.s    21$                  ; yes

         ; No function result available ... check if a 'CALL' instruction.

         cmpi.b   #TRACE_S,d7          ; scan mode?
         beq.s    6$                   ; yes
         ori.b    #CFF_NULLEXPR!CFF_SIMPLE,c_Flags(a5)
         move.l   c_Keys(a5),d0        ; instruction code
         and.l    PRIMask(pc),d0       ; opcode
         cmp.l    CALLCode(pc),d0      ; a 'CALL' instruction?
         beq.s    6$                   ; yes
         moveq    #ERR10_016,d6        ; load error
         bra.s    6$

         ; Check if an intermediate result should be traced ...

2$:      moveq    #ACT_INTL,d1         ; "literal" trace action

21$:     cmpi.b   #TRACE_I,d7          ; trace intermediates?
         bne.s    6$                   ; no
         movea.l  a4,a0                ; environment
         movea.l  TSTRING(a2),a1       ; load string
         bsr      rxTrace              ; trace it
         bra.s    6$

         ; A symbol token ... check whether fixed or variable.

3$:      cmpi.b   #T_SYMFIXED,d0       ; fixed symbol?
         beq.s    2$                   ; yes ... consider as string

         ; A variable symbol ... look up the value if not in 'SCAN' mode.

         cmpi.b   #TRACE_S,d7          ; trace 'SCAN' mode?
         beq.s    6$                   ; yes ... skip resolution

         ; Get the symbol's value ... SIGNAL ON NOVALUE may force error.

         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; symbol token
         moveq    #0,d0                ; (no node)
         CALLSYS  LookUpValue          ; D0=error A0=node A1=string
         move.l   d0,d6                ; an error?
         bne.s    6$                   ; yes ...

         ; Check whether the result is a 'private' string.  If so, we may
         ; need to make a copy of the result ... if a function is called,
         ; the side effects may include dropping symbol values.  If the
         ; string is private and we don't make a copy, the token "no string"
         ; flag is set to prevent recycling (subtle point).

         btst     #NSB_KEEP,ns_Flags(a1)
         beq.s    5$                   ; ... not private
         tst.b    d3                   ; a function definition?
         beq.s    4$                   ; no

         ; Sigh ... better make a copy of the result ...

         movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=string
         bra.s    5$

         ; Set 'NOSTR' flag to prevent recycling ...

4$:      bset     #TFB_NOSTR,t_Flags(a2)

5$:      movea.l  TSTRING(a2),a0       ; load old string
         cmpa.l   a0,a1                ; same string?
         beq.s    6$                   ; yes
         move.l   a1,TSTRING(a2)       ; ... install new string
         movea.l  a0,a1                ; old string
         movea.l  a4,a0                ; environment
         bsr      FreeString           ; release old string

         ; Reclassify as a TSTRING token, but leave token flags intact.

6$:      move.b   #T_STRING,t_Type(a2) ; 'STRING' token
         move.l   d6,d0                ; an error?
         beq      1$                   ; no ... loop back
         bra      SEError

         ; A separator found ... check for multiple expressions.

7$:      addq.b   #T_TERM-T_COMMA,t_Type(a2)
         btst     #(IPB_MULTEVAL-16),(c_Keys+1)(a5)
         bne.s    SEEval               ; ... further scanning required

         ; Last expression in this clause ... set 'CFB_SIMPLE' flag.

SELast   bset     #CFB_SIMPLE,c_Flags(a5)

         ; Expression scan completed ... check for special cases.

SEEval   cmp.l    a2,d2                ; null expression?
         bne.s    1$                   ; no

         ; A null expression ... set flag.

         bset     #CFB_NULLEXPR,c_Flags(a5)
         bra.s    SENext               ; next expression

         ; Check for a one-operand expression.

1$:      movea.l  d2,a0                ; start token
         move.l   t_Pred(a2),d1        ; last expression token
         cmp.l    d2,d1                ; one operand?
         bne.s    3$                   ; no ...
         cmpi.b   #T_STRING,t_Type(a0) ; a string token?
         bne.s    4$                   ; no ... error case?

         ; Only one operand ... copy the result string if it's owned,
         ; and install it in the terminal token.  Result may be NULL
         ; in the case of a CALL instruction with no function result.

         move.l   TSTRING(a0),d0       ; a result string?
         beq.s    2$                   ; no ...

         clr.l    TSTRING(a0)          ; clear slot
         movea.l  a4,a0                ; storage environment
         movea.l  d0,a1                ; result string
         bsr      MayCopyString        ; D0=A1=string
         move.l   a1,TSTRING(a2)       ; install it

         ; Trace the result, if requested.

         cmpi.b   #TRACE_R,d7          ; trace results?
         bne.s    2$                   ; no ...
         movea.l  a4,a0                ; environment
         moveq    #ACT_RESULT,d1       ; trace action
         bsr      rxTrace              ; trace it

         ; Release the start token.

2$:      move.l   d2,d0                ; token
         bsr      QFreeToken           ; release it
         bra.s    SENext               ; next expression

         ; A multi-operand expression.  Check for an (assignment,value)
         ; case ... no need to build a parse tree.

3$:      btst     #TFB_ASSIGN,t_Flags(a0)
         beq.s    4$                   ; ... not an assignment
         cmp.l    (a0),d1              ; one operand?
         beq.s    5$                   ; yes

         ; Build a parse tree for the expression ...

4$:      movea.l  t_Pred(a0),a2        ; prior token
         move.l   a2,CLINK(a5)         ; token fence
         bsr      ExprTree             ; D0=error
         bne.s    SEError              ; error ...
         movea.l  (a2),a0              ; first token

         ; Call the expression evaluator ... registers D2-D6 are scratch.

5$:      bsr      EvalExpr             ; D0=error
         bne.s    SEError              ; ... error

         ; Check whether to loop back to scan further ... A3 is scan token.

SENext   btst     #CFB_SIMPLE,c_Flags(a5)
         bne.s    SEExit               ; ... all done
         clr.l    CLINK(a5)            ; clear slot
         bra      SEPreScan            ; loop back

SEErr42  moveq    #ERR10_042,d0        ; "unbalanced parentheses"

         ; An error occurred ... flag clause.

SEError  movea.l  a5,a0                ; clause
         bsr      ErrClause            ; flag an error

SEExit   movem.l  (sp)+,d2-d6/a2/a3
         rts

         ; Constructs a FUNCtion call instruction from an expression.
         ; Tokens are unlinked from the parent clause and added to the
         ; new clause, beginning in the present scan position.

SEFunc   move.l   a2,CLINK(a5)         ; save scan token

         ; Allocate the new clause structure.

         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; parent clause
         bsr      NewClause            ; D0=A1=new clause
         movea.l  d0,a5                ; save clause

         ; Push the new clause into the execution pipeline ...

         lea      ev_EStack(a4),a0     ; execution stack
         bsr      PushTail             ; stack it

         move.w   #C_INSTRUCT<<8!CFF_FINAL!CFF_PARSED,c_Type(a5)
         move.l   #KTF_FUNC,c_Keys(a5) ; instruction code

         ; Make sure there's a "placeholder" token for the parent clause.
         ; If possible, this is done by moving the function token forward
         ; to the blank or open parenthesis following the token.

         moveq    #0,d4                ; nesting count
         movea.l  (a2),a0              ; next token
         btst     #TFB_BLANK,t_Flags(a0)
         bne.s    2$                   ; ... a blank
         cmpi.b   #T_OPEN,t_Type(a0)   ; an OPEN parenthesis?
         beq.s    1$                   ; yes

         ; No placeholder token available ... allocate a new one.

         movea.l  a4,a0                ; environment
         suba.l   a1,a1                ; delayed linkage
         bsr      AddToken             ; D0=A0=token

         ; Insert it into the parent clause following the FUNCDEF token.

         movea.l  a2,a0                ; insertion point
         movea.l  d0,a1                ; first and last token
         bsr      Relink               ; insert it
         move.l   (a2),a0              ; next token
         bra.s    2$

1$:      moveq    #-1,d4               ; need a 'CLOSE'
         addq.w   #2,d4                ; initial nesting

         ; Move the FUNCDEF token forward and redefine as a placeholder.

2$:      move.l   a0,d2                ; save token
         addq.w   #t_Type,a0           ; data area
         addq.w   #t_Type,a2           ; data area
         move.w   (a2),(a0)+           ; copy t_Type/t_Flags
         move.w   #T_FUNCTION<<8,(a2)+ ; load t_Type/t_Flags
         move.w   (a2)+,(a0)+          ; copy t_Offset
         move.l   (a2),(a0)            ; copy t_Data
         clr.l    (a2)                 ; ... clear slot

         ; Now scan the list looking for OPEN, CLOSE, and STOPTOKENs.

         movea.l  d2,a1                ; scan token
         moveq    #1<<T_OPEN!1<<T_CLOSE,d6
         bra.s    5$                   ; jump in

3$:      subq.b   #T_OPEN,d0           ; an OPEN parenthesis?
         beq.s    4$                   ; yes
         subq.w   #1,d4                ; no ... decrement nesting
         bne.s    5$                   ; ... still nested

         tst.l    d4                   ; need a CLOSE?
         bpl.s    5$                   ; no
         addq.b   #T_END-T_CLOSE,t_Type(a1)
         bra.s    7$                   ; all done

4$:      addq.w   #1,d4                ; increment nesting

5$:      move.l   a1,d3                ; save last token
         movea.l  (a1),a1              ; next token
         move.b   t_Type(a1),d0        ; token type
         btst     d0,d6                ; nesting token?
         bne.s    3$                   ; yes ... loop back

         ; Check for a FUNCdef token ... set clause flag.

         btst     #TFB_FUNCDEF,t_Flags(a1)
         beq.s    6$                   ; ... not a function
         bset     #CFB_ANYFUNC,c_Flags(a5)

6$:      subq.b   #T_END,d0            ; the STOPTOKEN?
         bne.s    5$                   ; no ... loop back

         ; End of parent list ... add a STOPTOKEN to the new list.

         bsr      EndToken             ; get a STOPTOKEN
         movea.l  d3,a1                ; restore last token

         ; Unlink from the old clause and relink in the new.

7$:      movea.l  d2,a0                ; first token
         bsr      Unlink               ; D0=first A1=last
         lea      cTL_Head(a5),a0      ; insertion point
         bsr      Relink               ; relink

         ; Check for errors and loop back to scan new clause.

         move.w   d4,d0                ; nested?
         beq      SERestart            ; no ... loop back
         bra      SEErr42              ; error ... "unbalanced parentheses"
