

        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "defs.i"
        INCLUDE "cd.i"
        INCLUDE "cdprivate.i"

        OPT     p=68020

************************************************************************
***
***  External References
***
************************************************************************

        IFD     DVL

        XDEF    PutChar,Print,PutDec,PutHex

*
***********************************************************************
***     
***     PutChar - put a character to the debugging display      
***
***             D0 == ASCII character to display
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

PutChar:
                save    d0/d1/a0/a1/a6
                exec    RawPutChar
                move.l  #800,d0
1$              dbf     d0,1$
                restore d0/d1/a0/a1/a6
                rts

***********************************************************************
***     
***     Print - print a string of ASCII characters
***
***             A0 -> null terminated string to print
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

Print:
                save    d0-d1/a0-a1
2$:             move.b  (a0)+,d0
                beq.s   1$
                bsr.s   PutChar
                bra.s   2$
1$:             restore d0-d1/a0-a1
                rts



***********************************************************************
***     
***     PutDec - print a value as decimal digits
***
***             D0 == value to print
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

PutDec:
                save    d0/d1/a0/a1
                lea     decFmt(pc),a0
putCommon:
                move.l  d0,-(sp)
                move.l  sp,a1
                bsr.s   RawPrint
                addq.l  #4,sp
                restore d0/d1/a0/a1
                rts

decFmt          dc.b    '%ld ',0
                ds.w    0


***********************************************************************
***     
***     PutHex - print a value as hexidecimal digits
***
***             D0 == value to print
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

PutHex:
                save    d0/d1/a0/a1
                lea     hexFmt(pc),a0
                bra.s   putCommon

hexFmt          dc.b    '$%lx ',0
                ds.w    0





RawPrint:       save    d0-d1/a0-a3/a6
                lea     PutChar(pc),a2
                exec    RawDoFmt
                restore d0-d1/a0-a3/a6
                rts


        ENDC


***********************************************************************

        XDEF    EndCode
EndCode         dc.w    0

        END
