
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
        XREF    DoPacket

        XREF    PutHex
        XREF    PutChar

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


************************************************************************
*                                                                      *
*   AllocAligned                                                       *
*                                                                      *
*       in:     D0 = Size of block                                     *
*               D1 = Memory type                                       *
*                                                                      *
*      out:     Memory block that is D0 Aligned                        *
*                                                                      *
************************************************************************

 FUNCTION AllocAligned

                save    d2                                              ; Make copy of size
                move.l  d0,d2

                save    d0,d1                                           ; Don't allow task switching
                exec    Forbid
                restore d0,d1

                lsl.l   #1,d0                                           ; Allocate twice as much memory as we need
                exec    AllocMem
                tst.l   d0
                beq     1$

                save    d0                                              ; Free the memory (don't worry, noone can grab it)
                move.l  d0,a1
                move.l  d2,d0
                lsl.l   #1,d0
                exec    FreeMem
                restore d0

                move.l  d0,d1                                           ; Allocate memory at aligned address
                move.l  d2,d0
                add.l   d2,d1
                sub.l   #1,d2
                not.l   d2
                and.l   d2,d1
                move.l  d1,a1
                exec    AllocAbs
1$
                save    d0                                              ; Permit task switching again
                exec    Permit
                restore d0

                restore d2                                              ; Return the result
                tst.l   d0
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

                jsr     Disable

                cmp.w   #$0800,db_Attenuation(db)                       ; Mute or UnMute?
                bhs     1$
                jsr     MuteCD
                bra     99$
1$              jsr     UnMuteCD
99$
                jsr     Enable                                          ; All done
                rts



************************************************************************
*                                                                      *
*   MuteCD - Mute CD audio                                             *
*                                                                      *
************************************************************************

 FUNCTION MuteCD

                clr.w   d0                                              ; Mute the audio
                and.w   #$FF7D,db_DriveBits(db)
                move.w  db_DriveBits(db),db_Packet(db)
                jsr     SendPacket
                rts


************************************************************************
*                                                                      *
*   UnMuteCD - Mute CD audio                                           *
*                                                                      *
************************************************************************

 FUNCTION UnMuteCD

                clr.w   d0                                              ; UnMute the audio
                and.w   #$FF7F,db_DriveBits(db)
                or.w    #$0002,db_DriveBits(db)
                move.w  db_DriveBits(db),db_Packet(db)
                jsr     SendPacket
                rts


************************************************************************
*                                                                      *
*   LightOn - Turn on drive light                                      *
*                                                                      *
************************************************************************

 FUNCTION LightOn

                or.w    #$0081,db_DriveBits(db)
                move.w  db_DriveBits(db),db_Packet(db)                  ; Turn on drive light
                jsr     DoPacket
                rts


************************************************************************
*                                                                      *
*   LightOff - Turn off drive light                                    *
*                                                                      *
************************************************************************

 FUNCTION LightOff

                clr.w   d0                                              ; Turn off drive light
                and.w   #$FF7E,db_DriveBits(db)
                move.w  db_DriveBits(db),db_Packet(db)
                jsr     SendPacket
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
                and.l   #-1-INTF_SUBCODE,CDINT2ENABLE(hb)
                and.l   #-1-CF_SUBCODE,CDCONFIG(hb)
                rts
2$
                or.l    #CF_SUBCODE,CDCONFIG(hb)                        ; ...otherwise, turn them on
                or.l    #INTF_SUBCODE,CDINT2ENABLE(hb)
99$             rts
                


************************************************************************
***
***  ClearSubcodeInterrupt
***
***     Turn all subcode interrupt off
***
************************************************************************

 FUNCTION ClearSubcodeInterrupt

                and.l   #-1-INTF_SUBCODE,CDINT2ENABLE(hb)
                and.l   #-1-CF_SUBCODE,CDCONFIG(hb)
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

                and.l   #-1-INTF_RXDMADONE,CDINT2ENABLE(hb)             ; Disable command receive interrupts
                move.b  #1,db_Disabled(db)
                rts

************************************************************************
***
***  EnableRX
***
************************************************************************

 FUNCTION EnableRX

                clr.b   db_Disabled(db)
                or.l    #INTF_RXDMADONE,CDINT2ENABLE(hb)                ; Enable command receive interrupts
                rts



************************************************************************
*                                                                      *
*   FlickerLight - Invert the state of the drive light                 *
*                                                                      *
************************************************************************

 FUNCTION FlickerLight

                move.w  #$0501,d0                                       ; Invert state of drive light
                tst.b   db_FlickerLight(db)
                beq     1$
                move.w  #$0500,d0
1$              not.b   db_FlickerLight(db)
                move.w  d0,db_Packet(db)
                clr.w   d0
                jmp     SendPacket


**********************************************************************************************************************
