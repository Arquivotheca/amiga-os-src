
*
* Lattice's atol was acting flakey, and since I have little
* patience for such things, I tossed together a minimalistic
* atol routine.  It doesn't, however, allow for negatives or leading
* spaces.
*
        include     "rcp_rev.i"

    xdef        _myatol

_myatol:
        moveq.l     #0,d0
        move.l      d0,d1
1$      move.b      (a0)+,d1
        beq.s       2$
        sub.b       #'0',d1

        move.l      d0,-(sp)            * times 10
        add.l       d0,d0
        add.l       d0,d0
        add.l       (sp)+,d0
        add.l       d0,d0

        add.l       d1,d0
        bra         1$

2$      rts

        VERSTAG

        end


