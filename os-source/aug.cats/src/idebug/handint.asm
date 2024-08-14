
   xdef   _HandlerInterface
   xref   _myhandler


*************************************************************************
*   HandlerInterface()
*
*   This code is needed to convert the calling sequence performed by
*   the input.task for the input stream management into something
*   that a C program can understand.
*
*   This routine expects a pointer to an InputEvent in A0, a pointer
*   to a data area in A1.  These values are transferred to the stack
*   in the order that a C program would need to find them.  Since the
*   actual handler is written in C, this works out fine. 
*
*   Author: Rob Peck, 12/1/85
*

     CODE

_HandlerInterface:
   movem.l   A0/A1,-(A7)
   jsr       _myhandler
   addq.l    #8,A7
   rts

     END

