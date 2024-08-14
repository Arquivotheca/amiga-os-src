

        include         "exec/types.i"
        include         "exec/execbase.i"
        include         "dos/dos.i"

        xref            _AbsExecBase
        xref            _LVOSetFunction
        xref            _LVOWait
        xref            _LVOPermit
        xref            _LVODisable
        xref            _LVOEnable
        xref            _kprintf
        xref            _LVOOpenLibrary,_LVOCloseLibrary
        xref            _LVOAllocMem,_LVOFindTask

blo macro
 bcs \1
 endm



Start:
        move.l          _AbsExecBase,a6
        lea.l           nipcname,a1
        jsr             _LVOFindTask(a6)
        tst.l           d0
        beq             9$
        move.l          d0,NIPCBase

        move.l          _AbsExecBase,a1
        move.l          #_LVOAllocMem,d1
        move.l          d1,a0
        lea.l           NewCode,a2
        move.l          a2,d0
        jsr             _LVOSetFunction(a6)
        move.l          d0,RomCode

        move.l          _AbsExecBase,a6
        move.l          #SIGBREAKF_CTRL_C,d0
        jsr             _LVOWait(a6)

        move.l          _AbsExecBase,a1
        move.l          #_LVOAllocMem,d1
        move.l          d1,a0
        move.l          RomCode,d0
        jsr             _LVOSetFunction(a6)

        move.l          _AbsExecBase,a6
        move.l          #SIGBREAKF_CTRL_C,d0
        jsr             _LVOWait(a6)

9$      moveq.l         #0,d0
        rts

NewCode:

        movem.l         d0-d1/a0-a1,-(sp)
        cmp.l           #20,d0
        beq             1$
        cmp.l           #24,d0
        beq             1$
        cmp.l           #18,d0
        beq             1$
        cmp.l           #92,d0
        beq             1$
        cmp.l           #1500,d0
        beq             1$
        sub.l           a1,a1
        move.l          a6,-(sp)
        jsr             _LVOFindTask(a6)
        move.l          (sp)+,a6
        cmp.l           NIPCBase,d0
        bne             1$
        movem.l         (sp),d0-d1/a0-a1
        move.l          d0,-(sp)
        pea.l           FoundOne
        jsr             _kprintf
        add.l           #8,sp
        lea.l           16(sp),a1
        move.l          a1,-(sp)
        move.w          #16,-(sp)
2$      move.l          2(sp),a1
        move.l          (a1)+,-(sp)
        move.l          a1,6(sp)
        pea.l           HexMask
        jsr             _kprintf
        add.l           #8,sp
        move.w          (sp),d1
        sub.w           #1,d1
        move.w          d1,(sp)
        tst.w           d1
        bne             2$
        add.l           #6,sp

        pea.l           crlf
        jsr             _kprintf
        add.l           #4,sp

1$      movem.l         (sp)+,d0-d1/a0-a1

        move.l          a0,-(sp)
        move.l          RomCode,a0
        jsr             (a0)
        move.l          (sp)+,a0
        rts


        section "",bss

RomCode     ds.l            1
NIPCBase    ds.l            1

        section "",data

nipcname dc.b 'nipc_supervisor',0
*nipcname dc.b 'MemoryTest',0
FoundOne dc.b 10
         dc.b '* size: %ld  Stack:',10,0
HexMask  dc.b '%lx ',0
crlf     dc.b 10,0

        end


