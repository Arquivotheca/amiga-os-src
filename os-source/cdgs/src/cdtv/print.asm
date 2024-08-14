************************************************************************
***                                                                  ***
***                -= DEBUGGING PRINT FUNCTIONS =-                   ***
***                                                                  ***
************************************************************************
***                                                                  ***
***    Written by Carl Sassenrath, Sassenrath Research, Ukiah, CA    ***
***                                                                  ***
***    Modified 6/1/91: Jerry Horanoff                               ***
***                                                                  ***
************************************************************************

        INCLUDE "include:exec/types.i"
        INCLUDE "include:exec/nodes.i"
        INCLUDE "include:exec/lists.i"
        INCLUDE "defs.i"
        INCLUDE "cddev.i"
        INCLUDE "cdtv.i"

        INCLUDE "macros.i"

************************************************************************
***
***  External References
***
************************************************************************

        XDEF    PutChar,PutNewLine,PutStr,PutDec,PutHex
        XDEF    Print,PrintHere
        XDEF    MustPutStr,MustPrint


PRINTON         equ     1       ; enable print functions



************************************************************************
***
***  Monitor - print encoded debugging info to serial port.
***
************************************************************************
 FUNCTION Monitor
                save    d0/d1/a0/a1/a6
                move.l  5*4(sp),a0      ; ptr to flag and item

        ;------ Is this monitor flag enabled?
                move.b  (a0)+,d0        ; flag bit
                btst    d0,d1
                beq.s   8$
                add.b   #'0',d0
                call    SysBase,RawPutChar

        ;------ Print item value:
                moveq   #'0',d0
                add.b   (a0)+,d0
                call2   RawPutChar

                moveq   #'.',d0         ; V35.9
                call2   RawPutChar      ; V35.9

8$:             restore d0/d1/a0/a1/a6
9$:             addq.l  #2,(sp)
                rts

*
************************************************************************
***
***  MonVal - monitor a register value
***
************************************************************************
 FUNCTION MonVal
                save    d0/d1/a0/a1/a6
                move.l  5*4(sp),a0      ; ptr to flag and item

                move.l  (sp),d0
                bsr     PutHexVal

8$:             restore d0/d1/a0/a1/a6
9$:             addq.l  #2,(sp)
                rts


HexTab          dc.b    '0123456789ABCDEF'

PutHexVal:
                save    d2/d3
                move.l  d0,d2

                moveq   #'$',d0
                call    SysBase,RawPutChar

                moveq   #7,d3
2$:             rol.l   #4,d2
                moveq.l #$F,d0
                and.b   d2,d0
                move.b  HexTab(pc,d0.w),d0
                call2   RawPutChar
                dbf     d3,2$

                moveq   #'.',d0         ; V35.9
                call2   RawPutChar      ; V35.9

                restore d2/d3
                rts


        IFNE    PRINTON

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
                call    SysBase,RawPutChar
                move.l  #800,d0
1$              dbf     d0,1$
                restore d0/d1/a0/a1/a6
                rts

***********************************************************************
***     
***     PutNewLine - generate a new line
***
***             No args
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

PutNewLine:
                save    d0
                moveq   #NL,d0
                bsr.s   PutChar
                restore d0
1$:             rts


***********************************************************************
***     
***     PutStr - print a string of ASCII characters
***
***             A0 -> null terminated string to print
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

PutStr:
MustPutStr:     save    d0-d1/a0-a1
2$:             move.b  (a0)+,d0
                beq.s   1$
                bsr.s   PutChar
                bra.s   2$
1$:             restore d0-d1/a0-a1
ps_exit         rts



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
                bsr.s   MustPrint
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


*
***********************************************************************
***     
***     Print - print formatted values
***
***             A0 -> format specification string
***             A1 -> parameter array with values to print
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

Print:
MustPrint:      save    d0-d1/a0-a3/a6
                lea     PutChar(pc),a2
                call    SysBase,RawDoFmt
                restore d0-d1/a0-a3/a6
p_exit:         rts


***********************************************************************
***     
***     PrintHere - print next instruction as two ASCII characters
***
***             No args
***
***             Nothing returned
***
***             All registers preserved.
***
***********************************************************************

PrintHere:
                save    d0/d1/a0/a1
                move.l  4*4(sp),a0
                move.b  (a0)+,d0
                bsr     PutChar
                move.b  (a0)+,d0
                beq.s   1$
                bsr     PutChar
1$:             restore d0/d1/a0/a1
2$:             addq.l  #2,(sp)
                rts             

        ELSE

*
***********************************************************************
***     
***     Null functions when no printing is needed.
***
***********************************************************************

PutChar:
PutNewLine:
PutStr:
PutDec:
PutHex:
Print:
MustPutStr:
MustPrint:
                rts

PrintHere:
                addq.l  #2,(sp)
                rts             

        ENDC

        XDEF    EndCode
EndCode         dc.w    0

        END
