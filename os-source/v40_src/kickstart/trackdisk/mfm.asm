 
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* mfm.asm
*
* Source Control
* ------ -------
* 
* $Id: mfm.asm,v 27.7 90/06/01 23:17:41 jesup Exp $
*
* $Locker:  $
*
* $Log:	mfm.asm,v $
* Revision 27.7  90/06/01  23:17:41  jesup
* Conform to include standard du jour
* 
* Revision 27.6  90/06/01  16:30:07  jesup
* Fix to make trackdisk no clear a bit of it's tcb nexttask field
* 
* Revision 27.5  90/03/16  01:06:28  jesup
* Add comments
* 
* Revision 27.4  89/12/10  18:34:21  jesup
* Added read/write to anywhere in memory
* 
* Revision 27.3  89/04/27  23:38:46  jesup
* fixed autodocs
* 
* Revision 27.2  89/02/17  20:20:14  jesup
* Addition of MfmSumBufferDecode
* Many minor code optimizations
* 
* Revision 27.1  85/06/24  13:37:27  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:20  neil
* *** empty log message ***
* 
*
*************************************************************************

****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/memory.i'

	INCLUDE 'hardware/custom.i'

	INCLUDE 'resources/disk.i'

	INCLUDE 'devices/timer.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'

	SECTION section


****** Imported Names ***********************************************

****** Tables *******************************************************

****** Defines ******************************************************

****** Functions ****************************************************

	XREF	TDDskBlk
	XREF	WaitDskBlk
	XREF	TDDoBlit

****** System Library Functions *************************************

    EXTERN_LIB  QBlit
    EXTERN_LIB	CopyMem
    EXTERN_LIB	CopyMemQuick
    EXTERN_LIB	TypeOfMem

****** Exported Names ***********************************************

****** Functions ****************************************************

    XDEF    TDMfmSecEncode
    XDEF    TDMfmSecDecode
    XDEF    TDMfmEncode
    XDEF    TDMfmDecode
    XDEF    TDMfmLongEncode
    XDEF    TDMfmLongDecode
    XDEF    TDMfmSumBuffer
    XDEF    TDMfmSumBufferDecode
    XDEF    TDMfmCPUDecode
    XDEF    TDMfmFixBit

****** Data *********************************************************

****** Local Definitions ********************************************

NULLS       EQU     $0AAAAAAAA              ; two bytes of MFM coded NULLS
FUNNYA1     EQU     $044894489              ; a magic illegal pattern



*-- used to talk with the blitter queue
 STRUCTURE BL,0
    APTR    BL_NEXT
    APTR    BL_FUNC             ;-- function that will be called
    ULONG   BL_LENGTH           ;-- number of bytes to write
    APTR    BL_DATA             ;-- pointer to user data
    APTR    BL_BUFFER           ;-- pointer to mfm encoded area
    UWORD   BL_BLTSIZE          ;-- parameter for bltsize register
    UBYTE   BL_SHIFT            ;-- bit offset of data in BUFFER
    UBYTE   BL_JUNK             ;-- padding for word allignment
    UWORD   BL_SAVE             ;-- a place to save a word
    APTR    BL_UNIT
    LABEL   BL_SIZE


*****i* trackdisk.device/internal/TDMfmSecEncode ********************
*
*   NAME
*       TDMfmSecEncode - Encode a sector in AMIGA0.0 Mfm format.
*
*   SYNOPSIS
*       TDMfmSecEncode( TrackID, DataPtr, BufferPtr ), TDLib
*                       D0,      A0,      A1,          A6
*
*   FUNCTION
*       This routine encodes a sectors worth of data into a buffer.
*       It not only encodes using our funny sector encoding scheme,
*       but adds all the extra crap that is needed to make a disk
*       sector.
*
*   INPUTS
*       TrackID - a long that contains the sector identifier that is
*           to be put in the header.  This routine does not interpret
*           the trackid.
*
*       DataPtr - a word pointer into the users data buffer.  It
*           MUST be word alligned.
*
*       BufferPtr - a word pointer into the track buffer.  For the
*           moment it MUST be word alligned, but in the future we
*           will support arbitrary bit allignement so we may update
*           sectors in place.
*
*   EXCEPTIONS
*
*   SEE ALSO
*       MfmSecDecode, MfmEncode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*       A sector looks like the following (before MFM encoding):
*
*   00 00 A1* A1* Format TrackID SecID SecPos SecLabel HdrSum DataSum <data>
*
*   00  - a byte of data zero's
*   A1*  - a funny A1, which will have a missing clock once encoded.
*   Format - a byte to identify the format used to encode this Sector
*   TrackID - a byte to identify which track we are on.
*   SecID - a byte to identify which sector we are
*   SecPos - a byte to identify the sector offset from the gap
*   SecLabel - a byte array used to hold sector recovery information
*   HdrSum - four bytes for a checksum for the header (exclusive OR)
*   DataSum - four bytes to sum/crc/etc the data (exclusive OR)
*   <data>  - 512 bytes of data, to be encoded in a funny way
*
*   = 544 bytes/sector.  MFM encoding will double this size.
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*       A1 -- BUFFER pointer
*       A0 -- DATA pointer
*
*
**********************************************************************


TDMfmSecEncode:

            MOVEM.L A2/A4/D2,-(SP)
            MOVE.L  A1,A4
            MOVE.L  A0,A2
            MOVE.L  D0,D2

*       ;-- take care of the header stuff

*           ;-- Add the two "bytes" of nulls (4 bytes when encoded)
*	    ;-- MfmLongEncode takes care of all clock bit issues
            MOVEQ   #0,D0
            LEA     MFMSEC_ZEROS(A4),A0
            BSR     TDMfmLongEncode

*           ;-- Add the two "bytes" of funny A$1
            MOVE.L  #FUNNYA1,MFMSEC_A1(A4)

*           ;-- Add the format word and sector label
            MOVE.L  D2,D0                ; restore D0

            LEA     MFMSEC_FMT(A4),A0
            BSR     TDMfmLongEncode (D0,A0) ; A0 will be bumped

*	    ;------ A0 now points to the beginning of the label
	    MOVEQ   #(TD_LABELSIZE>>2)-1,D2
SecEncode_10:
	    MOVEQ   #0,D0
	    BSR     TDMfmLongEncode (D0,A0)
	    DBF     D2,SecEncode_10

*	    ;------ Compute the header sum
	    LEA     MFMSEC_FMT(A4),A0
	    MOVEQ   #MFMFMT_HDRSUM,D1
	    BSR     TDMfmSumBuffer (D1,A0)

	    LEA     MFMSEC_FMT+MFMFMT_HDRSUM(A4),A0
	    BSR     TDMfmLongEncode (D0,A0)

*           ;-- encode the data.  A0,A1 will be updated
            MOVE.L  #TD_SECTOR,D0

            MOVE.L  A2,A0
            LEA     MFMSEC_DATA(A4),A1
            BSR.S   TDMfmEncode     (D0,A0,A1)

*           ;-- compute the checksum and put it in the buffer
            LEA     MFMSEC_DATA(A4),A0
	    MOVE.W  #MFM_SECTOR,D1
            BSR     TDMfmSumBuffer (D1,A0)

            LEA     MFMSEC_DATASUM(A4),A0
            BSR     TDMfmLongEncode

*           ;-- All done!
            MOVEM.L (SP)+,D2/A2/A4
            RTS



*****i* trackdisk.device/interal/TDMfmEncode ************************
*
*   NAME
*       TDMfmEncode - Encode data in AMIGA0.0 Mfm Format.
*
*   SYNOPSIS
*       TDMfmEncode( Length, DataPtr, BufferPtr ), TDLib
*                    D0,     A0,      A1,          A6
*
*   FUNCTION
*       This routine encodes data using the AMIGA0.0 Data encoding
*       scheme.  This method of coding is not very intuitive, but is
*       very fast to encode and decode, and is well suited to using
*       the blitter.
*
*   INPUTS
*       Length - the number of bytes to encode.  This must be divisible
*           by $10 (16) bytes to ease computing the blit length.
*           The length is before mfm encoding -- it will be twice
*           as much after mfm encoding.
*
*       DataPtr - a word pointer into the users data buffer.  It
*           MUST be word alligned.
*
*       BufferPtr - a word pointer into the track buffer.  For the
*           moment it MUST be word alligned, but in the future we
*           will support arbitrary bit allignement so we may update
*           sectors in place.
*
*
*   EXCEPTIONS
*
*   SEE ALSO
*       MfmSecEncode, MfmDecode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*       This routines will encode bytes in the following funny manner,
*   called AMIGA0.0mfm ("n" is the Length argument):
*
*       data looks like:          buffer looks like (all mfm encoded)
*
*       |<- 16 bits->|                |<------ 16 bits ------>|
* data addr                   buffer addr
* ----- /------------\        -----   /-----------------------\
* +0    | word  0    |        +0      |  odd bits of word 0   |
*       |------------|                |-----------------------|
* +1    | word  1    |        +1      |  odd bits of word 1   |
*       |------------|                |-----------------------|
*       |      *     |                |           *           |
*       |      *     |                |           *           |
*       |      *     |                |           *           |
*       |------------|                |-----------------------|
* +n -2 | word  n -2 |        +n -2   |  odd bits of word n-2 |
*       |------------|                |-----------------------|
* +n -1 | word  n -1 |        +n -1   |  odd bits of word n-1 |
*       \------------/                |-----------------------|
* +n                          +n      | even bits of word 0   |
*                                     |-----------------------|
*                             +n +1   | even bits of word 1   |
*                                     |-----------------------|
*                                     |           *           |
*                                     |           *           |
*                                     |           *           |
*                                     |-----------------------|
*                             +2n -2  | even bits of word n-2 |
*                                     |-----------------------|
*                             +2n -1  | even bits of word n-1 |
*                                     \-----------------------/
*                             +2n
*
*
*       The reason for this funny encoding is to allow easy decoding
*   of the mfm.  Mfm encoding is defined as a clock bit followed by
*   a data bit.  To decode this encoding of data bits, all we need to
*   do is:
*
*       ((odd bits word & 0x5555) << 1) | (even bits word & 0x5555)
*
*       This can all be done in one blitter pass.
*
*       The encoding is done in four separate passes.  Two passes
*   encode the odd bits, and two passes encode the even bits.  The
*   first pass encodes the data bits and the clock bits.  The second
*   pass corrects the clock bits of the first pass by unsetting the
*   clock bit of a zero cell that follows a one cell.  In all four
*   passes input C is a mask of $5555.  This mask will be used to
*   "cookie cut" the input into clock and data bits.  In all cases
*   C refers to data bits and C* to clock bits.
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************

TDMfmEncode:
*
* First check for type of memory
*
	movem.l	D0/A0/A1/A6,-(a7)
	move.l	TD_SYSLIB(a6),a6		; exec
	move.l	a0,a1				; ptr to destination
	CALLSYS	TypeOfMem
	btst	#MEMB_CHIP,d0			; is it chip mem?
	bne.s	encode_it

	movem.l	(a7),d0/a0			; size/src - tricky!
	move.l	TDU_CHIPBUF(a3),a1		; dest for cpu move
	move.l	a1,4(a7)			; new src after copy

	;-- are we aligned?
	move.l	a0,d1
	btst	#1,d1				; word or longword?
	beq.s	longword

	;-- word aligned - use CopyMem
	CALLSYS	CopyMem
	bra.s	encode_it

longword:
	CALLSYS	CopyMemQuick
	;-- fall thru

encode_it:
	;-- may pull different source (the unit chip buf) than asked for
	movem.l	(a7)+,d0/a0/a1/a6		; restore regs
*
*
*

*           ;-- get storage for the blitter buffer
            LINK    A2,#-BL_SIZE

*       ;-- this routine is completely blitter based.  prepare the
*       ;--     buffer that we will pass to the blitter.

*           ;-- compute bltsize
            MOVE.W  D0,D1
	    LSL.W   #2,D1
            OR.W    #$8,D1                 ; horizontal size = 8 words
            MOVE.W  D1,BL_BLTSIZE-BL_SIZE(A2)

*           ;-- save the arguments
            MOVEM.L D0/A0-A1,BL_LENGTH-BL_SIZE(A2)

*           ;-- intialize the blitter function pointer
            MOVE.L  #MfmEncodeBlit_00,BL_FUNC-BL_SIZE(A2)

	    MOVE.L  A3,BL_UNIT-BL_SIZE(A2)

*	    ;-- clear the dma done signal just in case
*	    ;-- stupid 68000 bclr!  It so happens DSKBLK is >= 8 and < 16
*	    ;-- really evil
	    BCLR.B  #TDSB_DSKBLK-8,TDU_TCB+TC_SIGRECVD+2(A3)

*           ;-- call the blit enqueue routine
            LEA     -BL_SIZE(A2),A1
	    DOGFX   QBlit

*           ;-- Wait on the blitter signal
	    BSR     WaitDskBlk
*MfmEncode_10
*            TST.L   BL_FUNC-BL_SIZE(A2)
*            BNE.S   MfmEncode_10

            MOVEM.L BL_LENGTH-BL_SIZE(A2),A0-A1/D0

*           ;-- we must now go back and fix the first cell of the
*           ;-- odd and even ranges -- we might have to unset the
*           ;-- clock bit of these cells.

	    MOVE.L  D0,D1
	    MOVE.L  A1,A0
	    BSR     TDMfmFixBit		; first word of even bits

	    ADD.L   D1,A0
	    BSR     TDMfmFixBit		; first word of odd bits

	    ;-- Must also do first word of next sector (we may have a 1 as last bit)
            ADD.L   D1,A0
	    BSR     TDMfmFixBit		; first word of next sectors NULLs (AAAA...)

MfmEncode_End
*           ;-- we are all done.  now we can clean up

            UNLK    A2

            RTS

*********************************************************************
*
*       All these blit routines are called with the following args:
*
*   A0: a pointer to _custom ($DFF000)
*   A1: a pointer to a BL data structure
*   A5: a scratch register
*
*       The BL structure will not be dequeued until it returns with
*   the Z function code set (e.g. it does a BEQ)
*
*   REGISTER ALLOCATION
*   A5 -- ptr to BUFFER
*   A0 -- ptr to _custom
*   A1 -- ptr to BL structure
*   D0 -- LENGTH of transfer
*   D1 -- ptr to DATA
*
*********************************************************************

MfmEncodeBlit_00:

*       ;-- first part of odd bit encode.  Equation is  AC + B*C*
*       ;-- where C is the clock/data cookie cut mask, A points
*       ;-- to the odd data bits of the source alligned to the
*       ;-- data portion of the destination, and B points to the
*       ;-- source data bits alligned for the clock bits of the
*       ;-- destination.

*           ;-- get out the magic registers
	    MOVE.L  A5,-(SP)

	    ;-- set up standard registers
	    MOVE.L  A1,A5
	    BSR     TDDoBlit
	    MOVE.L  A5,A1

            MOVEM.L BL_LENGTH(A1),D0-D1/A5

*           ;-- load up the B source
            MOVE.L  D1,bltbpt(A0)          ; B src = DATA
*****       MOVE.W  #0,bltbmod(A0)

*           ;-- load up the A source
            MOVE.L  D1,bltapt(A0)          ; A src = DATA
*****       MOVE.W  #0,bltamod(A0)

*           ;-- load up the C souce
*****       MOVE.W  #$5555,bltcdat(A0)

*           ;-- load up the D source
            MOVE.L  A5,bltdpt(A0)
*****       MOVE.W  #00,bltdmod(A0)

*       ;-- set the control registers
*           ;-- bltcon0 = A_OFFSET | USEA | USEB | USED | AC + B*C*
*           ;--            1000      0800   0400   0100   A0   11
            MOVE.W  #$01DB1,bltcon0(A0)
            MOVE.W  #0,bltcon1(A0)
*****       MOVE.W  #$0FFFF,bltafwm(A0)
*****       MOVE.W  #$0FFFF,bltalwm(A0)

*       ;-- and start the blit!
            MOVE.W  BL_BLTSIZE(A1),bltsize(A0)

            MOVE.L  #MfmEncodeBlit_10,BL_FUNC(A1)
	    MOVE.L  (SP)+,A5

            RTS

MfmEncodeBlit_10:

*       ;-- In this pass we will go back and patch up the encoding of
*       ;-- pass one.  We currently have a series of two bit cells,
*       ;-- each of which is either a "01" or "10" (data 1 or 0
*       ;-- respectively).  The mfm rules for encoding is that a
*       ;-- zero cell that follows a one cell must be a "00" instead
*       ;-- of a "10".  We will therefore go back and patch these cells.
*       ;--
*       ;-- The equation for this pass is A*BC* + BC
*       ;-- B is the result of the previous pass.  BC preserves all
*       ;-- the data bits of the cells we have encoded.  A is data
*       ;-- source delayed for two bits -- this results in the
*       ;-- odd bits alligned over the next cells clock. A*BC* will
*       ;-- unset the clock bit of any cell that follows a cell with
*       ;-- data of one -- just what we want!


*           ;-- get out the magic registers
	    MOVE.L  A5,-(SP)
            MOVEM.L BL_LENGTH(A1),D0-D1/A5

*           ;-- load up the B source
            MOVE.L  A5,bltbpt(A0)          ; B src = BUFFER

*           ;-- load up the A source
            MOVE.L  D1,bltapt(A0)          ; A src = DATA

*           ;-- load up the D source
            MOVE.L  A5,bltdpt(A0)

*       ;-- set the control registers
*           ;-- bltcon0 = A_OFFSET | USEA | USEB | USED | A*BC* + BC
*           ;--            2000      0800   0400   0100    04     88
            MOVE.W  #$02D8C,bltcon0(A0)

*       ;-- and start the blit!
            MOVE.W  BL_BLTSIZE(A1),bltsize(A0)

            MOVE.L  #MfmEncodeBlit_20,BL_FUNC(A1)

	    MOVE.L  (SP)+,A5
            RTS

MfmEncodeBlit_20:

*       ;-- first part of even bit encode.  Equation is  AC + B*C*
*       ;-- where C is the clock/data cookie cut mask, A points
*       ;-- to the even data bits of the source alligned to the
*       ;-- data portion of the destination, and B points to the
*       ;-- source data bits alligned for the clock bits of the
*       ;-- destination.


*           ;-- get out the magic registers
	    MOVE.L  A5,-(SP)
            MOVEM.L BL_LENGTH(A1),D0-D1/A5

*           ;-- make D1 point to the last word in the source area
            ADD.L   D0,D1
            SUBQ.L  #2,D1

*           ;-- make A5 point to the end of the even bits dest area
            ADDA.L  D0,A5
            ADDA.L  D0,A5
            SUBQ.L  #2,A5

*           ;-- load up the B source
            MOVE.L  D1,bltbpt(A0)          ; B src = DATA

*           ;-- load up the A source
            MOVE.L  D1,bltapt(A0)          ; A src = DATA

*           ;-- load up the D source
            MOVE.L  A5,bltdpt(A0)

*       ;-- set the control registers
*           ;-- bltcon0 = A_OFFSET | USEA | USEB | USED | AC + B*C*
*           ;--            0000      0800   0400   0100   A0   11
            MOVE.W  #$00DB1,bltcon0(A0)

*           ;-- bltcon1 = B_OFFSET | DESC
*           ;--             1000      02
            MOVE.W  #$1002,bltcon1(A0)


*       ;-- and start the blit!
            MOVE.W  BL_BLTSIZE(A1),bltsize(A0)

            MOVE.L  #MfmEncodeBlit_30,BL_FUNC(A1)
	    MOVE.L  (SP)+,A5

            RTS

MfmEncodeBlit_30:

*       ;-- This pass serves the same purpose as the one after
*       ;-- MfmEncodeBlit_30.  It is structured very similarly.
*       ;-- The major difference is that we will delay the A blit
*       ;-- input for only one bit time instead of two.  This is
*       ;-- because we are now encoding the even numbered bits, which
*       ;-- are already in the correct places in the "cell" (e.g. bit
*       ;-- zero is already in bit zero).  We need to shift right one
*       ;-- more bit to unset those clocks that should not be set.


*           ;-- get out the magic registers
	    MOVE.L  A5,-(SP)
            MOVEM.L BL_LENGTH(A1),D0-D1/A5

*           ;-- load up the B source
            ADDA.L  D0,A5                   ; A4 -> begin of even BUFFER
            MOVE.L  A5,bltbpt(A0)          ; B src = BUFFER

*           ;-- load up the A source
            MOVE.L  D1,bltapt(A0)          ; A src = DATA

*           ;-- load up the D source
            MOVE.L  A5,bltdpt(A0)

*       ;-- set the control registers
*           ;-- bltcon0 = A_OFFSET | USEA | USEB | USED | A*BC* + BC
*           ;--            1000      0800   0400   0100    04     88
            MOVE.W  #$01D8C,bltcon0(A0)
            MOVE.W  #0,bltcon1(A0)

*       ;-- and start the blit!
            MOVE.W  BL_BLTSIZE(A1),bltsize(A0)

            MOVE.L  #MfmEncodeBlit_End,BL_FUNC(A1)

	    MOVE.L  (SP)+,A5
            RTS

MfmEncodeBlit_End

	    MOVEQ   #0,D0
            MOVE.L  D0,BL_FUNC(A1)
	    MOVE.L  BL_UNIT(A1),A1
	    BSR     TDDskBlk
	    MOVEQ   #0,D0

            RTS


*****i* trackdisk.device/interal/TDMfmSecDecode *********************
*
*   NAME
*       TDMfmSecDecode - Decode a sector in AMIGA0.0 Mfm format.
*
*   SYNOPSIS
*       TDMfmSecDecode( TrackID, DataPtr, BufferPtr ), TDLib
*                       D0,      A0,      A1,          A6
*
*   FUNCTION
*       This routine encodes a sectors worth of data into a buffer.
*       It not only encodes using our funny sector encoding scheme,
*       but adds all the extra crap that is needed to make a disk
*       sector.
*
*   INPUTS
*       DataPtr - a word pointer into the users data buffer.  It
*           MUST be word alligned.
*
*       BufferPtr - a word pointer into the track buffer.  For the
*           moment it MUST be word alligned, but in the future we
*           will support arbitrary bit allignement so we may update
*           sectors in place.
*
*   OUTPUTS
*       TrackID - a word that contains the sector identifier that is
*           to be put in the header.  This routine does not interpret
*           the trackid.
*
*   EXCEPTIONS
*
*   SEE ALSO
*       MfmSecEncode, MfmDecode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*       See TDMfmEncode for a description of the sector format
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************


TDMfmSecDecode:

            MOVEM.L D2/A2/A4,-(SP)

            MOVE.L  A0,A2
            MOVE.L  A1,A4

*       ;-- take care of the header stuff

*           ;-- skip the null and A1* stuff (4 decoded bytes, 8 encoded)
            ADDQ.L  #8,A4

*           ;-- Add the two "bytes" of funny A$1
            MOVE.L  A4,A0
            BSR     TDMfmLongDecode
            MOVE.L  D0,D2
            ADDQ.L  #8,A4

*           ;-- !!! Do something with the checksum
            ADDQ.L  #8,A4

*           ;-- encode the data.  A0,A1 will be updated
            MOVE.L  #TD_SECTOR,D0
            MOVE.L  A4,A1
            MOVE.L  A2,A0

            BSR.S   TDMfmDecode     (D0,A0,A1)

*           ;-- All done!
            MOVE.L  D2,D0
            MOVEM.L (SP)+,D2/A2/A4
            RTS



*****i* trackdisk.device/interal/TDMfmDecode *************************
*
*   NAME
*       TDMfmDecode - Decode data in AMIGA0.0 Mfm Format.
*
*   SYNOPSIS
*       TDMfmDecode( Length, DataPtr, BufferPtr ), TDLib
*                    D0,     A0,      A1,          A6
*
*   FUNCTION
*       This routine decodes data using the AMIGA0.0 Data encoding
*       scheme.  This method of coding is not very intuitive, but is
*       very fast to encode and decode, and is well suited to using
*       the blitter.
*	Modified to allow transfer to anywhere in memory, using the CPU
*	if needed.
*
*   INPUTS
*       Length - the number of bytes to encode.  This must be divisible
*           by $10 (16) bytes to ease computing the blit length.
*           The length is before mfm encoding -- it will be twice
*           as much after mfm encoding.
*
*       DataPtr - a word pointer into the users data buffer.  It
*           MUST be word alligned.
*
*       BufferPtr - a word pointer into the track buffer.  For the
*           moment it MUST be word alligned, but in the future we
*           will support arbitrary bit allignement so we may update
*           sectors in place.
*
*
*   EXCEPTIONS
*
*   SEE ALSO
*       MfmSecDecode, MfmEncode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*       See TDMfmEncode for a description of the AMIGA0.0 Mfm encoding
*   format.
*
*       We will do the decoding in one pass of the blitter (which is
*   the whole reason behind AMIGA0.0mfm encoding scheme). The equation
*   for the blit is:
*
*           (A << 1)C* + BC
*
*       Where A points to the "odd numbered bit" area, and B points
*   to the "even numbered bit" area.  C, as usual, is a bit mask
*   of 0x5555 -- e.g. all the even numbered bits.  Because we need
*   to left shift the odd bits, we will run the blit in descending
*   mode.
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************


TDMfmDecode:
*
* First check for type of memory
*
	movem.l	d0/a0/a1/a6,-(a7)
	move.l	TD_SYSLIB(a6),a6		; exec
	move.l	a0,a1				; ptr to destination
	CALLSYS	TypeOfMem
	movem.l	(a7)+,d1/a0/a1/a6		; tricky!!! (size into d1!)
	btst	#MEMB_CHIP,d0			; is it chip mem?
	bne.s	use_blit

*	;-- use the cpu to decode the bytes
	;-- takes input in d1 (size of dest), a0 (dest), and a1 (src)
	bra	TDMfmCPUDecode			; decode and exit

use_blit:
	move.l	d1,d0				; put length back in d0!

*           ;-- get storage for the blitter buffer
            LINK    A2,#-BL_SIZE

*       ;-- this routine is completely blitter based.  prepare the
*       ;--     buffer that we will pass to the blitter.

*           ;-- save the arguments
            MOVEM.L D0/A0-A1,BL_LENGTH-BL_SIZE(A2)

*           ;-- compute bltsize
            LSL.W   #2,D0                   ; remember, D0 mod 16 == 0
            OR.W    #$8,D0                 ; horizontal size = 8 words
            MOVE.W  D0,BL_BLTSIZE-BL_SIZE(A2)

*           ;-- intialize the blitter function pointer
            MOVE.L  #MfmDecodeBlit_0,BL_FUNC-BL_SIZE(A2)

	    MOVE.L  A3,BL_UNIT-BL_SIZE(A2)

*	    ;-- clear the dma done signal just in case
*	    ;-- stupid 68000 bclr!  It so happens DSKBLK is >= 8 and < 16
*	    ;-- really evil
	    BCLR.B  #TDSB_DSKBLK-8,TDU_TCB+TC_SIGRECVD+2(A3)

*           ;-- call the blit enqueue routine
            LEA     -BL_SIZE(A2),A1
            DOGFX   QBlit

*           ;-- Wait on the blitter signal
	    BSR     WaitDskBlk
MfmDecode_10
*****       TST.L   BL_FUNC-BL_SIZE(A2)
*****       BNE.S   MfmDecode_10

*           ;-- we are all done.  now we can clean up

            MOVEM.L BL_LENGTH-BL_SIZE(A2),A0-A1/D0
            UNLK    A2

            RTS

*********************************************************************
*
*       All these blit routines are called with the following args:
*
*   A1: a pointer to _custom ($DFF000)
*   A2: a pointer to a BL data structure
*   A5: a scratch register
*
*       The BL structure will not be dequeued until it returns with
*   the Z function code set (e.g. it does a BEQ)
*
*   REGISTER ALLOCATION
*   A5 -- ptr to BUFFER
*   A1 -- ptr to _custom
*   A2 -- ptr to BL structure
*   D0 -- LENGTH of transfer
*   D1 -- ptr to DATA
*
*********************************************************************

MfmDecodeBlit_0:

*       ;-- first part of even bit encode.  Equation is  AC + B*C*
*       ;-- where C is the clock/data cookie cut mask, A points
*       ;-- to the even data bits of the source alligned to the
*       ;-- data portion of the destination, and B points to the
*       ;-- source data bits alligned for the clock bits of the
*       ;-- destination.


*           ;-- get out the magic registers
	    MOVE.L  A5,-(SP)

	    MOVE.L  A1,A5
	    BSR     TDDoBlit
	    MOVE.L  A5,A1

            MOVEM.L BL_LENGTH(A1),D0-D1/A5

*           ;-- load up the A source
	    LEA     -1(A5,D0.L),A5
            MOVE.L  A5,bltapt(A0)          ; A src = &BUFFER[LEN -1]
*****       MOVE.W  #0,bltamod(A0)

*           ;-- load up the B source
            ADDA.L  D0,A5
            MOVE.L  A5,bltbpt(A0)          ; B src = &BUFFER[2*LEN -1]
*****       MOVE.W  #0,bltbmod(A0)

*           ;-- load up the C souce
*****       MOVE.W  #$5555,bltcdat(A0)

*           ;-- load up the D source
            ADD.L   D0,D1
            SUBQ.L  #1,D1
            MOVE.L  D1,bltdpt(A0)          ; D dst = &BUFFER[LEN -1]
*****       MOVE.W  #0,bltdmod(A0)

*       ;-- set the control registers
*           ;-- bltcon0 = A_OFFSET | USEA | USEB | USED | AC* + BC
*           ;--            1000      0800   0400   0100   50    88
            MOVE.W  #$01DD8,bltcon0(A0)

*           ;-- bltcon1 = B_OFFSET | DESC
*           ;--             0000      02
            MOVE.W  #$0002,bltcon1(A0)

*****       MOVE.W  #$0FFFF,bltafwm(A0)
*****       MOVE.W  #$0FFFF,bltalwm(A0)

*       ;-- and start the blit!
            MOVE.W  BL_BLTSIZE(A1),bltsize(A0)

            MOVE.L  #MfmDecodeBlit_10,BL_FUNC(A1)
	    MOVE.L  (SP)+,A5

            RTS


MfmDecodeBlit_10

	    MOVEQ   #0,D0
            MOVE.L  D0,BL_FUNC(A1)
	    MOVE.L  BL_UNIT(A1),A1
	    BSR     TDDskBlk
	    MOVEQ   #0,D0

            RTS



*****i* trackdisk.device/interal/TDMfmLongEncode *********************
*
*   NAME
*       TDMfmLongEncode - Encode data in AMIGA0.0 Mfm Format by hand
*
*   SYNOPSIS
*       TDMfmLongEncode( LongVal, BufferPtr ), TDLib
*                        D0,      A0,          A6
*
*   FUNCTION
*       This routine encodes data using the AMIGA0.0 Data encoding
*       scheme.  The routine is much like TDMfmEncode, but is does
*       not use the blitter to do its work.
*
*   INPUTS
*       LongVal - the long number to be encoded.
*
*       BufferPtr - a word pointer into the track buffer.
*
*
*   EXCEPTIONS
*
*   SEE ALSO
*       TDMfmSecEncode, TDMfmEncode, TDMfmLongDecode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************

TDMfmLongEncode:
            MOVEM.L D2-D3,-(SP)

            MOVE.L  D0,D3

	;-- HalfEncode sets it's leading bit accordingly
            LSR.L   #1,D0                   ; data bits in D0
            BSR.S     MfmHalfEncode

            MOVE.L  D3,D0
            BSR.S     MfmHalfEncode

*           ;-- now fix up the following word -- A0 already points to it
	    BSR     TDMfmFixBit

            MOVEM.L (SP)+,D2-D3
            RTS




MfmHalfEncode:

            AND.L   #$055555555,D0
            MOVE.L  D0,D2
            EOR.L   #$055555555,D2
            MOVE.L  D2,D1
            LSL.L   #1,D2
            LSR.L   #1,D1
            BSET    #31,D1
            AND.L   D2,D1

*           ;-- recombine to get an mfm encoded longword
            OR.L    D1,D0

*           ;-- go back and fix up the MSB (if needed)
            BTST    #0,-1(A0)
            BEQ.S   HalfEncode_10

*           ;-- unset MSB
            BCLR    #31,D0

HalfEncode_10:

            MOVE.L  D0,(A0)+

            RTS


*****i* trackdisk.device/interal/TDMfmLongDecode *********************
*
*   NAME
*       TDMfmLongDecode - Decode data in AMIGA0.0 Mfm Format by hand
*
*   SYNOPSIS
*       LongVal = TDMfmLongDecode( BufferPtr ), TDLib
*       D0,                        A0,          A6
*
*   FUNCTION
*       This routine decodes data using the AMIGA0.0 Data encoding
*       scheme.  The routine is much like TDMfmDecode, but is does
*       not use the blitter to do its work.
*
*   INPUTS
*       BufferPtr - a word pointer into the track buffer.
*
*   OUTPUTS
*       LongVal - the long number after decoding.
*
*   EXCEPTIONS
*
*   SEE ALSO
*       TDMfmSecDecode, TDMfmDecode, TDMfmLongEncode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*       A1 is UNCHANGED!!!
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************


TDMfmLongDecode:
	    MOVE.L  #$055555555,D0	    ; mask for clock bits
	    MOVE.L  D0,D1
            AND.L   (A0)+,D0                ; get odd bits
            AND.L   (A0)+,D1                ; get even bits
            LSL.L   #1,D0                   ; get odd bits in right pos
            OR.L    D1,D0                   ; combine bits
            RTS



*****i* trackdisk.device/interal/TDMfmSumBuffer **********************
*
*   NAME
*       TDMfmSumBuffer - Checksum a buffer's worth of data
*
*   SYNOPSIS
*       Checksum = TDMfmSumBuffer( Size, BufferPtr ), TDLib
*       D0,                        D1:W,   A0,          A6
*
*   FUNCTION
*       This routine computes a checksum of an MFM sector buffer.
*       The checksum is computed after the sector has been encoded.
*       Currently the checksum is a longword by longword exclusive OR.
*
*   INPUTS
*	Size - size (in bytes) of buffer to be summed.  Must be an
*		integral number of LONGWORDS -- eg lower two bits must be
*		null.
*       BufferPtr - a long word pointer into the track buffer.
*
*   OUTPUTS
*       Checksum - the checksum of the buffer.
*
*   EXCEPTIONS
*
*   SEE ALSO
*       TDMfmSecDecode, TDMfmDecode, TDMfmLongEncode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************


TDMfmSumBuffer:
            MOVE.L D2,-(SP)

	    LSR.W   #2,D1
	    SUBQ.W  #1,D1
            MOVEQ   #0,D0

SumBuf_00
            MOVE.L  (A0)+,D2
            EOR.L   D2,D0		; stupid non-orthoganalities
            DBF     D1,SumBuf_00

	    AND.L   #$55555555,D0

            MOVE.L (SP)+,D2
            RTS


*****i* trackdisk.device/interal/TDMfmSumBufferDecode **********************
*
*   NAME
*       TDMfmSumBufferDecode - Checksum a buffer's worth of data and xfer it
*
*   SYNOPSIS
*       Checksum = TDMfmSumBufferDecode( Size, BufferPtr, DestPtr ), TDLib
*       D0,                              D1:W,   A0,        A1,       A6
*
*   FUNCTION
*       This routine computes a checksum of an MFM sector buffer.
*       The checksum is computed after the sector has been encoded.
*       Currently the checksum is a longword by longword exclusive OR.
*	While computing the checksum, the data is decoded and moved
*	into the destination.
*
*   INPUTS
*	Size - size (in bytes) of buffer to be summed.  Must be an
*		integral number of LONGWORDS -- eg lower two bits must be
*		null.
*       BufferPtr - a long word pointer into the track buffer.
*
*	DestPtr - the destination for the decoded data
*
*   OUTPUTS
*       Checksum - the checksum of the buffer.
*
*   EXCEPTIONS
*
*   SEE ALSO
*       TDMfmSecDecode, TDMfmDecode, TDMfmLongEncode,TDMfmSumBuffer
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************


TDMfmSumBufferDecode:
            MOVEM.L D2/D3/D4/A2,-(SP)

	    LSR.W   #1,D1		; half the buffer
	    LEA     0(A0,D1.W),A2	; points to even bits
	    LSR.W   #2,D1		; # of longwords (for half)
	    SUBQ.W  #1,D1		; DBRA entered at top
	    MOVE.L  #$55555555,D4	; mask for clock bits
            MOVEQ   #0,D0		; initial checksum

SumBufDec_00			; first we grab the odd bits
            MOVE.L  (A0)+,D2
	    MOVE.L  (A2)+,D3	; then the even bits
            EOR.L   D2,D0
	    EOR.L   D3,D0	; add into checksum
	    AND.L   D4,D2
	    AND.L   D4,D3	; mask off clock bits
	    ASL.L   #1,D2	; allign &
	    OR.L    D3,D2	;  combine them
	    MOVE.L  D2,(A1)+	; and store in dest
            DBRA    D1,SumBufDec_00

	    AND.L   D4,D0	; mask clock bits of checksum

            MOVEM.L (SP)+,D2/D3/D4/A2
            RTS

*****i* trackdisk.device/interal/TDMfmCPUDecode **********************
*
*   NAME
*       TDMfmCPUDecode - Xfer a sector using the CPU (no checksum check)
*
*   SYNOPSIS
*       TDMfmCPUDecode( Size, DestPtr, BufferPtr ), TDLib
*                       D1:W,   A0,        A1,       A6
*
*   FUNCTION
*	THis function decodes and copies a sector into a destination using
*	the CPU only.  Note: the input regs are _different_ than
*	TDMfmSunCheckDecode!
*
*   INPUTS
*	Size - size (in bytes) of destination buffer.  Must be a multiple
*	       of 4.
*       BufferPtr - a long word pointer into the track buffer.
*
*	DestPtr - the destination for the decoded data
*
*   OUTPUTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*       TDMfmSecDecode, TDMfmDecode, TDMfmLongEncode,TDMfmSumBuffer,
*	TDMfmSumBufferDecode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*
**********************************************************************


TDMfmCPUDecode:
            MOVEM.L D2/D3/A2,-(SP)
					; d1 is size of DEST (1/2 size src)
	    LEA     0(A1,D1.W),A2	; points to even bits
	    LSR.W   #2,D1		; # of longwords (for half)
	    SUBQ.W  #1,D1		; DBRA entered at top
	    MOVE.L  #$55555555,D0	; mask for clock bits

CPUDec_00			; first we grab the odd bits
            MOVE.L  (A1)+,D2
	    MOVE.L  (A2)+,D3	; then the even bits
	    AND.L   D0,D2
	    AND.L   D0,D3	; mask off clock bits
	    ASL.L   #1,D2	; allign &
	    OR.L    D3,D2	;  combine them
	    MOVE.L  D2,(A0)+	; and store in dest
            DBRA    D1,CPUDec_00

            MOVEM.L (SP)+,D2/D3/A2
            RTS


*****i* trackdisk.device/interal/TDMfmFixBit *************************
*
*   NAME
*       TDMfmFixBit - Fixup an mfm word's clock bit after a memory change
*
*   SYNOPSIS
*       TDMfmFixBit( BufferPtr ), TDLib
*                    A0,          A6
*
*   FUNCTION
*       This routine fixes up memory after the mfm buffer has been changed.
*       It ensures that the most significant bit (the clock bit) of the
*       indicated byte is correctly set for the current MFM data bits around
*	it.
*
*	This routine does not change A0, A1 and D1
*
*   INPUTS
*       BufferPtr - a byte pointer into the track buffer.
*
*   OUTPUTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*       TDMfmSecDecode, TDMfmDecode, TDMfmLongEncode
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*       A6 -- TDLib
*
*

TDMfmFixBit:
		MOVE.B	(A0),D0

*		;------ see if the previous bit is a one
		BTST	#0,-1(A0)
		BNE.S	FixBit_DataOne

*		;------ the previous bit (a data bit) is zero.  See if
*		;------ we need to set the clock bit
		BTST	#6,D0
		BNE.S	FixBit_End

*		;------ While the clock bit might already be set, it
*		;------ does not hurt to set it again
		BSET	#7,D0
		BRA.S	FixBit_Write

FixBit_DataOne:
*		;------ While the clock bit might already be cleared, it
*		;------ does not hurt to clear it again
		BCLR	#7,D0

FixBit_Write:
		MOVE.B	D0,(A0)
FixBit_End:
		RTS

	END
