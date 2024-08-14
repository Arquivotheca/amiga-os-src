 

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

        XREF    GetNextSectorBuffer
        XREF    FreeSectorBuffer
        XREF    Attenuate

        XDEF    TransferSample

        XREF    PutHex
        XREF    PutChar

        INT_ABLES

************************************************************************
*                                                                      *
*   TransferSample                                                     *
*                                                                      *
*       in:     SARG1 = Start Position                                 *
*               SARG2 = Stop Position                                  *
*               SARG3 = Address                                        *
*               SARG4 = XLDMACount                                     *
*               IOR   = IORequest                                      *
*                                                                      *
************************************************************************

TransferSample
                save    d2-d7/a2-a3                                     ; Save all used registers
                move.l  db_CDROMPage(db),a3                             ; A3 = CDROMPage

                tst.l   db_SARG4(db)                                    ; Any more data to transfer?
                beq     Exit

                cmp.l   #2,db_SampleDump(db)                            ; Dump first frame
                beq     ReadData
                add.l   #1,db_SampleDump(db)
                jsr     GetNextSectorBuffer
                cmp.l   #-100,d0
                beq     ReadData
                jsr     FreeSectorBuffer

                cmp.l   #1,db_SampleDump(db)                            ; Set desired attenuation now.
                beq     ReadData
                jsr     Attenuate
ReadData
                jsr     GetNextSectorBuffer                             ; Get data if available
                move.l  d0,d3
                cmp.l   #-100,d0
                bne     TransferData
Exit
                restore d2-d7/a2-a3
                rts

*************** Transfer 'till end of XL node **************************

TransferData:
                jsr     TransferFromSector                              ; Transfer all or part of the sector

                bne     2$                                              ; If all of the sector was transferred...
                move.l  d3,d0                                           ; ... free the buffer and start on next sector
                jsr     FreeSectorBuffer
2$
                tst.l   db_SARG4(db)                                    ; End of transfer?
                beq     TransferEntryComplete
                bra     ReadData

*
*************** Transfer of CDXL entry is complete *********************

TransferEntryComplete:
                move.l  db_XferEntry(db),a0                             ; Report actual amount transferred
                move.l  CDXL_Length(a0),d0
                move.l  d0,CDXL_Actual(a0)
                add.l   d0,IO_ACTUAL(ior)

                tst.l   CDXL_IntCode(a0)                                ; If an XL interrupt is defined, call it
                beq     2$
                lea     db_XLIntr(db),a1
                exec    Cause
2$
                move.l  db_XferEntry(db),a0                             ; Transfer complete?
                move.l  MLN_SUCC(a0),a0                                 ; - End of list?
                tst.l   MLN_SUCC(a0)
                beq     TransferComplete
                move.l  CDXL_Length(a0),d0                              ; - Node length 0?
                beq     TransferComplete

                move.l  a0,db_XferEntry(db)                             ; New node
                clr.l   CDXL_Actual(a0)

                move.l  d0,db_SARG4(db)                                 ; Set new transfer address and length
                move.l  CDXL_Buffer(a0),db_SARG3(db)

                bra     ReadData                                        ; Resume transferring data


*************** Transfer is complete ***********************************

TransferComplete:
                and.l   #-1-(INTF_PBX|INTF_OVERFLOW),CDINT2ENABLE(hb)   ; Turn off audio decode hardware
                and.l   #-1-CF_CDROM,CDCONFIG(hb)

                restore d2-d7/a2-a3                                     ; Report read status
                clr.w   d0
                rts



*
*=======================================================================
*=                                                                     =
*=  TransferFromSector - Transfer as much data from sector as possible =
*=                                                                     =
*=======================================================================

TransferFromSector:
                move.l  #2352,d1                                        ; Audio data begins where header would be
                lea     ROM_HEADER(a3,d3.l),a1

*
*************** Mode/Form determined.  A1=Address, D1=Length ***********

BeginTransfer:
                sub.l   db_SectorIndex(db),d1                           ; Subtract amount of data already transferred D1 = remain
                add.l   db_SectorIndex(db),a1                           ; - Add in current sector index               A1 = source
                move.l  db_SARG3(db),a0                                 ; - Destination address                       A0 = dest

                clr.l   d2                                              ; Don't transfer more data than requested
                move.l  d1,d0
                cmp.l   db_SARG4(db),d0
                bls     1$
                move.l  db_SARG4(db),d0
                move.l  db_SectorIndex(db),d2                           ; - D0 = Transfer size
                add.l   d0,d2                                           ; - D2 = next sector index
1$
                move.l  d2,db_SectorIndex(db)                           ; Next time, index will equal this
                add.l   d0,db_SARG3(db)                                 ; - Next time, start at this address
                sub.l   d0,db_SARG4(db)                                 ; - Less data to transfer next time

                cmp.l   #44,d0                                          ; Is there enough data to do a big copy?
                blo     3$

                movem.l d2-d7/a2-a6,-(sp)                               ; Copy data in 44 byte chunks
2$              movem.l (a1)+,d2-d7/a2-a6
                movem.l d2-d7/a2-a6,(a0)
                add.l   #44,a0
                sub.l   #44,d0
                cmp.l   #44,d0
                bhs     2$
                movem.l (sp)+,d2-d7/a2-a6
3$
                tst.w   d0                                              ; Any data to transfer?
                beq     5$

4$              move.w  (a1)+,(a0)+                                     ; Move words at a time
                subq.w  #2,d0
                bne     4$
5$
                tst.l   db_SectorIndex(db)                              ; Are we done with this sector?
                rts



