* =======================     ConvertBoolean     =======================
* Converts two string operands to boolean values.
* Registers:   A0 -- environment
*              A1 -- left operand
*              A2 -- right operand
*              A4 -- environment
*              D6 -- opcode
* Return:      D0 -- error code or 0
*              D1 -- right result
*              D2 -- left result
* Scratch:     D2 
ConvertBoolean
         bsr      CVs2bool             ; D0=error D1=boolean
         bne.s    1$                   ; ... conversion error
         move.l   d1,d2                ; save result

         movea.l  a4,a0                ; storage environment
         movea.l  a2,a1                ; RIGHT operand
         bsr      CVs2bool             ; D0=error D1=boolean

1$:      rts

* ===========================     EvalOp     ===========================
* Evaluates REXX operations.  Saves registers and calls the appropriate
* function for the specified operation.
* Registers:   A0 -- storage environment
*              A1 -- left operand
*              D0 -- opcode
*              D1 -- right operand
* Return:      D0 -- error code or 0
*              D1 -- result string
STACKBF  SET      8
EvalOp
         movem.l  d2/d3/d6/a2-a5,-(sp)
         subq.w   #STACKBF,sp
         movea.l  sp,a5                ; temporary stack
         movea.l  a0,a4                ; storage environment
         movea.l  a1,a3                ; save left
         movea.l  d1,a2                ; save right

         moveq    #0,d3                ; negation mask
         moveq    #OPERSEQ,d6          ; load mask
         and.b    d0,d6                ; opcode byte

         moveq    #0,d0                ; clear offset
         move.b   EOTable-1(pc,d6.w),d0; load offset
         jmp      EOTable(pc,d0.w)     ; branch

         ; Table of jump offsets for operator codes

EOTable  dc.b     11$-EOTable          ; 1 XOR
         dc.b     12$-EOTable          ; 2 OR

         dc.b     13$-EOTable          ; 3 AND

         dc.b     2$-EOTable           ; 4 LE
         dc.b     2$-EOTable           ; 5
         dc.b     2$-EOTable           ; 6
         dc.b     2$-EOTable           ; 7
         dc.b     2$-EOTable           ; 8
         dc.b     2$-EOTable           ; 9
         dc.b     21$-EOTable          ; 10 EXNE
         dc.b     22$-EOTable          ; 11 EXEQ
         dc.b     0                    ; (reserved)
         dc.b     0
         dc.b     0
         dc.b     0

         dc.b     EOConcat-EOTable     ; 16 OP_CC
         dc.b     EOConcat-EOTable     ; 17 OP_CCBL

         dc.b     EOFast-EOTable       ; 18 SUB
         dc.b     EOFast-EOTable       ; 19 ADD

         dc.b     EOFast-EOTable       ; 20 MULT
         dc.b     EOMath-EOTable       ; 21 DIV1
         dc.b     EOMath-EOTable       ; 22 DIV2
         dc.b     EOMath-EOTable       ; 23 DIV3

         dc.b     EOMath-EOTable       ; 24 EXPN

         dc.b     EONEG-EOTable        ; 25 OP_NEG
         dc.b     EOCNV-EOTable        ; 26 OP_SAME
         dc.b     14$-EOTable          ; 27 NOT
         CNOP     0,2

         ; OP_XOR

11$:     bsr.s    ConvertBoolean       ; D0=error D1=right D2=left
         eor.w    d2,d1                ; XOR result
         bra.s    3$

         ; OP_OR

12$:     bsr.s    ConvertBoolean       ; D0=error D1=right D2=left
         or.w     d2,d1                ; OR result
         bra.s    3$

         ; OP_AND

13$:     bsr.s    ConvertBoolean       ; D0=error D1=right D2=left
         and.w    d2,d1                ; AND result
         bra.s    3$

         ; OP_NOT operation

14$:     movea.l  a2,a1                ; right operand
         bsr      CVs2bool             ; D0=error D1=boolean
         not.w    d1                   ; NOT result
         bra.s    3$

         ; OP_EXNE ... not exactly equals.

21$:     moveq    #-1,d3               ; EXNE operation

         ; OP_EXEQ ... exactly equals

22$:     bsr.s    ConvertString        ; convert operands
         movea.l  a3,a0                ; left operand
         movea.l  a2,a1                ; right operand
         CALLSYS  CmpString            ; D0=boolean
         move.w   d0,d1                ; save result
         moveq    #0,d0                ; clear error
         eor.w    d3,d1                ; ... boolean result
         bra.s    3$

         ; A string or numeric comparison ...

2$:      bsr.s    ConvertString        ; A2/A3=operands
         bsr      OpCompare            ; D0=error D1=boolean

         ; A boolean result in CCR ... use global TRUE/FALSE for value.

3$:      movem.l  rl_FALSE(a6),d1/a1   ; load FALSE/TRUE
         beq.s    EODone               ; FALSE result
         move.l   a1,d1                ; TRUE result
         bra.s    EODone

         ; Concatenation operation ... both operands must be strings.

EOConcat bsr.s    ConvertString        ; convert operands
         bsr.s    OpConcat             ; D0=error D1=result

         ; Cleanup operations ... release any temporary strings.

EODone   move.l   d0,d2                ; save error
         swap     d6                   ; count=>lower
         subq.w   #1,d6                ; loop count
         bcs.s    2$                   ; ... nothing to do

         move.l   d1,d3                ; save result
         movea.l  sp,a5                ; temporary stack

1$:      movea.l  (a5)+,a1             ; operand
         movea.l  a4,a0                ; environment
         bsr      FreeString           ; release string
         dbf      d6,1$                ; loop back

         move.l   d3,d1                ; restore result

2$:      move.l   d2,d0                ; set CCR
         bra.s    EOOut

         ; OP_SUB, OP_ADD, or OP_MULT ... attempt an integer operation.

EOFast   bsr      FastMath             ; D0=error D1=result
         bra.s    EOOut

         ; An arithmetic operation

EOMath   bsr      OpMath               ; D0=error D1=result
         bra.s    EOOut

         ; OP_NEG ... unary negation.

EONEG    bset     #WORDLEN-1,d3        ; sign bit

         ; OP_SAME ... unary conversion.

EOCNV    movea.l  a2,a1                ; right operand
         movea.l  d3,a3                ; save sign
         bsr      ConvertDouble        ; D0=error D2/D3=operand A0=dec
         bne.s    EOOut                ; ... error
         move.l   d2,d0                ; upper operand
         or.l     d3,d0                ; zero?
         beq.s    1$                   ; yes ...
         move.l   a3,d0                ; sign bit
         eor.l    d0,d2                ; ... XOR it in

1$:      move.w   a0,d1                ; decimal places
         bsr      DoubleToString       ; D0=error D1=result

EOOut    addq.w   #STACKBF,sp          ; restore stack
         movem.l  (sp)+,d2/d3/d6/a2-a5
         rts

* =======================     ConvertString      =======================
* Converts the operands to string format.
* Registers:   A0 -- environment
*              A1 -- left operand
*              A2 -- right operand
*              A5 -- temporary stack
ConvertString
         swap     d6                   ; count => lower
         btst     #NSB_STRING,ns_Flags(a1)
         bne.s    1$                   ; ... already a string

         bsr      CVd2s                ; D0=string
         move.l   d0,a3                ; new left operand
         move.l   d0,(a5)+             ; install temporary
         addq.w   #1,d6                ; increment count

1$:      btst     #NSB_STRING,ns_Flags(a2)
         bne.s    2$                   ; ... already a string

         movea.l  a4,a0                ; storage environment
         movea.l  a2,a1                ; right operand
         bsr      CVd2s                ; D0=string
         move.l   d0,a2                ; new right operand
         move.l   d0,(a5)+             ; install temporary
         addq.w   #1,d6

2$:      swap     d6                   ; opcode=>lower, count=>upper
         rts

* ==========================     OpConcat     ==========================
* Evaluates concatenation operations.  Private to 'EvalOp'.
* Registers:   A2 -- right operand
*              A3 -- left operand
*              A4 -- environment
*              D6 -- opcode
* Return:      D0 -- error code or 0
*              D1 -- result string
* Scratch:     D2/D3/A2/a3
OpConcat
         cmpi.b   #(OPERSEQ&OP_CC),d6  ; plain concatenation?
         seq      d3                   ; -1 or 0
         addq.b   #1,d3                ; 0 or 1

         move.l   d3,d1                ; initial length
         move.w   ns_Length(a3),d3     ; LEFT length
         add.l    d3,d1
         moveq    #0,d2                ; clear length
         move.w   ns_Length(a2),d2     ; RIGHT length
         add.l    d2,d1                ; ... total length

         moveq    #ERR10_009,d0        ; "string too long"
         cmp.l    MaxLen(pc),d1        ; too long?
         bgt.s    2$                   ; yes ...

         ; Allocate the result string ...

         movea.l  a4,a0                ; environment
         moveq    #NSF_STRING,d0       ; string type
         bsr      GetString            ; D0=A0=string

         ; .. and install the left operand string.

         lea      ns_Buff(a3),a1       ; left source
         addq.w   #ns_Buff,a0          ; destination
         movea.l  d0,a3                ; save string
         move.w   d3,d0                ; left length
         CALLSYS  StrcpyN              ; D0=hash A0=scan

         ; Check whether to install a blank.

         cmpi.b   #(OPERSEQ&OP_CC),d6  ; plain concatenation?
         beq.s    1$                   ; yes
         moveq    #' ',d1
         move.b   d1,(a0)+             ; insert blank
         add.b    d1,d0                ; update hash code

         ; Install the RIGHT operand string.

1$:      move.b   d0,ns_Hash(a3)       ; save hash code
         lea      ns_Buff(a2),a1       ; right source
         move.w   d2,d0                ; right length
         CALLSYS  StrcpyN              ; D0=hash
         add.b    d0,ns_Hash(a3)       ; final hash code

         move.l   a3,d1                ; result
         moveq    #0,d0                ; clear error

2$:      rts

* ==========================     OpCompare     =========================
* Performs numeric or string comparison operations.  If both operands are
* numeric, an arithmetic comparison is performed; otherwise, the operands
* are compared as strings.  This routine is private to 'EvalOp'.
* Registers:   A2 -- right operand
*              A3 -- left operand
*              A4 -- storage environment
*              D6 -- opcode
* Return:      D0 -- error code or 0
*              D1 -- boolean result
* Scratch:     D2-D7/A2-A5

* Operator flag bits
CB_LE    SET   1<<(OP_LE-OP_LE)
CB_LT    SET   1<<(OP_LT-OP_LE)
CB_GE    SET   1<<(OP_GE-OP_LE)
CB_GT    SET   1<<(OP_GT-OP_LE)
CB_NE    SET   1<<(OP_NE-OP_LE)
CB_EQ    SET   1<<(OP_EQ-OP_LE)
OpCompare
         movea.w  ev_NumDigits(a4),a5  ; digits setting
         move.b   ns_Flags(a3),d0
         move.b   ns_Flags(a2),d1

         ; First check if either string is non-numeric.  If so, a string
         ; comparison is performed.

         move.b   d1,d2
         or.b     d0,d2                ; OR flags
         btst     #NSB_NOTNUM,d2       ; either string non-numeric?
         bne.s    OCString             ; yes

         ; Modify the current setting of numeric digits by the fuzz value,
         ; and skip the test for integer operations if less than 9 digits.

         move.w   a5,d2                ; current digits
         sub.w    ev_NumFuzz(a4),d2    ; new value
         cmpi.w   #9,d2                ; at least 9 digits?
         blt.s    1$                   ; no ...

         ; Digits >= 9 ... check if both strings in integer (binary) form.

         and.b    d0,d1                ; AND flags
         btst     #NSB_BINARY,d1       ; both integers?
         beq.s    1$                   ; no ...

         move.l   (a3),d0              ; (IVALUE) left operand
         sub.l    (a2),d0              ; difference (left - right)
         bra.s    OCCheck

         ; Attempt numeric conversions.  First the left operand ...

1$:      move.w   d2,ev_NumDigits(a4)  ; update digits
         movea.l  a3,a1                ; left operand
         bsr.s    ConvertDouble        ; D0=error D2/D3=left operand
         bne.s    OCString             ; ... error

         ; ... now check the RIGHT operand

2$:      movea.l  a2,a1                ; right operand
         movem.l  d2/d3,-(sp)          ; push left
         bsr.s    ConvertDouble        ; D0=error D2/D2=right operand
         movem.l  (sp)+,d0/d1          ; pop left
         bne.s    OCString             ; ... string compare

         ; Both operands numeric ... perform an IEEE DP compare operation.

         move.l   a6,a2                ; save REXX base
         movea.l  rl_IeeeDPBase(a6),a6 ; library base
         CALLSYS  IEEEDPCmp            ; D0=(-1,0,1) result
         movea.l  a2,a6                ; restore REXX base
         bra.s    OCTestF

         ; String comparison to be performed ... both operands are strings.

OCString movea.l  a3,a0                ; left operand
         bsr      TrimLeading          ; D0=length A0=start
         movea.l  a0,a1                ; start of string
         move.w   d0,d1                ; LEFT operand length

         movea.l  a2,a0                ; right operand
         bsr      TrimLeading          ; D0=length A0=start (A1/D1 preserved)

         ; Compare the strings, padding with blanks as necessary.

1$:      move.w   d0,d2                ; right characters?
         or.w     d1,d2                ; left characters?
         beq.s    OCCheck              ; no ... strings match

         move.b   (a1)+,d3             ; load left character
         subq.w   #1,d1                ; left character?
         bcc.s    2$                   ; yes

         subq.w   #1,a1                ; back up
         clr.w    d1                   ; clear count
         moveq    #' ',d3              ; load blank

2$:      move.b   (a0)+,d2             ; load character
         subq.w   #1,d0                ; right character?
         bcc.s    3$                   ; yes

         subq.w   #1,a0                ; back up
         clr.w    d0                   ; clear count
         moveq    #' ',d2              ; load blank

3$:      cmp.b    d2,d3                ; compare (left - right)
         beq.s    1$                   ; loop back

         ; Comparison result in CCR ... create a (-1,0,1) test value.

OCCheck  slt      d0                   ; -1 or 0
         sgt      d1                   ; -1 or 0
         sub.b    d1,d0                ; -1, 0, or 1
         ext.w    d0                   ; extend to word

OCTestF  move.b   OCFlags(pc,d0.w),d1  ; load flag mask
         subq.b   #(OPERSEQ&OP_LE),d6  ; operator sequence
         btst     d6,d1                ; operator flag set?
         sne      d1                   ; set flag

         move.w   a5,ev_NumDigits(a4)  ; restore numeric digits
         moveq    #0,d0                ; error code
         ext.w    d1                   ; set CCR
         rts

         ; Operator test flags

         dc.b     CB_NE!CB_LT!CB_LE    ; left < right
OCFlags  dc.b     CB_EQ!CB_LE!CB_GE    ; left = right
         dc.b     CB_NE!CB_GT!CB_GE    ; left > right
         CNOP     0,2

* ========================     ConvertDouble     =======================
* Private routine to convert operand strings to IEEE DP format.
* Registers:   A1 -- string to be converted
* Return:      D0 -- error
*              D2/D3 -- IEEE DP value
*              A0 -- decimal places
ConvertDouble
         movea.l  a4,a0                ; storage environment
         btst     #NSB_FLOAT,ns_Flags(a1)
         beq      CVs2d                ; D0=error D2/D3=result A0=dec places

         ; Already converted ... load DP value.

         movem.l  DVALUE(a1),d2/d3     ; load DP value
         moveq    #0,d1
         move.b   DECPLACE(a1),d1      ; decimal places
         movea.w  d1,a0
         moveq    #0,d0                ; return code
         rts

* =========================     FastMath     ===========================
* Attempts an integer add/subtract/multiply operation, and falls through
* if it fails.
* Registers:   A1 -- left operand
*              A2 -- right operand
* Return:      D0 -- error
*              D1 -- result
FastMath
         moveq    #NSF_BINARY,d0       ; integer mask
         and.b    ns_Flags(a1),d0      ; binary?
         and.b    ns_Flags(a2),d0      ; binary?
         beq.s    OpMath               ; no

         ; Both operands in integer (binary) format.

         move.l   (a2),d2              ; right operand
         move.l   (a1),d3              ; left operand
         cmpi.b   #(OPERSEQ&OP_ADD),d6 ; addition operation?
         beq.s    3$                   ; yes
         bcs.s    2$                   ; ... subtraction

         ; Multiplication ... check whether -32768 <= operands <= 32767

         cmp.l    d3,d2                ; right >= left?
         bge.s    1$                   ; yes
         exg      d2,d3                ; larger=>D2, smaller=>D3

1$:      cmpi.l   #32767,d2            ; larger <= 32767?
         bgt.s    OpMath               ; no
         cmpi.l   #-32768,d3           ; smaller >= -32768?
         blt.s    OpMath               ; no
         muls     d3,d2                ; D2=product
         bra.s    FMConvert

2$:      neg.l    d2                   ; negate operand

3$:      add.l    d3,d2                ; perform operation
         bvs.s    OpMath               ; overflow ... fall through

         ; Create the return string ... one quantum in size.

FMConvert
         movea.l  a4,a0                ; environment
         bsr      FindQuickOne         ; D0=A0=small string
         move.l   a0,d1                ; return string
         move.l   d2,(a0)+             ; (IVALUE) install value
         move.l   #NSF_BINARY<<8,(a0)  ; ns_Length/ns_Flags/ns_Hash
         moveq    #0,d0                ; clear error
         rts

* ===========================     OpMath     ===========================
* Performs arithmetic operations on string operands.  Arithmetic is done
* using IEEE DP operands; the result is left as an IEEE DP value.  This
* routine is private to 'EvalOp'.
* Registers:   A0 -- environment
*              A1 -- left operand
*              A2 -- right operand
*              A3 -- left operand
*              A4 -- storage environment
*              D6 -- opcode (lower byte)
* Return:      D0 -- error code or 0
*              D1 -- result string
* Scratch:     D2-D7/A2-A5
OpMath
         movem.l  d4/d5/d7,-(sp)       ; extra save
         movea.l  rl_IeeeDPBase(a6),a5 ; IEEE DP base

         ; Convert the LEFT operand

         bsr.s    ConvertDouble        ; D0=error D2/D3=result A0=dec places
         bne.s    OMExit               ; ... error
         move.w   a0,d7                ; decimal places
         move.l   d2,d4                ; save LEFT result
         move.l   d3,d5

         ; Check whether to skip the right operand conversion.

         cmpi.b   #(OPERSEQ&OP_EXPN),d6; exponentiation operation?
         bne.s    1$                   ; no ...
         btst     #NSB_BINARY,ns_Flags(a2)
         bne.s    2$                   ; integer ... skip RIGHT conversion

         ; Convert the right operand.

1$:      swap     d7                   ; left decimal places=>upper
         movea.l  a2,a1                ; right operand string
         bsr      ConvertDouble        ; D0=error D2/D3=result A0=dec places
         bne.s    OMExit               ; ... error
         move.w   a0,d7                ; right decimal places

         ; Check first for an exponentiation operation.

2$:      move.l   d4,d0                ; left operand
         move.l   d5,d1

         cmpi.b   #(OPERSEQ&OP_EXPN),d6; exponentiation operation?
         bcs.s    OMInfix              ; no ...

         ; An exponentiation operation ... verify an integer exponent.

         btst     #NSB_BINARY,ns_Flags(a2)
         beq.s    OMErr48              ; error ... not an integer

         ; Check for a zero base.

         swap     d7                   ; use left decimal places
         bsr      DPTst                ; base zero? (D0/D1 preserved)
         beq.s    3$                   ; yes ...

         ; Perform the exponentiation operation ... should check for 
         ; overflow or NaN return.

         movea.l  (a2),a0              ; exponent value
         bsr      DPPow                ; D0/D1=operand to integer power
         move.l   d0,d2                ; save result
         move.l   d1,d3

         ; Check for exponent overflow ...

         swap     d0                   ; exponent=>lower
         move.w   #EXPMASK>>16,d1      ; maximum exponent
         and.w    d1,d0                ; extract exponent
         cmp.w    d1,d0                ; exponent overflow?
         beq.s    OMErr48              ; yes ... error

         ; Compute the final decimal places (should check for overflow).

         move.l   (a2),d0              ; exponent >= 0?
         bmi.s    4$                   ; no ...
         mulu     d0,d7                ; D7=decimal places
         bra.s    5$

         ; Base is zero ... return 0.0 or 1.0 based on sign of exponent.

3$:      moveq    #0,d2                ; assume 0.0 return
         moveq    #0,d3
         tst.l    (a2)                 ; exponent >= 0?
         bmi.s    OMErr48              ; operand < 0 ... error
         bgt.s    4$                   ; operand > 0 ... return 0.0
         move.l   DPF1(pc),d2          ; operand = 0 ... return 1.0

4$:      moveq    #0,d7                ; no decimal places

5$:      bra      OMResult

OMErr48  moveq    #ERR10_048,d0        ; invalid operand

OMExit   movem.l  (sp)+,d4/d5/d7
         rts

         ; An infix operation

OMInfix  cmpi.b   #(OPERSEQ&OP_ADD),d6 ; addition operation?
         beq.s    1$                   ; yes
         bhi.s    2$                   ; ... check further
         bchg     #WORDLEN-1,d2        ; ... negate operand

         ; Addition (or subtraction) operation ...

1$:      exg      a5,a6                ; REXX=>A5, IEEE=>A6
         CALLSYS  IEEEDPAdd            ; D0/D1=sum

         ; ... use the maximum of the decimal places.

         move.w   d7,d2                ; right decimal places
         swap     d7
         cmp.w    d2,d7                ; left >= right?
         bcc.s    OMSwap               ; yes
         move.w   d2,d7                ; no -- use right
         bra.s    OMSwap

         ; Multiplication operation?

2$:      cmpi.b   #(OPERSEQ&OP_MULT),d6; multiplication operation?
         bne.s    3$                   ; no ...
         exg      a5,a6                ; REXX=>A5, IEEE=>A6
         CALLSYS  IEEEDPMul            ; D0/D1=product

         ; ... use the sum of the decimal places.

         move.w   d7,d2                ; right decimal places
         swap     d7                   ; left decimal places
         add.w    d2,d7                ; ... use sum
         bra.s    OMSwap

         ; A division operation (ordinary, integer, or remainder)

3$:      move.l   d2,d0                ; RIGHT operand
         move.l   d3,d1
         bsr      DPTst                ; (CCR) zero divisor?
         beq.s    OMErr48              ; yes ... error

         move.l   d4,d0                ; load dividend
         move.l   d5,d1
         exg      a5,a6                ; REXX=>A5, IEEE=>A6
         movem.l  d2/d3,-(sp)          ; push divisor
         CALLSYS  IEEEDPDiv            ; D0/D1=quotient
         movem.l  (sp)+,d2/d3          ; pop divisor

         cmpi.b   #(OPERSEQ&OP_DIV1),d6; ordinary division?
         beq.s    OMZero               ; yes ...

         ; Split the operand into integral and fractional parts ...

         exg      a5,a6                ; IEEE=>A5, REXX=>A6
         bsr      DPModf               ; D0/D1=fraction  A0/A1=integer
         exg      a5,a6
         move.l   a0,d0                ; integer part (upper)
         move.l   a1,d1                ; integer part (lower)
         cmpi.b   #(OPERSEQ&OP_DIV2),d6; integer division?
         beq.s    OMZero               ; yes ...

         ; Remainder operation (dividend - integer x divisor)

         CALLSYS  IEEEDPMul            ; D0/D1=integer x divisor
         move.l   d0,d2
         move.l   d1,d3
         move.l   d4,d0                ; left operand
         move.l   d5,d1
         CALLSYS  IEEEDPSub            ; D0/D1=dividend - integer x divisor

         ; Compute number of decimal places ...

OMZero   moveq    #0,d7                ; no places reserved

OMSwap   exg      a5,a6                ; IEEE=>A5, REXX=>A6
         move.l   d0,d2                ; save result
         move.l   d1,d3

         ; Operation complete ... allocate the result string.

OMResult move.w   d7,d1                ; decimal places
         movem.l  (sp)+,d4/d5/d7       ; early restore

* =======================     DoubleToString     =======================
* Converts a double-precision operand to a 'FLOAT' string structure.
* Registers:   D1 -- decimal places
*              D2/D3 -- operand
* Return:      D0 -- error
*              D1 -- string
DoubleToString
         movea.l  a4,a0                ; environment
         move.w   d1,a3                ; save decimal places
         bsr      FindQuickOne         ; D0=A0=small string
         exg      d0,a3                ; dec places=>D0, string=>A3
         clr.l    (a0)+                ; (IVALUE)
         move.l   #NSF_DPNUM<<8,(a0)   ; ns_Length/ns_Flags/ns_Hash

         movem.l  d2/d3,DVALUE(a3)     ; install result
         move.b   d0,DECPLACE(a3)      ; decimal places?
         bne.s    1$                   ; yes ... not an integer

         ; No decimal places ... check if the result is a integer.

         move.l   d3,d1                ; lower word zero?
         bne.s    1$                   ; no ... not an integer
         move.l   d2,d0                ; load result
         bsr      DPModf               ; D0/D1=fraction  A0/A1=integer
         or.l     d0,d1                ; fractional part zero?
         bne.s    1$                   ; no ...

         ; Convert to an integer and make sure it's not too big.

         move.l   d2,d0                ; load result
         move.l   d3,d1
         bsr      DPFix                ; D0=integer D1=scale
         tst.l    d1                   ; scaling factor?
         bne.s    1$                   ; yes ... too big for LONG integer
         move.l   d0,(a3)              ; save value
         bset     #NSB_BINARY,ns_Flags(a3)

1$:      move.l   a3,d1                ; result
         moveq    #0,d0                ; clear error
         rts
