* == rxlocate.asm ======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     rxLocate     ==========================
* Searches the Labels Array for the specified label.
* Registers:   A0 -- environment
*              A1 -- label string
* Return:      D0 -- error code
*              D1 -- source position
rxLocate
         movem.l  d2/a2-a4,-(sp)
         movea.l  a0,a4
         movea.l  a1,a3                ; label to search

         movea.l  ev_GlobalPtr(a4),a0  ; global pointer
         move.l   gn_LabSeg(a0),d0     ; a Labels Array?
         beq.s    2$                   ; no

         movea.l  d0,a2
         move.l   (a2)+,d2             ; (sCount) any labels?
         beq.s    2$                   ; no
         subq.w   #1,d2                ; loop count

1$:      movea.l  (a2)+,a0             ; test string
         movea.l  a3,a1                ; search string
         CALLSYS  CmpString            ; D0=boolean
         dbne     d2,1$                ; loop back
         bne.s    3$                   ; ... matched

2$:      moveq    #ERR10_030,d0        ; "label not found"
         bra.s    4$

         ; Label found ... store line number in SIGL and return position.

3$:      movea.l  a4,a0                ; environment
         move.l   ev_SrcLine(a4),d0    ; current line number
         moveq    #STSIGL,d1           ; variable index
         bsr      AssignSpecial

         movea.l  -(a2),a0             ; matched label string
         move.l   (a0),d1              ; (IVALUE) source position
         moveq    #0,d0                ; error code

4$:      movem.l  (sp)+,d2/a2-a4
         rts

* ==========================     rxSignal     ==========================
* Sets the interrupt flag for the specified condition if the error code
* is non-zero.  The error code is stored in the special variable 'RC'.
* The returned error is the error code associated with the interrupt, and
* the boolean return indicates whether the tested condition was enabled.
* Registers:   A0 -- environment
*              D0 -- error code
*              D1 -- signal flag bit
* Return:      D0 -- error code or 0
*              D1 -- boolean flag
rxSignal
         movem.l  d2-d4/d6/a2/a4,-(sp)
         movea.l  a0,a4                ; save environment

         moveq    #0,d3                ; clear return flag
         move.l   STATUS(a4),d4        ; machine status
         moveq    #0,d6                ; clear return

         ; Check whether an interrupt is already pending (rare).

         bset     #EXB_SIGNAL,d4       ; interrupt pending?
         bne.s    5$                   ; yes
         move.l   d0,d6                ; ... default return code

         ; Index into the signal string array to get the interrupt label.

         lea      ErrLabel(pc),a2      ; error label array
         move.w   d1,d0                ; interrupt index
         lsl.w    #4,d0                ; scale for 16 bytes
         adda.w   d0,a2                ; ... signal label
         move.b   ns_Flags(a2),d2      ; load flags

         ; Check whether machine is in 'DEBUG' mode (interrupts disabled).

         btst     #EXB_DEBUG,d4        ; 'DEBUG' mode?
         bne.s    3$                   ; ... nothing to do

         ; Check whether interrupt is mandatory or just to set 'RC'.

         btst     #SETRC,d2            ; set 'RC' code?
         beq.s    1$                   ; no ... check interrupt

         ; Interrupt optional ... ONLY if an error condition exists.

         tst.l    d6                   ; an error?
         beq.s    2$                   ; no

         ; Check whether the interrupt signal is enabled.

1$:      move.w   EVINTF(a4),d0        ; interrupt flags
         bclr     d1,d0                ; enabled? (clear it)
         beq.s    2$                   ; no ...

         ; Disable the condition and set the interrupt flag.

         moveq    #-1,d3               ; set 'trapped' flag
         move.w   d0,EVINTF(a4)        ; update interrupts
         move.l   d4,STATUS(a4)        ; update status (interrupt set)

         ; Release the previous 'signal' string and install the new one.

         movea.l  ev_GlobalPtr(a4),a1  ; global pointer
         move.l   gn_SigLabel(a1),d1   ; previous string
         move.l   a2,gn_SigLabel(a1)   ; install new
         movea.l  d1,a1                ; old string
         bsr      FreeKeepStr          ; release it

         ; Check whether to set the special variable 'RC'.

2$:      btst     #SETRC,d2            ; set 'RC' code?
         beq.s    3$                   ; no ...
         movea.l  a4,a0                ; environment
         move.l   d6,d0                ; error code
         moveq    #STRC,d1             ; index for 'RC'
         bsr      AssignSpecial        ; ... install value

         ; Determine what error code to return ... each interrupt has an
         ; associated error flagged as "load if set" or "load if not set".

3$:      moveq    #LOADNOT,d0          ; "load if not set"
         move.w   d3,d1                ; trapped?
         beq.s    4$                   ; no
         moveq    #LOADSET,d0          ; "load if set"

         ; Check if the associated error code should be loaded.

4$:      btst     d0,d2                ; load new error code?
         beq.s    5$                   ; no ...
         move.l   (a2),d6              ; associated error

         ; Load the return error code.

5$:      move.l   d6,d0                ; error code
         movem.l  (sp)+,d2-d4/d6/a2/a4
         rts
