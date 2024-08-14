* == bifunc.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Main routine for Built-In function library

* Special return flags
NUMBER   EQU      16                   ; numeric return?
BOOLEAN  EQU      17                   ; booelan return?
NULLSTR  EQU      18                   ; null return?
NEWSTR   EQU      31                   ; allocate a string? (sign bit)

MAXARGS  EQU      MAXRMARG             ; argument limit
MAXCONV  EQU      4                    ; maximum numeric arguments

* ===========================     BIFunc     ===========================
* Built-In functions for REXX interpreter.  Arglist clause has the function
* name as the first token.
* Registers:   A0 -- environment
*              A1 -- clause
* Return:      D0 -- error code or 0
*              D1 -- result string
STACKBF  SET      MAXARGS*4+MAXCONV*4
BIFunc
         movem.l  d2-d7/a2-a5,-(sp)
         lea      -STACKBF(sp),sp      ; argblock
         movea.l  a0,a4                ; environment
         movea.l  cTL_Head(a1),a5      ; first token

         moveq    #0,d7                ; argument count

         ; Check whether this is a built-in function ...

         bsr      BIFList              ; function found?
         beq      BIF_E15              ; no ...

         ; Return: D4=jump offset D5=argument byte D6=conversion byte

         moveq    #$0F,d3              ; nibble mask
         and.b    d5,d3                ; minimum arguments
         lsr.b    #4,d5                ; maximum arguments

         ; Count arguments in the arglist, skipping the first (name) one,
         ; and transfer the strings to the argblock array.

         move.l   (a5),d2              ; first argument token
         movea.l  sp,a3                ; address of argblock

1$:      movea.l  d2,a2                ; token to check
         move.l   (a2),d2              ; a successor?
         beq.s    2$                   ; no ...

         cmpi.b   #MAXARGS,d7          ; argblock full?
         beq.s    101$                 ; yes
         addq.b   #1,d7                ; increment count
         move.l   TDATA(a2),d1         ; load string
         clr.l    TDATA(a2)            ; clear slot
         move.l   d1,(a3)+             ; install string
         bne.s    1$                   ; ... not NULL

         ; Argument omitted ... make sure the minimum number are available.

         cmp.b    d3,d7                ; count > minimum?
         bgt.s    1$                   ; yes
101$     bra      BIF_E17              ; ... wrong number

         ; Check for a special case:  one omitted argument.

2$:      subq.b   #1,d7                ; just one?
         bne.s    21$                  ; no
         tst.l    d1                   ; is it NULL?
         beq.s    3$                   ; yes ... zero count

21$:     addq.b   #1,d7                ; restore count

         ; Check the argument count ...

3$:      moveq    #' ',d2              ; default pad character
         cmp.b    d3,d7                ; count >= minimum?
         blt.s    101$                 ; no -- error
         cmp.b    d5,d7                ; count <= maximum?
         bgt.s    101$                 ; no -- error
         bne.s    4$                   ; less than max ...

         ; Extract the pad character, if the last argument is supplied.

         tst.l    d1                   ; last argument given?
         beq.s    4$                   ; no ...
         movea.l  d1,a2
         tst.w    ns_Length(a2)        ; length > 0?
         beq.s    4$                   ; no ...
         move.b   ns_Buff(a2),d2       ; load pad character

4$:      move.l   d2,d5                ; install pad character

         ; Check if numeric (integer) conversion required ... specified by
         ; setting the bit in the argument flags byte.  The value of the
         ; last numeric argument is left in D3.

BFConvert
         movea.l  sp,a3                ; start of argblock
         move.w   d7,d2                ; argument count
         moveq    #0,d3                ; clear value
         bra.s    2$                   ; jump in

1$:      movea.l  (a3)+,a0             ; argument string
         asr.b    #1,d6                ; flag bit set?
         bcc.s    2$                   ; no ...
         move.l   a0,d0                ; a string?
         beq.s    2$                   ; no

         bsr      CVs2i                ; D0=error D1=value
         bne      BIF_E18              ; ... error
         move.l   d1,(MAXARGS*4-4)(a3) ; store result
         move.l   d1,d3                ; save result
         bmi      BIF_E18              ; ... invalid

2$:      tst.b    d6                   ; flags left?
         dbeq     d2,1$                ; loop back

         ; Load the registers and call the function.

BIF_Jump movem.l  (sp),a0/a1           ; first/second arguments
         movea.l  ev_GlobalPtr(a4),a2  ; global data structure
         movea.l  gn_TBuff(a2),a3      ; temporary work area
         movea.l  sp,a5                ; argument block
         moveq    #0,d0
         moveq    #0,d1
         moveq    #0,d2

         ; Check the argument count to load default values ...

         cmpi.b   #2,d7                ; two arguments?
         beq.s    1$                   ; yes
         blt.s    2$                   ; one or zero ...

         movea.l  2*4(sp),a2           ; 3rd argument string
         move.l   a2,d6                ; supplied?
         beq.s    1$                   ; no ...
         move.w   ns_Length(a2),d2
         addq.w   #ns_Buff,a2          ; offset the string

1$:      move.l   a1,d6                ; second argument?
         beq.s    2$                   ; no ...
         move.w   ns_Length(a1),d1
         addq.w   #ns_Buff,a1          ; offset the string

2$:      move.l   a0,d6                ; first argument?
         beq.s    3$                   ; no ...
         move.w   ns_Length(a0),d0     ; 1st argument length
         addq.w   #ns_Buff,a0          ; offset the string

         ; Call the function ...

3$:      moveq    #0,d6                ; default return code
         jsr      BIF_Jump(pc,d4.w)    ; D6=error
         tst.l    d6                   ; an error?
         bne.s    BIF_Err              ; yes

         ; Check whether a return string needs to be prepared.

         move.l   d7,d0
         swap     d0                   ; flags=>lower
         tst.w    d0                   ; any set?
         beq.s    BIFDone              ; no

         tst.l    d7                   ; allocate a string?
         bpl.s    4$                   ; no

         ; Allocate a new string structure and copy the string into it.

         move.w   d3,d1                ; length
         bsr      BIFString            ; D0=A0=string structure
         addq.w   #ns_Buff,a0          ; destination buffer
         movea.l  a3,a1                ; source string
         exg      d0,d3                ; length=>D0, string=>D3
         CALLSYS  StrcpyN              ; D0=hash
         movea.l  d3,a0                ; string structure
         move.b   d0,ns_Hash(a0)       ; install hash byte
         bra.s    BIFDone 

4$:      btst     #NUMBER,d7           ; an integer?
         beq.s    5$                   ; no

         ; Convert an integer to a string.

         movea.l  a4,a0                ; environment
         move.l   d3,d0                ; value
         bsr      CVi2s                ; D0=A0=string
         bra.s    BIFDone 

5$:      movea.l  rl_NULL(a6),a0       ; default return
         btst     #BOOLEAN,d7          ; a boolean value?
         beq.s    BIFDone              ; no ... NULL return

         ; Load the appropriate (global) boolean value string.

         movem.l  rl_FALSE(a6),a0/a1   ; FALSE/TRUE strings
         tst.w    d3                   ; false?
         beq.s    BIFDone              ; yes ...
         movea.l  a1,a0                ; TRUE return
         bra.s    BIFDone 

         ; Error return codes 

BIF_E15  moveq    #ERR10_015,d6        ; function not found
         bra.s    BIF_Err

BIF_E17  moveq    #ERR10_017,d6        ; wrong number of arguments
         bra.s    BIF_Err

BIF_E18  moveq    #ERR10_018,d6        ; invalid argument

BIF_Err  suba.l   a0,a0                ; clear the return

         ; Loop through the argblock and release the argument strings,
         ; unless the argument is the return string.

BIFDone  movea.l  a0,a2                ; save return string
         movea.l  sp,a5                ; argblock
         bra.s    2$                   ; jump in

1$:      move.l   (a5)+,d1             ; argument string?
         beq.s    2$                   ; no
         cmp.l    a2,d1                ; return string?
         beq.s    2$                   ; yes ... don't recycle!

         movea.l  a4,a0                ; environment
         movea.l  d1,a1                ; string
         bsr      FreeString           ; release it
2$:      dbf      d7,1$                ; loop back

         movea.l  a2,a0                ; result string
         move.l   d6,d0                ; return code
         lea      STACKBF(sp),sp       ; restore stack
         movem.l  (sp)+,d2-d7/a2-a5
         rts

         ; Shared returns

BFNewstr cmp.l    MaxLen(pc),d3        ; too long?
         bgt.s    BFErr09              ; yes
         bset     #NEWSTR,d7           ; a new string
         rts

BFNumber bset     #NUMBER,d7           ; numeric return
         rts

BFBool   bset     #BOOLEAN,d7          ; boolean return
         rts

BFNull   bset     #NULLSTR,d7          ; null return
         rts

         ; Shared error returns

BFErr03  moveq    #ERR10_003,d6        ; allocation failure
         rts

BFErr09  moveq    #ERR10_009,d6        ; string too long
         rts

*BFErr15  moveq    #ERR10_015,d6        ; function not found
*         rts
*
BFErr18  moveq    #ERR10_018,d6        ; invalid operand
         rts

* ==========================     BIFString2     ========================
* Conditional allocation of 'bare' strings.  If the string has the SOURCE
* or KEEP attributes, a new string is allocated.
BIFString2
BIFMaybeString
         cmp.w    ns_Length(a0),d1        ; same length?
         bne.s    BIFString               ; no ... get new string

         move.b   ns_Flags(a0),d0         ; attribute flags
         andi.b   #NSF_SOURCE!NSF_KEEP,d0 ; owned?
         beq.s    BS001                   ; no -- return original

* ==========================     BIFString     =========================
* Private routine to allocate 'bare' strings.
* Registers:   D1 -- length
* Return:      D0 -- string structure
*              A0 -- same
BIFString
         movea.l  a4,a0                ; environment
         moveq    #NSF_STRING,d0       ; string type
         bsr      GetString            ; D0=A0=string

BS001    move.l   a0,d0
         rts

* ========================     AddArgument     =========================
* Adds a string to the argument block for recycling.
* Registers:   D0 -- string
*              D7 -- argument count
*              A5 -- argblock
AddArgument
         lsl.w    #2,d7                ; scale for 4 bytes
         move.l   d0,0(a5,d7.w)        ; install string
         lsr.w    #2,d7                ; restore count
         addq.w   #1,d7                ; increment count
         rts
