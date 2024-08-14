

        xdef        _Div64by32
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


