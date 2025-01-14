* == bif100.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ============================     ABS     =============================
* Computes the absolute value of the argument.
* USAGE: ABS(number)
BIFabs
         lea      -ns_Buff(a0),a3      ; recover string
         movea.l  a4,a0                ; environment
         movea.l  a3,a1
         moveq    #OP_GE,d0            ; operand >= 0?
         move.l   rl_FALSE(a6),d1      ; use FALSE as 0
         CALLSYS  EvalOp               ; D0=error D1=result
         bne.s    1$                   ; ... error

         ; Check whether the operand was converted successfully ...

         btst     #NSB_NOTNUM,ns_Flags(a3)
         bne      BFErr18              ; error ... not numeric

         movea.l  a3,a0                ; original argument
         cmp.l    rl_TRUE(a6),d1       ; argument >= 0?
         beq.s    1$                   ; yes ... return original

         ; Argument less than zero ... negate it.

         movea.l  a4,a0                ; environment
         move.b   #OP_NEG,d0           ; negation operator (prefix)
         move.l   a3,d1                ; argument
         CALLSYS  EvalOp               ; D0=error D1=result
         beq.s    2$                   ; success ... convert result

1$:      move.l   d0,d6                ; load error
         rts

         ; Conversion always required ... add intermediate to argblock.

2$:      move.l   d1,d0                ; intermediate
         bsr      AddArgument          ; D7=new count

         ; Convert the result to a string.

         movea.l  a4,a0                ; environment
         movea.l  d1,a1                ; operand
         bra      CVd2s                ; D0=A0=string

* ============================     TRUNC     ===========================
* Truncates a number to the specified number of decimal places.
* USAGE: TRUNC(number,[places])
BIFtrunc
         moveq    #0,d4                ; clear index
         moveq    #'.',d5              ; load period

         ; Convert the first argument to a number.

         move.b   #OP_SAME,d0          ; force conversion
         subq.w   #ns_Buff,a0          ; first argument
         move.l   a0,d1                ; use as right operand
         movea.l  a4,a0                ; environment
         CALLSYS  EvalOp               ; D0=error D1=result
         exg      d0,d1                ; result=>D0, error=>D1
         move.l   d1,d6                ; conversion error?
         bne.s    8$                   ; yes

         ; Append the intermediate operand to the argument block.

         bsr      AddArgument          ; D7=new count
         movea.l  d0,a3                ; new operand

         ; Check whether the intermediate operand needs to be converted.

         btst     #NSB_STRING,ns_Flags(a3)
         bne.s    2$                   ; ... no conversion
         movea.l  a4,a0                ; environment
         movea.l  a3,a1                ; operand string
         bsr      CVd2s                ; D0=string D1=exponent length
         cmp.l    a3,d0                ; same string?
         beq.s    1$                   ; yes

         ; Append the converted intermediate to the argument block.

         bsr      AddArgument          ; D7=new count (D1 preserved)
         movea.l  d0,a3                ; new operand

1$:      tst.l    d1                   ; an exponent?
         bne      BFErr18              ; yes ... error

         ; Determine the length of the integer part of the operand.

2$:      lea      ns_Buff(a3),a2       ; start of string
         move.w   ns_Length(a3),d0     ; string length
         subq.w   #1,d0                ; loop count
         movea.l  a2,a0                ; scan pointer

         ; Scan the string for the decimal point.

3$:      cmp.b    (a0)+,d5             ; a period?
         dbeq     d0,3$                ; loop back
         bne.s    31$                  ; ... not found
         subq.w   #1,a0                ; back up

31$:     move.l   a0,d2                ; end of integer
         sub.l    a2,d2                ; ... integer length

         ; Compute the final length.

         tst.l    d3                   ; any decimal places?
         beq.s    4$                   ; no
         addq.l   #1,d3                ; ... plus decimal point
4$:      add.l    d2,d3                ; add integer part

         cmp.l    MaxLen(pc),d3        ; too long?
         bgt      BFErr09              ; yes

         ; Allocate a string structure for the result.

         move.l   d3,d1                ; final length
         bsr      BIFString            ; D0=A0=string

         ; Install the result in the new string structure.

         moveq    #0,d0                ; clear hash
         bra.s    7$                   ; jump in

5$:      move.b   (a2)+,d1             ; load character
         cmp.w    ns_Length(a3),d4     ; at end?
         bcs.s    6$                   ; no
         moveq    #'.',d1              ; load '.'
         cmp.w    d2,d4                ; end of integer?
         beq.s    6$                   ; yes
         moveq    #'0',d1              ; load '0'

6$:      move.b   d1,ns_Buff(a0,d4.l)  ; install character
         add.b    d1,d0                ; accumulate hash
         addq.w   #1,d4                ; advance index
7$:      dbf      d3,5$                ; loop back
         move.b   d0,ns_Hash(a0)       ; install hash

8$:      rts

* ============================     MIN     =============================
* Selects the minimum operand from the arguments.
* Usage: MAX(number1,number2,[number3...])
BIFmin
         moveq    #OP_LE,d2            ; comparison opcode
         bra.s    BIF101

* ============================     MAX     =============================
* Selects the maximum operand from the arguments.
* Usage: MAX(number1,number2,[number3...])
BIFmax
         moveq    #OP_GE,d2            ; comparison opcode

BIF101   movea.l  ev_GlobalPtr(a4),a2  ; global pointer (A2 not valid)
         move.w   d7,d3                ; argument count (>= 2)
         subq.w   #1,d3                ; loop count
         move.b   #OP_SAME,d0          ; conversion opcode (first iteration)
         moveq    #0,d5                ; clear index

1$:      move.l   0(a5,d5.w),d1        ; test operand?
         beq.s    BF1001               ; no
         movea.l  a4,a0                ; environment
         movea.l  a3,a1                ; pivot operand (second iteration)
         move.l   d1,d6                ; save operand
         CALLSYS  EvalOp               ; D0=error D1=result
         movea.l  d6,a0                ; test operand
         move.l   d0,d6                ; an error?
         bne.s    4$                   ; yes ...

         btst     #NSB_NOTNUM,ns_Flags(a0)
         bne.s    BF1001               ; error ... not numeric

         ; Check whether to replace the pivot operand.

         tst.w    d5                   ; first iteration?
         beq.s    2$                   ; yes ...
         cmp.l    rl_TRUE(a6),d1       ; pivot operand still extreme?
         beq.s    3$                   ; yes ...

2$:      move.w   d5,d4                ; pivot operand index
         movea.l  0(a5,d4.w),a3        ; pivot operand

3$:      addq.w   #4,d5                ; advance test index
         move.b   d2,d0                ; comparison opcode
         dbf      d3,1$                ; decrement and loop back

         ; Return the selected operand (no need to clear argblock entry).

         movea.l  a3,a0                ; result operand

4$:      rts

BF1001   bra      BFErr18              ; invalid operand

* ============================     SIGN     ============================
* Tests the argument and returns {-1,0,1} if negative, zero, or positive.
* Usage: SIGN(x)
BIFsign
         lea      -ns_Buff(a0),a3      ; recover operand string
         moveq    #-1,d3               ; default return
         moveq    #OP_LT,d0            ; initial test opcode

1$:      movea.l  a4,a0                ; environment
         movea.l  a3,a1                ; left operand
         move.l   rl_FALSE(a6),d1      ; use global FALSE as 0
         CALLSYS  EvalOp               ; D0=error D1=result

         ; Make sure the operand was numeric ...

         btst     #NSB_NOTNUM,ns_Flags(a3)
         bne.s    BF1001               ; error ... not a number

         cmp.l    rl_TRUE(a6),d1       ; TRUE result?
         beq.s    BF1002               ; yes -- all done
         moveq    #OP_EQ,d0            ; next opcode to test
         addq.l   #1,d3                ; advance result
         beq.s    1$                   ; loop back?

BF1002   bra      BFNumber

* =============================     ARG     ============================
* Usage: ARG([number,[{E | O}]])
BIFarg
         moveq    #1,d4                ; default argument
         tst.b    d7                   ; argument number?
         beq.s    1$                   ; no
         move.l   d3,d4                ; specified argument
         beq.s    BF1001               ; ... error

         ; Scan the token list and count the number of arguments.

1$:      movea.l  ev_ArgList(a4),a1    ; arglist clause
         movea.l  cTL_Head(a1),a1      ; first token (function name)
         move.l   (a1),d1              ; first argument token
         moveq    #0,d3                ; clear count
         bra.s    3$                   ; jump in

2$:      addq.w   #1,d3                ; increment count
         cmp.l    d3,d4                ; selected argument?
         bne.s    3$                   ; no
         move.l   TSTRING(a1),d2       ; ... save string
3$:      movea.l  d1,a1                ; current token
         move.l   (a1),d1              ; a successor?
         bne.s    2$                   ; yes

         ; Check for special case: one token without an argument string.

         tst.l    d2                   ; argument exists?
         bne.s    4$                   ; yes
         subq.w   #1,d3                ; one argument?
         beq.s    4$                   ; yes ... zero count
         addq.w   #1,d3                ; ... restore count

4$:      cmpi.b   #1,d7                ; arguments specified?
         blt.s    BF1002               ; no ... return count
         bgt.s    6$                   ; check option

         ; Return the selected argument string, copying if necessary.

         tst.l    d2                   ; argument exists?
         beq      BFNull               ; no ... null return
         movea.l  d2,a1                ; argument string
         tst.b    ns_Flags(a1)         ; 'SOURCE' attribute?
         bmi.s    5$                   ; yes ... no copy needed
         movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=string

5$:      movea.l  a1,a0                ; returned string
         rts

         ; 'Exists' / 'Omitted' options ... return boolean existence flag.

6$:      moveq    #0,d3                ; clear flag
         tst.l    d2                   ; argument exists?
         sne      d3                   ; set default return

         bclr     #LOWERBIT,d5         ; uppercase
         cmpi.b   #'E',d5              ; 'Exists' option?
         beq.s    7$                   ; yes ...
         cmpi.b   #'O',d5              ; 'Omitted' option?
         bne      BFErr18              ; no -- error
         not.b    d3                   ; invert flag

7$:      bra      BFBool               ; boolean return

* ============================     TRACE     ===========================
* Retrieves the current TRACE setting and (optionally) sets a new mode.
* Usage: TRACE([options])
BIFtrace
         movea.l  a0,a2                ; save string
         move.l   d0,d2                ; save length

         ; Build a string with the current trace option ...

         movea.l  a3,a1                ; buffer area
         move.b   TRACEOPT(a4),d0      ; current trace mode
         btst     #EXB_ACTIVE,d0       ; interactive tracing?
         beq.s    1$                   ; no ...
         move.b   #'?',(a3)+           ; yes -- '?' prefix

1$:      btst     #EXB_NOCOMM,d0       ; command inhibition?
         beq.s    2$                   ; no ...
         move.b   #'!',(a3)+           ; yes -- '!' prefix

2$:      andi.w   #TRACEMASK,d0        ; trace option
         move.b   TRTable(pc,d0.w),(a3)+

         movea.l  a4,a0                ; environment
         move.l   a3,d1                ; end of buffer
         sub.l    a1,d1                ; ... length
         moveq    #NSF_ALPHA,d0        ; string flags
         bsr      AddString            ; D0=A0=string

         tst.b    d7                   ; any arguments?
         beq.s    3$                   ; no ... return option

         ; Update the tracing options ...

         movea.l  a0,a3                ; save string
         bsr      AddArgument          ; D7=new count
         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; options string
         move.l   d2,d1                ; length
         bsr      SetTrace             ; D0=error
         move.l   d0,d6                ; return code
         movea.l  a3,a0                ; return option string

3$:      rts

TRTable  dc.b     'OCEILRNASB'         ; codes for trace options
         CNOP     0,2

* ============================     FORM     ============================
* Returns the current NUMERIC FORM setting.
* USAGE: FORM()
BIFform
         lea      pSCI(pc),a3          ; SCIENTIFIC format
         moveq    #10,d3               ; load length
         btst     #EFB_SCI,EVFLAGS(a4) ; SCIENTIFIC?
         bne.s    1$                   ; yes
         lea      pENG(pc),a3          ; ENGINEERING
         moveq    #11,d3               ; load length

1$:      bra      BFNewstr

* ============================     FUZZ     ============================
* Returns the current NUMERIC FUZZ setting.
* USAGE: FUZZ()
BIFfuzz
         move.w   ev_NumFuzz(a4),d0
         bra.s    BIFlength            ; load 'fuzz'

* ===========================     DIGITS     ===========================
* Returns the current NUMERIC DIGITS setting.
* USAGE: DIGITS()
BIFdigits
         move.w   ev_NumDigits(a4),d0  ; load 'digits'

* ===========================     LENGTH     ===========================
* Returns the length of a string.
* USAGE: LENGTH(string)
BIFlength
         move.w   d0,d3                ; string length

BF1003   bra      BFNumber             ; numeric return

* ============================     HASH     ============================
* Returns the hash code of a string.
* USAGE: HASH(string)
BIFhash
         move.b   (ns_Hash-ns_Buff)(a0),d3
         bra.s    BF1003

* ==========================     ERRORTEXT     =========================
* Returns the error message associated with a Rexx return code.
* USAGE: ERRORTEXT(error)
BIFerrortxt
         move.l   d3,d0                ; error code
         jmp      _LVOErrorMsg(a6)     ; D0=flag A0=string

* ===========================     ADDRESS     ==========================
* Returns the current Host Address (environment).
* USAGE: ADDRESS()
BIFaddress
         movea.l  ev_COMMAND(a4),a0    ; current host address
         rts

* ============================     BITOR     ===========================
* Usage: BITOR(string1,[string2],[pad])
BIFbitor
         moveq    #BIF104-BIF104,d4    ; jump offset for OR
         bra.s    BIF102

* ============================     BITXOR    ===========================
* Usage: BITXOR(string1,[string2],[pad])
BIFbitxor
         moveq    #BIF105-BIF104,d4    ; jump offset for XOR
         bra.s    BIF102

* ===========================     BITAND     ===========================
* Usage: BITAND(string1,[string2],[pad])
BIFbitand
         moveq    #BIF106-BIF104,d4    ; jump offset for AND

BIF102   cmp.w    d0,d1                ; 2nd length <= 1st length?
         bls.s    1$                   ; yes
         exg      a0,a1                ; no ... interchange operands
         exg      d0,d1

1$:      movea.l  a0,a2                ; save strings
         movea.l  a1,a3
         move.w   d0,d2                ; save lengths
         move.w   d1,d3

         ; Check whether the string is already owned.
         ; N.B. EXTernal strings may be directly modified here.

         lea      -ns_Buff(a0),a1      ; string structure
         move.b   ns_Flags(a1),d1      ; attribute flags
         andi.b   #(NSF_SOURCE!NSF_KEEP),d1
         beq.s    2$                   ; ... not owned

         ; String owned ... make a copy.

         movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=string
         lea      ns_Buff(a1),a2       ; new buffer

2$:      moveq    #0,d1                ; clear hash
         bra.s    BIF108               ; jump right in

BIF103   move.b   d5,d0                ; pad character
         subq.l   #1,d3                ; any left?
         bmi.s    1$                   ; no ...
         move.b   (a3)+,d0             ; load character

1$:      jmp      BIF104(pc,d4.w)      ; branch for operation

BIF104   or.b     d0,(a2)              ; OR strings
         bra.s    BIF107

BIF105   eor.b    d0,(a2)              ; XOR strings
         bra.s    BIF107

BIF106   and.b    d0,(a2)              ; AND strings

BIF107   add.b    (a2)+,d1             ; accumulate hash code

BIF108   dbf      d2,BIF103            ; decrement and loop
         move.b   d1,ns_Hash(a1)       ; install hash
         movea.l  a1,a0
         rts

* ============================     BITSET     ==========================
* Sets the specified bit in a string.
* Usage: BITSET(string,bit)
BIFbitset
         moveq    #BIF113-BIF113,d4    ; jump offset for SET
         bra.s    BIF112

* ============================     BITCHG    ===========================
* Usage: BITCHG(string,bit)
BIFbitchg
         moveq    #BIF114-BIF113,d4    ; jump offset for CHG
         bra.s    BIF112

* ============================     BITCLR     ==========================
* Usage: BITCLR(string,bit)
BIFbitclr
         moveq    #BIF115-BIF113,d4    ; jump offset for CLR
         bra.s    BIF112

* ============================     BITTST    ===========================
* Usage: BITTST(string,bit)
BIFbittst
         moveq    #BIF118-BIF113,d4    ; jump offset for TST
         moveq    #0,d5                ; clear flag

BIF112   move.l   d3,d1                ; bit position
         lsr.l    #3,d1                ; byte index
         cmp.l    d0,d1                ; index < length?
         bge      BFErr18              ; no -- error

         move.w   d0,d2                ; operand length
         sub.w    d1,d2                ; byte offset from end
         subq.w   #1,d2
         andi.b   #$07,d3              ; mask position

         ; Check whether to new string is required ...

         lea      -ns_Buff(a0),a1      ; string structure
         tst.w    d5                   ; BITTST function?
         beq.s    1$                   ; yes -- new string not needed

         ; Check whether to copy the string structure ...

         move.b   ns_Flags(a1),d1      ; attribute flags
         andi.b   #(NSF_SOURCE!NSF_KEEP),d1
         beq.s    1$                   ; ... not owned
         movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=string

         ; Load the test byte and adjust the hash code ...

1$:      movea.l  a1,a0                ; return string
         addq.w   #ns_Hash,a1          ; index to hash byte
         move.b   (a1)+,d1             ; initial hash code
         adda.l   d2,a1                ; index to test byte
         move.b   (a1),d0              ; load test byte
         sub.b    d0,d1                ; back out test byte
         jmp      BIF113(pc,d4.w)      ; branch for operation

BIF113   bset     d3,d0                ; set bit
         bra.s    BIF116

BIF114   bchg     d3,d0                ; toggle bit
         bra.s    BIF116

BIF115   bclr     d3,d0                ; clear bit

BIF116   move.b   d0,(a1)              ; install the new byte
         add.b    d0,d1                ; update hash code
         move.b   d1,ns_Hash(a0)       ; install hash
         rts

         ; Test the selected bit and return a boolean result.

BIF118   btst     d3,d0                ; bit set?
         sne      d3                   ; set flag
         ext.w    d3                   ; extend
         ext.l    d3                   ; extend
         bra      BFBool               ; boolean return
