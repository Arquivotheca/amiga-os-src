
        xref _FastRand
        xdef @FastRand

         xdef _ACTB,_ACFB
         xref _CTB,_CFB
_ACTB:
         movem.l     d2-d7/a2-a6,-(sp)
         move.l      a0,-(sp)
         move.l      d0,-(sp)
         move.l      a1,-(sp)
         jsr         _CTB
         add.l       #3*4,sp
         movem.l     (sp)+,a2-a6/d2-d7
         rts

_ACFB:
         movem.l     d2-d7/a2-a6,-(sp)
         move.l      a1,-(sp)
         move.l      d0,-(sp)
         move.l      a0,-(sp)
         jsr         _CFB
         add.l       #3*4,sp
         movem.l     (sp)+,a2-a6/d2-d7
         rts

@FastRand:
        move.l      d0,-(sp)
        jsr         _FastRand
        add.l       #4,sp
        rts



         end

