**************************************************************************
** Copyright 1991 CONSULTRON
*
*       asmsupp.i  -- random low level assembly support routines
*
**************************************************************************

CLEAR   macro           ;quick way to clear a D register on 68000
        moveq   #0,\1
        endm

BHS     macro
        bcc.\0  \1
        endm

BLO     macro
        bcs.\0  \1
        endm

EVEN    macro           ;word align code stream
        ds.w    0
        endm

LINKSYS macro           ;link to a library without having to see a _LVO
        LINKLIB _LVO\1,\2
        endm

CALLSYS macro           ;call a library without having to see a _LVO
        CALLLIB _LVO\1
        endm

XLIB    macro           ;define a library reference without the _LVO
        XREF    _LVO\1
        endm

* ---- SPECIAL Breakpoint for "Metascope Debugger" to allow trace 
ILLEGAL macro           ; ILLEGAL trap
        dc.w   $4AFC
        endm

RetD0   EQUR    d0
RetD1   EQUR    d1
RetD2   EQUR    d2
RetD3   EQUR    d3
RetD4   EQUR    d4
RetD5   EQUR    d5
RetD6   EQUR    d6
RetD7   EQUR    d7
RetA0   EQUR    a0
RetA1   EQUR    a1
RetA2   EQUR    a2
RetA3   EQUR    a3
RetA4   EQUR    a4
RetA5   EQUR    a5
RetA6   EQUR    a6
RetA7   EQUR    a7

ArgD0   EQUR    d0
ArgD1   EQUR    d1
ArgD2   EQUR    d2
ArgD3   EQUR    d3
ArgD4   EQUR    d4
ArgD5   EQUR    d5
ArgD6   EQUR    d6
ArgD7   EQUR    d7
ArgA0   EQUR    a0
ArgA1   EQUR    a1
ArgA2   EQUR    a2
ArgA3   EQUR    a3
ArgA4   EQUR    a4
ArgA5   EQUR    a5
ArgA6   EQUR    a6
ArgA7   EQUR    a7

UnitNum EQUR    d7      ; Unit number;
IORequest EQUR  a1      ; IORequest pointer
IOReq   EQUR    a2      ; IORequest pointer
Task    EQUR    a4      ; Task struct pointer
Unit    EQUR    a3      ; Unit pointer
Device  EQUR    a5      ; Device pointer
AbsExecBase EQUR  a6    ; AbsExecBase pointer


UBYTE_SIZE      EQU     1      ; # of bytes
UWORD_SIZE      EQU     2      ; # of bytes
ULONG_SIZE      EQU     4      ; # of bytes
UBYTE_BSIZE     EQU     8      ; # of bits

