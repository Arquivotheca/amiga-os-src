
*   devmona.asm  --- assembler interface for devmon.c
*                 Carolyn Scheppner --- CBM  05/90

   INCLUDE   'exec/types.i'
   INCLUDE   'exec/io.i'

	SECTION CODE

   XREF   _AbsExecBase

   XREF	  _stricmp

   XREF	  _unitptr
   XREF	  _unitnum
   XREF   _allunits
   XREF	  _deviceName
   XREF   _TheDevice

   XREF   _MyBEGIN
   XREF   _MyABORT
   XREF   _MyCLOSE
   XREF	  _MyOPEN
   XREF	  _MyEXPUNGE
   XREF	  _MySendIO
   XREF	  _MyCheckIO
   XREF	  _MyCheckIORts
   XREF	  _MyWaitIO
   XREF	  _MyAbortIO
   XREF	  _MyDoIO
   XREF   _MyDoIORts
   XREF	  _MyReplyMsg
   XREF   _MyOpenDevice
   XREF   _MyOpenDevRts
   XREF	  _MyCloseDevice
   XREF	  _MyCloseDevRts

   XDEF   _myBEGIN
   XDEF   _myABORT
   XDEF   _myCLOSE
   XDEF   _myEXPUNGE
   XDEF	  _myOPEN
   XDEF	  _mySendIO
   XDEF   _myCheckIO
   XDEF	  _myWaitIO
   XDEF	  _myAbortIO
   XDEF	  _myDoIO
   XDEF	  _myReplyMsg
   XDEF	  _myOpenDevice
   XDEF	  _myCloseDevice

   XDEF   _RealBEGIN
   XDEF   _RealABORT
   XDEF	  _RealOPEN
   XDEF   _RealCLOSE
   XDEF   _RealEXPUNGE
   XDEF   _RealSendIO
   XDEF	  _RealCheckIO
   XDEF   _RealWaitIO
   XDEF   _RealAbortIO
   XDEF   _RealDoIO
   XDEF   _RealReplyMsg
   XDEF   _RealOpenDevice
   XDEF   _RealCloseDevice
   
   XDEF   _usecnt

_myBEGIN:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   tst.l   _allunits
   bgt.s   ball

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   bbye
ball:
   move.l  a1,-(sp)            ;push ptr to io request
   jsr     _MyBEGIN
   addq.l  #4,a7
   
bbye:
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   move.l  _RealBEGIN(pc),-(sp)		;continue to real BEGIN
   sub.l   #1,_usecnt
   rts


_myABORT:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   tst.l   _allunits
   bgt.s   aall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   abye
aall:
   move.l  a1,-(sp)            ;push ptr to io request
   jsr     _MyABORT
   addq.l  #4,a7
   
abye:
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   move.l  _RealABORT(pc),-(sp)		;continue to real ABORT
   sub.l   #1,_usecnt
   rts


_myCLOSE:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   tst.l   _allunits
   bgt.s   call

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   cbye
call:
   move.l  a1,-(sp)            ;push ptr to ior
   jsr     _MyCLOSE
   addq.l  #4,a7

cbye:
   movem.l (sp)+,d0-d1/a0-a1   ;restore registers
   move.l  _RealCLOSE(pc),-(sp)    ;continue to real CLOSE
   sub.l   #1,_usecnt
   rts


_myOPEN:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   tst.l   _allunits
   bgt.s   oall

   cmp.l   _unitnum,d0	       ;unit number in d0 for Open
   bne.s   obye
oall:
   move.l  a1,-(sp)            ;push ptr to io request
   jsr     _MyOPEN
   addq.l  #4,a7

obye:
   movem.l (sp)+,d0-d1/a0-a1   	;restore registers
   move.l  _RealOPEN(pc),-(sp)  ;continue to real OPEN
   sub.l   #1,_usecnt
   rts

_myEXPUNGE:
   add.l  #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   tst.l   _allunits
   bgt.s   eall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   ebye

eall:
   jsr     _MyEXPUNGE

ebye:
   movem.l (sp)+,d0-d1/a0-a1   ;restore registers
   moveq.l #0,d0               ;means unable to expunge
   sub.l   #1,_usecnt
   rts                         ;keep changed device from being expunged




_mySendIO:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   sbye

   tst.l   _allunits
   bgt.s   sall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   sbye
sall:
   move.l  a1,-(sp)            ;push ptr to io request
   jsr     _MySendIO
   addq.l  #4,a7
   
sbye:
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   move.l  _RealSendIO(pc),-(sp)	;continue to real rtn
   sub.l   #1,_usecnt
   rts



_myWaitIO:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   wbye

   tst.l   _allunits
   bgt.s   wall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   wbye
wall:
   move.l  a1,-(sp)            ;push ptr to io request
   jsr     _MyWaitIO
   addq.l  #4,a7
   
wbye:
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   move.l  _RealWaitIO(pc),-(sp)	;continue to real rtn
   sub.l   #1,_usecnt
   rts

_myAbortIO:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   aibye

   tst.l   _allunits
   bgt.s   aiall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   aibye
aiall:
   move.l  a1,-(sp)            ;push ptr to io request
   jsr     _MyAbortIO
   addq.l  #4,a7
   
aibye:
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   move.l  _RealAbortIO(pc),-(sp)	;continue to real rtn
   sub.l   #1,_usecnt
   rts



_myDoIO:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   dbye

   tst.l   _allunits
   bgt.s   dall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   dbye
dall:
   move.l  a1,-(sp)             ;push ptr to io request
   jsr     _MyDoIO
   addq.l  #4,a7
   bra.s   dbbye		;go to the exit with the comeback
   
dbye:
   movem.l (sp)+,d0-d1/a0-a1   		;leaving for good - restore registers
   move.l  _RealDoIO(pc),-(sp)		;continue to real rtn
   sub.l   #1,_usecnt
   rts

dbbye:
   movem.l (sp)+,d0-d1/a0-a1   		;we'll be back - restore registers
   move.l  a1,-(sp)			;save ior
   pea.l   dback(pc)			;we want it to come back here
   move.l  _RealDoIO(pc),-(sp)		;continue to real rtn
   rts
dback:
   movem.l d0-d1/a0-a1,-(sp)   		;save registers
   move.l  d0,-(sp)
   move.l  20(sp),-(sp)		        ;get old ior ptr and push again
   jsr     _MyDoIORts
   addq.l  #8,a7			;remove new pushes
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   addq.l  #4,a7			;get rid of old push
   sub.l   #1,_usecnt
   rts


_myCheckIO:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   chbye

   tst.l   _allunits
   bgt.s   chall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   chbye
chall:
   move.l  a1,-(sp)             ;push ptr to io request
   jsr     _MyCheckIO
   addq.l  #4,a7
   bra.s   chbbye		;go to the exit with the comeback
   
chbye:
   movem.l (sp)+,d0-d1/a0-a1   		;leaving for good - restore registers
   move.l  _RealCheckIO(pc),-(sp)	;continue to real rtn
   sub.l   #1,_usecnt
   rts

chbbye:
   movem.l (sp)+,d0-d1/a0-a1   		;we'll be back - restore registers
   move.l  a1,-(sp)			;save ior
   pea.l   chback(pc)			;we want it to come back here
   move.l  _RealCheckIO(pc),-(sp)	;continue to real rtn
   rts
chback:
   movem.l d0-d1/a0-a1,-(sp)   		;save registers
   move.l  d0,-(sp)
   move.l  20(sp),-(sp)		        ;get old ior ptr and push again
   jsr     _MyCheckIORts
   addq.l  #8,a7			;remove new pushes
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   addq.l  #4,a7			;get rid of old push
   sub.l   #1,_usecnt
   rts


_myReplyMsg:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers


   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   rbye

   tst.l   _allunits
   bgt.s   rall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   rbye
rall:
   move.l  a1,-(sp)            ;push ptr to io request
   jsr     _MyReplyMsg
   addq.l  #4,a7
   
rbye:
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   move.l  _RealReplyMsg(pc),-(sp)	;continue to real rtn
   sub.l   #1,_usecnt
   rts



_myOpenDevice:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   	;save registers

   movem.l d0-d1/a0-a1,-(sp)	; save again during stricmp
   move.l  a0,-(sp)
   move.l  _deviceName,-(sp)
   jsr	   _stricmp		; is this our device ?
   addq	   #8,sp
   tst.l   d0
   beq.s   odus			; yes
   movem.l (sp)+,d0-d1/a0-a1	; saved before stricmp
   bra.s   odbye		; no
odus:
   movem.l (sp)+,d0-d1/a0-a1	; saved before stricmp
   tst.l   _allunits
   bgt.s   odall

   cmp.l   _unitnum,d0	       	;unit number in d0 for Open
   bne.s   odbye
odall:
   move.l  a1,-(sp)            	;push ptr to io request
   jsr     _MyOpenDevice
   addq.l  #4,a7
   bra.s   odbbye		;go to the exit with the comeback

odbye:
   movem.l (sp)+,d0-d1/a0-a1   	;restore registers
   move.l  _RealOpenDevice(pc),-(sp)  ;continue to real rtn
   sub.l   #1,_usecnt
   rts

odbbye:
   movem.l (sp)+,d0-d1/a0-a1   		;we'll be back - restore registers
   move.l  a1,-(sp)			;save ior
   pea.l   odback(pc)			;we want it to come back here
   move.l  _RealOpenDevice(pc),-(sp)		;continue to real rtn
   rts
odback:
   movem.l d0-d1/a0-a1,-(sp)   		;save registers
   move.l  d0,-(sp)
   move.l  20(sp),-(sp)		        ;get old ior ptr and push again
   jsr     _MyOpenDevRts
   addq.l  #8,a7			;remove new pushes
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   addq.l  #4,a7			;get rid of old push
   sub.l   #1,_usecnt
   rts


_myCloseDevice:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)   ;save registers

   move.l  IO_DEVICE(a1),d0
   cmp.l   _TheDevice,d0
   bne.s   cdbye

   tst.l   _allunits
   bgt.s   cdall

   move.l  IO_UNIT(a1),d0
   cmp.l   _unitptr,d0
   bne.s   cdbye
cdall:
   move.l  a1,-(sp)             ;push ptr to io request
   jsr     _MyCloseDevice
   addq.l  #4,a7
   bra.s   cdbbye		;go to the exit with the comeback
   
cdbye:
   movem.l (sp)+,d0-d1/a0-a1   		;leaving for good - restore registers
   move.l  _RealCloseDevice(pc),-(sp)	;continue to real rtn
   sub.l   #1,_usecnt
   rts

cdbbye:
   movem.l (sp)+,d0-d1/a0-a1   		;we'll be back - restore registers
   move.l  a1,-(sp)			;save ior
   pea.l   cdback(pc)			;we want it to come back here
   move.l  _RealCloseDevice(pc),-(sp)	;continue to real rtn
   rts
cdback:
   movem.l d0-d1/a0-a1,-(sp)   		;save registers
   move.l  d0,-(sp)
   move.l  20(sp),-(sp)		        ;get old ior ptr and push again
   jsr     _MyCloseDevRts
   addq.l  #8,a7			;remove new pushes
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   addq.l  #4,a7			;get rid of old push
   sub.l   #1,_usecnt
   rts


_RealBEGIN:	DC.L	0
_RealABORT:	DC.L	0
_RealOPEN:	DC.L	0
_RealCLOSE:	DC.L	0
_RealEXPUNGE:	DC.L	0

_RealSendIO:	DC.L	0
_RealCheckIO:	DC.L	0
_RealDoIO:	DC.L	0
_RealWaitIO:	DC.L	0
_RealAbortIO:	DC.L	0
_RealReplyMsg:	DC.L	0
_RealOpenDevice:	DC.L	0
_RealCloseDevice:	DC.L	0

_usecnt		DC.L	0



  

	END

