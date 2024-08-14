
* giveFract.asm

* REQUIREMENTS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*
*               ONLY WORKS on 68020 or better!
*
* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

* TO BE CALLED FROM C.

* To Assemble
* sc:c/asm -m2 UDiv6432.asm
* Add the -d flag to include debug symbols...nifty with CPR.


* ULONG FractionalPart =  giveFract(ULONG dividend, ULONG divisor, ULONG multNo)


* MultNo is a multiple of 10, indicating how precise you want the
* decimal portion to be.
* for example,  666 = giveFract(2,3,1000);
* NOTE THAT NO ROUND-OFF IS PERFORMED.

; Handy C Prototype:
; extern ULONG giveFract(ULONG,ULONG,ULONG);



    XDEF _giveFract


    SECTION fractsect,CODE
    _giveFract:
        movem.l     d2-d4,-(sp)         ; save old registers (stackchange: 3)
    
        move.l      4*4(sp),d0             ; dividend 
        move.l      5*4(sp),d2             ; divisor
        move.l      6*4(sp),d1             ; multNo

        tst.l       d2                     ; divisor better not be zero
        beq.s       fail                   ; bail out if divisor == 0

; We only want to work with the remainder of dividend/divisor.

; d0 - dividend, d1 - multNo, d2 - divisor, d3 - scratch, d4 - scratch

;       divul.l     <ea>,dr:dq      dq contains dividend -- 32/32->32
        divul.l     d2,d4:d0              ; d4 now contains number to be multiplied.

; We now have that remainder.  Multiply it by the MultNo.

; d0 - quotient(scratch), d1 - multNo, d2 - divisor, d3 - scratch, d4 - remainder

;       mulu.l      <ea>,dh:dl  - dl is register for 2nd multiplicand
        mulu.l      d1,d0:d4              ; multiply remainder by multNo

; We now use the result of the previous step as the dividend in the next,
; and do   functionResult = dividend/divisor  (throw away extra remainder).

; d0 - dividend(HI), d1 - multNo, d2 - divisor, d3 - scratch, d4 - dividend(LO)

;       divu.l      <ea>,dr:dq             dq has low bits of 64-bit dividend.
        divu.l      d2,d0:d4               ; do the division
                                           ; Reg   Before            After
                                           ; d4     low 32          quotient
                                           ; d0     hi  32          remainder

        exg         d0,d4                  ; put quotient into d0

        bra.s       bye                    ; our result is now in d0, we're done.


    fail:
        moveq.l     #0,d0                  ; return 0 for error.

    bye:
        movem.l     (sp)+,d2-d4         ; restore registers
        rts

    end
