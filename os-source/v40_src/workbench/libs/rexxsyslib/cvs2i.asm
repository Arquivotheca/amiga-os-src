* == cvs2d.asm =========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ===========================     CVs2i     ============================
* Converts from NexxStr format to an integer value.  The string flags are
* updated as appropriate.
* Registers:      A0 -- string structure
* Return:         D0 -- error code or 0
*                 D1 -- integer value
STACKBF  SET      MAXINTD+MAXEXPD+3    ; stack buffer
CVs2i
         move.b   ns_Flags(a0),d1      ; load flags
         btst     #NSB_NOTNUM,d1       ; non-numeric?
         beq.s    1$                   ; no ... check further
         moveq    #ERR10_047,d0        ; default error
         rts                           ; quick exit

         ; Check for an integer value.

1$:      btst     #NSB_BINARY,d1       ; integer value?
         beq.s    2$                   ; no ...
         move.l   (a0),d1              ; (IVALUE) load value
         moveq    #0,d0                ; clear error
         rts                           ; quick exit

2$:      movem.l  d2-d4/d6/a2,-(sp)    ; delayed save
         link     a5,#-STACKBF         ; stack buffer
         movea.l  a0,a2                ; save string

         moveq    #NSF_NOTNUM,d4       ; default flags (non-numeric)
         moveq    #ERR10_047,d6        ; default error ("conversion error")

         ; Scan the string and copy the digits to the buffer area.

         moveq    #MAXINTD,d0          ; max digits
         movea.l  sp,a1                ; buffer area
         bsr.s    ScanDigits           ; D0=len D1=flags (CCR) A0=dec A1=exp
         bmi.s    5$                   ; error ... non-numeric

         IFNE     NMB_NOTNUM-7
         FAIL     'NMB_NOTNUM must be sign bit!'
         ENDC

         ; A numeric value ... check whether it's an integer.

         moveq    #NSF_NUMBER,d4       ; "numeric" flags
         move.l   a1,d3                ; exponent >= 0?
         bmi.s    5$                   ; no ... not an integer

         ; Convert and scale the number.

         move.w   a0,d2                ; save decimal places
         movea.l  sp,a0                ; buffer area
         CALLSYS  CVa2i                ; D0=value D1=scan length
         move.l   d0,d1                ; converted value
         bra.s    4$                   ; jump in

3$:      add.l    d1,d1                ; value X 2
         bvs.s    5$                   ; ... overflow
         move.l   d1,d0                ; save value
         add.l    d1,d1                ; value X 4
         bvs.s    5$                   ; ... overflow
         add.l    d1,d1                ; value X 8
         bvs.s    5$                   ; ... overflow
         add.l    d0,d1                ; value X 10
         bvs.s    5$                   ; ... overflow

4$:      subq.l   #1,d3                ; scaling required?
         bcc.s    3$                   ; yes ... loop back

         moveq    #0,d6                ; clear error
         move.l   d1,(a2)              ; (IVALUE) install value

         ; String has an integer value, but can't be flagged as 'integer'
         ; unless there are no decimal positions.

         tst.w    d2                   ; any decimal places?
         bne.s    5$                   ; yes ...
         moveq    #NSF_INTNUM,d4       ; flag as integer

         ; OR in the new attribute flags.

5$:      or.b     d4,ns_Flags(a2)      ; update string flags

         move.l   d6,d0                ; set CCR
         unlk     a5                   ; restore stack
         movem.l  (sp)+,d2-d4/d6/a2
         rts

* =========================     ScanDigits     =========================
* Scans a string of numeric digits and extracts attributes.
* Registers:      A0 -- string structure
*                 A1 -- output buffer area
*                 D0 -- maximum digits (>= 2)
* Return:         D0 -- length of output string
*                 D1 -- attribute flags (CCR)
*                 A0 -- decimal places (if number)
*                 A1 -- exponent value (if number)
NONZERO  SET      15                   ; non-zero number?
ScanDigits
         moveq    #0,d1
         move.w   ns_Length(a0),d1     ; load length
         addq.w   #ns_Buff,a0          ; advance string
         cmp.w    d0,d1                ; length < maximum?
         bcc.s    SDSlowly             ; no ... too long

         ; Attempt a quick conversion for integers.

         swap     d0                   ; save max digits
         move.w   d1,d0                ; string length
         subq.w   #1,d0                ; loop count
         beq.s    1$                   ; ... one digit

         ; Make sure there's no leading zero ...

         cmpi.b   #'0',(a0)            ; leading '0' digit?
         beq.s    2$                   ; yes ... failure

         ; Scan characters until a non-digit is found.

1$:      cmpi.b   #'0',(a0)            ; >= '0'?
         bcs.s    2$                   ; no ...
         cmpi.b   #'9',(a0)            ; <= '9'?
         bhi.s    2$                   ; no
         move.b   (a0)+,(a1)+          ; install digit
         dbf      d0,1$                ; loop back

         ; An integer found ... load flags and return.

         clr.b    (a1)                 ; install a null
         moveq    #0,d0
         movea.l  d0,a0                ; clear decimal
         movea.l  d0,a1                ; clear exponent
         move.w   d1,d0                ; full length
         moveq    #(1<<NMB_DIGITS)!(1<<NMB_INTEGER),d1
         rts

         ; Quick conversion failed ... restore registers and fall through.

2$:      addq.w   #1,d0                ; remaining length
         neg.w    d0                   ; negate it
         add.w    d1,d0                ; any digits?
         beq.s    3$                   ; no ...

         ; Install the attributes for the (non-zero) integer part.

         ori.l    #(1<<NONZERO!1<<NMB_DIGITS)<<16,d1

3$:      swap     d0                   ; restore maximum

         ; Registers: A0=scan A1=buffer D0=digits/max D1=flags/length

         STRUCTURE TempVars,0
         UWORD    DecPlaces            ; decimal places
         UWORD    PreScale             ; exponent prescale
         LABEL    TEMPVARS             ; size: 4 bytes

STACKBF  SET      TEMPVARS
SDSlowly
         movem.l  d2-d7,-(sp)
         clr.l    -(sp)                ; clear TempVars

         move.w   d0,d2                ; maximum length
         moveq    #0,d3                ; flags register
         move.w   d1,d4                ; string length
         moveq    #0,d5                ; output length
         moveq    #0,d6                ; digits counter
         moveq    #STSKIP1,d7          ; initial state

         ; Check whether any digits were found in the prescan ...

         swap     d1                   ; flags=>lower
         move.w   d1,d3                ; initial flags?
         beq.s    SDLoop               ; no ... jump in

         ; Partial scan available ... resume in 'digit' state.

         swap     d0                   ; digits=>lower
         sub.w    d0,d4                ; remaining length
         move.w   d0,d5                ; output count (all digits)
         move.w   d0,d6                ; digit count
         moveq    #STDIGIT,d7          ; enter 'digit' state
         bra.s    SDLoop               ; jump in

         ; Check for and suppress any leading zero digits.

SDCheckZero
         tst.w    d3                   ; non-zero yet?
         bmi.s    SDOutput             ; yes ...

         bset     #NMB_DIGITS,d3       ; set 'digits' flag
         beq.s    1$                   ; ... first one

         ; Previous digit was a leading zero ... back up counters.

         subq.w   #1,a1                ; back up
         subq.w   #1,d5                ; decrement count
         subq.w   #1,d6                ; decrement digits

         ; Check whether the current digit is a zero ...

1$:      tst.b    d0                   ; zero digit?
         beq.s    SDOutput             ; yes

         ; Set the 'non-zero' flag.

         bset     #NONZERO,d3          ; set 'non-zero' flag

         ; Install the output character.

SDOutput move.b   -1(a0),(a1)+         ; install character
         addq.w   #1,d5                ; advance index

         ; Begin the scan loop.

SDLoop   subq.w   #1,d4                ; any left?
         bcs      SDEnd                ; ... all done
         move.b   (a0)+,d0             ; test character

         ; Select the state based on whether the character is a digit.

         move.w   d7,d1                ; non-digit index
         subi.b   #'0',d0              ; >= '0'?
         bcs.s    1$                   ; no ...
         cmpi.b   #'9'-'0',d0          ; <= '9'?
         bhi.s    1$                   ; no ...
         addq.w   #SDNUM,d1            ; ... digit index

1$:      move.b   SDTable(pc,d1.w),d1  ; load offset
         jmp      SDTable(pc,d1.w)     ; branch to state

STSKIP1  EQU      0                    ; skip blanks before number
STDIGIT  EQU      1                    ; process digits
STFRACT  EQU      2                    ; fractional part

SDTable  dc.b     SD1NonDigit-SDTable  ; prologue
         dc.b     SD2NonDigit-SDTable  ; integer begins
         dc.b     SD3NonDigit-SDTable  ; integer ends
SDNUM    EQU      *-SDTable

         dc.b     SD1Digit-SDTable     ; first digit
         dc.b     SD2Digit-SDTable     ; integer digits
         dc.b     SD3Digit-SDTable     ; fraction digits

         ; First state ... skip blanks and look for a sign or period.

SD1NonDigit
         cmpi.b   #' '-'0',d0          ; a space?
         beq.s    SDLoop               ; yes
         cmpi.b   #PERIOD-'0',d0       ; a period?
         beq.s    SD2NonDigit          ; yes

         bset     #NMB_SIGN,d3         ; set 'sign' flag
         bne.s    SDNaN                ; error ... already set

         addq.b   #'0'-'+',d0          ; a plus sign?
         beq.s    SDLoop               ; yes ... discard it
         subq.b   #'-'-'+',d0          ; a minus sign?
         beq.s    SDOutput             ; yes ... keep it
         bra.s    SDNaN                ; ... error

         ; End of prologue ... enter the 'digit' state.

SD1Digit
         moveq    #STDIGIT,d7          ; transit to 'digit' state

         ; Digit state ... gather characters.

SD2Digit
         addq.w   #1,d6                ; bump digit count
         cmp.w    d2,d6                ; too many digits?
         bls.s    SDCheckZero          ; no ... output it

         ; Too many digits ... increment scaling count and ignore it.

         subq.w   #1,d6                ; back out count
         addq.w   #1,PreScale(sp)      ; increment scaling
         bra.s    SDLoop               ; discard digit

         ; End of integer part ... determine the state transition.

SD2NonDigit
         cmpi.b   #PERIOD-'0',d0       ; decimal point?
         bne.s    SD3NonDigit          ; no

         ; Decimal point found ... enter 'fraction' state.

         moveq    #STFRACT,d7          ; state transition
         bset     #NMB_PERIOD,d3       ; set flag
         bra.s    SDLoop               ; discard period

         ; A fractional digit ... decrement exponent prescale.

SD3Digit
         cmp.w    d2,d6                ; too many digits?
         bcc.s    SDLoop               ; yes ... ignore it

         addq.w   #1,d6                ; increment digits
         subq.w   #1,PreScale(sp)      ; decrement exponent
         addq.w   #1,(sp)              ; (Decplaces) increment decimal
         bra      SDCheckZero          ; output digit

         ; End of 'digit' or 'fraction' state ... check for an exponent.

SD3NonDigit
         cmpi.b   #'E'-'0',d0          ; 'E' exponent?
         beq.s    1$                   ; yes ...
         cmpi.b   #'e'-'0',d0          ; 'e' exponent?
         bne.s    SDTrailing           ; no ...

         ; An exponent ... convert to an integer value.

1$:      bset     #NMB_EXPE,d3         ; set 'exponent' flag
         move.l   a1,-(sp)             ; push output pointer
         CALLSYS  CVa2i                ; D0=value D1=length A0=scan
         movea.l  (sp)+,a1             ; pop output pointer
         tst.w    d1                   ; any digits?
         beq.s    SDNaN                ; no ... error

         ; Update the character count.

         sub.w    d1,d4                ; any left?
         beq.s    SDEnd                ; no

         subq.w   #1,d4                ; loop count
         addq.w   #1,a0                ; preincrement

         ; End of scan ... skip over any trailing spaces.

SDTrailing
         subq.w   #1,a0                ; back up
         moveq    #' ',d1              ; load space

1$:      cmp.b    (a0)+,d1             ; a space?
         dbne     d4,1$                ; loop back

         ; Check that all characters were processed.

         addq.w   #1,d4                ; at end?
         bne.s    SDNaN                ; no ... error

         ; End of scan ... check final attributes.

SDEnd    btst     #NMB_DIGITS,d3       ; any digits?
         bne.s    SDFinal              ; yes

         ; Error ... set the 'not a number' flag.

SDNaN    bset     #NMB_NOTNUM,d3       ; not a number

         ; Scan completed ... D0 is exponent, if any.

SDFinal  move.w   (sp)+,d4             ; (DecPlaces) pop decimal places
         move.w   (sp)+,d1             ; (PreScale) pop prescale exponent
         move.l   d0,d7                ; save exponent

         ; Check whether to round the digits.

         cmp.w    d2,d6                ; digits >= maximum?
         bcs.s    2$                   ; no ...
         move.w   d2,d0                ; maximum digits
         subq.w   #1,d0                ; required digits
         lea      -1(a1),a0            ; guard digit
         bsr      RoundDigits          ; D1=updated prescale (A1 preserved)
         bra.s    2$                   ; jump in

         ; Trim any trailing zeros and update the PreScale.

1$:      addq.w   #1,d1                ; (PreScale) increment exponent

2$:      clr.b    (a1)                 ; install a null
         subq.w   #1,d5                ; any left?
         bcs.s    3$                   ; no
         tst.w    d3                   ; non-zero number?
         bpl.s    3$                   ; no ... retain zero
         cmpi.b   #'0',-(a1)           ; trailing zero?
         beq.s    1$                   ; yes ... loop back

3$:      addq.w   #1,d5                ; restore length

         ; Check whether an exponent was decoded (in D7).

         movea.w  d1,a1                ; (PreScale) initial exponent
         btst     #NMB_EXPE,d3         ; an exponent?
         beq.s    4$                   ; no ...
         adda.l   d7,a1                ; add to prescale

         ; Now we check whether the number is an integer by the REXX
         ; "whole number" definition.  First check whether the exponent
         ; is negative ... must be a fractional number.

4$:      move.l   a1,d0                ; exponent < zero?
         bmi.s    SDOut                ; yes ... not an integer

         ; Check whether exponential notation is required.

         add.w    d5,d0                ; total length
         cmp.w    d2,d0                ; length < maximum?
         bcc.s    SDOut                ; no ... not a "whole" number

         ; A "whole number" ... set the 'NMB_INTEGER' attribute.

         bset     #NMB_INTEGER,d3      ; an integer

SDOut    movea.w  d4,a0                ; decimal places
         move.l   d5,d0                ; output length
         move.b   d3,d1                ; attribute flags (CCR)
         movem.l  (sp)+,d2-d7
         rts
