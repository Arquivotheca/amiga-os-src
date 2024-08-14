 

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

        XREF    MSFBINtoBCD
        XREF    MSFBCDtoBIN
        XREF    LSNtoMSFPOS
        XREF    MSFtoLSNPOS
        XREF    LSNtoMSF
        XREF    MSFtoLSN
        XREF    BCDtoBIN
        XREF    BINtoBCD

        XREF    DoPacket
        XREF    SendPacket
        XREF    WaitPacket
        XREF    DoECC

        XREF    MuteCD
        XREF    Attenuate

        XREF    PutHex
        XREF    PutChar

        XDEF    ClearPrefetch
        XDEF    ClearPrefetchIfError

        INT_ABLES

*
************************************************************************
*                                                                      *
*   READ - Read CD-ROM data from disk                                  *
*                                                                      *
*       in:     ARG1 = Start position offset (word aligned)            *
*               ARG2 = Address                                         *
*               ARG3 = XLDMACount                                      *
*               ARG4 = Speed                                           *
*               ARG5 = MaxTransfer                                     *
*               IOR  = IORequest                                       *
*                                                                      *
************************************************************************

 FUNCTION READ
                save    d2-d7/a2-a3                                     ; Save all used registers
                move.l  db_CDROMPage(db),a3                             ; A3 = CDROMPage

                move.l  db_ARG1(db),d2                                  ; Calculate sector and index
                clr.l   d0
                move.w  db_Info+CDINFO_SectorSize(db),d0
                divul.l d0,d1:d2
                move.l  d2,db_ARG1(db)
                move.l  d1,db_SectorIndex(db)

                move.l  db_XferEntry(db),a0                             ; Make sure first XLDMACount is <= MaxTransfer
                move.l  CDXL_Length(a0),d0
                cmp.l   db_ARG5(db),d0
                bls     1$
                move.l  db_ARG5(db),db_ARG3(db)
1$
                clr.b   db_ReadError(db)                                ; No error just yet


*************** Start a brand new READ *********************************

ReadData:
                jsr     GetSector                                       ; Get the sector we requested

                tst.l   d1                                              ; ECC problem?
                beq     1$
                move.b  #CDERR_BadSecSum,db_ReadError(db)
1$
                cmp.l   #-103,d0                                        ; CD-ROM Data?
                bne     2$
                restore d2-d7/a2-a3
                move.w  #CDERR_BadDataType,d0
                rts
2$
                cmp.l   #-104,d0                                        ; Read aborted?
                bne     TransferData
                restore d2-d7/a2-a3
                move.w  #CDERR_ABORTED,d0
                rts


*************** Transfer 'till end of XL node **************************

TransferData:
                jsr     TransferFromSector                              ; Transfer all or part of the sector
                bmi     TransferComplete

                bne     2$                                              ; If all of the sector was transferred...
                move.l  d3,d0                                           ; ... free the buffer and start on next sector
                jsr     FreeSectorBuffer
                addq.l  #1,db_ARG1(db)
2$
                tst.l   db_ARG3(db)                                     ; End of transfer?
                beq     TransferEntryComplete
                bra     ReadData

*
*************** Transfer of CDXL entry is complete *********************

TransferEntryComplete:
                move.l  db_XferEntry(db),a0                             ; Report actual amount transferred
                move.l  CDXL_Length(a0),d0
                cmp.l   db_ARG5(db),d0
                bls     1$
                move.l  db_ARG5(db),d0
1$              move.l  d0,CDXL_Actual(a0)
                add.l   d0,IO_ACTUAL(ior)
                sub.l   d0,db_ARG5(db)

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
                tst.l   db_ARG5(db)                                     ; - Max length exhausted?
                beq     TransferComplete

                move.l  a0,db_XferEntry(db)                             ; New node
                clr.l   CDXL_Actual(a0)

                cmp.l   db_ARG5(db),d0                                  ; Set new address and length
                bls     3$
                move.l  db_ARG5(db),d0
3$              move.l  d0,db_ARG3(db)
                move.l  CDXL_Buffer(a0),db_ARG2(db)

                bra     ReadData                                        ; Resume transferring data


*************** Transfer is complete ***********************************

TransferComplete:
                restore d2-d7/a2-a3                                     ; Report read status
                clr.w   d0
                move.b  db_ReadError(db),d0
                rts



*
*=======================================================================
*=                                                                     =
*=  TransferFromSector - Transfer as much data from sector as possible =
*=                                                                     =
*=======================================================================

TransferFromSector:
                move.b  ROM_HEADER+3(a3,d3.l),d0                        ; Determine mode of sector
                cmp.b   #1,d0
                bne     1$

                move.l  #2048,d1                                        ; Mode 1 (no SH w/2048)
                lea     ROM_DATA(a3,d3.l),a1

                cmp.w   db_Info+CDINFO_SectorSize(db),d1                ; Are the sector sizes equal?
                bne     Error_BadSectorType
                bra     BeginTransfer
1$
                cmp.b   #2,d0                                           ; Mode 2?
                bne     ErrorUnknownMode

                cmp.w   #2048,db_Info+CDINFO_SectorSize(db)             ; Are we expecting a mode 1 sector?
                bne     2$

                move.b  ROM_HEADER+6(a3,d3.l),d0                        ; Get a valid "Submode" byte
                btst    #5,d0                                           ; - Test "FORM" bit of "Submode" byte of "SUBHEADER".
                bne     Error_BadSectorType

                lea     ROM_DATA+8(a3,d3.l),a1                          ; Mode 2 Form 1 (SH w/2048)
                move.l  #2048,d1
                bra     BeginTransfer
2$
                clr.l   d1                                              ; Default to Mode 2 Form 2 (SH w/2328)
                move.w  db_Info+CDINFO_SectorSize(db),d1
                lea     ROM_DATA+8(a3,d3.l),a1
                cmp.w   #2336,d1                                        ; - Alternate Mode 2 Form 2 (SH w/2336)
                bne     BeginTransfer
                lea     ROM_DATA(a3,d3.l),a1
                bra     BeginTransfer

ErrorUnknownMode:
                move.b  #CDERR_BadSecID,db_ReadError(db)                ; Unknown mode
                move.w  db_Info+CDINFO_SectorSize(db),d1                ; - use requested size
                bra     BeginTransfer

Error_BadSectorType:
                move.b  #CDERR_BadSecID,db_ReadError(db)                ; Abort read
                move.w  #-1,d0
                rts


*
*************** Mode/Form determined.  A1=Address, D1=Length ***********

BeginTransfer:
                sub.l   db_SectorIndex(db),d1                           ; Subtract amount of data already transferred D1 = remain
                add.l   db_SectorIndex(db),a1                           ; - Add in current sector index               A1 = source
                move.l  db_ARG2(db),a0                                  ; - Destination address                       A0 = dest

                clr.l   d2                                              ; Don't transfer more data than requested
                move.l  d1,d0
                cmp.l   db_ARG3(db),d0
                bls     1$
                move.l  db_ARG3(db),d0
                move.l  db_SectorIndex(db),d2                           ; - D0 = Transfer size
                add.l   d0,d2                                           ; - D2 = next sector index
1$
                move.l  d2,db_SectorIndex(db)                           ; Next time, index will equal this
                add.l   d0,db_ARG2(db)                                  ; - Next time, start at this address
                sub.l   d0,db_ARG3(db)                                  ; - Less data to transfer next time

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



*
*=======================================================================
*=                                                                     =
*=  WithinRange - See if data is or is approaching data requested      =
*=                                                                     =
*=      in:     D0.l = Sector we have just read                        =
*=              D2.l = Sector we are trying to read                    =
*=                                                                     =
*=     out:                                                            =
*=              z flag = true or false                                 =
*=                                                                     =
*=    note:     D1 is trashed                                          =
*=                                                                     =
*=======================================================================

WithinRange:
                cmp.l   d2,d0                                           ; Are we past where we want to be?
                beq     3$
                bmi     2$
1$              clr.w   d1                                              ; - Not within range
                rts
2$
                move.l  d0,d1                                           ; Are we way short of where we want to be?
                add.l   #10,d1
                cmp.l   d2,d1
                bmi     1$
3$              move.w  #1,d1
                rts



*=======================================================================
*=                                                                     =
*=  ClearPrefetch - Free all prefetch buffers so drive can use them    =
*=                                                                     =
*=======================================================================

ClearPrefetch:
                save    a3
                move.l  db_CDROMPage(db),a3                             ; A3 = CDROMPage

                clr.l   d0                                              ; Set Invalid mark (status word) in buffers
1$              move.w  #SECSTSF_INVALID,ROM_STATUS+2(a3,d0.l)

                add.w   #$1000,d0
                cmp.w   #(PBXSIZE*$1000)&$FFFF,d0
                bne     1$

                restore a3
                rts


*=======================================================================
*=                                                                     =
*=  ClearPrefetchIfError - Free prefetch if next sector has an error   =
*=                                                                     =
*=======================================================================

ClearPrefetchIfError:
                save    a3/d3
                move.l  db_CDROMPage(db),a3                             ; A3 = CDROMPage

                jsr     GetNextSectorBufferECC                          ; Clear prefetch if next sector has an error
                tst.l   d1
                beq     1$
                jsr     ClearPrefetch
1$
                restore a3/d3
                rts


*
*=======================================================================
*=                                                                     =
*=  GetSector - Get sector requested (from prefetch or by reading)     =
*=                                                                     =
*=      out:                                                           =
*=              D0.l = Sector requested                                =
*=                     -103 if not CD-ROM data                         =
*=                     -104 aborted                                    =
*=                                                                     =
*=              D1.l = Non-Zero = ECC failed                           =
*=                                                                     =
*=              D3.l = Index into buffer                               =
*=                                                                     =
*=======================================================================

GetSector:
                move.l  db_ARG1(db),d0                                  ; Do we need to remap this sector?
                cmp.l   db_Remap(db),d0
                bhs     1$
                add.l   db_RemapStart(db),d0
1$
                move.l  d0,d2                                           ; Get oldest new sector (if one is available)
                jsr     GetNextSectorBufferECC
                cmp.l   #-100,d0
                bne     4$

                tst.b   db_Reading(db)                                  ; If we are no longer reading, start reading again
                bne     2$

                jsr     StartRead                                       ; Begin reading new data
                beq     2$

                clr.l   d1                                              ; Bad data type encountered
                move.l  #-103,d0
                rts
2$
                move.l  #SIGF_CMDDONE|SIGF_PBX|SIGF_ABORTDRIVE,d0       ; Wait for data or read abort
                exec    Wait

                btst    #SIGB_CMDDONE,d0                                ; If we reached EOD before our sector, seek back
                beq     3$
                sub.l   #8,db_SeekAdjustment(db)
                bra     7$
3$
                btst    #SIGB_ABORTDRIVE,d0                             ; If read was aborted, stop reading and return
                beq     GetSector
                clr.w   d0
                jsr     StopRead
                clr.l   d1
                move.l  #-104,d0
                rts
4$
                tst.l   d1                                              ; If ECC failed, report error
                bne     5$

                cmp.l   d0,d2                                           ; If this is the sector we requested, return it
                bne     6$
                cmp.b   #1,db_Reading(db)                               ; - We are now reading requested data
                bne     5$
                move.b  #2,db_Reading(db)                               
5$              rts
6$
                tst.b   db_Reading(db)                                  ; If we are doing a read attempt, see if the drive went
                beq     8$                                              ;   where it was supposed to.
                jsr     WithinRange
                bne     8$

                cmp.b   #1,db_Reading(db)                               ; Adjust read if we are searching, otherwise, just reseek
                bne     7$
                move.l  d2,d1
                sub.l   d0,d1
                sub.l   #2,d1                                           ; - A bit more for good measure
                add.l   d1,db_SeekAdjustment(db)
7$
                clr.w   d0                                              ; Stop current read and try reading again
                jsr     StopRead
                jsr     ClearPrefetch
                bra     GetSector
8$
                move.l  d3,d0                                           ; We don't want this sector, free it and try another
                jsr     FreeSectorBuffer
                bra     GetSector

*
*=======================================================================
*=                                                                     =
*=  GetNextSectorBufferECC - Find oldest new sector.  Return LSN/ERROR =
*=                                                                     =
*=       in:    D2.l = Sector we are trying to find                    =
*=                                                                     =
*=      out:                                                           =
*=              D0.l = Sector (LSN)                                    =
*=                     -100 if no data                                 =
*=                                                                     =
*=              D1.l = Non-Zero = ECC failed                           =
*=                                                                     =
*=              D3.l = Index into buffer                               =
*=                                                                     =
*=======================================================================

GetNextSectorBufferECC:
                jsr     GetNextSectorBuffer                             ; Find oldest new sector's address

                cmp.l   #-100,d0                                        ; Data available?
                beq     2$

                move.l  d0,d3                                           ; Perform ECC on data.  Return error if data is bad
                lea     0(a3,d3.l),a0
                jsr     DoECC
                beq     1$

                jsr     1$                                              ; ECC failed
                move.l  #1,d1
                rts
1$
                move.l  ROM_HEADER(a3,d3.l),d0                          ; Figure out the logical sector number
                lsr.l   #8,d0
                jsr     MSFBCDtoBIN
                jsr     MSFtoLSNPOS
2$
                clr.l   d1                                              ; No ECC error
                rts


*
*=======================================================================
*=                                                                     =
*=  GetNextSectorBuffer - Find oldest new sector and return index      =
*=                                                                     =
*=      out:                                                           =
*=              D0.l = Index into CDROMPage or -100 if no data         =
*=                                                                     =
*=======================================================================

GetNextSectorBuffer:
                clr.l   d0                                              ; Traverse buffers to find the next sector
1$              move.l  ROM_STATUS(a3,d0.l),d1
                btst    #SECSTSB_INVALID,d1                             ; - Is this the next sector?
                bne     2$
                and.l   #SECSTSF_CNT,d1
                cmp.b   db_BufferCount(db),d1
                bne     2$

                tst.l   d0
                rts                                                     ; - Return index into CDROMPage
2$
                add.w   #$1000,d0                                       ; Try next buffer
                cmp.w   #(PBXSIZE*$1000)&$FFFF,d0
                bne     1$

                move.l  #-100,d0                                        ; No new sectors in buffer
                rts



*=======================================================================
*=                                                                     =
*=  FreeSectorBuffer - Free up CDROMPage buffer                        =
*=                                                                     =
*=      in:                                                            =
*=              D0.l = CDROMPage buffer index                          =
*=                                                                     =
*=======================================================================

FreeSectorBuffer:
                move.w  #SECSTSF_INVALID,ROM_STATUS+2(a3,d0.l)          ; Set invalid flag of sector

                add.b   #1,db_BufferCount(db)                           ; Next buffer should contain the new count
                and.b   #$1F,db_BufferCount(db)

                rts


*
*=======================================================================
*=                                                                     =
*=  StartRead - Start drive playing and enable CD-ROM decoding         =
*=                                                                     =
*=      in:     D2.l = Sector to begin read                            =
*=                                                                     =
*=======================================================================

StartRead:
                jsr     ClearPrefetch                                   ; Free all buffers

                jsr     MuteCD                                          ; Mute CD audio

                move.l  #SS_MICROS,db_Speed(db)
                cmp.l   #150,db_ARG4(db)
                bne     1$
                move.l  #DS_MICROS,db_Speed(db)
1$
                move.l  #MN_SIZE,d0                                     ; Tell drive task to start reading data
                move.l  #MEMF_PUBLIC|MEMF_CLEAR,d1
                exec    AllocMem
                tst.l   d0
                beq     2$
                move.l  d0,a1
                move.b  #NT_MESSAGE,LN_TYPE(a1)
                move.w  #1,MN_LENGTH(a1)
                move.l  d2,MN_REPLYPORT(a1)
                move.l  db_TaskMsgPort(db),a0
                exec    PutMsg
2$
                move.w  db_Info+CDINFO_Status(db),d0                    ; Start disk spinning if not spinning
                btst    #CDSTSB_SPIN,d0
                bne     3$
                or.w    #CDSTSF_SPIN,db_Info+CDINFO_Status(db)
3$
                move.b  #1,db_Reading(db)                               ; We are now attempting to read data

                move.b  #0,db_BufferCount(db)                           ; Next expected buffer count

                clr.w   d0                                              ; Success
                rts
4$
                clr.w   d0                                              ; Return an error if the play was not successful
                jsr     StopRead
                move.w  #1,d0
                rts


*
*=======================================================================
*=                                                                     =
*=  StopRead - Stop decoding of CD-ROM data and pause drive            =
*=                                                                     =
*=  in:     D0.w = no response?                                        =
*=                                                                     =
*=======================================================================

 FUNCTION StopRead
                save    d0

                tst.b   db_Reading(db)                                  ; Make sure we don't pause what is already paused
                beq     1$

                clr.b   db_Reading(db)                                  ; No longer reading
1$
                move.w  #1,db_DriveAbortTimer(db)                       ; Abort CD-ROM interrupt now

                move.l  #MN_SIZE,d0                                     ; Tell drive task to stop reading data
                move.l  #MEMF_PUBLIC|MEMF_CLEAR,d1
                exec    AllocMem
                tst.l   d0
                beq     2$
                move.l  d0,a1
                move.b  #NT_MESSAGE,LN_TYPE(a1)
                move.w  #2,MN_LENGTH(a1)
                move.l  db_TaskMsgPort(db),a0
                exec    PutMsg
2$
                restore d0
                rts





