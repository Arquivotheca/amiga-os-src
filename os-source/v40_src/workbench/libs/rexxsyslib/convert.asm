* == convert.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ============================     CVa2i     ===========================
* Converts a string of ASCII decimal digits to a (signed) LONG integer.
* Scanning stops when a non-digit character is found or an overflow occurs.
* Registers:      A0 -- string to be converted
* Return:         D0 -- integer
*                 D1 -- number of digits scanned
*                 A0 -- pointer to last character
BIGNUM   SET      $7FFFFFFF            ; largest integer
CVa2i
         move.l   a0,-(sp)             ; push start
         moveq    #0,d0                ; clear result

         ; Check for a leading '+' or '-' sign ...

         moveq    #'+',d1              ; load plus sign
         sub.b    (a0),d1              ; plus sign?
         beq.s    1$                   ; yes
         addq.b   #'-'-'+',d1          ; minus sign?
         bne.s    2$                   ; no ... jump in

1$:      addq.w   #1,a0                ; advance pointer

2$:      moveq    #'0',d1              ; load '0' digit
         sub.b    (a0),d1              ; character >= '0'?
         bhi.s    3$                   ; no
         neg.b    d1                   ; putative digit
         cmpi.b   #'9'-'0',d1          ; <= '9'?
         bhi.s    3$                   ; no

         ; A digit found ... make sure there's room for it.

         cmpi.l   #BIGNUM/10,d0        ; result too big?         14
         bgt.s    3$                   ; yes                      8

         move.l   d0,a1                ; save result              4
         lsl.l    #3,d0                ; result X 8              14
         add.l    a1,d0                ; result X 9               8
         add.l    a1,d0                ; result X 10              8
         add.l    d1,d0                ; digit fits?              8
         bvc.s    1$                   ; yes

         move.l   a1,d0                ; restore result

         ; Conversion done ... check for negative number.

3$:      movea.l  (sp)+,a1             ; pop start
         cmpi.b   #'-',(a1)            ; minus sign?
         bne.s    4$                   ; no ...
         neg.l    d0                   ; negate value

4$:      move.l   a0,d1                ; last position
         sub.l    a1,d1                ; ... digit count
         rts

* ===========================     CVi2arg     ==========================
* Converts an integer into ASCII digits and installs it in an argstring.
* Registers:   D0 -- (long) integer
* Return:      D0 -- argstring or 0
*              A0 -- same
CVi2arg
         moveq    #12,d1               ; maximum digits
         suba.l   d1,sp                ; stack buffer

         movea.l  sp,a0                ; buffer area
         CALLSYS  CVi2a                ; D0=digits
         movea.l  sp,a0                ; string
         CALLSYS  CreateArgstring      ; D0=A0=argstring

         lea      12(sp),sp            ; restore stack
         rts

* ===========================     CVi2az     ===========================
* Converts a signed long integer to ASCII with leading zeroes.
* Registers:   A0 -- output buffer
*              D0 -- number to convert
*              D1 -- maximum digits
* Return:      D0 -- length converted
*              A0 -- new buffer position
*              A1 -- buffer pointer
CVi2az
         move.l   a0,-(sp)             ; push buffer

         ; Prefill the buffer with '0' digits ...

         movea.l  a0,a1                ; buffer area
         move.w   d1,-(sp)             ; push count
         bra.s    2$                   ; jump in

1$:      move.b   #'0',(a1)+           ; install zero fill
2$:      dbf      d1,1$                ; loop back
         move.w   (sp)+,d1             ; pop count

         move.l   a1,-(sp)             ; push end
         bsr.s    CVDigits             ; D0=digits A0=start A1=end
         movea.l  (sp)+,a1             ; pop end
         move.l   a1,d0
         sub.l    a0,d0                ; total digits
         bra.s    CV001

* ============================     CVi2a     ===========================
* Converts a signed long integer to ASCII.
* Registers:   A0 -- output buffer
*              D0 -- number to convert
*              D1 -- maximum digits (> 0)
* Return:      D0 -- length converted
*              A0 -- new buffer position
*              A1 -- buffer pointer
CVi2a
         move.l   a0,-(sp)             ; push buffer
         bsr.s    CVDigits             ; D0=digits A0=buffer A1=end

         ; Reverse the characters ...

CV001    move.l   a1,-(sp)             ; push end
         CALLSYS  StrflipN             ; reverse digits
         movea.l  (sp)+,a0             ; pop end
         clr.b    (a0)                 ; install a null

         move.l   a0,d0                ; end of buffer
         movea.l  (sp)+,a1             ; buffer pointer
         sub.l    a1,d0                ; final count
         rts

* ==========================     CVDigits     ==========================
* Converts an unsigned integer to ASCII digits and copies them in reverse
* order into the specified buffer.  The length must be 2 greater than the
* maximum number of digits.
* Registers:   A0 -- buffer
*              D0 -- integer
*              D1 -- digits to convert (> 0)
* Return:      D0 -- non-zero digits
*              A0 -- start of digits
*              A1 -- end of buffer
CVDigits
         tst.l    d0                   ; positive?
         bpl.s    0$                   ; yes

         neg.l    d0                   ; negate operand
         move.b   #'-',(a0)+           ; insert sign
         subq.w   #1,d1                ; decrement count

0$:      move.w   d2,-(sp)
         movea.l  a0,a1                ; save start
         move.w   d1,d2                ; digits
         subq.w   #1,d2                ; loop count

         ; Divide by 10 and get the remainder (A0/A1 preserved).

1$:      moveq    #10,d1               ; load divisor
         cmp.l    d1,d0                ; divisor <= dividend?
         bcs.s    2$                   ; no ... quit early

         bsr.s    Ldivu                ; D0=quotient D1=remainder
         addi.b   #'0',d1              ; digit
         move.b   d1,(a0)+             ; install it
         tst.l    d0                   ; anything more?
         dbeq     d2,1$                ; yes -- loop back
         bra.s    3$

         ; Early termination ... saves time on last digit.

2$:      addi.b   #'0',d0              ; digit
         move.b   d0,(a0)+             ; install it

3$:      move.l   a0,d0                ; end of buffer
         exg      a0,a1                ; start=>A0, end=>A1
         move.w   (sp)+,d2
         sub.l    a0,d0                ; digit count
         rts

* =============================     Ldivu     ==========================
* Divides a unsigned long integer by a 16-bit divisor.  A0/A1 are preserved.
* Registers:   D0 -- dividend
*              D1 -- divisor [0:15]
* Return:      D0 -- quotient
*              D1 -- remainder
Ldivu
         swap     d0                   ; prepare for upper division  4
*         tst.w    d0                   ; upper word zero?            4
*         bne.s    0$                   ; yes                         8
*
*         ; Upper word zero ... only one division required.
*
*         swap     d0                   ; restore dividend            4
*         divu     d1,d0                ; remainder/quotient        140
*         moveq    #0,d1                ; clear remainder             4
*         swap     d0                                                 4
*         move.w   d0,d1                ; install remainder           4
*         clr.w    d0                                                 4
*         swap     d0                   ; quotient=>lower             4
*         rts
*
*0$:
         move.w   d2,-(sp)                                           8
         move.w   d1,d2                ; save divisor                4

         moveq    #0,d1                                              4
         move.w   d0,d1                ; upper dividend              4
         beq.s    1$                   ; ... nothing to do          (8)

         ; Perform the upper division

         divu     d2,d1                ; remainder/quotient        140
         move.w   d1,d0                ; install upper quotient      4

         ; Perform the lower division.

1$:      swap     d0                   ; restore value               4
         move.w   d0,d1                ; install lower dividend      4
         divu     d2,d1                ; remainder/quotient        140
         move.w   d1,d0                ; lower quotient              4
         clr.w    d1                   ; clear lower word            4
         swap     d1                   ; return remainder            4
         move.w   (sp)+,d2                                           8
         rts

* ===========================     CVarg2s     ==========================
* Copies an argstring into internal memory and then releases it.  The
* argstring is saved in the temporary value slot in case of failure.
* Registers:   A0 -- environment
*              A1 -- argstring
* Return:      D0 -- string
*              A0 -- same
CVarg2s
         move.l   a3,-(sp)
         movea.l  ev_GlobalPtr(a0),a3  ; base environment
         move.l   a1,gn_TempValue(a3)  ; save argstring

         ; Copy the argstring to a new string structure ...

         move.w   (ra_Length-ra_Buff)(a1),d1
         moveq    #NSF_STRING,d0       ; string type
         bsr      AddString            ; D0=A0=string

         ; ... and release the argstring structure.

         movea.l  gn_TempValue(a3),a0  ; argstring
         clr.l    gn_TempValue(a3)     ; clear slot
         movea.l  d0,a3                ; save string
         CALLSYS  DeleteArgstring      ; release argstring

         move.l   a3,d0                ; return string
         movea.l  d0,a0
         movea.l  (sp)+,a3
         rts

* ===========================     CVi2s     ============================
* Converts a long integer to a NexxStr structure.
* Registers:      A0 -- storage environment
*                 D0 -- value to be converted
* Return:         D0 -- string structure
*                 A0 -- same
*                 D1 -- exponent length (cleared)
STACKBF  SET      MAXINTD+MAXEXPD+3    ; stack buffer size
CVi2s
         link     a5,#-STACKBF         ; stack buffer
         movea.l  sp,a1                ; save pointer
         move.l   d0,-(sp)             ; push value
         move.l   a0,-(sp)             ; push environment

         ; Convert the integer to an ASCII string.

         moveq    #MAXINTD,d1          ; maximum digits
         movea.l  a1,a0                ; buffer area
         CALLSYS  CVi2a                ; D0=length A1=buffer

         ; Allocate a string structure for the result.

         movea.l  (sp)+,a0             ; pop environment
         move.l   d0,d1                ; length
         moveq    #NSF_INTNUM,d0       ; string flag
         bsr      AddString            ; D0=A0=string structure
         move.l   (sp)+,(a0)           ; (IVALUE) install value

         moveq    #0,d1                ; no exponent
         unlk     a5                   ; restore stack
         rts

* ===========================     CVx2c     ============================
* Converts a string of hex or binary digits to character (packed) format.
* The return code is 0 if everything worked, or an error code otherwise.
* No length checking is performed, so the buffer must be large enough.
* Registers:   A0 -- output buffer
*              A1 -- string to be converted
*              D0 -- conversion flag: ~0=>hex 0=>binary
*              D1 -- length of string
* Return:      D0 -- error code or 0
*              D1 -- length of packed string
*              A0 -- output buffer
*              A1 -- "byte-in-progress" flag
BINSHIFT SET   1
HEXSHIFT SET   4
CVx2c
         movem.l  a0/d2-d4,-(sp)
         moveq    #-1,d2               ; initial length

         moveq    #HEXSHIFT-BINSHIFT,d3; load shift
         and.b    d0,d3                ; mask it
         addq.b   #BINSHIFT,d3         ; add binary shift
         bra.s    4$                   ; jump in

         ; Scan the input string in reverse order.

1$:      move.b   0(a1,d1.l),d0        ; character to convert

         ; Check for a blank ... allowed only at byte boundaries (d4 == 0)

         cmpi.b   #BLANK,d0            ; a blank?
         beq.s    5$                   ; yes

         CALLSYS  ToUpper              ; D0=UPPERcase
         subi.b   #'0',d0              ; >= '0'?
         bcs.s    8$                   ; no ... invalid digit
         subq.b   #2,d0                ; < 2?
         bcs.s    3$                   ; yes

         cmpi.b   #HEXSHIFT,d3         ; hex conversion?
         bne.s    8$                   ; no ... error
         subq.b   #10-2,d0             ; < 10?
         bcs.s    2$                   ; yes

         subq.b   #'A'-'0'-10,d0       ; >= 'A'?
         bcs.s    8$                   ; ... error
         subq.b   #'F'-'A',d0          ; <= 'F'?
         bhi.s    8$                   ; no -- invalid digit

         addq.b   #'F'-'A',d0          ; offset to 'F'
2$:      addq.b   #10-2,d0             ; offset to 10
3$:      addq.b   #2,d0                ; offset to 2

         ; A valid digit found ... shift first, then OR it in.

         lsl.b    d4,d0                ; shift left ...
         or.b     d0,0(a0,d2.l)        ; and OR it in
         add.b    d3,d4                ; increment shift count
         cmpi.b   #7,d4                ; byte completed?
         bls.s    6$                   ; no

         ; Start and initialize a digit

4$:      addq.l   #1,d2                ; increment length
         clr.b    0(a0,d2.l)           ; clear the digit
         moveq    #0,d4                ; clear shift count

5$:      tst.b    d4                   ; byte in progress?
         bne.s    8$                   ; yes ... error
6$:      dbf      d1,1$                ; loop back

         ; Conversion done ... check for a digit in progress.

         tst.b    d4                   ; byte in progress?
         beq.s    7$                   ; no ...
         addq.l   #1,d2                ; yes -- add one to length

         ; Reverse the digits in the buffer

7$:      move.l   d2,d0                ; length
         CALLSYS  StrflipN             ; reverse the digits

         movea.w  d4,a1                ; "byte-in-progress" flag
         move.l   d2,d1                ; converted length
         moveq    #0,d0                ; everything OK
         bra.s    9$

         ; Conversion error

8$:      moveq    #ERR10_047,d0        ; error code

9$:      movem.l  (sp)+,a0/d2-d4
         rts

* ============================     CVc2x     ===========================
* Converts a character string to hex or binary digits.
* Registers:   A0 -- output buffer
*              A1 -- character string
*              D0 -- conversion flag: ~0=>hex 0=>binary
*              D1 -- length
* Return:      D0 -- error code or 0
*              D1 -- converted length
*              A0 -- output buffer
CVc2x
         movem.l  a0/d2-d6,-(sp)

         ; Load parameters for hex conversion

         moveq    #1,d3                ; multiplier shift
         moveq    #$0F,d4              ; digit mask
         moveq    #2-1,d5              ; inner count
         moveq    #4,d6                ; inner shift

         tst.w    d0                   ; hex conversion?
         bne.s    1$                   ; yes

         ; Load parameters for binary conversion

         moveq    #3,d3                ; multiplier shift
         moveq    #$01,d4              ; digit mask
         moveq    #8-1,d5              ; inner count
         moveq    #1,d6                ; inner shift

         ; Compute the new length

1$:      move.l   d1,d0                ; length
         lsl.l    d3,d0                ; new length
         adda.l   d0,a0                ; end of buffer
         move.l   d0,-(sp)             ; push it
         bra.s    5$                   ; jump in

2$:      move.w   d5,d2                ; inner loop counter
         moveq    #0,d3                ; shift count

3$:      move.b   0(a1,d1.l),d0        ; conversion byte
         lsr.b    d3,d0                ; shift it ...
         and.w    d4,d0                ; AND with digit mask
         cmpi.b   #9,d0                ; <= 9?
         bls.s    4$                   ; yes
         addq.b   #'A'-'0'-10,d0       ; offset to 'A'
4$:      addi.b   #'0',d0              ; offset to '0'
         move.b   d0,-(a0)             ; install it
         add.b    d6,d3                ; increment shift
         dbf      d2,3$                ; loop back

5$:      dbf      d1,2$                ; loop back

         move.l   (sp)+,d1             ; pop length
         moveq    #0,d0                ; no error
         movem.l  (sp)+,a0/d2-d6
         rts
