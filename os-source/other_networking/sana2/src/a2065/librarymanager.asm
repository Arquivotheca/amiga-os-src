**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**



        nolist

        include "includes.asm"
        include "globals.i"

        list


        XDEF    OpnLibs,ClsLibs

ExpLib  dc.b    'expansion.library',0
UtlLib  dc.b    'utility.library',0

Libs    dc.l    ExpLib,ExpansionBase
        dc.l    UtlLib,UTBase
        dc.l    0,0

;
;       Opens the librarys needed by this program
;
;       return non-zero if all libraries open with no problems
;
;       Assumes A6 contains SYSBASE
;

OpnLibs movem.l a1/a2/d1,-(sp)
        lea     Libs,a2

1$      tst.l   (a2)
        beq.s   9$
        move.l  (a2)+,a1
        moveq   #0,d0
        CALL    OpenLibrary
        tst.l   d0
        beq.s   9$
        move.l  (a2)+,d1
        move.l  d0,0(a5,d1.l)
        bra.s   1$

9$      movem.l (sp)+,a1/a2/d1
        rts

;
;       ClsLibs
;
;       Assumes A6 contains SYSBASE
;

ClsLibs movem.l a1/a2,-(sp)
        lea     Libs,a2

1$      tst.l   (a2)+
        beq.s   9$
        move.l  (a2)+,d0
        move.l  0(a5,d0.l),d0
        tst.l   d0
        beq.s   1$
        move.l  d0,a1
        CALL    CloseLibrary
        bra.s   1$

9$      movem.l (sp)+,a1/a2
        rts

        END
