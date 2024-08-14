* == dpmodf.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ===========================     DPModf     ===========================
* Routine to return integer and fractional parts of an IEEE DP number.
* The fractional part of the operand is masked off, and is then subtracted
* from the original value.
* Registers:   D0/D1 -- number in IEEE DP format
* Return:      D0/D1 -- fractional part
*              A0/A1 -- integer part
DPModf
         movea.l  d0,a0                ; save operand

         ; Extract the exponent ...

         bsr.s    DPExp                ; D0=exponent (registers preserved)
         bmi.s    2$                   ; number < 1.0
         cmpi.w   #EXPSHIFT-WORDLEN/2,d0
         blt.s    3$

         ; Screen the exponent based on the first non-zero half-word.

         movea.w  #EXPSHIFT+WORDLEN,a1 ; initial limit
         tst.w    d1                   ; 1st half-word non-zero?
         bne.s    1$                   ; yes
         subi.w   #WORDLEN/2,a1        ; update limit

         tst.l    d1                   ; 2nd half-word non-zero?
         bne.s    1$                   ; yes
         subi.w   #WORDLEN/2,a1

1$:      cmp.w    a1,d0                ; exponent >= limit?
         blt.s    3$                   ; no ... partly fractional

         ; An integer value ... clear the fractional part.

         movea.l  d1,a1                ; lower word
         moveq    #0,d0                ; clear fraction
         moveq    #0,d1
         rts

         ; Operand entirely fractional .. clear integer part.

2$:      moveq    #0,d0                ; clear operand
         exg      d0,a0                ; restore upper operand
         movea.l  a0,a1                ; clear lower word
         rts

         ; Quick tests failed ... do it the hard way (A1 = exponent).

3$:      movea.w  d0,a1                ; exponent
         move.l   a0,d0                ; restore register

         ; Operand has an integer and fractional part.

DPModSlowly
         movem.l  d2-d5/a6,-(sp)       ; delayed save
         move.w   a1,d2                ; exponent
         moveq    #0,d5                ; clear lower word

         cmpi.w   #EXPSHIFT,d2         ; upper part fractional?
         ble.s    1$                   ; yes ...

         ; Lower word fractional ...

         subi.w   #EXPSHIFT+1,d2       ; net shift count
         bset     #WORDLEN-1,d5        ; set sign bit
         asr.l    d2,d5                ; bitmask for lower part
         move.l   d0,d4                ; load the upper part
         and.l    d1,d5                ; AND in the lower part
         bra.s    2$

         ; Upper part fractional ... create a bitmask for it.

1$:      move.l   #SIGNBIT!EXPMASK,d4  ; load initial bitmask
         asr.l    d2,d4                ; bitmask for upper word
         and.l    d0,d4                ; AND in the upper word

         ; Subtract out the integral part ...

2$:      move.l   d4,d2                ; integer part (upper)
         move.l   d5,d3                ; integer part (lower)
         movea.l  rl_IeeeDPBase(a6),a6
         CALLSYS  IEEEDPSub            ; D0/D1=fraction

         movea.l  d4,a0                ; integer return
         movea.l  d5,a1
         movem.l  (sp)+,d2-d5/a6
         rts

* ===========================     DPExp     ============================
* Loads the exponent from an IEEE DP operand.  Only D0 is modified, and
* the condition codes reflect the state of the exponent.
* Registers:   D0/D1 -- operand in IEEE DP format
* Return:      D0 -- exponent
DPExp
         bclr     #WORDLEN-1,d0        ; clear sign
         swap     d0                   ; upper word zero?
         bne.s    1$                   ; no
         tst.l    d1                   ; lower word zero?
         beq.s    2$                   ; yes ... return zero

1$:      andi.w   #EXPMASK>>16,d0      ; mask off exponent
         lsr.w    #EXPSHIFT-WORDLEN/2,d0
         subi.w   #EXPOFFSET,d0        ; subtract bias
         ext.l    d0                   ; extend sign

2$:      rts

* ==========================     DPFix     =============================
* Truncates an operand in IEEE DP format to its integer value.  If the 
* result is too large to be expressed as a long integer, a remainder shift
* count is returned.
* Registers:      D0/D1 -- IEEE DP operand
* Return:         D0    -- integer
*                 D1    -- remainder shift count (base 2 exponent)
DPFix
         movea.l  d0,a0                ; save upper word
         bclr     #WORDLEN-1,d0        ; clear sign

         tst.l    d0                   ; upper word zero?
         bne.s    1$                   ; no
         tst.l    d1                   ; lower word zero?
         beq.s    DPFZero              ; yes

         ; Extract the exponent and see if it's positive ...

1$:      bsr.s    DPExp                ; D0=exponent (registers preserved)
         bpl.s    DPFTest              ; ... positive

         ; Zero operand or negative exponent ... return zero value.

         moveq    #0,d1                ; clear shift

DPFZero  moveq    #0,d0                ; clear operand
         rts

         ; Exponent positive ... screen for special cases.

DPFTest  move.w   d0,d1                ; save exponent

         ; Mask off the exponent and restore the "hidden" bit.

         move.l   a0,d0                ; restore operand
         andi.l   #~(SIGNBIT!EXPMASK),d0
         bset     #EXPSHIFT,d0         ; set hidden bit

         ; Compute the right-hand shift count ...

         subq.w   #EXPSHIFT-WORDLEN/2,d1
         neg.w    d1                   ; right shift >= 0?
         bpl.s    DPFFast              ; ... quickest shift
         addi.w   #WORDLEN/2,d1        ; EXPSHIFT-exponent
         bpl.s    DPFFinal             ; ... quick shift

         ; Worst case situation ... must use upper half of lower word to get
         ; maximum precision result.  Shift count is (WORDLEN-2-exponent).

         addi.w   #WORDLEN-2-EXPSHIFT,d1
         blt.s    DPFZero              ; error ... exponent too large

         ; Make space in the upper word ...

         lsl.l    #8,d0                ; make space in upper word
         lsl.l    #WORDLEN-EXPSHIFT-2-8,d0

         ; ... and align the upper part of the lower word.

         swap     d1                   ; quick shift
         lsr.w    #EXPSHIFT+2-WORDLEN/2,d1

         or.w     d1,d0                ; OR in the lower part
         swap     d1                   ; restore shift count
         bra.s    DPFFinal

         ; Exponent <= EXPSHIFT-WORDLEN/2 ... quickest shift case.

DPFFast  swap     d0                   ; shift WORDLEN/2

         ; Shift the operand over and check the sign bit ...

DPFFinal lsr.l    d1,d0                ; final shift

         move.l   a0,d1                ; operand negative?
         bpl.s    1$                   ; no ...
         neg.l    d0                   ; yes -- complement it

1$:      moveq    #0,d1                ; no scaling count
         rts

* ===========================     DPTst     ============================
* Routine to test an IEEE DP operand.  All registers are preserved.
* Registers:   D0/D1 -- IEEE DP operand
* Return:      CCR -- zero/non-zero flag
DPTst
         move.l   d0,-(sp)
         bclr     #WORDLEN-1,d0        ; clear sign
         or.l     d1,d0                ; any bits? (set CCR)
         movem.l  (sp)+,d0             ; (preserves CCR)
         rts

* ============================     DPNorm     ==========================
* Normalizes an unsigned number to IEEE double precision format.
* Registers:   D0/D1 -- extended integer to be normalized
*              A0    -- exponent (base 10)
* Return:      D0/D1 -- normalized IEEE DP number
DPNorm
         move.l   d2,-(sp)
         moveq    #WORDLEN/2,d2        ; initial shift

         swap     d0                   ; upper word used?
         beq.s    1$                   ; no
         move.w   #(EXPOFFSET+WORDLEN+EXPSHIFT),a1
         bra.s    2$

         ; Test the lower word ...

1$:      swap     d1                   ; swap halves
         move.w   d1,d0                ; upper half-word used?
         beq.s    DPNLowest            ; no

         ; Full lower operand used ... swap into place.

         move.w   #(EXPOFFSET+EXPSHIFT+WORDLEN/2),a1
         clr.w    d1                   ; clear lower part
         swap     d0                   ; swap operand

         ; Now check the upper operand to determine the shift count.

2$:      tst.w    d0                   ; upper half-word used?
         beq.s    DPNLower             ; no ...

         ; Upper half-word used ... mask nibble and load shift count.
         ; N.B. caller must prevent overflow ..

         move.b   DPNCount(pc,d0.w),d2 ; load shift
         swap     d0                   ; restore operand
         bra.s    DPNShift

         ; Shift counts indexed by nibble ...

DPNCount dc.b     5,4,3,3,2,2,2,2,1,1,1,1,1,1,1,1
         dc.b     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

         ; Only the lower half-word used ...

DPNLower swap     d0                   ; restore operand
         cmp.w    d2,d0                ; lower nibble only?
         bcc.s    DPNTest              ; no

         ; Only the lower nibble used ... use a swap for the shift.

         sub.w    d2,a1                ; update exponent
         move.b   DPNCount(pc,d0.w),d2 ; load shift
         swap     d0                   ; nibble=>upper
         swap     d1
         move.w   d1,d0                ; copy up
         clr.w    d1                   ; clear lower
         bra.s    DPNShift

         ; Only the lower half-word of lower operand used ... check for
         ; special case of integers from 0-15.

DPNLowest
         swap     d1                   ; operand=>D1 lower
         cmp.w    d2,d1                ; lower nibble only?
         bcs.s    DPNQuickest          ; yes

         move.w   d1,d0                ; install operand
         move.w   #(EXPOFFSET+EXPSHIFT),a1
         clr.w    d1                   ; clear lower part

         ; Determine which nibble (in $FFF0) is non-zero ...

DPNTest  move.w   d3,-(sp)             ; delayed save
         move.w   d0,d3                ; copy operand

         cmpi.w   #$00FF,d3            ; upper byte used?
         bls.s    1$                   ; no
         lsr.w    #WORDLEN/4,d3        ; swap byte
         subq.w   #WORDLEN/4,d2        ; update shift

1$:      cmpi.w   #$000F,d3            ; lower middle nibble?
         bls.s    2$                   ; no
         lsr.b    #WORDLEN/8,d3        ; align nibble
         subq.w   #WORDLEN/8,d2        ; update shift

         ; Have the nibble index ... add in the final shift.

2$:      add.b    DPNCount(pc,d3.w),d2
         move.w   (sp)+,d3

         ; Shift count determined ... update exponent and shift upper word.

DPNShift sub.w    d2,a1                ; update exponent
         lsl.l    d2,d0                ; shift operand
         bclr     #EXPSHIFT,d0         ; clear "hidden" bit

         ; Install the exponent (and free up register D2).

         exg      a1,d2                ; exponent=>D2, shift=>A1
         lsl.w    #EXPSHIFT-WORDLEN/2,d2
         swap     d0                   ; swap operand
         or.w     d2,d0                ; install it
         swap     d0                   ; restore operand

         ; Now check whether the lower operand needs to be shifted.

         tst.l    d1                   ; anything to do?
         beq.s    DPNScale             ; no

         exg      a1,d3                ; shift=>D3, D3=>A1
         moveq    #-1,d2               ; load mask
         rol.l    d3,d1                ; rotate operand
         lsl.l    d3,d2                ; create the mask
         not.l    d2                   ; invert it

         ; Install the partial word shifted out of the lower operand.

         and.l    d1,d2                ; mask the bits
         or.l     d2,d0                ; OR 'em in
         eor.l    d2,d1                ; knock 'em out

         move.l   a1,d3                ; quick restore
         bra.s    DPNScale

         ; External entry point if 0 <= operand <= 15 ... exponent in A0.

DPNQuick
         move.l   d2,-(sp)

         ; Integer from 0 to 15 ... load from table.

DPNQuickest
         add.w    d1,d1                ; scale for 2 bytes
         move.w   OneDigit(pc,d1.w),d0 ; load upper half
         swap     d0                   ; swap into position
         moveq    #0,d1                ; clear lower word

         ; Any prescaling required?

DPNScale move.w   a0,d2                ; decimal exponent given?
         beq.s    1$                   ; no ...

         move.l   d3,-(sp)             ; delayed save
         move.l   a6,-(sp)
         move.l   d0,d2                ; save operand
         move.l   d1,d3

         ; Scale the result by the decimal exponent multiplier.

         move.w   a0,d0                ; exponent
         bsr.s    DPPow10              ; D0/D1=multiplier
         movea.l  rl_IeeeDPBase(a6),a6 ; IEEE base
         CALLSYS  IEEEDPMul            ; D0/D1=result
         movea.l  (sp)+,a6
         move.l   (sp)+,d3

1$:      move.l   (sp)+,d2
         rts

         ; IEEE numbers from 0.0 to 15.0 (upper half-word only)

OneDigit dc.w     0                    ; IEEE 0.0
         dc.w     $3FF0                ; IEEE 1.0
         dc.w     $4000                ; IEEE 2.0
         dc.w     $4008                ; IEEE 3.0
         dc.w     $4010                ; IEEE 4.0
         dc.w     $4014                ; IEEE 5.0
         dc.w     $4018                ; IEEE 6.0
         dc.w     $401C                ; IEEE 7.0
         dc.w     $4020                ; IEEE 8.0
         dc.w     $4022                ; IEEE 9.0
         dc.w     $4024                ; IEEE 10.0
         dc.w     $4026                ; IEEE 11.0
         dc.w     $4028                ; IEEE 12.0
         dc.w     $402A                ; IEEE 13.0
         dc.w     $402C                ; IEEE 14.0
         dc.w     $402E                ; IEEE 15.0

* ============================     DPPow     ===========================
* Raises an IEEE double-precision operand to an integer power.
* Registers:   D0/D1 -- IEEE DP operand
*              A0    -- integer exponent
* Return:      D0/D1 -- result
DPPow
         tst.l    d1                   ; lower word zero?
         bne.s    DPPower              ; no
         cmp.l    DPF10(pc),d0         ; IEEE DP 10.0?
         bne.s    DPPower              ; no ...

         ; Operand is IEEE 10.0 ... attempt a fast evaluation.

         move.l   a0,d0                ; exponent

* =========================     DPPow10     ============================
* Returns a power of 10 in IEEE DP format.
* Registers:   D0 -- exponent
DPPow10
         lea      DPF1(pc),a1          ; start of array
         move.w   d0,d1                ; exponent positive?
         bmi.s    1$                   ; no

         ; Exponent positive ... check for result in array.

         cmpi.w   #9,d1                ; exponent <= 9?
         bgt.s    2$                   ; no

         ; Load the result from the array ... upper word only.

         add.w    d1,d1
         add.w    d1,d1                ; scale for 4 bytes
         adda.w   d1,a1                ; index into array
         move.l   (a1),d0              ; load upper value
         moveq    #0,d1                ; clear lower word
         rts                           ; quick exit

         ; Exponent negative ... fall through to invert value.

1$:      cmpi.w   #-9,d1               ; exponent >= -9?
         blt.s    2$                   ; no

         ; Scale for double-word entries and load value.

         add.w    d1,d1
         add.w    d1,d1
         add.w    d1,d1                ; scale for 8 bytes
         adda.w   d1,a1                ; array index
         movem.l  (a1),d0/d1
         rts

         ; Value not within array ... fall through to evaluate.

2$:      move.l   d0,a0                ; exponent
         move.l   DPF10(pc),d0         ; load IEEE 10.0
         moveq    #0,d1                ; clear lower

         ; Raises an IEEE DP operand to an integer power.

DPPower
         cmpa.w   #1,a0                ; exponent = 1?
         beq.s    DPPExit              ; yes

         movem.l  d2-d6/a2/a3/a6,-(sp)
         movea.l  rl_IeeeDPBase(a6),a6
         movea.l  d0,a2
         movea.l  d1,a3

         move.w   a0,d6                ; save exponent
         ext.l    d6                   ; positive?
         bpl.s    1$                   ; yes
         neg.w    d6                   ; complement it

         ; Initialize the product and test the low-order bit ...

1$:      move.l   d0,d4                ; initial product
         move.l   d1,d5
         lsr.w    #1,d6                ; bit set?
         bcs.s    3$                   ; yes ... jump in

         move.l   DPF1(pc),d4          ; load 1.0
         moveq    #0,d5                ; clear lower word
         bra.s    3$                   ; jump in

         ; Square the multiplier ...

2$:      move.l   a2,d0                ; load multiplier
         move.l   a3,d1
         move.l   a2,d2
         move.l   a3,d3
         CALLSYS  IEEEDPMul            ; D0/D1=result
         movea.l  d0,a2                ; update multiplier
         movea.l  d1,a3

         ; Shift right and check the low-order bit ...

         lsr.w    #1,d6                ; low-order bit set?
         bcc.s    3$                   ; no ...

         ; Accumulate the product ...

         move.l   d4,d2                ; multiply result
         move.l   d5,d3
         CALLSYS  IEEEDPMul            ; D0/D1=result
         move.l   d0,d4                ; update result
         move.l   d1,d5

3$:      tst.w    d6                   ; all done?
         bne.s    2$                   ; no ... loop back

         ; Loop finished ... check for inversion.

         move.l   d4,d0                ; load result
         move.l   d5,d1
         tst.l    d6                   ; exponent positive?
         bpl.s    4$                   ; yes ...

         move.l   d0,d2                ; load result
         move.l   d1,d3
         move.l   DPF1(pc),d0          ; load 1.0
         moveq    #0,d1                ; clear lower word
         CALLSYS  IEEEDPDiv            ; D0/D1=inverted result

4$:      movem.l  (sp)+,d2-d6/a2/a3/a6

DPPExit  rts

         ; Double-precision constants

         CNOP     0,4
DPFN9    dc.l     $3E112E0B            ; 10**-9
         dc.l     $E826D694

DPFN8    dc.l     $3E45798E            ; 10**-8
         dc.l     $E2308C39

DPFN7    dc.l     $3E7AD7F2            ; 10**-7
         dc.l     $9ABCAF48

DPFN6    dc.l     $3EB0C6F7            ; 10**-6
         dc.l     $A0B5ED8D

DPFN5    dc.l     $3EE4F8B5            ; 10**-5
         dc.l     $88E368F0

DPFN4    dc.l     $3F1A36E2            ; 10**-4
         dc.l     $EB1C432C

DPFN3    dc.l     $3F50624D            ; 10**-3
         dc.l     $D2F1A9FB

DPFN2    dc.l     $3F847AE1            ; 10**-2
         dc.l     $47AE147A

DPFN1    dc.l     $3FB99999            ; 10**-1
         dc.l     $99999999

         ; Double-precision constants (upper word only)

DPF1     dc.l     $3FF00000            ; IEEE DP 1.0 value

DPF10    dc.l     $40240000            ; IEEE DP 10.0 value

DPF102   dc.l     $40590000            ; 10**2

DPF103   dc.l     $408F4000            ; 10**3

DPF104   dc.l     $40C38800            ; 10**4

DPF105   dc.l     $40F86A00            ; 10**5

DPF106   dc.l     $412E8480            ; 10**6

DPF107   dc.l     $416312D0            ; 10**7

DPF108   dc.l     $4197D784            ; 10**8

DPF109   dc.l     $41CDCD65            ; 10**9
