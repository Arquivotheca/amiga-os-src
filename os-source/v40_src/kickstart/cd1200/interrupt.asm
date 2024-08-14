

        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/devices.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/interrupts.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/io.i"
        INCLUDE "exec/errors.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "hardware/intbits.i"
        INCLUDE "devices/trackdisk.i"
        INCLUDE "exec/ables.i"

        INCLUDE "defs.i"
        INCLUDE "cd.i"
        INCLUDE "cdprivate.i"
        INCLUDE "cdgs_hw.i"

        OPT     p=68020

************************************************************************
*                                                                      *
*    External References                                               *
*                                                                      *
************************************************************************

        XREF    EnableRX
        XREF    DisableRX

        XREF    MSFtoLSNPOS
        XREF    LSNtoMSFPOS

        XREF    MSFBINtoBCD
        XREF    MSFBCDtoBIN
        XREF    LSNtoMSF
        XREF    MSFtoLSN
        XREF    BCDtoBIN
        XREF    BINtoBCD

        XREF    Attenuate
        XREF    SetSubcodeInterrupt
        XREF    CauseInterruptList
        XREF    FlickerLight

        XREF    StopRead
        XREF    ClearPrefetch

        XREF    PutHex
        XREF    PutChar

        INT_ABLES



*
************************************************************************
*                                                                      *
*   XLIntrProc - XL node interrupt processor                           *
*                                                                      *
************************************************************************

 FUNCTION XLIntrProc
                save    a2                                              ; Call XL entry as an interrupt
                move.l  db_XferEntry(a1),a2
                move.l  CDXL_IntCode(a2),a0
                move.l  CDXL_IntData(a2),a1
                jsr     (a0)
                restore a2

                move.w  #1,d0                                           ; Clear server chain
                rts


*
************************************************************************
*                                                                      *
*   IntrProc - Main interrupt processing routine                       *
*                                                                      *
************************************************************************

 FUNCTION IntrProc
                move.l  CDINT2ENABLE+CDHARDWARE,d0                      ; Is this our interrupt?
                and.l   CDSTATUS+CDHARDWARE,d0

                bne     ServiceInt                                      ; Do something about it

                clr.l   d0                                              ; Wasn't our interrupt
                rts


ServiceInt:
                save    d2/db/hb                                        ; Initialize registers
                move.l  a1,db
                move.l  #CDHARDWARE,hb

                move.l  d0,d2                                           ; Information from drive available?
                btst    #INTB_RXDMADONE,d2
                beq     1$
                bsr     CDRXDMADone
1$
                btst    #INTB_TXDMADONE,d2                              ; Transmit of drive command complete?
                beq     2$
                bsr     CDTXDMADone
2$
                btst    #INTB_SUBCODE,d2                                ; Subcode interrupt?
                beq     3$
                bsr     CDSubCode
3$
                btst    #INTB_PBX,d2                                    ; CD-ROM interrupt?
                beq     4$
                bsr     CDROM
4$
                restore d2/db/hb                                        ; Clear server chain
                move.l  #1,d0
                rts


************************************************************************
*                                                                      *
*    CDRXDMADone                                                       *
*                                                                      *
*       Information from drive is available                            *
*                                                                      *
************************************************************************

CDRXDMADone:
                jsr     DisableRX

                move.l  db_CDCOMRXPage(db),a0                           ; Is this the first byte of a packet?
                tst.b   db_ReceivingCmd(db)
                bne     3$

                clr.w   d0                                              ; Get command byte
                move.b  db_ComRXInx(db),d0
                move.b  0(a0,d0.w),d0
                and.b   #$0f,d0

                lea     ResponseLen(pc),a1                              ; Prepare to receive rest of packet
                move.b  0(a1,d0.w),d0
                add.b   db_CDCOMRXCMP(db),d0
                subq.b  #1,d0
                move.b  d0,db_CDCOMRXCMP(db)
                move.b  #1,db_ReceivingCmd(db)

                move.b  d0,CDCOMRXCMP(hb)                               ; Clear the interrupt

                sub.b   CDCOMRXINX(hb),d0                               ; If the rest of this packet is here, parse again
                bmi     CDRXDMADone
                bne     2$
                move.l  CDSTATUS(hb),d0
                btst    #INTB_RXDMADONE,d0
                beq     CDRXDMADone
2$
                jsr     EnableRX
                rts
3$
                save    d2-d4                                           ; Get address and length of packet  
                clr.w   d2
                move.b  db_ComRXInx(db),d2
                move.b  0(a0,d2.w),d0
                and.w   #$0f,d0
                lea     ResponseLen(pc),a1
                move.b  0(a1,d0.w),d3

                clr.b   d0                                              ; Make sure checksum of packet is valid
                clr.b   d1
4$              and.w   #$FF,d2
                move.b  0(a0,d2.w),d4
                add.b   d4,d0
                add.b   #1,d2
                add.b   #1,d1
                cmp.b   d1,d3
                bne     4$
                restore d2-d4

                cmp.b   #$FF,d0                                         ; Checksum ok?
                bne     10$

                clr.w   d1                                              ; Were we instructed to ignore this packet?
                move.b  db_ComRXInx(db),d1
                move.b  0(a0,d1.w),d0     
                cmp.b   db_IgnoreResponse(db),d0
                beq     10$

                cmp.b   #CHCMD_QCODE,d0                                 ; Is this a TOC entry?
                bne     5$
                cmp.b   #1,db_ReadingTOC(db)
                beq     CDTOC
                bra     ReturnInt
5$
                and.b   #$0F,d0                                         ; Is this a response to a Q-Code command?
                cmp.b   #CHCMD_QCODE,d0
                beq     CDQCodeResponse

                addq.b  #1,d1                                           ; Is this a response to a play command?
                cmp.b   #CHCMD_SETPLAY,d0
                beq     6$
                cmp.b   #CHCMD_PLAY,d0
                bne     8$
6$
                move.w  db_Info+CDINFO_Status(db),d0                    ; Are we playing?
                btst    #CDSTSB_PLAYING,d0
                bne     7$

                move.b  0(a0,d1.w),d0                                   ; Is this a play-has-begun packet?
                bmi     CDCMDResponse
                and.b   #$78,d0
                cmp.b   #$08,d0
                bne     8$
                cmp.w   #CDCMD_PLAY,db_CMD(db)
                bne     8$

                or.w    #CDSTSF_PLAYING,db_Info+CDINFO_Status(db)       ; Play working (must set quickly incase datatype is wrong)
                jsr     SetSubcodeInterrupt
                bra     CDCMDResponse
7$
                move.w  db_Info+CDINFO_Status(db),d0                    ; Is this a play completion packet?
                btst    #CDSTSB_PLAYING,d0
                beq     8$
                move.b  0(a0,d1.w),d0
                move.b  d0,db_PlayStatus(db)
                bmi     CDPlayDone
                and.b   #$78,d0
                beq     CDPlayDone
8$
                move.b  0(a0,d1.w),d0                                   ; Is this a progress packet?  If so, ignore it.
                btst    #7,d0
                bne     9$
                and.b   #$70,d0
                bne     ReturnInt
9$
                subq.b  #1,d1                                           ; Is this a disk-change packet?
                move.b  0(a0,d1.w),d0
                and.b   #$0f,d0
                cmp.b   #CHCMD_DISKCHANGE,d0
                beq     CDDiskChange

                bra     CDCMDResponse                                   ; Response to drive command
10$
                bra     ReturnInt


ReturnInt:
                move.l  db_CDCOMRXPage(db),a0                           ; Increment pointer to beginning of next packet
                clr.w   d0
                move.b  db_ComRXInx(db),d0
                move.b  0(a0,d0.w),d0
                and.w   #$0f,d0
                lea     ResponseLen(pc),a1
                move.b  0(a1,d0.w),d1
                add.b   d1,db_ComRXInx(db)

                move.b  db_CDCOMRXCMP(db),d0                            ; Configure hardware to wait for a command byte again
                addq.b  #1,d0
                move.b  d0,db_CDCOMRXCMP(db)
                move.b  #0,db_ReceivingCmd(db)

                move.b  d0,CDCOMRXCMP(hb)                               ; Clear the interrupt

                sub.b   CDCOMRXINX(hb),d0                               ; If we have received more data, parse again
                bmi     CDRXDMADone
                bne     1$
                move.l  CDSTATUS(hb),d0
                btst    #INTB_RXDMADONE,d0
                beq     CDRXDMADone
1$
                jsr     EnableRX
                rts

ResponseLen:    dc.b    1,3,3,3,3,3,16,21,3,1,3,1,1,1,1,1



*
************************************************************************
*                                                                      *
*    CDTXDMADone - Drive command buffer is empty                       *
*                                                                      *
************************************************************************

CDTXDMADone:
                and.l   #-1-INTF_TXDMADONE,CDINT2ENABLE(hb)            ; Disable transmit DMA interrupt
                rts

*
************************************************************************
*                                                                      *
*    CDCMDResponse - Response to standard command                      *
*                                                                      *
************************************************************************

CDCMDResponse:
                move.b  db_ComRXInx(db),db_PacketAddress(db)            ; Remember address of packet

                SIGNAL  SIGF_CMDDONE                                    ; Command has completed
                bra     ReturnInt



************************************************************************
*                                                                      *
*    CDPlayDone - Play complete.                                       *
*                                                                      *
************************************************************************

CDPlayDone:
                cmp.w   #CDCMD_QCODE,db_CMD(db)                         ; If we are waiting for a Q-Code packet, wait no longer
                bne     1$
                SIGNAL  SIGF_QCODE
1$
                SIGNAL  SIGF_PLAYDONE                                   ; Signal task about play completion
                bra     ReturnInt



************************************************************************
*                                                                      *
*    CDQCodeResponse - Response to a Q-Code command.  Copy packet      *
*                                                                      *
*    in:    D1 = packet index                                          *
*                                                                      *
************************************************************************

CDQCodeResponse:
                save    d2
                lea     db_QCode(db),a0                                 ; Copy result to Q-Code packet
                move.l  db_CDCOMRXPage(db),a1                           ; Is this Q-Code packet valid?
                clr.w   d2
1$              move.b  d2,d0
                bsr     GetPackByte
                move.b  d0,(a0)+
                add.b   #1,d2
                cmp.b   #16,d2
                bne     1$
                restore d2

                bra     CDCMDResponse                                   ; Do standard response now



************************************************************************
*                                                                      *
*    CDDiskChange - Disk drawer state has changed                      *
*                                                                      *
************************************************************************

CDDiskChange
                move.l  db_CDCOMRXPage(db),a1                           ; Save the state of the drive door (zero = openned)
                clr.w   d0
                move.b  db_ComRXInx(db),d0
                add.b   #1,d0
                move.b  0(a1,d0.w),d0
                and.b   #$03,d0

                cmp.b   db_OpenState(db),d0
                beq     ReturnInt
                move.b  d0,db_OpenState(db)
                bne     3$                                              ; - Is door now openned?

                tst.w   db_Info+CDINFO_EjectReset(db)
                beq     1$
                exec    ColdReboot
1$
                tst.b   db_ReadingTOC(db)                               ; Abort disk command (if one is occurring)
                bne     2$
                move.l  db_ClassDReq(db),d0
                beq     3$
                move.l  d0,a1
                move.b  #CDERR_NoDisk,IO_ERROR(a1)
2$
                SIGNAL  SIGF_ABORTDRIVE                                 ; Abort them
3$
                SIGNAL  SIGF_DISKCHANGE                                 ; Disk drawer state has changed
                bra     ReturnInt




************************************************************************
*                                                                      *
*    CDTOC - This is a TOC entry that needs to be processed            *
*                                                                      *
*       in:  D1 = packet index                                         *
*                                                                      *
************************************************************************

CDTOC
                save    d2

                save    d1                                              ; Invert current drive light state
                jsr     FlickerLight
                restore d1

                move.w  db_Info+CDINFO_Status(db),d0                    ; Is the TOC already valid?
                btst    #CDSTSB_TOC,d0
                bne     99$

                move.l  db_CDCOMRXPage(db),a1                           ; Is this Q-Code packet valid?
                move.b  #1,d0
                bsr     GetPackByte
                tst.b   d0
                bmi     99$
                move.b  #2,d0
                bsr     GetPackByte
                tst.b   d0
                bne     99$

                move.b  #3,d0                                           ; Is this a position packet?
                bsr     GetPackByte
                and.b   #CTLADR_ADRMASK,d0
                cmp.b   #ADR_POSITION,d0
                beq     0$

                cmp.b   #ADR_HYBRID,d0                                  ; Is this a hybrid muti-session?
                bne     99$
                move.b  #5,d0
                bsr     GetPackByte
                cmp.b   #$B0,d0
                bne     99$

                move.b  #5,d0                                           ; Mark beginning of next lead-in area
                bsr     GetPackLong
                and.l   #$00FFFFFF,d0
                move.l  d0,db_TOCNext(db)
                bra     99$
0$
                move.b  #4,d0                                           ; TNO must be zero during lead-in track
                bsr     GetPackByte
                tst.b   d0
                bne     99$

                lea     db_TOC(db),a0                                   ; First track?
                move.b  #5,d0
                bsr     GetPackByte
                cmp.b   #$A0,d0
                bne     1$

                addq.b  #1,db_CDICount(db)                              ; If we have seen the first track packet an excessive
                cmp.b   #3*10,db_CDICount(db)                           ;    number of times, let's assume this is a CDI disk
                bhs     100$

                bra     5$
1$
                cmp.b   #$A1,d0                                         ; Last track?
                bne     2$
                move.b  #10,d0
                bsr     GetPackByte
                jsr     BCDtoBIN
                move.b  d0,TOCS_LastTrack(a0)
                bra     5$
2$
                cmp.b   #$A2,d0                                         ; Lead-out address?  Last track must be valid first
                bne     3$
                clr.l   d2
                move.b  TOCS_LastTrack(a0),d2
                beq     99$

                addq.w  #1,d2                                           ; Create a false entry just past table of contents
                mulu    #TOCEntry_SIZE,d2
                move.b  #0,TOCE_Track(a0,d2.w)
                move.b  #0,TOCE_CtlAdr(a0,d2.w)
                move.b  #9,d0
                bsr     GetPackLong
                and.l   #$00FFFFFF,d0
                jsr     MSFBCDtoBIN
                move.l  d0,TOCS_LeadOut(a0)
                move.l  d0,TOCE_Position(a0,d2.w)
                bra     5$
3$
                jsr     BCDtoBIN                                        ; Is the index valid?
                tst.b   d0
                beq     99$
                cmp.b   #100,d0
                bhs     99$

                clr.w   d2                                              ; Make a TOC entry
                move.b  d0,d2
                mulu    #TOCEntry_SIZE,d2
                move.b  d0,TOCE_Track(a0,d2.w)
                move.b  #3,d0
                bsr     GetPackByte
                move.b  d0,TOCE_CtlAdr(a0,d2.w)
                move.b  #9,d0
                bsr     GetPackLong
                and.l   #$00FFFFFF,d0
                jsr     MSFBCDtoBIN
                move.l  d0,TOCE_Position(a0,d2.w)
5$
                cmp.b   #1,TOCS_FirstTrack(a0)                          ; Is summary information valid?
                bne     99$
                tst.b   TOCS_LastTrack(a0)
                beq     99$
                tst.l   TOCS_LeadOut(a0)
                beq     99$

                move.b  #1,d0                                           ; Are all entries (including lead-out track) valid?
                move.w  #TOCEntry_SIZE,d2
6$              tst.b   TOCE_Track(a0,d2.w)
                beq     99$
                addq.b  #1,d0
                add.w   #TOCEntry_SIZE,d2
                cmp.b   TOCS_LastTrack(a0),d0
                bls     6$
98$
                move.b  #2,db_ReadingTOC(db)                            ; No longer reading TOC

                SIGNAL  SIGF_TOCDONE                                    ; TOC has been read successfully, stop playing
99$
                restore d2

                bra     ReturnInt
100$:
                move.b  #1,TOCSummary_SIZE+TOCE_Track(a0)                           ; Create first track of CDI disk
                move.b  #CTL_DATA|ADR_POSITION,TOCSummary_SIZE+TOCE_CtlAdr(a0)
                move.l  #$000200,TOCSummary_SIZE+TOCE_Position(a0)

                tst.b   (TOCSummary_SIZE*2)+TOCE_Track(a0)                          ; If there is no track 2, just make the
                bne     101$                                                        ;    beginning of track 2 = lead-out.
                move.b  #2,(TOCSummary_SIZE*2)+TOCE_Track(a0)
                move.b  #CTL_DATA|ADR_POSITION,(TOCSummary_SIZE*2)+TOCE_CtlAdr(a0)
                move.l  TOCS_LeadOut(a0),(TOCSummary_SIZE*2)+TOCE_Position(a0)
101$
                bra     98$



GetPackByte:
                add.b   d1,d0
                and.w   #$FF,d0
                move.b  0(a1,d0.w),d0
                rts


GetPackLong:
                save    d2/d3
                move.b  d0,d2
                bsr     GetPackByte
                move.b  d0,d3
                lsl.l   #8,d3
                add.b   #1,d2
                move.b  d2,d0
                bsr     GetPackByte
                move.b  d0,d3
                lsl.l   #8,d3
                add.b   #1,d2
                move.b  d2,d0
                bsr     GetPackByte
                move.b  d0,d3
                lsl.l   #8,d3
                add.b   #1,d2
                move.b  d2,d0
                bsr     GetPackByte
                move.b  d0,d3
                move.l  d3,d0
                restore d2/d3
                rts




************************************************************************
*                                                                      *
*    CDROM - CD-ROM data interrupt                                     *
*                                                                      *
************************************************************************

CDROM
                save    d2
                move.l  CDSTATUS(hb),d0                                 ; Has there been a DMA overrun problem?
                btst    #INTB_OVERFLOW,d0
                beq     1$
                bsr     CDOverflow
                bra     4$
1$
                move.w  CDPBX(hb),d0                                    ; Prefetch buffers full?
                clr.w   d1
                move.w  #16,d2
2$              lsr.w   #1,d0
                bcc     3$
                addq.b  #1,d1
3$              subq.w  #1,d2
                bne     2$
                cmp.b   #2,d1
                bhi     4$
                move.w  #1,d0                                           ; - Disable CD-ROM hardware and stop drive
                jsr     StopRead
4$
                SIGNAL  SIGF_PBX                                        ; Signal READ command that data is available

                move.w  #0,CDPBX(hb)                                    ; Clear the interrupt
                restore d2
                rts



************************************************************************
*                                                                      *
*    CDOverflow - CD-ROM buffer overflow interrupt                     *
*                                                                      *
************************************************************************

CDOverflow
                move.w  #1,d0                                           ; Disable CD-ROM hardware, stop drive, and invalidate
                jsr     StopRead
                jsr     ClearPrefetch
                rts



************************************************************************
*                                                                      *
*    CDSubCode - Subcode frame complete                                *
*                                                                      *
************************************************************************

CDSubCode

*************** Auto Q *************************************************

                tst.b   db_AutoQ(db)                                    ; Q-Code interrupt enabled?
                beq     1$

                SIGNAL  SIGF_QCODE                                      ; Notify Q-Code command to try again
1$
*************** Auto Frame *********************************************

                tst.b   db_AutoFrame(db)                                ; Frame interrupt enabled?
                beq     2$

                lea     db_FrameList(db),a0                             ; Cause frame interrupts
                bsr     CauseInterruptList
2$
*************** Auto Fade **********************************************

                tst.b   db_AutoFade(db)                                 ; Fade interrupt enabled?
                beq     6$

                move.w  db_Attenuation(db),d0                           ; Fade up or down
                move.w  db_TargetAttenuation(db),d1
                add.w   db_FadeStepSize(db),d0
                tst.w   db_FadeStepSize(db)
                bmi     3$

                cmp.w   d1,d0                                           ; Fade down until we reach target
                bmi     5$
                bra     4$
3$
                cmp.w   d1,d0                                           ; Fade up until we reach target
                bpl     5$
4$
                save    d1                                              ; We have reached our fade value, turn off fading
                clr.b   db_AutoFade(db)
                jsr     SetSubcodeInterrupt
                restore d0
5$
                move.w  d0,db_Attenuation(db)                           ; Set the attenuator
                bsr     Attenuate
6$
*************** Clear the interrupt ************************************

                move.b  #0,CDSUBINX(hb)                                 ; Clear the interrupt
                rts



