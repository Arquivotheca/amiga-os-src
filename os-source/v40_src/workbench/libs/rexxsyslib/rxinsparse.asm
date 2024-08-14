* == rxinsparse.asm ====================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     rxInsPARSE     =========================
* Implements the 'PARSE' instruction.
* [instruction module environment]
STRING   SET      16
FREESTR  SET      17
UPPERCS  SET      18

STACKBF  SET      160                  ; stack buffer size
rxInsPARSE
         lea      -STACKBF(sp),sp      ; stack buffer
         btst     #KT13B_UPPER,d5      ; uppercase conversion?
         beq.s    1$                   ; no 
         bset     #UPPERCS,d5          ; yes -- set flag

         ; Determine the input option.

1$:      andi.w   #KT13_OPTMASK,d5     ; option code?
         beq      rIPE33               ; error -- missing keyword

         ; Load the offset to the selected option.

         move.b   (rPJump-1)(pc,d5.w),d5
         movea.l  ev_ArgList(a4),a1    ; arglist clause
         move.l   cTL_Head(a1),d7      ; first arglist token

         ; Load registers and branch to process input option.

rPScan   movea.l  a0,a2                ; scan token
         movea.l  cTL_Head(a5),a1      ; first token
         movea.l  ev_GlobalPtr(a4),a0  ; global pointer
         movea.l  gn_TBuff(a0),a3      ; global work area

         move.l   rl_STDIN(a6),d1      ; STDIN name
         move.l   TSTRING(a1),d2       ; symbol/result string
         moveq    #0,d3                ; clear length
         jmp      rPJump(pc,d5.w)      ; branch to option

         ; Table of jump offsets in option order.

rPJump   dc.b     2$-rPJump            ; 'ARG'
         dc.b     3$-rPJump            ; 'EXTERNAL'
         dc.b     1$-rPJump            ; 'NUMERIC'
         dc.b     4$-rPJump            ; 'PULL'
         dc.b     5$-rPJump            ; 'SOURCE'
         dc.b     6$-rPJump            ; 'VAR'
         dc.b     8$-rPJump            ; 'VALUE'
         dc.b     7$-rPJump            ; 'VERSION'
         CNOP     0,2

         ; 'PARSE NUMERIC' ... current numeric options.

1$:      lea      pSCI(pc),a0          ; assume 'SCIENTIFIC' format
         moveq    #1<<EFB_SCI,d1       ; load mask
         and.b    EVFLAGS(a4),d1       ; scientific?
         bne.s    101$                 ; yes
         lea      pENG(pc),a0          ; 'ENGINEERING' format
101$:    move.l   a0,-(sp)             ; push format

         ; Push the digits/fuzz values and format the stream.

         move.l   ev_NumDigits(a4),-(sp)
         movea.l  sp,a1                ; data stream
         lea      NumerFmt(pc),a0      ; format string
         move.l   a3,d0                ; buffer area
         bsr      FormatString         ; D0=length A0=buffer
         addq.w   #(4+4),sp            ; restore stack
         move.w   d0,d3                ; length of string
         bra.s    rPCopy

         ; 'PARSE ARG' ... get invocation argument string.

2$:      movea.l  d7,a0                ; current token
         movea.l  (a0),a0              ; advance token
         tst.l    (a0)                 ; a successor?
         beq.s    rPCopy               ; no

         move.l   a0,d7                ; arglist token
         move.l   TSTRING(a0),d2       ; argument string?
         beq.s    rPCopy               ; no
         bset     #STRING,d5           ; set flag
         bra.s    rPCopy

         ; 'PARSE EXTERNAL' ... read from external stream.

3$:      move.l   rl_STDERR(a6),d1     ; STDERR name

         ; 'PARSE PULL' ... read from STDIN stream.

4$:      movea.l  d1,a1                ; logical name
         move.l   ev_Prompt(a4),d0     ; prompt string
         addq.l   #ns_Buff,d0          ; offset to string
         bsr      PullString           ; D0=length
         move.l   d0,d3                ; length
         bra.s    rPCopy

         ; 'PARSE SOURCE' ... source data string.

5$:      move.l   gn_SOURCE(a0),d2     ; global SOURCE string
         bset     #STRING,d5           ; set flag
         bra.s    rPCopy

         ; 'PARSE VAR' ... retrieve variable value.

6$:      btst     #TTB_SYMBOL,t_Type(a1)
         beq      rIPE37               ; error ... not a symbol
         bset     #STRING,d5           ; set flag
         movea.l  a4,a0                ; environment
         moveq    #0,d0                ; (no node)
         CALLSYS  LookUpValue          ; D0=error A1=value
         move.l   d0,d6                ; error?
         bne      rIPOut               ; yes
         exg      a1,d2                ; name=>A1, value=>D2

         ; Check whether the name string is also the value string ...

         cmp.l    a1,d2                ; name matches value?
         beq.s    rPAdvance            ; yes
         bset     #FREESTR,d5          ; no -- must release it
         bra.s    rPAdvance

         ; 'PARSE VERSION' ... use version/configuration (argstring).

7$:      move.l   rl_Version(a6),a3
         move.w   (ra_Length-ra_Buff)(a3),d3
         bra.s    rPCopy

         ; 'PARSE VALUE expr WITH' ... use expression result.

8$:      tst.l    d2                   ; a result?
         beq.s    rPAdvance            ; no
         bset     #STRING,d5

         ; Check whether to advance the scan token.

rPAdvance
         cmpa.l   cTL_Head(a5),a2      ; the first token?
         bne.s    rPCopy               ; no
         movea.l  (a2),a2              ; ... advance scan token

         ; Parse string is always copied to a temporary buffer ...

rPCopy   bclr     #STRING,d5           ; a string structure?
         beq.s    1$                   ; no
         movea.l  d2,a3                ; string structure
         move.w   ns_Length(a3),d3     ; string length
         addq.w   #ns_Buff,a3          ; offset to string

         ; Check whether a temporary work space must be allocated.

1$:      movea.l  sp,a0                ; stack buffer
         cmpi.w   #STACKBF,d3          ; length < STACKBF?
         bls.s    2$                   ; yes
         movea.l  a4,a0                ; environment
         move.l   d3,d0                ; length
         move.l   d3,d6                ; ... save length
         bsr      FindSpace            ; D0=A0=space

2$:      move.l   a0,d4                ; save buffer area
         movea.l  a3,a1                ; input pointer
         move.l   d3,d0                ; length
         move.w   #_LVOStrcpyN,d1      ; default offset
         btst     #UPPERCS,d5          ; uppercase conversion?
         beq.s    3$                   ; no
         subq.w   #(_LVOStrcpyN-_LVOStrcpyU),d1

3$:      jsr      0(a6,d1.w)           ; copy to buffer

         ; Check whether to release the parse string ...

         bclr     #FREESTR,d5          ; release the string?
         beq.s    4$                   ; no
         movea.l  a4,a0                ; environment
         movea.l  d2,a1                ; string
         bsr      FreeString           ; release it

         ; Process the template.

4$:      movea.l  a2,a1                ; scan token
         move.l   d4,d0                ; parse string
         move.l   d3,d1                ; length
         bsr.s    PatMatch             ; D0=error A1=token
         exg      d0,d6                ; length=>D0, error=>D6
         movea.l  a1,a2                ; update the scan token

         ; Temporary work space to be released?

         tst.l    d0                   ; buffer allocated?
         beq.s    5$                   ; no
         movea.l  a4,a0                ; environment
         movea.l  d4,a1                ; buffer
         bsr      FreeSpace            ; recycle it

         ; Check whether to continue the scan.

5$:      move.l   d6,d0                ; error?
         bne.s    rIPOut               ; yes
         cmpi.b   #T_END,t_Type(a2)    ; the STOPTOKEN?
         beq.s    rIPOut               ; yes ... all done
         movea.l  (a2),a0              ; next token
         bra      rPScan               ; loop back

rIPE33   moveq    #ERR10_033,d6        ; "missing keyword"
         bra.s    rIPOut

rIPE37   moveq    #ERR10_037,d6        ; "invalid template"

rIPOut   lea      STACKBF(sp),sp       ; restore stack
         rts

* ==========================     PatMatch     ==========================
* Template processing for the 'PARSE' instruction.  Private to rxInsPARSE.
* Registers:      A1 -- starting token
*                 D0 -- parse string
*                 D1 -- length
* Return:         D0 -- error
*                 A1 -- token
PATTERN  SET      16
PLACE    SET      19
DELTA    SET      20
INVERT   SET      21
SUSPEND  SET      22
DONE     SET      31                   ; (sign bit)
PatMatch
         movem.l  d2-d7/a2/a3/a5,-(sp)
         movea.l  a1,a2                ; starting token
         movea.l  d0,a5                ; parse string

         moveq    #0,d2                ; 'next' position
         moveq    #0,d3                ; 'match' position
         move.l   d1,d5                ; length
         moveq    #0,d6                ; clear variable count
         bra.s    PMScan               ; jump in

PMNext   movea.l  (a2),a2              ; advance token

         ; Begin the token scan ...

PMScan   movea.l  (a2),a3              ; next token
         move.b   t_Type(a2),d0        ; token type

         btst     #TTB_SYMBOL,d0       ; a symbol?
         beq      PMcheck              ; no

         ; Check if it's a pattern symbol.

         bclr     #PATTERN,d5          ; a pattern symbol?
         beq.s    PMPlace              ; no

         ; A pattern ... make sure the next token is a close parenthesis.

         cmpi.b   #T_CLOSE,t_Type(a3)  ; close parenthesis?
         bne      PMErr37              ; no -- error

         ; Retrieve the value of the pattern symbol.

         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; token
         moveq    #0,d0                ; (no node)
         CALLSYS  LookUpValue          ; D0=error D1=literal A1=value
         bne      PMOut                ; ... error

         tst.w    d1                   ; a literal?
         beq.s    1$                   ; no ...
         cmpa.l   TSTRING(a2),a1       ; same as token?
         bne.s    2$                   ; no ... don't need to copy

         ; Make a copy of the value string.

1$:      movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=value

         ; Reclassify the T_CLOSE token and install the pattern string.

2$:      move.b   #T_STRING,t_Type(a3) ; reclassify token
         move.l   a1,TSTRING(a3)       ; install pattern
         bra.s    PMNext               ; loop back

         ; An assignment symbol found ... save first token pointer.

PMAssign tst.w    d6                   ; already a variable?
         bne.s    1$                   ; yes
         move.w   d2,d3                ; save 'match' position
         move.l   a2,d7                ; save first token

1$:      addq.w   #1,d6                ; increment count
         bra.s    PMNext               ; loop back

         ; Check for a "place" or fixed symbol (specifying position).

PMPlace  bclr     #PLACE,d5            ; a "place" token?
         bne.s    1$                   ; yes
         btst     #TFB_PERIOD,t_Flags(a2)
         bne.s    PMAssign             ; ... a "placeholder"
         cmpi.b   #T_SYMFIXED,d0       ; a fixed symbol?
         bne.s    PMAssign             ; no ... a variable

         ; Retrieve the current value of a position variable.  Literal value
         ; will cause a syntax error, so no need to recyle value string.

1$:      movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; token
         moveq    #0,d0                ; (node address)
         CALLSYS  LookUpValue          ; D0=error A1=value
         bne      PMOut                ; ... error

         ; Convert the retrieved value to an integer.

         movea.l  a1,a0                ; value string
         bsr      CVs2i                ; D0=error D1=value
         bne      PMOut                ; ... error
         bclr     #INVERT,d5           ; preceding '-' sign?
         beq.s    2$                   ; no
         neg.l    d1                   ; ... negate it

2$:      moveq    #-1,d0               ; load -1
         bclr     #DELTA,d5            ; relative position?
         beq.s    3$                   ; no
         move.l   d2,d0                ; 'last' position

         ; Keep 'next' position within bounds 0 <= 'next' <= length.

3$:      add.l    d0,d1                ; new 'next' position
         bpl.s    4$                   ; ... positive
         moveq    #0,d1                ; clear it

4$:      move.w   d5,d2                ; (longword) length
         cmp.l    d1,d2                ; new 'next' <= length?
         bcc.s    5$                   ; yes
         move.w   d5,d1                ; ... use length

5$:      move.w   d1,d2                ; update 'next'
         bra      PMtest               ; check for assignment

         ; Check for other token types.

PMcheck  subq.b   #T_STRING,d0         ; a string?
         beq.s    PMstrn               ; yes
         subq.b   #T_COMMA-T_STRING,d0 ; a comma?
         beq.s    1$                   ; yes
         subq.b   #T_END-T_COMMA,d0    ; the STOPTOKEN?
         bne.s    2$                   ; no

         ; COMMA or STOPTOKEN ... scan finished.

1$:      bset     #DONE,d5             ; set 'done'
         move.w   d5,d2                ; assign remaining string
         bra.s    PMtest               ; check for assignment

         ; Token must be followed by a symbol ... verify it now.

2$:      btst     #TTB_SYMBOL,t_Type(a3)
         beq.s    5$                   ; error ... not a symbol

         addq.b   #T_END-T_OPERATOR,d0 ; an operator?
         beq.s    3$                   ; yes

         bset     #PATTERN,d5          ; assume 'pattern' flag
         subq.b   #T_OPEN-T_OPERATOR,d0; an open parenthesis?
         beq.s    4$                   ; yes
         bra.s    5$                   ; ... error

         ; An operator token ... must be followed by a positional symbol.

3$:      bset     #PLACE,d5            ; set 'place' flag
         move.b   TOPCODE(a2),d0       ; load opcode
         cmpi.b   #OP_EQ,d0            ; an '='?
         beq.s    4$                   ; yes

         bset     #DELTA,d5            ; 'delta' flag
         subi.b   #OP_ADD,d0           ; a '+' sign
         beq.s    4$                   ; yes

         bset     #INVERT,d5           ; 'invert' flag
         addq.b   #OP_ADD-OP_SUB,d0    ; a '-' sign?
         bne.s    5$                   ; no ... error

4$:      bra      PMNext               ; loop back

5$:      bra      PMErr37              ; template error

         ; A pattern string ... search for a match.

PMstrn   movea.l  TSTRING(a2),a3       ; pattern string
         move.w   ns_Length(a3),d1     ; pattern length
         beq.s    3$                   ; ... nothing to do

         move.w   d5,d0                ; total length
         sub.w    d1,d0                ; pattern <= length?
         bcs.s    3$                   ; no
         sub.w    d2,d0                ; 'next' <= remainder?
         bcs.s    3$                   ; no ...

         ; Set up the search loop.

1$:      lea      0(a5,d2.l),a0        ; start position
         lea      ns_Buff(a3),a1       ; pattern string
         move.w   ns_Length(a3),d1     ; pattern length
         subq.w   #1,d1                ; loop count

2$:      cmpm.b   (a0)+,(a1)+          ; matched?
         dbne     d1,2$                ; loop back
         beq.s    4$                   ; ... pattern matched

         ; Advance the starting index and check for more characters.

         addq.w   #1,d2                ; advance 'next'
         subq.w   #1,d0                ; any left?
         bcc.s    1$                   ; yes ... try again

         ; Pattern not matched ... set 'next' to end of string.

3$:      move.w   d5,d2                ; set 'next'
         bra.s    PMtest               ; check for assignment

         ; Pattern matched ... "suspend" characters past match position.

4$:      bset     #SUSPEND,d5          ; set flag
         movem.l  d0/a0,-(sp)          ; push length/pointer
         move.w   d2,d5                ; update length

         ; Check for any previous assignment tokens when a 'match' is made.

PMtest   tst.w    d6                   ; any variables?
         beq.s    PMRestore            ; no

         move.w   d2,d1                ; assignment 'next' index
         subq.w   #1,d6                ; decrement count
         beq.s    PMValue              ; last variable ...

         ; Multiple variables ... break out a word from the string.

         lea      0(a5,d3.l),a1        ; current 'match' position
         moveq    #' ',d0              ; load space
         subq.w   #1,d3                ; pre-decrement

         ; Advance the 'match' index past any leading spaces.

1$:      addq.w   #1,d3                ; advance 'match'
         cmp.w    d3,d5                ; 'match' < length?
         bls.s    2$                   ; no
         cmp.b    (a1)+,d0             ; a blank?
         beq.s    1$                   ; yes -- loop back

2$:      move.w   d3,d1                ; 'match' => 'next'

         ; Advance the 'next' position past the current word.

3$:      cmp.w    d1,d5                ; 'next' < length?
         bls.s    PMValue              ; no
         addq.w   #1,d1                ; advance 'next'
         cmp.b    (a1)+,d0             ; a blank?
         bne.s    3$                   ; no -- loop back

         ; Compute the length of the value string.

PMValue  move.w   d1,d4                ; save 'next'
         sub.w    d3,d1                ; 'next' - 'match' > 0?
         bhi.s    1$                   ; yes ... use length

         ; 'Next' position <= 'Match' position ... use remainder or null.

         move.l   rl_NULL(a6),d0       ; default value string
         move.w   d5,d1                ; total length
         sub.w    d3,d1                ; remaining length > 0?
         bls.s    2$                   ; no ... use null

         ; Create the value string structure.

1$:      movea.l  a4,a0                ; environment
         lea      0(a5,d3.l),a1        ; start position
         moveq    #NSF_STRING,d0       ; string type
         bsr      AddString            ; D0=A0=string

         ; Save the value string and update the assignment token.

2$:      move.w   d4,d3                ; 'next' => 'match'
         move.l   d0,d4                ; save value string
         movea.l  d7,a1                ; assignment token
         move.l   (a1),d7              ; next assignment token

         ; Check whether the token is just a "placeholder".

         moveq    #ACT_PHOLD,d1        ; trace action
         btst     #TFB_PERIOD,t_Flags(a1)
         bne.s    3$                   ; a "placeholder" ...

         ; A variable symbol ... make the assignment.

         moveq    #0,d1                ; (node address)
         movea.l  a4,a0                ; environment
         CALLSYS  AssignValue          ; D0=error A0=value A1=node
         moveq    #ACT_ASSIGN,d1       ; trace action

         ; Check whether to trace the value string.

3$:      move.b   TRACEOPT(a4),d0      ; trace byte
         btst     d0,#1<<TRACE_I!1<<TRACE_R
         beq.s    4$                   ; ... no tracing
         movea.l  a4,a0                ; environment
         movea.l  d4,a1                ; value string
         bsr      rxTrace              ; trace it

         ; Maybe release the value string (protected by KEEP if necessary.)

4$:      movea.l  a4,a0                ; environment
         movea.l  d4,a1                ; value string
         bsr      FreeString           ; recycle it
         bra.s    PMtest               ; ... loop back

         ; Check whether part of the string was "suspended" ...

PMRestore
         bclr     #SUSPEND,d5          ; suspended value?
         beq.s    1$                   ; no

         ; Restore the suspended value to the parse string.

         movem.l  (sp)+,d0/a1          ; pop length/pointer
         add.w    d0,d5                ; restore length
         lea      0(a5,d2.l),a0        ; current end
         CALLSYS  StrcpyN              ; copy it

         ; Check whether to continue the scan.

1$:      tst.l    d5                   ; scan done?
         bpl      PMNext               ; no ... loop back

         movea.l  a2,a1                ; updated token
         moveq    #0,d0                ; clear return

PMOut    movem.l  (sp)+,d2-d7/a2/a3/a5
         rts

         ; Template error return ... 

PMErr37  moveq    #ERR10_037,d0        ; invalid template
         bra.s    PMOut

         ; Constant strings

NumerFmt dc.b     '%d %d %s',0         ; format for numeric options
pSCI     dc.b     'SCIENTIFIC',0
pENG     dc.b     'ENGINEERING',0
         CNOP     0,2
