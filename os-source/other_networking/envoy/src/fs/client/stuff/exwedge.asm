

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


        xref            _LVOExNext

Start:
        move.l          _AbsExecBase,a6

        lea.l           DName,a1
        moveq.l         #0,d0
        jsr             _LVOOpenLibrary
        move.l          d0,a5



        move.l          a5,a1
        move.l          #_LVOExNext,d1
        move.l          d1,a0
        lea.l           NewCode,a2
        move.l          a2,d0
        jsr             _LVOSetFunction(a6)
        move.l          d0,RomCode

        movem.l         d0-d7/a0-a6,-(sp)
        move.l          d0,-(sp)
        pea.l           OldWas
        jsr             _kprintf
        add.l           #8,sp
        movem.l         (sp)+,d0-d7/a0-a6

        move.l          _AbsExecBase,a6
        move.l          #SIGBREAKF_CTRL_C,d0
        jsr             _LVOWait(a6)

        move.l          a5,a1
        move.l          #_LVOExNext,d1
        move.l          d1,a0
        move.l          RomCode,d0
        jsr             _LVOSetFunction(a6)

        move.l          _AbsExecBase,a6
        move.l          #SIGBREAKF_CTRL_C,d0
        jsr             _LVOWait(a6)



        moveq.l         #0,d0
        rts

NewCode:

        movem.l         d0-d1/a0-a1,-(sp)
        move.l          d2,a0

        move.l          RomCode,a0
        jsr             (a0)
        movem.l         (sp)+,d0-d1/a0-a1
        rts


        section "",bss

RomCode     ds.l            1

        section "",data

ThrowAFit dc.b 7
          dc.b 'Enable count illegal on task %lx, count %lx.  Forced to -1.',10,0

OldWas dc.b 'Old ptr was %lx',10,0

        end


