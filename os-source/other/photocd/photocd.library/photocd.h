#ifndef LIBRARIES_PHOTOCD_H
#define LIBRARIES_PHOTOCD_H

/*
**
**      Photo CD library structures and constants
**
**      $Id: photocd.h,v 40.6 94/03/01 11:12:53 jjszucs Exp $
**
**      (C) Copyright 1993 Commodore-Amiga Inc.
**      All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/********************************************************************************
 *                                                                              *
 *  Resolution codes                                                            *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_RES_BAD     0   /* Invalid resolution */
#define PHOTOCD_RES_BASE16  1   /* Base/16  ( 192 x 128)  */
#define PHOTOCD_RES_BASE4   2   /* Base/4   ( 384 x 256)  */
#define PHOTOCD_RES_BASE    3   /* Base     ( 768 x 512)  */
#define PHOTOCD_RES_4BASE   4   /* 4Base    (1536 x 1024) */
#define PHOTOCD_RES_16BASE  5   /* 16Base   (3072 x 2048) */
#define PHOTOCD_RES_COUNT   5   /* Number of resolutions */

#define PHOTOCD_BASE_WIDTH  768 /* Base resolution width */
#define PHOTOCD_BASE_HEIGHT 512 /* Base resolution height */

/********************************************************************************
 *                                                                              *
 *  Image data format codes                                                     *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_FORMAT_RGB  0   /* RGB format */
#define PHOTOCD_FORMAT_YUV  1   /* YUV format */

/********************************************************************************
 *                                                                              *
 *  Transition codes                                                            *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_TRANS_CUT           0x00            /* Cut */
#define PHOTOCD_TRANS_CROSS         0x01            /* Cross-fade */
#define PHOTOCD_TRANS_RANDOM        0x02            /* Random fade */
#define PHOTOCD_TRANS_BOTTOM2TOP    0x03            /* Bottom-to-top wipe */
#define PHOTOCD_TRANS_TOP2BOTTOM    0x04            /* Top-to-bottom wipe */
#define PHOTOCD_TRANS_LEFT2RIGHT    0x05            /* Left-to-right wipe */
#define PHOTOCD_TRANS_RIGHT2LEFT    0x06            /* Right-to-left wipe */

/********************************************************************************
 *                                                                              *
 *  Medium codes                                                                *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_MEDIUM_COLOR_NEG    0x00            /* Color negative */
#define PHOTOCD_MEDIUM_COLOR_REV    0x01            /* Color reversal */
#define PHOTOCD_MEDIUM_COLOR_HARD   0x02            /* Color hard-copy */
#define PHOTOCD_MEDIUM_THERMAL_HARD 0x03            /* Thermal hard-copy */
#define PHOTOCD_MEDIUM_BW_NEG       0x04            /* Black & white negative */
#define PHOTOCD_MEDIUM_BW_REV       0x05            /* Black & white reversal */
#define PHOTOCD_MEDIUM_BW_HARD      0x06            /* Black & white hard-copy */
#define PHOTOCD_MEDIUM_INTERNEG     0x07            /* Internegative */
#define PHOTOCD_MEDIUM_SYNTH        0x08            /* Synthetic */
#define PHOTOCD_MEDIUM_UNDEFINED    0xFF            /* Undefined */

/********************************************************************************
 *                                                                              *
 *  Codeset codes                                                               *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_CODESET_ISO646_38CH 0x01            /* ISO 646 (38-character) */
#define PHOTOCD_CODESET_ISO646_65CH 0x02            /* ISO 646 7-bit (65-character) */
#define PHOTOCD_CODESET_ISO646      0x03            /* ISO 646 (95-character) */
#define PHOTOCD_CODESET_ISO8859_1   0x04            /* ISO 8859-1 */
#define PHOTOCD_CODESET_ISO2022     0x05            /* ISO 2022 */
#define PHOTOCD_CODESET_NOT_2375    0x06            /* Not registered according
                                                       to ISO 2375 */
#define PHOTOCD_CODESET_UNDEFINED   0xFF            /* Undefined */

/********************************************************************************
 *                                                                              *
 *  SBA (Scene Balance Algorithm) Codes                                         *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_SBA_NEUTRAL_COLOR   0x00            /* Neutral SBA on; color SBA on */
#define PHOTOCD_SBA_NONE            0x01            /* Neutral SBA off; color SBA off */
#define PHOTOCD_SBA_NEUTRAL         0x02            /* Neutral SBA on; color SBA off */
#define PHOTOCD_SBA_COLOR           0x03            /* Neutral SBA off; color SBA on */

/********************************************************************************
 *                                                                              *
 *  Error codes                                                                 *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_ERR_NONE        0   /* No error */
#define PHOTOCD_ERR_MEMORY      1   /* Insufficent free memory */
#define PHOTOCD_ERR_NO_DISC     2   /* No disc present in CD-ROM drive */
#define PHOTOCD_ERR_DOS         3   /* dos.library error */
#define PHOTOCD_ERR_DATA_FORMAT 4   /* Invalid Photo CD data */
#define PHOTOCD_ERR_NOT_FOUND   5   /* Specified image or session not found */
#define PHOTOCD_ERR_ARGS        6   /* Invalid arguments */

/********************************************************************************
 *                                                                              *
 *  Inputs Tags                                                                 *
 *                                                                              *
 ********************************************************************************/

#define PCD_ErrorCode   (TAG_USER+1)    /* (ULONG *) Pointer to ULONG where
                                           error code (one of PHOTOCD_ERR_*)
                                           is deposited if an error is
                                           encountered. This variable is
                                           unchanged if the operation is
                                           successful. */

#define PCD_Source      (TAG_USER+2)    /* (STRPTR) Path to Photo CD filesystem
                                           structure. */

#define PCD_Disc        (TAG_USER+2)    /* Operate on disc */

#define PCD_Session     (TAG_USER+3)    /* (UBYTE) Operate on session specified
                                           by cardinal session number. */

#define PCD_Image       (TAG_USER+4)    /* (UWORD) Operate on image specified
                                           by cardinal image number. */

#define PCD_File        (TAG_USER+5)    /* (BPTR) Operate on file handle. */

#define PCD_Lines       (TAG_USER+6)    /* (ULONG *) Number of lines of
                                           image buffer memory to allocate. */

#define PCD_Overview    (TAG_USER+7)    /* (BOOL) Use Overview Pack. */

#define PCD_Resolution  (TAG_USER+8)    /* (UBYTE) Resolution on which to
                                           operate. One of PHOTOCD_RES_*. */

#define PCD_Format      (TAG_USER+9)    /* (UBYTE) Image data format on
                                           which to operate. One of
                                           PHOTOCD_FORMAT_*. */

#define PCD_StartLine   (TAG_USER+10)   /* (UWORD) Ordinal number of first
                                           line of image on which to operate. */

#define PCD_EndLine     (TAG_USER+11)   /* (UWORD) Ordinal number of last
                                           line of image on which to operate. */

#define PCD_GammaCorrect (TAG_USER+12)  /* (BOOL) Gamma-correct RGB values. */

#define PCD_LineCall    (TAG_USER+13)   /* (struct Hook *) Hook to call
                                           while loading lines with
                                           GetPCDImageData(). */

#define PCD_LineCallIntvl (TAG_USER+14) /* (UWORD) Interval between calls
                                           to line callback hook. Only
                                           applicable if used with PCD_LineCall.
                                           Default is 1 (call for each line). */

/********************************************************************************
 *                                                                              *
 *  Disc Information Tags                                                       *
 *                                                                              *
 ********************************************************************************/

#define PCDDisc_Signature   (TAG_USER+1)    /* (STRPTR) Disc signature */
#define PCDDisc_Version     (TAG_USER+2)    /* (UBYTE) Specification version
                                               number */
#define PCDDisc_Revision    (TAG_USER+3)    /* (UBYTE) Specification revision
                                               number */
#define PCDDisc_SerNo       (TAG_USER+4)    /* (STRPTR) Disc serial number */
#define PCDDisc_CreateStamp (TAG_USER+5)    /* (struct DateStamp *) Disc
                                               creation stamp */
#define PCDDisc_ModifyStamp (TAG_USER+6)    /* struct DateStamp *) Disc
                                               modification stamp */
#define PCDDisc_nImages     (TAG_USER+7)    /* (UWORD) Number of images
                                               on disc */
#define PCDDisc_IntrlvADPCM (TAG_USER+8)    /* (UBYTE) Number of ADPCM audio
                                               sectors per interleaved group */
#define PCDDisc_IntrlvImage (TAG_USER+9)    /* (UBYTE) Number of image sectors
                                               per interleaved group */
#define PCDDisc_MinRes      (TAG_USER+10)   /* (UBYTE) Lowest image resoluton
                                               on disc. One of PHOTOCD_RES_*. */
#define PCDDisc_MaxRes      (TAG_USER+11)   /* (UBYTE) Highest image resolution
                                               on disc. One of PHOTOCD_RES_*. */
#define PCDDisc_LeadoutStart (TAG_USER+12)  /* (ULONG) Start of final lead-out
                                               area on disc.
                                               cd.device/struct RMSF packed
                                               as 32-bit unsigned long word. */
#define PCDDisc_nSessions   (TAG_USER+13)   /* (UBYTE) Number of sessions
                                               on disc. */

/********************************************************************************
 *                                                                              *
 *  Session Information Tags                                                    *
 *                                                                              *
 ********************************************************************************/

#define PCDSess_nImages     (TAG_USER+32)    /* (UWORD) Number of images
                                               recorded in this session */
#define PCDSess_CDDAStart   (TAG_USER+33)    /* (ULONG) Start of first CD-DA
                                               track in this session.
                                               cd.device/struct RMSF packed as
                                               32-bit unsigned long word. */
#define PCDSess_LeadoutStart (TAG_USER+34)   /* (ULONG) Start of lead-out area
                                               of this session.
                                               cd.device/struct RMSF packed as
                                               32-bit unsigned long word. */
#define PCDSess_WrtrVndr    (TAG_USER+35)    /* (STRPTR) Vendor identification
                                               of writer device used to write
                                               this session. */
#define PCDSess_WrtrProd    (TAG_USER+36)    /* (STRPTR) Product identification
                                               of writer device used to write
                                               this session. */
#define PCDSess_WrtrVer      (TAG_USER+37)   /* (UBYTE) Version number (also
                                               known as major version number)
                                               of writer device used to write
                                               this session. */
#define PCDSess_WrtrRev     (TAG_USER+38)    /* (UBYTE) Revision number (also
                                               known as minor version number)
                                               of writer device used to write
                                               this session. */
#define PCDSess_WrtrDate    (TAG_USER+39)    /* (struct DateStamp *) Revision
                                               date of firmware of writer
                                               device used to write
                                               this session. */
#define PCDSess_WrtrSerNo   (TAG_USER+40)    /* (STRPTR) Serial number of
                                               writer device used to write
                                               this session. */
#define PCDSess_CreateStamp (TAG_USER+41)   /* (struct DateStamp *) Session
                                               creation stamp */

/********************************************************************************
 *                                                                              *
 *  Image Information Tags                                                      *
 *                                                                              *
 ********************************************************************************/

#define PCDImg_4BaseHCT     (TAG_USER+64)    /* (UBYTE) 4Base Huffman Code Table.
                                                In the range 1..4, corresponding
                                                to the four 4Base Huffman Code
                                                Table classes specified by
                                                the Photo CD standard. */
#define PCDImg_IPE          (TAG_USER+65)    /* (BOOL) TRUE if an Image Pack
                                                Extension (IPE) is present.
                                                FALSE if IPE is not present. */
#define PCDImg_ResOrder     (TAG_USER+66)    /* (UBYTE) Resolution order
                                                (highest resolution available
                                                for this image). One of
                                                PHOTOCD_RES_*. */
#define PCDImg_Rotation     (TAG_USER+67)    /* (WORD) Degrees of counter-
                                               clockwise rotation required
                                               to display image in intended
                                               orientation. */
#define PCDImg_4BaseStop    (TAG_USER+68)   /* (UWORD) Sector offset of first
                                               sector after 4Base ICD. */
#define PCDImg_16BaseStop   (TAG_USER+69)   /* (UWORD) Sector offset of first
                                               sector after 16Base ICD. */
#define PCDImg_IPEStop      (TAG_USER+70)   /* (UWORD) Sector offset of first
                                               sector after IPE. */
#define PCDImg_IntrlvADPCM  (TAG_USER+71)    /* (UBYTE) Number of ADPCM audio
                                               sectors per interleaved group */
#define PCDImg_IntrlvImage  (TAG_USER+72)    /* (UBYTE) Number of image sectors
                                               per interleaved group */
#define PCDImg_PrefFast     (TAG_USER+73)   /* (BOOL) TRUE if fast loading
                                               is preferred. FALSE if high
                                               resolution is preferred. */
#define PCDImg_PrefRes      (TAG_USER+74)   /* (UBYTE) Preferred resolution
                                               for single-speed (150KB/s)
                                               reader. One of PHOTOCD_RES_*. */
#define PCDImg_MagX         (TAG_USER+75)   /* (UWORD) X-axis coordinate
                                               of center of magnification area. */
#define PCDImg_MagY         (TAG_USER+76)   /* (UWORD) Y-axis coordinate of
                                               center of magnification area. */
#define PCDImg_MagFactor    (TAG_USER+77)   /* (UWORD) Linear
                                               magnification factor. */
#define PCDImg_DispOffX     (TAG_USER+78)   /* (UWORD) X-axis coordinate on
                                               display area for center of
                                               image. */
#define PCDImg_DispOffY     (TAG_USER+79)   /* (UWORD) Y-axis coordinate on
                                               display area for center of
                                               image. */
#define PCDImg_Transition   (TAG_USER+80)   /* (UBYTE) Transition descriptor.
                                               One of PHOTOCD_TRANS_*. */
#define PCDImg_Signature    (TAG_USER+81)   /* (STRPTR) Image Pack Information
                                               signature. */
#define PCDImg_SpecVer      (TAG_USER+82)   /* (UBYTE) Version number (also
                                               known as major version number)
                                               of Photo CD specification to
                                               which this image conforms. */
#define PCDImg_SpecRev      (TAG_USER+83)   /* (UBYTE) Revision number (also
                                               known as minor version number)
                                               of Photo CD specification to
                                               which this image conforms. */
#define PCDImg_PIWVer       (TAG_USER+84)   /* (UBYTE) Version number (also
                                               known as major version number)
                                               of imaging workstation software
                                               used to process this image. */
#define PCDImg_PIWRev       (TAG_USER+85)   /* (UBYTE) Revision number (also
                                               known as minor version number)
                                               of imaging workstation software
                                               used to process this image. */
#define PCDImg_16BaseMag    (TAG_USER+86)   /* (UWORD) Magnification factor
                                               applied to 16Base source image. */
#define PCDImg_ScanStamp    (TAG_USER+87)   /* (struct DateStamp *) Image
                                               scanning stamp. */
#define PCDImg_ModifyStamp  (TAG_USER+88)   /* (struct DateStamp *) Last image
                                               modification stamp. */
#define PCDImg_Medium       (TAG_USER+89)   /* (UBYTE) Medium of source image.
                                               One of PHOTOCD_MEDIUM_*. */
#define PCDImg_ProdType     (TAG_USER+90)   /* (STRPTR) Product type of
                                               original image. */
#define PCDImg_ScnrVndr     (TAG_USER+91)   /* (STRPTR) Identity of scanner
                                               vendor. */
#define PCDImg_ScnrProd     (TAG_USER+92)   /* (STRPTR) Identity of scanner
                                               product. */
#define PCDImg_ScnrVer      (TAG_USER+93)   /* (UBYTE) Version number (also
                                               known as major version number)
                                               of scanner firmware. */
#define PCDImg_ScnrRev      (TAG_USER+94)   /* (UBYTE) Revision number (also
                                               known as minor version number)
                                               of scanner firmware. */
#define PCDImg_ScnrDate     (TAG_USER+95)   /* (struct DateStamp *) Scanner
                                               firmware revision date. */
#define PCDImg_ScnrSerNo    (TAG_USER+96)   /* (STRPTR) Serial number of
                                               scanner. */
#define PCDImg_ScnrPixel    (TAG_USER+97)   /* (UWORD) Pixel size of
                                               scanner. Fixed-point representation
                                               with two fractional digits. */
#define PCDImg_PIWMfgr      (TAG_USER+98)   /* (STRPTR) Imaging workstation
                                               manufacturer. */
#define PCDImg_PhtfinCharSet (TAG_USER+99)  /* (UBYTE) Photofinisher's name
                                               character set. One of
                                               PHOTOCD_CHARSET_*. */
#define PCDImg_PhtfinEscape (TAG_USER+100)  /* (STRPTR) Photofinisher's name
                                               escape sequences (for ISO 2022
                                               character set). */
#define PCDImg_PhtfinName   (TAG_USER+101)  /* (STRPTR) Photofinisher's name. */
#define PCDImg_SBAVer       (TAG_USER+102)  /* (UBYTE) Version number (also
                                               known as major version number)
                                               of scene balance algorithm. */
#define PCDImg_SBARev       (TAG_USER+103)  /* (UBYTE) Revision number (also
                                               known as minor version number)
                                               of scene balance algorithm. */
#define PCDImg_SBACommand   (TAG_USER+104)  /* (UBYTE) Scene balance algorithm
                                               command. One of PHOTOCD_SBA_*. */
#define PCDImg_SBAData      (TAG_USER+105)  /* (UBYTE *) Scene balance algorithm
                                               data. */
#define PHOTOCD_SBA_DATASIZE          100   /* Size of PCDImg_SBAData */
#define PCDImg_Copyright    (TAG_USER+106)  /* (STRPTR) Filename of text file
                                               containing copyright/use rights
                                               text. */

/********************************************************************************
 *                                                                              *
 *  Image Constants                                                             *
 *                                                                              *
 ********************************************************************************/

#define PHOTOCD_BYTES_PER_PIXEL     4       /* Bytes per pixel for RGB/YUV data */

/********************************************************************************
 *                                                                              *
 *  Macros                                                                      *
 *                                                                              *
 ********************************************************************************/

/* Compute size of image buffer required for specified width and height */
#define PCDImageBufferSize(width,height)   (width*height*PHOTOCD_BYTES_PER_PIXEL)

/* Compute offset of pixel */
#define PCDImagePixelOffset(resolution,x,y) \
    (((PhotoCDBase->ResWidth[resolution]*y)+x)*PHOTOCD_BYTES_PER_PIXEL)

/* Fetch color component for pixel from buffer */
#define GetPCDPixelR(imageBuffer,resolution,x,y)   (imageBuffer[PCDImagePixelOffset(resolution,x,y)+1])
#define GetPCDPixelG(imageBuffer,resolution,x,y)   (imageBuffer[PCDImagePixelOffset(resolution,x,y)+2])
#define GetPCDPixelB(imageBuffer,resolution,x,y)   (imageBuffer[PCDImagePixelOffset(resolution,x,y)+3])
#define GetPCDPixelY(imageBuffer,resolution,x,y)   (imageBuffer[PCDImagePixelOffset(resolution,x,y)+1])
#define GetPCDPixelU(imageBuffer,resolution,x,y)   (imageBuffer[PCDImagePixelOffset(resolution,x,y)+2])
#define GetPCDPixelV(imageBuffer,resolution,x,y)   (imageBuffer[PCDImagePixelOffset(resolution,x,y)+3])

#endif /* LIBRARIES_PHOTOCD_H */
