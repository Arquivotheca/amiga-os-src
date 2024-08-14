/* -----------------------------------------------------------------------
 * vtf.h		handshake_src
 *
 * $Locker:  $
 *
 * $Id: vtf.h,v 1.1 91/05/10 14:03:33 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/vtf.h,v 1.1 91/05/10 14:03:33 bj Exp $
 *
 * $Log:	vtf.h,v $
 * Revision 1.1  91/05/10  14:03:33  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*
    VTF.H
    File Transfer Header file
    ERH
*/

#define XON_XOFF        0
#define LINE_BY_LINE    1
#define XMODEM          2
#define XMODEM_CRC      3
#define YMODEM          4
#define YMODEM_BATCH    5
#define KERMIT          6
#define KERMIT_7BIT     7
#define KERMIT_TEXT     8
#define XPROTOCOL       9

#define XMODEM_HEADER_SIZE  3
#define XMODEM_BLOCK_SIZE   128
#define XMODEM_CHECK_SIZE   1
#define XMODEM_CRC_SIZE     2

#define BLOCKSIZE       1024
#define MAX_RETRIES     10
#define OUTGOING        RESET
#define INCOMING        SET

/*
    Below is the definition of the structure of the File Transfer
    Control Block (FTCB).
*/

typedef struct {
    long            fh,         /* File handle of associated file        */
                    sv_fh;

    unsigned short  blocksize,  /* Total no. of data bytes in block.     */
                    hdrsize,    /* Total no. of bytes in block header.   */
                    checksize,  /* Total no. of bytes in CRC/Checksum.   */
                    size,       /* Size of current block from file       */
                    blockno,    /* Block Number                          */
                    retries,    /* No of retries                         */
                    total_errors;/* Total number of errors in transfer   */

    unsigned char   sv_parity,  /* Saved parity                          */
                    sv_stpbits, /* Saved Stop bits                       */
                    sv_bpc,     /* Saved bits per character              */
                    protocol,   /* Protocol of transfer                  */
                    crcmode,    /* CRC mode indicator                    */
                    chksum,     /* Checksum                              */
                    low,        /* Low byte of CRC                       */
                    high,       /* High byte of CRC                      */
                    trans_act,  /* Flag to indicate that a file is open  */
                    *block,     /* Pointer to block from file            */
                    *ptr,       /* Pointer into current block            */
                    *dta;       /* Pointer to disk transfer area         */
    } FTCB;

#define CRC(data,crc) (crctbl[((crc>>8)^data)&255]^(crc<<8))


From bj Thu May  9 20:45:07 1991
Received: by cbmvax.cbm.commodore.com (5.57/UUCP-Project/Commodore 2/8/91)
	id AA01151; Thu, 9 May 91 20:45:03 EDT
Date: Thu, 9 May 91 20:45:03 EDT
From: bj (Brian Jackson)
Message-Id: <9105100045.AA01151@cbmvax.cbm.commodore.com>
To: bj
Subject: vtf.c
Status: RO

/*
    VTF.H
    File Transfer Header file
    ERH
*/

/*$Header: HOG:Other/inet/src/c/hs/h/RCS/vtf.h,v 1.1 91/05/10 14:03:33 bj Exp $*/

#define XON_XOFF        0
#define LINE_BY_LINE    1
#define XMODEM          2
#define XMODEM_CRC      3
#define YMODEM          4
#define YMODEM_BATCH    5
#define KERMIT          6
#define KERMIT_7BIT     7
#define KERMIT_TEXT     8
#define XPROTOCOL       9

#define XMODEM_HEADER_SIZE  3
#define XMODEM_BLOCK_SIZE   128
#define XMODEM_CHECK_SIZE   1
#define XMODEM_CRC_SIZE     2

#define BLOCKSIZE       1024
#define MAX_RETRIES     10
#define OUTGOING        RESET
#define INCOMING        SET

/*
    Below is the definition of the structure of the File Transfer
    Control Block (FTCB).
*/

typedef struct {
    long            fh,         /* File handle of associated file        */
                    sv_fh;

    unsigned short  blocksize,  /* Total no. of data bytes in block.     */
                    hdrsize,    /* Total no. of bytes in block header.   */
                    checksize,  /* Total no. of bytes in CRC/Checksum.   */
                    size,       /* Size of current block from file       */
                    blockno,    /* Block Number                          */
                    retries,    /* No of retries                         */
                    total_errors;/* Total number of errors in transfer   */

    unsigned char   sv_parity,  /* Saved parity                          */
                    sv_stpbits, /* Saved Stop bits                       */
                    sv_bpc,     /* Saved bits per character              */
                    protocol,   /* Protocol of transfer                  */
                    crcmode,    /* CRC mode indicator                    */
                    chksum,     /* Checksum                              */
                    low,        /* Low byte of CRC                       */
                    high,       /* High byte of CRC                      */
                    trans_act,  /* Flag to indicate that a file is open  */
                    *block,     /* Pointer to block from file            */
                    *ptr,       /* Pointer into current block            */
                    *dta;       /* Pointer to disk transfer area         */
    } FTCB;

#define CRC(data,crc) (crctbl[((crc>>8)^data)&255]^(crc<<8))

/** end **/


