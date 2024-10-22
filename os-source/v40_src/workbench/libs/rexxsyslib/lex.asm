* == lex.asm ===========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     IsOper     ============================
* Scans the input stream for operator sequences.  If found, the opcode is
* returned.
* Registers: A0 -- scan position
*            D0 -- mask byte: clause started?
* Return:    A0 -- updated scan pointer
*            D0 -- opcode or 0
STOPBIT  SET      16                ; termination character?
BLANKBIT SET      31                ; blank characters?
IsOper
         movem.l  d2-d6/a2/a3,-(sp)
         movea.l  rl_CTABLE(a6),a2     ; attribute table
         move.b   d0,d6                ; save mask

         moveq    #0,d0                ; clear opcode
         moveq    #0,d1                ; low word: counter high word: flags
         moveq    #0,d2                ; clear character code
         moveq    #' ',d5              ; load blank
         clr.l    -(sp)                ; initialize the operator buffer
         bra.s    2$                   ; jump in

         ; Leading and imbedded blanks in the input stream are skipped.
         ; If an operator is found, trailing blanks are also skipped.  A
         ; blank may define a concatenation operation.

         ; N.B. loop will terminate after no more than three characters

1$:      bset     #BLANKBIT,d1         ; set 'BLANK' flag
2$:      move.b   (a0)+,d2             ; test character
         cmp.b    d2,d5                ; a blank?
         beq.s    1$                   ; yes -- loop back

         tst.w    d1                   ; any operator characters yet?
         beq.s    3$                   ; no ...

         ; Check for an operator sequence as soon as we have a character.

         movea.l  sp,a3                ; character buffer
         bsr.s    OperCode             ; D0=opcode
         tst.b    d0                   ; opcode determined?
         bne.s    5$                   ; yes -- all done

         ; Check whether the character is a REXX operator.

3$:      btst     #CTB_REXXOPR,0(a2,d2.w)
         beq.s    4$                   ; ...not an operator
         move.b   d2,0(sp,d1.w)        ; store the character
         addq.w   #1,d1                ; increment count
         bra.s    2$                   ; loop back

         ; Scan finished, but no operator sequence found.  Check for a
         ; blank operator ... blanks are ignored at the start of a clause,
         ; but define a concatenation operator elsewhere.

4$:      tst.l    d1                   ; any blanks?
         bpl.s    9$                   ; no ...
         moveq    #OP_CCBL,d0          ; blank (concatenation) operator
         and.b    d6,d0                ; operator or 0
         bra.s    9$

         ; Opcode determined ... do we need to check for trailing blanks?

5$:      btst     #STOPBIT,d1          ; termination character?
         bne.s    9$                   ; yes ...

6$:      cmp.b    (a0)+,d5             ; a blank?
         beq.s    6$                   ; loop back

9$:      subq.w   #1,a0                ; back up the scan pointer
         addq.w   #4,sp                ; restore the stack ...
         movem.l  (sp)+,d2-d6/a2/a3
         rts

* =========================     OperCode     ===========================
* Scans a buffer of operator characters looking for a valid sequence.
* The operator characters are in a buffer padded with NULLs.  This routine
* is private to 'IsOper'.
* Registers:   A3 -- buffer
*              D2 -- next character
* Return:      D0 -- opcode
* Scratch:     D3/D4
OperCode
         move.b   (a3),d3              ; first buffer character ...
         cmpi.b   #'~',d3              ; special case?
         beq.s    OCSpecial            ; yes ...

         ; Scan the list of first operator characters ...

         lea      OCOpChar+OCNUM(pc),a1; start of table
         moveq    #OCNUM-1,d4          ; loop count

1$:      cmp.b    -(a1),d3             ; a match?
         dbeq     d4,1$                ; loop back

         ; For the operators in this table, one character look-ahead is
         ; sufficient to resolve sequences ...

         move.b   OCAlt(a1),d0         ; load alternate opcode
         cmp.b    OCTest(a1),d2        ; test character matches next?
         bne.s    OCStop               ; no
         move.b   OCPri(a1),d0         ; load primary opcode
         rts

         ; Special case:  sequences beginning with '~' may require three
         ; characters to resolve ...

OCSpecial
         moveq    #'=',d3              ; load '='
         tst.b    1(a3)                ; second character available?
         bne.s    3$                   ; must be a '=' ... check further

         ; Check for allowed next characters

         cmpi.b   #'<',d2              ; '~<' sequence?
         bne.s    1$                   ; no ...
         moveq    #OP_GE,d0            ; load '>=' code
         rts

1$:      cmpi.b   #'>',d2              ; '~>' sequence?
         bne.s    2$                   ; no ...
         moveq    #OP_LE,d0            ; load '<=' code
         rts

2$:      cmp.b    d3,d2                ; '~=' sequence?
         beq.s    OCOut                ; yes -- wait for next character
         move.b   #OP_NOT,d0           ; load '~' code
         bra.s    OCStop

         ; Preceding sequence was '~='

3$:      moveq    #OP_NE,d0            ; load '~='
         cmp.b    d3,d2                ; next character a '='?
         bne.s    OCStop               ; no
         moveq    #OP_EXNE,d0          ; load '~==' code
         rts

         ; The 'STOPBIT' is set if a valid sequence has been found and
         ; the next character is not part of it.  It prevents the scan
         ; pointer from being advanced further ...

OCStop   bset     #STOPBIT,d1          ; terminal character

OCOut    rts

         ; The OpTable blocks provide the test character (the next one in
         ; sequence) and the possible opcodes.  The primary opcode is used
         ; if the next operator character matches the test character; the
         ; alternate code is used if the next character is DOES NOT match
         ; the test character.

         ; Table of first-characters

OCOpChar dc.b     '%'
         dc.b     '^'
         dc.b     '&'
         dc.b     '>'
         dc.b     '<'
         dc.b     '|'
         dc.b     '/'
         dc.b     '='
         dc.b     '*'
         dc.b     '-'
         dc.b     '+'                  ; search starts here ...
OCNUM    EQU      *-OCOpChar           ; number of codes to search

         ; Table of test (decision) characters

OCTest   EQU      *-OCOpChar           ; table of test characters
         dc.b     $FF                  ; no test for '%' operator
         dc.b     $FF                  ; no test for '^' operator
         dc.b     '&'                  ; test for '&' operator
         dc.b     '='                  ; test for '>' operator
         dc.b     '='                  ; test for '<' operator
         dc.b     '|'                  ; test for '|' operator
         dc.b     '/'                  ; test for '/' operator
         dc.b     '='                  ; test for '=' operator
         dc.b     '*'                  ; test for '*' operator
         dc.b     $FF                  ; no test for '-' operator
         dc.b     $FF                  ; no test for '+' operator

         ; Table of primary opcodes

OCPri    EQU      *-OCOpChar
         dc.b     0                    ; none
         dc.b     0                    ; none
         dc.b     OP_XOR               ; '&&' sequence
         dc.b     OP_GE                ; '>=' sequence
         dc.b     OP_LE                ; '<=' sequence
         dc.b     OP_CC                ; '||' sequence
         dc.b     OP_DIV3              ; '//' sequence
         dc.b     OP_EXEQ              ; '==' sequence
         dc.b     OP_EXPN              ; '**' sequence
         dc.b     0                    ; none
         dc.b     0                    ; none

         ; Table of alternate opcodes

OCAlt    EQU      *-OCOpChar           ; table of alternate opcodes
         dc.b     OP_DIV2              ; '%'
         dc.b     OP_XOR               ; '^'
         dc.b     OP_AND               ; '&'
         dc.b     OP_GT                ; '>'
         dc.b     OP_LT                ; '<'
         dc.b     OP_OR                ; '|' 
         dc.b     OP_DIV1              ; '/'
         dc.b     OP_EQ                ; '='
         dc.b     OP_MULT              ; '*'
         dc.b     OP_SUB               ; '-'
         dc.b     OP_ADD               ; '+'

* ==========================     IsString     ==========================
* Scans the input stream for strings and returns the type byte or 0.
* Registers:   A0 -- scan pointer
* Returns:     D0 -- string type code or 0
*              D1 -- total length
*              A0 -- updated scan pointer
IsString
         moveq    #0,d0                ; clear return
         move.b   (a0),d1              ; test character
         subi.b   #DQUOTE,d1           ; a double quote?
         beq.s    1$                   ; yes
         subq.b   #QUOTE-DQUOTE,d1     ; a single quote?
         bne.s    7$                   ; no

1$:      move.b   (a0),d0              ; load delimiter
         moveq    #-2,d1               ; initial length

         ; Skip over a double delimiter

2$:      addq.w   #1,a0                ; advance scan
         addq.l   #1,d1                ; increment length

         ; Scan the input stream for another delimiter

3$:      addq.l   #1,d1                ; increment length
         cmp.b    (a0)+,d0             ; a delimiter?
         bne.s    3$                   ; no ... loop back

         cmp.b    (a0),d0              ; double delimiter?
         beq.s    2$                   ; yes

         ; Check whether it's a HEX or BINARY string ...

         move.b   (a0),d0              ; next character
         bclr     #LOWERBIT,d0         ; convert to UPPER
         cmpi.b   #'X',d0              ; 'X' follows?
         beq.s    4$                   ; yes ...
         cmpi.b   #'B',d0              ; 'B' follows?
         bne.s    6$                   ; no ...
         moveq    #STR_BINARY,d0       ; yes -- BINARY string
         bra.s    5$

4$:      moveq    #STR_HEX,d0          ; HEX string

         ; We now test the character following the 'X' (or 'B').  If it's
         ; not a symbol character, the 'X' ('B') means a HEX (BINARY) string

5$:      move.w   d0,-(sp)             ; push code
         addq.w   #1,a0                ; advance pointer
         move.b   (a0),d0              ; test character
         movea.l  rl_CTABLE(a6),a1     ; attribute table
         adda.w   d0,a1                ; offset to character
         move.w   (sp)+,d0             ; pop code
         btst     #CTB_REXXSYM,(a1)    ; a REXX symbol?
         beq.s    7$                   ; no

         ; Not a HEX or BINARY string ... back up and reclassify.

         subq.w   #1,a0                ; back up

6$:      moveq    #STR_ASCII,d0        ; an ASCII string

7$:      tst.b    d0                   ; set CCR
         rts

* ==========================     IsSymbol     ==========================
* Scans the input stream for REXX symbols, and returns the type and length.
* The scan pointer is advanced.
* Registers:      A0 -- scan pointer
* Return:         D0 -- symbol opcode or 0 (UBYTE)
*                 D1 -- length of symbol   (UWORD)
*                 A0 -- scan pointer
ONEPER   SET      0                    ; at least one period?
NOTFIRST SET      15                   ; after first character?
FIXED    SET      31                   ; fixed symbol?
IsSymbol
         move.l   d3,-(sp)
         moveq    #0,d0                ; clear index
         move.l   a0,d1                ; save start
         moveq    #0,d3                ; flag register
         movea.l  rl_CTABLE(a6),a1     ; character table

         ; Begin the scan loop ...

1$:      move.b   (a0)+,d0             ; test character
         move.b   0(a1,d0.w),d0        ; attribute byte
         btst     #CTB_REXXSYM,d0      ; a REXX symbol character?
         beq.s    5$                   ; no ...

         ; Check for special cases ... period or leading digit.

         cmpi.b   #PERIOD,-1(a0)       ; a period?
         bne.s    2$                   ; no ...
         bset     #ONEPER,d3           ; at least one period
         beq.s    3$                   ; ... first period
         st       d3                   ; ... multiple periods
         bra.s    1$                   ; loop back

2$:      btst     #CTB_DIGIT,d0        ; a digit character?
         beq.s    4$                   ; no ...

         ; A symbol is 'FIXED' if its first character is a period or digit.

3$:      tst.w    d3                   ; first character?
         bmi.s    1$                   ; no
         bset     #FIXED,d3            ; set 'FIXED' flag

4$:      bset     #NOTFIRST,d3         ; set 'not first' flag
         bra.s    1$                   ; loop back

         ; A non-symbol character ... check for exponential notation.

5$:      tst.l    d3                   ; a fixed symbol?
         bpl.s    7$                   ; no

         moveq    #'-',d0              ; load '-' sign
         sub.b    -1(a0),d0            ; '-' sign?
         beq.s    6$                   ; yes
         subq.b   #'-'-'+',d0          ; '+' sign?
         bne.s    7$                   ; no

         ; Check for trailing 'E' for exponential notation.

6$:      move.b   -2(a0),d0            ; load character
         bclr     #5,d0                ; UPPERcase
         cmpi.b   #'E',d0              ; exponential?
         beq.s    1$                   ; yes ... loop back

         ; Scan complete ... determine final symbol attributes.

7$:      moveq    #0,d0                ; default return
         subq.w   #1,a0                ; back up
         sub.l    a0,d1                ; (start - end)
         beq.s    8$                   ; ... no characters

         neg.l    d1                   ; character count
         moveq    #T_SYMFIXED,d0       ; assume a FIXED symbol
         tst.l    d3                   ; fixed symbol?
         bmi.s    8$                   ; yes ...

         moveq    #T_SYMVAR,d0         ; assume a VARIABLE symbol
         tst.b    d3                   ; any periods?
         beq.s    8$                   ; no ...

         ; A stem or compound variable ...

         moveq    #T_SYMCMPD,d0        ; assume a COMPOUND symbol
         tst.b    d3                   ; multiple periods?
         bmi.s    8$                   ; yes -- compound symbol
         cmpi.b   #PERIOD,-1(a0)       ; last character a period?
         bne.s    8$                   ; no ...
         moveq    #T_SYMSTEM,d0        ; ... a STEM variable

8$:      move.l  (sp)+,d3
         tst.b    d0                   ; set CCR
         rts

         ; Table of special character codes relative to '('

SPCTable
SPCTABLE dc.b     T_OPEN               ; $28 (
         dc.b     T_CLOSE              ; $29 )
         dc.b     0                    ; $2A
         dc.b     0                    ; $2B
         dc.b     T_COMMA              ; $2C ,
         dc.b     0                    ; $2D
         dc.b     0                    ; $2E
         dc.b     0                    ; $2F
         dc.b     0                    ; $30
         dc.b     0                    ; $31
         dc.b     0                    ; $32
         dc.b     0                    ; $33
         dc.b     0                    ; $34
         dc.b     0                    ; $35
         dc.b     0                    ; $36
         dc.b     0                    ; $37
         dc.b     0                    ; $38
         dc.b     0                    ; $39
         dc.b     T_LABEL              ; $3A :
         dc.b     T_END                ; $3B ;

         CNOP     0,2                  ; restore alignment
