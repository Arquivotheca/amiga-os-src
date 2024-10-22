/*** image.c *****************************************************************
 *
 *  $Id: image.c,v 40.8 94/03/01 11:13:15 jjszucs Exp Locker: jjszucs $
 *
 *  photocd.library
 *  Image Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
$Log:   image.c,v $
 * Revision 40.8  94/03/01  11:13:15  jjszucs
 * Implemented PCD_LineCall and PCD_LineCallIntvl tags
 *
 * Revision 40.7  94/01/13  11:38:22  jjszucs
 * Changed structure tag of Photo CD library base to PhotoCDLibrary
 * Eliminated (bogus, IMHO) warnings caused by assigning ~0 to
 *     UWORD values
 *
 * Revision 40.6  93/12/21  11:22:26  jjszucs
 * Last check-in failed because local file was in use.
 *
 * Revision 40.4  93/11/29  17:45:00  jjszucs
 * Pixel copy and interpolation and YCC-to-RGB conversion functions
 * completely re-written.
 * PCD_GammaCorrect tag of GetPCDImageDataA() allows caller to
 * select gamma-corrected or uncorrected RGB data for PHOTOCD_FORMAT_RGB.
 * Increased size of Image Component Data (ICD) cache to 12 lines of
 * Y component data and 6 lines of C1 and C2 component data; this most
 * closely matches the 14-sector pre-fetch of cd.device for a Base
 * (768x512) image.
 * Eliminated vulnerabilities in ICD cache hit/miss detection code for
 * PCD_File source in GetPCDImageDataA().
 * OpenPhotoCD() now invalidates ICD cache when handle is initialized.
 *
 * Revision 40.3  93/11/19  17:41:32  jjszucs
 * Down-coded pixel copy and YCC-to-RGB conversion to assembly language
 *
 * Revision 40.2  93/11/18  20:00:11  jjszucs
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
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*
 *  ANSI includes
 */

#include <stdio.h>
#include <string.h>

/*
 *  Local includes
 */

#include "photocd.h"
#include "photocdbase.h"

#include "internal.h"

/*
 *  Private declarations
 */

static BOOL intrnlLoadImgData(struct PhotoCDLibrary *PhotoCDBase,
    struct PhotoCDHandle *pHandle, BPTR imageHandle,
    UBYTE *imageBuffer, UBYTE resolution,
    UWORD width, UWORD height, UBYTE format, BOOL gamma,
    UWORD startLine, UWORD endLine,
    struct Hook *lineHook, UWORD lineIntvl);

/* RGB mapping look-up table */
static UBYTE rgbLUT[512] = {
  2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,
 18,  19,  20,  21,  22,  23,  23,  24,  25,  26,  27,  28,  29,  30,  32,  33,
 34,  35,  36,  37,  38,  39,  40,  41,  42,  42,  43,  44,  45,  46,  47,  48,
 49,  50,  51,  52,  53,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  68,  69,  70,  71,  72,  73,  74,  74 , 75,  76,  77,
 78,  79,  80,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  90,  91,
 92,  93,  94,  95,  96,  97,  98,  98,  99, 100, 101, 102, 103, 104, 105, 106,
106, 107, 108, 109, 110, 111, 112, 113, 113, 114, 115, 116, 117, 118, 119, 119,
120, 121, 122, 123, 124, 125, 126, 126, 127, 128, 129, 130, 131, 132, 132, 133,
134, 135, 136, 137, 138, 139, 139, 140, 141, 142, 143, 144, 145, 145, 146, 147,
148, 149, 150, 151, 152, 152, 153, 154, 155, 156, 157, 158, 158, 159, 160, 161,
162, 163, 164, 164, 165, 166, 167, 168, 169, 170, 170, 171, 172, 173, 174, 175,
176, 176, 177, 178, 179, 180, 181, 182, 182, 183, 184, 185, 186, 187, 188, 188,
189, 190, 191, 192, 193, 194, 195, 195, 196, 197, 198, 199, 200, 201, 202, 202,
203, 204, 205, 206, 207, 208, 208, 209, 210, 211, 212, 213, 214, 214, 215, 216,
217, 218, 219, 219, 220, 221, 222, 223, 224, 224, 225, 226, 227, 227, 228, 229,
230, 230, 231, 232, 233, 234, 234, 235, 235, 236, 237, 237, 238, 239, 239, 240,
240, 241, 242, 242, 243, 243, 244, 244, 245, 245, 245, 246, 246, 247, 247, 247,
248, 248, 249, 249, 249, 250, 250, 250, 250, 251, 251, 251, 251, 251, 252, 252,
252, 252, 252, 252, 253, 253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254,
254, 254, 254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

/****** photocd.library/PCDImageBufferSize ***********************************
*
*   NAME
*       PCDImageBufferSize()    -   compute image buffer size
*
*   SYNOPSIS
*       ULONG PCDImageBufferSize(UWORD width,UWORD height);
*
*       cbImageBuffer=ImageBufferSize(width,height);
*
*   FUNCTION
*       Compute buffer size needed for a Photo CD image of the
*       specified dimensions.
*
*   INPUTS
*       width           -   width of image (pixels)
*       height          -   height of image (pixels)
*
*   RESULT
*       cbImageBuffer   -   required size of image buffer (bytes)
*
*   EXAMPLE
*
*   NOTES
*       PCDImageBufferSize() is implemented as a macro defined in
*       libraries/photocd.h and libraries/photocd.i.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****************************************************************************
 *                                                                          *
 *  PCDImageBufferSize() is implemented as a macro defined                  *
 *      in libraries/photocd.h and libraries/photocd.i.                     *
 *                                                                          *
 ****************************************************************************/

/****** photocd.library/PCDImagePixelOffset ***********************************
*
*   NAME
*       PCDImagePixelOffset()    -   compute offset to pixel in Photo CD
*                                    image buffer
*
*   SYNOPSIS
*       ULONG PCDImagePixelOffset(UBYTE resolution,UWORD x,UWORD y);
*
*       offset=PCDImagePixelOffset(resolution,x,y);
*
*   FUNCTION
*       Compute offset to pixel in Photo CD image buffer.
*
*   INPUTS
*       resolution      -   resolution of image; one of PHOTOCD_RES_*
*       x               -   X-axis coordinate of pixel
*       y               -   Y-axis coordinate of pixel
*
*   RESULT
*       offset          -   offset into buffer for pixel triplet
*
*   EXAMPLE
*
*   NOTES
*       PCDImagePixelOffset() is implemented as a macro defined in
*       libraries/photocd.h and libraries/photocd.i.
*
*       This macro is array-oriented, causing the compiler to generate
*       multiplication instructions, and should not be used in time-
*       critical code. For such situations, a pointer-based method
*       should be used.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****************************************************************************
 *                                                                          *
 *  PCDImagePixelOffset() is implemented as a macro defined                 *
 *      in libraries/photocd.h and libraries/photocd.i.                     *
 *                                                                          *
 ****************************************************************************/

/****** photocd.library/GetPCDPixel* **************************************
*
*   NAME
*       GetPCDPixelR(), GetPCDPixelG(), GetPCDPixelB(),
*           GetPCDPixelY(), GetPCDPixelU(), GetPCDPixelV()
*
*   SYNOPSIS
*       UBYTE GetPCDPixelR(UBYTE *imageBuffer,UBYTE resolution,UWORD x,UWORD y);
*       UBYTE GetPCDPixelG(UBYTE *imageBuffer,UBYTE resolution,UWORD x,UWORD y);
*       UBYTE GetPCDPixelB(UBYTE *imageBuffer,UBYTE resolution,UWORD x,UWORD y);
*       UBYTE GetPCDPixelY(UBYTE *imageBuffer,UBYTE resolution,UWORD x,UWORD y);
*       UBYTE GetPCDPixelU(UBYTE *imageBuffer,UBYTE resolution,UWORD x,UWORD y);
*       UBYTE GetPCDPixelV(UBYTE *imageBuffer,UBYTE resolution,UWORD x,UWORD y);
*
*       red=GetPCDPixelR(imageBuffer,resolution,x,y);
*       green=GetPCDPixelG(imageBuffer,resolution,x,y);
*       blue=GetPCDPixelB(imageBuffer,resolution,x,y);
*       luminance=GetPCDPixelY(imageBuffer,resolution,x,y);
*       chrominance1=GetPCDPixelU(imageBuffer,resolution,x,y);
*       chrominance2=GetPCDPixelV(imageBuffer,resolution,x,y);
*
*   FUNCTION
*       Retrieve red, green, blue, luminance, chrominance channel 1,
*       or chrominance channel 2 data for the specified pixel in an
*       image buffer.
*
*   INPUTS
*       imageBuffer     -   buffer containing Photo CD image data
*       resolution      -   resolution of image data; one of PHOTOCD_RES_*
*       x               -   X-axis coordinate of pixel
*       y               -   Y-axis coordinate of pixel
*
*   RESULT
*       red             -   Red component of pixel
*       green           -   Green component of pixel
*       blue            -   Blue component of pixel
*       luminance       -   Luminance component of pixel
*       chrominance1    -   Chrominance channel 1 component of pixel
*       chrominance2    -   Chrominance channel 2 component of pixel
*
*   EXAMPLE
*
*   NOTES
*       The GetPCDPixel*() functions are implemented as a set of macros
*       that access a resolution table in PhotoCDBase.
*
*       The GetPCDPixel*() functions are array-oriented, causing the
*       compiler to generate multiplication instructions, and should
*       not be used in time-critical code. For such situations, a
*       pointer-based method should be used.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

/****************************************************************************
 *                                                                          *
 *  GetPCDPixel*() are implemented as macros defined                        *
 *      in libraries/photocd.h and libraries/photocd.i.                     *
 *                                                                          *
 ****************************************************************************/

/****** photocd.library/AllocPCDImageBuffer *******************************
*
*   NAME
*       AllocPCDImageBuffer()   -   allocate Photo CD image buffer
*
*   SYNOPSIS
*       UBYTE *AllocPCDImageBuffer(UBYTE resolution,Tag firstTag,...);
*       pImageBuffer=AllocPCDImageBuffer(resolution,firstTag,...);
*
*       UBYTE *AllocPCDImageBufferA(UBYTE resolution,struct TagItem *tags);
*       pImageBuffer=AllocPCDImageBufferA(resolution,tags);
*
*   FUNCTION
*       Allocate buffer for a Photo CD image of the specified resolution.
*
*   INPUTS
*       resolution  -   Resolution for which to allocate buffer;
*                       one of PHOTOCD_RES_*
*
*       tags        -   Tag array containing additional parameters
*
*   INPUT TAGS
*       PCD_Lines       -   (UWORD) Number of lines of image buffer memory
*                           to allocate. The default is to allocate a buffer
*                           for a full image of the specified resolution.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       FreePCDImageBuffer()
*
******************************************************************************
*
*/

UBYTE * ASM AllocPCDImageBufferA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (d0) UBYTE resolution,
    REG (a0) struct TagItem *tags)
{

    UWORD width, height;
    UBYTE *imageBuffer;

#ifdef DEBUG
    kprintf("AllocPCDImageBufferA():\n");
    kprintf("   Entry\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   resolution=%d\n",(short) resolution);
    kprintf("   tags=$%08lx\n",tags);
#endif /* DEBUG */

    /* Fetch resolution */
    if (!GetPCDResolution(PhotoCDBase,resolution,&width,&height)) {
        return NULL;
    }

    /* PCD_Lines tag overrides default (full-image) height */
    height=GetTagData(PCD_Lines,height,tags);

    /* Allocate image buffer */
    imageBuffer=AllocVec(width*height*PHOTOCD_BYTES_PER_PIXEL,NULL);
#ifdef DEBUG
    kprintf("   buffer size=%ld\n",width*height*PHOTOCD_BYTES_PER_PIXEL);
#endif /* DEBUG */

    /* Return */
#ifdef DEBUG
    kprintf("   Return\n");
    kprintf("   imageBuffer=$%08lx\n",imageBuffer);
#endif /* DEBUG */
    return imageBuffer;

}

/****** photocd.library/FreePCDImageBuffer ********************************
*
*   NAME
*       FreePCDImageBuffer()    -   free Photo CD image buffer
*
*   SYNOPSIS
*       void FreePCDImageBuffer(UBYTE *imageBuffer);
*       FreePCDImageBuffer(imageBuffer);
*
*   FUNCTION
*       Free Photo CD image buffer allocated with AllocPCDImageBuffer().
*
*   INPUTS
*       imageBuffer     -   Photo CD image buffer allocated with
*                           AllocPCDImageBuffer()
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
*       AllocPCDImageBuffer()
*
******************************************************************************
*
*/

void ASM FreePCDImageBuffer(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (a0) UBYTE *imageBuffer)
{

#ifdef DEBUG
    kprintf("FreePCDImageBuffer():\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   imageBuffer=$%08lx\n",imageBuffer);
#endif /* DEBUG */

    /* If image buffer passed ... */
    if (imageBuffer) {

        /* Free image buffer */
        FreeVec(imageBuffer);

    }

#ifdef DEBUG
    kprintf("   Return\n");
#endif /* DEBUG */

}

/****** photocd.library/GetPCDResolution ***********************************
*
*   NAME
*       GetPCDResolution()      -   get dimensions of specified Photo CD
*                                   image resolution
*
*   SYNOPSIS
*       BOOL GetPCDResolution(UBYTE resolution,UWORD *pWidth,UWORD *pHeight);
*
*       success=GetPCDResolution(resolution,pWidth,pHeight);
*
*   FUNCTION
*       Get dimensions of specified Photo CD image resolution.
*
*   INPUTS
*       resolution      -   resolution; one of PHOTOCD_RES_*.
*
*       pWidth          -   pointer to UWORD to which width is to be written
*
*       pHeight         -   pointer to UWORD to which height is to be written
*
*   RESULT
*       success         -   TRUE if success; FALSE if failure
*
*       *pWidth         -   width of specified Photo CD image
*                           resolution (pixels); unchanged if this
*                           function fails
*
*       *pHeight        -   height of specified Photo CD image
*                           resolution (pixels); unchanged if this
*                           function fails
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

BOOL ASM GetPCDResolution(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (d0) UBYTE resolution,
    REG (a0) UWORD *pWidth,REG (a1) UWORD *pHeight)
{

#ifdef DEBUG
    kprintf("GetPCDResolution():\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   resolution=%d\n",(short) resolution);
    kprintf("   pWidth=$%08lx\n",pWidth);
    kprintf("   pHeight=$%08lx\n",pHeight);
#endif /* DEBUG */

    /* Bounds-check */
    if (resolution==PHOTOCD_RES_BAD || resolution>PHOTOCD_RES_COUNT) {
#ifdef DEBUG
        kprintf("   Bounds-check failed\n");
        kprintf("   Return\n");
#endif /* DEBUG */
        return FALSE;
    }

    /* Return width and height */
    *pWidth=PhotoCDBase->ResWidth[resolution-1];
    *pHeight=PhotoCDBase->ResHeight[resolution-1];
#ifdef DEBUG
    kprintf("   Return\n");
#endif /* DEBUG */
    return TRUE;

}

/****** photocd.library/GetPCDImageData ***********************************
*
*   NAME
*       GetPCDImageData()   -   load and decode Photo CD image data
*
*   SYNOPSIS
*       BOOL GetPCDImageData(void *pcdHandle, UBYTE *imageBuffer,
*           Tag firstTag,...);
*       success=GetPCDImageData(pcdHandle, buffer, firstTag,...);
*
*       BOOL GetPCDImageDataA(void *pcdHandle, UBYTE *imageBuffer,
*           struct TagItem *tags);
*       success=GetPCDImageDataA(pcdHandle, imageBuffer, tags);
*
*   FUNCTION
*       Load and decode data from an image on a Photo CD disc or from
*       a file.
*
*   INPUTS
*       pcdHandle   -   Photo CD handle returned by OpenPhotoCD();
*                       this may be NULL if the PCD_File tag is used
*                       to specify a Photo CD image file
*
*       imageBuffer -   Pointer to buffer for loaded image data.
*                       This buffer must be sufficently large
*                       to hold 24 bits (3 bytes) per retrieved image
*                       pixel. The buffer size may be computed using
*                       PCDImageBufferSize() macro or the buffer may
*                       be allocated using AllocPCDImageBuffer(),
*                       which automatically computes the buffer size.
*
*       tags        -   Tag array containing additional parameters.
*
*   INPUT TAGS
*       All tags are optional, unless otherwise indicated.
*
*       PCD_ErrorCode       (ULONG *) Pointer to ULONG where error code
*                           (one of PHOTOCD_ERR_*) is deposited if this
*                           function fails. This variable is unchanged
*                           if the operation is successful.
*
*       PCD_File            (BPTR) dos.library file handle to a Photo CD
*                           image file (normally with a .PCD extension)
*                           from which the image is to be loaded. Either
*                           this tag or the PCD_Image tag must be specified
*                           to indicate the image that is to be loaded.
*
*       PCD_Image           (UWORD) Cardinal image number of image to load
*                           from Photo CD source on which the handle has been
*                           opened. If this tag is specified, the pcdHandle
*                           argument must be a Photo CD handle returned
*                           by OpenPhotoCD(). Either this tag or
*                           PCD_File must be specified to indicate the
*                           image that is to be loaded.
*
*       PCD_Overview        (BOOL) If the value of this tag is TRUE,
*                           the image is loaded from the Overview Pack.
*                           The Overview Pack contains Base/16 (192 x 128)
*                           versions of all images on the disc in a single,
*                           contiguous block, allowing high-speed loading
*                           of these low-resolution images. In this case,
*                           the image must be specified with PCD_Image
*                           (PCD_File may not be used) and PCD_Resolution
*                           must be PHOTOCD_RES_BASE16 (which is the
*                           default for this tag if PCD_Overview is TRUE).
*
*       PCD_Resolution      (UBYTE) Resolution of image data to be loaded.
*                           One of PHOTOCD_RES_*. The default for this tag
*                           is PHOTOCD_RES_BASE (768 x 512), unless PCD_Overview
*                           is TRUE, in which case PHOTOCD_RES_BASE16
*                           (192 x 128) is the default (and only valid
*                           resolution).
*
*       PCD_Format          (UBYTE) Format to use for loaded image data.
*                           One of PHOTOCD_FORMAT_*. The default for this tag
*                           is PHOTOCD_FORMAT_YUV.
*
*       PCD_StartLine       (UWORD) Ordinal number of the first image line
*                           to be loaded. The default value for this tag
*                           is the first line of the image.
*
*       PCD_EndLine         (UWORD) Ordinal number of the last image line
*                           to be loaded. The default value of this tag
*                           is the last line of the image.
*
*       PCD_GammaCorrect    (BOOL) TRUE to gamma-correct RGB values; FALSE
*                           to return uncorrected RGB values. This tag is
*                           only useful if PCD_Format is PHOTOCD_FORMAT_RGB.
*
*       PCD_LineCall        (struct Hook *) Hook to call while loading lines.
*                           This hook for every <n> lines, as defined by
*                           PCD_LineCallIntvl (or defaulting to 1).
*
*                           This function is called with the standard
*                           hook call conventions, with the following
*                           parameters:
*
*                               hook (A0)   -   Hook structure passed as
*                                               PCD_LineCall tag value
*                               object (A2) -   NULL
*                               message (A1)-   (ULONG *) pointer to line
*                                               number of line for which
*                                               hook is being called.
*
*       PCD_LineCallIntvl   (UWORD) Interval between calls to PCD_LineCall
*                           callback hook. This tag is only applicable with
*                           PCD_LineCall. The default is 1.
*
*   RESULT
*       success     -   TRUE if success; FALSE if failure
*
*   EXAMPLE
*
*   NOTES
*
*       To obtain the best possible performance from this function:
*
*       o   The number of lines read in each call should be a multiple of 2.
*
*       o   The starting line number (specified with PCD_StartLine and
*           defaulting to 0) of each call should be a multiple of 2.
*
*       o   Image data should be read in sequential order (i.e., beginning with
*           line 0 of the image and advancing (preferably by a multiple of 2 lines)
*           toward the last line of the image).
*
*       By following these guidelines, the caller allows the function to:
*
*       o   Utilize cached image data to interpolate chrominance channel 1 (U)
*           and channel 2 (V) data, which is only present for every other pixel
*           in Photo CD images.
*
*       o   Utilize the pre-fetching features of CDFileSystem. Note that
*           CDFileSystem will disable pre-fetching for the life of a file
*           handle if any reverse seeks occur. Reading image lines in
*           non-sequential order will cause reverse seeks, and the
*           resulting disabling of prefetching in CDFileSystem, to occur.
*
*   BUGS
*
*   SEE ALSO
*       DecodePCDImageData()
*
******************************************************************************
*
*/

BOOL ASM GetPCDImageDataA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (a0) void *pcdHandle,
    REG (a1) UBYTE *imageBuffer,
    REG (a2) struct TagItem *tags)
{

    struct PhotoCDHandle *pHandle;

    BPTR imageHandle, internalImgHndl;

    UWORD image;
    struct TagItem *imageInfo;

    ULONG errorCode;
    ULONG *pErrorCode;

    UBYTE resolution;
    UBYTE format;
    UWORD startLine, endLine;
    BOOL gamma;

    UWORD width, height;

    struct Hook *lineHook;
    UWORD lineIntvl;

    /*
     *  Initialization
     */

#ifdef DEBUG
    kprintf("GetPCDImageDataA():\n");
    kprintf("   Entry\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   pcdHandle=$%08lx\n",pcdHandle);
    kprintf("   imageBuffer=$%08lx\n",imageBuffer);
    kprintf("   tags=$%08lx\n",tags);
#endif /* DEBUG */

    pHandle=pcdHandle;
    errorCode=PHOTOCD_ERR_NONE;
    internalImgHndl=NULL;
    image=0;

    /*
     *  Fetch parameters
     */

    resolution=GetTagData(PCD_Resolution,PHOTOCD_RES_BASE,tags);
    format=GetTagData(PCD_Format,PHOTOCD_FORMAT_YUV,tags);
    gamma=GetTagData(PCD_GammaCorrect,FALSE,tags);
    lineHook=(struct Hook *) GetTagData(PCD_LineCall, NULL, tags);
    lineIntvl=GetTagData(PCD_LineCallIntvl, 1, tags);
    if (GetPCDResolution(PhotoCDBase,resolution,&width,&height)) {

#ifdef DEBUG
        kprintf("GetPCDResolution() returned width=%lu height=%lu for resolution=%ld\n",
            width,height,resolution);
#endif /* DEBUG */

        startLine=GetTagData(PCD_StartLine,0,tags);
        endLine=GetTagData(PCD_EndLine,height-1,tags);
#ifdef DEBUG
        kprintf("   startLine=%ld\n",startLine);
        kprintf("   endLine=%ld\n",endLine);
#endif /* DEBUG */

        /*
         *  Open image file
         */

        /* Use PCD_File, if specified ... */
        if (!(imageHandle=(BPTR) GetTagData(PCD_File,NULL,tags))) {

            /* ... else open specified image on source ... */
            if (image=GetTagData(PCD_Image,0,tags)) {

                imageHandle=internalImgHndl=
                    openPCDImage(PhotoCDBase,pHandle,image);

            } else {

                errorCode=PHOTOCD_ERR_ARGS;

            }

        }

        /*
         *  Retrieve image information
         */

        if (imageHandle) {

            /* If source is file or cache is missed ... */
            if (FindTagItem(PCD_File,tags) ||
                pHandle->cacheImage!=image ||
                pHandle->cacheRes!=resolution) {

                /* Obtain image information */
                imageInfo=ObtainPhotoCDInfo(PhotoCDBase,pHandle,
                    PCD_File, imageHandle,
                    PCD_ErrorCode, &errorCode,
                    TAG_DONE);
                if (imageInfo) {

                    /* Update cache key */
                    /* N.B.: Cache will always miss on next call if
                             PCD_File is used, since pcdHandle->cacheImage
                             is set to 0 in this case. This is as intended. */
                    if (pHandle->cacheImage!=image ||
                        pHandle->cacheRes!=resolution) {
                        pHandle->cacheImage=image;
                        pHandle->cacheRes=resolution;
                        pHandle->cacheLine=Largest(UWORD);
                    }

                    /* Load image information cache */
                    pHandle->HCT4Base=GetTagData(PCDImg_4BaseHCT,-1,imageInfo);
                    pHandle->fIPE=GetTagData(PCDImg_IPE,FALSE,imageInfo);
                    pHandle->resOrder=GetTagData(PCDImg_ResOrder,PHOTOCD_RES_BAD,imageInfo);
                    pHandle->rotation=GetTagData(PCDImg_Rotation,0,imageInfo);
                    pHandle->stopSector4Base=GetTagData(PCDImg_4BaseStop,-1,imageInfo);
                    pHandle->stopSector16Base=GetTagData(PCDImg_16BaseStop,-1,imageInfo);
                    pHandle->stopSectorIPE=GetTagData(PCDImg_IPEStop,-1,imageInfo);
                    pHandle->intrlvImage=GetTagData(PCDImg_IntrlvImage,1,imageInfo);
                    pHandle->intrlvADPCM=GetTagData(PCDImg_IntrlvADPCM,0,imageInfo);

                    /* Release image information */
                    ReleasePhotoCDInfo(PhotoCDBase,imageInfo);

                }

            }

            /* If no error ... */
            if (errorCode==PHOTOCD_ERR_NONE) {

                /* Load image data */
                if (!intrnlLoadImgData(PhotoCDBase,
                    pHandle, imageHandle, imageBuffer,
                    resolution, width, height, format, gamma,
                    startLine, endLine,
                    lineHook, lineIntvl)) {
                    errorCode=PHOTOCD_ERR_DATA_FORMAT;
                }

            }

        }

    } else {

        errorCode=PHOTOCD_ERR_ARGS;

    }

    /*
     *  Return
     */

    /* Close internal image file handle (if any) */
    if (internalImgHndl) {

        Close(internalImgHndl);

    }

    /* If error ... */
    if (errorCode!=PHOTOCD_ERR_NONE) {

        /* Return error code */
        pErrorCode=(ULONG *) GetTagData(PCD_ErrorCode,NULL,tags);
        if (pErrorCode) {
            *pErrorCode=errorCode;
        }

    }

    /* Return success/failure */
#ifdef DEBUG
    kprintf("   Return\n");
    kprintf("   errorCode=%ld\n",errorCode);
#endif /* DEBUG */
    return (BOOL) (errorCode==PHOTOCD_ERR_NONE);

}

/****************************************************************************
 *                                                                          *
 *  intrnlLoadImageData() - internal image data loading/decoding            *
 *                                                                          *
 *  This function handles the actual loading and decoding of Photo CD image *
 *  data from an Image Pack. GetPCDImageData() provides an external         *
 *  interface and performs initialization.                                  *
 *                                                                          *
 ****************************************************************************/

static BOOL intrnlLoadImgData(struct PhotoCDLibrary *PhotoCDBase,
    struct PhotoCDHandle *pHandle, BPTR imageHandle,
    UBYTE *imageBuffer, UBYTE resolution,
    UWORD width, UWORD height, UBYTE format, BOOL gamma,
    UWORD startLine, UWORD endLine,
    struct Hook *lineHook, UWORD lineIntvl)
{

    LONG icdOffset;

    UWORD thisLine;
    ULONG thisOffset, thisSize;
    ULONG groupOffset;
    UWORD lineOfCache;

    UBYTE *pDest;
    UBYTE *pYSrc, *pC1Src, *pC2Src;

#ifdef DEBUG
    kprintf("intrlLoadImgData(PhotoCDBase=$%08lx,\n",PhotoCDBase);
    kprintf("   pHandle=$%08lx, imageHandle=$%08lx,\n",pHandle,imageHandle);
    kprintf("   imageBuffer=$%08lx, resolution=%ld,\n",imageBuffer,resolution);
    kprintf("   width=%lu, height=%lu, format=%ld, gamma=%ld,\n",width,height,format,gamma);
    kprintf("   startLine=%lu, endLine=%lu,",startLine,endLine);
    kprintf("   lineHook=$%08lx, lineIntvl=%lu\n", lineHook, lineIntvl);
#endif /* DEBUG */

    /* Compute offset of Image Component Data */
    switch (resolution) {

        case PHOTOCD_RES_BASE16:
            icdOffset=ICDSectorBase16*CDROM_SECTOR_SIZE;
            break;

        case PHOTOCD_RES_BASE4:
            icdOffset=ICDSectorBase4*CDROM_SECTOR_SIZE;
            break;

        case PHOTOCD_RES_BASE:
            icdOffset=ICDSectorBase*CDROM_SECTOR_SIZE;
            break;

        case PHOTOCD_RES_4BASE:
            icdOffset=ICDSector4Base*CDROM_SECTOR_SIZE;
            break;

        case PHOTOCD_RES_16BASE:
            icdOffset=(pHandle->stopSector4Base+
                +1 /* skip ICA */
                +9 /* skip LPT-MRS */
                +2 /* skip LPT */
                +2 /* skip HCT */
                )*CDROM_SECTOR_SIZE;
            break;

    }

    /* Initialize destination pointer */
    pDest=imageBuffer;

    /* Loop through lines */
    for (thisLine=startLine;thisLine<=endLine;thisLine++) {

        /* If cache miss ... */
        if ((thisLine/ICDCacheLines)*ICDCacheLines!=pHandle->cacheLine) {

            /* Set ICD cache key */
            pHandle->cacheLine=thisLine;

            /* Compute Image Component Data offset and size
               for this cached group of lines */
            thisOffset=icdOffset+
                (thisLine/2)*3*width;
            thisSize=(ICDCacheLines*width)+
                     (ICDCacheLines/2*width);

            /* Seek to ICD data */
            if (Seek(imageHandle,thisOffset,OFFSET_BEGINNING)==-1) {
                return FALSE;
            }

            /* Read ICD data */
            if (Read(imageHandle,pHandle->ICDCache,thisSize)!=thisSize) {
                return FALSE;
            }

        }

        /* Compute start of Y, C1, and C2 data for this line */
        lineOfCache=thisLine-pHandle->cacheLine;
        groupOffset=
            (lineOfCache/2)*3*width;
        pYSrc=
            pHandle->ICDCache+
            groupOffset+
            ((lineOfCache%2)*width);
        pC1Src=
            pHandle->ICDCache+
            groupOffset+
            width*2;
        pC2Src=
            pC1Src+width/2;

        /* If RGB format requested ... */
        if (format==PHOTOCD_FORMAT_RGB) {

            /* If gamma-correction requested ... */
            if (gamma) {

                /* Write data in gamma-corrected RGB format */
                writeGamma(pYSrc,pC1Src,pC2Src,pDest,width,rgbLUT);

            } else {

                /* Write data in uncorrected RGB format */
                writeRGB(pYSrc,pC1Src,pC2Src,pDest,width);

            }

        } else {

            /* Write data in YUV format */
            writeYUV(pYSrc,pC1Src,pC2Src,pDest,width);

        }

        /* Advance destination pointer */
        pDest+=width*PHOTOCD_BYTES_PER_PIXEL;

        /* If callback hook specified ... */
        if (lineHook) {

            /* ... and on a line at the specified callback interval ... */
            if ((thisLine-startLine)%lineIntvl==0) {

                CallHook(lineHook, NULL, thisLine);

            }

        }

    }

    /* Success */
    return TRUE;

}
