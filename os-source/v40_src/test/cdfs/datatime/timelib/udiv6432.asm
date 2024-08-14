
* ONLY WORKS on 68020 or better!
* TO BE CALLED FROM C.

* To Assemble
* sc:c/asm -m2 UDiv6432.asm
* Add the -d flag to include debug symbols...nifty with CPR.

*   UDiv6432(ULONG d64hi, ULONG d64lo, ULONG d32, ULONG *quotient, ULONG *remainder)
*                d0           d1         a0            a1              a2
* d64hi = high 32 bits of 64-bit dividend
* d64lo = low  32 bits of 64-bit dividend
* d32   = 32-bit divisor
* quotient  = ptr to 32-bit quotient
* remainder = ptr to 32-bit remainder (optional, may be NULL)


* Note that the TST.L's of address registers only work on 68020's or better
* Note also that the entire point of this exercise, DIVU.L, ALSO only
* works on 68020's or better.

* Handy C Prototype:
; extern BOOL UDiv6432(ULONG,ULONG,ULONG,ULONG *,ULONG *);

    XDEF _UDiv6432
    SECTION UDiv6432,CODE
    _UDiv6432:
        movem.l     a2/d2-d3,-(sp)   ; save old registers (stackchange: 5)
    
        move.l      4*4(sp),d1             ; dividend (hi)
        move.l      5*4(sp),d0             ; dividend (lo)
        move.l      6*4(sp),d2             ; divisor
        move.l      7*4(sp),a1             ; ptr to quotient
        move.l      8*4(sp),a2             ; ptr to remainder

        move.l      d2,a0                  ; divisor in d2 and a0, set flag.
        beq.s       fail                   ; bail out
        move.l      a1,d3                  ; test a1 by setting flag
        beq.s       fail                   ; bail out if we can't

        divu.l      d2,d1:d0               ; do the division
                                           ; Reg   Before            After
                                           ; d0     low 32          quotient
                                           ; d1     hi  32          remainder

        move.l      d0,(a1)                ; save quotient
        move.l      a2,d3                  ; see if we need to do remainder, set flag
        beq.s       done                   ; not really a failure, though
        move.l      d1,(a2)                ; save remainder

    done:
        moveq.l     #1,d0                  ; return TRUE
        bra.s       bye

    fail:
        moveq.l     #0,d0                  ; return FALSE

    bye:
        movem.l     (sp)+,a2/d2-d3         ; restore registers
        rts

    end
