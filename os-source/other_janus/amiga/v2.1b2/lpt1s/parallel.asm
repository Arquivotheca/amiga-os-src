;*********************************************************************
;
; parallel.asm -- lpt1 driver for janus
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
;
;*********************************************************************

sidecar		equ	1	;0 = bridgeboard, 1 = sidecar

   INCLUDE "assembly.i"

   NOLIST
   INCLUDE "exec/types.i"
   INCLUDE "exec/nodes.i"
   INCLUDE "exec/lists.i"
   INCLUDE "exec/ports.i" 
   INCLUDE "exec/interrupts.i"
   INCLUDE "exec/execbase.i"
   INCLUDE "hardware/intbits.i"
   INCLUDE "hardware/cia.i"
   INCLUDE "resources/misc.i"
   INCLUDE "resources/cia.i"
   INCLUDE "janus/janusbase.i"
   INCLUDE "janus/janusreg.i"
   INCLUDE "janus/services.i"
   LIST

   INCLUDE "asmsupp.i"

   XREF   _JanusBase

   XDEF   _OpenLPT1
   XDEF   _CloseLPT1

   XLVO   AddIntServer
   XLVO   AllocMem
   XLVO   CloseLibrary
   XLVO   FreeMem
   XLVO   OpenLibrary
   XLVO   OpenResource
   XLVO   RemIntServer

   XLVO   AddICRVector
   XLVO   RemICRVector

   XLVO   GetParamOffset
   XLVO   SetJanusHandler
   XLVO   SetJanusEnable
   XLVO   SetJanusRequest

   XLVO   Enable
   XLVO   Disable
   XLVO   RawPutChar

   XREF   _SysBase
   XREF   _ciaa
   XREF   _ciab


;---------------------------------------------------------------------
   SECTION section,data
MiscBase       DS.L   1
CIAABase       DS.L   1
;JanusBase     DS.L   1
StrobeIS       DS.B   IS_SIZE
AcknowledgeIS  DS.B   IS_SIZE

;------ OpenLPT1 -----------------------------------------------------
      SECTION  section,code
_OpenLPT1: 
      movem.l  d2-d7/a2-a6,-(a7)

      ;!!!!!!  try to expunge the parallel.device, if it
      ;!!!!!!  exists, so that if it is not open, it will
      ;!!!!!!  release the misc.resource for the parallel
      ;!!!!!!  port & associated bits
      move.l   #$FFFFFF,d0       ; impossibly large value
      moveq    #0,d1
      move.l   _SysBase,a6
      CALLLVO  AllocMem
      tst.l    d0
      beq.s    purged
      move.l   d0,a1          ; WOW, the impossible happened!
      move.l   #$FFFFFF,d0
      CALLLVO  FreeMem

purged:
      ;------  get ciaa.resource
      lea      craName(pc),a1
      CALLLVO  OpenResource
      move.l   d0,CIAABase
      beq      errOpen1       ; resource not found
   
   IFGE   INFOLEVEL-50
   move.l   d0,-(a7)
   INFOMSG 1,<'CIAABase: 0x%lx'>
   addq.l   #4,a7
   ENDC

      ;------  get misc.resource
      lea      mrName(pc),a1
      CALLLVO  OpenResource
      move.l   d0,MiscBase
      beq      errOpen1       ; resource not found

   IFGE   INFOLEVEL-50
   move.l   d0,-(a7)
   INFOMSG 1,<'MiscBase: 0x%lx'>
   addq.l   #4,a7
   ENDC
      move.l   d0,a6

      ;------  get the parallel port resource itself 
      moveq    #MR_PARALLELPORT,d0 
      lea      lpName(pc),a1
      jsr      MR_ALLOCMISCRESOURCE(a6)
      tst.l    d0
      bne      errOpen1       ; parallel port busy 

      moveq    #MR_PARALLELBITS,d0 
      lea      lpName(pc),a1
      move.l   a1,StrobeIS+LN_NAME ; cache name while it's here
      move.l   a1,AcknowledgeIS+LN_NAME
      jsr      MR_ALLOCMISCRESOURCE(a6)
      tst.l    d0
      bne      errOpen2       ; parallel bits busy 

      ;------  set up the parallel port
      move.b   #$ff,_ciaa+ciaddrb  ; parallel port data direction
      and.b    #$f8,_ciab+ciaddra  ;   bits direction
   
      ;------  get janus.library       
      lea      jlName(pc),a1
      moveq    #0,d0
      move.l   _SysBase,a6
      CALLLVO  OpenLibrary
      move.l   d0,_JanusBase
      beq      errOpen3       ; library not found

   IFGE   INFOLEVEL-50
   move.l   d0,-(a7)
   INFOMSG 1,<'JanusBase: 0x%lx'>
   addq.l   #4,a7
   ENDC
      move.l   d0,a6
      move.l   jb_IoBase(a6),a0
      move.l   a0,StrobeIS+IS_DATA ; cache IO Register base now
      move.l   a0,AcknowledgeIS+IS_DATA

      ;------  set up the parallel port strobe interrupt
      move.l   #JSERV_LPT1INT,d0
      CALLLVO  GetParamOffset
      cmp.w    #$ffff,d0
      bne      errOpen4       ; parallel port already allocated

      moveq    #JSERV_LPT1INT,d0
      lea      StrobeIS,a1
      move.l   #PPStrobe,IS_CODE(a1)
      CALLLVO  SetJanusHandler

      moveq    #JSERV_LPT1INT,d0
      moveq    #0,d1
      CALLLVO  SetJanusRequest     ; don't tell me old news

      moveq    #JSERV_LPT1INT,d0
      moveq    #1,d1
      CALLLVO  SetJanusEnable

      ;------  set up the parallel port acknowledge interrupt
      moveq    #CIAICRB_FLG,d0
      lea      AcknowledgeIS,a1
      move.l   #PPAcknowledge,IS_CODE(a1)
      move.l   CIAABase,a6
      CALLLVO  AddICRVector

      ;------  let the sidecar know things are ready
      clr.w    CharPending
      clr.w    CharWaiting
               move.l   StrobeIS+IS_DATA,a1
      move.b   #%11001000,d0
;     moveq    #$FFFFFFdf,d0 ; not BUSY, not N ACKNOWLEDGE
                             ; not PAPER_OUT, not IOerror
      move.b   d0,jio_Lpt1Status(a1)   ***


	IFEQ	sidecar
	move.b	#~JCNTRLF_CLRBUSY,jio_Control(a1)
	ENDC
	IFNE	sidecar
      bclr.b   #JCNTRLB_CLRBUSY,jio_Control(a1)
	ENDC
      move.l   a1,-(sp)      save iobase pointer
      bsr      updateBits

GotVertbInt    lea.l   VBIS,a1         get int server struct
      clr.l    LN_SUCC(a1)
      clr.l    LN_PRED(a1)
      move.l   (sp)+,IS_DATA(a1)
      move.l   #VBH,IS_CODE(a1)   this code
      move.b   #127,LN_PRI(a1)
      move.l   #BUS,LN_NAME(a1)
      move.b   #NT_INTERRUPT,LN_TYPE(a1)
      moveq.l  #INTB_VERTB,d0
      movea.l  _SysBase,a6
      CALLLVO  AddIntServer

   IFGE   INFOLEVEL-50
   pea   jio_Lpt1Status(a1)
   INFOMSG 1,<'LPT1Status Janus: 0x%lx'>
   move.l   #_ciab+ciapra,(a7)
   INFOMSG 1,<'_ciab+ciapra at: 0x%lx'>
   addq.l   #4,a7
   ENDC

      moveq    #1,d0
endOpen:
      movem.l  (a7)+,d2-d7/a2-a6
      rts

errOpen1:
   INFOMSG 50,<'errOpen1'>
      bsr      cleanup1
      bra      errOpen
errOpen2:
   INFOMSG 50,<'errOpen2'>
      bsr      cleanup2
      bra.s    errOpen
errOpen3:
   INFOMSG 50,<'errOpen3'>
      bsr      cleanup3
      bra.s    errOpen
errOpen4:
   INFOMSG 50,<'errOpen4'>
      bsr.s    cleanup4
errOpen:
      moveq    #0,d0
      bra.s    endOpen


;------ CloseLPT1 ----------------------------------------------------
_CloseLPT1:
      move.l   a6,-(a7)
      bsr.s    cleanup
      move.l   (a7)+,a6
      rts

cleanup:
      moveq.l  #INTB_VERTB,d0
      lea.l    VBIS,a1
      move.l   _SysBase,a6
      jsr      _LVORemIntServer(a6)

      ;------  remove the parallel port acknowledge interrupt
      moveq    #CIAICRB_FLG,d0
      lea      AcknowledgeIS,a1
      move.l   CIAABase,a6
      CALLLVO  RemICRVector

      ;------  remove the parallel port strobe interrupt
      moveq    #JSERV_LPT1INT,d0
      moveq    #0,d1
      move.l   _JanusBase,a6
      CALLLVO  SetJanusEnable

      moveq    #JSERV_LPT1INT,d0
      suba.l   a1,a1 
      CALLLVO  SetJanusHandler
         
      ;------  close janus.library
cleanup4:
      move.l   _JanusBase,a1
      move.l   _SysBase,a6
      CALLLVO  CloseLibrary

      ;------  free parallel bits resource
cleanup3:
      moveq    #MR_PARALLELBITS,d0 
      move.l   MiscBase,a6
      jsr      MR_FREEMISCRESOURCE(a6)

      ;------  free parallel port resource
cleanup2:
      moveq    #MR_PARALLELPORT,d0 
      jsr      MR_FREEMISCRESOURCE(a6)
cleanup1:
      rts

;------ PPStrobe -----------------------------------------------------
;
;   1.  Flag VBH that a character has been sent to the printer
;   2.   Write data byte      
;   3.   Update SELECT and NOPAPER            
;
PPStrobe bsr   StopInts
      tst.w    CharPending      did PC send a double
      bne.s    FUPC         yes, ignore it

      move.b   _ciab+ciapra,d0      can we send character now ?
      btst.b   #CIAB_PRTRBUSY,d0
      beq.s    SendNow         yes, send it out
; printer went busy again so defer the send to the next vertical blank
      move.w   #1,CharWaiting      tell vb we need to send
      bra.s    SendDeferred

SendNow move.b jio_Lpt1Data(a1),_ciaa+ciaprb   send the data byte
;****** fix for sidecar, doesn't set its own busy bit ********
	IFNE	sidecar
      bclr.b   #JPCLCB_BUSY,jio_Lpt1Status(a1)
	ENDC

      move.w   #1,CharPending         tell VB a char sent
      clr.w    CharWaiting         no character waiting
SendDeferred   bsr.s   updateBits

FUPC  bsr      StartInts
      rts

updateBits:
      move.b   _ciab+ciapra,d1
      btst     #CIAB_PRTRSEL,d1      do select line
      beq.s    1$
      bset     #JPCLCB_SELECT,jio_Lpt1Status(a1)
      bra.s    2$
1$    bclr     #JPCLCB_SELECT,jio_Lpt1Status(a1)
2$    btst     #CIAB_PRTRPOUT,d1      and paperout line
      beq.s    3$
      bset     #JPCLCB_NOPAPER,jio_Lpt1Status(a1)
      bra.s    4$
3$    bclr     #JPCLCB_NOPAPER,jio_Lpt1Status(a1)
4$    rts

;------ PPAcknowledge ------------------------------------------------
;
;   1.   drop through and do the VB routine
;
PPAcknowledge:

;==============================================================================
; handler called during vertical blank to keep the busy bit updated and tell
; the PC when we are ready for another character to be sent.
;==============================================================================
VBH   bsr      StopInts      no interrupts now please
      tst.w    CharWaiting      did we defer a send
      beq.s    10$         no, so process normally
      bsr      PPStrobe      pretend pc called us again
      bra.s    StillBusy      and exit now

10$   bsr.s    updateBits      always update the bits
      tst.w    CharPending      did we just send a character
      beq.s    StillBusy      no, no need to update busy

CheckBusy move.b _ciab+ciapra,d0      is printer free yet ?
      btst.b   #CIAB_PRTRBUSY,d0
      bne.s    StillBusy

      clr.w    CharPending      no chars going out now
	IFEQ	sidecar
	move.b	#~JCNTRLF_CLRBUSY,jio_Control(a1)
	ENDC
	IFNE	sidecar
      bset.b   #JCNTRLB_CLRBUSY,jio_Control(a1)  clear busy bits
	ENDC
      bset.b   #JPCLCB_BUSY,jio_Lpt1Status(a1)

      btst     #JPCLSB_IRQENABLE,jio_Lpt1Control(a1)
      beq.s    StillBusy      and interrupt PC maybe
      move.b   #JPCLPT1INT,jio_PcIntGen(a1)

StillBusy bsr  StartInts      interrupts back on
      moveq.l  #0,d0
      rts

;==============================================================================
; Stop any interrupts
;==============================================================================
StopInts movem.l a1/a6,-(sp)
      movea.l  _SysBase,a6
      jsr      _LVODisable(a6)
      movem.l  (sp)+,a1/a6
      rts

;==============================================================================
; Start interrupts again
;==============================================================================
StartInts movem.l a1/a6,-(sp)
      movea.l  _SysBase,a6
      jsr      _LVOEnable(a6)
      movem.l  (sp)+,a1/a6
      rts


lpName:  DC.B  'LPT1.jserver',0
         CNOP  0,2
BUS      DC.B  'busy update',0
         CNOP  0,2
mrName:  MISCNAME
craName: CIAANAME
jlName:  JANUSNAME

         BSS
VBIS     DS.B  IS_SIZE
CharPending   DS.W   1
CharWaiting   DS.W   1
         END

