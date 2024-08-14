
        include     "exec/types.i"
        include     "exec/io.i"

SYS macro
 jsr _LVO\1(a6)
 endm

XSYS macro
    xref _LVO\1
    endm

    xref    _AbsExecBase,_kprintf

    XSYS SetFunction
    XSYS OpenDevice
    XSYS Wait
    XSYS CloseDevice


main:
        move.l  _AbsExecBase,a6

        move.l  a6,a1
        move.l  #_LVOOpenDevice,d0
        move.l  d0,a0
        lea.l   NewCode,a2
        move.l  a2,d0
        SYS     SetFunction
        move.l  d0,Old

        move.l  a6,a1
        move.l  #_LVOCloseDevice,d0
        move.l  d0,a0
        lea.l   NewCodeC,a2
        move.l  a2,d0
        SYS     SetFunction
        move.l  d0,OldC

        move.l  #$1000,d0
        SYS     Wait

        move.l  Old,d0
        move.l  #_LVOOpenDevice,d1
        move.l  d1,a0
        move.l  a6,a1
        SYS     SetFunction

        move.l  OldC,d0
        move.l  #_LVOCloseDevice,d1
        move.l  d1,a0
        move.l  a6,a1
        SYS     SetFunction

        rts

NewCodeC:
        movem.l a0-a1/d0-d2,-(sp)
        move.l  IO_DEVICE(a1),a0
        move.w  LIB_OPENCNT(a0),d0
        ext.l   d0
        move.l  d0,-(sp)
        move.l  a0,d0
        move.l  d0,-(sp)
        pea.l   CloseMsg
        jsr     _kprintf
        add.l   #12,sp
        movem.l (sp)+,a0-a1/d0-d2

        move.l  a2,-(sp)
        move.l  OldC,a2
        jsr     (a2)
        move.l  (sp)+,a2
        rts


NewCode:
        move.l  a3,-(sp)
        movem.l a0-a1/d0-d2,-(sp)

        move.l  a1,a3

        move.l  (a0),d2
        cmp.l   #$74696d65,d2
        beq     9$
        move.w  IO_COMMAND(a1),d1
        move.l  d1,-(sp)
        move.l  d0,-(sp)
        move.l  a0,-(sp)
        pea.l   OpenMsg
        jsr     _kprintf
        add.l   #16,sp
9$      movem.l (sp)+,a0-a1/d0-d2

        move.l  a2,-(sp)
        move.l  Old,a2
        jsr     (a2)
        move.l  (sp)+,a2

        tst.l   d0
        beq     10$

        movem.l a0-a1/d0-d1,-(sp)
        moveq.l #0,d0
        move.b  IO_ERROR(a3),d0
        move.l  d0,-(sp)
        pea.l   ECode
        jsr     _kprintf
        add.l   #8,sp
        movem.l (sp)+,a0-a1/d0-d1

10$     move.l  (sp)+,a3

        rts

OldC ds.l   1
Old ds.l    1

CloseMsg dc.b 'Atmpt close: %lx OpenCount %ld',10,0

OpenMsg dc.b 'Attempt to open device: <%s>, Unit: %ld %lx',10,0
ECode dc.b 'Error code: %lx',10,0

        end


