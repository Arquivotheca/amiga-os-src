/*** image.c *****************************************************************
 *
 *  $Id: image.c,v 1.8 94/03/31 16:08:48 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Image Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   image.c,v $
 * Revision 1.8  94/03/31  16:08:48  jjszucs
 * o   Applying mirror manipulation to portrait-orientation images no longer
 *     causes inappropriate cropping.
 *
 * o   The Normalize glyph is now a 65x65 bitmap, like all other button glyphs.
 *     The glyph was 64x64 and strange lines when the glyph was displayed
 *     as a 65x65 mask was applied.
 *
 * o   The Zoom In and Zoom Out glyphs had black vertical line in the far
 *     right column. This has been eliminated.
 *
 * o   The interface panel is now made visible when the thumbnail state
 *     is initially entered at the start of a session.
 *
 * o   Changed fixed palette range from 16...240 to 0...255 for each
 *     color component (R/G/B). Although the clipped color component range
 *     was theoretically better for image quality, this change significantly
 *     reduces the complexity of the fixed palette code (which is called for
 *     all displayed pixels), increasing image display speed. The observed
 *     impact of the change on image quality is neglible.
 *
 * o   For NTSC systems, quick scaling was inadvertently being performed
 *     on the X-axis for portrait-orientation images. This increased
 *     the aspect ratio distortion, instead of decreasing the aspect
 *     ratio (as was the intent). This has been corrected, with significant
 *     code savings as a bonus.
 *
 * o   Center of zoom box now accurately corresponds to zoom cursor for
 *     all cases.
 *
 * o   Zoom center point is now at the center of the lenses of the magnifying
 *     glass image, not at the center of the entire image (which includes
 *     the handle).
 *
 * o   Zoom cursor bounds-checking is now correct for all cases.
 *
 * o   Zooming of portrait-orientation images now works correctly.
 *
 * o   Yet another attempt was made to implement a smooth scroll in the
 *     thumbnail display. However, due to the depth of the thumbnail screen
 *     (8 bitplanes), this results in a noticable "inchworm" effect. Since this
 *     is undesirable, jump scrolling continues to be used. This is being noted
 *     in this release note (and the associated RCS logs) primarily as a note
 *     to myself (and possibly others) once and for all that this is a
 *     Bad Idea(TM).
 *
 * o   Yet more changes to the "Easter Egg" text.
 *
 *
 * Revision 1.7  94/03/16  18:20:18  jjszucs
 * Zoom manipulation implemented.
 *
 * Fixed palette image display implemented with
 * DISPLAY_FIXED_PALETTE conditional compilation
 * option.
 *
 * Pan X/Y offsets eliminated from image node
 * structure, no longer needed as pan is not
 * necessary.
 *
 * All image manipulation features correctly mark
 * images as manipulated. Normalize button is
 * now correctly enabled/disabled depending on
 * state of image.
 *
 * Control list cleared during all state transitions.
 * Prevents potential crashes due to inappropriate
 * controls remaining available.
 *
 * Further changes to "Easter Egg" screen per davidj
 * and eric.
 *
 * Revision 1.6  94/03/09  17:05:52  jjszucs
 * Slideshow playback code implemented.
 *
 * Revision 1.5  94/03/08  13:55:38  jjszucs
 * Color quantization function is now called directly, without an
 * intermediate data-hiding function. This eliminates one function
 * call per pixel.
 *
 * Quantization octree is now exposed in application context. This
 * supports direct calling of the color quantization code.
 *
 * Color table is now exposed in the application context. This allows
 * HAM8 color selection function to access color table directly,
 * without overhead of accessor function.
 *
 *
 * Revision 1.4  94/03/01  16:11:48  jjszucs
 * Generalized scaling and aspect ratio adjustment code replaced
 * with quick scaling (for NTSC only) performed by averaging every
 * 5th line (or column) into previous line or column.
 * Mirror conditional replaced with pre-calculated step value
 * to move from column to column.
 * Hook function used with PCD_LineCall/PCD_LineCallIntvl tags of
 * photocd.library/GetPCDImageData() to dynamically update busy
 * bar while image is loading.
 * Display screen hidden during image processing to improve
 * chip RAM bandwidth availability.
 * Quantization octree now generated from every 2nd pixel
 * horizontally and vertically. Because natural images do not
 * typically contain many isolated pixels, this modification
 * is appropriate for this application.
 * Busy bar now terminates at 100% in all cases.
 *
 * Revision 1.3  94/02/18  15:56:01  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:04:47  jjszucs
 * Large portions of debugging output removed
 * Display and interface screen now opened with interleaved bitmaps
 *     to reduce artifacts during erasing
 * Potential Enforcer hit resulting from gaps in the quantization
 *     octree has been worked around
 * Now uses low-resolution non-interlaced HAM8 display for images.
 *     This produces good image quality, while minimizing the pixel
 *     processing and video RAM requirements.
 * Now scales images to adjust for aspect ratio and fit entire image
 *     on-screen. This corrects the aspect ratio distortions observed
 *     in Photo CD (Amiga CD) 40.1 and eliminates the need to scroll
 *     HAM8 displays.
 *
 * Revision 1.1  94/01/06  11:57:19  jjszucs
 * Initial revision
 *
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* ANSI includes */
#include <stdio.h>
#include <math.h>

/* Amiga includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>

#include <libraries/lowlevel.h>

#include <intuition/intuition.h>

#include <libraries/photocd.h>
#include <libraries/photocdbase.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/photocd_protos.h>
#include <clib/debug_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/photocd_pragmas.h>

/* Local includes */
#include "global.h"
#include "display.h"
#include "interface.h"
#include "image.h"
#include "quantization.h"
#include "scaling.h"

/****************************************************************************
 *                                                                          *
 *  Local Prototypes                                                        *
 *                                                                          *
 ****************************************************************************/

static ULONG __saveds __asm lineFunc(
    register __a0 struct Hook *hook,
    register __a2 APTR object,
    register __a1 APTR message
);

/****************************************************************************
 *                                                                          *
 *  Local Definitions                                                       *
 *                                                                          *
 ****************************************************************************/

#define LINE_CALL_INTVL         10      /* Interval between calls to
                                           GetPCDImageData() callback hook */

#define SCREEN_SLIDE_STEP       4       /* Increment for screen sliding */

/****************************************************************************
 *                                                                          *
 *  displayImage()  -   display image                                       *
 *                                                                          *
 ****************************************************************************/

/* To improve performance, the generalized scaling and aspect ratio
   adjustment code (used through version 40.8) has been replaced
   by four special cases, each representing one of the expected real-world
   conditions:

   o NTSC:

        Landscape   -   Display at low-resolution non-interlaced
                        with maximum overscan. Average every nth
                        line with every n+1nth line, beginning
                        at line n. The approximate integer value
                        of n is 5. Crop equal number of pixels at
                        left and right edges.

        Portrait    -   Display at high-resolution interlaced
                        with maximum overscan. Crop equal number
                        of pixels at both edges.

  o PAL:

        Landscape   -   Display at low-resolution non-interlaced
                        with maximum overscan. Crop equal number
                        of pixels at left and right edges.

        Portrait   -   Display at high-resolution interlaced
                        with maximum overscan as is.

    This algorithm was proposed by Carolyn Scheppner, CATS
    (carolyn@cbmvax).

*/

BOOL displayImage(struct appContext *appContext,struct imageNode *imageNode)
{

    BOOL        status;

    ULONG       *imageBuffer;
    UWORD       imageWidth, imageHeight;
    UWORD       pageWidth, pageHeight;

    UWORD       outputWidth, outputHeight;

    UWORD       x, y;
    UWORD       magnifyX, magnifyY;

    struct      RastPort *rastPort;
    UBYTE       pixelValue;
    ULONG       lastRGB;
    ULONG       pixelRGB, neighborRGB;
    ULONG       *pRow, *pPixel;
    ULONG       progress;

    UBYTE       *lineBuffer;
    UBYTE       *pOutput;

    LONG        pixelStep, rowStep, colStep;
    UWORD       xOffset, yOffset;
    WORD        xCrop;

    BOOL        fPortrait;
    BOOL        fNTSC;
    BOOL        fSkip;

    int         slide;

    static struct Hook lineHook;

    UBYTE       pen;
    UBYTE       gunR, gunG, gunB;

    /* Assume success */
    status=TRUE;

    /* Busy interface */
    if (!interfaceBusyStart(appContext, GLYPH_BUSY_IMAGE)) {

        return FALSE;

    }

    /* Is this a portrait-orientation image? */
    fPortrait=imageNode->in_Rotation%180?FALSE:TRUE;

    /* Is this an NTSC system */
    fNTSC=(GfxBase->DisplayFlags&PAL)?FALSE:TRUE;

    /* Change to appropriate display mode */
    changeDisplay(appContext, as_Image, fPortrait);

#ifdef DISPLAY_FIXED_PALETTE

    /* Load fixed palette */
    pen=0;
    for (gunR=0; gunR<FIXED_PALETTE_GUN_RANGE; gunR++) {

        for (gunG=0; gunG<FIXED_PALETTE_GUN_RANGE; gunG++) {

            for (gunB=0; gunB<FIXED_PALETTE_GUN_RANGE; gunB++) {

                SetRGB32(
                    &appContext->ac_DisplayScreen->ViewPort,
                    pen,
                    Gun32(FIXED_PALETTE_GUN_MIN+(gunR*FIXED_PALETTE_GUN_STEP)),
                    Gun32(FIXED_PALETTE_GUN_MIN+(gunG*FIXED_PALETTE_GUN_STEP)),
                    Gun32(FIXED_PALETTE_GUN_MIN+(gunB*FIXED_PALETTE_GUN_STEP))
                );

                pen++;

            }

        }

    }

#endif /* DISPLAY_FIXED_PALETTE */

    /* Hide display screen to increase bandwidth */
    MoveScreen(appContext->ac_DisplayScreen,
        0, appContext->ac_DisplayScreen->Height);

    /* Initialize line callback hook */
    lineHook.h_Entry=(HOOKFUNC) lineFunc;
    lineHook.h_SubEntry=NULL;
    lineHook.h_Data=appContext;

    /* Fetch image dimensions */
    if (GetPCDResolution(
        imageNode->in_Magnification,
        &imageWidth, &imageHeight)) {

        /* Compute page width (visible portion of image) */
        pageWidth=
            (imageNode->in_Magnification==PHOTOCD_RES_BASE)?
                imageWidth/2:imageWidth;
        pageHeight=
            (imageNode->in_Magnification==PHOTOCD_RES_BASE)?
                imageHeight/2:imageHeight;

        /* Fetch magnification coordinates, rotating as necessary */
        switch (imageNode->in_Rotation) {

            case 0:
            case 90:
                magnifyX=imageNode->in_MagnifyX;
                magnifyY=imageNode->in_MagnifyY;
                break;

            case 180:
                magnifyX=imageWidth-imageNode->in_MagnifyX-pageWidth;
                magnifyY=imageHeight-imageNode->in_MagnifyY-pageHeight;
                break;

            case 270:
                magnifyX=imageWidth-imageNode->in_MagnifyX-pageWidth;
                magnifyY=imageNode->in_MagnifyY;
                break;

        }

        /* Find image buffer */
        imageBuffer=appContext->ac_ImageBuffer;
        if (imageBuffer) {

            /* Compute output dimensions */
            outputWidth=fPortrait?
                (fNTSC?NTSC_PORTRAIT_WIDTH:PAL_PORTRAIT_WIDTH):
                (fNTSC?NTSC_LANDSCAPE_WIDTH:PAL_LANDSCAPE_WIDTH);
            outputHeight=fPortrait?
                (fNTSC?NTSC_PORTRAIT_HEIGHT:PAL_PORTRAIT_HEIGHT):
                (fNTSC?NTSC_LANDSCAPE_HEIGHT:PAL_LANDSCAPE_HEIGHT);

            /* Store busy bar total and start in application context
               for use by line callback hook */
#ifdef DISPLAY_FIXED_PALETTE
            appContext->ac_HookTotal=pageHeight*2;
#else
            appContext->ac_HookTotal=pageHeight*3;
#endif /* DISPLAY_FIXED_PALETTE */
            appContext->ac_HookStart=magnifyY;

            /* Load image data in RGB format */
            if (GetPCDImageData(appContext->ac_PhotoCDHandle,
                (UBYTE *) imageBuffer,
                PCD_Image, imageNode->in_Number,
                PCD_Resolution, imageNode->in_Magnification,
                PCD_Format, PHOTOCD_FORMAT_RGB,
                PCD_StartLine, magnifyY,
                PCD_EndLine, magnifyY+pageHeight-1,
                PCD_GammaCorrect, FALSE,
                PCD_LineCall, &lineHook,
                PCD_LineCallIntvl, LINE_CALL_INTVL,
                TAG_DONE)) {

                /* Update busy panel */
#ifdef DISPLAY_FIXED_PALETTE
                interfaceBusyUpdate(appContext, pageHeight*2, pageHeight);
#else
                interfaceBusyUpdate(appContext, pageHeight*3, pageHeight);
#endif /* DISPLAY_FIXED_PALETTE */

                /* Quantize image */
#ifdef DISPLAY_FIXED_PALETTE
                if (TRUE) {
#else
                if (obtainQuantization(appContext,
                    &appContext->ac_DisplayScreen->ViewPort,
                    imageBuffer, imageWidth, pageHeight,
                    imageWidth,
                    IMAGE_QUANT_X_SAMPLE, IMAGE_QUANT_Y_SAMPLE)) {
#endif /* DISPLAY_FIXED_PALETTE */

                    /* Compute cropping */
                    xCrop=fPortrait?
                            ((pageWidth-outputWidth)/2):
                            0;

                    /* Compute output offset */
                    xOffset=appContext->ac_DisplayWindow->Width/2-
                        outputWidth/2;
                    yOffset=appContext->ac_DisplayWindow->Height/2-
                        outputHeight/2;

                    /* Store picture dimensions and offsets for later
                       use by zoom-in code */
                    appContext->ac_ZoomPicX=xOffset;
                    appContext->ac_ZoomPicY=yOffset;
                    appContext->ac_ZoomPicW=outputWidth;
                    appContext->ac_ZoomPicH=outputHeight;
                    appContext->ac_ZoomCropX=xCrop;
                    appContext->ac_ZoomCropY=0;
                    appContext->ac_ZoomImgW=fPortrait?imageWidth:imageHeight;
                    appContext->ac_ZoomImgH=fPortrait?imageHeight:imageWidth;

                    /* Allocate line buffer */
                    lineBuffer=AllocVec(outputWidth,NULL);
                    if (lineBuffer) {

                        /* Display image */
                        /* N.B.: Rotation is counter-clockwise. */
                        rastPort=appContext->ac_DisplayWindow->RPort;

                        switch (imageNode->in_Rotation) {

                            case 0:
                                pRow=
                                    imageBuffer+magnifyX;
                                pixelStep=1;
                                rowStep=imageWidth;
                                break;

                            case 90:
                                pRow=imageBuffer+
                                    (imageWidth-1-magnifyX);
                                pixelStep=imageWidth;
                                rowStep=-1;
                                break;

                            case 180:
                                pRow=
                                    imageBuffer+
                                    (imageWidth*(pageHeight-1))+
                                    imageWidth-magnifyX-1;
                                pixelStep=-1;
                                rowStep=-imageWidth;
                                break;

                            case 270:
                                pRow=imageBuffer+
                                    (imageWidth*(pageHeight-1));
                                pixelStep=-imageWidth;
                                rowStep=1;
                                break;

                        }

                        /* Compute column step */
                        colStep=imageNode->in_Mirror?
                            -pixelStep:
                            pixelStep;

                        /* If NTSC ... */
                        if (fNTSC) {

                            for (y=0; y<outputHeight; y++) {

                                if (imageNode->in_Mirror) {

                                    pPixel=
                                        pRow+
                                        ((imageWidth-xCrop-1)*pixelStep);

                                } else {

                                    pPixel=pRow+(xCrop*pixelStep);

                                }
                                pOutput=lineBuffer;

                                fSkip=
                                    ((y+1)%QUICKSCALE_NTSC==0) &&
                                    (y<outputHeight-1);

                                if (fSkip) {

                                    for (x=0; x<outputWidth; x++) {

                                        pixelRGB=*pPixel;
                                        neighborRGB=*(pPixel+rowStep);
                                        RinRGB(pixelRGB)=
                                            (RinRGB(pixelRGB)+
                                            RinRGB(neighborRGB))/2;
                                        GinRGB(pixelRGB)=
                                            (GinRGB(pixelRGB)+
                                            GinRGB(neighborRGB))/2;
                                        BinRGB(pixelRGB)=
                                            (BinRGB(pixelRGB)+
                                            BinRGB(neighborRGB))/2;

                                        /* For first column ... */
                                        if (x==0) {

                                            /* Always use CLUT value */
#ifdef DISPLAY_FIXED_PALETTE
                                            pixelValue=quantizeFixedPal(
                                                appContext, pixelRGB);
                                            lastRGB=fixedPalRGB(
                                                appContext, pixelValue);
#else
                                            pixelValue=
                                                quantizeRGB(
                                                    appContext,
                                                    appContext->ac_QuantTree,
                                                    pixelRGB);
                                            lastRGB=
                                                appContext->ac_ColorTable[pixelValue];
#endif /* DISPLAY_FIXED_PALETTE */

                                        /* ... else ... */
                                        } else {

                                            /* Use better of HAM or CLUT */
                                            pixelValue=ham8Pick(appContext,
                                                pixelRGB, &lastRGB);

                                        }

                                        pPixel+=colStep;

                                        *pOutput=pixelValue;
                                        pOutput++;

                                    }

                                } else {

                                    for (x=0; x<outputWidth; x++) {

                                        /* Get pixel value */
                                        pixelRGB=*pPixel;

                                        /* For first column ... */
                                        if (x==0) {

                                            /* Always use CLUT value */
#ifdef DISPLAY_FIXED_PALETTE
                                            pixelValue=quantizeFixedPal(
                                                appContext, pixelRGB);
                                            lastRGB=fixedPalRGB(
                                                appContext, pixelValue);
#else
                                            pixelValue=
                                                quantizeRGB(
                                                    appContext,
                                                    appContext->ac_QuantTree,
                                                    *pPixel);
                                            lastRGB=
                                                appContext->ac_ColorTable[pixelValue];
#endif /* DISPLAY_FIXED_PALETTE */
                                        /* ... else ... */
                                        } else {

                                            /* Use better of HAM or CLUT */
                                            pixelValue=ham8Pick(appContext,
                                                *pPixel, &lastRGB);

                                        }

                                        pPixel+=colStep;

                                        *pOutput=pixelValue;
                                        pOutput++;

                                    }

                                }

                                /* Output line */
                                WriteChunkyPixels(rastPort,
                                    xOffset, y+yOffset,
                                    xOffset+outputWidth-1, y+yOffset,
                                    lineBuffer, outputWidth);

                                /* Update busy bar */
                                progress=(y*pageHeight)/outputHeight;
#ifdef DISPLAY_FIXED_PALETTE
                                interfaceBusyUpdate(appContext,
                                    pageHeight*2,
                                    pageHeight+progress);
#else
                                interfaceBusyUpdate(appContext,
                                    pageHeight*3,
                                    pageHeight*2+progress);
#endif /* DISPLAY_FIXED_PALETTE */
                                /* Advance to next row */
                                if (fSkip) {

                                    pRow+=rowStep*2;

                                } else {

                                    pRow+=rowStep;

                                }

                            }

                        } else {

                            /* PAL */
                            for (y=0; y<outputHeight; y++) {

                                if (imageNode->in_Mirror) {

                                    pPixel=
                                        pRow+
                                        (((fPortrait?imageHeight:imageWidth)-
                                         xCrop-1)*pixelStep);

                                } else {

                                    pPixel=pRow+(xCrop*pixelStep);

                                }
                                pOutput=lineBuffer;

                                for (x=0; x<outputWidth; x++) {

                                    /* For first column ... */
                                    if (x==0) {

                                        /* Always use CLUT value */
#ifdef DISPLAY_FIXED_PALETTE
                                        pixelValue=quantizeFixedPal(
                                            appContext, *pPixel);
                                        lastRGB=fixedPalRGB(
                                            appContext, pixelValue);
#else
                                        pixelValue=
                                            quantizeRGB(
                                                appContext,
                                                appContext->ac_QuantTree,
                                                *pPixel);
                                        lastRGB=
                                            appContext->ac_ColorTable[pixelValue];
#endif /* DISPLAY_FIXED_PALETTE */

                                    /* ... else ... */
                                    } else {

                                        /* Use better of HAM or CLUT */
                                        pixelValue=ham8Pick(appContext,
                                            *pPixel, &lastRGB);

                                    }

                                    pPixel+=colStep;

                                    *pOutput=pixelValue;
                                    pOutput++;

                                }

                                /* Output line */
                                WriteChunkyPixels(rastPort,
                                    xOffset, y+yOffset,
                                    xOffset+outputWidth-1, y+yOffset,
                                    lineBuffer, outputWidth);

                                /* Update busy bar */
                                progress=(y*pageHeight)/outputHeight;
#ifdef DISPLAY_FIXED_PALETTE
                                interfaceBusyUpdate(appContext,
                                    pageHeight*2,
                                    pageHeight+progress);
#else
                                interfaceBusyUpdate(appContext,
                                    pageHeight*3,
                                    pageHeight*2+progress);
#endif /* DISPLAY_FIXED_PALETTE */
                                /* Advance to next row */
                                pRow+=rowStep;

                            }

                        }

                        /* Reveal screen with sliding effect */
                        for (slide=0;
                             slide<appContext->ac_DisplayScreen->Height;
                             slide+=SCREEN_SLIDE_STEP) {

                            MoveScreen(appContext->ac_DisplayScreen,
                                0, -SCREEN_SLIDE_STEP);

                        }
                        MoveScreen(appContext->ac_DisplayScreen,
                            0,
                            -(appContext->ac_DisplayScreen->Height%SCREEN_SLIDE_STEP)
                        );

                        /* Success */
                        status=TRUE;

                        /* Release line buffer */
                        FreeVec(lineBuffer);

                    } else {

#ifdef DEBUG
                        printf("displayImage(): Error allocating line buffer\n");
#endif /* DEBUG */
                        status=TRUE;

                    }

#ifndef DISPLAY_FIXED_PALETTE
                    /* Release quantization */
                    releaseQuantization(appContext);
#endif /* DISPLAY_FIXED_PALETTE */

                } else {

#ifdef DEBUG
                    printf("displayImage(): obtainQuantization() failure\n");
#endif /* DEBUG */
                    status=FALSE;

                }

            } else {
#ifdef DEBUG
                printf("displayImage():GetPCDImageData() failure\n");
#endif /* DEBUG */
                status=FALSE;

            }


        } else {

#ifdef DEBUG
            printf("displayImage(): AllocPCDImageBuffer() failure\n");
#endif /* DEBUG */
            status=FALSE;

        }

    } else {

#ifdef DEBUG
        printf("displayImage(): GetPCDResolution() failure\n");
#endif /* DEBUG */
        status=FALSE;

    }

    /* Unbusy interface */
    interfaceBusyEnd(appContext);

    /* Return status */
    return status;

}

/****************************************************************************
 *                                                                          *
 *  lineFunc()  -   line callback hook for GetPCDImageData()                *
 *                                                                          *
 ****************************************************************************/

static ULONG __saveds __asm lineFunc(
    register __a0 struct Hook *hook,
    register __a2 APTR object,
    register __a1 APTR message
)
{

    struct appContext *appContext;
    UWORD total, current;

    /* Get application context */
    appContext=hook->h_Data;

    /* Fetch busy bar total from application context */
    total=appContext->ac_HookTotal;

    /* Message is pointer to busy bar current */
    current=*((ULONG *) message)-appContext->ac_HookStart;

    /* Update busy bar */
    interfaceBusyUpdate(appContext, total, current);

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  imageSelect()  -   select image in image state                          *
 *                                                                          *
 ****************************************************************************/

BOOL imageSelect(struct appContext *appContext,struct imageNode *newSelection)
{

    BOOL changed;

    if (appContext->ac_Selection!=newSelection) {

        /* Update selection */
        appContext->ac_Selection=newSelection;
        changed=TRUE;

    } else {

        changed=FALSE;

    }

    /* Update status panel */
    updateStatus(appContext);

    /* If selection has changed ... */
    if (changed) {

        /* Erase display */
        EraseRect(appContext->ac_DisplayWindow->RPort,
            0, 0,
            appContext->ac_DisplayWindow->Width-1,
            appContext->ac_DisplayWindow->Height-1);

        /* Update display */
        displayImage(appContext,appContext->ac_Selection);

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  imageNextImage()  -   go to next image in image state                   *
 *                                                                          *
 ****************************************************************************/

void imageNextImage(struct appContext *appContext)
{

    struct imageNode *newSelection;

    /* Find next image, wrapping around to first */
    newSelection=(struct imageNode *) appContext->ac_Selection->in_Node.mln_Succ;
    if (newSelection->in_Node.mln_Succ==NULL) {

        newSelection=(struct imageNode *) appContext->ac_ImageList.mlh_Head;

    }

    /* Update selection */
    imageSelect(appContext,newSelection);

}

/****************************************************************************
 *                                                                          *
 *  imagePreviousImage()  -  go to previous image in image state            *
 *                                                                          *
 ****************************************************************************/

void imagePreviousImage(struct appContext *appContext)
{

    struct imageNode *newSelection;

    /* Find previous image, wrapping around to last */
    newSelection=(struct imageNode *) appContext->ac_Selection->in_Node.mln_Pred;
    if (newSelection==(struct imageNode *) &appContext->ac_ImageList) {

        newSelection=(struct imageNode *) appContext->ac_ImageList.mlh_TailPred;

    }

    /* Update selection */
    imageSelect(appContext,newSelection);

}


/****************************************************************************
 *                                                                          *
 *  imageThumbnail()  -  switch to thumbnail state from image state         *
 *                                                                          *
 ****************************************************************************/

void imageThumbnail(struct appContext *appContext)
{

    /* Enter Thumbnail state */
    newState(appContext,as_Thumbnail);

}

/****************************************************************************
 *                                                                          *
 *  imageControls()  -  set-up control panel for image state                *
 *                                                                          *
 ****************************************************************************/

void imageControls(struct appContext *appContext)
{

    /* Control array */
    static struct controlItem controlArray[] ={

        {
            ct_Button,
            IMAGE_THUMBNAIL,
            imageThumbnail,
            163, 5,
            65, 65,
            GLYPH_THUMBNAIL,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_BLUE, RAWKEY_PORT1_BUTTON_BLUE, RAWKEY_ESC, NULL },
        },
        {
            ct_Button,
            IMAGE_PREV_IMAGE,
            imagePreviousImage,
            234, 5,
            65, 65,
            GLYPH_PREV_IMAGE,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_REVERSE, RAWKEY_PORT1_BUTTON_REVERSE, NULL },
        },
        {
            ct_Button,
            IMAGE_NEXT_IMAGE,
            imageNextImage,
            305, 5,
            65, 65,
            GLYPH_NEXT_IMAGE,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_FORWARD, RAWKEY_PORT1_BUTTON_FORWARD, NULL },
        },
        {
            ct_Button,
            IMAGE_MANIPULATE,
            manipulateDo,
            376, 5,
            65, 65,
            GLYPH_MANIPULATE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            IMAGE_SLIDE,
            slideDo,
            447, 5,
            65, 65,
            GLYPH_SLIDE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            IMAGE_INTERFACE,
            interfaceToggle,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_GREEN, RAWKEY_PORT1_BUTTON_GREEN, RAWKEY_HELP, NULL },
        },
        {
            ct_End
        }

    };

    /* Set controls, with thumbnail as default */
    setControls(appContext, controlArray, &(controlArray[0]));

}
