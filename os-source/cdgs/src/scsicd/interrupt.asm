

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


************************************************************************
*                                                                      *
*    CDSubCode - Subcode frame complete                                *
*                                                                      *
************************************************************************

 FUNCTION CDSubCode

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
6$
*************** Clear the interrupt ************************************

                rts



