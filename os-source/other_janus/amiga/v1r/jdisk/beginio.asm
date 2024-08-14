;********************************************************************
;
; beginio.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
; Steve. added the synchronous format patch into PerformCmd.  jan/87
;********************************************************************


        INCLUDE "assembly.i"

        NOLIST
        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/devices.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/io.i"
        INCLUDE "exec/errors.i"
        INCLUDE "exec/ables.i"
        LIST

        INCLUDE "jddata.i" 
        INCLUDE "asmsupp.i"
        INCLUDE "printf.mac"

        XDEF    JDBeginIO
        XDEF    JDAbortIO
        XDEF    JDCmds
        XDEF    EndCommand
                
        XLVO    ReplyMsg    

        XREF    JDCDummy 
        XREF    JDCInvalid
        XREF    JDCRead
        XREF    JDCWrite
        XREF    JDCMotor
        XREF    JDCSeek
        XREF    JDCFormat
        XREF    JDCChangeNum
        XREF    JDCChangeState
        XREF    JDCProtStatus
        XREF    ExerciseBlocks
  
        INT_ABLES

;DEBUG_CODE EQU 1

;------ jdisk.device/BeginIO --------------------------------------------
JDBeginIO       movem.l d2/d3/a2/a3/a6,-(a7)   
                movea.l a6,a2                   ; save device pointer
                movea.l a1,a3                   ; save the IOReq pointer
                andi.b  #(~(IOF_CURRENT!IOF_DONE)),IO_FLAGS(a3)
                clr.b   IO_ERROR(a3)            ; no errors (yet!)
                clr.b   LN_TYPE(a3)             ; it's NOT NT_REPLYMSG

;------ is the command in bounds? ----------------------------------------
                move.w  IO_COMMAND(a3),d2
        printf  <'Got command %d\n'>,d2
                cmp.w   #NUM_COMMANDS,d2        ; check against max
                blt.s   cmdExists               ; nope, too big
                moveq   #0,d2                   ; point to error vector

cmdExists       move.l  #IMMEDIATE_COMMANDS,d0  ; can we do this now ?
                btst    d2,d0
                bne     performCmd              ; yep, doesn't need I/O
                                          
;------ command needs Janus I/O so queue it in case Janus is busy --------
                moveq   #0,d3                   ; cmd won't be dispatched
                DISABLE a0
                tst.b   IO_ERROR(a3)            ; did it fail already ?
                bne.s   queueEnable             ; don't queue if an error
                lea     jd_CmdQueue(a2),a0      ; add to jdisk queue
                ADDTAIL                         ; a1=IORequest still

;------ check if this request is first in the queue ----------------------
                move.l  jd_CmdQueue+LH_HEAD(a2),a0
                cmpa.l  a0,a3
                bne.s   queueEnable             ; nope, so just leave there

;------ mark this command as current so we execute it straight away ------
                bset    #IOB_CURRENT,IO_FLAGS(a3)
                seq     d3                      ; set if to be dispatched

queueEnable     ENABLE  a0

;------ is this at the top of the queue, if yes then go start it ----------
                tst.b   d3                      ; 0 if this is not current
                bne.s   performCmd              ; it's current, start it up

;------ this command will be executed later so it's not quick anymore -----
                bclr    #IOB_QUICK,IO_FLAGS(a3)
                bra.s   endBIO                  ; return to caller

;------ command at queue head should be executed now.  If this is a format
;------ command then call the synchronous patch to do bad block mapping or
;------ bad block table creation before performing the callers format.
performCmd      move.l  a3,a1                   ; set up regs for commands
                move.l  a2,a6                   ; a1=IORequest a6=devptr
                cmpi.w  #11,d2                  ; is this format
                bne.s   NotFormat               ; nope, just execute it

                jsr     ExerciseBlocks          ; patch! go hammer on blocks
                movea.l a3,a1                   ; fixup IOReq ptr again

NotFormat       lsl.w   #2,d2                   ; get offset to command
                move.l  JDCmds(pc,d2.w),a0
                jsr     (a0)                    ; call this command

endBIO          movem.l (a7)+,d2/d3/a2/a3/a6 
                rts


JDCmds          dc.l    JDCInvalid      ; error vector
                dc.l    JDCDummy        ; Reset
                dc.l    JDCRead
                dc.l    JDCWrite
                dc.l    JDCDummy        ; Update
                dc.l    JDCDummy        ; Clear
                dc.l    JDCDummy        ; Stop
                dc.l    JDCDummy        ; Start
                dc.l    JDCDummy        ; Flush

                dc.l    JDCMotor
                dc.l    JDCSeek
                dc.l    JDCFormat       ; not asynchronous anymore
                dc.l    JDCDummy        ; Remove
                dc.l    JDCChangeNum
                dc.l    JDCChangeState
                dc.l    JDCProtStatus

NUM_COMMANDS    EQU     (*-JDCmds)/4
IMMEDIATE_COMMANDS  EQU $0f3f3


;------ jdisk.device/AbortIO -----------------------------------------
JDAbortIO       DISABLE a0
                btst    #IOB_CURRENT,IO_FLAGS(a1)
                beq.s   abortable

                ;------ already running: can't make command complete faster
                ENABLE  a0
                rts

abortable       moveq   #IOERR_ABORTED,d0
                bra.s   abortEndCommand


;------ EndCommand ---------------------------------------------------
EndCommand      DISABLE a0

abortEndCommand movem.l a2/a3/a6,-(a7)
                move.l  a1,a3
                move.l  a6,a2
                
                bset    #IOB_DONE,IO_FLAGS(a3)
                bne     endEnable
                move.b  d0,IO_ERROR(a3)
                
                ;------ is this command from the queue
                btst    #IOB_CURRENT,IO_FLAGS(a3)
                beq.s   endNotQueued

                move.l  a3,a1
                REMOVE

                ;------ is this command still quick?
endNotQueued    btst    #IOB_QUICK,IO_FLAGS(a3)
                bne.s   endEnable
                
                ;------ reply the command
                move.l  a3,a1
                move.l  jd_ExecBase(a2),a6
                CALLLVO ReplyMsg

endEnable       ENABLE  a0
                movem.l (a7)+,a2/a3/a6 
                rts

        END
