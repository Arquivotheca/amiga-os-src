* == bif500.asm ========================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     DATATYPE     ==========================
* USAGE: DATATYPE(string,[option])
BIFdatatype
         lea      -ns_Buff(a0),a2      ; recover string
         cmpi.b   #2,d7                ; option argument?
         blt      BIFDTN               ; no ... default case

         ; Check for character option ...

         bset     #BOOLEAN,d7          ; all options are boolean
         move.l   d0,d2                ; save length
         beq.s    B5Out                ; ... nothing to check

         bclr     #LOWERBIT,d5         ; convert to uppercase
         subi.w   #'A',d5              ; >= 'A'?
         bcs.s    B5Err18              ; no
         cmpi.w   #'X'-'A',d5          ; <= 'X'?
         bhi.s    B5Err18              ; no

         move.b   B5Table(pc,d5.w),d5  ; load offset
         jmp      B5Table(pc,d5.w)     ; branch

B5Table  dc.b     BIFDTA-B5Table       ; 'A' Alphanumeric?
         dc.b     B5Bin-B5Table        ; 'B' Binary?
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     BIFDTL-B5Table       ; 'L' Lowercase?
         dc.b     BIFDTM-B5Table       ; 'M' Mixed case?
         dc.b     BIFDTN-B5Table       ; 'N' Number?
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     B5Err18-B5Table
         dc.b     BIFDTS-B5Table       ; 'S' Symbol?
         dc.b     B5Err18-B5Table
         dc.b     BIFDTU-B5Table       ; 'U' Upper?
         dc.b     B5Err18-B5Table
         dc.b     BIFDTW-B5Table       ; 'W' Whole number?
         dc.b     B5Hex-B5Table        ; 'X' Hexadecimal?
         CNOP     0,2

B5Err18  bra      BFErr18              ; invalid operand

         ; 'HEX' option:  check for string of HEX digits

B5Hex    moveq    #-1,d0               ; set 'HEX' flag
         move.w   (ra_Length-ra_Buff)(a3),d5
         lsl.l    #1,d5                ; maximum length
         bra.s    BIFdt_pack_string

         ; 'BINARY' option:  check for string of BINARY digits

B5Bin    moveq    #0,d0                ; set 'BINARY' flag
         move.w   (ra_Length-ra_Buff)(a3),d5
         lsl.l    #3,d5                ; maximum length

         ; Make sure the packed string will fit in the work buffer.

BIFdt_pack_string
         cmp.l    d2,d5                ; length <= maximum?
         bcs.s    B5Err18              ; no ... error

         ; Pack the character string (still in A0!)

         movea.l  a0,a1                ; string to scan
         movea.l  a3,a0                ; work area
         move.l   d2,d1                ; length
         CALLSYS  CVx2c                ; D0=error D1=length
         seq      d3                   ; success

B5Out    rts

         ; 'Alphanumeric' option ... accept alphabetic or digit characters.

BIFDTA   moveq    #CTF_ALPHA!CTF_DIGIT,d5
         bra.s    BIFdt_check_attribute

BIFDTL   move.b   #CTF_LOWER,d5        ; lowercase flag
         bra.s    BIFdt_check_attribute

         ; 'Mixed' option ... accept alphabetic characters of either case.

BIFDTM   moveq    #CTF_ALPHA,d5        ; alphanumeric flag
         bra.s    BIFdt_check_attribute

BIFDTU   moveq    #CTF_UPPER,d5        ; uppercase flag

         ; Check the specified attribute characters.

BIFdt_check_attribute
         subq.w   #1,d2                ; loop count
         movea.l  rl_CTABLE(a6),a1     ; attribute table

1$:      move.b   (a0)+,d3             ; test character
         move.b   0(a1,d3.w),d0        ; attribute character
         and.b    d5,d0                ; bits set?
         dbeq     d2,1$                ; loop back

         sne      d3                   ; set flag
         rts

         ; 'SYMBOL' option:  check if the string is a REXX symbol.

BIFDTS   CALLSYS  IsSymbol             ; D0=symbol code D1=length
         beq.s    1$                   ; ... not a symbol
         cmp.w    d1,d2                ; lengths match?
         seq      d3                   ; set flag

1$:      rts

         ; 'WHOLE' option ... check for an integer number under the current
         ; numeric digits setting.

BIFDTW   movea.l  a4,a0                ; storage environment
         movea.l  a2,a1                ; string to check
         bsr      CVs2d                ; D0=error D1=flags D2/D3=number

         moveq    #0,d3                ; load FALSE
         tst.l    d0                   ; converted?
         bne.s    1$                   ; no ... not a number

         ; Conversion succeeded ... check for the NMB_INTEGER attribute.

         moveq    #1<<NMB_INTEGER,d3   ; load mask
         and.l    d1,d3                ; select flag

1$:      rts

         ; Status not known ... attempt a numeric conversion.  After the
         ; conversion, one of 'NSB_NOTNUM' or 'NSB_NUMBER' will be set.

BIFdt_convert
         movea.l  a4,a0                ; storage environment
         movea.l  a2,a1                ; string to check
         bsr      CVs2d                ; D0=error D1=flags D2/D3=number

         ; Default option ... checks whether string is character or numeric,
         ; and returns both the appropriate string and the boolean flag.
         ; No conversion is necessary if attribute bits are already set.

BIFDTN   move.b   ns_Flags(a2),d1      ; load attributes
         moveq    #0,d3                ; load FALSE
         lea      BIFpCHR(pc),a0       ; default 'CHAR' string
         btst     #NSB_NOTNUM,d1       ; non-numeric?
         bne.s    1$                   ; yes ... all done

         lea      BIFpNUM(pc),a0       ; default 'NUM' string
         btst     #NSB_NUMBER,d1       ; numeric?
         beq.s    BIFdt_convert        ; no ... loop back

         ; A numeric value ... set the boolean flag.

         moveq    #-1,d3               ; load TRUE

1$:      rts

* ===========================     SYMBOL     ===========================
* Tests whether a name is a valid symbol and returns {BAD | LIT | VAR}.
* Usage: SYMBOL(name)
BIFsymbol
         moveq    #BIF512-BIF512,d4
         bra.s    BIF511

* ===========================      VALUE     ===========================
* Returns the value associated with a symbol name.
* Usage: VALUE(name)
BIFvalue
         moveq    #BIF513-BIF512,d4

BIF511   lea      -ns_Buff(a0),a2      ; save symbol string
         move.l   d0,d2                ; save length

         ; Check whether the argument is a valid symbol ...

         CALLSYS  IsSymbol             ; D0=type D1=length
         move.b   d0,d5                ; save type
         move.w   d1,d3                ; save length
         cmp.w    d2,d3                ; lengths match?
         bne.s    3$                   ; no ...

         ; Convert the symbol string to UPPERcase, then check for value.
         ; N.B. The uppercase string replaces the original in the argblock,
         ; since if a copy had to be made, the original string doesn't need
         ; to be recycled.  The stem and compound strings are stored in the
         ; argblock for recycling.

         movea.l  a4,a0                ; storage environment
         movea.l  a2,a1                ; symbol name
         bsr      UpperString          ; D0=A1=string
         move.l   a1,(a5)              ; install in argblock

         moveq    #0,d0                ; clear compound string
         moveq    #-1,d1               ; set literal flag

         subi.b   #T_SYMFIXED,d5       ; a FIXED symbol?
         beq.s    3$                   ; yes ...
         subq.b   #T_SYMCMPD-T_SYMFIXED,d5
         bne.s    2$                   ; not compound ...

         ; Expand the compound symbol for the look-up ...
         ; N.B. Can't we use LookUpValue here??

         movea.l  a4,a0                ; environment
         CALLSYS  SymExpand            ; D0=error A1=stem D1=compound
         move.l   d0,d6                ; error?
         bne.s    BIF599               ; yes ...

         ; Check whether the compound value is the original string ...

         move.l   d1,d0                ; compound name
         cmp.l    (a5),d0              ; same string?
         beq.s    1$                   ; yes
         bsr      AddArgument          ; D7=new count (D0/A1 preserved)

1$:      exg      d0,a1                ; stem=>D0, compound=>A1
         bsr      AddArgument          ; D7=new count (D0/A1 preserved)
         exg      d0,a1                ; compound=>D0, stem=>A1

         ; Symbol value look-up ...

2$:      movea.l  a4,a0                ; environment
         moveq    #0,d1                ; clear node
         bsr      GetValue             ; D0=A0=node D1=literal A1=value

3$:      movea.l  a1,a0                ; symbol value
         cmp.w    d2,d3                ; symbol lengths match?
         jmp      BIF512(pc,d4.w)      ; (split trace)

         ; 'SYMBOL' function:  return description string

BIF512   lea      BIFpBAD(pc),a0       ; default return: BAD
         bne.s    1$                   ; not a symbol ...
         lea      BIFpLIT(pc),a0       ; default return: LIT
         tst.w    d1                   ; a literal?
         bne.s    1$                   ; yes ...
         lea      BIFpVAR(pc),a0       ; no -- return VAR
1$:      rts

         ; 'VALUE' function:  return value string

BIF513   beq.s    BIF599               ; all OK ...
         moveq    #ERR10_031,d6        ; not a symbol ...

BIF599   rts

         ; Static string structures ...

         CNOP     0,4
BIFpBAD  dc.l     0
         dc.w     3
         dc.b     NSF_SOURCE!NSF_NOTNUM!NSF_STRING
         dc.b     199,'BAD',0,0,0,0,0

BIFpLIT  dc.l     0
         dc.w     3
         dc.b     NSF_SOURCE!NSF_NOTNUM!NSF_STRING
         dc.b     233,'LIT',0,0,0,0,0

BIFpVAR  dc.l     0
         dc.w     3
         dc.b     NSF_SOURCE!NSF_NOTNUM!NSF_STRING
         dc.b     233,'VAR',0,0,0,0,0

BIFpNUM  dc.l     0
         dc.w     3
         dc.b     NSF_SOURCE!NSF_NOTNUM!NSF_STRING
         dc.b     240,'NUM',0,0,0,0,0

BIFpCHR  dc.l     0
         dc.w     4
         dc.b     NSF_SOURCE!NSF_NOTNUM!NSF_STRING
         dc.b     30,'CHAR',0,0,0,0
