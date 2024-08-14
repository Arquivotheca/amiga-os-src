;********************************************************************
;
; cmds.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************


        INCLUDE "assembly.i"

        NOLIST
        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/io.i"
        INCLUDE "exec/errors.i"

        INCLUDE "../janus/i86block.i"
        INCLUDE "../janus/services.i"
        LIST

        INCLUDE "jddata.i" 
        INCLUDE "asmsupp.i"
        INCLUDE "printf.mac"

        XDEF    JDCDummy  
        XDEF    JDCInvalid
        XDEF    JDCRead
        XDEF    JDCWrite
        XDEF    JDCMotor
        XDEF    JDCSeek
        XDEF    JDCFormat
        XDEF    JDCChangeNum
        XDEF    JDCChangeState
        XDEF    JDCProtStatus
                            
        XREF    JDCmds
        XREF    EndCommand
        XREF    ExerciseBlocks          
        XLVO    SendJanusInt   

DEBUG_CODE EQU 1
                    
;------ jdisk.device/Dummy, does nothing but return a good error code ------
JDCDummy        moveq.l #0,d0
        printf  <'JDCDummy:\n'>
ecRts           bsr     EndCommand              ; start next command maybe
                rts                             ; all done

;------ jdisk.device/Invalid -----------------------------------------
JDCInvalid      moveq   #IOERR_NOCMD,d0         ; command not known
        printf  <'JDCInvalid:\n'>
                bra.s   ecRts

;------ jdisk.device/Motor IO_ACTUAL=1 (motor is running always) --------
;------ jdisk.device/ChangeNum IO_ACTUAL=1 (disk is never changed) ------
JDCMotor:
        printf  <'JDCMotor:\n'>
JDCChangeNum    moveq   #1,d0
                move.l  d0,IO_ACTUAL(a1)        ; set IO_ACTUAL
        printf  <'JDCChangeNum:\n'>
                bra     JDCDummy                ; return OK error code

;------ jdisk.device/ChangeState IO_ACTUAL=0 (disk is always there) -----
;------ jdisk.device/ProtStatus IO_ACTUAL=0 (disk never write-protected) 
JDCChangeState:
JDCProtStatus   clr.l   IO_ACTUAL(a1)
        printf  <'JDCProtStatus:\n'>
                bra     JDCDummy                ; return no error

;---------------------------------------------------------------------------
;------ jdisk.device/Read reads requested length from requested offset
;------ breaks up the read into TB_SIZE reads unless there is a bad block
;------ somewhere in the requested area.  If this is the case the read is
;------ broken up into at least 3 pieces, the area before the bad block,
;------ the bad block itself (512 bytes which are mapped to another block)
;------ and the area after the bad block.  More than 1 bad block in the
;------ requested area will result in more reads of smaller areas
;------ NOTE: only a6 is stacked because this code can also be re-entered
;------       from the interrupt handler if the length requested has not
;------       been read yet.  The interrupt handler also stacks a6.
;---------------------------------------------------------------------------
JDCRead         move.l  a6,-(a7)                ; just a6 for ReRead
                clr.l   IO_ACTUAL(a1)           ; nothing read yet
;------ this is where the interrupt handler re-enters the read code --------
ReRead          moveq   #ADR_FNCTN_READ,d0      ; what we are doing
                bsr     CommonSetup             ; map to bad blocks maybe
;------ set up where to go when janus interrupts us to say the data are ready
                move.l  #JDIRead,jd_CmdTerm+IS_CODE(a6)
                bra     CommonSendInt           ; send command and cleanup

;------ jdisk.device/Format --------------------------------------------
; by the time we get here the format has actually been completed so all
; we need to do is cleanup and start the next command in the queue. The
; format command is not considered QUICKIO since it needs to use the
; same janus resources as read and write, which could be pending when a
; format command arrives at the port.  So here's the kluge, as soon as
; we get here, free ourselves from the list and start another command
; (which could be another format command!!).
;-----------------------------------------------------------------------
JDCFormat       move.l  a6,-(sp)                ; so we can kluge in to...
                bra     TermCommand             ; the term command routine

;---------------------------------------------------------------------------
;------ jdisk.device/Write writes requested length from requested offset
;------ breaks up the write into TB_SIZE writes unless there is a bad block
;------ somewhere in the requested area.  If this is the case the write is
;------ broken up into at least 3 pieces, the area before the bad block,
;------ the bad block itself (512 bytes which are mapped to another block)
;------ and the area after the bad block.  More than 1 bad block in the
;------ requested area will result in more writes of smaller areas
;------ NOTE: only a6 is stacked because this code can also be re-entered
;------       from the interrupt handler if the length requested has not
;------       been written yet.  The interrupt handler also stacks a6.
;---------------------------------------------------------------------------
JDCWrite        move.l  a6,-(a7)                ; just a6 for ReWrite
                clr.l   IO_ACTUAL(a1)           ; nothing written yet
;------ this is where the interrupt handler re-enters the write code -------
ReWrite         moveq   #ADR_FNCTN_WRITE,d0     ; what we are doing
                bsr     CommonSetup             ; map bad blocks maybe
;------ set up where to go when janus interrupts us to say the data are ready
                move.l  #JDIWrite,jd_CmdTerm+IS_CODE(a6)
;------ copy data to the shared buffer (d0 holds length from CommonSetup) --
                move.l  IO_DATA(a1),a0          ; source data area
                move.l  jd_TrackBuffer(a6),a1   ; destination
                lsr.w   #2,d0                   ; copying longwords
                bra.s   2$                      ; length is 1 too many
1$              move.l  (a0)+,(a1)+
2$              dbra    d0,1$

;------ Called by both read and write to send the janus interrupt.  The
;------ AmigaDskReq has been set up with all required parameters by now
CommonSendInt   moveq   #JSERV_HARDDISK,d0      ; its a hard disk command
                move.l  jd_JanusBase(a6),a6     ; using janus library
                CALLLVO SendJanusInt            ; get moving PC !
                move.l  (a7)+,a6
                rts                             ; and thats it
                  
;------ CommonSetup --------------------------------------------------
;---- Given a command in d0, sets up the AmigaDskReq with the correct
;---- parameters for the read or write and returns the length of data
;---- being read/written in d0.  Bad block mapping is handled here.
;---------------------------------------------------------------------
CommonSetup     bclr    #IOB_QUICK,IO_FLAGS(a1) ; this isn't quick
                move.l  jd_AmigaDskReq(a6),a0   ; set up dsk request    
                move.w  d0,adr_Fnctn(a0)        ; using this command
                move.w  IO_UNIT+2(a1),adr_Part(a0)      ; this partition
                move.l  IO_OFFSET(a1),adr_Offset(a0)  ; this offset (for now)
                move.l  IO_LENGTH(a1),d0        ; can we use this much data ?
                cmpi.l  #TB_SIZE,d0
                ble.s   1$                      ; yes <= TB_SIZE
                move.l  #TB_SIZE,d0             ; nope, it's too big
;---- this is the bad block mapping patch.  If the chunk of PC hard disk
;---- being read has a bad block in it, then the length and offset of the
;---- current read or write will be adjusted to read the data before the
;---- bad block or the bad block itself.  d0 will contain the length to be
;---- read once we return here.  adr_Offset will be changed if block was bad.
1$              cmpi.w  #ADR_FNCTN_SEEK,adr_Fnctn(a0)
                beq.s   2$                      ; no mapping for seek routine
                jsr     PatchBadBlocks          ; adjust for bad stuff
2$              move.l  d0,adr_Count(a0)        ; maybe adjusted by patch
                move.l  d0,jd_RequestedCount(a6) ; remember for int handlers
                move.l  a1,jd_CmdTerm+IS_DATA(a6)   ; save the IORequest ptr
        printf  <'len=%ld offset=%ld\n'>,adr_Count(a0),adr_Offset(a0)
                rts                             ; and go perform command


;------ jdisk.device/Seek --------------------------------------------
JDCSeek         move.l  a6,-(a7)
                clr.l   IO_LENGTH(a1)           ; no bytes involved here
                clr.l   IO_ACTUAL(a1)
                moveq   #ADR_FNCTN_SEEK,d0
                bsr     CommonSetup
;------ set the device task to complete this routine -------------------
                move.l  #JDISeek,jd_CmdTerm+IS_CODE(a6)
                bra     CommonSendInt

;------ JDI routines -------------------------------------------------
;---- called at interrupt time, a1 points to the IORequest we used
;---------------------------------------------------------------------
JDIRead         move.l  a6,-(a7)                ; just a6 for ReRead
                bsr     CheckError              ; set up regs and lengths
                tst.l   d0                      ; any error
                bne     TermCommand             ; yes, quit command now

                move.l  adr_Count(a0),d0        ; copy the data we read ...
                lsr.w   #2,d0                   ; ... to the callers buffer
                move.l  a1,-(a7)                ; save IORequest pointer
                move.l  jd_TrackBuffer(a6),a0
                move.l  IO_DATA(a1),a1
                sub.l   jd_RequestedCount(a6),a1 ; backup destination address
                bra.s   2$
1$              move.l  (a0)+,(a1)+
2$              dbra    d0,1$
                move.l  (a7)+,a1                ; restore IORequest ptr
                move.l  IO_LENGTH(a1),d0        ; did we read everything
                bne     ReRead                  ; no, read the next bit
                bra.s   TermCommand             ; yes, command completed

JDISeek         move.l  a6,-(a7)
                bsr.s   CheckError              ; just get d0 = error


;------ TermCommand -----------------------------------------------------
;---- Clears the IOB_CURRENT bit in the IORequest and dispatches any new
;---- commands that are waiting in the queue for this device.
;------------------------------------------------------------------------
TermCommand     bsr     EndCommand              ; not current anymore
                move.l  jd_CmdQueue+LH_HEAD(a6),a1
                tst.l   (a1)                    ; command queue is empty
                beq.s   jdiRts
                bset    #IOB_CURRENT,IO_FLAGS(a1)
                bne.s   jdiRts                  ; command already running
                move.w  IO_COMMAND(a1),d0       ; get the command type
;-----------------------------------------------------------------------------
; I am forced to put yet another patch in here!  If this command is format too
; then we must call the synchronous section of the command before dispatching
; the format proper.  I sure hope this section of code is re-entrant!!!
;-----------------------------------------------------------------------------
                cmpi.w  #11,d0                  ; is this format ?
                bne.s   NotFormatHere           ; nope, business as usual
                movem.l a0-a1/d0-d1,-(sp)       ; **** be safe ****
                jsr     ExerciseBlocks          ; do the real format
                movem.l (sp)+,a0-a1/d0-d1       ; and carry on
NotFormatHere   lsl.w   #2,d0
                lea     JDCmds(pc),a0           ; get command address
                move.l  0(a0,d0.w),a0           ; and dispatch it
                jsr     (a0)
jdiRts          move.l  (a7)+,a6                ; stacked by all JDI routines
                rts

;------ CheckError ---------------------------------------------------------
;---- When a command has been performed across the janus interface, this
;---- routine checks to see if it completed successfully.  If it did, then
;---- the various values in the IORequest are updated to reflect the amount
;---- of data that was written or read.  This info is used by the active
;---- JDI routine to determine if a reread or rewrite should be posted.
;---- Also sets up the following regs: a6=device, a0=AmigaDskReq
;---------------------------------------------------------------------------
CheckError      move.l  IO_DEVICE(a1),a6        ; get this device ptr
                move.l  jd_AmigaDskReq(a6),a0   ; and the request we posted
                moveq   #0,d0                   ; assume no error
                move.w  adr_Err(a0),d0          ; get the real error
                tst.b   d0
                bne.s   cmdError                ; we screwed up
                lsr.w   #8,d0
                bne.s   cmdBIOSErr              ; PC found something wrong
                move.l  adr_Count(a0),d0        ; how much did we read/write ?
                cmp.l   jd_RequestedCount(a6),d0
                bne.s   lenError                ; well it didn't do it anyway
                add.l   d0,IO_ACTUAL(a1)        ; update length read/written
                sub.l   d0,IO_LENGTH(a1)        ; this much left to do
                add.l   d0,IO_DATA(a1)          ; new data pointer
                add.l   d0,IO_OFFSET(a1)        ; new disk offset
                moveq   #0,d0                   ; no error occured
cmdError        rts
                                  
cmdBIOSErr      BSET    #IOB_BIOSERR,IO_FLAGS(a1)
                bra.s   cmdError                ; the PC screwed up
lenError        moveq   #JD_WRONGLENGTH,d0
                bra.s   cmdError                ; someone screwed up


;------ JDIWrite ------
JDIWrite        move.l  a6,-(a7)                ; just a6 for ReWrite
                bsr.s   CheckError              ; did it write OK ?
                tst.l   d0
                bne     TermCommand             ; no, quit this command now

                tst.w   IO_UNIT(a1)             ; to be verified?  
                bne.s   verifyRead              ; yes, read it to check it

                tst.l   IO_LENGTH(a1)           ; have we written everything
                beq     TermCommand             ; yep, return the IORequest
                bra     ReWrite                 ; no, loop for another write

verifyRead      move.l  jd_RequestedCount(a6),d0
                sub.l   d0,IO_ACTUAL(a1)        ; back up IO fields ...
                add.l   d0,IO_LENGTH(a1)        ; to read same area again
                sub.l   d0,IO_DATA(a1)
                sub.l   d0,IO_OFFSET(a1)
                moveq   #ADR_FNCTN_READ,d0      ; performing a read now
                bsr     CommonSetup
;------ set the device task to complete this routine ----------------------
                move.l  #JDIVerify,jd_CmdTerm+IS_CODE(a6)
                bra     CommonSendInt           ; post the read request

;---- JDIVerify, gets here when a verifying read request completes --------
JDIVerify:
                move.l  a6,-(a7)                ; must be just a6
                bsr     CheckError              ; did the read go OK
                tst.l   d0
                bne     TermCommand             ; no, return an error
                move.l  adr_Count(a0),d0        ; compare to written data
                lsr.w   #2,d0
                move.l  a2,-(a7)
                move.l  jd_TrackBuffer(a6),a0
                move.l  IO_DATA(a1),a2
                sub.l   jd_RequestedCount(a6),a2
                moveq   #0,d1                   ; assume no error
                bra.s   2$
1$              cmp.l   (a0)+,(a2)+
2$              dbne    d0,1$
                movea.l (a7)+,a2
                bne.s   failedVerify            ; flag set from cmp
                move.l  IO_LENGTH(a1),d0        ; have we done now ?
                bne     ReWrite                 ; nope, write before verify
                bra     TermCommand             ; yes, return IORequest

;---- if verify failed, then just write again (and again, and again.....)
failedVerify    move.l  jd_RequestedCount(a6),d0
                sub.l   d0,IO_ACTUAL(a1)
                add.l   d0,IO_LENGTH(a1)        ; must read this much again
                sub.l   d0,IO_DATA(a1)          ; move to original source data
                sub.l   d0,IO_OFFSET(a1)        ; and original disk dest
                bra     ReWrite                 ; try again

;============================================================================
; NewLength = PatchBadBlocks( Length, AmigaDskReq, IORequest )  device
;    d0                         d0         a0          a1         a6
;
; The bad block patcher.   Checks to see if the current offset is on a bad
; block of the PC disk.  If it is then adr_Offset is set to the offset where
; the bad block is mapped to and d0 is set to 512 (the length of 1 block)
; If the first block is not a mapped bad one then the current offset and
; length are used to see if a bad block is present in the area being read
; or written.  If this is the case, then the read or write is truncated to
; just include the blocks BEFORE the bad area.  The next reread or rewrite
; will find that the whole buffer was not read or written and will update
; the pointers which will now point to the bad block itself.  Refer to the
; top of this routine description to see what happens.
;============================================================================
PatchBadBlocks  movem.l d1-d3/a0-a3,-(sp)
                move.l  d0,d2                   ; save request length
                movea.l a0,a2                   ; save diskrequest struct
                move.w  adr_Part(a2),d0         ; get unit number
                movea.l jd_Units(a6),a0         ; find this unit
10$             cmpa.w  #0,a0                   ; did we find the unit ?
                beq     NoUnitFound             ; nope, exit with nothing done
                cmp.w   BB_Unit(a0),d0          ; do we have the unit pointer
                beq.s   20$                     ; yep, a0=unit
                movea.l BB_Next(a0),a0          ; chain to next unit
                bra.s   10$                     ; and keep going

;---- we have a pointer to the unit in a0, check it has a bad block map -----
20$             cmpi.l  #$DEFACED,BB_Valid(a0)  ; valid bad block map ?
                bne     NoUnitFound             ; nope, exit with nothing done
                movea.l a0,a3                   ; stash unit pointer
                move.l  adr_Offset(a2),d0       ; offset we are looking for
                lea.l   BB_BlockMaps(a3),a0     ; point to block maps

;---- loop, looking for a bad block at the current offset -------------------
30$             move.l  OB_PhysicalBlock(a0),d1 ; get a bad block offset
                bmi     NoUnitFound             ; nothing here for us
                cmp.l   d1,d0                   ; is this a bad block
                beq.s   GotBadBlock             ; yep, go change len/offset
                blt.s   NotBadYet               ; got to first > current
                addq.l  #OB_SIZEOF,a0           ; move to the next
                bra.s   30$                     ; keep looking

;---- BINGO! we found a bad block at the current offset.  Change adr_Offset
;---- to point to the mapped block and change the length to read to 512.
GotBadBlock     move.l  OB_MappedBlock(a0),adr_Offset(a2)
        printf  <'Found a bad block at %ld\n'>,d0
                move.l  #512,d2                 ; the amount to read now
                bra     NoUnitFound             ; all done

;---- didn't find a bad block at the current offset but we should check that
;---- the current offset+length doesn't contain the next bad block in the list
;---- On entry here, D0 still contains the current offset for read/write.
NotBadYet       sub.l   d0,d1                   ; length from current to bad
                cmp.l   d1,d2                   ; is it > length we are using
                ble.s   NoUnitFound             ; yes, so bad not here
                move.l  d1,d2                   ; no, truncate to area before
        printf  <'Truncating block\n'>
                                                ; following bad block
NoUnitFound     move.l  d2,d0                   ; restore request length
                movem.l (sp)+,d1-d3/a0-a3
                rts

                END


