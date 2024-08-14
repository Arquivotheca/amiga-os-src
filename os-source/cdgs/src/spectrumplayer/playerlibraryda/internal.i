
EXEC            macro   function
                move.l  a6,-(sp)
                move.l  4,a6
                jsr     LVO.\1(A6)
                move.l  (sp)+,a6
                endm

************************************************************************
***
***  Library Vector Offsets (LVO's)
***
************************************************************************

FUNCDEF         macro
LVO.\1          equ     LVOFF
LVOFF           set     LVOFF-6
                endm
LVOFF           set     -5*6

        include "include:exec/exec_lib.i"

LVO.FindConfigDev equ -72

