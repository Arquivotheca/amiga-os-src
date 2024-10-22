
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
        INCLUDE "exec/ables.i"
        INCLUDE "hardware/intbits.i"
        INCLUDE "devices/trackdisk.i"
        INCLUDE "dos/dos_lib.i"
        INCLUDE "dos/dos.i"

        INCLUDE "defs.i"
        INCLUDE "cd.i"
        INCLUDE "cdprivate.i"
        INCLUDE "cdgs_hw.i"

        OPT     p=68020

************************************************************************
***
***  External References
***
************************************************************************

        INT_ABLES

        XREF    DoCmd
        XREF    DevName
        XREF    SendPacket
        XREF    DoSCSI

        XREF    PutHex
        XREF    PutChar
        XREF    Print

        XDEF    SCSIDevName
        XDEF    SCSIUnit

*
************************************************************************
***
*** LSNtoMSFPOS
***
*** in:         D0 = (long)LSN
***
*** out:        D0 = (long)0x00MMSSFF
***
************************************************************************

 FUNCTION LSNtoMSFPOS

        save    d1

        move.l  d0,d1                                                   ; Add on start of first track
        move.l  db_TOC+TOCEntry_SIZE+TOCE_Position(db),d0
        jsr     MSFtoLSN
        add.l   d0,d1

        clr.l   d0                                                      ; Calculate Frame (Block)
        divu    #75,d1
        swap    d1
        move.b  d1,d0

        clr     d1                                                      ; Calculate Minute and Second
        swap    d1
        divu    #60,d1
        swap    d1
        lsl     #8,d1
        or.l    d1,d0

        restore d1
        rts


************************************************************************
***
*** MSFtoLSNPOS
***
*** in:         D0 = (long)0x00MMSSFF
***
*** out:        D0 = (long)LSN
***
************************************************************************

 FUNCTION MSFtoLSNPOS

        save    d1/d2

        swap    d0                                                      ; Calculate sector from minute
        move.w  d0,d1
        mulu    #60*75,d1

        swap    d0                                                      ; Add in sector from frame
        clr.l   d2
        move.b  d0,d2
        add.l   d2,d1

        lsr.w   #8,d0                                                   ; Add in sector from second
        mulu    #75,d0
        add.l   d1,d0

        move.l  d0,d1                                                   ; Subtract off start of first track
        move.l  db_TOC+TOCEntry_SIZE+TOCE_Position(db),d0
        jsr     MSFtoLSN
        sub.l   d0,d1
        move.l  d1,d0

        restore d1/d2
        rts


*
************************************************************************
***
*** LSNtoMSF
***
*** in:         D0 = (long)LSN
***
*** out:        D0 = (long)0x00MMSSFF
***
************************************************************************

 FUNCTION LSNtoMSF

        save    d1

        move.l  d0,d1                                                   ; Calculate Frame (Block)
        clr.l   d0
        divu    #75,d1
        swap    d1
        move.b  d1,d0

        clr.w   d1                                                      ; Calculate Minute and Second
        swap    d1
        divu    #60,d1
        swap    d1
        lsl     #8,d1
        or.l    d1,d0

        restore d1
        rts


************************************************************************
***
*** MSFtoLSN
***
*** in:         D0 = (long)0x00MMSSFF
***
*** out:        D0 = (long)LSN
***
************************************************************************

 FUNCTION MSFtoLSN

        save    d1/d2

        swap    d0                                                      ; Calculate sector from minute
        move.w  d0,d1
        mulu    #60*75,d1

        swap    d0                                                      ; Add in sector from frame
        clr.l   d2
        move.b  d0,d2
        add.l   d2,d1

        lsr.w   #8,d0                                                   ; Add in sector from second
        mulu    #75,d0
        add.l   d1,d0

        restore d1/d2
        rts


*
************************************************************************
***
*** BCDtoBIN
***
*** in:
***
***     D0 = (byte)BCD
***
*** out:
***
***     D0 = (byte)BIN
***
***
************************************************************************

 FUNCTION BCDtoBIN

        save    d1

        move.w  d0,d1                                                   ; Isolate one's digit
        and.w   #$000f,d1

        lsr.w   #4,d0                                                   ; Isolate ten's digit
        and.w   #$000f,d0

        mulu    #10,d0                                                  ; Multiply ten's digit

        add.w   d1,d0                                                   ; Add one's digit

        restore d1
        rts


************************************************************************
***
*** BINtoBCD
***
*** in:
***
***     D0 = (byte)BIN
***
*** out:
***
***     D0 = (byte)BCD
***
***
************************************************************************

 FUNCTION BINtoBCD

        save    a0                                                      ; Save all effected registers

        and.w   #$00ff,d0                                               ; Convert to BCD
        lea     BCDTable(pc),a0
        move.b  0(a0,d0.w),d0

        restore a0                                                      ; Restore registers
        rts


BCDTable:
        dc.b    $00,$01,$02,$03,$04,$05,$06,$07,$08,$09
        dc.b    $10,$11,$12,$13,$14,$15,$16,$17,$18,$19
        dc.b    $20,$21,$22,$23,$24,$25,$26,$27,$28,$29
        dc.b    $30,$31,$32,$33,$34,$35,$36,$37,$38,$39
        dc.b    $40,$41,$42,$43,$44,$45,$46,$47,$48,$49
        dc.b    $50,$51,$52,$53,$54,$55,$56,$57,$58,$59
        dc.b    $60,$61,$62,$63,$64,$65,$66,$67,$68,$69
        dc.b    $70,$71,$72,$73,$74,$75,$76,$77,$78,$79
        dc.b    $80,$81,$82,$83,$84,$85,$86,$87,$88,$89
        dc.b    $90,$91,$92,$93,$94,$95,$96,$97,$98,$99


************************************************************************
***
*** MSFBINtoBCD
***
*** in:         D0 = (long)0x00MMSSFF (bin)
***
*** out:        D0 = (long)0x00MMSSFF (bcd)
***
************************************************************************

 FUNCTION MSFBINtoBCD

                save    d1/d2
                move.l  d0,d1
                swap    d0
                jsr     BINtoBCD
                move.w  d0,d2
                swap    d2
                move.w  d1,d0
                jsr     BINtoBCD
                move.w  d0,d2
                lsr.l   #8,d1
                move.b  d1,d0
                jsr     BINtoBCD
                lsl.w   #8,d0
                or.w    d0,d2
                move.l  d2,d0
                restore d1/d2
                rts




************************************************************************
***
*** MSFBCDtoBIN
***
*** in:         D0 = (long)0x00MMSSFF (bcd)
***
*** out:        D0 = (long)0x00MMSSFF (bin)
***
************************************************************************

 FUNCTION MSFBCDtoBIN

                save    d1/d2
                move.l  d0,d1
                swap    d0
                jsr     BCDtoBIN
                move.w  d0,d2
                swap    d2
                move.w  d1,d0
                jsr     BCDtoBIN
                move.w  d0,d2
                lsr.l   #8,d1
                move.b  d1,d0
                jsr     BCDtoBIN
                lsl.w   #8,d0
                or.w    d0,d2
                move.l  d2,d0
                restore d1/d2
                rts




************************************************************************
***
*** CheckSeekRange
***
*** in:         D1.l = position
***
*** out:        D0.w = error
***
************************************************************************

 FUNCTION CheckSeekRange
                move.l  db_TOC+TOCS_LeadOut(db),d0                      ; Compare seek position with lead-out track of TOC
                jsr     MSFtoLSNPOS
                cmp.l   d0,d1
                bhs     1$

                clr.w   d0                                              ; Seek within range
                rts
1$
                move.w  #1,d0                                           ; Seek out of range
                rts




************************************************************************
***
*** CheckPlayRange
***
*** in:         D1.l = start position
***             D2.l = stop position
***
*** out:        D0.w = error
***
************************************************************************

 FUNCTION CheckPlayRange

                jsr     CheckSeekRange                                  ; If seek range is bad, play range is bad
                bne     2$

                save    d1                                              ; Check end position of play
                move.l  d2,d1
                beq     1$
                sub.l   #1,d1
1$              jsr     CheckSeekRange
                restore d1
                tst.w   d0
2$
                rts





************************************************************************
***
***  InitIntr
***
***     in:     D0 = Interrupt bit number
***             D1 = Priority
***             DB = Data
***             A0 = Code
***             A1 = Node
***
************************************************************************

 FUNCTION InitIntr

                move.b  #NT_INTERRUPT,LN_TYPE(a1)                       ; Set up interrupt node
                move.b  d1,LN_PRI(a1)
                move.l  db,IS_DATA(a1)
                move.l  a0,IS_CODE(a1)  
                lea     DevName(pc),a0
                move.l  a0,LN_NAME(a1)

                exec    AddIntServer                                    ; Add interrupt to server chain
                rts


************************************************************************
***
***  InitPort
***
***     in:     A0 = Message port
***
************************************************************************

 FUNCTION InitPort

                clr.b   MP_FLAGS(a0)                                    ; Initialize message port
                move.b  d0,MP_SIGBIT(a0)
                lea     db_Task(db),a1
                move.l  a1,MP_SIGTASK(a0)
                lea     MP_MSGLIST(a0),a1
                NEWLIST a1
                rts




*
************************************************************************
***     
***  Attenuate(d0)
***
***     Set the DAC attenuator.
***
***             D0 == new attenuation value (0-7FFF)
***
************************************************************************

 FUNCTION Attenuate

                move.w  db_Attenuation(db),d0
                move.w  d0,db_LastAttenuation(db)
                lsr.w   #7,d0
                jsr     SetDAC
                rts


************************************************************************
*                                                                      *
*   MuteCD - Mute CD audio                                             *
*                                                                      *
************************************************************************

 FUNCTION MuteCD

                clr.b   d0                                              ; Mute CD
                jsr     SetDAC
                rts



************************************************************************
*                                                                      *
*   UnMuteCD - Mute CD audio                                           *
*                                                                      *
************************************************************************

 FUNCTION UnMuteCD

                jsr     Attenuate                                       ; UnMute CD
                rts




************************************************************************
***     
***  SetDAC
***
***     Set the DAC attenuator.
***
***             D0 == new attenuation value (0-FF)
***
************************************************************************

 FUNCTION SetDAC

                clr.l   db_ModeSelect(db)                               ; Set up mode select data
                move.l  #$0e0e0400,db_ModeSelect+4(db)
                clr.l   db_ModeSelect+8(db)
                move.l  #$01000200,db_ModeSelect+12(db)
                move.b  d0,db_ModeSelect+13(db)
                move.b  d0,db_ModeSelect+15(db)

                lea     db_command(db),a0                               ; Set up mode select command and send
                move.b  #S_MODE_SELECT,(a0)
                move.b  #$10,1(a0)
                clr.w   2(a0)
                move.l  #20,d1
                move.b  d1,4(a0)
                clr.b   5(a0)
                move.l  #6,d0
                bset    #31,d0
                lea     db_ModeSelect(db),a1
                jsr     DoSCSI
                rts



************************************************************************
***     
***  SetSectorSize
***
***     Configure the drive with the requested sector size
***
************************************************************************

 FUNCTION SetSectorSize

                move.l  #8,db_ModeSelect(db)                            ; Set up mode select data
                move.w  #2048,d0
                move.b  #1,db_ModeSelect+4(db)
                move.w  db_Info+CDINFO_SectorSize(db),d1
                cmp.w   d0,d1
                beq     1$
                move.w  #2336,d0
                move.b  #2,db_ModeSelect+4(db)
1$              move.w  d0,db_ModeSelect+10(db)

                clr.b   db_ModeSelect+4(db)

                clr.b   db_ModeSelect+5(db)
                clr.l   db_ModeSelect+6(db)

                lea     db_command(db),a0                               ; Set up mode select command and send
                move.b  #S_MODE_SELECT,(a0)
                move.b  #$10,1(a0)
                clr.w   2(a0)
                move.l  #12,d1
                move.b  d1,4(a0)
                clr.b   5(a0)
                move.l  #6,d0
                bset    #31,d0
                lea     db_ModeSelect(db),a1
                jsr     DoSCSI
                rts



*
************************************************************************
***
***  CauseInterruptList - Cause all interrupts contained in list
***
***     in:     A0 = Interrupt List
***
************************************************************************

 FUNCTION CauseInterruptList

                save    a2                                              ; Traverse interrupt list
                move.l  a0,a2
1$              move.l  (a2),a2
                tst.l   (a2)
                beq.s   2$
                move.l  IO_DATA(a2),a1                                  ; Call interrupt if one is defined
                exec    Cause
                bra.s   1$
2$
                restore a2                                              ; All done
                rts



************************************************************************
***
***  SetSubcodeInterrupt
***
***     Turn subcode interrupt on or off depending on flags.
***     This function will only succeed if playing audio.
***
************************************************************************

 FUNCTION SetSubcodeInterrupt

                move.w  db_Info+CDINFO_Status(db),d0                    ; If we are playing, set subcode interrupt
                btst    #CDSTSB_PLAYING,d0
                beq     99$

                move.b  db_AutoQ(db),d1                                 ; If we don't need subcode interrupts, turn them off

                btst    #CDSTSB_PAUSED,d0                               ; Don't fade or generate frame interrupts if paused
                bne     1$
                or.b    db_AutoFrame(db),d1
                or.b    db_AutoFade(db),d1
1$
                tst.b   d1                                              ; Generate subcode interrupts?
                bne     2$
                rts
2$
99$             rts
                


************************************************************************
***
***  ClearSubcodeInterrupt
***
***     Turn all subcode interrupt off
***
************************************************************************

 FUNCTION ClearSubcodeInterrupt

                rts
                


************************************************************************
***
***  Disable
***
************************************************************************

 FUNCTION Disable

                save    a0                                              ; Disable interrupts
                DISABLE a0
                restore a0
                rts

************************************************************************
***
***  Enable
***
************************************************************************

 FUNCTION Enable

                save    a0                                              ; Enable interrupts
                ENABLE  a0
                restore a0
                rts


************************************************************************
***
***  DisableRX
***
************************************************************************

 FUNCTION DisableRX

                move.b  #1,db_Disabled(db)
                rts

************************************************************************
***
***  EnableRX
***
************************************************************************

 FUNCTION EnableRX

                clr.b   db_Disabled(db)
                rts





************************************************************************
*                                                                      *
*   GetConfig - get configuration information from cd.device.config    *
*                                                                      *
************************************************************************

BUFFSIZE    equ     $1000

 FUNCTION GetConfig

                save    d2-d4/a2-a3/a6

                move.l  #BUFFSIZE,d0                                    ; Allocate file buffer
                move.l  #MEMF_PUBLIC,d1
                exec    AllocMem
                tst.l   d0
                beq     99$
                move.l  d0,a2

                lea     DOSName(pc),a1                                  ; Open dos.library
                clr.l   d0
                exec    OpenLibrary
                tst.l   d0
                beq     98$
                move.l  d0,a6

                lea     FileName(pc),a0                                 ; Open cd.device.config for read
                move.l  a0,d1
                move.l  #MODE_OLDFILE,d2
                jsr     _LVOOpen(a6)
                tst.l   d0
                beq     97$
                move.l  d0,a3

                move.l  d0,d1                                           ; Read a resonable amount into buffer
                move.l  a2,d2
                move.l  #BUFFSIZE,d3
                jsr     _LVORead(a6)
                tst.l   d0
                beq     96$
                move.l  d0,d3

                clr.l   d2                                              ; Find beginning of device name
1$              move.b  0(a2,d2.l),d0
                cmp.b   #10,d0              ; - Line Feed
                beq     2$
                cmp.b   #13,d0              ; - Return
                beq     2$
                cmp.b   #$20,d0             ; - Space
                beq     2$
                cmp.b   #9,d0               ; - Tab
                beq     2$
                cmp.b   #0,d0               ; - NULL
                beq     2$
                bra     3$
2$              add.l   #1,d2
                cmp.l   d3,d2
                bhs     96$
                bra     1$
3$
                lea     SCSIDevName(pc),a1                              ; Copy the device name
                clr.l   d1
4$              move.b  0(a2,d2.l),d0
                cmp.b   #10,d0              ; - Line Feed
                beq     6$
                cmp.b   #13,d0              ; - Return
                beq     6$
                cmp.b   #$20,d0             ; - Space
                beq     6$
                cmp.b   #9,d0               ; - Tab
                beq     6$
                cmp.b   #0,d0               ; - NULL
                beq     6$
                add.l   #1,d2               ; - Unexpected end of file?
                cmp.l   d3,d2
                bhs     5$
                move.b  d0,0(a1,d1.l)       ; - Copy the byte
                add.l   #1,d1
                cmp.l   #38,d1              ; - Name too long?
                bhs     6$
                bra     4$
5$              move.b  #0,0(a1,d1.l)       ; - End name now and exit
                bra     96$
6$              move.b  #0,0(a1,d1.l)       ; - End scsi.device name

7$              move.b  0(a2,d2.l),d0                                   ; Find next line
                cmp.b   #10,d0              ; - Line Feed
                beq     10$
                cmp.b   #13,d0              ; - Return
                beq     10$
                add.l   #1,d2
                cmp.l   d3,d2
                bhs     96$
                bra     7$

10$             move.b  0(a2,d2.l),d0                                   ; Find beginning of unit number
                cmp.b   #10,d0              ; - Line Feed
                beq     11$
                cmp.b   #13,d0              ; - Return
                beq     11$
                cmp.b   #$20,d0             ; - Space
                beq     11$
                cmp.b   #9,d0               ; - Tab
                beq     11$
                cmp.b   #0,d0               ; - NULL
                beq     11$
                bra     12$
11$             add.l   #1,d2
                cmp.l   d3,d2
                bhs     96$
                bra     10$
12$
                clr.l   d4                                              ; Compute unit number
                clr.l   d0
20$             move.b  0(a2,d2.l),d0
                cmp.b   #'0',d0
                blo     21$
                cmp.b   #'9',d0
                bhi     21$
                sub.l   #'0',d0
                mulu    #10,d4
                add.l   d0,d4
                add.l   #1,d2
                cmp.l   d3,d2
                bhs     96$
                bra     20$
21$             move.l  d4,SCSIUnit(pc)
96$
                move.l  a3,d1                                           ; Close file
                jsr     _LVOClose(a6)
97$
                move.l  a6,a1                                           ; Close dos.library
                exec    CloseLibrary
98$
                move.l  a2,a1                                           ; Free file buffer
                move.l  #BUFFSIZE,d0
                exec    FreeMem
99$
                restore d2-d4/a2-a3/a6

                jsr     CheckConfig                                     ; Test to see if device and unit exist
                rts


DOSName:        dc.b    'dos.library',0
                ds.w    0

FileName:       dc.b    'DEVS:cd.device.config',0
                ds.w    0

SCSIDevName:    dc.b    'scsi.device',0
                ds.b    27
                dc.b    0
                ds.w    0

SCSIUnit:       dc.l    0


*=======================================================================
*=                                                                     =
*=   CheckConfig - get configuration information from cd.device.config =
*=                                                                     =
*=======================================================================

CheckConfig:
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
                save    d0
                tst.b   d0
                bne     1$

                move.l  db_SCSIIOR(db),a1                               ; Close scsi.device
                exec    CloseDevice
1$
                move.l  db_SCSIIOR(db),a0                               ; Delete IORequest
                exec    DeleteIORequest

                move.l  db_SCSIPort(db),a0                              ; Delete message port
                exec    DeleteMsgPort

                restore d0
                tst.b   d0
                rts



**********************************************************************************************************************
