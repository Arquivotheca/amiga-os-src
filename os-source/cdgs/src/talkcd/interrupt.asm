


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
        INCLUDE "/cd/cd.i"
        INCLUDE "/cd/cdgs_hw.i"
        INCLUDE "/cd/cdprivate.i"

        OPT     p=68020

SIGNALRCV       MACRO
                move.l  #1,d0
                move.b  db_InitSignal(db),d1
                asl.l   d1,d0
                move.l  db_InitTask(db),a1
                exec    Signal
                ENDM


************************************************************************
*                                                                      *
*   IntrProc - Main interrupt processing routine                       *
*                                                                      *
************************************************************************

 FUNCTION _IntrProc
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
                SIGNALRCV                                               ; Signal task that packet is here

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



