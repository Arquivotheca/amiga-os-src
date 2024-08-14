* == cvs2d.asm =========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     CVs2bool     ===========================
* Converts from string operands to a boolean result.
* Registers:      A0 -- storage environment
*                 A1 -- string structure
* Return:         D0 -- error code or 0
*                 D1 -- boolean result
CVs2bool
         move.l   (a1),d0              ; (IVALUE) load value
         move.b   ns_Flags(a1),d1      ; load flags
         btst     #NSB_BINARY,d1       ; integer value?
         bne.s    3$                   ; yes ...

         ; Check if operand in FLOATing format

         btst     #NSB_FLOAT,d1        ; FLOATing format?
         bne.s    1$                   ; yes

         ; Operand in string form ... do a quick test if length = 1.

         moveq    #'0',d0              ; load '0'
         sub.b    ns_Buff(a1),d0       ; minus first character
         neg.b    d0                   ; negate it
         cmpi.w   #1,ns_Length(a1)     ; length = 1?
         beq.s    3$                   ; yes

         ; Attempt to convert the operand to numeric form ...

         movem.l  d2/d3,-(sp)          ; delayed save
         bsr.s    CVs2d                ; D0=error code D2/D3=value
         exg      d0,d2                ; upper=>D0
         exg      d1,d3                ; lower=>D1
         movem.l  (sp)+,d2/d3          ; restore (CCR preserved)
         beq.s    2$                   ; converted
         bra.s    6$                   ; ... conversion error

1$:      movem.l  DVALUE(a1),d0/d1     ; load value

         ; Test an IEEE DP operand for 0.0 or 1.0 value

2$:      tst.l    d1                   ; lower word zero?
         bne.s    6$                   ; ... error
         cmp.l    DPF1(pc),d0          ; IEEE 1.0?
         beq.s    4$                   ; yes ... TRUE
         lsl.l    #1,d0                ; upper word zero?
         beq.s    5$                   ; yes ... FALSE
         bra.s    6$                   ; error

         ; Test an integer result for 0 or 1

3$:      tst.l    d0                   ; FALSE result?
         beq.s    5$                   ; yes
         subq.l   #1,d0                ; TRUE result?
         bne.s    6$                   ; no ... error

4$:      moveq    #-1,d1               ; TRUE result
         moveq    #0,d0                ; clear error
         rts

5$:      moveq    #0,d1                ; FALSE result
         moveq    #0,d0                ; clear error
         rts

         ; Error ... not a boolean value.

6$:      moveq    #ERR10_046,d0        ; "invalid boolean"
         rts

* ===========================     CVs2d     ============================
* Converts operands from NexxStr format to IEEE DP format.
* Registers:      A0 -- storage environment
*                 A1 -- string structure
* Return:         D0 -- error code or 0
*                 D1 -- flag byte
*                 D2/D3 -- IEEE DP result
*                 A0 -- number of decimal places
CVs2d
         btst     #NSB_NOTNUM,ns_Flags(a1)
         bne.s    2$                   ; not a number ...

         ; Get the maximum number of digits to convert.

         moveq    #1,d0                ; load one
         add.w    ev_NumDigits(a0),d0  ; ... digits to convert

         ; Check whether the string is an integer.  If so, and its length
         ; is less than or equal to the current precision, we can FLOAT it.

         btst     #NSB_BINARY,ns_Flags(a1)
         beq.s    S2DStart             ; not an integer ...
         cmp.w    ns_Length(a1),d0     ; digits > length?
         ble.s    S2DStart             ; no ...

         ; An integer value ... float it using the normalizer.

         moveq    #0,d2                ; clear sign
         move.l   (a1),d1              ; load lower operand
         bpl.s    1$                   ; ... positive
         neg.l    d1                   ; negate it
         bset     #WORDLEN-1,d2        ; set sign

1$:      moveq    #0,d0                ; clear upper operand
         suba.l   a0,a0                ; clear exponent
         bsr      DPNorm               ; D0/D1=IEEE DP number
         or.l     d0,d2                ; OR into the sign
         move.l   d1,d3

         moveq    #(NMF_DIGITS!NMF_INTEGER),d1
         moveq    #0,d0                ; clear error
         movea.l  d0,a0                ; no decimal places
         rts

         ; Non-numeric ... return "conversion error".

2$:      moveq    #ERR10_047,d0        ; load error code
         rts

         ; Quick conversion failed ... max digits in D0.

S2DStart movem.l  d4/a2/a4,-(sp)       ; delayed save
         movea.l  a1,a2                ; string
         movea.l  ev_GlobalPtr(a0),a4
         movea.l  gn_TBuff(a4),a4      ; temporary buffer work area

         ; Scan the string using the temporary buffer.

         movea.l  a2,a0                ; string structure
         movea.l  a4,a1                ; work area
         bsr      ScanDigits           ; D0=len D1=flags (CCR) A0=dec A1=exp
         move.b   d1,d4                ; numeric scan flag byte
         bmi.s    S2DNaN               ; not a number ...

         ; Convert the number ... prescale is in A1.

         bset     #NSB_NUMBER,ns_Flags(a2)
         exg      a0,a4                ; buffer=>A0, decimal places=>A4
         bsr.s    CVa2d                ; D0/D1=IEEE DP number
         move.l   d0,d2                ; save converted number
         move.l   d1,d3

         ; Check whether the number can be expressed as a 4-byte integer.
         ; If so, save its value in the original string to speed up future
         ; conversions.

         tst.w    d3                   ; lower word zero?
         bne.s    3$                   ; no ... skip integer test
         cmp.w    a4,d3                ; any decimal places? (D3 zero)
         bne.s    3$                   ; yes ...
         btst     #NMB_INTEGER,d4      ; an integer number?
         beq.s    3$                   ; no ...

         bsr      DPFix                ; D0=integer D1=scale
         tst.l    d1                   ; scaling left over?
         bne.s    3$                   ; yes ... too big for a LONG integer

         ; A LONG integer ... install value and update flags.

         move.l   d0,(a2)              ; (IVALUE) save value
         bset     #NSB_BINARY,ns_Flags(a2)

3$:      move.b   d4,d1                ; flag bits
         movea.l  a4,a0                ; decimal places ...
         moveq    #0,d0                ; all OK
         bra.s    S2DExit

         ; If a string can't be converted, the 'NSB_NOTNUM' flag bit is
         ; set to prevent repeated conversion attempts.

S2DNaN   bset     #NSB_NOTNUM,ns_Flags(a2)
         moveq    #ERR10_047,d0        ; conversion error

S2DExit  movem.l  (sp)+,d4/a2/a4
         rts

* ===========================     CVa2d     ============================
* Converts a string of ASCII digits to an IEEE double-precision format.
* Scanning stops at the first character that is not part of the number.
* The string to be scanned must have been pre-processed by the digit
* scanner; no checks are made for blanks, multiple signs, etc.
* Registers:      A0 -- string to be scanned
*                 A1 -- exponent
* Return:         D0/D1 -- converted number
CVa2d
         tst.b    1(a0)                ; one digit?
         bne.s    CVa2dSlowly          ; no ...

         ; Just one digit ... convert to integer.

         moveq    #0,d0                ; clear upper word
         moveq    #'0',d1              ; load '0'
         sub.b    (a0),d1              ; subtract digit
         neg.b    d1                   ; digit index

         ; Use the "quick" normalizer entry point for table lookup.

         movea.w  a1,a0                ; exponent
         bra      DPNQuick             ; D0/D1=IEEE operand

         ; Multiple digits ... do it the slow way.

CVa2dSlowly
         movem.l  d2/d3/d6,-(sp)       ; delayed save
         moveq    #9,d1                ; "quick" limit
         moveq    #0,d2                ; upper word
         moveq    #0,d3                ; lower word
         moveq    #0,d6
         move.w   a1,d6                ; save exponent

         cmpi.b   #'-',(a0)            ; a '-' sign?
         bne.s    1$                   ; no
         addq.w   #1,a0                ; advance pointer
         bset     #WORDLEN-1,d6        ; set sign bit

         ; String has been prescanned to contain only digits ... no need
         ; for further error checking done here.

1$:      moveq    #'0',d0              ; load '0' digit
         sub.b    (a0)+,d0             ; character >= 0?
         bhi.s    4$                   ; no ... end of scan
         neg.b    d0                   ; 0-9 digit value

         ; A digit character found ... scale the partial result and add it.
         ; If possible, use the faster add instead of multiply.

         subq.w   #1,d1                ; quick digit?
         bmi.s    2$                   ; no

         add.l    d3,d3                ; result X 2
         add.l    d3,d0                ; ... plus digit
         asl.l    #2,d3                ; result X 8
         add.l    d0,d3                ; result X 10 + digit
         bra.s    1$                   ; loop back

         ; Number too large for quick scaling ... multiply it out.

2$:      movea.w  d0,a1                ; save digit
         move.l   d3,d1                ; lower word
         mulu     #10,d3               ; first partial product
         swap     d1
         mulu     #10,d1               ; second p.p.

         move.l   d2,d0                ; upper word zero?
         beq.s    3$                   ; ... skip multiply
         mulu     #10,d2               ; third p.p.
         swap     d0
         mulu     #10,d0               ; fourth p.p.
         swap     d0                   ; product => upper word
         add.l    d0,d2                ; (no overflow possible)

3$:      swap     d1
         move.w   d1,d0                ; upper word of 2nd p.p.
         move.w   a1,d1                ; install digit value
         add.l    d1,d3                ; lower sum
         addx.l   d0,d2                ; upper sum

         moveq    #-1,d1               ; restore "quick" limit
         bra.s    1$                   ; loop back

         ; Integer conversion complete ... normalize the number.

4$:      move.l   d2,d0                ; load result
         move.l   d3,d1
         movea.w  d6,a0                ; exponent
         bsr      DPNorm               ; D0/D1=IEEE number

         clr.w    d6                   ; clear lower word
         or.l     d6,d0                ; OR in sign
         movem.l  (sp)+,d2/d3/d6
         rts

* ==========================     RoundDigits     =======================
* Rounds a buffer of ASCII digits if the guard digit is greater than '4'.
* Registers:      A0 -- guard digit
*                 D0 -- required digits (> 0)
*                 D1 -- exponent
* Return:         D1 -- updated exponent
RoundDigits
         cmpi.b   #'4',(a0)            ; rounding required?
         bge.s    2$                   ; yes ... jump in
         rts

1$:      addq.b   #1,-(a0)             ; round up ...
         cmpi.b   #'9',(a0)            ; overflow?
         bls.s    3$                   ; no ... all done
2$:      move.b   #'0',(a0)            ; install a '0'
         dbf      d0,1$                ; loop back

         ; All digits overflowed ... propagate carry and adjust exponent.

         addq.b   #1,(a0)              ; carry '1' digit
         addq.w   #1,d1                ; increment exponent

3$:      rts

* ===========================     CVFalse     ==========================
* Returns the global 'FALSE' string structure.
* Return:         D0 -- FALSE string
*                 A0 -- same
CVFalse
         move.l   rl_FALSE(a6),d0      ; load global '0'
         movea.l  d0,a0
         rts

* ===========================     CVd2s     ============================
* Converts from internal binary or IEEEDP floating point to string format.
* The returned string is always either a new or static structure.
* Registers:      A0 -- storage environment
*                 A1 -- string structure
* Return:         D0 -- string structure
*                 A0 -- same
*                 D1 -- exponent length
CVd2s
         move.l   (a1),d0              ; load integer value

         ; Check whether string is in floating format ... assume integer.

         btst     #NSB_FLOAT,ns_Flags(a1)
         beq      CVi2s                ; D0=A0=string D1=exponent (cleared)

         ; Check for a 0.0 operand ... always reported as '0'.

         movem.l  DVALUE(a1),d0/d1     ; load IEEE DP value
         bsr      DPTst                ; (registers preserved)
         beq.s    CVFalse              ; ... zero value

         ; Non-zero floating result ... IEEE operand in D0/D1.

CVFloat  movem.l  d2-d7/a2-a4,-(sp)    ; delayed save
         movea.l  a0,a4                ; save environment
         movea.l  a1,a3                ; string structure
         movea.l  ev_GlobalPtr(a4),a2
         movea.l  gn_TBuff(a2),a2      ; temporary buffer area

         move.w   ev_NumDigits(a4),d2  ; maximum digits
         moveq    #1,d3                ; minimum length
         move.w   d2,d4                ; total digits
         subq.w   #1,d4                ; ... last digit position
         moveq    #0,d5                ; clear exponent length
         moveq    #0,d6                ; sign bit/decimal position
         moveq    #0,d7                ; clear offset

         IFLT     TEMPBUFF-(ENV_MAX_DIGITS+2+MAXINTD)
         FAIL     "Conversion buffer too small!"
         ENDC

         ; Convert from IEEE double-precision to ASCII digits.

         movea.l  a2,a0                ; buffer area
         movea.w  d2,a1
         addq.w   #2,a1                ; total conversion digits
         bsr      CVd2a                ; D0=sign flag D1=decimal position
         beq.s    1$                   ; non-negative ...
         addq.w   #1,d3                ; add sign character
         bset     #WORDLEN-1,d6        ; set sign bit

         ; Check whether any rounding is required.

1$:      lea      0(a2,d2.w),a0        ; guard digit
         move.w   d2,d0                ; required digits
         bsr.s    RoundDigits          ; D1=updated exponent
         move.w   d1,d6                ; save decimal position

         ; Check if exponential form required ...

         moveq    #0,d0                ; clear register
         move.b   DECPLACE(a3),d0      ; load decimal places

         cmp.w    d2,d6                ; decimal position > ndigits?
         bgt.s    CVFExp               ; yes ...
         move.w   d2,d1                ; ndigits
         neg.w    d1                   ; -ndigits
         cmp.w    d1,d6                ; decimal position < -ndigits?
         bge.s    CVFTrim              ; no ...

         ; Exponential format required ... calculate the exponent.

CVFExp   lea      1(a2,d2.w),a0        ; end of digits
         move.b   #'E',(a0)+           ; install 'E'
         moveq    #1,d5                ; update length

         move.w   d6,d0                ; decimal position
         subq.w   #1,d0                ; exponent < 0?
         bmi.s    1$                   ; yes ... automatic sign
         move.b   #'+',(a0)+           ; install plus sign
         addq.w   #1,d5                ; increment length

         ; Check which numeric format was specified.

1$:      btst     #EFB_SCI,EVFLAGS(a4) ; scientific?
         bne.s    3$                   ; yes

         ; 'ENGINEERING' format ... exponent must be a multiple of 3.

         move.w   d0,d1                ; exponent
         ext.l    d1                   ; extend to long
         divs     #3,d1                ; remainder/quotient
         swap     d1                   ; remainder=>lower (0,1,2)
         tst.w    d1                   ; remainder >= 0?
         bpl.s    2$                   ; yes
         addq.w   #3,d1                ; add offset (1,2)
2$:      sub.w    d1,d0                ; adjust exponent

         ; Convert the exponent to an integer.

3$:      sub.w    d0,d6                ; new decimal position
         ext.l    d0                   ; extend to long
         moveq    #MAXINTD,d1          ; maximum digits
         CALLSYS  CVi2a                ; D0=digits
         add.w    d0,d5                ; ... length of exponent
         moveq    #0,d0                ; clear decimal places

         ; Compute the final offset index in the digit string (may be < 0).

CVFTrim  tst.w    d6                   ; decimal position > 0?
         bgt.s    1$                   ; yes ...
         move.w   d6,d7                ; no ... offset = position - 1
         subq.w   #1,d7

         ; Trim trailing zeros from the string ... D0 has decimal places.

1$:      lea      1(a2,d4.w),a0        ; scan position
         add.w    d6,d0                ; last decimal position

2$:      cmp.w    d0,d4                ; last index < decimal position?
         blt.s    3$                   ; yes ... all done
         cmpi.b   #'0',-(a0)           ; trailing zero?
         dbne     d4,2$                ; ... decrement digits

         ; Compute the length of the result string.

3$:      move.l   d3,d1                ; default length
         add.w    d4,d1                ; plus last index
         sub.w    d7,d1                ; minus offset
         add.w    d5,d1                ; plus exponent length

         ; Check whether the string includes the decimal point.

         cmp.w    d6,d4                ; last index >= decimal position?
         blt.s    CVFString            ; no ...
         addq.w   #1,d1                ; add one for decimal point

         ; Allocate a string structure.

CVFString
         movea.l  a4,a0                ; storage environment
         moveq    #(NSF_STRING!NSF_NUMBER),d0
         movea.w  d1,a4                ; save length
         bsr      GetString            ; D0=A0=string
         exg      a0,a4                ; structure=>a4, length=>A0

         ; Copy the exponent string, if any.

         move.l   d5,d0                ; exponent to copy?
         beq.s    1$                   ; no ...
         suba.w   d0,a0                ; length excluding exponent
         lea      ns_Buff(a4,a0.w),a0  ; start of exponent area
         lea      1(a2,d2.w),a1        ; start of exponent string
         CALLSYS  StrcpyN              ; D0=hash code

1$:      lea      ns_Buff(a4),a0       ; allocated buffer area
         tst.l    d6                   ; operand positive?
         bpl.s    5$                   ; jump in
         moveq    #'-',d1              ; load '-' sign
         bra.s    4$                   ; jump in

         ; Begin the copy loop ... D7 is running index.

2$:      cmp.w    d6,d7                ; index at decimal position?
         bne.s    3$                   ; no ...
         moveq    #'.',d1              ; load decimal
         move.b   d1,(a0)+             ; insert decimal
         add.b    d1,d0                ; update hash code

         ; Insert zeros until the index is in string.

3$:      moveq    #'0',d1              ; load a '0'
         addq.w   #1,d7                ; index >= 0?
         ble.s    4$                   ; no
         move.b   (a2)+,d1             ; ... load a digit

4$:      move.b   d1,(a0)+             ; install digit
         add.b    d1,d0                ; update hash code
5$:      cmp.w    d4,d7                ; index <= last index?
         ble.s    2$                   ; yes ... loop back

         move.b   d0,ns_Hash(a4)       ; install hash code

         ; Check if the converted number is an integer.  Integer values
         ; are propagated if the new string has sufficient precision.

         moveq    #NSB_BINARY,d4       ; test bit
         btst     d4,ns_Flags(a3)      ; an integer?
         beq.s    CVFOut               ; no ...
         cmp.w    ns_Length(a3),d2     ; digits sufficient for length?
         blt.s    CVFOut               ; no ...
         move.l   (a3),(a4)            ; propagate value
         bset     d4,ns_Flags(a4)      ; set flag

CVFOut   move.l   d5,d1                ; exponent length
         move.l   a4,d0                ; return string
         movea.l  d0,a0
         movem.l  (sp)+,d2-d7/a2-a4
         rts

* ============================     CVd2a     ===========================
* Converts an IEEE DP number to an ASCII string and returns the decimal
* point position.
* Registers:      A0 -- output buffer
*                 A1 -- number of digits
*                 D0/D1 -- number in IEEE DP format
* Return:         D0 -- sign flag
*                 D1 -- decimal point position
CVd2a
         movem.l  d2-d7/a2/a5,-(sp)
         movea.l  a0,a2                ; buffer area
         movea.l  rl_IeeeDPBase(a6),a5 ; IEEE library base

         moveq    #0,d4                ; clear decimal point position
         move.l   d0,d5                ; save sign and exponent
         move.w   a1,d5                ; save digit count in lower word

         ; Prescale the number so that 1.0 <= nuumber < 10.0

         bclr     #WORDLEN-1,d0        ; clear sign bit
         bra.s    4$                   ; jump in

         ; Number > 10.0**9 ... scale downwards.

1$:      asr.w    #2,d0                ; exponent/4
         neg.l    d0                   ; reverse sense
         bra.s    3$

         ; Exponent < 0 ... scale upwards.

2$:      neg.l    d0                   ; reverse it
         asr.w    #2,d0                ; -exponent/4
         addq.w   #1,d0                ; ... plus one

         ; Get the scaling factor ... uses table lookup if possible.

3$:      sub.w    d0,d4                ; update the decimal position
         bsr      DPPow10              ; D0/D1=multiplier

         exg      a5,a6
         CALLSYS  IEEEDPMul            ; D0/D1=scaled operand
         exg      a5,a6

         ; Check the exponent to see whether any scaling is required.

4$:      move.l   d0,d2                ; save value
         move.l   d1,d3
         bsr      DPExp                ; D0=exponent
         bmi.s    2$                   ; negative ... scale it
         cmpi.w   #30,d0               ; too big?
         bgt.s    1$                   ; yes ... scale it

         ; Prescaling complete ... extract the integer part.

         move.l   d2,d0                ; load operand
         move.l   d3,d1

         moveq    #12,d6               ; maximum digits
         move.w   #_LVOCVi2a,d7        ; conversion routine

         ; Split the operand into integer and fractional parts.

CVD2A_Split
         bsr      DPModf               ; D0/D1=fraction A0/A1=integer
         move.l   d0,d2                ; save fractional part
         move.l   d1,d3

         ; Truncate the integer part (guaranteed to succeed).

         move.l   a0,d0                ; load integer part
         move.l   a1,d1
         bsr      DPFix                ; D0=integer D1=excess??

         ; Convert the integer to ASCII digits.

         move.l   d6,d1                ; maximum digits
         movea.l  a2,a0                ; buffer area
         jsr      0(a6,d7.w)           ; D0=digits A0=scan
         adda.w   d0,a2                ; advance pointer
         add.w    d0,d4                ; advance decimal point
         sub.w    d0,d5                ; any digits left?
         bls.s    4$                   ; no ... all done

         ; Now convert the fractional part.

         move.l   d2,d0                ; upper fraction
         or.l     d3,d0                ; operand zero?
         beq.s    3$                   ; yes ... complete zero fill

         moveq    #9,d6                ; preferred scale
         move.w   #_LVOCVi2az,d7       ; convert with zero-fill

         cmp.w    d5,d6                ; scale <= limit?
         ble.s    1$                   ; yes
         move.w   d5,d6                ; ... use limit

         ; Scale the fractional part ... factors from 10.0 to 10**9

1$:      move.l   d6,d0                ; exponent
         bsr      DPPow10              ; D0/D1=scaling factor
         exg      a5,a6                ; REXX=>A5, IEEE=>A6
         CALLSYS  IEEEDPMul            ; D0/D1=product
         exg      a5,a6

         sub.w    d6,d4                ; predecrement decimal point
         bra.s    CVD2A_Split          ; loop back

         ; Operand was zero ... fill out array with '0' digits.

2$:      move.b   #'0',(a2)+           ; install '0'
3$:      dbf      d5,2$                ; loop back

4$:      move.l   d4,d1                ; decimal position
         add.l    d5,d5                ; sign bit => X
         subx.l   d0,d0                ; -1 or 0
         tst.l    d0                   ; set CCR
         movem.l  (sp)+,d2-d7/a2/a5
         rts
