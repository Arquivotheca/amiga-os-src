#ifndef JANUS_HARDDISK_H
#define JANUS_HARDDISK_H

/*************************************************************************
 * (PC Side File)
 *
 * HardDisk.h -- Structures for using the PC hard disk with JSERV_HARDISK
 *
 * Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
 *
 * 7-15-88 - Bill Koester - Modified for self inclusion of required files
 **************************************************************************/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif


/* disk request structure for raw amiga access to 8086's disk
   goes directly to PC BIOS (via PC int 13 scheduler) */

struct HDskAbsReq {

   UWORD    har_FktCode;   /* bios function code (see ibm tech ref)  */
   UWORD    har_Count;     /* sector count                           */
   UWORD    har_Track;     /* cylinder #                             */
   UWORD    har_Sector;    /* sector #                               */
   UWORD    har_Drive;     /* drive                                  */
   UWORD    har_Head;      /* head                                   */
   UWORD    har_Offset;    /* offset of buffer in MEMF_BUFFER memory */
   UWORD    har_Status;    /* return status                          */
   };


/* definition of an AMIGA disk partition.  returned by info function */

struct HDskPartition {

   UWORD    hdp_Next;      /* 8088 ptr to next part.  0 -> end of list   */
   UWORD    hdp_BaseCyl;   /* cyl # where partition starts               */
   UWORD    hdp_EndCyl;    /* last cyclinder # of this partition         */
   UWORD    hdp_DrvNum;    /* DOS drive number (80H, 81H, ...)           */
   UWORD    hdp_NumHeads;  /* number of heads for this drive             */
   UWORD    hdp_NumSecs;   /* number of sectors per track for this drive */
   };


/* disk request structure for higher level amiga disk request to 8086    */

struct HardDskReq {

   UWORD    hdr_Fnctn;     /* function code (see below)                  */
   UWORD    hdr_Part;      /* partition number (0 is first partition)    */
   ULONG    hdr_Offset;    /* byte offset into partition                 */
   ULONG    hdr_Count;     /* number of bytes to transfer                */
   UWORD    hdr_BufferAddr;/* offset into MEMF_BUFFER memory for buffer  */
   UWORD    hdr_Err;       /* return code, 0 if all OK                   */
   };


/* function codes for HardDskReq hdr_Fnctn word                         */

#define HDR_FNCTN_INIT  0  /* given nothing, sets adr_Part to #partitions*/
#define HDR_FNCTN_READ  1  /* given partition, offset, count, buffer     */
#define HDR_FNCTN_WRITE 2  /* given partition, offset, count, buffer     */
#define HDR_FNCTN_SEEK  3  /* given partition, offset                    */
#define HDR_FNCTN_INFO  4  /* given part, buff adr, cnt, copys in a      */
                           /* DskPartition structure. cnt set to actual  */
                           /* number of bytes copied.                    */


/* error codes for hdr_Err, returned in low byte                         */

#define HDR_ERR_OK         0  /* no error                                */
#define HDR_ERR_OFFSET     1  /* offset not on sector boundary           */
#define HDR_ERR_COUNT      2  /* dsk_count not a multiple of sector size */
#define HDR_ERR_PART       3  /* partition does not exist                */
#define HDR_ERR_FNCT       4  /* illegal function code                   */
#define HDR_ERR_EOF        5  /* offset past end of partition            */
#define HDR_ERR_MULPL      6  /* multiple calls while pending service    */
#define HDR_ERR_FILE_COUNT 7  /* too many open files                     */

/* error condition from IBM-PC BIOS, returned in high byte               */

#define HDR_ERR_SENSE_FAIL       0xff
#define HDR_ERR_UNDEF_ERR        0xbb
#define HDR_ERR_TIME_OUT         0x80
#define HDR_ERR_BAD_SEEK         0x40
#define HDR_ERR_BAD_CNTRLR       0x20
#define HDR_ERR_DATA_CORRECTED   0x11  /* data corrected                 */
#define HDR_ERR_BAD_ECC          0x10
#define HDR_ERR_BAD_TRACK        0x0b
#define HDR_ERR_DMA_BOUNDARY     0x09
#define HDR_ERR_INIT_FAIL        0x07
#define HDR_ERR_BAD_RESET        0x05
#define HDR_ERR_RECRD_NOT_FOUND  0x04
#define HDR_ERR_BAD_ADDR_MARK    0x02
#define HDR_ERR_BAD_CMD          0x01


#endif /* End of JANUS_HARDDISK_H conditional compilation                */
