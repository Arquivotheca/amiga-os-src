 

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
        XREF    DoSCSI

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



*=======================================================================
*=                                                                     =
*=  ClearPrefetch - Free all prefetch buffers so drive can use them    =
*=                                                                     =
*=======================================================================

ClearPrefetch:
                save    a3
                move.l  db_CDROMPage(db),a3                             ; A3 = CDROMPage

                clr.l   d0                                              ; Set Invalid mark (status word) in buffers
                move.w  #SECSTSF_INVALID,ROM_STATUS+2(a3,d0.l)
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

                tst.w   ROM_STATUS+2(a3)
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
*=       in:                                                           =
*=              ARG1 = Sector to get                                   =
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
                clr.l   ROM_STATUS(a3)

                lea     db_command(db),a0                               ; Read a sector
                move.b  #S_READ10,(a0)
                clr.b   1(a0)
                move.l  d2,2(a0)
                clr.w   6(a0)
                clr.l   d1
                move.w  db_Info+CDINFO_SectorSize(db),d1
                cmp.w   #2048,d1
                beq     2$
                move.w  #2336,d1
2$              move.b  #1,8(a0)
                clr.b   9(a0)
                move.l  #10,d0
                lea     ROM_DATA(a3),a1
                jsr     DoSCSI
                move.l  d0,d1                                           ; - ECC status
                beq     3$
                move.l  #SECSTS_ERROR,ROM_STATUS(a3)
3$              clr.l   d3                                              ; - Buffer 0
                move.l  d2,d0                                           ; - Sector requested

                tst.b   d1                                              ; Check for an error
                bne     5$

                clr.l   d0                                              ; Abort signal received?
                move.l  d0,d1
                exec    SetSignal
                btst    #SIGB_ABORTDRIVE,d0
                beq     4$

                clr.l   d0                                              ; Clear this signal and return error
                move.l  #SIGF_ABORTDRIVE,d1
                exec    SetSignal
                move.l  #-104,d0
                rts
4$
                clr.l   d1                                              ; Success
5$              rts





