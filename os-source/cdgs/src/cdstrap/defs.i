
******* System Library Offsets
FUNCDEF     macro
LVO.\1      equ LVOFF
LVOFF       set LVOFF-6
        endm

LVOFF       set -5*6


LVO.Open    equ -5*6
LVO.Close   equ -6*6
LVO.Read    equ -7*6
LVO.Write   equ -8*6
LVO.Input   equ -9*6
LVO.Output  equ -10*6
LVO.LoadSeg equ -25*6

    include "V40:include/exec/exec_lib.i"


exec        macro   function
        move.l  a6,-(sp)
        move.l  4,a6
        jsr LVO.\1(A6)
        move.l  (sp)+,a6
        endm

