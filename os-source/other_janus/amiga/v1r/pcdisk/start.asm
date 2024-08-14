;==============================================================================
; Handler code to allow the PC program jlink to access files on the Amiga side.
; The PC will use these files as virtual volumes and access them as disk units.
;
; Copyright (c) 1987, Commodore Amiga Inc., All rights reserved
;==============================================================================

      INCLUDE   "mydata.i"

      NOLIST
      INCLUDE "exec/memory.i"
      INCLUDE "/j2500/i86block.i"
      INCLUDE "/j2500/janus.i"
      INCLUDE "/j2500/setupsig.i"
      INCLUDE "/j2500/services.i"
      INCLUDE   "printf.mac"
      LIST

      XREF   _AbsExecBase
      XREF   _LVOAllocSignal,_LVOOpenLibrary
      XREF   _LVOFreeSignal,_LVOCloseLibrary

      XREF   _LVOAllocJanusMem,_LVOFreeJanusMem
      XREF   _LVOSetupJanusSig,_LVOCleanupJanusSig
      XREF   _LVOJanusMemToOffset

      XREF   MainProg

      XDEF   _main

DEBUG_CODE EQU 1

_main      lea.l   -md_SIZEOF(sp),sp   data space on stack
   DEBUGINIT
   printf   <'PCDisk starting up\n'>
      movea.l   sp,a5         always pointed to by a5
      movea.l   a5,a0         clear all pointers
      moveq.l   #(md_SIZEOF/2)-1,d0   and un-initialised data
10$      clr.w   (a0)+
      dbra   d0,10$

      movea.l   _AbsExecBase,a6      get exec library
      move.l   a6,md_ExecLib(a5)   and stash it

      lea.l   JLibName(pc),a1      open janus library
      moveq.l   #LIBVERSION,d0
      jsr   _LVOOpenLibrary(a6)
      move.l   d0,md_JanusLib(a5)
      beq   StartError1      didn't get it

      lea.l   DosName(pc),a1      open dos library
      moveq.l   #LIBVERSION,d0
      jsr   _LVOOpenLibrary(a6)
      move.l   d0,md_DosLib(a5)
      beq   StartError2      didn't get it

      moveq.l   #-1,d0         get a signal bit for janus
      jsr   _LVOAllocSignal(a6)
      move.w   d0,md_SigBit(a5)
      bmi.s   StartError3
      moveq.l   #1,d1         convert to a signal mask
      lsl.l   d0,d1
      move.l   d1,md_SigMask(a5)

      move.l   d0,d1         set up janus signal
      moveq.l   #JSERV_NEWASERV,d0   this signal please
      moveq.l   #AmigaDskReq_SIZEOF,d2   this structure size for parms
      move.l   #MEMF_PARAMETER!MEM_WORDACCESS,d3
      movea.l   md_JanusLib(a5),a6
      jsr   _LVOSetupJanusSig(a6)   get signal struct
      move.l   d0,md_JanusSig(a5)   did we get it ?
      beq.s   StartError4      nope, better quit now

      move.l   #DBUFSIZE,d0      allocate data buffer
      move.l   #MEMF_BUFFER!MEM_BYTEACCESS,d1
      jsr   _LVOAllocJanusMem(a6)   in byte access memory
      move.l   d0,md_DataBuffer(a5)   and save the pointer
      beq.s   StartError5      didn't get it
      jsr   _LVOJanusMemToOffset(a6) convert physical to offset
      movea.l   md_JanusSig(a5),a0
      movea.l   ss_ParamPtr(a0),a0   get parameter mem address
      move.l   a0,md_ParamBlock(a5)   and stash it for later use
      move.w   d0,adr_BufferAddr(a0)   set up diskreq buffer address

      jsr   MainProg      go wait for everything

;==============================================================================
; Exit points if anything goes wrong during init, not called once running.
;==============================================================================

StartError5   movea.l   md_JanusLib(a5),a6   cleanup janus signal
      movea.l   md_JanusSig(a5),a0
      jsr   _LVOCleanupJanusSig(a6)
StartError4   movea.l   md_ExecLib(a5),a6   free signal bit
      move.w   md_SigBit(a5),d0
      jsr   _LVOFreeSignal(a6)
StartError3   movea.l   md_DosLib(a5),a1   close DOS library
      jsr   _LVOCloseLibrary(a6)
StartError2   movea.l   md_JanusLib(a5),a1   close janus library
      jsr   _LVOCloseLibrary(a6)
StartError1   moveq.l   #100,d0         error return
      lea.l   md_SIZEOF(sp),sp   reclaim stack space
      rts

JLibName   DC.B   'janus.library',0
      CNOP   0,2
DosName      DC.B   'dos.library',0
      CNOP   0,2

      END


