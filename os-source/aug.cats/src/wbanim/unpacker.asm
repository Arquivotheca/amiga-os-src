* unpacker.asm - optimized from disassembly of unpacker.c

	CODE

	XDEF _unpackrow

_unpackrow
        link.w     a5,#-20
        movem.l    d4/d5/d6/d7/a2/a3,-(sp)
        movea.l    8(a5),a0
        movea.l    (a0),a3
        movea.l    12(a5),a0
        movea.l    (a0),a2
        move.w     18(a5),d5
        move.w     22(a5),d4
        move.w     #1,-18(a5)
        move.w     #-128,-20(a5)
L0028   tst.w      d4
        ble.s      L007c
        subq.w     #1,d5
        tst.w      d5
        bmi.s      L0080
        move.b     (a3)+,d7
        ext.w      d7
        blt.s      L0052
        addq.w     #1,d7
        sub.w      d7,d5
        tst.w      d5
        bmi.s      L0080
        sub.w      d7,d4
        tst.w      d4
        bmi.s      L0080
L0046   movea.l    a2,a0
        addq.l     #1,a2
        move.b     (a3)+,(a0)
        subq.w     #1,d7
        bgt.s      L0046
        bra.s      L0028
L0052   move.w     -20(a5),d0
        cmp.w      d7,d0
        beq.s      L0028
        move.l     d7,d0
        neg.w      d0
        move.l     d0,d7
        addq.w     #1,d7
        subq.w     #1,d5
        tst.w      d5
        bmi.s      L0080
        sub.w      d7,d4
        tst.w      d4
        bmi.s      L0080
        move.b     (a3)+,d6
L0070   movea.l    a2,a0
        addq.l     #1,a2
        move.b     d6,(a0)
        subq.w     #1,d7
        bgt.s      L0070
        bra.s      L0028
L007c   clr.w      -18(a5)
L0080   movea.l    8(a5),a0
        move.l     a3,(a0)
        movea.l    12(a5),a0
        move.l     a2,(a0)
        move.w     -18(a5),d0
        movem.l    (sp)+,a3/a2/d7/d6/d5/d4
        unlk       a5
        rts  

	END
