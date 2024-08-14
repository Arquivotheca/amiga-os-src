;********************************************************************
;
; badblocks.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************

        NOLIST
        INCLUDE "exec/types.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/io.i"
        INCLUDE "../janus/i86block.i"
        INCLUDE "../janus/janus.i"
        INCLUDE "../janus/setupsig.i"
        INCLUDE "../janus/services.i"
        LIST

        INCLUDE "jddata.i" 
        INCLUDE "asmsupp.i"
        INCLUDE "printf.mac"

        XLVO    AllocSignal
        XLVO    CloseLibrary
        XLVO    Disable
        XLVO    Enable
        XLVO    FreeMem
        XLVO    FreeSignal
        XLVO    OpenLibrary 
        XLVO    Wait
        XLVO    AllocMem

        XLVO    AllocJanusMem
        XLVO    CleanupJanusSig
        XLVO    FreeJanusMem
        XLVO    GetParamOffset
        XLVO    JanusMemToOffset
        XLVO    SendJanusInt
        XLVO    SetJanusHandler
        XLVO    SetJanusEnable
        XLVO    SetParamOffset
        XLVO    SetupJanusSig

        XDEF    ExerciseBlocks

;DEBUG_CODE EQU 1

;============================================================================
; This code is patched into beginio and is called whenever a format command
; is found in the IORequest.  jdisk is switched into synchronous mode by
; killing the janus handler and setting up a janus signal in its place.  If
; $DEFACED is not in units BBM then the bad block map area of this partition
; is extensively exercised and tested and a new bad block map is installed.
; The track in the IORequest is then written and re-read multiple times to see
; if there are any bad blocks.  If there are, then these are mapped to good
; blocks in the bad block table which is then written back out to disk. When
; everything is finished, the janus handler is re-enabled and control is
; passed back to BeginIO so that the callers format command is executed.
; Passed the IORequest in A1 and the device pointer in a6.
;============================================================================

ExerciseBlocks  movem.l d2-d7/a2-a6,-(sp)
                movea.l a1,a2                   ; save the IOReq pointer
                movea.l a6,a5                   ; and the device pointer
                movea.l jd_JanusBase(a5),a6     ; stop janus interrupts
                moveq.l #JSERV_HARDDISK,d0      ; for the harddisk
                moveq.l #0,d1
                CALLLVO SetJanusEnable
                move.l  d0,d5                   ; save old int state

                moveq.l #JSERV_HARDDISK,d0      ; get rid of the handler
                suba.l  a1,a1
                CALLLVO SetJanusHandler
                move.l  d0,d6                   ; stash the old handler

                moveq.l #JSERV_HARDDISK,d0      ; get rid of parameter offset
                moveq.l #-1,d1
                CALLLVO SetParamOffset
                move.l  d0,d7                   ; stash the old offset

                movea.l jd_ExecBase(a5),a6      ; get a signal bit
                moveq.l #-1,d0                  ; any one will do
                CALLLVO AllocSignal
                move.l  d0,d4                   ; save to free later
                move.l  d0,d1                   ; setup for AllocJanusSig

                moveq.l #JSERV_HARDDISK,d0      ; this signal please
                moveq.l #AmigaDskReq_SIZEOF,d2  ; this structure
                move.l  #MEMF_PARAMETER!MEM_WORDACCESS,d3
                movea.l jd_JanusBase(a5),a6
                CALLLVO SetupJanusSig           ; get signal struct
                tst.l   d0                      ; did we get it ?
                beq     ExerciseEnd1            ; nope, better quit now

                movea.l d0,a3                   ; save janussig pointer
                move.l  jd_TrackBuffer(a5),d0   ; get buffer address
                CALLLVO JanusMemToOffset
                movea.l ss_ParamPtr(a3),a1
                move.w  d0,adr_BufferAddr(a1)   ; save in disk request struct

                movea.l jd_Units(a5),a0         ; find this unit
FindExUnit      cmpa.w  #0,a0                   ; got to end of list ?
                beq     ExerciseEnd2            ; yep, so error out
                move.l  IO_UNIT(a2),d0          ; is it this unit ?
                cmp.w   BB_Unit(a0),d0
                beq.s   GotExUnit               ; yes, go stash ptr
                movea.l BB_Next(a0),a0          ; chain to next unit
                bra     FindExUnit              ; keep looking

GotExUnit       movea.l a0,a4                   ; stash unit ptr
                move.w  d0,adr_Part(a1)         ; all ops on this partition
                tst.l   IO_OFFSET(a2)           ; if formatting track 0...
                bne     GotBadBlockMap          ; always create bad block map

;=============================================================================
; This disk doesn't have a properly initialised bad block map, so it is time
; to create it.  First, write and read 2 consecutive blocks that will contain
; the memory image of the bad block map.  If the blocks do not pass the test
; then add 512 to RealMapOffset and try the next block along.  Do this up
; to 16 times before failing.  Once this test is complete, the remaining
; blocks in the last 2 tracks are thoroughly tested and entered into the bad
; block map if they pass the tests.  Finally, when all is set up, the bad
; block map is written to the disk ready to be read when units are init'ed.
; Uses some info that was set up by the init sequence, namely, the number of
; blocks there are on the last 2 tracks (BB_TotalBlocks) and the byte offset
; to the first block on these last 2 tracks (BB_BadMapOffset).  
;=============================================================================
NewBadBlockMap  lea.l   BB_Valid(a4),a0         ; put in the test pattern
                move.w  #(1024/4)-1,d1          ; we use 2 blocks (512*2)
                move.l  #$f0f00f0f,d0           ; the test pattern
10$             move.l  d0,(a0)+
                dbra    d1,10$

;---- first find a good place for the bad block map header blocks ----------
                move.l  BB_BadMapOffset(a4),d2  ; first offset to check
                moveq.l #16-1,d3                ; max times to check
GetBBMOffset    movea.l a3,a0                   ; sig struct
                lea.l   BB_Valid(a4),a1         ; test pattern data
                move.l  d2,d0                   ; the offset we want
                move.l  #1024,d1                ; the length to write
                bsr     WriteTestBlock          ; go do it
                tst.l   d0                      ; did it work OK
                beq     GotBBMOffset            ; yep, set up blocks free
                addi.l  #512,d2                 ; failed, move to next
                subi.w  #1,BB_TotalBlocks(a4)   ; 1 less block to use
                dbra    d3,GetBBMOffset         ; and try again maybe
                bra     ExerciseEnd2            ; no good place for bad blocks

;---- we now have a good block area on the disk for our bad block map so
;---- update important variables and mark the block as valid ($DEFACED)
GotBBMOffset    move.l  #$DEFACED,BB_Valid(a4)  ; we now have a bad block map
                move.l  d2,BB_RealMapOffset(a4) ; this is where it lives
                clr.w   BB_NumBlocks(a4)        ; we have mapped this many
                move.w  BB_TotalBlocks(a4),d0   ; how many bad can we map ?
                subq.w  #2,d0                   ; we just used 2
                move.w  d0,BB_MaxBlocks(a4)     ; this is how many are left
                move.w  d0,d3                   ; need count for check loop
                subq.w  #1,d3                   ; fix it for dbra
                addi.l  #1024,d2                ; skip past header blocks
                move.l  a2,-(sp)                ; need a spare address reg
                lea.l   BB_BlockMaps(a4),a2     ; point to mapping data

;---- now check all the blocks we can map to and enter them in the table
;---- if they check out without any errors.  Set OB_PhysicalBlock to -1
CheckMappers    move.l  #-1,OB_PhysicalBlock(a2) ; no block mapped here yet
                movea.l a3,a0                   ; sig struct
                lea.l   BB_Valid(a4),a1         ; test pattern data
                move.l  d2,d0                   ; the offset we want
                move.l  #512,d1                 ; the length to write
                bsr     WriteTestBlock          ; go do it
                tst.l   d0                      ; did it work OK
                beq.s   GotMapper               ; we can map to this one
                subi.w  #1,BB_MaxBlocks(a4)     ; one less available for mapng
                bra     NextMapper              ; go find another
GotMapper       move.l  d2,OB_MappedBlock(a2)   ; we can map to this one
                addq.l  #OB_SIZEOF,a2           ; move to next descriptor
NextMapper      addi.l  #512,d2                 ; move to the next block
                dbra    d3,CheckMappers         ; more to check

;---- OK, bad block map is set up, write it out at the correct offset on disk
                move.l  (sp)+,a2                ; get back spare reg we used
                movea.l a3,a0                   ; sig struct
                lea.l   BB_Valid(a4),a1         ; the bad block map
                move.l  BB_RealMapOffset(a4),d0 ; the offset we want
                move.l  #1024,d1                ; the length to write
                bsr     WriteTestBlock          ; go do it (no error check)

;=============================================================================
; We have a bad block map for this unit so test all the blocks referenced in
; the callers IORequest and map them to good blocks if any of them have any
; problems.  When we return to BeginIO, the format will actually be done so
; the "real" format command is patched to return immediately and not do
; anything else.
;=============================================================================
GotBadBlockMap  move.l  IO_OFFSET(a2),d3        ; this offset
                move.l  IO_LENGTH(a2),d2        ; total length to check
                movem.l a4/d4,-(sp)             ; need more registers
                move.l  #TB_SIZE,d4             ; usual size to read
                movea.l IO_DATA(a2),a4          ; where data comes from
TestCallerBlks  move.l  d3,d0                   ; test at this offset
                movea.l a3,a0                   ; sig struct
                cmp.l   d2,d4                   ; can we write a track ?
                ble.s   10$                     ; yep, carry on reading
                move.l  d2,d4                   ; no, just write whats left
10$             movea.l a4,a1                   ; source data area
                move.l  d4,d1                   ; amount to test
                bsr     WriteTestBlock          ; go do the test
                tst.l   d0                      ; was it good ?
                beq     BlockOK                 ; yep, just carry on

; we detected a bad block somewhere in the last write, d0 contains the offset
; to the offending longword.  Map the block containing the error to a good
; block in the bad block map and write it out there.  The updated bad block
; map must also be written out. Update d4 to reflect that not all of the last
; block was written and carry on writing from the block directly after the
; bad one.  If the bad block table is full, then we just give up and the
; format commands verify cycle will report the error.  Since we can map over
; 120 bad blocks, the disk must be really screwed up anyway if this happens.
                andi.l  #$fffffe00,d0           ; get start of bad block
        printf  <'Bad at %ld when rdng len=%ld at %ld\n'>,d0,d4,d3
                add.l   d0,d3                   ; d3=offset to bad block
                adda.l  d0,a4                   ; pointer to bad data
                sub.l   d0,d2                   ; d2 = length remaining
                movem.l (sp),d0/a0              ; get values we saved
                                                ; a0=unit pointer
                                                ; d0=sig (not used here)
                move.w  BB_NumBlocks(a0),d0
                cmp.w   BB_MaxBlocks(a0),d0     ; any room left to map ?
                beq     NotMapped               ; nope, so leave the error
                addi.w  #1,BB_NumBlocks(a0)     ; one more mapped now
                mulu.w  #OB_SIZEOF,d0           ; index to block map
                lea.l   BB_BlockMaps(a0,d0.w),a1 ; a1 points to next entry
                move.l  d3,OB_PhysicalBlock(a1) ; save in the table
                move.l  OB_MappedBlock(a1),d0   ; where we will map to
                move.l  d0,-(sp)                ; just stash it for later

;---- we have made a change to the bad block map, so write it out again ----
                move.l  BB_RealMapOffset(a0),d0 ; where to write to
                lea.l   BB_Valid(a0),a1         ; where data is
                movea.l a3,a0                   ; sig struct
                move.l  #1024,d1                ; amount to write
                bsr     WriteTestBlock          ; write it out

;---- now write the bad block of data to the new block offset (on the stack)
                movea.l a3,a0                   ; sig struct
                movea.l a4,a1                   ; where the data is
                move.l  (sp)+,d0                ; offset to write to
                move.l  #512,d1                 ; length to write
        printf  <'Mapped to block at %ld\n'>,d0
                bsr     WriteTestBlock          ; go write it out

;---- update offsets and addresses to point beyond the block we mapped ----
NotMapped       move.l  #512,d0
                add.l   d0,d3                   ; this is the new offset
                add.l   d0,a4                   ; new data address
                sub.l   d0,d2                   ; new length remaining
                bra     BlockMapped             ; carry on with the rest

;---- enters here if the whole block written was OK and no block was mapped
BlockOK         add.l   d4,d3                   ; update to new offset
                add.l   d4,a4                   ; and new data area
                sub.l   d4,d2                   ; and get length remaining
BlockMapped     bne     TestCallerBlks          ; go back if not 0
                movem.l (sp)+,a4/d4             ; get back saved registers

;=============================================================================
; various exit points for when things go wrong during patch initialisation.
;=============================================================================

ExerciseEnd2    movea.l a3,a0                   ; clean janus signal
                movea.l jd_JanusBase(a5),a6
                CALLLVO CleanupJanusSig

ExerciseEnd1    move.l  d4,d0                   ; free up the signal
                bmi.s   ExerciseEnd0            ; didn't get it
                movea.l jd_ExecBase(a5),a6
                CALLLVO FreeSignal

ExerciseEnd0    movea.l jd_JanusBase(a5),a6
                move.l  d7,d1                   ; reset parameter offset
                moveq.l #JSERV_HARDDISK,d0
                CALLLVO SetParamOffset

                moveq.l #JSERV_HARDDISK,d0      ; reset to the old handler
                movea.l d6,a1
                CALLLVO SetJanusHandler

                move.l  d5,d1                   ; set janus int back...
                moveq.l #JSERV_HARDDISK,d0      ; ...to what it was...
                CALLLVO SetJanusEnable          ; ...for the harddisk
                movem.l (sp)+,d2-d7/a2-a6
                rts

;===========================================================================
; Result = WriteTestBlock( byteoffset, length, sigstruct, data )
;   d0                          d0       d1       a0       a1
;
; Returns 0 if no errors were detected or d0 = offset to the bad longword.
; Writes data out to the hard disk and then just reads it back again to check
; that nothing is screwed up.  Complements data in the buffer to ensure that
; the comparison will fail in case the PC didn't read anything at all.
;===========================================================================
WriteTestBlock  movem.l d2-d4/a2-a3,-(sp)
                move.l  d0,d2                   ; save byte offset
                move.l  d1,d3                   ; save length
                lsr.w   #2,d1                   ; get length in longwords
                subq.w  #1,d1                   ; for dbra loops
                move.w  d1,d4                   ; stash to save later calcs
                movea.l a0,a2                   ; save sigstruct
                movea.l a1,a3                   ; save source data area
;---- copy data to the track buffer ---------------------------------------
                movea.l jd_TrackBuffer(a5),a1   ; destination area
                movea.l a3,a0                   ; source area
                move.l  d4,d1                   ; get length in longwords
10$             move.l  (a0)+,(a1)+             ; copy the data
                dbra    d1,10$
;---- write data out to disk ----------------------------------------------
                move.l  d2,d0                   ; byte offset
                move.l  d3,d1                   ; length
                movea.l a2,a0                   ; sigstruct
                bsr     WriteBlock              ; ignore errors for now

;---- block written, convert original data in the trackbuffer to it's
;---- complemented state.  This gaurantees that compares will fail if the
;---- harddisk does not fill the whole buffer with the data we just wrote.
                movea.l jd_TrackBuffer(a5),a1   ; destination area
                move.l  d4,d1                   ; longword count
20$             not.l   (a1)+                   ; complement data
                dbra    d1,20$
;---- now read the data back and compare it to source data ----------------
                move.l  d2,d0                   ; byte offset
                move.l  d3,d1                   ; length
                movea.l a2,a0                   ; signal structure
                bsr     ReadBlock               ; read it back in
                                                ; if error, find where below
                movea.l a3,a1                   ; what we compare against
                movea.l jd_TrackBuffer(a5),a0   ; what we are comparing
                move.l  d4,d1                   ; longword count
30$             move.l  (a0)+,d0                ; get the longword
                cmp.l   (a1)+,d0                ; compare to source data
                bne     Compare1Fail            ; error here, get offset
                dbra    d1,30$                  ; OK so far
                moveq.l #0,d0                   ; no error
                bra.s   TestDone                ; passed the test

;---- failed the test, find out where the error is -------------------------
Compare1Fail    move.l  a0,d0                   ; calculate where error is
                subq.l  #4,d0                   ; went one longword too far
                sub.l   jd_TrackBuffer(a5),d0   ; d0=offset to bad longword
                subq.l  #1,d0                   ; BLOCK BEFORE !!!!
                bgt.s   TestDone                ; return this to caller
                moveq.l #1,d0                   ; need at least 1 to flag err

TestDone        movem.l (sp)+,d2-d4/a2-a3
                rts

;===========================================================================
; Result = ReadBlock( byteoffset, length, sigstruct )
;   d0                   d0         d1       a0
;
; Reads the given block to the track buffer and returns the error code in d0
;===========================================================================
ReadBlock       move.l  d2,-(sp)
                moveq.l #ADR_FNCTN_READ,d2      ; this is what we want to do
                bsr     CommonIOStuff           ; go do it
                move.l  (sp)+,d2
                rts

;===========================================================================
; Result = WriteBlock( byteoffset, length, sigstruct )
;   d0                    d0         d1        a0
;
; Writes a block from the track buffer and returns the error code in d0
;===========================================================================
WriteBlock      move.l  d2,-(sp)
                moveq.l #ADR_FNCTN_WRITE,d2     ; this is what we want to do
                bsr     CommonIOStuff           ; go do it
                move.l  (sp)+,d2
                rts

;===========================================================================
; code common to both read and write.  d2 = the function code for diskreqest
;===========================================================================
CommonIOStuff   movem.l a2-a3/a6,-(sp)
                movea.l jd_JanusBase(a5),a6     ; using janus lib
                movea.l a0,a3                   ; we will need this later
                movea.l ss_ParamPtr(a3),a2      ; get disk request struct
                move.w  d2,adr_Fnctn(a2)        ; this is the function
                move.l  d0,adr_Offset(a2)       ; this offset
                move.l  d1,adr_Count(a2)        ; this many bytes
                moveq.l #JSERV_HARDDISK,d0      ; get ready ...
                CALLLVO SendJanusInt            ; go, read or write
                movea.l jd_ExecBase(a5),a6      ; wait for IO to complete
                move.l  ss_SigMask(a3),d0
                CALLLVO Wait
                moveq.l #0,d0                   ; return error code
                move.w  adr_Err(a2),d0
                movem.l (sp)+,a2-a3/a6
                rts

                END


