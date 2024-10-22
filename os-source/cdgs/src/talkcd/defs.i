
************************************************************************
***
***  Standard Defines
***
************************************************************************

SysBase equ 4

LF      equ 10
NL      equ 10
CR      equ 13
TAB     equ 9
FF      equ 12

************************************************************************
***
***  Standard Macros
***
************************************************************************

clear       macro   data-register
        moveq.l #0,\1
        endm

save        macro   (regs)
        movem.l \1,-(sp)
        endm

restore     macro   (regs)
        movem.l (sp)+,\1
        endm

exec        macro   function
        move.l  a6,-(sp)
        move.l  4,a6
        jsr LVO.\1(A6)
        move.l  (sp)+,a6
        endm

FUNCTION    macro
        XDEF    \1
\1:
        endm


NEWLISTX    MACRO                   ; list
        MOVE.L  \1,LH_HEAD(\1)
        ADDQ.L  #4,(\1)             ; Get address of LH_TAIL
        CLR.L   LH_TAIL(\1)         ; Clear LH_TAIL
        MOVE.L  \1,LH_TAILPRED(\1)  ;Address of LH_TAIL to LH_HEAD
        ENDM


ADDTAILX    MACRO                   ; A0-list(destroyed) A1-node D0-(destroyed)
        LEA     LH_TAIL(A0),A0
        MOVE.L  LN_PRED(A0),D0
        MOVE.L  A1,LN_PRED(A0)
        MOVE.L  A0,(A1)
        MOVE.L  D0,LN_PRED(A1)
        MOVE.L  D0,A0
        MOVE.L  A1,(A0)
        ENDM


************************************************************************
***
***  Library Vector Offsets (LVO's)
***
************************************************************************

FUNCDEF     macro
LVO.\1      equ LVOFF
LVOFF       set LVOFF-6
        endm
LVOFF       set -5*6
    include "include:exec/exec_lib.i"

LVO.FindConfigDev equ -72

