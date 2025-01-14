*   cmda.asm  --- assembler interface for Cmd.c
*                 Carolyn Scheppner --- CBM  05/87

   INCLUDE   'exec/types.i'

	section code

   XREF   _AbsExecBase
   XREF   _MyBeginIO
   XREF   _MyClose
   XREF   _RealClose

   XDEF   _myBeginIO
   XDEF   _myClose
   XDEF   _myExpunge



_myBeginIO:
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyBeginIO
   addq.l  #4,a7

   movem.l (a7)+,d0-d7/a0-a6   ;restore registers
   rts


_myClose:
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyClose
   addq.l  #4,a7

   movem.l (a7)+,d0-d7/a0-a6   ;restore registers

   move.l  _RealClose,a0       ;continue to real Close
   jmp     (a0)


_myExpunge:
   moveq.l #0,d0               ;means unable to expunge
   rts                         ;keep changed device from being expunged

   END


  

