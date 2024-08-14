
*   devmona.asm  --- assembler interface for devmon.c
*                 Carolyn Scheppner --- CBM  05/90

   INCLUDE   'exec/types.i'
   INCLUDE   'exec/io.i'

	SECTION CODE

   XREF   _AbsExecBase

   XREF	  _unitptr
   XREF	  _unitnum
   XREF   _allunits
   XREF   _TheDevice

   XREF   _MyBEGIN
   XREF   _MyABORT
   XREF   _MyCLOSE
   XREF	  _MyOPEN
   XREF	  _MyEXPUNGE
   XREF	  _MySendIO
   XREF	  _MyWaitIO
   XREF	  _MyAbortIO
   XREF	  _MyDoIO
   XREF   _MyDoIORts
   XREF	  _MyReplyMsg

   XDEF   _myBEGIN
   XDEF   _myABORT
   XDEF   _myCLOSE
   XDEF   _myEXPUNGE
   XDEF	  _myOPEN
   XDEF	  _mySendIO
   XDEF	  _myWaitIO
   XDEF	  _myAbortIO
   XDEF	  _myDoIO
   XDEF	  _myReplyMsg

   XDEF   _RealBEGIN
   XDEF   _RealABORT
   XDEF	  _RealOPEN
   XDEF   _RealCLOSE
   XDEF   _RealEXPUNGE
   XDEF   _RealSendIO
   XDEF   _RealWaitIO
   XDEF   _RealAbortIO
   XDEF   _RealDoIO
   XDEF   _RealReplyMsg

   XDEF   _usecnt

_myBEGIN:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   tst.l   _allunits
   bgt.s   ball

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   bbye
ball:
   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyBEGIN
   addq.l  #4,a7
   
bbye:
   movem.l (a7)+,d0-d7/a0-a6   		;restore registers
   move.l  _RealBEGIN(pc),-(a7)		;continue to real BEGIN
   sub.l   #1,_usecnt
   rts


_myABORT:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   tst.l   _allunits
   bgt.s   aall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   abye
aall:
   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyABORT
   addq.l  #4,a7
   
abye:
   movem.l (a7)+,d0-d7/a0-a6   		;restore registers
   move.l  _RealABORT(pc),-(a7)		;continue to real ABORT
   sub.l   #1,_usecnt
   rts


_myCLOSE:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   tst.l   _allunits
   bgt.s   call

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   cbye
call:
   move.l  a1,-(a7)            ;push ptr to ior
   jsr     _MyCLOSE
   addq.l  #4,a7

cbye:
   movem.l (a7)+,d0-d7/a0-a6   ;restore registers
   move.l  _RealCLOSE(pc),-(a7)    ;continue to real CLOSE
   sub.l   #1,_usecnt
   rts


_myOPEN:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   tst.l   _allunits
   bgt.s   oall

   cmp.l   _unitnum,d0	       ;unit number in d0 for Open
   bne.s   cbye
oall:
   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyOPEN
   addq.l  #4,a7

obye:
   movem.l (a7)+,d0-d7/a0-a6   	;restore registers
   move.l  _RealOPEN(pc),-(a7)  ;continue to real OPEN
   sub.l   #1,_usecnt
   rts


_myEXPUNGE:
   add.l  #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   tst.l   _allunits
   bgt.s   eall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   ebye

eall:
   jsr     _MyEXPUNGE

ebye:
   movem.l (a7)+,d0-d7/a0-a6   ;restore registers
   moveq.l #0,d0               ;means unable to expunge
   sub.l   #1,_usecnt
   rts                         ;keep changed device from being expunged




_mySendIO:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   sbye

   tst.l   _allunits
   bgt.s   sall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   sbye
sall:
   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MySendIO
   addq.l  #4,a7
   
sbye:
   movem.l (a7)+,d0-d7/a0-a6   		;restore registers
   move.l  _RealSendIO(pc),-(a7)	;continue to real rtn
   sub.l   #1,_usecnt
   rts


_myWaitIO:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   wbye

   tst.l   _allunits
   bgt.s   wall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   wbye
wall:
   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyWaitIO
   addq.l  #4,a7
   
wbye:
   movem.l (a7)+,d0-d7/a0-a6   		;restore registers
   move.l  _RealWaitIO(pc),-(a7)	;continue to real rtn
   sub.l   #1,_usecnt
   rts


_myAbortIO:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   aibye

   tst.l   _allunits
   bgt.s   aiall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   aibye
aiall:
   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyAbortIO
   addq.l  #4,a7
   
aibye:
   movem.l (a7)+,d0-d7/a0-a6   		;restore registers
   move.l  _RealAbortIO(pc),-(a7)	;continue to real rtn
   sub.l   #1,_usecnt
   rts



_myDoIO:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers


   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   dbye

   tst.l   _allunits
   bgt.s   dall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   dbye
dall:
   move.l  a1,-(a7)             ;push ptr to io request
   jsr     _MyDoIO
   addq.l  #4,a7
   bra.s   dbbye		;go to the exit with the comeback
   
dbye:
   movem.l (a7)+,d0-d7/a0-a6   		;leaving for good - restore registers
   move.l  _RealDoIO(pc),-(a7)		;continue to real rtn
   sub.l   #1,_usecnt
   rts

dbbye:
   movem.l (a7)+,d0-d7/a0-a6   		;we'll be back - restore registers
   move.l  a1,-(a7)			;save ior
   pea.l   dback(pc)			;we want it to come back here
   move.l  _RealDoIO(pc),-(a7)		;continue to real rtn
   rts
dback:
   movem.l d0-d7/a0-a6,-(a7)   		;save registers
   move.l  60(sp),-(sp)		        ;get old ior ptr and push again
   jsr     _MyDoIORts
   addq.l  #4,a7			;remove new push
   movem.l (a7)+,d0-d7/a0-a6   		;restore registers
   addq.l  #4,a7			;get rid of old push
   sub.l   #1,_usecnt
   rts



_myReplyMsg:
   add.l   #1,_usecnt
   movem.l d0-d7/a0-a6,-(a7)   ;save registers


   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   rbye

   tst.l   _allunits
   bgt.s   rall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   rbye
rall:
   move.l  a1,-(a7)            ;push ptr to io request
   jsr     _MyReplyMsg
   addq.l  #4,a7
   
rbye:
   movem.l (a7)+,d0-d7/a0-a6   		;restore registers
   move.l  _RealReplyMsg(pc),-(a7)	;continue to real rtn
   sub.l   #1,_usecnt
   rts


_RealBEGIN:	DC.L	0
_RealABORT:	DC.L	0
_RealOPEN:	DC.L	0
_RealCLOSE:	DC.L	0
_RealEXPUNGE:	DC.L	0

_RealSendIO:	DC.L	0
_RealDoIO:	DC.L	0
_RealWaitIO:	DC.L	0
_RealAbortIO:	DC.L	0
_RealReplyMsg:	DC.L	0

_usecnt		DC.L	0
   END


  
