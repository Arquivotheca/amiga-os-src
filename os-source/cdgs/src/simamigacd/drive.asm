
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
        INCLUDE "include:devices/timer.i"
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
        XREF    Attenuate
        XREF    StartTransfer
        XREF    SetSubcodeInterrupt
        XREF    ClearSubcodeInterrupt
        XREF    CDSubCode

        XREF    InitIntr

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
                dc.l    0
                dc.l    ID_PACKET



****************************************************************************
*                                                                          *
*   SPIN - Motor on                                                        *
*                                                                          *
****************************************************************************

SPIN:
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
                or.w    #CDSTSF_DISK|CDSTSF_SPIN,db_Info+CDINFO_Status(db)  ; Disk is spinning

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

                clr.w   d0                                                  ; ReadTOC successful
                rts


************************************************************************
*                                                                      *
*   SEEK - Seek to specified position                                  *
*                                                                      *
*       in:     ARG1 = Seek position                                   *
*                                                                      *
************************************************************************

SEEK:
                move.l  #MN_SIZE,d0                                     ; Tell drive task to start reading data
                move.l  #MEMF_PUBLIC|MEMF_CLEAR,d1
                exec    AllocMem
                tst.l   d0
                beq     2$
                move.l  d0,a1
                move.b  #NT_MESSAGE,LN_TYPE(a1)
                move.w  #3,MN_LENGTH(a1)
                move.l  db_ARG1(db),MN_REPLYPORT(a1)
                move.l  db_TaskMsgPort(db),a0
                exec    PutMsg

                move.l  #SIGF_CMDDONE,d0                                ; Wait for seek to finish
                exec    Wait
2$
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

                move.w  db_Info+CDINFO_Status(db),d0                    ; Make sure play is not in CD-ROM area
                btst    #CDSTSB_CDROM,d0
                beq     1$
                lea     db_TOC(db),a0
                add.l   #TOCEntry_SIZE*2,a0
                move.l  TOCE_Position(a0),d0
                jsr     MSFtoLSNPOS
                cmp.l   db_ARG1(db),d0
                bhi     90$
1$
                move.l  db_ARG1(db),d0                                  ; Set up play command (start & end)
                move.l  d0,db_PlayStart(db)
                move.l  d0,d1
                move.l  db_ARG2(db),d0
                sub.l   #1,d0
                move.l  d0,db_PlayStop(db)
                move.l  d0,d2

                move.l  d1,db_PlayPosition(db)                          ; If playing in reverse mode, start at stop time
                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_SEARCH,d0
                beq     2$
                btst    #CDSTSB_DIRECTION,d0
                beq     2$
                move.l  d2,db_PlayPosition(db)
2$
                move.l  #SS_MICROS,db_Speed(db)
                cmp.w   #150,db_Info+CDINFO_PlaySpeed(db)
                bne     3$
                move.l  #DS_MICROS,db_Speed(db)
3$
                or.w    #CDSTSF_SPIN,db_Info+CDINFO_Status(db)          ; Start disk spinning if stopped

                move.w  db_Attenuation(db),d0                           ; Set desired attenuation
                jsr     Attenuate

                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_PAUSED,d0
                bne     4$

                clr.w   db_AbortTimer(db)
                jsr     SendTimerReq
4$
                or.w    #CDSTSF_PLAYING,db_Info+CDINFO_Status(db)       ; Play working
                jsr     SetSubcodeInterrupt

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

                move.l  #1,d1                                           ; Search mode?
                move.w  db_Info+CDINFO_Status(db),d0
                btst    #CDSTSB_SEARCH,d0
                beq     1$

                move.l  #4,d1                                           ; Reverse direction?
                btst    #CDSTSB_DIRECTION,d0
                beq     1$

                sub.l   d1,db_PlayPosition(db)                          ; Decrement when going reverse
                move.l  db_PlayPosition(db),d0
                cmp.l   db_PlayStart(db),d0
                bhi     2$
                move.l  db_PlayStart(db),db_PlayPosition(db)
                SIGNAL  SIGF_PLAYDONE
                bra     2$
1$
                add.l   d1,db_PlayPosition(db)                          ; Increment when going forward
                move.l  db_PlayPosition(db),d0
                cmp.l   db_PlayStop(db),d0
                blo     2$
                move.l  db_PlayStop(db),db_PlayPosition(db)
                SIGNAL  SIGF_PLAYDONE
2$
                jsr     SendTimerReq                                    ; Send off another request
9$              clr.w   db_AbortTimer(db)

                jsr     CDSubCode                                       ; Call subcode interrupt

                restore db/ior
                clr.w   d0                                              ; Clear server chain
                rts


************************************************************************
*                                                                      *
*   SendTimerReq - Send a timer interrupt request                      *
*                                                                      *
************************************************************************

 FUNCTION SendTimerReq

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

                clr.w   db_AbortTimer(db)                               ; Restart frame interrupts (if necessary)
                jsr     SendTimerReq
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
                move.w  db_Info+CDINFO_Status(db),d0                                    ; Are we currently playing audio?
                btst    #CDSTSB_PLAYING,d0
                beq     6$

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
*   ID_PACKET - Get drive ID packet                                    *
*                                                                      *
************************************************************************

ID_PACKET:
                or.w    #CDSTSF_CLOSED,db_Info+CDINFO_Status(db)
                rts




************************************************************************

