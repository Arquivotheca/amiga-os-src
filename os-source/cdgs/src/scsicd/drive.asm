
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/interrupts.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/devices.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/io.i"
        INCLUDE "devices/timer.i"
        INCLUDE "hardware/intbits.i"
        INCLUDE "exec/ables.i"

        INCLUDE "defs.i"
        INCLUDE "cd.i"
        INCLUDE "cdprivate.i"
        INCLUDE "cdgs_hw.i"

        OPT     p=68020

************************************************************************
*                                                                      *
*   External References                                                *
*                                                                      *
************************************************************************

        XREF    MSFBINtoBCD
        XREF    MSFBCDtoBIN
        XREF    LSNtoMSFPOS
        XREF    MSFtoLSNPOS
        XREF    LSNtoMSF
        XREF    MSFtoLSN
        XREF    BCDtoBIN
        XREF    BINtoBCD

        XREF    MuteCD
        XREF    Attenuate
        XREF    StartTransfer
        XREF    SetSubcodeInterrupt
        XREF    ClearSubcodeInterrupt
        XREF    SendTimerReq
        XREF    CDSubCode
        XREF    SCSIDevName
        XREF    SCSIUnit

        XREF    PutHex
        XREF    PutChar

        XREF    READ
        XREF    ClearPrefetchIfError

        XDEF    READTOC
        XDEF    ID_PACKET

        INT_ABLES

************************************************************************
*                                                                      *
*   DoCmd - emulate microcontroller command.                           *
*                                                                      *
*       out:  D0.w = error                                             *
*                                                                      *
************************************************************************

 FUNCTION DoCmd
                lea     DriveCMD(pc),a0                                 ; Call command routine
                move.w  db_CMD(db),d0
                lsl.l   #2,d0
                move.l  0(a0,d0.w),a1
                jsr     (a1)

                clr.w   db_CMD(db)                                      ; Command complete, report error
                tst.b   IO_ERROR(ior)
                bne     1$
                move.b  d0,IO_ERROR(ior)

1$              clr.w   d0                                              ; Set/Clear error flag
                move.b  IO_ERROR(ior),d0
                rts


DriveCMD:
                dc.l    SPIN
                dc.l    STOP
                dc.l    READTOC
                dc.l    SEEK
                dc.l    PLAY
                dc.l    READ
                dc.l    PAUSE
                dc.l    RESUME
                dc.l    SETPLAYMODE
                dc.l    Q_CODE
                dc.l    ID_PACKET



 FUNCTION TestUnitReady

                lea     db_command(db),a0                                   ; Flush out error
                clr.l   (a0)
                clr.w   4(a0)
                move.l  #6,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI

                lea     db_command(db),a0                                   ; Get audio status
                move.b  #S_READ_SUB_CHANNEL,(a0)
                clr.b   1(a0)
                clr.l   2(a0)
                clr.w   6(a0)
                move.b  #2,d1
                move.b  d1,8(a0)
                clr.b   9(a0)
                move.l  #10,d0
                lea     db_SCSIBuffer(db),a1
                jsr     DoSCSI
                bne     2$

                lea     db_command(db),a0                                   ; Is there a disk?
                clr.l   (a0)
                clr.w   4(a0)
                move.l  #6,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI
                bne     2$

                lea     db_SCSIBuffer(db),a1                                ; Current status
                move.b  1(a1),d0
                bne     1$
                move.b  #$15,d0
1$              rts
2$
                move.b  #0,d0                                               ; No disk
                rts






****************************************************************************
*                                                                          *
*   SPIN - Motor on                                                        *
*                                                                          *
****************************************************************************

SPIN:
                move.w  db_Info+CDINFO_Status(db),d0                        ; Read the table of contents if not read yet
                btst    #CDSTSB_TOC,d0
                beq     READTOC

                or.w    #CDSTSF_DISK|CDSTSF_SPIN,db_Info+CDINFO_Status(db)  ; Disk is spinning

                clr.w   d0                                                  ; Spin successful
                rts


****************************************************************************
*                                                                          *
*   STOP - Motor off                                                       *
*                                                                          *
****************************************************************************

STOP:
                and.w   #-1-CDSTSF_SPIN,db_Info+CDINFO_Status(db)           ; Disk is not spinning

                clr.w   d0                                                  ; Stop successful
                rts



****************************************************************************
*                                                                          *
*   READTOC - Read table of contents                                       *
*                                                                          *
****************************************************************************

READTOC:
                move.w  db_Info+CDINFO_Status(db),d0                        ; Has the TOC already been read?
                btst    #CDSTSB_TOC,d0
                beq     1$
                clr.w   d0
                rts
1$
                jsr     ReadTOC                                             ; Read TOC information
                bne     99$

                save    d2                                                  ; Is this a multi-session disk?
                tst.l   db_MultiSession(db)
                beq     98$
                move.l  db_RemapStart(db),d0
                beq     98$

                add.l   #16,d0                                              ; Start with sector 16 of this session
                move.l  d0,d2

                jsr     ReadVolDscType                                      ; Is first VDT 1?  If not, not a bridge-disk
                cmp.w   #1,d0
                bne     98$
2$
                add.l   #1,d2                                               ; Search sectors until terminator is found
                move.l  d2,d0
                jsr     ReadVolDscType
                bmi     98$
                cmp.w   #$FF,d0
                bne     2$

                sub.l   db_RemapStart(db),d2                                ; Store first sector NOT to remap
                add.l   #1,d2
                move.l  d2,db_Remap(db)
98$
                restore d2
                clr.w   d0                                                  ; Return status of TOC
99$             rts


*===========================================================================
*=                                                                         =
*=  ReadTOC - Read TOC information, traverse multiple TOCs                 =
*=                                                                         =
*=      in:     D0 = Sector offset of VolDscType to read                   =
*=                                                                         =
*===========================================================================

ReadTOC:
                clr.l   db_TOCNext(db)                                      ; No next pointer (not a multi-session disk) yet
                clr.l   db_MultiSession(db)
                clr.l   db_Remap(db)
                clr.l   db_RemapStart(db)

                lea     db_TOC(db),a0                                       ; Clear out all other entries
                move.w  #TOCEntry_SIZE,d0
1$              move.b  #0,TOCE_Track(a0,d0.w)
                add.w   #TOCEntry_SIZE,d0
                cmp.w   #TOCEntry_SIZE*101,d0
                bne     1$

                move.b  #1,db_TOC+TOCS_FirstTrack(db)                       ; Clear out summary information
NextSession:
                move.b  db_TOC+TOCS_LastTrack(db),db_MSLastTrackTemp(db)    ; Initialize multi-session variables
                clr.b   db_TOC+TOCS_LastTrack(db)
                move.l  db_TOC+TOCS_LeadOut(db),db_MSLeadOutTemp(db)
                clr.l   db_TOC+TOCS_LeadOut(db)

                move.w  #75,db_CurrentSpeed(db)                             ; - Remember our speed

                lea     db_command(db),a0                                   ; Read TOC
                move.b  #S_READTOC,(a0)
                move.b  #2,1(a0)
                clr.l   2(a0)
                move.l  #10,d0
                move.b  #1,6(a0)
                move.b  #$03,7(a0)                                          ; - 804 bytes
                move.b  #$24,8(a0)
                clr.b   9(a0)
                lea     db_SCSIBuffer(db),a1
                move.l  #804,d1
                jsr     DoSCSI
                bne     toc_error

                save    d2-d4
                lea     db_SCSIBuffer(db),a0
                lea     db_TOC(db),a1
                clr.w   d2
                move.w  #4,d3
                move.w  #TOCEntry_SIZE,d4
2$
                add.w   #1,d2

                move.b  1(a0,d3.w),d0
                lsr.b   #4,d0
                move.b  1(a0,d3.w),d1
                asl.b   #4,d1
                or.b    d1,d0
                move.b  d0,TOCE_CtlAdr(a1,d4.w)
                move.b  d2,TOCE_Track(a1,d4.w)
                move.l  4(a0,d3.w),d0
                move.l  d0,TOCE_Position(a1,d4.w)
3$
                add.w   #8,d3
                add.w   #TOCEntry_SIZE,d4
                cmp.b   3(a0),d2
                blo     2$

                cmp.b   #$aa,2(a0,d3.w)
                bne     3$
                move.l  4(a0,d3.w),d0
                move.l  d0,TOCS_LeadOut(a1)
                move.b  #1,TOCS_FirstTrack(a1)
                move.b  d2,TOCS_LastTrack(a1)    
                restore d2-d4

                DISABLE a1
                move.w  db_Info+CDINFO_Status(db),d0                        ; Modify status
                or.w    #CDSTSF_DISK|CDSTSF_SPIN,d0                         ; - Disk is spinning
                move.b  db_TOC+TOCEntry_SIZE+TOCE_CtlAdr(db),d1             ; - Is this a data disk?
                and.b   #CTLADR_CTLMASK,d1
                cmp.b   #CTL_DATA,d1
                bne     4$
                bset    #CDSTSB_CDROM,d0
4$
                bset    #CDSTSB_TOC,d0                                      ; TOC is now valid
                move.w  d0,db_Info+CDINFO_Status(db)
                ENABLE  a1

                lea     db_command(db),a0                                   ; Get mode of this disk
                move.b  #S_READ_HEADER,(a0)
                clr.b   1(a0)
                move.l  #16,2(a0)
                clr.l   6(a0)
                move.b  #2,d1
                move.b  d1,8(a0)
                clr.b   9(a0)
                move.l  #10,d0
                move.l  db_CDROMPage(db),a1
                lea     ROM_DATA(a1),a1
                jsr     DoSCSI
                move.l  db_CDROMPage(db),a1
                move.b  ROM_DATA(a1),ROM_HEADER+3(a1)

                clr.w   d0                                                  ; ReadTOC successful
                rts
toc_error:
                move.w  #CDERR_NoDisk,d0                                    ; ReadTOC not successful
                rts


*===========================================================================
*=                                                                         =
*=  ReadVolDscType - Read a word of data from a PVD sector                 =
*=                                                                         =
*=      in:     D0 = Sector offset of VolDscType to read                   =
*=                                                                         =
*===========================================================================

ReadVolDscType:
                save    d2/d3                                               ; Calculate offset and reset retry counter
                move.l  d0,d2
                mulu.l  #2048,d2
                move.w  #5,d3
1$
                move.w  #CDCMD_READ,db_CMD(db)                              ; Read data

                lea     db_XferNode(db),a1                                  ; Create transfer entry
                move.l  a1,db_XferEntry(db)
                move.l  d2,db_ARG1(db)                                      ; - Store start block
                lea     db_VolDscType(db),a0
                move.l  a0,db_ARG2(db)                                      ; - Start of XL node
                move.l  a0,CDXL_Buffer(a1)
                move.l  #2,db_ARG3(db)
                move.l  #2,CDXL_Length(a1)
                move.l  #75,db_ARG4(db)                                     ; - Set desired drive speed
                move.l  #2,db_ARG5(db)
                clr.l   CDXL_IntCode(a1)
                clr.l   CDXL_Actual(a1)
                move.b  #1,db_ECC(db)                                       ; - Enable ECC of data
                lea     db_IOR(db),ior
                clr.b   IO_ERROR(ior)
                clr.l   IO_ACTUAL(ior)

                jsr     READ                                                ; Do the read

                bne     99$                                                 ; If no error, return PVD type
                restore d2/d3
                clr.l   d0
                move.b  db_VolDscType(db),d0
                tst.w   d0
                rts
99$
                jsr     ClearPrefetchIfError
                subq.l  #1,d3                                               ; Retry?
                bne     1$

                restore d2/d3
                move.w  #-1,d0                                              ; Error, set negative flag
                rts



************************************************************************
*                                                                      *
*   SEEK - Seek to specified position                                  *
*                                                                      *
*       in:     ARG1 = Seek position                                   *
*                                                                      *
************************************************************************

SEEK:
                lea     db_command(db),a0                               ; Seek to requested position
                move.b  #S_SEEK10,(a0)
                clr.b   1(a0)
                move.l  db_ARG1(db),2(a0)
                clr.l   6(a0)
                move.l  #10,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI

                clr.w   d0                                              ; Seek successful
                rts



************************************************************************
*                                                                      *
*   PLAY - Play audio                                                  *
*                                                                      *
*       in:     ARG1    = Start Position                               *
*               ARG2    = Stop Position                                *
*                                                                      *
************************************************************************

PLAY:
                save    d2

                jsr     MuteCD

                move.l  db_ARG1(db),d0                                  ; Set up play command (start & end)
                move.l  d0,db_PlayStart(db)
                jsr     LSNtoMSFPOS
                move.l  d0,d1
                move.l  db_ARG2(db),d0
                sub.l   #1,d0
                move.l  d0,db_PlayStop(db)
                jsr     LSNtoMSFPOS
                move.l  d0,d2

                move.w  db_Info+CDINFO_Status(db),d0                    ; Start at end of request if in reverse mode
                btst    #CDSTSB_SEARCH,d0
                beq     1$
                btst    #CDSTSB_DIRECTION,d0
                beq     1$
                move.l  db_PlayStop(db),d0
                sub.l   #75,d0
                cmp.l   db_PlayStart(db),d0
                blo     1$
                jsr     LSNtoMSFPOS
                move.l  d0,d1
                move.l  db_PlayStop(db),db_LastQPos(db)
                bra     2$
1$
                move.l  d1,db_LastQPos(db)                              ; Save expected Q-code position
2$              clr.w   db_LastQState(db)

                lea     db_command(db),a0                               ; Send play command to drive
                move.b  #S_PLAY_AUDIO_MSF,(a0)
                clr.b   1(a0)
                move.l  d1,2(a0)
                asl.l   #8,d2
                move.l  d2,6(a0)
                move.l  #10,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI
                bne     90$

                or.w    #CDSTSF_PLAYING,db_Info+CDINFO_Status(db)       ; Play working

                move.l  db_TimerIOR(db),a1                              ; Recalibrate timer
                exec    AbortIO
                jsr     SendTimerReq

                move.w  db_Info+CDINFO_Status(db),d0                    ; If pause mode is set, pause immediately
                btst    #CDSTSB_PAUSED,d0
                beq     3$
                lea     db_command(db),a0                               ; Pause the audio
                move.b  #S_PAUSE_RESUME,(a0)
                clr.b   1(a0)
                clr.l   2(a0)
                clr.l   6(a0)
                move.l  #10,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI
3$
                move.w  db_Attenuation(db),d0                           ; Set desired attenuation
                jsr     Attenuate

                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_PAUSED,d0
                bne     4$

                clr.w   db_AbortTimer(db)
                jsr     SendFrameTimerReq
4$
                restore d2                                              ; Play started successfully
                clr.w   d0
                rts
90$
                restore d2                                              ; Play could not start, probably bad data
                move.w  #CDERR_BadDataType,d0
                rts



************************************************************************
*                                                                      *
*   FrameInt - Frame call interrupt processor                          *
*                                                                      *
************************************************************************

 FUNCTION FrameInt
                save    db/ior

                move.l  a1,db
                move.l  db_IORFrm(db),ior

                tst.w   db_AbortTimer(db)                               ; Request aborted?
                bne     9$

                jsr     SendFrameTimerReq                               ; Send off another request
9$              clr.w   db_AbortTimer(db)

                jsr     CDSubCode                                       ; Call subcode interrupt

                restore db/ior
                clr.w   d0                                              ; Clear server chain
                rts


************************************************************************
*                                                                      *
*   SendFrameTimerReq - Send a timer interrupt request                 *
*                                                                      *
************************************************************************

 FUNCTION SendFrameTimerReq

                move.l  db_IORFrm(db),a1                                ; Send a timer request and interrupt in a 75th/150th
                clr.l   IOTV_TIME+TV_SECS(a1)                           ;   of a second
                move.l  db_Speed(db),IOTV_TIME+TV_MICRO(a1)
                move.w  #TR_ADDREQUEST,IO_COMMAND(a1)
                clr.b   IO_ERROR(a1)

                exec    SendIO
                rts


*
************************************************************************
*                                                                      *
*   PAUSE - Place drive in pause mode                                  *
*                                                                      *
************************************************************************

PAUSE:
                move.w  db_Info+CDINFO_Status(db),d0                    ; Are we playing audio?
                btst    #CDSTSB_PLAYING,d0
                beq     1$

                jsr     ClearSubcodeInterrupt                           ; Turn off frame interrupts

                lea     db_command(db),a0                               ; Pause the audio
                move.b  #S_PAUSE_RESUME,(a0)
                clr.b   1(a0)
                clr.l   2(a0)
                clr.l   6(a0)
                move.l  #10,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI

                move.w  #1,db_AbortTimer(db)
1$
                or.w    #CDSTSF_PAUSED,db_Info+CDINFO_Status(db)        ; We are now in pause mode

                clr.w   d0                                              ; Pause successful
                rts




************************************************************************
*                                                                      *
*   RESUME - Take drive out of pause mode                              *
*                                                                      *
************************************************************************

RESUME:
                and.w   #-1-CDSTSF_PAUSED,db_Info+CDINFO_Status(db)     ; We are no longer in pause mode

                move.w  db_Info+CDINFO_Status(db),d0                    ; Are we playing audio?
                btst    #CDSTSB_PLAYING,d0
                beq     1$

                lea     db_command(db),a0                               ; Resume the audio
                move.b  #S_PAUSE_RESUME,(a0)
                clr.b   1(a0)
                clr.l   2(a0)
                clr.l   6(a0)
                move.b  #$01,8(a0)
                move.l  #10,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI

                move.l  db_TimerIOR(db),a1                              ; Recalibrate timer
                exec    AbortIO
                jsr     SendTimerReq

                clr.w   db_AbortTimer(db)                               ; Restart frame interrupts (if necessary)
                jsr     SendFrameTimerReq
                jsr     SetSubcodeInterrupt
1$
                clr.w   d0                                              ; Resume successful
                rts




****************************************************************************************
*                                                                                      *
*   SETPLAYMODE - Switch the mode of play                                              *
*                                                                                      *
*       in:     ARG1 = PlayMode                                                        *
*                                                                                      *
****************************************************************************************

SETPLAYMODE:
                and.w   #-1-(CDSTSF_SEARCH|CDSTSF_DIRECTION),db_Info+CDINFO_Status(db)  ; Enable search mode?
                tst.l   db_ARG1(db)
                beq     1$
                or.w    #CDSTSF_SEARCH,db_Info+CDINFO_Status(db)

                cmp.l   #1,db_ARG1(db)                                                  ; Which direction do we search?
                beq     1$
                or.w    #CDSTSF_DIRECTION,db_Info+CDINFO_Status(db)
1$
                move.w  db_Info+CDINFO_Status(db),d0                                    ; Are we currently playing audio?
                btst    #CDSTSB_PLAYING,d0
                beq     9$

                move.w  db_Info+CDINFO_Status(db),d0                                    ; Set mode only in normal play
                btst    #CDSTSB_SEARCH,d0
                bne     9$

                jsr     Q_CODE                                                          ; Get current disk play position
                move.l  db_QCode+8(db),d0
                and.l   #$00ffffff,d0
                jsr     MSFtoLSNPOS
                move.l  d0,d1

                move.l  db_PlayStop(db),d2

                move.l  d1,d0                                                           ; Convert positions to MSF
                jsr     LSNtoMSFPOS
                move.l  d0,d1
                move.l  d2,d0
                jsr     LSNtoMSFPOS
                move.l  d0,d2

                lea     db_command(db),a0                                               ; Send play command to drive
                move.b  #S_PLAY_AUDIO_MSF,(a0)
                clr.b   1(a0)
                move.l  d1,2(a0)
                asl.l   #8,d2
                move.l  d2,6(a0)
                move.l  #10,d0
                move.l  #0,a1
                clr.l   d1
                jsr     DoSCSI
9$
                clr.w   d0                                                              ; SetPlayMode successful
                rts



************************************************************************
*                                                                      *
*   QCODE - Retrieve a Q-Code packet                                   *
*                                                                      *
************************************************************************

 FUNCTION Q_CODE

                move.w  db_Info+CDINFO_Status(db),d0                    ; Are we playing audio?
                btst    #CDSTSB_PLAYING,d0
                beq     99$

                move.w  db_Info+CDINFO_Status(db),d0                    ; Have we already reported a packet in pause mode?
                and.w   db_LastQState(db),d0
                btst    #CDSTSB_PAUSED,d0
                bne     1$                                              ; - Then report the same packet again

                lea     db_command(db),a0                               ; Get me a Q-Code packet
                move.b  #S_READ_SUB_CHANNEL,(a0)
                move.b  #$02,1(a0)
                move.b  #$40,2(a0)
                move.b  #1,3(a0)
                clr.l   4(a0)
                move.b  #16,d1
                move.b  d1,8(a0)
                clr.b   9(a0)
                move.l  #10,d0
                lea     db_QCode(db),a1
                jsr     DoSCSI
                bne     2$

                move.b  db_QCode+5(db),d1                               ; Is this a position packet?
                and.b   #$F0,d1
                cmp.b   #$10,d1
                bne     1$

                move.l  db_QCode+8(db),d1                               ; Get Q-Code absolute position
                and.l   #$00FFFFFF,d1

                move.l  d1,db_LastQPos(db)                              ; This is now the last Q position and state
                move.w  db_Info+CDINFO_Status(db),db_LastQState(db)

                clr.b   db_AutoQ(db)                                    ; Turn off QCode interrupts (if they happened to be on)
                jsr     SetSubcodeInterrupt
1$
                clr.w   d0                                              ; Command successful
                rts
2$
                move.l  db_ClassDReq(db),a0
                cmp.b   #CDERR_NoDisk,IO_ERROR(a0)                      ; Was the disk ejected?
                beq     99$
                clr.w   d0                                              ; Return last Q-Code packet
                rts
99$
                move.w  #CDERR_InvalidState,d0                          ; Audio is not playing
                rts




************************************************************************
*                                                                      *
*   ID_PACKET - Get drive ID packet                                    *
*                                                                      *
************************************************************************

ID_PACKET:
                exec    CreateMsgPort                                   ; Create msg port Frm
                move.l  d0,db_SCSIPort(db)
                move.l  d0,a0

                move.l  db_SCSIPort(db),a0                              ; Create I/O request Frm
                move.l  #IOSTD_SIZE,d0
                exec    CreateIORequest
                move.l  d0,db_SCSIIOR(db)

                lea     SCSIDevName(pc),a0                              ; Open scsi.device with this request
                move.l  SCSIUnit(pc),d0
                move.l  db_SCSIIOR(db),a1
                clr.l   d1
                exec    OpenDevice
                tst.b   d0
                bne     99$

                jsr     TestUnitReady                                   ; Is there a disk?
                beq     1$
                or.w    #CDSTSF_CLOSED,db_Info+CDINFO_Status(db)
                move.b  #1,db_OpenState(db)
1$              rts
99$
                move.b  #1,db_NoHardware(db)
                rts



************************************************************************


************************************************************************
*                                                                      *
*   DoSCSI - Send SCSI command to drive                                *
*                                                                      *
*   in:     A0.l = Command *                                           *
*           D0.w = CmdLength (if bit 31 is set, WRITE operation)       *
*           A1.l = Data *                                              *
*           D1.l = DataLength                                          *
*                                                                      *
************************************************************************

 FUNCTION DoSCSI

                tst.b   db_NoHardware(db)                               ; Is drive present?
                beq     0$
                rts
0$
                move.l  a1,db_cmdblk+scsi_Data(db)                      ; Set up command
                move.l  d1,db_cmdblk+scsi_Length(db)
                move.l  a0,db_cmdblk+scsi_Command(db)
                move.w  d0,db_cmdblk+scsi_CmdLength(db)
                clr.w   db_cmdblk+scsi_CmdActual(db)
                move.b  #SCSI_READ|SCSI_AUTOSENSE,d0
                btst    #31,d0
                beq     1$
                move.b  #SCSI_WRITE|SCSI_AUTOSENSE,d0
1$              move.b  d0,db_cmdblk+scsi_Flags(db)
                clr.b   db_cmdblk+scsi_Status(db)
                lea     db_SenseData(db),a0
                move.l  a0,db_cmdblk+scsi_SenseData(db)
                move.l  #20,db_cmdblk+scsi_SenseLength(db)
                clr.w   db_cmdblk+scsi_SenseActual(db)

                move.l  db_SCSIIOR(db),a1                               ; Set up and execute request
                move.w  #HD_SCSICMD,IO_COMMAND(a1)
                lea     db_cmdblk(db),a0
                move.l  a0,IO_DATA(a1)
                clr.l   IO_OFFSET(a1)
                clr.l   IO_ACTUAL(a1)
                move.l  #scsi_SIZEOF,IO_LENGTH(a1)
                exec    DoIO

                move.l  db_SCSIIOR(db),a1                               ; Report status
                clr.l   d0
                move.b  IO_ERROR(a1),d0
                rts



