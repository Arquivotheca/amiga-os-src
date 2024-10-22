/*** photocd.c ***************************************************************
 *
 *  $Id: photocd.c,v 40.3 94/01/13 11:38:55 jjszucs Exp $
 *  photocd.library
 *  Commonly-Used Photo CD Functions Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
$Log:	photocd.c,v $
 * Revision 40.3  94/01/13  11:38:55  jjszucs
 * Changed structure tag of Photo CD library base to PhotoCDLibrary
 * Eliminated (bogus, IMHO) warnings caused by assigning ~0 to
 *     UWORD values
 * 
 * Revision 40.2  93/11/18  20:00:52  jjszucs
 * Added RCS id and log substitions
 *
*/

/*
 *  ANSI includes
 */

#include <stdio.h>

/*
 *  Amiga includes
 */

#include <exec/types.h>

#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/*
 *  Local includes
 */

#include "photocd.h"
#include "photocdbase.h"

#include "internal.h"

/****************************************************************************
 *                                                                          *
 *  DatePhotoCDToAmiga()    -   convert Photo CD date/time format to        *
 *                              AmigaDOS date stamp                         *
 *                                                                          *
 ****************************************************************************/

void DatePhotoCDToAmiga(ULONG photoCDStamp,struct DateStamp *pDateStamp)
{

    /* Conversion from Photo CD date/time format (offset from 00:00:00UTC 1 Jan 1970)
        to AmigaDOS date/time format (offset from 00:00:00UTC 1 Jan 1978) includes
        adjustment for two leap years (1972 and 1976) */

    pDateStamp->ds_Days=
        (photoCDStamp/(HOURS_PER_DAY*MINUTES_PER_HOUR*SECONDS_PER_MINUTE))
        -(DAYS_PER_YEAR*6+DAYS_PER_LEAP_YEAR*2);
    pDateStamp->ds_Minute=
        (photoCDStamp%HOURS_PER_DAY*MINUTES_PER_HOUR*SECONDS_PER_MINUTE)/SECONDS_PER_MINUTE;
    pDateStamp->ds_Tick=
        (photoCDStamp%SECONDS_PER_MINUTE)*TICKS_PER_SECOND;

}

/****************************************************************************
 *                                                                          *
 *  decodeResOrd()      -   convert Photo CD resolution order code          *
 *                              photocd.library resolution code             *
 *                                                                          *
 ****************************************************************************/

UBYTE decodeResOrd(UBYTE resolutionOrder)
{

    UBYTE resolutionCode;

    switch ((resolutionOrder&resOrdMinMask)>>resOrdMinBitPos) {

        case resOrdBase:
            resolutionCode=PHOTOCD_RES_BASE;
            break;

        case resOrd4Base:
            resolutionCode=PHOTOCD_RES_4BASE;
            break;

        case resOrd16Base:
            resolutionCode=PHOTOCD_RES_16BASE;
            break;

        default:
            resolutionCode=PHOTOCD_RES_BAD;
            break;

    }

    return resolutionCode;

}

/****************************************************************************
 *                                                                          *
 *  unpad() -   remove trailing blanks from fixed-length padded string      *
 *                                                                          *
 *  string must have sufficent space for cbString + 1 bytes.                *
 *  cbString is length of fixed-length field.                               *
 *                                                                          *
 *  As an example, if string contains a 8-byte fixed-length padded          *
 *  string, cbString would be 8 and string must be a 9-byte  memory         *
 *  block.                                                                  *
 *                                                                          *
 ****************************************************************************/

void unpad(char *string,int cbString)
{

    char *pThisChar;
    int index;

    /* Begin at end of fixed-length string;
       index is always one character ahead */
    index=cbString;
    pThisChar=&(string[index-1]);

    /* Loop until non-space character or start of fixed-length string */
    while (*(pThisChar--)==' ' && (index--)>0) {
        ;   /* No-op */
    }

    /* Write NUL terminator */
    string[index]='\0';

}

/****************************************************************************
 *                                                                          *
 *  openPCDImage() - open Photo CD image file on Photo CD disc              *
 *                                                                          *
 ****************************************************************************/

BPTR openPCDImage(struct PhotoCDLibrary *PhotoCDBase,struct PhotoCDHandle *pHandle,UWORD imageNumber)
{

    BPTR oldCurrentDir;
    BPTR imageHandle;

    char filename[cbImagePath+1];

    /* Change current directory to Photo CD source */
    oldCurrentDir=CurrentDir(pHandle->sourceLock);

    /* Construct filename */
    bsprintf(filename,imagePathFmt,imageNumber);

    /* Open image file */
    imageHandle=Open(filename,MODE_OLDFILE);

    /* Restore current directory */
    CurrentDir(oldCurrentDir);

    /* Return image file handle; NULL if failure */
    return imageHandle;

}

/****************************************************************************
 *                                                                          *
 *  readMRS() - read Photo CD Microcontroller-Readable Sectors (MRS)        *
 *              data                                                        *
 *                                                                          *
 *  N.B.: This function does not bounds-check. Caveat caller!               *
 *                                                                          *
 ****************************************************************************/

BOOL readMRSSector(struct PhotoCDLibrary *PhotoCDBase,BPTR fileHandle,UWORD nSector,void *pBuffer,ULONG length)
{

    char *pSector;
    char *pSrc, *pDest;
    int index;
    BOOL success;

    success=TRUE;

    /* Allocate sector buffer */
    pSector=AllocMem(MRS_SECTOR_SIZE,NULL);
    if (pSector) {

        /* Seek to sector */
        if (Seek(fileHandle,nSector*MRS_SECTOR_SIZE,OFFSET_BEGINNING)!=-1L) {

            /* Read sector */
            if (Read(fileHandle,pSector,MRS_SECTOR_SIZE)==MRS_SECTOR_SIZE) {

                pSrc=pSector+MRS_HEADER_SIZE;
                pDest=pBuffer;

                for (index=0;index<length;index++) {
                    *(pDest++)=*pSrc;
                    pSrc+=MRS_NBYTE_SIZE;
                }

            } else {

                success=FALSE;

            }

        } else {

            success=FALSE;

        }

        FreeMem(pSector,MRS_SECTOR_SIZE);

    } else {

         success=FALSE;

    }

    return success;

}
