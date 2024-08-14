* == bif700.asm ========================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =============================     C2B     ============================
* Converts from character (packed) to a string of BINARY digits.
* Usage: C2B(string)
BIFc2b
         move.l   d0,d1                ; string length
         lsl.l    #3,d0                ; unpacked length
         bra.s    BF7c2x01

* =============================     C2X     ============================
* Converts from character (packed) to a string of HEX digits.
* Usage: C2X(string)
BIFc2x
         move.l   d0,d1                ; string length
         lsl.l    #1,d0                ; unpacked length
         moveq    #-1,d2               ; set 'hex' flag

BF7c2x01 move.w   #_LVOCVc2x,d4        ; function offset
         bra.s    BF7000

* =============================     B2C     ============================
* Packs a string of binary digits.
* Usage: B2C(string)
BIFb2c
         move.l   d0,d1                ; string length
         addq.l   #7,d0                ; round up
         lsr.l    #3,d0                ; packed length
         bra.s    BF7x2c01

* =============================     X2C     ============================
* Packs a string of hex digits.
* Usage: X2C(string)
BIFx2c
         move.l   d0,d1                ; string length
         addq.l   #1,d0                ; round up
         lsr.l    #1,d0                ; packed length
         moveq    #-1,d2               ; set 'hex' flag

BF7x2c01 move.w   #_LVOCVx2c,d4        ; function offset

         ; Make sure the unpacked string will fit in the work buffer.

BF7000   move.w   (ra_Length-ra_Buff)(a3),d3
         cmp.l    d3,d0                ; too long?
         bhi.s    BF7001               ; yes ... error

         ; Convert the string using the global work buffer.

         movea.l  a0,a1                ; string to convert
         movea.l  a3,a0                ; buffer area
         move.l   d2,d0                ; HEX/BINARY flag
         jsr      0(a6,d4.w)           ; D0=error D1=length
         bne.s    BF7001               ; ... error

         move.w   d1,d3                ; final length
         bra      BFNewstr             ; new string

BF7001   bra      BFErr18              ; invalid operand

* =============================     D2C     ============================
* Converts from decimal digits to a (packed) binary string.
* Usage: D2C(string,[length])
BIFd2c 
         moveq    #4,d4                ; maximum length
         moveq    #0,d5                ; no conversion
         bra.s    BF7d2x01

* =============================     D2X     ============================
* Converts from a decimal (whole) number to a string of hexadecimal digits.
* The optional length argument specifies how many digits to keep.
* Usage: D2X(string,[length])
BIFd2x
         moveq    #8,d4                ; maximum length
         moveq    #-1,d5               ; hex conversion

BF7d2x01 subq.w   #ns_Buff,a0          ; recover string
         bsr      CVs2i                ; D0=error D1=value
         bne.s    BF7001               ; ... error

         move.l   d1,(a3)              ; install value
         exg      d1,d5                ; flag=>D1, value=>D5

         move.l   d1,d0                ; convert to hex?
         beq.s    1$                   ; no

         ; Convert the decimal value to a hex digit string.

         movea.l  a3,a1                ; value buffer
         addq.w   #4,a3                ; advance pointer
         movea.l  a3,a0                ; output buffer
         moveq    #4,d1                ; four bytes
         CALLSYS  CVc2x                ; D0=(zero) D1=length A0=buffer

         moveq    #'0',d0              ; trim '0' digits

         ; Check whether the length argument was supplied ...

1$:      cmpi.b   #2,d7                ; length argument?
         beq.s    3$                   ; yes

         ; Length not specified ... trim leading nulls or '0' digits.

         not.l    d2                   ; value mask
         move.l   d4,d3                ; maximum length
         subq.w   #2,d3                ; loop count
         movea.l  a3,a0                ; value string

2$:      cmp.b    (a0)+,d0             ; trim character?
         dbne     d3,2$                ; loop back
         addq.w   #2,d3                ; ... final length

         ; Check length and advance value pointer.

3$:      sub.l    d3,d4                ; length <= maximum?
         bcs.s    BF7003               ; no ... error
         adda.l   d4,a3                ; advance pointer

         ; Make sure the value >= 0 if no length specified ...

         and.l    d2,d5                ; (masked) value >= 0?
         bpl      BFNewstr             ; yes ...

BF7003   bra      BFErr18              ; invalid operand

* ============================     X2D     =============================
* Converts a string of hex digits to the decimal equivalent.
* Usage: X2D(string,[length])
BIFx2d
         move.l   d0,d1                ; length
         addq.l   #1,d0                ; round up
         lsr.l    #1,d0                ; packed length
         cmp.w    (ra_Length-ra_Buff)(a3),d0
         bhi.s    BF7003               ; ... string too long

         ; Convert the string from hex digits to character ...

         movea.l  a0,a1                ; string
         movea.l  a3,a0                ; buffer area
         moveq    #-1,d0               ; hex conversion
         CALLSYS  CVx2c                ; D0=error D1=length A0=buffer A1=flag
         bne.s    BF7003               ; error ...

         move.l   d1,d0                ; final length
         add.w    d1,d1                ; available nibbles
         move.w   a1,d2                ; partial byte?
         beq.s    BF7c2d01             ; no
         subq.w   #1,d1                ; ... decrement count
         bra.s    BF7c2d01

* =============================     C2D     ============================
* Converts a character (packed) string to the corresponding decimal value.
* Usage: C2D(string,[length])
BIFc2d
         move.l   d0,d1
         add.l    d1,d1                ; available nibbles
         add.l    d3,d3                ; requested nibbles

         ; Check whether the length (digits) argument specified ...

BF7c2d01 moveq    #8,d2                ; maximum nibbles
         cmpi.b   #2,d7                ; length argument?
         beq.s    1$                   ; yes

         cmp.l    d2,d1                ; length <= maximum?
         bgt.s    BF7003               ; no ... error
         move.l   d2,d3                ; use maximum

1$:      cmp.l    d2,d3                ; request <= maximum?
         bgt.s    BF7003               ; no ... error

2$:      move.w   d3,d1                ; nibbles to load
         sub.w    d1,d2                ; nibble shift count
         lsl.b    #2,d2                ; ... bit shift count

         ; Check whether any null padding is required.

         adda.l   d0,a0                ; end of string
         add.l    d0,d0                ; available nibbles
         move.l   d1,d4                ; requested nibbles
         sub.l    d0,d4                ; null padding required?
         bgt.s    3$                   ; yes
         moveq    #0,d4                ; clear null count

3$:      moveq    #0,d3                ; clear result
         bra.s    7$                   ; jump in

         ; Load the selected nibbles and OR them into the result.

4$:      cmp.w    d4,d1                ; nibble >= null count?
         bcs.s    6$                   ; no

         lsr.b    #4,d5                ; rotate nibble
         not.w    d6                   ; even nibble?
         bpl.s    5$                   ; yes
         move.b   -(a0),d5             ; ... load nibble

5$:      moveq    #$0F,d0              ; nibble mask
         and.b    d5,d0                ; select nibble
         or.b     d0,d3                ; OR it in

6$:      ror.l    #4,d3                ; rotate right

7$:      dbf      d1,4$                ; loop back

         ; Adjust the result (with sign extension).

         asr.l    d2,d3                ; final shift

         moveq    #0,d6                ; clear error
         bra      BFNumber

         ; Data structure for random number generation

         STRUCTURE RandomData,0
         UWORD    rd_Index
         UWORD    rd_Tap
         UWORD    rd_Length            ; length of register
         STRUCT   rd_Reg,55*2          ; state register
         LABEL    rd_SIZEOF

* ============================     RANDOM     ==========================
* Returns a random integer within the specified interval.
* Usage: RANDOM([low],[high],[seed])
BIFrandom
         moveq    #0,d5                ; clear seed
         cmpi.b   #3,d7                ; seed argument?
         bne.s    1$                   ; no ...
         move.l   d3,d5                ; load seed argument

1$:      moveq    #0,d4                ; default lower limit
         move.l   #999,d3              ; default upper limit
         tst.l    d1                   ; high limit?
         beq.s    2$                   ; no
         move.l   (MAXARGS*4+4)(a5),d3 ; load high limit

2$:      tst.l    d0                   ; low limit?
         beq.s    3$                   ; no
         move.l   (MAXARGS*4+0)(a5),d4 ; load low limit

         ; Get the RandomData structure, initializing if necessary.

3$:      move.l   d5,d0                ; seed or 0
         movea.l  ev_GlobalPtr(a4),a0
         bsr      SetRandData          ; D0=A0=RandomData

         ; Compute the interval from low to high

         sub.l    d4,d3                ; interval
         bmi      BFErr18              ; error
         beq.s    4$                   ; degenerate
         cmpi.l   #1000,d3             ; too big?
         bgt      BFErr18              ; yes

         ; Get a random number on the interval [0,65535)

         bsr.s    NextNumber           ; D0=value

         ; Compute the scaling factor as (MAXINT/range) + 1

         move.l   #65535,d1            ; maximum integer
         addq.l   #1,d3                ; range for 0 - interval
         divu     d3,d1
         addq.w   #1,d1                ; round up

         ; Scale the number

         bsr      Ldivu                ; D0=quotient D1=remainder
         move.l   d0,d3                ; load quotient

         ; Offset result by the lower limit

4$:      add.l    d4,d3                ; offset by low limit
         bra      BFNumber

* =========================     NextNumber     =========================
* Computes the next random number.
* Registers:   A0 -- RandomData structure
* Return:      D0 -- next value in interval [0,65535]
NextNumber
         move.w   d2,-(sp)

         movea.l  a0,a1
         move.w   (a1)+,d0             ; current index
         move.w   (a1)+,d1             ; tap offset
         move.w   (a1)+,d2             ; register length
         adda.w   d0,a1                ; current value

         ; Compute the tapped index

         add.w    d0,d1                ; tapped entry
         cmp.w    d1,d2                ; wrap-around?
         bhi.s    1$                   ; no
         sub.w    d2,d1                ; shift index

         ; Calculate the new value

1$:      move.w   rd_Reg(a0,d1.w),d1   ; load tap value
         add.w    (a1),d1              ; add current value

         ; Advance the index, wrapping around if necessary.

         addq.w   #2,d0                ; advance index
         cmp.w    d0,d2                ; wrap-around?
         bhi.s    2$                   ; no
         sub.w    d2,d0                ; shift index

2$:      move.w   d0,(a0)              ; install new index
         move.w   d1,(a1)              ; install new value
         moveq    #0,d0
         move.w   d1,d0                ; return value
         move.w   (sp)+,d2
         rts

* ============================     RANDU     ===========================
* Generates random numbers on the interval [0,1).  The number of digits
* generated is determined by the current Numeric Digits setting.
* Usage: RANDU([seed])
BIFrandu:
         move.l   d3,d0                ; seed
         movea.l  a2,a0                ; global pointer
         bsr.s    SetRandData          ; D0=A0=RandomData pointer

         ; Loop for the number of Numeric Digits

         move.w   ev_NumDigits(a4),d2  ; digits to get
         move.l   d2,d3                ; digit length
         addq.w   #2,d3                ; plus '0.'
         cmpi.w   #TEMPBUFF,d3         ; too long?
         bhi      BFErr18              ; yes

         move.l   a0,d4                ; save pointer
         moveq    #'0',d5
         movea.l  a3,a2                ; output buffer
         move.w   #'0.',(a2)+          ; leading '0.'
         bra.s    2$                   ; jump in

         ; Get the random digits and copy them to the output buffer.

1$:      movea.l  d4,a0
         bsr.s    NextNumber           ; D0=value
         divu     #6554,d0             ; scale for 0-9
         add.b    d5,d0                ; ... ASCII digit
         move.b   d0,(a2)+             ; install it
2$:      dbf      d2,1$                ; loop back

         bra      BFNewstr

* =========================     SetRandData     ========================
* Initializes the random number generator.
* Registers:      A0 -- global pointer
*                 D0 -- seed or 0
* Return:         D0 -- RandomData pointer
*                 A0 -- same
SetRandData:
         move.l   d0,-(sp)
         move.l   gn_RSeed(a0),d0      ; a RandomData structure?
         bne.s    1$                   ; yes

         ; Allocate and initialize the structure

         move.l   a0,-(sp)             ; push global
         moveq    #rd_SIZEOF,d0
         bsr      FindSpace            ; D0=A0=RandomData
         movea.l  a0,a1
         clr.w    (a1)+                ; index
         move.w   #48,(a1)+            ; tap (55-31)*2
         move.w   #110,(a1)+           ; length (55*2)

         moveq    #123,d0              ; initial seed?
         bsr.s    InitRandData         ; D0=A0=RandomData

         movea.l  (sp)+,a1             ; pop global
         move.l   a0,gn_RSeed(a1)      ; save structure

         ; Already have a structure ... see if we're seeding it.

1$:      movea.l  d0,a0
         move.l   (sp)+,d0             ; a seed?
         beq.s    IRDOut               ; no

* =========================     InitRandData     =======================
* Registers:   A0 -- RandomData structure
*              D0 -- seed
* Return:      D0 -- RandomData structure
*              A0 -- same
InitRandData
         lea      rd_Length(a0),a1     ; length entry
         move.w   (a1)+,d1             ; register size
         lsr.w    #1,d1                ; ... number of entries
         bra.s    2$                   ; jump in

         ; Initialize the register using a linear congruence generator

1$:      mulu     #625,d0              ; multiply
         add.l    #6571,d0
         divu     #31104,d0
         clr.w    d0
         swap     d0                   ; get remainder
         move.w   d0,(a1)+             ; install it
2$:      dbf      d1,1$                ; loop back

IRDOut   move.l   a0,d0                ; return value
         rts
