/*** internal.h *****************************************************************
 *
 *  $Id: internal.h,v 40.5 94/01/13 11:39:29 jjszucs Exp Locker: jjszucs $
 *
 *  photocd.library
 *  Internal Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1993 Commodore-Amiga, Inc.
 *
 ********************************************************************************/

/********************************************************************************
 *                                                                              *
 *  Includes                                                                    *
 *                                                                              *
 ********************************************************************************/

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif  /* UTILITY_TAGITEM_H */

#ifdef DEBUG
#include <clib/debug_protos.h>
#endif /* DEBUG */

/********************************************************************************
 *                                                                              *
 *  Library base macros                                                         *
 *                                                                              *
 ********************************************************************************/

#ifndef INTERNAL_LIBINIT
#define SysBase     PhotoCDBase->SysBase
#define DOSBase     PhotoCDBase->DOSBase
#define UtilityBase PhotoCDBase->UtilityBase
#endif /* INTERNAL_LIBINIT */

/********************************************************************************
 *                                                                              *
 *  Operating system versions                                                   *
 *                                                                              *
 ********************************************************************************/

#define KICKSTART_VERSION   40      /* Minimum version of Kickstart */
#define WORKBENCH_VERSION   40      /* Minimum version of Workbench */

#define AMIGA_START_YEAR    1978    /* January 1, 1978: The Beginning of Time
                                       (at least to Amigas) :-) */
#define AMIGA_FIRST_LEAP_YEAR 1980  /* First leap year in AmigaDOS date system */

/********************************************************************************
 *                                                                              *
 *  Hard addresses                                                              *
 *                                                                              *
 ********************************************************************************/

#define HARD_EXEC_BASE  0x00000004      /* exec.library base pointer */

/********************************************************************************
 *                                                                              *
 *  Stupid compiler tricks                                                      *
 *                                                                              *
 *  Coming soon to _Late Night with David Letterman_ on CBS-TV :-)              *
 *                                                                              *
 ********************************************************************************/

/* Assembly language interface declaration macros */
#define ASM           __asm
#define REG(x)        register __ ## x

/* unaligned() and xfer*() are necessary to prevent the compiler from
   word-aligning fields in structures that must exactly match data read from
   the disc. */
#define unaligned(type,name) UBYTE name[sizeof(type)]   /* Unaligned field;
                                                           use only with
                                                           primitive types */
#define xferUBYTE(field) ((UBYTE) *field)               /* Unaligned UBYTE transfer */
#define xferUWORD(field) (*(UWORD *) field)             /* Unaligned UWORD transfer */
#define xferULONG(field) (*(ULONG *) field)             /* Unaligned ULONG transfer */

#define Largest(type)   ((1<<sizeof(type)*8)-1)         /* Largest value for type */

/********************************************************************************
 *                                                                              *
 *  Photo CD definitions                                                        *
 *                                                                              *
 ********************************************************************************/

/*
 *  Copyright/Use Rights Codes
 */

#define PHOTOCD_COPYRIGHT_PRESENT   0x01            /* Copyright/use rights restrictions
                                                       specified */
#define PHOTOCD_COPYRIGHT_ABSENT    0xFF            /* Copyright/use rights restrictions
                                                       not specified */

/*
 *  Filesystem Data Retrieval Structure
 */

#define PHOTOCD_DEFAULT_SOURCE  "CD0:"              /* Default source for
                                                       Photo CD disc */

#define imagePathFmt "PHOTO_CD/IMAGES/IMG%04ld.PCD"  /* Photo CD image path format */
#define cbImagePath 48                               /* Length of Photo CD image path */

/*
 *  Microcontroller-Reaadable Sectors Data Retrieval Structure
 */

#define MRS_SECTOR_SIZE         2048                /* Size of MRS sector */
#define MRS_NBYTE_SIZE          4                   /* Size of MRS NBYTE */
#define MRS_HEADER_SIZE         64                  /* Size of MRS header */

/*
 *  PhotoCDDiscInfo - Photo CD disc information
 *
 *  This structure contains the fixed fields of the PHOTO_CD/INFO.PCD
 *  file. The variable fields (Session Descriptor #1..n and Image
 *  Descriptor #1..n) are not included.
 *
 */

struct PhotoCDDiscInfo {

    /* +0 */
#define cbDiscSignature     8                       /* Length of disc signature */
    UBYTE           signature[cbDiscSignature];     /* Disc Signature */

    /* +8 */
    unaligned(UBYTE,specVersion);                   /* Specification Version Number */
    /* +9 */
    unaligned(UBYTE,specRevision);                  /* Specification Revision Number */

    /* +10 */
#define cbDiscSerNo 12                              /* Length of disc serial number */
    UBYTE           serNo[cbDiscSerNo];             /* Disc Serial Number */

    /* +22 */
    unaligned(ULONG,createStamp);                   /* Disc Creation Time and Date
                                                       (seconds since 00:00:00 UTC 1970 Jan 1) */
    /* +26 */
    unaligned(ULONG,modifyStamp);                   /* Disc Modification Time and Date
                                                       (seconds since 00:00:00 UTC 1970 Jan 1) */

    /* +30 */
    unaligned(UWORD,nImages);                       /* Number of Image Packs on Disc */

    /* +32 */
    unaligned(UBYTE,interleave);                    /* Disc Interleave Ratio */
#define intrlvADPCMBitPos 4                         /* Bit position of ADPCM component of interleave ratio */
#define intrlvADPCMMask 0xF0                        /* Mask of ADPCM component of interleave ratio */
#define intrlvImageBitPos 0                         /* Bit position of image component of interleave ratio */
#define intrlvImageMask 0x0F                        /* Mask of image component of interleave ratio */

    /* +33 */
    unaligned(UBYTE,resolution);                    /* Disc Image Pack Resolution Order */
#define resOrdMaxBitPos 4                           /* Bit position of maximum component of resolution order */
#define resOrdMaxMask   0xF0                        /* Mask of maximum component of resolution order */
#define resOrdMinBitPos 0                           /* Bit position of minimum component of resolution order */
#define resOrdMinMask   0x0F                        /* Mask of minimum component of resolution order */
#define resOrdBase      0x00                        /* Resolution order code for Base */
#define resOrd4Base     0x01                        /* Resolution order code for 4Base */
#define resOrd16Base    0x02                        /* Resolution order code for 16Base */

    /* +34 */
#define cbATIME  3                                  /* Length of BCD-coded ATIME */
    UBYTE           leadoutStart[cbATIME];          /* Outermost Lead-out Area Start Time
                                                               (BCD-coded ATIME) */

    /* +37 */
    unaligned(UBYTE,nSessions);                     /* Number of Sessions on Disc */

};

/*
 *  PhotoCDSessionInfo - Photo CD session information
 *
 *  This structure exactly mirrors the Session Descriptor records in the
 *  PHOTO_CD/INFO.PCD file.
 *
 */

struct PhotoCDSessionInfo {

    /* +0 */
    unaligned(UWORD,nImages);                       /* Number of Image Packs in this Session */

    /* +2 */
    UBYTE           CDDAStart[cbATIME];             /* Start Time CD-DA (BCD-coded ATIME) */

    /* +5 */
    UBYTE           leadoutStart[cbATIME];          /* Start Time Lead-out Area (BCD-coded ATIME) */

    /* +8 */
#define cbWriterVendor 8
    UBYTE           writerVendor[cbWriterVendor];   /* Writer Vendor Identification */

    /* +16 */
#define cbWriterProduct 16
    UBYTE           writerProduct[cbWriterProduct]; /* Writer Product Identification */

    /* +32 */
#define cbWriterVersion 4
    UBYTE           writerVersion[cbWriterVersion]; /* Writer Firmware Revision Level (v.rr) */

    /* +36 */
#define cbWriterDate 8
    UBYTE           writerDate[cbWriterDate];       /* Writer Firmware Revision Date (mm/dd/yy) */

    /* +44 */
#define cbWriterSerNo 20
    UBYTE           writerSerNo[cbWriterSerNo];     /* Writer Serial Number */

    /* +64 */
    unaligned(ULONG,createStamp);                   /* Session Creation Time and Date */

};

/*
 *
 *  Photo CD Image Pack Image Component Attributes
 *
 *  This structure exactly mirrors the Image Pack Image Component Attributes
 *  structure in a Photo CD image file. However, the NBYTE structure used for
 *  the Microcontroller-Readable Sectors (MRS) Data Retrieval Structure is
 *  not used. This is abstracted through the use of the readMRSSector
 *
 */

struct PhotoCDIPICA {

    /* NBYTE +0 */
    unaligned(WORD,reserved);                   /* Reserved data */

    /* NBYTE +2 */
    unaligned(UBYTE,attributes);                /* Image Pack Attributes */
#define ipica4BaseHCTBitPos 6
#define ipica4BaseHCTMask 0x60
#define ipicaIPEBit 4
#define ipicaIPEFlag (1<<ipicaIPEBit)
#define ipicaResOrdBitPos 3
#define ipicaResOrdMask 0x0C
#define ipicaRotateBitPos 0
#define ipicaRotateMask 0x03
#define ipicaRotateIncrement 90

    /* NBYTE +3 */
    unaligned(UWORD,stop4Base);                 /* 4Base ICD Stop Sector Offset
                                                   relative to start of
                                                   Image Pack */
    /* NBYTE +5 */
    unaligned(UWORD,stop16Base);                /* 16Base ICD Stop Sector Offset
                                                   relative to start of
                                                   Image Pack */
    /* NBYTE +7 */
    unaligned(UWORD,stopIPE);                   /* IPE Stop Sector Offset
                                                   relative to start of
                                                   Image Pack */
    /* NBYTE +9 */
    unaligned(UBYTE,interleave);                /* Interleaving Ratio */

    /* NBYTE +10 */
    unaligned(UBYTE,prefRes);                   /* ADPCM Image Resolution Selection */
#define ipicaFastBitPos 3
#define ipicaFastMask (1<<ipicaFastBitPos)
#define ipicaResSelBitPos 0
#define ipicaResSelMask 0x07

    /* NBYTE +11 */
    unaligned(UBYTE,magX);                      /* ADPCM Image Magnification Panning
                                                   (X-axis coordinate) */
    /* NBYTE +12 */
    unaligned(UBYTE,magY);                      /* ADPCM Image Magnification Panning
                                                   (Y-axis coordinate) */
    /* NBYTE +13 */
    unaligned(UBYTE,magFactor);                 /* ADPCM Image Magnification Factor
                                                   (magnification=(value+1)/16 */
    /* NBYTE +14 */
    unaligned(UBYTE,dispOffX);                  /* ADPCM Image Display Offset
                                                   (X-axis coordinate) */
    /* NBYTE +15 */
    unaligned(UBYTE,dispOffY);                  /* ADPCM Image Display Offset
                                                   (Y-axis coordinate) */
    /* NBYTE +16 */
    unaligned(UBYTE,transition);                /* ADPCM Image Transition Descriptor;
                                                   one of PHOTOCD_TRANS_* */

    /* +68 */

};

/*
 *
 *  Photo CD Image Pack Information
 *
 *  This structure exactly mirrors the Image Pack Information structure in
 *  a Photo CD image file.
 *
 */

#define ipiUndef 0xFF       /* Code for undefined field */

struct PhotoCDIPI {

#define cbIPISignature 7
    /* +0 */
    UBYTE               signature[cbIPISignature];  /* IPI Signature */

    /* +7 */
#define cbSpecVer 2
    UBYTE               specVer[cbSpecVer];         /* Specification Version Number;
                                                       BCD-encoded; first digit is
                                                       version number, second digit
                                                       is revision number */

    /* +9 */
#define cbPIWVer 2
    UBYTE               piwVer[cbPIWVer];           /* PIW Authoring Software Release;
                                                       BCD-encoded; first digit is
                                                       version number, second digit
                                                       is revision number */
    /* +11 */
#define cbMag16Base 2
    UBYTE               mag16Base[cbMag16Base];     /* Image Magnification Descriptor;
                                                       BCD-coded */

    /* +13 */
    unaligned(ULONG,scanStamp);                     /* Image Scanning Time and Date;
                                                       offset in seconds from 1970
                                                       January 1 00:00:00 UTC */

    /* +17 */
    unaligned(ULONG,modifyStamp);                   /* Last Modification Time and Date;
                                                       offset in seconds from 1970
                                                       January 1 00:00:00 UTC */

    /* +21 */
    unaligned(UBYTE,medium);                        /* Medium of original recording;
                                                       one of PHOTOCD_MEDIUM_* */

    /* +22 */
#define cbProdType 20
    UBYTE               prodType[cbProdType];       /* Product Type of Original Recording */

    /* +42 */
#define cbScnrVndr 20
    UBYTE               scnrVndr[cbScnrVndr];       /* Scanner Vendor Identity */

    /* +62 */
#define cbScnrProd 16
    UBYTE               scnrProd[cbScnrProd];       /* Scanner Product Identity */

    /* +78 */
#define cbScnrVer 4
    UBYTE               scnrVer[cbScnrVer];         /* Scanner Firmware Revision Level;
                                                       X.YY format */

    /* +82 */
#define cbScnrDate 8
    UBYTE               scnrDate[cbScnrDate];        /* Scanner Firmware Revision Date;
                                                        MM/DD/YY format */

    /* +90 */
#define cbScnrSerNo 20
    UBYTE               scnrSerNo[cbScnrSerNo];      /* Scanner Serial Number */

    /* +110 */
#define cbScnrPixel 2
    UBYTE               scnrPixel[cbScnrPixel];      /* Scanner Pixel Size;
                                                        BCD-coded, n.n um */

    /* +112 */
#define cbPIWMfgr 20
    UBYTE               piwMfgr[cbPIWMfgr];          /* PIW Equipment Manufacturer */

    /* +132 */
    unaligned(UBYTE,phtfinCharSet);                  /* Photofinisher's Name Character Set;
                                                        one of PHOTOCD_CHARSET_* */

    /* +133 */
#define cbPhtfinEscape 32
    UBYTE               phtfinEscape[cbPhtfinEscape];/* Photofinisher's Name Escape Sequences */

    /* +165 */
#define cbPhtfinName 60
    UBYTE               phtfinName[cbPhtfinName];    /* Photofinisher's Name */

    /* +225 */
#define cbSBASignature 3
    UBYTE               SBASignature[cbSBASignature];/* SBA Signature */
#define sbaSignatureString "SBA"

    /* +228 */
    unaligned(UBYTE,SBAVer);                         /* SBA Version */

    /* +229 */
    unaligned(UBYTE,SBARev);                         /* SBA Revision */

    /* +230 */
    unaligned(UBYTE,SBACommand);                     /* SBA Command;
                                                        one of PHOTOCD_SBA_* */

    /* +231 */
#define cbSBAData 100
    UBYTE               SBAData[cbSBAData];           /* SBA Data */

    /* +331 */
    unaligned(UBYTE,copyrightStatus);                /* Copyright/Use Rights Status;
                                                         one of PHOTOCD_COPYRIGHT_* */

    /* +333 */
#define cbCopyrightFile 12
    UBYTE               copyrightFile[cbCopyrightFile]; /* Copyright/Use Rights Text File name */

};

/*
 *  Image Pack offsets
 */

#define sectorIPICA     0       /* Sector offset of Image Pack Image Component Attributes */
#define offsetIPA       2048    /* Byte offset of Image Pack Attributes */
#define offsetIPI       3584    /* Byte offset of Image Pack Information */
#define offsetICI       4096    /* Byte offset of Image Component Information */

#define ICDSectorBase16 4       /* First sector of Base/16 Image Component Data */
#define ICDSectorBase4  23      /* First sector of Base/4 Image Component Data */
#define ICDSectorBase   96      /* First sector of Base Image Component Data */
#define ICDSector4Base  389     /* First sector of 4Base Image Component Data */

/********************************************************************************
 *                                                                              *
 *  Internal data structures                                                    *
 *                                                                              *
 ********************************************************************************/

struct PhotoCDHandle {

    BPTR            sourceLock;                     /* Lock on source
                                                       filesystem */

    BPTR            infoHandle;                     /* File handle to
                                                       PHOTO_CD/INFO.PCD */

    /* Cached data status */
    UWORD           cacheImage;                     /* Image number currently
                                                       in cache */
    UBYTE           cacheRes;                       /* Cache resolution */
    UWORD           cacheLine;                      /* Line number currently
                                                       in cache */

    /* Cached Image Pack Image Component Attributes */
    UBYTE           HCT4Base;                       /* 4Base Huffman Code Table */
    BOOL            fIPE;                           /* Image Pack Extension present? */
    UBYTE           resOrder;                       /* Image Pack Resolution Order
                                                       (one of PHOTOCD_RES_*) */
    WORD            rotation;                       /* Required Image Rotation */

    ULONG           stopSector4Base;                /* Offset to first sector
                                                       after 4Base Image Component
                                                       Data */
    ULONG           stopSector16Base;               /* Offset to first sector
                                                       after 16Base Image Component
                                                       Data */
    ULONG           stopSectorIPE;                  /* Offset to first sector
                                                       after Image Pack Extension */
    ULONG           intrlvImage;                    /* Number of image sectors
                                                       per interleaved group */
    ULONG           intrlvADPCM;                    /* Number of audio sectors
                                                       per interleaved group */

    /* Cached Image Component Data */
#define ICDCacheLines   32                          /* Number of lines in ICD cache;
                                                       must be a multiple of 2 less than
                                                       128 (smallest image size) */
#define cbICDCache (ICDCacheLines*3072)+((ICDCacheLines/2)*1536)+((ICDCacheLines/2)*1536) /* Size of ICD cache */
    UBYTE           ICDCache[cbICDCache];           /* Image Component Data buffer 1 */

};

/********************************************************************************
 *                                                                              *
 *  CD-ROM constants and macros                                                 *
 *                                                                              *
 ********************************************************************************/

#define CDROM_SECTOR_SIZE   2048        /* CD-ROM sector size (for Mode 1) */

/* Pack minute/second/frame as 32-bit unsigned long word
   (equivalent to cd.device/struct RMSF) */
#define PackMSF(m,s,f) ( ( (m) << 16 ) | ( (s) << 8 ) | (f) )

/********************************************************************************
 *                                                                              *
 *  General-purpose constants and macros                                        *
 *                                                                              *
 ********************************************************************************/

/* Access nybbles in 8-bit byte */
#define HIGHNYBBLE(byte)    ( ( (byte) & 0xF0 ) >> 4 )
#define LOWNYBBLE(byte)     ( (byte) & 0x0F )

/* Decode BCD byte */
#define BCDByte(bcd)        ( ( HIGHNYBBLE(bcd) * 10 ) + LOWNYBBLE(bcd) )

/********************************************************************************
 *                                                                              *
 *  Real-world constants                                                        *
 *                                                                              *
 ********************************************************************************/

#define SECONDS_PER_MINUTE      60                  /* Seconds per minute */
#define MINUTES_PER_HOUR        60                  /* Minutes per hour */
#define HOURS_PER_DAY           24                  /* Hours per day */
#define MONTHS_PER_YEAR         12                  /* Months per year */
#define DAYS_PER_YEAR           365                 /* Days per non-leap year */
#define DAYS_PER_LEAP_YEAR      366                 /* Days per leap year */
#define LEAP_YEAR_INTERVAL      4                   /* Interval between leap years */
#define LEAP_MONTH              2                   /* Month of year containing
                                                       leap day */

#define THIS_CENTURY            1900                /* Current century */

/********************************************************************************
 *                                                                              *
 *  External definitions                                                        *
 *                                                                              *
 ********************************************************************************/

#ifdef MAIN

struct PhotoCDLibrary *        PhotoCDBase=NULL;

#else

/********************************************************************************
 *                                                                              *
 *  External references                                                         *
 *                                                                              *
 ********************************************************************************/

extern struct PhotoCDLibrary * PhotoCDBase;

/********************************************************************************
 *                                                                              *
 *  Prototypes                                                                  *
 *                                                                              *
 ********************************************************************************/

#endif /* MAIN */

/* from amiga.c */
void SetTagItem(struct TagItem *tagItem,Tag tag,ULONG data);
BOOL DateComponentToAmiga(int month,int day,int year,
    int hour,int minute,int second,
    struct DateStamp *pDateStamp);

/* from photocd.c */
void DatePhotoCDToAmiga(ULONG photoCDStamp,struct DateStamp *pDateStamp);
UBYTE decodeResOrd(UBYTE resolutionOrder);
void unpad(char *string,int cbString);
BPTR openPCDImage(struct PhotoCDLibrary *PhotoCDBase,struct PhotoCDHandle *pHandle,UWORD image);
BOOL readMRSSector(struct PhotoCDLibrary *PhotoCDBase,BPTR fileHandle,UWORD nSector,void *pBuffer,ULONG length);

/* from bsprintf.a */
void bsprintf(char *buffer,char *format,int arg,...);

/* from writeyuv.a */
void ASM writeYUV(REG (a0) UBYTE *pYSrc,
    REG (a1) UBYTE *pC1Src,
    REG (a2) UBYTE *pC2Src,
    REG (a3) UBYTE *pDest,
    REG (d0) UWORD nPixels);

/* from writergb.a */
void ASM writeRGB(REG (a0) UBYTE *pYSrc,
    REG (a1) UBYTE *pC1Src,
    REG (a2) UBYTE *pC2Src,
    REG (a3) UBYTE *pDest,
    REG (d0) UWORD nPixels);

/* from writegamma.a */
void ASM writeGamma(REG (a0) UBYTE *pYSrc,
    REG (a1) UBYTE *pC1Src,
    REG (a2) UBYTE *pC2Src,
    REG (a3) UBYTE *pDest,
    REG (d0) UWORD nPixels,
    REG (a4) UBYTE *rgbLUT);

/* Internal prototypes of public functions */
BOOL ASM IsPhotoCD(REG (a6) struct PhotoCDLibrary *PhotoCDBase);

void * ASM OpenPhotoCDA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (a0) struct TagItem *tags);
void *OpenPhotoCD(struct PhotoCDLibrary *PhotoCDBase,Tag firstTag,...);
void ASM ClosePhotoCD(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (a0) void *pcdHandle);

struct TagItem * ASM ObtainPhotoCDInfoA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (a0) void *pcdHandle,REG (a1) struct TagItem *tags);
struct TagItem *ObtainPhotoCDInfo(struct PhotoCDLibrary *PhotoCDBase,void *pcdHandle,Tag firstTag,...);
void ASM ReleasePhotoCDInfo(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (a0) struct TagItem *pcdInfo);

UBYTE * ASM AllocPCDImageBufferA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (d0) UBYTE resolution,REG (a0) struct TagItem *tags);
UBYTE *AllocPCDImageBuffer(struct PhotoCDLibrary *PhotoCDBase,UBYTE resolution,Tag firstTag,...);
void ASM FreePCDImageBuffer(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (a0) UBYTE *imageBuffer);

BOOL ASM GetPCDResolution(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (d0) UBYTE resolution,REG (a0) UWORD *pWidth,REG (a1) UWORD *pHeight);

BOOL ASM GetPCDImageDataA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,REG (a0) void *pcdHandle,REG (a1) UBYTE *imageBuffer,REG (a2) struct TagItem *tags);
BOOL GetPCDImageData(struct PhotoCDLibrary *PhotoCDBase,void *pcdHandle,UBYTE *imageBuffer,Tag firstTag,...);
