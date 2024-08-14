#ifndef JANUS_PCDISK_H
#define JANUS_PCDISK_H

/************************************************************************
 * (PC Side File)
 *
 * PCDisk.h - PCDisk specific data structures
 *
 * 7-15-88 - Bill Koester - Modified for self inclusion of required files
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define PCDISK_BUFFER_SIZE 8704

/* Disk request structure for higher level Amiga file request from 8086 */

struct AmigaDiskReq {
   UWORD adr_Fnctn;              /* Function code (see below)           */
   UWORD adr_File;               /* File number (filehandle for PC side)*/
   UWORD adr_Offset_h;           /* High byte of offset into file       */
   UWORD adr_Offset_l;           /* Low  byte of offset into file       */
   UWORD adr_Count_h;            /* High byte of # of bytes to transfer */
                                 /* Actual number transfered on return  */
   UWORD adr_Count_l;            /* Low byte as above                   */
   UWORD adr_BufferOffset;       /* Offset into MEMF_BUFFER memory      */
   UWORD adr_Err;                /* Return code (see below) 0 if all OK */
};

/* Function codes for AmigaDiskReq adr_Fnctn word                       */

#define ADR_FNCTN_INIT     0     /* Currently not used                  */
#define ADR_FNCTN_READ     1     /* Given file, offset, count, buffer   */
#define ADR_FNCTN_WRITE    2     /* Given file, offset, count, buffer   */
#define ADR_FNCTN_SEEK     3     /* Given file, offset                  */
#define ADR_FNCTN_INFO     4     /* Currently not used                  */
#define ADR_FNCTN_OPEN_OLD 5     /* Given ASCIIZ filespec in buffer     */
#define ADR_FNCTN_OPEN_NEW 6     /* Given ASCIIZ filespec in buffer     */
#define ADR_FNCTN_CLOSE    7     /* Given file                          */
#define ADR_FNCTN_DELETE   8     /* Given ASCIIZ filespec in buffer     */

/* Error codes for adr_Err, returned in low byte                        */

#define ADR_ERR_OK         0     /* No error                            */
#define ADR_ERR_OFFSET     1     /* Not used                            */
#define ADR_ERR_COUNT      2     /* Not used                            */
#define ADR_ERR_FILE       3     /* File does not exist                 */
#define ADR_ERR_FNCTN      4     /* Illegal function code               */
#define ADR_ERR_EOF        5     /* Offset past end of file             */
#define ADR_ERR_MULPL      6     /* Not used                            */
#define ADR_ERR_FILE_COUNT 7     /* Too many open files                 */
#define ADR_ERR_SEEK       8     /* Seek error                          */
#define ADR_ERR_READ       9     /* Read went wrong                     */
#define ADR_ERR_WRITE      10    /* Write error                         */
#define ADR_ERR_LOCKED     11    /* File is locked                      */

#endif   /* End of JANUS_PCDISK_H conditional compilation               */
