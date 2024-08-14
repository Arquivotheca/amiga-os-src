* == readclause.asm ====================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     ReadClause     =========================
* Extracts the next clause from the source stream and stacks it in the
* execution pipeline.  No error code is returned, as the clause carries
* the error code.  Private to 'rxParse'.
* Registers:   A4 -- storage environment
* Scratch:     D2/D3/D5/A5
CLASSIFY SET      16                   ; ready to classify the clause?
PREVSYM  SET      17                   ; previous token a token?
CHKTHEN  SET      18                   ; look for a THEN symbol?
IMPLICIT SET      19                   ; implicit end-of-clause?
ENDCL    SET      31                   ; end of clause?
HASHTHEN EQU      $FF&('T'+'H'+'E'+'N'); hash code for 'THEN'
ReadClause
         movem.l  a2/a3,-(sp)

         ; Compute the source segment from the segment and PC registers.
         ; If the current offset is beyond the end of a source segment,
         ; the segment index is advanced.

         move.l   ev_Segment(a4),d3    ; segment array?
         beq      RCExit               ; no

         movea.l  d3,a3                ; segment array
         move.l   ev_PC(a4),d0         ; current PC
         bsr      SrcSegment           ; D0=segment  D1=offset

1$:      cmp.l    (a3),d0              ; end of segment array?
         bcc      RCExit               ; yes -- all done

         move.l   d0,d3
         lsl.l    #2,d3                ; scale for 4-bytes
         movea.l  sSeg(a3,d3.l),a2     ; source segment slot
         cmp.w    ns_Length(a2),d1     ; offset within segment?
         bcs.s    2$                   ; yes

         addq.l   #1,d0                ; increment segment
         moveq    #0,d1                ; reset offset
         bra.s    1$                   ; loop back

2$:      lea      ns_Buff(a2,d1.l),a3  ; initial scan position
         bsr      SrcPosition          ; D0=source position
         move.l   d0,d2                ; final source position

         ; Allocate a new clause ...

         movea.l  a4,a0                ; environment
         bsr      GetClause            ; D0=A0=clause
         movea.l  d0,a5                ; save the clause
         move.l   d2,c_SrcPos(a5)      ; install position

         ; ... and push it into the pipeline.

         lea      ev_EStack(a4),a0     ; execution pipeline
         movea.l  a5,a1                ; the clause
         bsr      PushStack            ; stack it ...

         ; Check whether the clause data has been cached ...

         move.l   (a2),d0              ; line number or cache
         btst     #NSB_FLOAT,ns_Flags(a2)
         beq.s    RCSlowly             ; ... no cache
         movea.l  d0,a0                ; cached clause
         move.w   c_SrcLine(a0),d0     ; ... update line number

         ; Check that the cached clause matches the current position ...

         cmp.l    c_SrcPos(a0),d2      ; same position?
         bne.s    RCSlowly             ; no

         ; Restore the clause data (from c_Type to c_NextPos).

         movem.l  c_Type(a0),d0/d1/d2/d3/a1/a2
         movem.l  d0/d1/d2/d3/a1/a2,c_Type(a5)

         ; Allocate and initialize the tokens.  Rather than linking each
         ; token individually, we build a chain and link all at one.

         move.l   (a0),d2              ; token count (at least one)
         subq.w   #1,d2                ; loop count
         moveq    #t_SIZEOF,d3         ; token size (one quantum)
         moveq    #0,d5                ; clear total

         subq.w   #4,sp                ; "last node"
         movea.l  sp,a1                ; ... initial node
         lea      c_SIZEOF(a0),a2      ; start of token data
         movea.l  ev_GlobalPtr(a4),a3  ; base environment

         ; Use the "hot list" directly ... allocation is very time-critical.

3$:      move.l   gn_HotList1(a3),d0   ; "hot" node?
         bne.s    4$                   ; yes

         ; "Hot list" empty ... call allocator to get a token.

         movea.l  a4,a0                ; environment
         move.l   a1,-(sp)             ; push node
         bsr      FindQuickOne         ; D0=A0=token
         movea.l  (sp)+,a1             ; pop node
         bra.s    5$

4$:      movea.l  d0,a0                ; new node
         move.l   (a0),gn_HotList1(a3) ; update hot list
         add.w    d3,d5                ; increment total

         ; Add the new token to the chain ...

5$:      move.l   a0,(a1)              ; install node
         move.l   a1,sn_Pred(a0)       ; back pointer
         movea.l  a0,a1                ; update last

         ; Copy the token data ...

         addq.w   #t_Type,a0           ; save area
         move.l   (a2)+,(a0)+          ; t_Type/t_Flags/t_Offset
         move.l   (a2)+,(a0)           ; t_Data
         dbf      d2,3$                ; loop back

         sub.l    d5,gn_TotFree(a3)    ; update free total

         ; Link the token chain to the clause ... A1 is last token.

         move.l   (sp)+,d0             ; first token
         lea      cTL_Head(a5),a0      ; insertion point
         bsr      Relink
         bra      RCExit

         ; Clause not cached ... start token scan.

RCSlowly movem.l  d4/d6/d7,-(sp)       ; additional save
         move.w   d0,c_SrcLine(a5)     ; install line number
         moveq    #0,d2                ; flags (upper) / offset (lower)
         move.l   a3,d3                ; save starting position

         ; Ready to begin the scan.  Scratch registers have been saved
         ; so 'NextToken' doesn't have to save anything.

RC_Scan  bsr      NextToken            ; D0=A0=token A3=scan pointer
         beq      RCErr08              ; no token? ... error
         movea.l  a0,a2                ; save token
         move.l   a3,a1                ; scan position
         suba.l   d3,a1                ; scan offset
         move.w   a1,d2                ; save offset

         ; Check if the clause can be classified yet ...

         bset     #CLASSIFY,d2         ; classify on second token
         beq.s    4$                   ; ... not ready yet
         tst.b    c_Type(a5)           ; already classifed?
         bne.s    4$                   ; yes ...

         ; Attempt to classify the clause ...

         bsr      TypeClause           ; D0=type D1=instruction
         beq.s    RCErr08              ; ... error
         move.b   d0,c_Type(a5)        ; clause type
         move.l   d1,c_Keys(a5)        ; instruction code

         ; Clause classified ... check for special processing.

         subq.b   #C_LABEL,d0          ; a 'LABEL' clause?
         bne.s    0$                   ; no

         ; A LABEL clause ... turn on the cache if enabled.

         btst     #EFB_CACHE,EVFLAGS(a4)
         beq.s    5$                   ; ... not enabled
         bset     #(EXB_CACHE-16),(STATUS+1)(a4)
         bra.s    5$

0$:      subq.b   #C_ASSIGN-C_LABEL,d0 ; an 'ASSIGNment' clause?
         beq.s    3$                   ; yes -- remove '=' token

         subq.b   #C_INSTRUCT-C_ASSIGN,d0
         bne.s    4$                   ; ... no special processing

         ; An INSTRUCTION clause ... check for special cases.

         and.l    PRIMask(PC),d1       ; primary opcode
         swap     d1                   ; instruction word=>lower

         moveq    #CHKTHEN,d0          ; 'look for THEN' flag
         cmpi.w   #KC_IF>>16,d1        ; an IF instruction?
         beq.s    1$                   ; yes ...
         cmpi.w   #KC_WHEN>>16,d1      ; a WHEN?
         beq.s    1$                   ; yes ...

         moveq    #IMPLICIT,d0         ; 'implicit end' flag
         cmpi.w   #KC_THEN>>16,d1      ; a 'THEN' instruction?
         beq.s    1$                   ; yes
         cmpi.w   #KC_ELSE>>16,d1      ; an 'ELSE'?
         beq.s    1$                   ; yes
         cmpi.w   #KC_OTHRW>>16,d1     ; an 'OTHERWISE'?
         bne.s    2$                   ; no ...

1$:      bset     d0,d2                ; set flag

         ; Check if current token is a 'BLANK' (which can be discarded ...)

2$:      btst     #TFB_BLANK,t_Flags(a2)
         beq.s    4$                   ; not a blank ...

         ; Current token not needed ... release it.

3$:      movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         move.l   a2,d0                ; current token
         bsr      FreeToken            ; release it

         ; Check for implicit end-of-clause ...

4$:      bclr     #IMPLICIT,d2         ; implicit end-of-clause?
         beq.s    6$                   ; no ...

         ; End of clause ... insert a STOPTOKEN.

5$:      moveq    #0,d0                ; no data
         moveq    #T_END,d1
         bsr      QToken               ; terminating token
         bra.s    RCOut                ; ... all done

6$:      tst.l    d2                   ; end of clause?
         bpl      RC_Scan              ; no -- loop back
         bra.s    RCOut

         ; Error in clause ... reclassify as an 'ERROR' clause

RCErr08  moveq    #ERR10_008,d0        ; error code
         movea.l  a5,a0
         bsr.s    ErrClause            ; flag it

RCOut    sub.l    d3,a3                ; final scan position
         move.w   a3,c_Len(a5)         ; ... clause length
         movem.l  (sp)+,d4/d6/d7       ; partial restore

RCExit   movem.l  (sp)+,a2/a3          ; quick restore
         rts

* ========================     ErrClause     ==========================
* Classifies a clause as an ERROR.
* Registers:   A0 -- clause
*              D0 -- error code
ErrClause
         move.b   #C_ERROR,c_Type(a0)  ; reclassify type
         or.l     #KTF_ERROR,d0        ; new instruction code
         move.l   d0,c_Keys(a0)        ; install it
         rts

* ======================     TypeClause     ============================
* Scans a token list to classify a clause.  Function is not called until
* two tokens have been read, or until the clause is terminated.  This
* routine is private to 'ReadClause'.
* Registers:   A4 -- environment
*              A5 -- clause
* Return:      D0 -- clause type or 0
*              D1 -- instruction word
TypeClause
         movea.l  cTL_Head(a5),a0         ; first token
         move.b   t_Type(a0),d0           ; token type

         btst     #TTB_SYMBOL,d0          ; a SYMBOL token?
         bne.s    1$                      ; yes
         subq.b   #T_STRING,d0            ; a STRING token?
         beq.s    TCComm                  ; yes
         subq.b   #T_OPERATOR-T_STRING,d0 ; an operator token?
         beq.s    TCComm                  ; yes
         subq.b   #T_OPEN-T_OPERATOR,d0   ; an open parenthesis?
         beq.s    TCComm                  ; yes
         subq.b   #T_END-T_OPEN,d0        ; a STOPTOKEN?
         beq.s    TCNull                  ; yes

         moveq    #0,d0                   ; clause not classified ...
         bra.s    TCOut

         ; A symbol token ... get the successor.  Except for NULL clauses,
         ; all clauses must have at least two tokens.

1$:      movea.l  (a0),a1              ; next token ...
         cmpi.b   #T_LABEL,t_Type(a1)  ; a LABEL?
         beq.s    TCLabel              ; yes -- a LABEL clause
         cmpi.b   #T_SYMFIXED,d0       ; a FIXED symbol?
         beq.s    TCComm               ; yes ...

         ; A variable symbol:  check if NEXT token is an '='

         btst     #TFB_EQUALS,t_Flags(a1)
         beq.s    2$                   ; ... not an '='

         ; An ASSIGNment statement found.  Flag the symbol ...

         bset     #TFB_ASSIGN,t_Flags(a0)
         move.l   #KTF_ASSGN,d1        ; load the instruction code
         moveq    #C_ASSIGN,d0         ; ASSIGNMENT type
         bra.s    TCOut

2$:      movea.l  TSTRING(a0),a0       ; string structure
         bsr      KeyTable             ; D0=keyword or 0
         beq.s    TCComm               ; ... a command

         ; Release the first token after identifying the instruction ...

         move.l   d0,-(sp)             ; push keyword
         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         move.l   cTL_Head(a5),d0      ; the first token
         bsr      FreeToken            ; release it ...

         move.l   (sp)+,d1             ; pop keyword
         moveq    #C_INSTRUCT,d0       ; 'INSTRUCTION' type
         bra.s    TCOut

         ; A 'NULL' clause ... blank line or comment

TCNull   move.l   #KTF_NULL,d1
         moveq    #C_NULL,d0           ; 'NULL' type
         bra.s    TCOut

         ; A 'LABEL' clause ... symbol followed by a colon

TCLabel  move.l   #KTF_LABEL,d1
         moveq    #C_LABEL,d0          ; 'LABEL' type
         bra.s    TCOut

         ; A 'COMMAND' clause

TCComm   move.l   #KTF_COMM,d1
         moveq    #C_COMMAND,d0        ; 'COMMAND' type

TCOut    rts

* =========================     NextToken     ==========================
* Scans the input stream and builds the next token.  Registers A4 and A5
* are preserved.  This routine is private to 'ReadClause'.
* Registers:   A3 -- scan pointer
*              A4 -- storage environment
*              A5 -- clause address
*              D2 -- scan flags (upper) / offset (lower)
* Return:      D0 -- allocated token
*              A0 -- same
*              D2 -- updated scan flags
*              A3 -- updated scan position
* Scratch:     D4/D5/D6/D7
NextToken
         move.l   a2,d7                ; quick register save

         ; Begin scanning by checking for operators.  The scan pointer may
         ; be advanced (past blanks) even if no operators are found ...

         btst     #CLASSIFY,d2         ; clause started?
         sne      d0                   ; ... set flag
         movea.l  a3,a0                ; scan pointer
         bsr      IsOper               ; D0=opcode A0=scan
         movea.l  a0,a3                ; always update scan position ...
         move.w   d0,d6                ; save opcode
         beq.s    NTCheck              ; ... check further

         moveq    #T_OPERATOR,d1       ; token type
         bsr      QToken               ; D0=A0=token

         moveq    #TFB_BLANK,d0        ; 'BLANK' flag
         cmpi.b   #OP_CCBL,d6          ; a blank operator?
         beq.s    1$                   ; yes
         moveq    #TFB_EQUALS,d0       ; 'EQUALS' flag
         cmpi.b   #OP_EQ,d6            ; an '=' token?
         bne.s    2$                   ; no

1$:      bset     d0,t_Flags(a0)       ; set flag

2$:      bclr     #PREVSYM,d2          ; update scan flags
         bra      NTOut

         ; Not an operator ... check character attributes.  Note carefully
         ; that the scan pointer is advanced ...

NTCheck  movea.l  rl_CTABLE(a6),a1     ; attribute table
         move.b   (a3)+,d6             ; current character (advance scan)
         adda.w   d6,a1                ; index to attribute
         btst     #CTB_REXXSYM,(a1)    ; a symbol?
         bne.s    1$                   ; yes
         btst     #CTB_REXXSPC,(a1)    ; special?
         beq.s    2$                   ; no ... try a string

         ; A special character ... load the token code from the table.

         lea      SPCTable(pc),a1      ; special character codes
         move.b   -'('(a1,d6.w),d6     ; load code
         bra      NTSpecial

         ; A symbol token ...

1$:      CALLSYS  IsSymbol             ; D0=token type D1=length A0=scan
         bra.s    NT_Symbol

         ; Check for a string token.  If found, the string starts at the
         ; initial position + 1.

2$:      bsr      IsString             ; D0=string type D1=length A0=scan
         bne.s    NT_String            ; string found ...

         ; An invalid token ... scan position already advanced.

NTError  suba.l   a0,a0                ; no token
         bra      NTOut                ; exit

         ; A string token was found ... save important data

NT_String
         exg      d0,d6                ; delimiter=>D0, string type=>D6
         movea.l  a3,a1                ; start of string
         movea.l  a0,a3                ; update scan position

         movea.l  ev_GlobalPtr(a4),a0  ; global data pointer
         movea.l  gn_TBuff(a0),a0      ; global work buffer

         ; Check whether it's a plain string

         subq.b   #STR_ASCII,d6        ; a ASCII string?
         bne.s    1$                   ; no
         bsr      Strcopy              ; D1=length A1=buffer
         bra.s    2$

         ; Convert a hex or binary string to character (packed) form

1$:      subq.b   #STR_HEX-STR_ASCII,d6; hex?
         seq      d0                   ; set flag
         move.l   a0,d4                ; save buffer
         CALLSYS  CVx2c                ; D0=error D1=length
         movea.l  d4,a1                ; restore buffer
         bne.s    NTError              ; error ...

         ; Allocate a string structure ...

2$:      movea.l  a4,a0                ; storage environment
         moveq    #NSF_STRING,d0       ; string type
         bsr      AddString            ; A0=D0=string structure

         ; Now it's time to allocate the token.  Test for implicit
         ; concatenation (and set 'symbol' flag).

         bset     #PREVSYM,d2          ; previous token a symbol?
         beq.s    3$                   ; no ...
         move.l   d0,d4                ; save string
         bsr      ImpCC                ; insert a concatenation operator
         move.l   d4,d0                ; restore string

3$:      moveq    #T_STRING,d1         ; token type
         bsr      QToken               ; D0=A0=token
         bra      NTOut

         ; Process the symbol token ...

NT_Symbol
         move.w   d1,d5                ; save length
         move.b   d0,d6                ; save symbol type
         move.l   a3,d4                ; start of symbol + 1
         subq.l   #1,d4                ; ... start of symbol
         movea.l  a0,a3                ; update the scan pointer

         ; Check for implicit concatenation (and set flag)

         bset     #PREVSYM,d2          ; previous symbol/string?
         beq.s    1$                   ; no ...
         bsr      ImpCC                ; insert the operator

1$:      move.b   d6,d1                ; token type (symbol)
         bsr      QToken               ; D0=A0=token
         movea.l  d0,a2                ; token address

         ; Determine the string type ...

         moveq    #NSF_STRING!NSF_NOTNUM,d0
         cmpi.b   #T_SYMFIXED,d6       ; a FIXED symbol?
         bne.s    2$                   ; no ...
         moveq    #NSF_STRING,d0       ; yes (may be a number)

2$:      movea.l  a4,a0                ; environment
         move.w   d5,d1                ; length
         bsr      GetString            ; D0=A0=string
         move.l   a0,TSTRING(a2)       ; install string
         move.l   a0,d6                ; save string

         ; Prepare to copy the string, converting to UPPERcase.

         addq.w   #ns_Buff,a0          ; destination
         movea.l  d4,a1                ; starting scan position
         move.w   d5,d0                ; length
         CALLSYS  StrcpyU              ; D0=hash
         movea.l  d6,a0                ; restore string
         move.b   d0,ns_Hash(a0)       ; install hash

         ; Check for special cases ...

         subq.w   #1,d5                ; length = 1?
         bne.s    3$                   ; no ...
         moveq    #TFB_PERIOD,d4       ; 'PERIOD' flag
         cmpi.b   #PERIOD,d0           ; a period? (hash code=character)
         beq.s    4$                   ; yes

3$:      subq.w   #4-1,d5              ; length=4?
         bne.s    5$                   ; no ...
         btst     #CHKTHEN,d2          ; looking for THEN?
         beq.s    5$                   ; no ...
         cmpi.b   #HASHTHEN,d0         ; hash code matches?
         bne.s    5$                   ; no
         moveq    #TFB_THEN,d4         ; 'THEN' flag

         ; Check for the 'THEN' keyword ...

         move.l   c_Keys(a5),d0        ; clause instruction code
         and.l    PRIMask(pc),d0       ; primary opcode
         bsr      SecCode              ; D0=keyword code
         beq.s    5$                   ; ... not found
         bset     #IMPLICIT,d2         ; implicit end

4$:      bset     d4,t_Flags(a2)       ; set flag

5$:      movea.l  a2,a0                ; return token
         bra.s    NTOut

         ; Create a special character token.

NTSpecial
         moveq    #0,d0                ; no data
         move.b   d6,d1                ; token type
         bsr.s    QToken               ; D0=A0=token

         ; Check for an open parenthesis ...

         moveq    #PREVSYM,d1          ; load flag
         subq.b   #T_OPEN,d6           ; open parenthesis?
         bne.s    1$                   ; no

         ; Check whether the previous token is a function definition ...

         bclr     d1,d2                ; previous symbol?
         beq.s    NTOut                ; no
         movea.l  sn_Pred(a0),a1       ; previous token
         cmpi.b   #T_CLOSE,t_Type(a1)  ; close parenthesis?
         beq.s    NTOut                ; yes ... not a function

         ; A function ... update attribute flags in the token and clause.

         bset     #TFB_FUNCDEF,t_Flags(a1)
         bset     #CFB_ANYFUNC,c_Flags(a5)
         bra.s    NTOut

         ; Check for a close parenthesis ... flag as "previous symbol".

1$:      bset     d1,d2                ; set 'previous symbol'
         subq.b   #T_CLOSE-T_OPEN,d6   ; close parenthesis?
         beq.s    NTOut                ; yes

         ; Check for a STOPTOKEN ...

         bclr     d1,d2                ; clear 'previous symbol'
         subq.b   #T_END-T_CLOSE,d6    ; a STOPTOKEN?
         bne.s    NTOut                ; no ...

         ; End of the clause ... set 'ENDCL' and 'CLASSIFY' flags.

         or.l     #(1<<ENDCL)!(1<<CLASSIFY),d2

NTOut    move.l   a0,d0                ; allocated token
         movea.l  d7,a2                ; quick restore
         rts

* ===========================     ImpCC     ============================
* Allocates an "implicit concatenation" operator token.  This routine is
* private to 'NextToken'.
ImpCC    moveq    #OP_CC,d0            ; opcode
         moveq    #T_OPERATOR,d1       ; token type
         bsr.s    QToken               ; D0=A0=token
         bset     #TFB_BLANK,t_Flags(a0)
         rts

* ===========================     QToken     ===========================
* Allocates and links a token.  This routine is private to 'NextToken'.
* Registers:   A4 -- storage environment
*              A5 -- clause
*              D0 -- token data
*              D1 -- token type
*              D2 -- token offset
* Return:      D0 -- token
*              A0 -- same
QToken   movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; clause
         bsr      AddToken             ; D0=A0=token
         move.w   d2,TOFFSET(a0)       ; save the offset
         rts

* ========================     CacheClause     =========================
* Caches the current clause.
* Registers:   A0 -- environment
*              A1 -- clause
CCSIZE   SET      c_SIZEOF             ; cached clause size
CacheClause
         movem.l  d2/a3-a5,-(sp)
         movea.l  a0,a4
         movea.l  a1,a5                ; save clause

         ; Make sure we're running the global segment ...

         movea.l  ev_GlobalPtr(a4),a3  ; global pointer
         movea.l  gn_SrcSeg(a3),a3     ; global segment
         cmpa.l   ev_Segment(a4),a3    ; global?
         bne.s    6$                   ; no

         ; Find the source string and set the 'FLOAT' flag

         move.l   c_SrcPos(a5),d0      ; source position
         bsr      SrcSegment           ; D0=segment D1=offset
         lsl.l    #2,d0                ; index
         movea.l  sSeg(a3,d0.l),a3     ; source string
         bset     #NSB_FLOAT,ns_Flags(a3)
         bne.s    6$                   ; already set

         ; Count the tokens ... must be at least one.

         move.l   cTL_Head(a5),d1      ; first token
         moveq    #0,d2                ; initial count

1$:      addq.w   #1,d2                ; increment count
         movea.l  d1,a1                ; token
         move.l   (a1),d1              ; successor?
         bne.s    1$                   ; yes

         subq.w   #1,d2                ; any tokens?
         beq.s    6$                   ; no??

         ; Set the 'cached' flag to prevent subsequent copies ...

         bset     #CFB_CACHED,c_Flags(a5)

         ; Allocate space for the cache data ...

         moveq    #CCSIZE,d0           ; cached clause size
         move.l   d2,d1                ; token count
         lsl.l    #3,d1                ; size for tokens
         add.w    d1,d0                ; total size
         movea.l  a4,a0                ; environment
         bsr      FindSpace            ; D0=A0=memory
         move.l   a0,(a3)              ; install cache

         ; Copy the clause data (from c_Type to c_NextPos).

         move.l   d2,(a0)              ; (c_Succ) token count
         addq.w   #c_Type,a5           ; start of data
         movem.l  (a5)+,d0/d1/d2/a1/a3/a4
         movem.l  d0/d1/d2/a1/a3/a4,c_Type(a0)

         ; Copy the token data ... A5 is cTL_Head list header.

3$:      lea      CCSIZE(a0),a0        ; end of clause
         bra.s    5$                   ; jump in

4$:      lea      t_Type(a5),a1        ; start of token data
         move.b   (a1),d1              ; token type
         move.l   (a1)+,(a0)+          ; t_Type/t_Flags/t_Offset
         move.l   (a1),(a0)+           ; t_Data
         beq.s    5$                   ; ... no string

         ; Check whether a string is present ... set 'SOURCE' attribute.

         subq.b   #T_OPERATOR,d1       ; operator token?
         beq.s    5$                   ; yes
         move.l   (a1),a1              ; string structure
         bset     #NSB_SOURCE,ns_Flags(a1)

5$:      movea.l  (a5),a5              ; next token
         tst.l    (a5)                 ; a successor?
         bne.s    4$                   ; yes

6$:      movem.l  (sp)+,d2/a3-a5
         rts

* ======================     SetCachePosition      =====================
* Installs the "next position" into the cached clause for the top range.
* Registers:   A0 -- environment
*              A1 -- clause
SetCachePosition
         btst     #CFB_CACHED,c_Flags(a1)
         beq.s    1$                   ; ... not cached

         move.l   a1,-(sp)             ; push clause
         movea.l  ev_RStack(a0),a1     ; (skHead) top range
         move.l   r_SrcPos(a1),d0      ; position of range
         bsr.s    FindCachedClause     ; D0=A0=cached clause
         movea.l  (sp)+,a1             ; pop clause
         beq.s    1$                   ; ... not found

         ; Cached clause found ... install this clause as "next" position.

         move.l   c_SrcPos(a1),c_NextPos(a0)

1$:      rts

* ======================     FindCachedClause     ======================
* Checks whether a clause has been cached and returns the cache pointer.
* Registers:   A0 -- environment
*              D0 -- source position
* Return:      D0 -- cached clause or 0
*              A0 -- same
FindCachedClause
         bsr      SrcSegment           ; D0=segment D1=offset (A0 preserved)
         lsl.l    #2,d0                ; scale index
         move.l   ev_Segment(a0),a0    ; segment array
         movea.l  sSeg(a0,d0.l),a1     ; source segment
         btst     #NSB_FLOAT,ns_Flags(a1)
         beq.s    1$                   ; ... no cache??

         ; Check that the cached clause matches the offset ...

         movea.l  (a1),a0              ; cached clause
         move.l   c_SrcPos(a0),d0      ; source position
         IFNE     SEGMASK-$0000FFFF
         andi.w   #SEGMASK,d0          ; mask offset
         ENDC
         cmp.w    d0,d1                ; matched?
         beq.s    2$                   ; yes

1$:      suba.l   a0,a0                ; clear return

2$:      move.l   a0,d0                ; set CCR
         rts

* ======================     FindCachedToken     =======================
* Returns a pointer to a cached (compressed) token.
* Registers:      A0 -- environment
*                 D0 -- source position
*                 D1 -- token offset
* Return:         D0 -- token or 0
*                 A0 -- same
FindCachedToken
         move.w   d1,-(sp)             ; push offset
         bsr.s    FindCachedClause     ; D0=A0=clause or 0
         movea.w  (sp)+,a1             ; pop offset
         beq.s    2$                   ; ... clause not cached

         move.l   (a0),d0              ; token count
         subq.w   #1,d0                ; loop ocunt
         lea      (c_SIZEOF-(t_SIZEOF-t_Type))(a0),a0

1$:      addq.w   #(t_SIZEOF-t_Type),a0; next token
         cmpa.w   (t_Offset-t_Type)(a0),a1
         dbeq     d0,1$                ; loop back
         beq.s    2$                   ; ... matched
         suba.l   a0,a0                ; clear return

2$:      move.l   a0,d0
         rts
