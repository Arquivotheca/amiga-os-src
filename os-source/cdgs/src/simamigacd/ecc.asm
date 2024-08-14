
**
**      $Id: ecc.asm,v 1.1 93/07/19 11:13:24 jerryh Exp Locker: jerryh $
**
**      cd.device Reed-Solomon Error Correction Code
**
**      Downcoded (based on code provided by Allan Havemose)
**
**      See C code for documents, and more details
**
**      (C) Copyright 1993 Commodore-Amiga, Inc.
**          All Rights Reserved
**
**      
**

                OPT     p=68020

** Includes
                INCLUDE "defs.i"
                INCLUDE "cd.i"
                INCLUDE "cdprivate.i"
                INCLUDE "cdgs_hw.i"                                          
                INCLUDE "exec/types.i"

** Imports
                XREF    MSFBINtoBCD
                XREF    MSFBCDtoBIN
                XREF    LSNtoMSFPOS
                XREF    MSFtoLSNPOS
                XREF    LSNtoMSF
                XREF    MSFtoLSN
                XREF    BCDtoBIN
                XREF    BINtoBCD

                XREF    PutHex
                XREF    PutChar

** Equates

TRY_MAX                 EQU     5

ECC_OK                  EQU     0
ECC_ERROR               EQU     1               ;unable to correct all errors
ECC_HEADER_ERROR        EQU     2               ;uncorrectable error in CDROM header!

P_ECC_PROB              EQU     $100            ;P-ECC not able to correct data */
Q_ECC_PROB              EQU     $100            ;Q-ECC not able to correct data */


                SECTION cdecc


********************************************************************************
*                                                                              *
*   DoECC - Perform error correction on CD-ROM data if necessary               *
*                                                                              *
*        in:                                                                   *
*                        a0 = pointer to 4k buffer                             *
*                                                                              *
*               ior equr a4 = pointer to IORequest  (you should not need)      *
*               hb  equr a5 = hardware base pointer (you should not need)      *
*               db  equr a6 = pointer to global data structure                 *
*                                                                              *
*       out:                                                                   *
*             D0.w      = 0: sector is now valid, else still invalid           *
*             Z flag    = status of d0.w                                       *
*                                                                              *
*     NOTES: d2-d7/a2-a6 should be preserved                                   *
*                                                                              *
********************************************************************************

 FUNCTION DoECC
                move.l  ROM_STATUS(a0),d0                                       ; If correction has already been performed, don't
                btst    #SECSTSB_CORRECTED,d0                                   ;     do it again.  Return status of sector.
                beq     1$
                jsr     IsError
                rts
1$
                or.l    #SECSTSF_CORRECTED,ROM_STATUS(a0)                       ; Correction will have been performed on this sector

                jsr     IsError                                                 ; Is there an error?
                bne     2$

                clr.w   d0                                                      ; Return good status
                rts
2$
                tst.b   db_ECC(db)                                              ; ECC enabled?
                beq     4$

                move.l  ROM_HEADER(a0),d1                                       ; Save header information just incase
                tst.b   db_PhotoCD(db)
                beq     3$
                clr.l   ROM_HEADER(a0)
3$
                save    d1/a0                                                   ; Do ECC on data
                jsr     PerformECC
                restore d1/a0

                tst.l   d0                                                      ; If not correctable, return error status
                beq     5$
4$              move.w  #1,d0
                rts
5$
                tst.l   ROM_HEADER(a0)                                          ; PhotoCD disk?
                bne     6$
                move.b  #1,db_PhotoCD(db)
                move.l  d1,ROM_HEADER(a0)
6$
                move.l  ROM_STATUS(a0),d0                                       ; Error was corrected, clear error flags
                and.l   #-1-SECSTS_ERROR,d0
                move.l  d0,ROM_STATUS(a0)
                clr.w   d0
                rts



*===============================================================================
*=                                                                             =
*=  IsError - Determine if there is an error in a 2k data block                =
*=                                                                             =
*=     out: D0.w = 0: no error, 1: error                                       =
*=                                                                             =
*===============================================================================

 FUNCTION IsError
                move.l  ROM_STATUS(a0),d0                                       ; Get sector status

                btst    #SECSTSB_SHORTSECTOR,d0                                 ; Is there a short sector?
                bne     3$

                move.b  ROM_HEADER+3(a0),d1                                     ; Don't error correct mode 2 form 2 sectors
                cmp.b   #2,d1
                bne     1$
                move.b  ROM_HEADER+6(a0),d1
                btst    #5,d1
                bne     2$
1$
                and.l   #(SECSTSF_EDC1|SECSTSF_EDC2),d0                         ; Did both ECCs fail?
                cmp.l   #(SECSTSF_EDC1|SECSTSF_EDC2),d0
                beq     3$
2$
                clr.w   d0                                                      ; Sector is good
                rts
3$
                moveq.w #1,d0                                                   ; Sector is bad
                rts




********************************************************************************
*                                                                              *
*   PerformECC - Perform error correction on CD-ROM data                       *
*                                                                              *
*        in:                                                                   *
*                        a0 = pointer to 4k buffer                             *
*                                                                              *
*               ior equr a4 = pointer to IORequest  (you should not need)      *
*               hb  equr a5 = hardware base pointer (you should not need)      *
*               db  equr a6 = pointer to global data structure                 *
*                                                                              *
*       out:                                                                   *
*             D0.l      = 0: sector is now valid, else still invalid           *
*             Z flag    = status of d0.l                                       *
*                                                                              *
*     NOTES: d2-d7/a2-a6 should be preserved                                   *
*                                                                              *
********************************************************************************
PerformECC:

; variable delays should be done here and unrecoverable errors should be simulated

                clr.l   d0
                rts

