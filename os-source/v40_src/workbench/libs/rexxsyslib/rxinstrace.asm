* == rxinstrace.asm ====================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     rxInsTRACE     =========================
* Implements the 'TRACE' instruction.
* [instruction module environment]
rxInsTRACE
         movea.l  a0,a2                ; check token
         moveq    #EXB_ACTIVE,d2       ; load flag

         ; Test for a numeric trace option ...

         move.l   a1,d1                ; a result?
         beq.s    2$                   ; no
         movea.l  a1,a0                ; result string
         bsr      CVs2i                ; D0=error D1=value
         bne.s    1$                   ; ... conversion error

         btst     d2,d4                ; interactive tracing?
         beq.s    5$                   ; no ... ignore option
         move.l   d1,ev_TraceCnt(a4)   ; set inhibition count
         bra.s    5$

         ; Check for trace options or modifiers ...

1$:      movea.l  TSTRING(a2),a1       ; load string
         move.w   ns_Length(a1),d1     ; null?
         addq.w   #ns_Buff,a1          ; advance pointer
         bne.s    3$                   ; ... not null

         ; No option specified ... revert to 'Normal' traacing.

2$:      lea      Normal(pc),a1        ; 'N'
         moveq    #1,d1                ; load length

         ; Check for interactive or 'SCAN' mode ... the trace mode is not
         ; changed unless this is a 'DEBUG' mode instruction.

3$:      btst     #EXB_DEBUG,d4        ; in 'DEBUG' mode?
         bne.s    4$                   ; yes
         btst     d2,d4                ; interactive tracing?
         bne.s    5$                   ; yes ... no change
         subq.b   #TRACE_S,d7          ; 'SCAN' mode?
         beq.s    5$                   ; yes -- no change

         ; Scan the options string and set the new trace mode.

4$:      movea.l  a4,a0                ; environment
         bsr.s    SetTrace             ; D0=error
         bne.s    7$                   ; ... error

         ; Check that the clause is properly terninated.

5$:      movea.l  a2,a0                ; check token
         btst     #ISB_SKIP1ST,d5      ; skip first token?
         beq.s    6$                   ; no
         movea.l  (a2),a0              ; ... use next token

6$:      bra      rxCheckSTOP

7$:      move.l   d0,d6                ; return error
         rts

* ==========================     SetTrace     ==========================
* Sets the trace mode to that specified in options string.
* Registers:   A0 -- storage environment
*              A1 -- options string
*              D1 -- length
* Return:      D0 -- error code
SetTrace
         lea      TRACEOPT(a0),a0      ; trace byte
         moveq    #0,d0                ; clear code

1$:      subq.w   #1,d1                ; any left?
         bcs.s    4$                   ; no
         move.b   (a1)+,d0             ; test character
         bsr      ToUpper              ; D0=uppercase

         subi.b   #'!',d0              ; command inhibition?
         bne.s    2$                   ; no ...
         bchg     #EXB_NOCOMM,(a0)     ; toggle mode
         bra.s    1$                   ; loop back

2$:      subi.b   #'?'-'!',d0          ; interactive trace?
         bne.s    3$                   ; no ...
         bchg     #EXB_ACTIVE,(a0)     ; toggle mode
         beq.s    1$                   ; ... entering mode
         clr.l    (ev_TraceCnt-TRACEOPT)(a0)
         bra.s    1$                   ; loop back

         ; Check for an "alphabetic" option ...

3$:      subq.b   #'A'-'?',d0          ; >= 'A'?
         bcs.s    5$                   ; no ... error
         cmpi.b   #'S'-'A',d0          ; <= 'S'?
         bhi.s    5$                   ; no ... error
         move.b   TrCodes(pc,d0.w),d0  ; valid code?
         beq.s    5$                   ; no
         subq.b   #1,d0                ; adjust code
         andi.b   #~TRACEMASK,(a0)     ; clear old option
         or.b     d0,(a0)              ; OR in new

4$:      moveq    #0,d0                ; return code
         rts

5$:      moveq    #ERR10_033,d0        ; missing keyword
         rts

         ; Table of alphabetic trace options ...

TrCodes  dc.b     TRACE_A+1            ; 'All'
         dc.b     TRACE_B+1            ; 'Background'
         dc.b     TRACE_C+1            ; 'Commands'
         dc.b     0
         dc.b     TRACE_E+1            ; 'Errors'
         dc.b     0
         dc.b     0
         dc.b     0
         dc.b     TRACE_I+1            ; 'Intermediates'
         dc.b     0
         dc.b     0
         dc.b     TRACE_L+1            ; 'Labels'
         dc.b     0
         dc.b     TRACE_N+1            ; 'Normal'
         dc.b     TRACE_O+1            ; 'Off'
         dc.b     0
         dc.b     0
         dc.b     TRACE_R+1            ; 'Results'
         dc.b     TRACE_S+1            ; 'Scan

Normal   dc.b     'N'                  ; 'Normal' option
         CNOP     0,2

* =========================     rxInsSIGNAL     ========================
* Implements the 'SIGNAL' instruction.
* [instruction module environment]
rxInsSIGNAL
         movea.l  a0,a2                ; save token
         btst     #ISB_NOEVAL,d5       ; set flags?
         beq.s    2$                   ; no

         ; First form: SIGNAL {ON/OFF} {ERROR,HALT,NOVALUE,SYNTAX,IOERR,...}

         moveq    #KT21_OPTMASK,d0     ; load mask
         and.w    d5,d0                ; an option?
         beq      rxErr34              ; no

         andi.w   #KT21_BITMASK,d0     ; flag bit
         move.w   EVINTF(a4),d1        ; interrupt flags

         ; Set or clear the selected interrupt flag.

         bset     d0,d1                ; clear the flag
         btst     #KT21B_ON,d5         ; 'ON' specified?
         bne.s    1$                   ; yes
         bclr     d0,d1                ; no ... set the flag
1$:      move.w   d1,EVINTF(a4)        ; install flag
         bra.s    4$

         ; Second form: SIGNAL {label | [VALUE] expr}

2$:      btst     #ISB_SKIP1ST,d5      ; skip first?
         beq.s    3$                   ; no
         movea.l  (a2),a2              ; advance check token

3$:      move.l   a1,d1                ; a result string?
         beq      rxErr45              ; no -- error

         ; Convert the result to UPPERcase.

         movea.l  a4,a0                ; environment
         bsr      UpperString          ; D0=A1=result
         bset     #NSB_KEEP,ns_Flags(a1)

         ; Install it as the global search label and set the interrupt flag.

         movea.l  gn_SigLabel(a3),a1   ; previous string
         move.l   d0,gn_SigLabel(a3)   ; new string
         movea.l  a4,a0                ; environment
         bsr      FreeKeepStr          ; release old string
         bset     #(EXB_SIGNAL-16),(STATUS+1)(a4)

         ; Check for the STOPTOKEN ...

4$:      movea.l  a2,a0                ; check token
         bra      rxCheckSTOP
