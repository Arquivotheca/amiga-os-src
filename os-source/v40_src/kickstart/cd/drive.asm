
        INCLUDE "include:exec/types.i"
        INCLUDE "include:exec/nodes.i"
        INCLUDE "include:exec/lists.i"
        INCLUDE "include:exec/ports.i"
        INCLUDE "include:exec/memory.i"
        INCLUDE "include:exec/interrupts.i"
        INCLUDE "include:exec/libraries.i"
        INCLUDE "include:exec/tasks.i"
        INCLUDE "include:exec/devices.i"
        INCLUDE "include:exec/execbase.i"
        INCLUDE "include:exec/io.i"
        INCLUDE "include:hardware/intbits.i"
        INCLUDE "include:exec/ables.i"

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
        XREF    LightOn
        XREF    LightOff
        XREF    Attenuate
        XREF    StartTransfer
        XREF    SetSubcodeInterrupt
        XREF    ClearSubcodeInterrupt

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



****************************************************************************
*                                                                          *
*   SPIN - Motor on                                                        *
*                                                                          *
****************************************************************************

SPIN:
                move.w  db_Info+CDINFO_Status(db),d0                        ; Read the table of contents if not read yet
                btst    #CDSTSB_TOC,d0
                beq     READTOC

                move.b  #CHCMD_PAUSE,db_Packet(db)                          ; Start the disk spinning
                jsr     DoPacket
                bmi     99$

                or.w    #CDSTSF_DISK|CDSTSF_SPIN,db_Info+CDINFO_Status(db)  ; Disk is spinning

                clr.w   d0                                                  ; Spin successful
                rts
99$
                move.w  #CDERR_NoDisk,d0                                    ; Could not spin up disk
                rts


****************************************************************************
*                                                                          *
*   STOP - Motor off                                                       *
*                                                                          *
****************************************************************************

STOP:
                move.l  #(CHCMD_STOP<<24)|$010000,db_Packet(db)             ; Stop the disk from spinning
                jsr     DoPacket

                and.w   #-1-CDSTSF_SPIN,db_Info+CDINFO_Status(db)           ; Disk is not spinning

                clr.w   d0                                                  ; Stop successful
                rts



****************************************************************************
*                                                                          *
*   READTOC - Read table of contents                                       *
*                                                                          *
****************************************************************************

READTOC:
                move.l  #SIGF_TOCDONE,d1                                    ; Make sure our signal bits are clear
                clear   d0
                exec    SetSignal

                move.w  db_Info+CDINFO_Status(db),d0                        ; Has the TOC already been read?
                btst    #CDSTSB_TOC,d0
                beq     1$
                clr.w   d0
                rts
1$
                jsr     ReadTOC                                             ; Read TOC information
                bne     99$

                move.w  db_Info+CDINFO_Status(db),d0                        ; Is this a data disk?
                btst    #CDSTSB_CDROM,d0
                beq     98$

                save    d2                                                  ; Is this a multi-session disk?
                tst.l   db_MultiSession(db)
                beq     97$
                move.l  db_RemapStart(db),d0
                beq     97$

                add.l   #16,d0                                              ; Start with sector 16 of this session
                move.l  d0,d2

                jsr     ReadVolDscType                                      ; Is first VDT 1?  If not, not a bridge-disk
                cmp.w   #1,d0
                bne     97$
2$
                add.l   #1,d2                                               ; Search sectors until terminator is found
                move.l  d2,d0
                jsr     ReadVolDscType
                bmi     97$
                cmp.w   #$FF,d0
                bne     2$

                sub.l   db_RemapStart(db),d2                                ; Store first sector NOT to remap
                add.l   #1,d2
                move.l  d2,db_Remap(db)
97$
                restore d2
98$
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
                clr.b   db_CDICount(db)
NextSession:
                move.b  db_TOC+TOCS_LastTrack(db),db_MSLastTrackTemp(db)    ; Initialize multi-session variables
                clr.b   db_TOC+TOCS_LastTrack(db)
                move.l  db_TOC+TOCS_LeadOut(db),db_MSLeadOutTemp(db)
                clr.l   db_TOC+TOCS_LeadOut(db)

                jsr     LightOn                                             ; Turn on drive light
                clr.b   db_FlickerLight(db)

                jsr     MuteCD                                              ; Mute CD audio

                move.l  db_TOCNext(db),d0                                   ; If we are not trying to read session 0, try
                move.l  d0,d1                                               ;    playing 30 seconds into the lead-in area.
                beq     1$
                jsr     MSFBCDtoBIN
                jsr     MSFtoLSN
                sub.l   #30*75,d0
                move.l  d0,d1
                add.l   #30*75,d1
1$
                jsr     LSNtoMSF
                jsr     MSFBINtoBCD
                move.l  d0,db_Packet(db)                                    ; Set up play command
                move.l  d1,d0
                jsr     LSNtoMSF
                jsr     MSFBINtoBCD
                lsl.l   #8,d0
                move.l  d0,db_Packet+4(db)
                move.l  #0,db_Packet+8(db)
                move.l  #0,db_Packet+12(db)
                move.b  #CHCMD_SETPLAY,db_Packet(db)                        ; - Play TOC
                move.b  #%00000011,db_Packet+7(db)                          ; - Mute audio
                move.b  #%00000000,db_Packet+10(db)                         ; - Enable Q-Code packets
                move.w  #75,db_CurrentSpeed(db)                             ; - Remember our speed

                move.b  #1,db_ReadingTOC(db)                                ; We are now attempting to read the TOC

                jsr     DoPacket                                            ; Start the read
                bpl     2$

                tst.l   db_MultiSession(db)                                 ; If there is an error, figure out what to do
                bne     DoneReadingTOC
                bra     toc_error
2$
                move.l  db_TOCNext(db),d0                                   ; Make note of position of first track of last session
                jsr     MSFBCDtoBIN
                jsr     MSFtoLSNPOS
                add.l   #150,d0
                move.l  d0,db_RemapStart(db)

                clr.l   db_TOCNext(db)                                      ; Reinitialize next pointer

                move.w  db_Info+CDINFO_Status(db),d0                        ; Read the table of contents if not read yet
                btst    #CDSTSB_SPIN,d0
                bne     5$
3$
                move.b  #CHCMD_PLAY,db_Packet(db)                           ; Start the disk spinning
                jsr     DoPacket
                bpl     4$
                and.b   #$F8,d0                                             ; - CLV error?  Retry
                cmp.b   #CH_ERR_NODISK,d0
                beq     toc_error
                cmp.b   #CH_ERR_DISKUNREADABLE,d0
                beq     toc_error
                bra     3$
4$
                or.w    #CDSTSF_DISK|CDSTSF_SPIN,db_Info+CDINFO_Status(db)  ; Disk is spinning
5$
                move.l  #SIGF_CMDDONE|SIGF_TOCDONE|SIGF_ABORTDRIVE,d0       ; Wait for TOC to finish
                exec    Wait

                btst    #SIGB_TOCDONE,d0                                    ; Was the read of the TOC successful?
                beq     toc_error

                move.l  db_TOCNext(db),d0                                   ; Could there be another session on the disk?
                or.l    d0,db_MultiSession(db)
                tst.l   db_TOCNext(db)
                bne     NextSession
DoneReadingTOC:
                tst.l   db_MultiSession(db)                                 ; Seek back to beginning if multi-session disk
                beq     1$
                clr.l   db_Packet(db)
                clr.l   db_Packet+4(db)
                clr.l   db_Packet+8(db)
                clr.l   db_Packet+12(db)
                move.b  #CHCMD_SETPLAY,db_Packet(db)
                move.b  #%00000011,db_Packet+7(db)
                jsr     DoPacket

                move.b  db_MSLastTrackTemp(db),db_TOC+TOCS_LastTrack(db)    ; Restore destroyed TOC information
                move.l  db_MSLeadOutTemp(db),db_TOC+TOCS_LeadOut(db)
1$
                move.b  #CHCMD_PAUSE,db_Packet(db)                          ; Pause play
                jsr     DoPacket

                clr.b   db_ReadingTOC(db)                                   ; No longer reading TOC

                DISABLE a1
                move.w  db_Info+CDINFO_Status(db),d0                        ; Modify status
                move.b  db_TOC+TOCEntry_SIZE+TOCE_CtlAdr(db),d1             ; - Is this a data disk?
                and.b   #CTLADR_CTLMASK,d1
                cmp.b   #CTL_DATA,d1
                bne     2$
                bset    #CDSTSB_CDROM,d0
2$
                bset    #CDSTSB_TOC,d0                                      ; TOC is now valid
                move.w  d0,db_Info+CDINFO_Status(db)
                ENABLE  a1

                jsr     LightOff                                            ; Turn off drive light

                clr.w   d0                                                  ; ReadTOC successful
                rts
toc_error:
                jsr     LightOff                                            ; Turn off drive light

                clr.b   db_ReadingTOC(db)                                   ; No longer reading TOC

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
                move.l  db_ARG1(db),d0                                  ; Set up play command
                jsr     LSNtoMSFPOS
                jsr     MSFBINtoBCD
                move.l  d0,db_Packet(db)
                move.b  #CHCMD_SETPLAY,db_Packet(db)
                move.l  #$FFFFFF8F,db_Packet+4(db)                      ; - Pause and mute after seek (end = end of disk)
                move.l  #$00000400,d1                                   ; - Current speed, no Q-Codes.
                cmp.w   #150,db_CurrentSpeed(db)
                blo     1$
                bset    #30,d1
1$              move.l  d1,db_Packet+8(db)
                jsr     DoPacket

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

                jsr     MuteCD                                          ; MuteCD for now

                jsr     LightOn                                         ; Turn on drive light

                move.l  db_ARG1(db),d0                                  ; Set up play command (start & end)
                jsr     LSNtoMSFPOS
                jsr     MSFBINtoBCD
                move.l  d0,db_PlayStart(db)
                move.l  d0,d1
                move.l  db_ARG2(db),d0
                sub.l   #1,d0
                jsr     LSNtoMSFPOS
                jsr     MSFBINtoBCD
                move.l  d0,db_PlayStop(db)
                move.l  d0,d2

                move.w  db_Info+CDINFO_Status(db),d0                    ; Swap start/stop addresses if in reverse mode
                btst    #CDSTSB_SEARCH,d0
                beq     1$
                btst    #CDSTSB_DIRECTION,d0
                beq     1$
                exg     d1,d2
1$
                move.l  d2,db_Packet+3(db)
                move.l  d1,db_Packet(db)
                move.l  d1,db_LastQPos(db)
                clr.w   db_LastQState(db)
                move.b  #CHCMD_SETPLAY,db_Packet(db)

                move.l  #$00000004,d1                                   ; Set search mode and speed
                btst    #CDSTSB_SEARCH,d0
                beq     3$
                btst    #CDSTSB_DIRECTION,d0
                bne     2$
                bset    #26,d1
                bra     3$
2$              bset    #27,d1
3$              cmp.w   #150,db_Info+CDINFO_PlaySpeed(db)
                blo     4$
                bset    #22,d1
4$              move.l  d1,db_Packet+7(db)
                move.w  db_Info+CDINFO_PlaySpeed(db),db_CurrentSpeed(db)
                move.b  #0,db_Packet+11(db)

                jsr     DoPacket                                        ; Send SETPLAY command
                bmi     90$

                btst    #1,d0                                           ; Start the disk spinning if stopped
                beq     5$
                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_SPIN,d0
                bne     6$
5$              move.b  #CHCMD_PLAY,db_Packet(db)
                jsr     DoPacket
                bmi     90$
                or.w    #CDSTSF_SPIN,db_Info+CDINFO_Status(db)
6$
                move.w  db_Info+CDINFO_Status(db),d0                    ; If pause mode is set, pause immediately
                btst    #CDSTSB_PAUSED,d0
                beq     7$
                move.b  #CHCMD_PAUSE,db_Packet(db)
                jsr     DoPacket
7$
                move.w  db_Attenuation(db),d0                           ; Set desired attenuation
                jsr     Attenuate

                restore d2                                              ; Play started successfully
                clr.w   d0
                rts
90$
                jsr     LightOff                                        ; Turn off drive light

                restore d2                                              ; Play could not start, probably bad data
                move.w  #CDERR_BadDataType,d0
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

                move.b  #CHCMD_PAUSE,db_Packet(db)                      ; Pause the audio
                jsr     DoPacket
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

                move.b  #CHCMD_PLAY,db_Packet(db)                       ; Pause the audio
                jsr     DoPacket

                jsr     SetSubcodeInterrupt                             ; Restart frame interrupts (if necessary)
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
                move.w  db_Info+CDINFO_Status(db),d0                                    ; Are we currently playing audio?
                btst    #CDSTSB_PLAYING,d0
                beq     6$

                jsr     Q_CODE                                                          ; Get current pos if we are still playing
                bne     6$

                jsr     MuteCD                                                          ; Turn off audio for a bit

                move.l  db_PlayStop(db),d0                                              ; SETPLAY from current pos to end of play
                cmp.l   #CDMODE_FREV,db_ARG1(db)
                bne     1$
                move.l  db_PlayStart(db),d0
1$              move.l  d0,db_Packet+3(db)
                move.l  db_QCode+9(db),d0
                and.l   #$00FFFFFF,d0
                move.l  d0,db_Packet(db)
                move.l  d0,db_LastQPos(db)
                move.b  #CHCMD_SETPLAY,db_Packet(db)

                move.l  #$00000004,d1                                                   ; Set search mode and direction
                move.l  db_ARG1(db),d0
                beq     3$
                cmp.l   #CDMODE_FFWD,d0
                bne     2$
                bset    #26,d1
                bra     3$
2$              bset    #27,d1
3$
                cmp.w   #150,db_Info+CDINFO_PlaySpeed(db)                               ; Set play speed
                blo     4$
                bset    #22,d1
4$              move.l  d1,db_Packet+7(db)
                move.b  #0,db_Packet+11(db)

                jsr     DoPacket                                                        ; Send SETPLAY command

                move.w  db_Info+CDINFO_Status(db),d0                                    ; Pause audio if in paused mode
                btst    #CDSTSB_PAUSED,d0
                beq     5$
                move.b  #CHCMD_PAUSE,db_Packet(db)
                jsr     DoPacket
5$
                move.w  db_Attenuation(db),d0                                           ; Reset desired attenuation
                jsr     Attenuate
6$
                and.w   #-1-(CDSTSF_SEARCH|CDSTSF_DIRECTION),db_Info+CDINFO_Status(db)  ; Enable search mode?
                tst.l   db_ARG1(db)
                beq     7$
                or.w    #CDSTSF_SEARCH,db_Info+CDINFO_Status(db)

                cmp.l   #1,db_ARG1(db)                                                  ; Which direction do we search?
                beq     7$
                or.w    #CDSTSF_DIRECTION,db_Info+CDINFO_Status(db)
7$
                clr.w   d0                                                              ; SetPlayMode successful
                rts



************************************************************************
*                                                                      *
*   QCODE - Retrieve a Q-Code packet                                   *
*                                                                      *
************************************************************************

Q_CODE:
                move.w  db_Info+CDINFO_Status(db),d0                    ; Are we playing audio?
                btst    #CDSTSB_PLAYING,d0
                beq     99$

                move.w  db_Info+CDINFO_Status(db),d0                    ; Have we already reported a packet in pause mode?
                and.w   db_LastQState(db),d0
                btst    #CDSTSB_PAUSED,d0
                bne     1$                                              ; - Then report the same packet again

                move.b  #CHCMD_QCODE,db_Packet(db)                      ; Get me a Q-Code packet
                jsr     DoPacket

                save    d0                                              ; Reissue pause command if paused.
                move.w  db_Info+CDINFO_Status(db),d0
                and.w   #(CDSTSF_PLAYING|CDSTSF_PAUSED),d0
                cmp.w   #(CDSTSF_PLAYING|CDSTSF_PAUSED),d0
                bne     0$
                move.b  #CHCMD_PAUSE,db_Packet(db)
                jsr     DoPacket
0$              restore d0
                tst.b   d0
                bmi     4$
1$
                tst.b   db_QCode+2(db)                                  ; Was the command successful?
                bne     4$

                move.b  db_QCode+3(db),d0                               ; Is this a position packet?
                and.b   #CTLADR_ADRMASK,d0
                cmp.b   #ADR_POSITION,d0
                bne     4$

                move.l  db_QCode+9(db),d1                               ; Get Q-Code absolute position
                and.l   #$00FFFFFF,d1

                move.w  db_Info+CDINFO_Status(db),d0                    ; What direction are we playing?
                and.w   #(CDSTSF_SEARCH|CDSTSF_DIRECTION),d0
                cmp.w   #(CDSTSF_SEARCH|CDSTSF_DIRECTION),d0
                beq     2$

                cmp.l   db_LastQPos(db),d1                              ; Playing forward, are the numbers increasing?
                bhs     3$

                move.l  db_LastQPos(db),d0                              ; Did someone bump the drive?
                sub.l   d1,d0
                cmp.l   #75,d0
                blo     4$

                move.l  d1,db_LastQPos(db)                              ; This is now the last Q position and state
                move.w  db_Info+CDINFO_Status(db),db_LastQState(db)

                clr.b   db_AutoQ(db)                                    ; Turn off QCode interrupts (if they happened to be on)
                jsr     SetSubcodeInterrupt

                move.w  #CDERR_SeekError,d0                             ; Unexpected seek
                rts
2$
                cmp.l   db_LastQPos(db),d1                              ; Playing backwards, are the numbers decreasing?
                bhi     4$
3$
                move.l  d1,db_LastQPos(db)                              ; This is now the last Q position and state
                move.w  db_Info+CDINFO_Status(db),db_LastQState(db)

                clr.b   db_AutoQ(db)                                    ; Turn off QCode interrupts (if they happened to be on)
                jsr     SetSubcodeInterrupt

                clr.w   d0                                              ; Command successful
                rts
4$
                move.l  db_ClassDReq(db),a0
                cmp.b   #CDERR_NoDisk,IO_ERROR(a0)                      ; Was the disk ejected?
                beq     99$

                move.b  #1,db_AutoQ(db)                                 ; Make Q-Code get reissued at next subcode interrupt
                jsr     SetSubcodeInterrupt

                move.l  #SIGF_QCODE|SIGF_ABORTQCODE,d0                  ; Wait to reissue new packet or play to be complete
                or.l    #SIGF_PLAYDONE|SIGF_ABORTDRIVE,d0
                exec    Wait

                btst    #SIGB_PLAYDONE,d0                               ; If the play is complete, reissue playdone signal
                beq     5$
                save    d0
                SIGNAL  SIGF_PLAYDONE
                restore d0
5$
                btst    #SIGB_ABORTDRIVE,d0                             ; If the play was aborted, reissue the abort signal
                beq     6$
                save    d0
                SIGNAL  SIGF_ABORTDRIVE
                restore d0
6$
                btst    #SIGB_ABORTQCODE,d0                             ; Was this command aborted?
                bne     99$
                btst    #SIGB_ABORTDRIVE,d0
                bne     99$

                btst    #SIGB_PLAYDONE,d0                               ; If the play is not complete, reissue Q-Code packet
                beq     Q_CODE
99$
                clr.b   db_AutoQ(db)                                    ; Turn off auto QCode interrupts
                jsr     SetSubcodeInterrupt

                move.w  #CDERR_InvalidState,d0                          ; Audio is not playing
                rts




************************************************************************
*                                                                      *
*   ID_PACKET - Get drive ID packet                                    *
*                                                                      *
************************************************************************

ID_PACKET:
                jsr     LightOff                                        ; Turn off drive light

                clr.w   d0                                              ; Get ID packet
                move.b  #CHCMD_SENDID,db_Packet(db)
                jsr     SendPacket

                save    d2                                              ; Check for timeout
                clr.l   d2
1$              clr.l   d0
                move.l  #SIGF_CMDDONE,d1
                exec    SetSignal
                add.l   #1,d2
                cmp.l   #1000,d2
                bhs     4$
                tst.l   d0
                beq     1$
                SIGNAL  SIGF_CMDDONE                                    ; Echo the signal
                restore d2

                jsr     WaitPacket
                bmi     3$

                save    d0                                              ; Is drive door closed?
                and.b   #$03,d0
                move.b  d0,db_OpenState(db)
                beq     2$
                or.w    #CDSTSF_CLOSED,db_Info+CDINFO_Status(db)
2$              restore d0
3$
                rts
4$
                restore d2
                move.b  #1,db_NoHardware(db)
                rts




************************************************************************


************************************************************************
*                                                                      *
*   SendPacket - Transmit command packet to drive                      *
*                                                                      *
*   in:     D0.w = Ignore response?                                    *
*                                                                      *
************************************************************************

 FUNCTION SendPacket
                tst.b   db_NoHardware(db)                               ; Is drive present?
                beq     0$
                rts
0$
                save    d0                                              ; Save input

                DISABLE a0
    IFD DEBUG
                move.l  #'[',d0
                jsr     PutChar
    ENDC
                move.b  db_Packet(db),d0                                ; Reset response ignoring if new command is last ignored
                and.b   #$0F,d0
                move.b  db_IgnoreResponse(db),d1
                and.b   #$0F,d1
                cmp.b   d0,d1
                bne     2$
                clr.b   db_IgnoreResponse(db)
2$
                save    d2/d3                                           ; Get length of packet
                lea     db_Packet(db),a0
                move.b  (a0),d0
                and.w   #$000f,d0
                lea     PacketLen(pc),a1
                move.b  0(a1,d0.w),d2

                or.b    db_PacketIndex(db),d0                           ; Insert packet index and increment
                move.b  d0,(a0)
3$              add.b   #$10,db_PacketIndex(db)
                beq     3$

                move.l  db_CDCOMTXPage(db),a1                           ; Copy and checksum packet
                clr.w   d3
                move.b  db_ComTXInx(db),d3
                clr.w   d1
                clr.l   d0
                move.b  #-1,d0
4$              move.b  0(a0,d1.w),0(a1,d3.w)
    IFD DEBUG
                save    d0
                clr.w   d0
                move.b  0(a0,d1.w),d0
                jsr     PutHex
                restore d0
    ENDC
                sub.b   0(a0,d1.w),d0
                addq.b  #1,d1
                addq.b  #1,d3
                cmp.b   d1,d2
                bne     4$
    IFD DEBUG
                jsr     PutHex
    ENDC
                move.b  d0,0(a1,d3.w)                                   ; Store checksum
                addq.b  #1,d3
                move.b  d3,db_ComTXInx(db)
                move.b  d3,CDCOMTXCMP(hb)
                restore d2/d3
    IFD DEBUG
                move.l  #']',d0
                jsr     PutChar
                move.l  #10,d0
                jsr     PutChar
    ENDC
                restore d0                                              ; Ignore response to this command?
                tst.w   d0
                beq     5$
                move.b  db_Packet(db),db_IgnoreResponse(db)
5$
                ENABLE  a0

                or.l    #INTF_TXDMADONE,CDINT2ENABLE(hb)                ; Enable transmit DMA interrupt
                rts

PacketLen:      dc.b    1,2,1,1,12,2,1,1,4,1,0,0,0,0,0,0




************************************************************************
*                                                                      *
*   WaitPacket - Wait for response from drive                          *
*                                                                      *
*          out: status =  success                                      *
*               d0.b   =  error                                        *
*                                                                      *
************************************************************************

 FUNCTION WaitPacket
                tst.b   db_NoHardware(db)                               ; Is drive present?
                beq     0$
                move.b  #-1,d0
                rts
0$
                move.l  #SIGF_CMDDONE,d0                                ; Wait for a response
                exec    Wait

                move.l  db_CDCOMRXPage(db),a0                           ; Check for Philips 3.1 bad command error
                clr.w   d0
                move.b  db_PacketAddress(db),d0
                add.b   #1,d0
                move.b  0(a0,d0.w),d0
                and.b   #$F8,d0
                cmp.b   #CH_ERR_BADCOMMAND,d0                           ; - If "bad command" error, do patch
                beq     1$
                tst.b   db_ReadingTOC(db)                               ; - do further tests if not reading TOC
                bne     9$
                cmp.b   #CH_ERR_ABNORMALSEEK,d0
                beq     1$
                cmp.b   #CH_ERR_TRACKJUMP,d0
                bne     9$
1$
                move.b  db_Packet(db),d0                                ; Stop the drive and resend the last command
                and.b   #$0F,d0                                         ; - Do not recurse on STOP command
                cmp.b   #CHCMD_STOP,d0
                beq     9$
                move.l  db_Packet(db),-(sp)
                move.b  #CHCMD_STOP,db_Packet(db)
                move.b  #0,db_Packet+1(db)
                jsr     DoPacket
                move.l  (sp)+,db_Packet(db)
                jsr     DoPacket
9$
                move.l  db_CDCOMRXPage(db),a0                           ; Return status byte in d0
                clr.w   d0
                move.b  db_PacketAddress(db),d0
                add.b   #1,d0
                move.b  0(a0,d0.w),d0
                rts


************************************************************************
*                                                                      *
*   DoPacket - Transmit a packet to drive and wait for it to complete  *
*                                                                      *
************************************************************************

 FUNCTION DoPacket
                clr.w   d0
                jsr     SendPacket                                      ; Send a packet and wait for response
                jsr     WaitPacket
                rts












