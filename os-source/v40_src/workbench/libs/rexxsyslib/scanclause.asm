* == scanclause.asm ====================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     ScanClause     =========================
* Second-stage parser for instruction clauses.  Checks for proper structure
* and builds the instruction code.  If an error is found, the clause is
* reclassified as an 'ERROR' clause.  Private to 'rxParse'.
* Registers:   A4 -- storage environment
*              A5 -- clause
NOTFIRST SET      15                   ; (sign bit)
REMTOKEN SET      16
ScanClause
         movem.l  d2-d4/a2/a3,-(sp)
         movea.l  cTL_Head(a5),a3      ; first token

         moveq    #0,d2                ; flags register
         move.l   c_Keys(a5),d3        ; instruction code
         move.l   PRIMask(pc),d4       ; opcode mask
         and.l    d3,d4                ; primary opcode

         ; Pre-scan check for 'DO' instructions:  see if an initializer
         ; expression is given.  If so, flag it and advance the pointer.

         cmp.l    DOCode(pc),d4        ; a 'DO' clause?
         bne.s    SCScan               ; no
         move.b   t_Type(a3),d0        ; token type
         cmpi.b   #T_END,d0            ; STOPTOKEN?
         beq.s    SCScan               ; yes

         ; Turn on the instruction cache if it's enabled ...

         btst     #EFB_CACHE,EVFLAGS(a4)
         beq.s    1$                   ; ... not enabled
         bset     #(EXB_CACHE-16),(STATUS+1)(a4)

1$:      btst     #TTB_SYMBOL,d0       ; a SYMBOL token?
         beq.s    SCScan               ; no ...
         cmpi.b   #T_SYMFIXED,d0       ; a fixed SYMBOL?
         beq.s    SCScan               ; yes ...
         movea.l  (a3),a2              ; the next token
         btst     #TFB_EQUALS,t_Flags(a2)
         beq.s    SCScan               ; ... not an '=' sign

         ; An initializer expression found.  Set the instruction code, save
         ; the symbol string, and delete the '=' token ...

         bset     #TFB_ASSIGN,t_Flags(a3)
         ori.w    #(ISF_EVAL!KT4_INIT),CSECKEYS(a5)
         movea.l  (a2),a3              ; advance starting pointer

         ; Remove the current token ...

SCRemove move.l   a2,d0                ; this token
         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      FreeToken            ; release it

         ; Begin the scanning loop ...

SCScan   movea.l  a3,a2                ; current test token
         movea.l  (a2),a3              ; next token to test
         move.l   c_Keys(a5),d3        ; current instruction code
         btst     #ISB_NOSCAN,d3       ; scanning completed?
         bne      SCEnd                ; yes ...
         move.b   t_Type(a2),d0        ; load token type 
         cmpi.b   #T_END,d0            ; a STOPTOKEN?
         beq      SCEnd                ; yes -- all done

         move.b   t_Flags(a2),d1       ; load token flags
         btst     #ISB_NOBLANK,d3      ; 'NOBLANK' mode?
         beq.s    1$                   ; no ...

         ; Processing for 'NOBLANK' mode:  blanks removed, but all other
         ; tokens passed through.

         btst     #TFB_BLANK,d1        ; this token a blank?
         bne.s    SCRemove             ; yes ... remove it
         bra.s    SCScan               ; loop back

1$:      btst     #ISB_SYMLIST,d3      ; symbol list processing?
         beq.s    2$                   ; no ...

         ; Processing for 'SYMLIST' mode:  Variable symbols accepted,
         ; blanks deleted, and anything else is an error ...

         btst     #TFB_BLANK,d1        ; a blank?
         bne.s    SCRemove             ; yes -- remove it
         btst     #TTB_SYMBOL,d0       ; a symbol?
         beq      SCErr31              ; no -- error
         cmpi.b   #T_SYMFIXED,d0       ; a fixed symbol?
         beq      SCErr40              ; yes -- error
         addq.w   #1,c_Count(a5)       ; increment symbol count
         bra.s    SCScan               ; loop back

2$:      btst     #TFB_BLANK,d1        ; a blank token?
         bne.s    SCScan               ; yes ... loop back

         btst     #IPB_MULTEVAL,d3     ; looking for multiple expressions?
         beq.s    3$                   ; no ...
         move.l   d3,d1
         andi.l   #(IPF_EVAL!ISF_EVAL),d1
         sne      d2                   ; set 'MULTIPLE' flag

3$:      cmpi.b   #T_STRING,d0         ; a STRING token?
         beq.s    5$                   ; yes ...
         cmpi.b   #T_SYMFIXED,d0       ; a fixed SYMBOL?
         beq.s    5$                   ; yes ...
         btst     #TTB_SYMBOL,d0       ; a SYMBOL token?
         beq.s    6$                   ; no ...

         ; Check for a secondary keyword token.

         movea.l  TSTRING(a2),a0       ; symbol
         move.l   d4,d0                ; primary opcode
         bsr      SecCode              ; D0=instruction code
         beq.s    5$                   ; ... not found

         ; A keyword token found ... check for exceptions.

         btst     #ISB_FIRST,d0        ; must be first sub-keyword?
         beq.s    31$                  ; no ...
         tst.w    d2                   ; was it the first?
         bmi.s    5$                   ; no -- not a keyword
         bra.s    4$

         ; Symbol is not flagged as 'FIRST'.  If we're looking for the
         ; first symbol/string and haven't found it yet, and this symbol
         ; is terminal, then it's not a keyword.

31$:     btst     #IPB_FIRSTSYM,d3     ; looking for first?
         beq.s    4$                   ; no ...
         btst     #ISB_FIRST,d3        ; found it yet?
         bne.s    4$                   ; yes ...
         btst     #ISB_NOSCAN,d0       ; terminal symbol?
         bne.s    5$                   ; yes -- not a keyword

         ; Check if both ISB_EVAL and ISB_NOEVAL set.  If so, it's not
         ; a keyword.  This allows keywords in one context to be used as
         ; ordinary symbols in an expression.

4$:      btst     #ISB_NOEVAL,d0       ; suppress evaluation?
         beq.s    7$                   ; no ...
         btst     #ISB_EVAL,d3         ; secondary (conditional) evaluation?
         beq.s    7$                   ; no ...

         ; Check for the first symbol (or string).

5$:      btst     #IPB_FIRSTSYM,d3     ; looking for first symbol?
         beq.s    6$                   ; no ...
         tst.w    d2                   ; first symbol/string?
         bmi.s    6$                   ; no ...

         ; First symbol/string processing:  clear the primary 'EVAL' flag,
         ; set 'SKIP1ST', and terminate scan.

         bclr     #IPB_EVAL,d3         ; suppress evaluation
         ori.w    #(ISF_SKIP1ST!ISF_NOSCAN),d3
         move.l   d3,c_Keys(a5)
         bra.s    SC_RemB

         ; Check whether to continue scanning.  Unless multiple expressions
         ; are being processed, the scan stops at the first exception.

6$:      tst.b    d2                   ; multiple expressions?
         bne      SCScan               ; yes -- loop back
         bra.s    SCEnd                ; no -- all done

         ; Sub-keyword processing:  the symbol has been accepted as a 
         ; keyword.  Check for conflicts with previous keywords.

7$:      move.w   d3,d1                ; current instruction subword
         andi.w   #KTF_SECMASK,d1      ; current sub-option
         and.w    d0,d1                ; keyword conflict?
         bne      SCErr36              ; yes ...

         or.w     d0,CSECKEYS(a5)      ; OR it in ...
         moveq    #REMTOKEN,d1         ; load flag
         bset     d1,d2                ; assume token not needed
         tst.b    d2                   ; multiple expressions?
         beq.s    SC_RemB              ; no ...

         ; Keep the token, redefining it as a separator (T_COMMA).  Save the
         ; subcode in the TOFFSET position, and release the string.

         bclr     d1,d2                ; keep the token
         move.w   #T_COMMA<<8!0,t_Type(a2)
         move.w   d0,TOFFSET(a2)       ; save the sub-keyword code

         cmp.l    DOCode(pc),d4        ; a 'DO' clause?
         bne.s    8$                   ; no ...

         ; Check for an 'UNTIL' and recode as a STOPTOKEN.

         cmpi.w   #KT4_UNTIL,d0        ; an 'UNTIL' symbol?
         bne.s    8$                   ; no ...
         addq.b   #T_END-T_COMMA,t_Type(a2)

8$:      movea.l  a4,a0                ; environment
         movea.l  TSTRING(a2),a1       ; keyword string
         bsr      FreeString           ; release it
         clr.l    TSTRING(a2)          ; clear slot

         ; Final processing for recognized symbols.  Set 'NOTFIRST' flag,
         ; and remove the preceding token if it's a blank.

SC_RemB  bset     #NOTFIRST,d2         ; not the first symbol/string
         movea.l  t_Pred(a2),a1        ; previous token
         tst.l    t_Pred(a1)           ; first token?
         beq.s    1$                   ; yes -- nothing to release

         btst     #TFB_BLANK,t_Flags(a1)
         beq.s    1$                   ; ... not a blank
         move.l   a1,d0                ; token
         movea.l  a4,a0                ; storage environment
         movea.l  a5,a1                ; clause
         bsr      FreeToken

1$:      bclr     #REMTOKEN,d2         ; remove current token?
         bne      SCRemove             ; yes ...
         bra      SCScan

         ; Token scan completed ... post-processing

SCEnd    cmp.l    CALLCode(pc),d4      ; a 'CALL' instruction?
         bne.s    1$                   ; no
         bclr     #ISB_SKIP1ST,d3      ; function name symbol/string?
         beq.s    SCErr32              ; no -- error
         movea.l  cTL_Head(a5),a0      ; function name token
         bset     #TFB_FUNCDEF,t_Flags(a0)

1$:      btst     #ISB_NOBLANK,d3      ; 'NOBLANK' set?
         beq.s    2$                   ; no ...
         bclr     #IPB_MULTEVAL,d3     ; no multiple expressions

2$:      btst     #IPB_REQDKEYW,d3     ; keyword required?
         beq.s    3$                   ; no ...
         move.w   #(ISF_NOSCAN!ISF_NOBLANK),d0
         and.w    d3,d0                ; terminal symbol?
         beq.s    SCErr34              ; no -- error

3$:      move.l   d3,c_Keys(a5)        ; save the final code
         bset     #CFB_PARSED,c_Flags(a5)
         bra.s    SCExit

         ; Error codes

SCErr31  moveq    #ERR10_031,d0        ; symbol expected
         bra.s    SCError

SCErr32  moveq    #ERR10_032,d0        ; function name not given
         bra.s    SCError

SCErr34  moveq    #ERR10_034,d0        ; required keyword not found
         bra.s    SCError

SCErr36  moveq    #ERR10_036,d0        ; keyword conflict
         bra.s    SCError

SCErr40  moveq    #ERR10_040,d0        ; invalid variable symbol

SCError  movea.l  a5,a0                ; error clause
         bsr      ErrClause            ; flag as an error

SCExit   movem.l  (sp)+,d2-d4/a2/a3
         rts

* ===========================     ScanDO     ===========================
* Final processing for DO instructions (called only if a range exists).
* Creates an increment expression if the range is active, and removes
* unneeded TO, BY, or FOR terms.  This routine is private to 'rxParse'.
* Registers:   A0 -- topmost range
*              A4 -- environment
*              A5 -- clause
ScanDO
         movem.l  d2-d6/a2,-(sp)

         ; Check for an initializer expression.

         moveq    #0,d2                ; clear start
         movea.l  cTL_Head(a5),a2      ; first token
         moveq    #(KT4_OPTMASK&KT4_INIT),d0
         and.w    CSECKEYS(a5),d0      ; an initializer?
         beq.s    2$                   ; no ...

         move.l   a2,d2                ; save start token
         move.l   r_BY(a0),d3          ; range BY value

         ; The initialization expression has the form "index = value" ...
         ; must transform it to "index = index + <BY value>".  Begin by
         ; looking up the value of the index symbol using the node pointer.

         move.l   r_Value(a0),d0       ; node or 0
         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; symbol token
         CALLSYS  LookUpValue          ; D0=error A0=node A1=string
         move.l   d0,d6                ; save error

         ; Check whether the value is the symbol string ... no recycling.

         cmpa.l   TSTRING(a2),a1       ; symbol string?
         bne.s    1$                   ; no
         bset     #TFB_NOSTR,t_Flags(a2)

         ; Create the increment expression in postfix order, transforming
         ; or creating tokens as required.

1$:      move.l   a1,d0                ; index value
         moveq    #T_STRING,d1         ; string token
         bsr.s    Q1QToken             ; A2=token

         move.l   d3,d0                ; BY result
         moveq    #T_STRING,d1         ; string token
         bsr.s    Q1QToken             ; A2=token

         move.b   #OP_ADD,d0           ; the '+' operator
         moveq    #T_OPERATOR,d1       ; operator token
         bsr.s    Q1QToken             ; A2=token
         movea.l  (a2),a2              ; ... next token

         ; Remove all unneeded tokens from the clause.  Scanning stops at
         ; the end of the clause or if a 'WHILE' token is found.  An 'UNTIL'
         ; token will have been recoded as a STOPTOKEN.

2$:      move.b   t_Type(a2),d0        ; token type
         subq.b   #T_END,d0            ; the STOPTOKEN?
         beq.s    4$                   ; yes ... all done

         addq.b   #T_END-T_COMMA,d0    ; 'COMMA' separator?
         bne.s    3$                   ; no
         cmpi.w   #KT4_WHILE,TKEYCODE(a2)
         beq.s    5$                   ; ... 'WHILE' keyword

3$:      move.l   a2,d0                ; current token
         movea.l  (a2),a2              ; successor token
         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      FreeToken            ; release token
         bra.s    2$                   ; ... loop back

         ; No more expressions ... set the clause 'SIMPLE' flag.

4$:      bset     #CFB_SIMPLE,c_Flags(a5)
         bra.s    6$

         ; A 'WHILE' separator ... transform to a 'TERM' token.

5$:      addq.b   #T_TERM-T_COMMA,t_Type(a2)

         ; Check whether to evaluate the increment expression.

6$:      tst.l    d2                   ; increment expression?
         beq.s    SDOOut               ; no ... all done

         ; Evalute the expression ... registers D2-D6 are scratch.

         move.l   d6,d0                ; error in lookup?
         bne.s    7$                   ; yes
         movea.l  d2,a0                ; starting token
         bsr      EvalExpr             ; D0=error
         beq.s    SDOOut

         ; An error occurred ... reclassify the clause.

7$:      movea.l  a5,a0                ; clause
         bsr      ErrClause

SDOOut   movem.l  (sp)+,d2-d6/a2
         rts

* ==========================     Q1QToken     ==========================
* Private routine to transform or allocate a token.  Transforms and reuses
* the next token unless it's an 'END' or 'WHILE' (T_COMMA).
* Registers:   D0 -- token data
*              D1 -- token type
*              A2 -- current token
*              A4 -- storage environment
*              A5 -- clause
* Return:      A2 -- new token
Q1QToken
         movea.l  d0,a1                ; save data
         movea.l  (a2),a0              ; next token
         move.b   t_Type(a0),d0        ; token type
         subq.b   #T_END,d0            ; END token?
         beq.s    3$                   ; yes
         addq.b   #T_END-T_COMMA,d0    ; COMMA token?
         bne.s    1$                   ; no ... reuse it
         cmpi.w   #KT4_WHILE,TKEYCODE(a0)
         beq.s    3$                   ; ... 'WHILE' token

         ; Not a terminator ... transform the existing token.

1$:      movea.l  a0,a2                ; next token
         move.b   d1,t_Type(a0)        ; install type
         move.l   TSTRING(a0),d1       ; old value
         move.l   a1,TSTRING(a0)       ; new value

         tst.l    d1                   ; a string?
         beq.s    2$                   ; no
         addq.b   #T_COMMA-T_OPERATOR,d0
         beq.s    2$                   ; an operator ... no string
         cmp.l    a1,d1                ; same value?
         beq.s    2$                   ; yes

         ; Release the old value ...

         movea.l  a4,a0                ; environment
         movea.l  d1,a1                ; old string
         bsr      FreeString           ; release it

2$:      rts

         ; Allocate and link a new token.

3$:      movea.l  a4,a0                ; environment
         moveq    #0,d0                ; clear register
         exg      d0,a1                ; data=>D0, clear A1
         bsr      AddToken             ; D0=A0=token
         move.w   TOFFSET(a2),TOFFSET(a0)

         ; Link the new token following the current one.

         exg      a2,a0                ; old token=>A0, new token=>A2
         movea.l  d0,a1                ; first and last token
         bra      Relink               ; link it
