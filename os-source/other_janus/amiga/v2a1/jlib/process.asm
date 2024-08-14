;*********************************************************************
;
; process.asm -- execute amiga code from the pc
;
; Copyright (c) 1986, Commodore Amiga Inc.  All rights reserved
;
;*********************************************************************

      INCLUDE "assembly.i"

      NOLIST
      INCLUDE "exec/types.i"
      INCLUDE "libraries/dos.i"

      INCLUDE "janus/i86block.i"
      INCLUDE "janus/janus.i"
      INCLUDE "janus/janusvar.i"
      INCLUDE "janus/janusreg.i"
      INCLUDE "janus/services.i"

      INCLUDE "asmsupp.i"
      LIST

      XDEF   AmigaCall

      XLIB   AllocSignal
      XLIB   CloseLibrary  
      XLIB   GetCC
      XLIB   OpenLibrary
      XLIB   Panic
      XLIB   SetSR
      XLIB   Wait

      XLIB   SendJanusInt

      XLIB   Exit

      XREF   _AbsExecBase
;------ AmigaCall ----------------------------------------------------
;   
;
;
AmigaCall:


;------ ACallSegment -------------------------------------------------
;   
;   This code looks like a piece of code that has been LoadSeg'ed
;   so that the BPTR of it can be passed to CreateProc.  It
;   implements the actual process environment of the AmigaCall.
;
   CNOP   0,4          ; longword align this code
ACallSegment:
      DC.L   0       ; next segment doesn't exist

      ;------ start from nothing ...            
      movea.l _AbsExecBase,a6
      moveq   #ACPSB_CALL,d0
      CALLSYS AllocSignal
      tst.l   d0
      bmi   acsPanic

      moveq   #ACPSB_EXIT,d0
      CALLSYS AllocSignal
      tst.l   d0
      bmi   acsPanic

waitProc:
      move.l   #ACPSF_CALL+ACPSF_EXIT,d0
      move.l   _AbsExecBase,a6
      CALLSYS Wait

      btst   #ACPSB_CALL,d0
      bne   checkExit

      moveq   #0,d0
      lea   jlName(pc),a1
      CALLSYS OpenLibrary

      ;------ janus.library will always exist
      movea.l d0,a6             

      ;------ get s68 area
      movea.l ja_ParamMem(a6),a1
      adda.l   #WordAccessOffset,a1
      moveq   #0,d0
      move.w   jb_Parameters(a1),d0
      move.w   JSERV_AMIGACALL*2(a1,d0.l),d0
      lea   0(a1,d0.l),a0

      moveq   #0,d0
      move.w   s68_ArgLength(a0),d0   ; get stack array size
      move.l   s68_ArgStack(a0),a1   ; get base of stack array
      add.l   d0,a1         ; adjust for predecrement mode
      lsr.w   #1,d0         ; get array word count
      bra.s   2$
1$
      move.w   -(a1),-(a7)      ; copy array to stack
2$
      dbf   d0,1$
      pea   acSubReturn(pc)    ; push return address
      move.l   s68_PC(a0),-(a7)   ; push jump address
      move.w   s68_CCR(a0),d0      ; set CCR
      move.w   #$00ff,d1      ;   (just the CCR)
      CALLSYS SetSR
      movem.l s68_D0(a0),d0-d7/a0-a6   ; get user registers
      rts            ; "jsr s68_PC(a0)"
acSubReturn:
      movem.l d0-d1/a0-a1/a6,-(a7)   ; save work registers

      movea.l _AbsExecBase,a6
      CALLSYS GetCC
      move.w   d0,-(a7)      ; save CCR

      ;------ start from nothing, again ...         
      moveq   #0,d0          
      lea   jlName(pc),a1
      CALLSYS OpenLibrary
      ;------ janus.library will always exist
      movea.l d0,a6             

      ;------ get s68 area
      movea.l ja_ParamMem(a6),a1
      adda.l   #WordAccessOffset,a1
      moveq   #0,d0
      move.w   jb_Parameters(a1),d0
      move.w   JSERV_AMIGACALL*2(a1,d0.l),d0
      lea   0(a1,d0.l),a0
      
      ;------ stash valid address registers
      movem.l a2-a5,s68_A2(a0)   ; stash a2-a5

      ;------ get remaining valid registers
      move.l   a0,a2         ; save s68 area
      move.l   a6,a3         ; save janus.library
      move.w   (a7)+,s68_CCR(a2)   ; stash CCR
      movem.l (a7)+,d0/d1/a0/a1
      move.l   (a7)+,s68_A6(a2)   ; stash a6
      movem.l d0-d7/a0-a1,s68_D0(a2)   ; stash d0-d7/a0-a1
      moveq   #0,d0
      move.w   s68_ArgLength(a2),d0
      add.l   d0,a7         ; restore stack

      moveq   #JSERV_AMIGACALL,d0
      move.l   a3,a6
      CALLSYS SendJanusInt

      ;------ close janus.library twice ('cause opened twice)
      move.l   a3,a1
      movea.l _AbsExecBase,a6
      CALLSYS CloseLibrary
      move.l   a3,a1
      CALLSYS CloseLibrary

      bra   waitProc


checkExit:
      btst   #ACPSB_EXIT,d0
      bne   waitProc

      moveq   #0,d0
      lea   dlName(pc),a1
      CALLSYS OpenLibrary   ; !!! ARGGH, I cannot close this !!!

      move.l   d0,a6      ; always there (or wouldn't be here)
      moveq   #0,d1
      CALLSYS Exit

acsPanic:
      move.l   #$7fffffac,d0
      CALLSYS Panic

jlName:    JANUSNAME
dlName:    DOSNAME

   END


