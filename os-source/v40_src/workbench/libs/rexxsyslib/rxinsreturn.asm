* == cvs2d.asm =========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ========================     rxInsRETURN     =========================
* Implements the 'EXIT' and 'RETURN' instructions.
* [instruction module routine]
rxInsRETURN
         clr.l    TSTRING(a0)          ; remove string from token

         ; Check whether an internal function is active.  The 'RETURN'
         ; instruction is equivalent to 'EXIT' in the base environment.

         moveq    #(EXB_EXIT-8),d2     ; 'EXIT' flag
         andi.w   #KTF_SECMASK,d5      ; 'EXIT' option?
         bne.s    1$                   ; yes ...
         tst.w    ev_Level(a4)         ; a function active?
         beq.s    1$                   ; no ...

         ; 'RETURN' being performed ... result may need to be copied.

         moveq    #(EXB_RETURN-8),d2   ; 'RETURN' flag

         ; Make a copy of the result string, if necessary.

         move.l   a1,d0                ; a result string?
         beq.s    1$                   ; no ...
         movea.l  a4,a0                ; storage environment
         bsr      MayCopyString        ; D0=A1=string

         ; Install the result in global name slot.

1$:      move.l   a1,EVNAME(a3)        ; install result

         ; Set the 'EXIT' or 'RETURN' flag as appropriate.

         bset     d2,(STATUS+2)(a4)    ; set flag
         rts

* ==========================     rxInsPUSH     =========================
* Implements the "PUSH" instruction.  The string is stacked to the STDIN
* stream.
rxInsPUSH
         move.w   #_LVOStackF,d4       ; output function
         bra.s    rQ001

* =========================     rxInsQUEUE     =========================
* Implements the "QUEUE" instruction.  Data is queued to the STDIN stream.
rxInsQUEUE
         move.w   #_LVOQueueF,d4       ; output function

rQ001    movea.l  rl_STDIN(a6),a2      ; logical name
         bra.s    rS001

* ==========================     rxInsSAY     ==========================
* Implements the 'SAY' instruction.
rxInsSAY
         move.w   #_LVOWriteF,d4       ; output function
         movea.l  rl_STDOUT(a6),a2     ; logical name

         ; Copy the string and append a 'newline' to it.

rS001    move.l   a1,d1                ; a result string?
         bne.s    1$                   ; yes ...
         movea.l  rl_NULL(a6),a1       ; ... use null string

1$:      movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=string
         exg      a1,a2                ; logical=>A1, string=>A2
         moveq    #0,d2                ; clear length
         move.w   ns_Length(a2),d2     ; load length
         move.b   #NEWLINE,ns_Buff(a2,d2.l)
         addq.l   #1,d2

         ; Locate the IoBuff node corresponding to the desired stream.

         movea.l  a3,a0                ; global pointer
         bsr      FindLogical          ; D0=A0=node
         beq.s    2$                   ; not found?

         ; Send the string to the selected output function.

         lea      ns_Buff(a2),a1       ; buffer
         move.l   d2,d0                ; requested length
         jsr      0(a6,d4.w)           ; D0=actual count D1=error code

         ; Save error code and release the temporary string.

2$:      move.l   d0,d4                ; save count
         move.l   d1,d3                ; save error code
         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; string
         bsr      FreeString           ; release it

         ; Check whether the full length was written ...

         cmp.l    d2,d4                ; actual = request?
         beq.s    3$                   ; yes

         ; An error occurred ... save secondary code in 'RC' and check
         ; whether the IOERR interrupt is enabled.

         movea.l  a4,a0                ; environment
         move.l   d3,d0                ; load error code
         moveq    #INB_IOERR,d1        ; interrupt flag
         bsr      rxSignal             ; D0=error D1=boolean
         move.l   d0,d6                ; return error

3$:      rts

* =========================     rxInsINTERP     ========================
* Implements the 'INTERPRET' instruction.
* [instruction module routine]
rxInsINTERP
         btst     #CFB_NULLEXPR,c_Flags(a5)
         bne      rxErr45              ; error ... result required

         ; Call the "string parser" (in non-DEBUG mode).

         movea.l  a4,a0                ; environment
         move.l   c_SrcPos(a5),d0      ; source position
         bsr      ParseString          ; D0=error
         move.l   d0,d6
         rts
