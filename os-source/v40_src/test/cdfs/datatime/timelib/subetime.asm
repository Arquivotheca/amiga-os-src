

* subETime
* VOID subETime(struct EClockVal *stop, struct EClockVal *start);
* subtracts two 64 bit numbers, modifying stop:
*     stop - start -> stop
*

    XDEF _subETime

    SECTION subetime,CODE
    _subETime:


    movem.l     d2/d3,-(sp)                    ; save registers
    movea.l     3*4(sp),a0                     ; get ptr to stop eclockval struct
    move.l      a0,d0                          ; set ccr to test for null
    beq.s       bye                            ; punt if null
    move.l      (a0)+,d0                       ; stop.ev_hi
    move.l      (a0),d1                        ; stop.ev_lo
    movea.l     4*4(sp),a1                     ; start pointer
    move.l      a1,d2                          ; set ccr to test for null
    beq.s       bye                            ; punt if NULL
    move.l      (a1)+,d2                       ; start.ev_hi
    move.l      (a1),d3                        ; start.ev_lo

*   Remember that these subs rely on the X and C flags in the CCR...
    sub.l       d3,d1                          ; stop.ev_lo - start.ev_lo into d1
    subx.l      d2,d0                          ; stop.ev_hi - start.ev_hi into d0
    
    move.l      d1,(a0)                        ; save stop.ev_hi
    move.l      d0,-(a0)                       ; save stop.ev_lo

    bye:
    movem.l     (sp)+,d2/d3                    ; restore regs
    rts                                        ; goodbye


    end

