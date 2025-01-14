

        xdef        _Div64by32,_Mul64by16
*
* Div64by32
*
* Divides a 64 bit unsigned number in D0 (ms), D1 (ls) by a 32 bit unsigned
* number in D2.  Returns the quotient in D0.
*
*
_Div64by32:
        movem.l         d2-d6,-(sp)
        moveq.l         #0,d3           * Overflow
        move.l          d3,d6           * Answer
        move.l          #64+1,d5        * Count
1$
        tst.l           d5
        beq.s           99$
        subq.l          #1,d5
        lsl.l           #1,d6

        cmp.l           d2,d3
        blo.s           2$
        bset            #0,d6
        sub.l           d2,d3
2$

        asl.l           #1,d1
        roxl.l          #1,d0
        roxl.l          #1,d3
        bra.s           1$
99$     move.l          d6,d0
        movem.l         (sp)+,d2-d6
        rts

*
* entry:
*       A0 - ptr to 64 bit multiplier
*       D0 - 16 bit multiplicant
*
_Mul64by16:
        move.l      d0,d2
        move.l      (a0),d0
        move.l      4(a0),d1
        bsr         Mul64by16
        move.l      d0,(a0)
        move.l      d1,4(a0)
        rts


*
* Mul64by16
*
* Entry:
*       D0, D1 make up the 64 bit number
*       the lower portion of D2 makes up the 16 bit number
*
* Exit:
*       D0, D1 make up the 64-bit product
*
Mul64by16:
        movem.l     d2-d4/a0,-(sp)
        sub.l       #8,sp
        move.l      sp,a0
        move.l      #0,(a0)      * product msl

        move.w      d2,d3
        move.w      d1,d4
        mulu        d3,d4
        move.l      d4,4(a0)       * product lsl

        move.l      d1,d4
        swap        d4
        mulu        d3,d4
        add.l       2(a0),d4
        move.l      d4,2(a0)

        move.w      d0,d4
        mulu        d3,d4
        add.l       (a0),d4
        move.l      d4,(a0)

        move.l      d0,d4
        swap        d4
        mulu        d3,d4
        add.w       (a0),d4
        move.w      d4,(a0)

        move.l      (a0),d0
        move.l      4(a0),d1

        add.l       #8,sp
        movem.l     (sp)+,d2-d4/a0
        rts

