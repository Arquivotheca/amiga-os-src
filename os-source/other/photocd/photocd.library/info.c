/*** info.c *****************************************************************
 *
 *  $Id: info.c,v 40.4 94/01/13 11:38:36 jjszucs Exp $
 *
 *  photocd.library
 *  Information Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
$Log:	info.c,v $
 * Revision 40.4  94/01/13  11:38:36  jjszucs
 * Changed structure tag of Photo CD library base to PhotoCDLibrary
 * Eliminated (bogus, IMHO) warnings caused by assigning ~0 to
 *     UWORD values
 * 
 * Revision 40.3  93/11/19  17:41:57  jjszucs
 * Down-coded pixel copy and YCC-to-RGB conversion to assembly language
 *
 * Revision 40.2  93/11/18  20:00:27  jjszucs
 * Added RCS id and log substitions
 *
*/

/*
 *  Amiga includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*
 *  ANSI includes
 */

#include <ctype.h>
#include <string.h>

/*
 *  Local includes
 */

#include "photocd.h"
#include "photocdbase.h"

#include "internal.h"

/*
 *  Local constants
 */

#define nPhotoCDInfoTags 64 /* Maximum number of tags (including terminator) */

/*
 *  Local functions
 */

/****** photocd.library/IsPhotoCD ******************************************
*
*   NAME
*       IsPhotoCD()     -   determine if a Photo CD disc is present
*
*   SYNOPSIS
*       BOOL IsPhotoCD(void);
*
*       fPhotoCD=IsPhotoCD();
*
*   FUNCTION
*       Determine if a Photo CD is present in the CD-ROM drive
*       (device CD0:).
*
*   INPUTS
*       None
*
*   RESULT
*       fPhotoCD            TRUE if a Photo CD disc is present;
*                           false if no disc is present or the
*                           disc is not in Photo CD format
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

BOOL ASM IsPhotoCD(REG (a6) struct PhotoCDLibrary *PhotoCDBase)
{

    BPTR fileHandle;

    UBYTE discSignature[cbDiscSignature];

    BOOL fIsPhotoCD;

    static UBYTE photoCDSignature[cbDiscSignature] ={
        'P', 'H', 'O', 'T', 'O', '_', 'C', 'D',
    };

#ifdef DEBUG
    kprintf("IsPhotoCD();\n");
    kprintf("   Entry\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
#endif /* DEBUG */

    /* Assume disc is not Photo CD */
    fIsPhotoCD=FALSE;

    /* Open disc information file */
    if (fileHandle=Open("CD0:PHOTO_CD/INFO.PCD",MODE_OLDFILE)) {

        /* Read disc signature */
        if (Read(fileHandle,discSignature,cbDiscSignature)==cbDiscSignature) {

            /* Verify signature */
            if (memcmp(discSignature,photoCDSignature,cbDiscSignature)==0) {
                fIsPhotoCD=TRUE;
            }

        }

        Close(fileHandle);

    }

    /* Return */
#ifdef DEBUG
    kprintf("   Return\n");
    kprintf("   fIsPhotoCD=%ld\n",(int) fIsPhotoCD);
#endif /* DEBUG */
    return fIsPhotoCD;

}

/****** photocd.library/ObtainPhotoCDInfo *********************************
*
*   NAME
*       ObtainPhotoCDInfo()     -   obtain Photo CD information
*
*   SYNOPSIS
*       struct TagItem *ObtainPhotoCDInfo(void *pcdHandle,Tag firstTag,...);
*       info=ObtainPhotoCDInfo(pcdHandle,firstTag,...);
*
*       struct TagItem *ObtainPhotoCDInfoA(void *pcdHandle,struct TagItem *tags);
*       info=ObtainPhotoCDInfoA(pcdHandle,tags);
*
*   FUNCTION
*       Obtain Photo CD disc, session, or image information.
*
*   INPUTS
*       pcdHandle       Photo CD handle returned by OpenPhotoCD(); this may
*                       be NULL for PCD_File queries only
*
*       tags            Tag array containing additional parameters
*
*   INPUT TAGS
*       All tags are optional unless otherwise indicated.
*
*       PCD_ErrorCode       (ULONG *) Pointer to ULONG where error code
*                           (one of PHOTOCD_ERR_*) is deposited if this
*                           function fails. This variable is unchanged
*                           if the operation is successful.
*
*       PCD_Disc            If this tag is present, obtain information on
*                           Photo CD disc. This is the default and is
*                           mutually exclusive with PCD_Session and PCD_Image.
*                           The PCDDisc_* result tags are returned
*                           in response to this query.
*
*       PCD_Session         (UBYTE) If this tag is present, obtain information on
*                           Photo CD session whose cardinal session number
*                           is specified as the tag value. This tag is
*                           mutually exclusive with PCD_Disc, PCD_Image, and
*                           PCD_File. The PCDSess_* result tags are returned
*                           in response to this query.
*
*       PCD_Image           (UWORD) If this tag is present, obtain information on
*                           the image whose cardinal image number is specified
*                           as the tag value. This tag is mutually exclusive
*                           with PCD_Disc, PCD_Session, and PCD_File. The
*                           PCDImg_* result tags are returned in response to
*                           this query.
*
*       PCD_File            (BPTR) If this tag is present, obtain information on
*                           the Photo CD image file whose file handle is specified
*                           as the tag value. This tag is mutually exclusive
*                           with PCD_Disc, PCD_Image, and PCD_Session. The
*                           PCDImg_* result tags are returned in response to
*                           this query.
*
*   RESULT
*       info                Tag array containing Photo CD disc, session,
*                           or image information (as requested); NULL if failure
*
*   RESULT TAGS
*       All tags are always returned unless otherwise indicated.
*
*       For PCD_Disc
*       ============
*
*       PCDDisc_Signature   (STRPTR) Null-terminated disc signature.
*                           The normal disc signature is "PHOTO_CD".
*
*       PCDDisc_Version     (UBYTE) Specification version number
*                           (also known as major version number).
*
*       PCDDisc_Revision    (UBYTE) Specification revision number
*                           (also known as minor version number).
*
*       PCDDisc_SerNo       (STRPTR) Disc serial number.
*
*       PCDDisc_CreateStamp (struct DateStamp *) Disc creation date and
*                           time stamp (UTC).
*
*       PCDDisc_ModifyStamp (struct DateStamp *) Disc modification date and
*                           time stamp (UTC).
*
*       PCDDisc_nImages     (UWORD) Number of image packs on disc.
*
*       PCDDisc_IntrlvADPCM (UBYTE) Number of ADPCM audio sectors per
*                           interleaved group. If this tag is not returned
*                           the disc does not contain any ADPCM audio,
*                           unless PCDDisc_IntrlvImage is also not returned.
*                           For more information on this case, see the
*                           description of PCDDisc_IntrlvImage.
*
*       PCDDisc_IntrlvImage (UBYTE) Number of image data sectors per
*                           interleaved group. If this tag is not returned,
*                           the interleave ratio is specified individually
*                           for each image and the PCDImg_IntrlvADPCM
*                           and PCDImg_IntrlvImage tags returned by
*                           ObtainPCDImageInfo() must be used to determine
*                           interleave ratio for each image.
*
*       PCDDisc_MinRes      (UBYTE) Lowest image resolution occuring on this
*                           disc. This is one of PHOTOCD_RES_*, defined in
*                           libraries/photocd.h.
*
*       PCDDisc_MaxRes      (UBYTE) Highest image resolution occuring on this
*                           disc. This is one of PHOTOCD_RES_*, defined in
*                           libraries/photocd.h.
*
*       PCDDisc_LeadoutStart    (ULONG) Start of final lead-out area on this
*                               disc. This is a cd.device/RMSF structure
*                               packed as a 32-bit unsigned long word.
*
*       PCDDisc_nSessions   (UBYTE) Total number of sessions on disc.
*
*       For PCD_Session
*       ===============
*       PCDSess_nImages     (UWORD) Number of images recorded in this
*                           session.
*
*       PCDSess_CDDAStart   (ULONG) Start of first CD-DA track in this
*                           session. This is a cd.device/struct RMSF
*                           packed as a 32-bit unsigned long word.
*
*       PCDSess_LeadoutStart (ULONG) Start of lead-out area of this
*                           session. This is a cd.device/struct RMSF
*                           packed as a 32-bit unsigned long word.
*
*       PCDSess_WrtrVndr    (STRPTR) Vendor identification of writer
*                           device used to write this session. If this
*                           information is not available, this tag
*                           is not returned.
*
*       PCDSess_WrtrProd    (STRPTR) Product identification of writer
*                           device used to write this session. This
*                           tag is not returned if this information
*                           is not available.
*
*       PCDSess_WrtrVer     (UBYTE) Version number (also known as
*                           major version number) of writer device
*                           used to write this session.
*
*       PCDSess_WrtrRev     (UBYTE) Revision number (also known as
*                           minor version number) of writer device
*                           used to write this session.
*
*       PCDSess_WrtrDate    (struct DateStamp *) Revision date
*                           of firmware of writer device used to
*                           write this session.
*
*       PCDSess_WrtrSerNo   (STRPTR) Serial number of writer device
*                           used to write this session.
*
*       PCDSess_CreateStamp (struct DateStamp *) Session creation
*                           date/time stamp.
*
*       For PCD_Image
*       =============
*
*       PCDImg_Start        (ULONG) Logical Sector Number address of first
*                           sector of Image Pack.
*
*       PCDImg_4BaseHCT     (UBYTE) 4Base Huffman Code Table, in the range
*                           1..4, corresponding to the four 4Base Huffman Code
*                           Table classes specified by the Photo CD standard.
*
*       PCDImg_IPE          (BOOL) TRUE if an Image Pack Extension (IPE)
*                           is present. FALSE if IPE is not present.
*
*       PCDImg_ResOrder     (UBYTE) Maximum resolution available for this image.
*                           One of PHOTOCD_RES_*.
*
*       PCDImg_Rotation     (WORD) Degrees of counter-clockwise rotation
*                           required to display image in intended orientation.
*
*       PCDImg_4BaseStop    (UWORD) Sector number of first sector after
*                           4Base Image Component Data, relative to start
*                           of Image Pack. This tag is not returned if the
*                           4Base ICD is not present.
*
*       PCDImg_16BaseStop   (UWORD) Sector number of first sector after 16Base
*                           Image Component Data, relative to start of
*                           Image Pack. This tag is not returned if the
*                           16Base ICD is not present.
*
*       PCDImg_IPEStop      (UWORD) Sector number of first sector after Image
*                           Pack Extension, relative to start of Image Pack.
*                           This is not returned if the IPE is not present.
*
*       PCDImg_IntrlvADPCM  (UBYTE) Number of ADPCM audio sectors per
*                           interleaved group. If this tag is not returned,
*                           no ADPCM audio is interleaved with this image.
*
*       PCDImg_IntrlvImage  (UBYTE) Number of image sectors per
*                           interleaved group.
*
*       PCDImg_PrefFast     (BOOL) If the value of this tag is TRUE, fast
*                           loading is preferred and the preferred resolution
*                           (from PCDImg_PrefRes) should be used as is. If
*                           this value is FALSE, high resolution is preferred
*                           and resolutions higher than the preferred resolution
*                           should be used if higher loading speeds (i.e., due to
*                           a double-speed CD-ROM drive) are available.
*
*       PCDImg_PrefRes      (UBYTE) Preferred resolution for single-speed
*                           (150KB/s) reader. One of PHOTOCD_RES_*.
*
*       PCDImg_MagX         (UWORD) X-axis coordinate of center of
*                           magnification area for playback.
*
*       PCDImg_MagY         (UWORD) Y-axis coordinate of center of
*                           magnification area for playback.
*
*       PCDImg_MagFactor    (UWORD) Linear magnification factor for playback.
*
*       PCDImg_DispOffX     (UWORD) X-axis coordinate on display area
*                           for center of image at playback.
*
*       PCDImg_DispOffY     (UWORD) Y-axis coordinate on display area
*                           for center of image at playback.
*
*       PCDImg_Transition   (UBYTE) Transition to use at playback.
*                           One of PHOTOCD_TRANS_*.
*
*       PCDImg_Signature    (STRPTR) Image Pack Information signature.
*                           "PCD_IPI" is the expected value.
*
*       PCDImg_SpecVer      (UBYTE) Version number (also known as major
*                           version number) of Photo CD specification to
*                           which this image conforms.
*
*       PCDImg_SpecRev      (UBYTE) Revision number (also known as minor
*                           version number) of Photo CD specification to
*                           which this image conforms.
*
*       PCDImg_PIWVer       (UBYTE) Version number (also known as major
*                           version number) of imaging workstation software
*                           used to process this image. This tag is not
*                           returned if this information is not available.
*
*       PCDImg_PIWRev       (UBYTE) Revision number (also known as minor
*                           version number) of imaging workstation software
*                           used to process this image. This tag is not
*                           returned if this information is not available.
*
*       PCDImg_16BaseMag    (UWORD) Magnification factor applied to 16Base
*                           source image in fixed-point representation with
*                           two fractional digits This tag is not returned
*                           if this information is not available.
*
*       PCDImg_ScanStamp    (struct DateStamp *) Image scanning date/time stamp.
*                           This tag is not returned if this information is
*                           not available.
*
*       PCDImg_ModifyStamp  (struct DateStamp *) Last image modification
*                           date/time stamp. This tag is not returned if this
*                           information is not available.
*
*       PCDImg_Medium       (UBYTE) Medium of source image. One of
*                           PHOTOCD_MEDIUM_*. This tag is not returned if this
*                           information is not available.
*
*       PCDImg_ProdType     (STRPTR) Product type of original image. This tag
*                           is not returned if this information is not
*                           available.
*
*       PCDImg_ScnrVndr     (STRPTR) Identity of scanner vendor. This tag is
*                           not returned if this information is not available.
*
*       PCDImg_ScnrProd     (STRPTR) Identity of scanner product. This tag is
*                           not returned if this information is not available.
*
*       PCDImg_ScnrVer      (UBYTE) Version number (also known as major
*                           version number) of scanner firmware.
*
*       PCDImg_ScnrRev      (UBYTE) Revision number (also known as minor
*                           version number) of scanner firmware.
*
*       PCDImg_ScnrDate     (struct DateStamp *) Scanner firmware revision
*                           date stamp. This tag is not returned if this
*                           information is not available.
*
*       PCDImg_ScnrSerNo    (STRPTR) Serial number of scanner. This tag is
*                           not returned if this information is not available.
*
*       PCDImg_ScnrPixel    (UWORD) Pixel size of scanner, measured in microns
*                           using fixed-point representation with two fractional
*                           digits. This tag is not returned if this information
*                           is not available.
*
*       PCDImg_PIWMfgr      (STRPTR) Imaging workstation manufacturer. This tag
*                           is not returned if this information is not available.
*
*       PCDImg_PhtfinCharSet (UBYTE) Photofinisher's name character set. One of
*                           PHOTOCD_CHARSET_*. This tag is not returned if this
*                           information is not available.
*
*       PCDImg_PhtfinEscape (STRPTR) Photofinisher's name escape sequences
*                           (for ISO 2022 character set). This tag is not returned
*                           if this information is not available.
*
*       PCDImg_PhtfinName   (STRPTR) Photofinisher's name. This tag is not
*                           returned if this information is not available.
*
*       PCDImg_SBAVer       (UBYTE) Version number (also known as major
*                           version number) of scene balance algorithm.
*                           This tag is not returned if SBA is not used for
*                           the image.
*
*       PCDImg_SBARev       (UBYTE) Revision number (also known as minor
*                           version number) of scene balance algorithm.
*                           This tag is not returned if SBA is not used
*                           for this image.
*
*       PCDImg_SBACommand   (UBYTE) Scene balance algorithm command.
*                           One of PHOTOCD_SBA_*. This tag is not returned
*                           if SBA is not used for this image.
*
*       PCDImg_SBAData      (UBYTE *) Scene balance algorithm data.
*                           This data is PHOTOCD_SBA_DATASIZE (currently 100)
*                           bytes in length and is padded with zero at the end.
*                           This tag is not returned if SBA is not used for
*                           this image.
*
*       PCDImg_Copyright    (STRPTR) Filename of text file containing
*                           copyright/use rights text. This tag is not
*                           returned if copyright/use rights restrictions
*                           are not specified.
*
*   EXAMPLE
*
*   NOTES
*       Do not modify or destroy the returned tag array or
*       ReleasePhotoCDInfo() will fail.
*
*   BUGS
*
*   SEE ALSO
*       ReleasePhotoCDInfo()
*
*****************************************************************************
*
*/

struct TagItem * ASM ObtainPhotoCDInfoA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (a0) void *pcdHandle,
    REG (a1) struct TagItem *tags)
{

    struct PhotoCDHandle *pHandle;

    UBYTE session;
    UWORD image;

    struct PhotoCDDiscInfo discInfo;
    struct PhotoCDSessionInfo sessionInfo;
    struct PhotoCDIPICA ipica;
    struct PhotoCDIPI ipi;

    ULONG errorCode;
    ULONG *pErrorCode;

    struct TagItem *pResultTags;
    struct TagItem *pThisTag;

    STRPTR resultString;
    struct DateStamp *pResultDate;
    UBYTE resultResolution;

    BPTR imageHandle, intrnlImgHndl;

    /*
     *  Initialization
     */

#ifdef DEBUG
    kprintf("ObtainPhotoCDInfoA():\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   pcdHandle=$%08lx\n",pcdHandle);
    kprintf("   tags=$%08lx\n",tags);
#endif /* DEBUG */

    /* Get Photo CD handle */
    pHandle=pcdHandle;

    /* Clear return tags, error code, and internal image handle */
    pResultTags=NULL;
    errorCode=PHOTOCD_ERR_NONE;
    intrnlImgHndl=NULL;

    /* Allocate result tag array */
    pResultTags=
        AllocVec(nPhotoCDInfoTags*sizeof(struct TagItem),NULL);
    if (pResultTags) {

        /* N.B.:
          1)    Errors during decoding are only marked.
                Actual error handling occurs in the return
                section. This avoids ridiculous levels of
                indentation or the use of a four-letter
                word (g?to) :-).

          2)    The pResultTags array, which is passed to
                ReleasePhotoCDInfo() in case of an error,
                must be valid throughout this process,
                including error conditions. Specifically:

                o   The tag array must be terminated with
                    TAG_DONE when ReleasePhotoCDInfo() is called.

                o   Tags of STRPTR or struct DateStamp * type
                    must not occur without a valid ti_Data
                    or ReleasePhotoCDInfo() will attempt to
                    free unallocated memory.

          3)    utility.library/PackStructureTags() does not
                appear readily suited to the needs of
                this code. As a result, a similar function
                is performed by hand. C'est la vie.
        */

        /* Initialize tag iterator */
        pThisTag=pResultTags;


        /*
         *  Obtain disc information
         */

        /* Handle PCD_Disc; this is default and mutually exclusive with
           PCD_Session, PCD_Image, and PCD_File */
        if (!FindTagItem(PCD_Session,tags) && !FindTagItem(PCD_Image,tags)
            && !FindTagItem(PCD_File,tags)) {

            /* Seek to Disc Descriptor */
            /* N.B.: This code tests for a -1 return from Seek(), which
                     is returned on an error (such as seeking beyond the
                     end of the file). Some filesystems (such as the
                     V36/V37 Original and Fast Filesystems) incorrectly
                     return the current position in these error conditions.
                     In addition to being desirable for other reasons,
                     the test of the Read() return value in the next
                     section of code allows this code to work on
                     filesystems that are broken in this manner. */
            if (Seek(pHandle->infoHandle,0L,OFFSET_BEGINNING)!=-1L) {

                /* Read disc descriptor */
                if (Read(pHandle->infoHandle,&discInfo,sizeof(discInfo))==sizeof(discInfo)) {

                    /* Decode Disc Signature */
                    if (resultString=AllocVec(cbDiscSignature+1,NULL)) {

                        memcpy(resultString,discInfo.signature,cbDiscSignature);
                        resultString[cbDiscSignature]='\0';

                        SetTagItem(pThisTag++,
                            PCDDisc_Signature,(ULONG) resultString);

                    } else {

                        errorCode=PHOTOCD_ERR_MEMORY;

                    }

                    /* Decode Specification Version Number */
                    SetTagItem(pThisTag++,PCDDisc_Version,
                        xferUBYTE(discInfo.specVersion));

                    /* Decode Specification Revision Number */
                    SetTagItem(pThisTag++,PCDDisc_Revision,
                        xferUBYTE(discInfo.specRevision));

                    /* Decode Disc Serial Number */
                    if (resultString=AllocVec(cbDiscSerNo+1,NULL)) {

                        memcpy(resultString,discInfo.serNo,cbDiscSerNo);
                        resultString[cbDiscSerNo]='\0';

                        SetTagItem(pThisTag++,
                            PCDDisc_SerNo,(ULONG) resultString);

                    } else {

                        errorCode=PHOTOCD_ERR_MEMORY;

                    }

                    /* Decode Disc Creation Time and Date */
                    if (pResultDate=AllocVec(sizeof(*pResultDate),NULL)) {

                        DatePhotoCDToAmiga(xferULONG(discInfo.createStamp),
                            pResultDate);

                        SetTagItem(pThisTag++,
                            PCDDisc_CreateStamp,(ULONG) pResultDate);

                    } else {

                        errorCode=PHOTOCD_ERR_MEMORY;

                    }

                    /* Decode Disc Modification Time and Date */
                    if (pResultDate=AllocVec(sizeof(*pResultDate),NULL)) {

                        DatePhotoCDToAmiga(xferULONG(discInfo.modifyStamp),
                            pResultDate);

                        SetTagItem(pThisTag++,
                            PCDDisc_ModifyStamp,(ULONG) pResultDate);

                    } else {

                        errorCode=PHOTOCD_ERR_MEMORY;

                    }

                    /* Decode Number of Image Packs on Disc */
                    SetTagItem(pThisTag++,
                        PCDDisc_nImages,xferUWORD(discInfo.nImages));

                    /* Decode Disc Interleave Ratio */
                    if (xferUBYTE(discInfo.interleave)) {

                        if (xferUBYTE(discInfo.interleave)&intrlvADPCMMask) {

                            SetTagItem(pThisTag++,
                                PCDDisc_IntrlvADPCM,
                                (xferUBYTE(discInfo.interleave)&intrlvADPCMMask)>>
                                    intrlvADPCMBitPos);

                        }

                        SetTagItem(pThisTag++,
                            PCDDisc_IntrlvImage,
                            (xferUBYTE(discInfo.interleave&intrlvImageMask))>>
                                intrlvImageBitPos);

                    }

                    /* Decode Disc Image Pack Resolution Order */
                    if ((resultResolution=
                         decodeResOrd((xferUBYTE(discInfo.resolution)&resOrdMaxMask)
                            >>resOrdMaxBitPos))
                        !=PHOTOCD_RES_BAD) {

                        SetTagItem(pThisTag++,
                            PCDDisc_MaxRes,resultResolution);

                    } else {

                        errorCode=PHOTOCD_ERR_DATA_FORMAT;

                    }

                    if ((resultResolution=
                        decodeResOrd((xferUBYTE(discInfo.resolution)&resOrdMinMask)
                            >>resOrdMinBitPos))
                            !=PHOTOCD_RES_BAD) {

                        SetTagItem(pThisTag++,
                            PCDDisc_MinRes,resultResolution);

                    } else {

                        errorCode=PHOTOCD_ERR_DATA_FORMAT;

                    }

                    /* Decode Outermost Lead-out Area Start Time */
                    SetTagItem(pThisTag++,
                        PCDDisc_LeadoutStart,
                        PackMSF(
                            BCDByte(discInfo.leadoutStart[0]),
                            BCDByte(discInfo.leadoutStart[1]),
                            BCDByte(discInfo.leadoutStart[2])
                        )
                    );

                    /* Decode Number of Sessions on Disc */
                    SetTagItem(pThisTag++,
                        PCDDisc_nSessions,xferUBYTE(discInfo.nSessions));

                } else {

                    errorCode=PHOTOCD_ERR_DOS;

                }

            } else {

                errorCode=PHOTOCD_ERR_DOS;

            }

        }

        /*
         *  Obtain session information
         */

        /* Handle PCD_Session; this is mutually exclusive with PCD_Disc,
            PCD_Image, and PCD_File */
        if ((session=GetTagData(PCD_Session,-1,tags))!=-1 &&
             !FindTagItem(PCD_Disc,tags) && !FindTagItem(PCD_Image,tags)
             && !FindTagItem(PCD_File,tags)) {

            /* Seek to session descriptor */
            /* N.B.: This code tests for a -1 return from Seek(), which
                     is returned on an error (such as seeking beyond the
                     end of the file). Some filesystems (such as the
                     V36/V37 Original and Fast Filesystems) incorrectly
                     return the current position in these error conditions.
                     In addition to being desirable for other reasons,
                     the test of the Read() return value in the next
                     section of code allows this code to work on
                     filesystems that are broken in this manner. */
            if (Seek(pHandle->infoHandle,
                sizeof(struct PhotoCDDiscInfo)+
                    (session-1)*sizeof(struct PhotoCDSessionInfo),
                OFFSET_BEGINNING)!=-1L) {

                /* Read session descriptor */
                if (Read(pHandle->infoHandle,
                    &sessionInfo,sizeof(sessionInfo))==sizeof(sessionInfo)) {

                    /* Decode Number of Image Packs in this Session */
                    SetTagItem(pThisTag++,
                        PCDSess_nImages,xferUWORD(sessionInfo.nImages));

                    /* Decode Start Time CD-DA */
                    SetTagItem(pThisTag++,
                        PCDSess_CDDAStart,
                            PackMSF(
                                BCDByte(sessionInfo.CDDAStart[0]),
                                BCDByte(sessionInfo.CDDAStart[1]),
                                BCDByte(sessionInfo.CDDAStart[2])
                            )
                        );

                    /* Decode Start Time Lead-out Area */
                    SetTagItem(pThisTag++,
                        PCDSess_LeadoutStart,
                            PackMSF(
                                BCDByte(sessionInfo.leadoutStart[0]),
                                BCDByte(sessionInfo.leadoutStart[1]),
                                BCDByte(sessionInfo.leadoutStart[2])
                            )
                        );

                    /* N.B.:
                        Although the Photo CD standard specifies that
                        optional fields (such as Writer Vendor
                        Identification) should be filled with blanks
                        (presumably meaning ' ', 32, 0x20, ASCII SPC)
                        if the information is not available, some
                        Photo CD discs use a null ('\0', 0, 0x00,
                        ASCII NUL) in this case. As a result,
                        the first character is tested against
                        both ASCII SPC and NUL to handle such
                        malformed discs.
                    */

                    /* Decode Writer Vendor Identification */
                    if (sessionInfo.writerVendor[0]!=' ' &&
                        sessionInfo.writerVendor[0]!='\0') {

                        if (resultString=AllocVec(cbWriterVendor+1,NULL)) {

                            memcpy(resultString,sessionInfo.writerVendor,cbWriterVendor);
                            unpad(resultString,cbWriterVendor);

                            SetTagItem(pThisTag++,
                                PCDSess_WrtrVndr,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* Decode Writer Product Identification */
                    if (sessionInfo.writerProduct[0]!=' ' &&
                        sessionInfo.writerProduct[0]!='\0') {

                        if (resultString=AllocVec(cbWriterProduct+1,NULL)) {

                            memcpy(resultString,sessionInfo.writerProduct,cbWriterProduct);
                            unpad(resultString,cbWriterProduct);

                            SetTagItem(pThisTag++,
                                PCDSess_WrtrProd,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* Decode Writer Firmware Revision Level;
                       format is x.yy (ASCII) */
                    if (sessionInfo.writerVersion[0]!=' ' &&
                        sessionInfo.writerVersion[0]!='\0') {

                        SetTagItem(pThisTag++,
                            PCDSess_WrtrVer,sessionInfo.writerVersion[0]-'0');

                        SetTagItem(pThisTag++,
                            PCDSess_WrtrRev,
                                (sessionInfo.writerVersion[2]-'0')*10+
                                (sessionInfo.writerVersion[3]-'0')
                            );

                    }

                    /* Decode Writer Firmware Revision Date;
                       format is MM/DD/YY (ASCII) */

                    /* N.B.: The Photo CD standard implies, although does
                             not explicitly state for all cases, that blanks
                             indicate an undefined ASCII field. However, Photo
                             CD images have been encountered which contain
                             "MM/DD/YY" for undefined ASCII-encoded date
                             fields. The test for a valid ASCII-encoded date
                             handles this case, as well as the expected use
                             of blanks to indicate an undefined value. */

                    if (isdigit(sessionInfo.writerDate[0]) &&
                        isdigit(sessionInfo.writerDate[1])) {

                        if (pResultDate=AllocVec(sizeof(*pResultDate),NULL)) {

                            /* N.B.: dos.library/StrToDate() cannot be used to
                                     convert from ASCII mm/dd/yy to struct DateStamp
                                     because this function expects the '/' character
                                     to be used as a seperator */
                            if (DateComponentToAmiga(
                                (sessionInfo.writerDate[0]-'\0')*10+(sessionInfo.writerDate[1]-'\0'),
                                (sessionInfo.writerDate[3]-'\0')*10+(sessionInfo.writerDate[4]-'\0'),
                                (sessionInfo.writerDate[6]-'\0')*10+(sessionInfo.writerDate[7]-'\0')+THIS_CENTURY,
                                0,0,0,
                                pResultDate)) {

                                SetTagItem(pThisTag++,
                                    PCDSess_WrtrDate,(ULONG) pResultDate);

                            } else {

                                FreeVec(pResultDate);
                                errorCode=PHOTOCD_ERR_DATA_FORMAT;

                            }

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* Decode Writer Serial Number */
                    if (sessionInfo.writerSerNo[0]!=' ' &&
                        sessionInfo.writerSerNo[0]!='\0') {

                        if (resultString=AllocVec(cbWriterSerNo+1,NULL)) {

                            memcpy(resultString,sessionInfo.writerSerNo,cbWriterSerNo);
                            unpad(resultString,cbWriterSerNo);

                            SetTagItem(pThisTag++,
                                PCDSess_WrtrSerNo,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* Decode Session Creation Time and Date */
                    if (pResultDate=AllocVec(sizeof(*pResultDate),NULL)) {

                        DatePhotoCDToAmiga(xferULONG(sessionInfo.createStamp),
                            pResultDate);

                        SetTagItem(pThisTag++,
                            PCDSess_CreateStamp,(ULONG) pResultDate);

                    } else {

                        errorCode=PHOTOCD_ERR_MEMORY;

                    }

                } else {

                    errorCode=PHOTOCD_ERR_DOS;

                }

            } else {

                errorCode=PHOTOCD_ERR_DOS;

            }

        }

        /*
         *  Obtain image information
         */

        /* Handle PCD_File; this is mutually exclusive with PCD_Disc,
           PCD_Session, and PCD_Image */
        imageHandle=GetTagData(PCD_File,NULL,tags);

        /* Handle PCD_Image; this is mutually exclusive with PCD_Disc,
            PCD_Session, and PCD_File */
        if ((image=GetTagData(PCD_Image,-1,tags))!=-1 &&
             !FindTagItem(PCD_Disc,tags) && !FindTagItem(PCD_Session,tags)) {

            if (!(imageHandle=intrnlImgHndl=openPCDImage(PhotoCDBase,pHandle,image))) {

                errorCode=PHOTOCD_ERR_DOS;

            }

        }

        /* If PCD_File or PCD_Image specified and file opened ... */
        if (FindTagItem(PCD_File,tags) || FindTagItem(PCD_Image,tags) &&
            imageHandle) {

            /* Read IP-ICA */
            if (readMRSSector(PhotoCDBase,imageHandle,sectorIPICA,&ipica,sizeof(ipica))) {

                /* PCDImg_4BaseHCT */
                SetTagItem(pThisTag++,
                    PCDImg_4BaseHCT,
                        (xferUBYTE(ipica.attributes)&ipica4BaseHCTMask)>>
                        ipica4BaseHCTBitPos);

                /* PCDImg_IPE */
                SetTagItem(pThisTag++,
                    PCDImg_IPE,
                        (xferUBYTE(ipica.attributes)&ipicaIPEFlag)?TRUE:FALSE);

                /* PCDImg_ResOrder */
                if ((resultResolution=decodeResOrd(
                    (xferUBYTE(ipica.attributes)&ipicaResOrdMask)>>
                        ipicaResOrdBitPos))!=PHOTOCD_RES_BAD) {

                    SetTagItem(pThisTag,
                        PCDImg_ResOrder,resultResolution);

                } else {

                    errorCode=PHOTOCD_ERR_DATA_FORMAT;

                }

                /* PCDImg_Rotation */
                SetTagItem(pThisTag++,
                    PCDImg_Rotation,
                        ((xferUBYTE(ipica.attributes)&ipicaRotateMask)>
                            ipicaRotateBitPos)*ipicaRotateIncrement);

                /* PCDImg_4BaseStop */
                if (ipica.stop4Base) {
                    SetTagItem(pThisTag++,
                        PCDImg_4BaseStop,xferUWORD(ipica.stop4Base));
                }

                /* PCDImg_16BaseStop */
                if (ipica.stop16Base) {
                    SetTagItem(pThisTag++,
                        PCDImg_16BaseStop,xferUWORD(ipica.stop16Base));
                }

                /* PCDImg_IPEStop */
                if (ipica.stopIPE) {
                    SetTagItem(pThisTag++,
                        PCDImg_IPEStop,xferUWORD(ipica.stopIPE));
                }

                /* PCDImg_IntrlvADPCM */
                if (xferUBYTE(ipica.interleave)&intrlvADPCMMask) {

                    SetTagItem(pThisTag++,
                        PCDImg_IntrlvADPCM,
                            (xferUBYTE(ipica.interleave)&intrlvADPCMMask)
                            >>intrlvADPCMBitPos);

                }

                /* PCDImg_IntrlvImage */
                SetTagItem(pThisTag++,
                    PCDImg_IntrlvImage,
                        (xferUBYTE(ipica.interleave)&intrlvImageMask)
                        >>intrlvImageBitPos);

                /* PCDImg_PrefFast */
                SetTagItem(pThisTag++,
                    PCDImg_PrefFast,
                        xferUBYTE(ipica.prefRes)&ipicaFastMask?TRUE:FALSE);

                /* PCDImg_PrefRes */
                /* N.B.: This relies on the fact that the ADPCM Image Resolution Selection
                         values correspond to the values of PHOTOCD_RES_*. */
                SetTagItem(pThisTag++,
                    PCDImg_PrefRes,
                        (xferUBYTE(ipica.prefRes)&ipicaResSelMask)>>ipicaResSelBitPos);

                /* PCDImg_MagX */
                SetTagItem(pThisTag++,
                    PCDImg_MagX,xferUBYTE(ipica.magX));

                /* PCDImg_MagY */
                SetTagItem(pThisTag++,
                    PCDImg_MagY,xferUBYTE(ipica.magY));

                /* PCDImg_MagFactor */
                SetTagItem(pThisTag++,
                    PCDImg_MagFactor,xferUBYTE(ipica.magFactor));

                /* PCDImg_DispOffX */
                SetTagItem(pThisTag++,
                    PCDImg_DispOffX,xferUBYTE(ipica.dispOffX));

                /* PCDImg_DispOffY */
                SetTagItem(pThisTag++,
                    PCDImg_DispOffY,xferUBYTE(ipica.dispOffY));

                /* PCDImg_Transition */
                SetTagItem(pThisTag++,
                    PCDImg_Transition,xferUBYTE(ipica.transition));

            } else {

                errorCode=PHOTOCD_ERR_DOS;

            }

            /* --- IPI --- */

            /* Seek to IPI */
            /* N.B.: This code tests for a -1 return from Seek(), which
                     is returned on an error (such as seeking beyond the
                     end of the file). Some filesystems (such as the
                     V36/V37 Original and Fast Filesystems) incorrectly
                     return the current position in these error conditions.
                     In addition to being desirable for other reasons,
                     the test of the Read() return value in the next
                     section of code allows this code to work on
                     filesystems that are broken in this manner. */
            if (Seek(imageHandle,offsetIPA,OFFSET_BEGINNING)!=-1L) {

                /* Read IPI */
                if (Read(imageHandle,&ipi,sizeof(ipi))==sizeof(ipi)) {

                    /* PCDImg_Signature */
                    if (resultString=AllocVec(cbIPISignature+1,NULL)) {

                        memcpy(resultString,ipi.signature,cbIPISignature);
                        resultString[cbIPISignature]='\0';

                        SetTagItem(pThisTag++,
                            PCDImg_Signature,(ULONG) resultString);

                    } else {

                        errorCode=PHOTOCD_ERR_MEMORY;

                    }

                    /* PCDImg_SpecVer */
                    SetTagItem(pThisTag++,
                        PCDImg_SpecVer,BCDByte(ipi.specVer[0]));

                    /* PCDImg_SpecRev */
                    SetTagItem(pThisTag++,
                        PCDImg_SpecRev,BCDByte(ipi.specVer[1]));

                    /* PCDImg_PIWVer */
                    if (ipi.piwVer[0]!=ipiUndef) {
                        SetTagItem(pThisTag++,
                            PCDImg_PIWVer,BCDByte(ipi.piwVer[0]));
                    }

                    /* PCDImg_PIWRev */
                    if (ipi.piwVer[1]!=ipiUndef) {
                        SetTagItem(pThisTag++,
                            PCDImg_PIWRev,BCDByte(ipi.piwVer[1]));
                    }

                    /* PCDImg_16BaseMag */
                    if (ipi.mag16Base[0]!=ipiUndef && ipi.mag16Base[1]!=ipiUndef) {

                        SetTagItem(pThisTag++,PCDImg_16BaseMag,
                            (BCDByte(ipi.mag16Base[0])*100+
                             BCDByte(ipi.mag16Base[1])));

                    }

                    /* PCDImg_ScanStamp */
                    if (xferULONG(ipi.scanStamp)!=-1L) {

                        if (pResultDate=AllocVec(sizeof(*pResultDate),NULL)) {

                            DatePhotoCDToAmiga(
                                xferULONG(ipi.scanStamp),
                                pResultDate);

                            SetTagItem(pThisTag++,
                                PCDImg_ScanStamp,(ULONG) pResultDate);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_ModifyStamp */
                    if (xferULONG(ipi.modifyStamp)!=-1L) {

                        if (pResultDate=AllocVec(sizeof(*pResultDate),NULL)) {

                            DatePhotoCDToAmiga(
                                xferULONG(ipi.modifyStamp),
                                pResultDate);

                            SetTagItem(pThisTag++,
                                PCDImg_ModifyStamp,(ULONG) pResultDate);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_Medium */
                    if (xferUBYTE(ipi.medium)!=ipiUndef) {

                        SetTagItem(pThisTag++,
                            PCDImg_Medium,xferUBYTE(ipi.medium));

                    }

                    /* PCDImg_ProdType */
                    if (ipi.prodType[0]!=' ' &&
                        ipi.prodType[0]!='\0') {

                        if (resultString=AllocVec(cbProdType+1,NULL)) {

                            memcpy(resultString,ipi.prodType,cbProdType);
                            unpad(resultString,cbProdType);

                            SetTagItem(pThisTag++,
                                PCDImg_ProdType,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_ScnrVndr */
                    if (ipi.scnrVndr[0]!=' ' &&
                        ipi.scnrVndr[0]!='\0') {

                        if (resultString=AllocVec(cbScnrVndr+1,NULL)) {

                            memcpy(resultString,ipi.scnrVndr,cbScnrVndr);
                            unpad(resultString,cbScnrVndr);

                            SetTagItem(pThisTag++,
                                PCDImg_ScnrVndr,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_ScnrProd */
                    if (ipi.scnrProd[0]!=' ' &&
                        ipi.scnrProd[0]!='\0') {

                        if (resultString=AllocVec(cbScnrProd+1,NULL)) {

                            memcpy(resultString,ipi.scnrProd,cbScnrProd);
                            unpad(resultString,cbScnrProd);

                            SetTagItem(pThisTag++,
                                PCDImg_ScnrVndr,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* N.B.: The code for PCDImg_ScnrVer and PCDImg_ScnrRev
                             decode the Scanner Firmware Revision Level,
                             which is ASCII-encoded as X.YY */

                    /* PCDImg_ScnrVer */
                    if (isdigit(ipi.scnrVer[0])) {

                        SetTagItem(pThisTag++,
                            PCDImg_ScnrVer,ipi.scnrVer[0]-'0');

                    }

                    /* PCDImg_ScnrRev */
                    if (isdigit(ipi.scnrVer[2]) &&
                        isdigit(ipi.scnrVer[3])) {

                        SetTagItem(pThisTag++,
                            PCDImg_ScnrRev,(ipi.scnrVer[2]-'0')*10+
                            ipi.scnrVer[3]-'0');

                    }

                    /* PCDImg_ScnrDate */
                    if (isdigit(ipi.scnrDate[0]) &&
                        isdigit(ipi.scnrDate[1])) {

                        if (pResultDate=AllocVec(sizeof(*pResultDate),NULL)) {

                            /* N.B.: dos.library/StrToDate() cannot be used to
                                     convert from ASCII mm/dd/yy to struct DateStamp
                                     because this function expects the '/' character
                                     to be used as a seperator */
                            if (DateComponentToAmiga(
                                (ipi.scnrDate[0]-'\0')*10+(ipi.scnrDate[1]-'\0'),
                                (ipi.scnrDate[3]-'\0')*10+(ipi.scnrDate[4]-'\0'),
                                (ipi.scnrDate[6]-'\0')*10+(ipi.scnrDate[7]-'\0')+THIS_CENTURY,
                                0,0,0,
                                pResultDate)) {

                                SetTagItem(pThisTag++,
                                    PCDImg_ScnrDate,(ULONG) pResultDate);

                            } else {

                                FreeVec(pResultDate);
                                errorCode=PHOTOCD_ERR_DATA_FORMAT;

                            }

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_ScnrSerNo */
                    if (ipi.scnrSerNo[0]!=' ' &&
                        ipi.scnrSerNo[0]!='\0') {

                        if (resultString=AllocVec(cbScnrSerNo+1,NULL)) {

                            memcpy(resultString,ipi.scnrSerNo,cbScnrSerNo);
                            unpad(resultString,cbScnrSerNo);

                            SetTagItem(pThisTag++,
                                PCDImg_ScnrSerNo,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_ScnrPixel */
                    if (ipi.scnrPixel[0]!=ipiUndef &&
                        ipi.scnrPixel[1]!=ipiUndef) {

                        SetTagItem(pThisTag++,
                            PCDImg_ScnrPixel,
                                (BCDByte(ipi.scnrPixel[0])*100)+
                                BCDByte(ipi.scnrPixel[1]));

                    }

                    /* PCDImg_PIWMfgr */
                    if (ipi.piwMfgr[0]!=' ' &&
                        ipi.piwMfgr[0]!='\0') {

                        if (resultString=AllocVec(cbPIWMfgr+1,NULL)) {

                            memcpy(resultString,ipi.piwMfgr,cbPIWMfgr);
                            unpad(resultString,cbPIWMfgr);

                            SetTagItem(pThisTag++,
                                PCDImg_PIWMfgr,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_PhtfinCharSet */
                    if (xferUBYTE(ipi.phtfinCharSet)!=ipiUndef) {
                        SetTagItem(pThisTag++,
                            PCDImg_PhtfinCharSet,
                                xferUBYTE(ipi.phtfinCharSet));
                    }

                    /* PCDImg_PhtfinEscape */
                    if (ipi.phtfinEscape[0]!=ipiUndef) {

                        if (resultString=AllocVec(cbPhtfinEscape+1,NULL)) {

                            memcpy(resultString,ipi.phtfinEscape,cbPhtfinEscape);

                            SetTagItem(pThisTag++,
                                PCDImg_PhtfinEscape,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_PhtfinName */
                    if (ipi.phtfinName[0]!=ipiUndef) {

                        if (resultString=AllocVec(cbPhtfinName+1,NULL)) {

                            memcpy(resultString,ipi.phtfinName,cbPhtfinName);
                            unpad(resultString,cbPhtfinName);

                            SetTagItem(pThisTag++,
                                PCDImg_PhtfinName,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* Detect Scene Balance Algorithm */
                    if (strncmp(
                        ipi.SBASignature,
                        sbaSignatureString,
                        cbSBASignature)==0) {

                        /* PCDImg_SBAVer */
                        SetTagItem(pThisTag++,
                            PCDImg_SBAVer,xferUBYTE(ipi.SBAVer));

                        /* PCDImg_SBARev */
                        SetTagItem(pThisTag++,
                            PCDImg_SBARev,xferUBYTE(ipi.SBARev));

                        /* PCDImg_SBACommand */
                        SetTagItem(pThisTag++,
                            PCDImg_SBACommand,xferUBYTE(ipi.SBACommand));

                        /* PCDImg_SBAData */
                        resultString=AllocVec(PHOTOCD_SBA_DATASIZE,NULL);
                        if (resultString) {

                            SetTagItem(pThisTag++,
                                PCDImg_SBAData,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                    /* PCDImg_Copyright */
                    if (xferUBYTE(ipi.copyrightStatus)==PHOTOCD_COPYRIGHT_PRESENT) {

                        if (resultString=AllocVec(cbCopyrightFile+1,NULL)) {

                            memcpy(resultString,ipi.copyrightFile,cbCopyrightFile);
                            unpad(resultString,cbCopyrightFile);

                            SetTagItem(pThisTag++,
                                PCDImg_Copyright,(ULONG) resultString);

                        } else {

                            errorCode=PHOTOCD_ERR_MEMORY;

                        }

                    }

                }

            }

        }

        /*
         *  End of information decoding
         */

        /* Terminate result tag array */
        SetTagItem(pThisTag++,TAG_DONE,NULL);

    } else {

        errorCode=PHOTOCD_ERR_MEMORY;

    }

    /*
     *  Return
     */

    /* Close image file (if opened) */
    if (intrnlImgHndl) {
        Close(intrnlImgHndl);
    }

    /* If error ... */
    if (errorCode!=PHOTOCD_ERR_NONE) {

        /* Destroy return tag array */
        ReleasePhotoCDInfo(PhotoCDBase,pResultTags);
        pResultTags=NULL;

        /* Return error code */
        pErrorCode=(ULONG *) GetTagData(PCD_ErrorCode,NULL,tags);
        if (pErrorCode) {
            *pErrorCode=errorCode;
        }

    }

    /* Return tag array (NULL if error) */
#ifdef DEBUG
    kprintf("   Return\n");
    kprintf("   pResultTags=$%08lx\n",pResultTags);
    kprintf("   errorCode=%lu\n",errorCode);
#endif /* DEBUG */
    return pResultTags;

}

/****** photocd.library/ReleasePhotoCDInfo ********************************
*
*   NAME
*       ReleasePhotoCDInfo()   -   release Photo CD information tag array
*
*   SYNOPSIS
*       void ReleasePhotoCDInfo(struct TagItem *info);
*
*       ReleasePhotoCDInfo(info);
*
*   FUNCTION
*       Release Photo CD information tag array returned by
*       ObtainPhotoCDInfo().
*
*   INPUTS
*       info            -   Photo CD information tag array returned
*                           by ObtainPhotoCDInfo(); passing NULL for this
*                           parameter is safe
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       ObtainPhotoCDInfo()
*
******************************************************************************
*
*/

void ASM ReleasePhotoCDInfo(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (a0) struct TagItem *info)
{

    struct TagItem *pThisTag, *pTagState;

#ifdef DEBUG
    kprintf("ReleasePhotoCDInfo():\n");
    kprintf("   Entry\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   info=$%08lx\n",info);
#endif /* DEBUG */

    /* If tag array passed ... */
    if (info) {

        /* Walk through tag array */
        pTagState=info;
        while (pThisTag=NextTagItem(&pTagState)) {

            /* Free memory associated with tag values */
            switch (pThisTag->ti_Tag) {

                /* STRPTR-type tags */
                case PCDDisc_Signature:
                case PCDDisc_SerNo:
                case PCDSess_WrtrVndr:
                case PCDSess_WrtrProd:
                case PCDSess_WrtrSerNo:
                case PCDImg_Signature:
                case PCDImg_ScnrSerNo:
                case PCDImg_PIWMfgr:
                case PCDImg_PhtfinEscape:
                case PCDImg_Copyright:
                    FreeVec((void *) pThisTag->ti_Data);
                    break;

                /* struct DateStamp *-type tags */
                case PCDDisc_CreateStamp:
                case PCDDisc_ModifyStamp:
                case PCDSess_WrtrDate:
                case PCDSess_CreateStamp:
                case PCDImg_ScanStamp:
                case PCDImg_ModifyStamp:
                case PCDImg_ScnrDate:
                    FreeVec((void *) pThisTag->ti_Data);
                    break;

                /* UBYTE *-type tags */
                case PCDImg_SBAData:
                    FreeVec((void *) pThisTag->ti_Data);
                    break;

                /* No action for other tag types */
                default:
                    break;

            }

        }

        /* Free tag array */
        FreeVec(info);

    }

#ifdef DEBUG
    kprintf("   Return\n");
#endif /* DEBUG */

}
