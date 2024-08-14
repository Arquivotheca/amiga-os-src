/*** thumbnail.c ************************************************************
 *
 *  $Id: thumbnail.c,v 1.5 94/03/17 16:28:01 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Thumbnail Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   thumbnail.c,v $
 * Revision 1.5  94/03/17  16:28:01  jjszucs
 * Global meta-image functions implemented
 *
 * Revision 1.4  94/03/08  14:00:57  jjszucs
 *
 *
 * Revision 1.3  94/02/18  15:58:41  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:06:46  jjszucs
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
 * Revision 1.1  94/01/06  11:58:38  jjszucs
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

/* Amiga includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>

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

/****************************************************************************
 *                                                                          *
 *  loadThumbnail() - load thumbnail for an image                           *
 *                                                                          *
 ****************************************************************************/

/* N.B.: This routine is specialized for a 1/2 x 1/2 reduction of source
         thumbnail data (192x128) to destination thumbnail data (96x64). */

UBYTE *loadThumbnail(struct appContext *appContext,
    struct imageNode *imageNode)
{

    UBYTE *thumbnail;

    UBYTE *imageBuffer;

    UWORD srcWidth, srcHeight;
    UWORD destWidth, destHeight;

    WORD  srcPixelsPerRow, srcPixelsPerCol;

    UWORD destX, destY;

    ULONG *pSrcRow, *pSrc;
    UBYTE *pDestRow, *pDest;

    ULONG pixelValue, mappedPixel;
    UBYTE thisPixelValue;

    /* If thumbnail is already loaded ... */
    if (imageNode->in_Thumbnail) {
        /* Return thumbnail */
        return imageNode->in_Thumbnail;
    }

    /* Get source width/height */
    GetPCDResolution(PHOTOCD_RES_BASE16,&srcWidth,&srcHeight);

    /* Allocate thumbnail */
    if (thumbnail=AllocVec(THUMBNAIL_WIDTH*THUMBNAIL_HEIGHT,NULL)) {

        /* Allocate image buffer */
        if (imageBuffer=AllocPCDImageBuffer(PHOTOCD_RES_BASE16,TAG_DONE)) {

            /* Load image data */
            if (GetPCDImageData(appContext->ac_PhotoCDHandle,
                imageBuffer,
                PCD_Image, imageNode->in_Number,
                PCD_Resolution, PHOTOCD_RES_BASE16,
                PCD_Format, PHOTOCD_FORMAT_YUV,
                TAG_DONE)) {

                /* Compute destination dimensions */
                destWidth=(imageNode->in_Rotation%180) ?
                                (srcHeight/2) : (srcWidth/2);
                destHeight=(imageNode->in_Rotation%180) ?
                                (srcWidth/2) : (srcHeight/2);

                /* Initialize source parameters for copy */
                switch (imageNode->in_Rotation) {

                    /* 0o rotation */
                    case 0:
                        pSrcRow=(ULONG *) imageBuffer;
                        srcPixelsPerCol=1;
                        srcPixelsPerRow=srcWidth;
                        break;

                    /* 90o CCW rotation */
                    case 90:
                        pSrcRow=(ULONG *) imageBuffer+srcWidth-1;
                        srcPixelsPerCol=srcWidth;
                        srcPixelsPerRow=-1;
                        break;

                    /* 180o CCW rotation */
                    case 180:
                        pSrcRow=(ULONG *) imageBuffer+
                            (srcWidth*srcHeight)-1;
                        srcPixelsPerCol=-1;
                        srcPixelsPerRow=-srcWidth;
                        break;

                    /* 270o CCW rotation */
                    case 270:
                        pSrcRow=(ULONG *) imageBuffer+
                            (srcWidth*(srcHeight-1));
                        srcPixelsPerCol=-srcWidth;
                        srcPixelsPerRow=1;
                        break;

                }

                /* Begin on first row of thumbnail */
                pDestRow=thumbnail;

                /* Y-axis loop */
                for (destY=0;destY<destHeight;destY++) {

                    /* Begin at first column of source row */
                    pSrc=pSrcRow;

                    /* Write to this destination row */
                    if (imageNode->in_Mirror) {
                        pDest=pDestRow+destWidth-1;
                    } else {
                        pDest=pDestRow;
                    }

                    /* X-axis loop */
                    for (destX=0;destX<destWidth;destX++) {

                        /* For each destination pixel, value is luminance (only) of
                           corresponding four source pixels. This is intended
                           to produce an 8-bit grayscale thumbnail. */
                        pixelValue=0;

                        thisPixelValue=YfromYCC(*pSrc);
                        pixelValue+=thisPixelValue;

                        thisPixelValue=YfromYCC(*(pSrc+srcPixelsPerCol));
                        pixelValue+=thisPixelValue;

                        thisPixelValue=YfromYCC(*(pSrc+srcPixelsPerRow));
                        pixelValue+=thisPixelValue;

                        thisPixelValue=YfromYCC(*(pSrc+srcPixelsPerRow+srcPixelsPerRow));
                        pixelValue+=thisPixelValue;

                        pixelValue/=4;

                        /* Map output pixel value to grayscale in pens 16...255 */
                        mappedPixel=16+((pixelValue*240)/256);

                        /* Write mapped pixel value */
                        *pDest=(UBYTE) mappedPixel;
                        if (imageNode->in_Mirror) {
                            pDest--;
                        } else {
                            pDest++;
                        }

                        pSrc+=srcPixelsPerCol*2;

                    }

                    pSrcRow+=srcPixelsPerRow*2;
                    pDestRow+=destWidth;

                }

            } else {

                FreeVec(thumbnail);
                thumbnail=NULL;

            }

            /* Release image buffer */
            FreePCDImageBuffer(imageBuffer);

        } else {

            FreeVec(thumbnail);
            thumbnail=NULL;

        }

    }

    /* Store thumbnail in image node */
    imageNode->in_Thumbnail=thumbnail;

    /* Return thumbnail */
    return thumbnail;

}

/****************************************************************************
 *                                                                          *
 *  unloadThumbnail() - unload thumbnail for an image                       *
 *                                                                          *
 ****************************************************************************/

void unloadThumbnail(struct appContext *appContext,struct imageNode *imageNode)
{

    /* Release thumbnail */
    FreeVec(imageNode->in_Thumbnail);
    imageNode->in_Thumbnail=NULL;
    unlockThumbnail(appContext, imageNode);

}

/****************************************************************************
 *                                                                          *
 *  lockThumbnail() - lock thumbnail against expunge                        *
 *                                                                          *
 ****************************************************************************/

void lockThumbnail(struct appContext *appContext, struct imageNode *imageNode)
{

    /* Set lock flag */
    imageNode->in_ThumbnailLock=TRUE;

}

/****************************************************************************
 *                                                                          *
 *  unlockThumbnail() - unlock thumbnail against expunge                    *
 *                                                                          *
 ****************************************************************************/

void unlockThumbnail(struct appContext *appContext, struct imageNode *imageNode)
{

    /* Clear lock flag */
    imageNode->in_ThumbnailLock=FALSE;

}

/****************************************************************************
 *                                                                          *
 *  thumbnailGridSize() - compute size of thumbnail grid                    *
 *                                                                          *
 ****************************************************************************/

void thumbnailGridSize(struct appContext *appContext,
    UWORD *pnRows,UWORD *pnColumns)
{

    /* Compute number of visible rows */
    *pnRows=
        (appContext->ac_DisplayWindow->Height-
            THUMBNAIL_VERT_SPACING-
            INTERFACE_PANE_HEIGHT)/
        (THUMBNAIL_FRAME_HEIGHT+THUMBNAIL_VERT_SPACING);

    /* Compute number of visible columns */
    *pnColumns=
        (appContext->ac_DisplayWindow->Width-THUMBNAIL_HORIZ_SPACING)/
        (THUMBNAIL_FRAME_WIDTH+THUMBNAIL_HORIZ_SPACING);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailGridPosition() - compute placement of thumbnail in thumbnail   *
 *                            grid in row/column terms                      *
 *                                                                          *
 ****************************************************************************/

void thumbnailGridPosition(struct appContext *appContext,
    struct imageNode *imageNode, UWORD *pRow, UWORD *pColumn)
{

    UWORD nRows, nColumns;
    UWORD firstSlideNumber, thisSlideNumber;
    UWORD row, column;


    /* Compute number of rows and columns */
    thumbnailGridSize(appContext,&nRows,&nColumns);

    /* Get number of first slide and this slide */
    firstSlideNumber=appContext->ac_Top?
        appContext->ac_Top->in_Sequence:
        0;
    thisSlideNumber=imageNode?
        imageNode->in_Sequence:
        0;

    /* Compute row and column of this image */
    row=(thisSlideNumber-firstSlideNumber)/nColumns;
    column=(thisSlideNumber-firstSlideNumber)%nColumns;

    /* Return position */
    *pRow=row;
    *pColumn=column;

}

/****************************************************************************
 *                                                                          *
 *  thumbnailGridPlace() - compute placement of thumbnail in thumbnail grid *
 *                         in pixels                                        *
 *                                                                          *
 ****************************************************************************/

void thumbnailGridPlace(struct appContext *appContext,struct imageNode *imageNode,
    UWORD *pX,UWORD *pY)
{

    UWORD row, column;
    UWORD x, y;

    /* Compute row/column position */
    thumbnailGridPosition(appContext, imageNode, &row, &column);

    /* Compute placement */
    x=
        THUMBNAIL_HORIZ_SPACING/2+
        column*(THUMBNAIL_FRAME_WIDTH+THUMBNAIL_HORIZ_SPACING);
    y=
        THUMBNAIL_VERT_SPACING/2+
        row*(THUMBNAIL_FRAME_HEIGHT+THUMBNAIL_VERT_SPACING);

    /* Return placement */
    *pX=x;
    *pY=y;

}

/****************************************************************************
 *                                                                          *
 *  putThumbnailFrame() - put thumbnail frame on display                    *
 *                                                                          *
 ****************************************************************************/

BOOL putThumbnailFrame(struct appContext *appContext,
    struct imageNode *imageNode, UWORD row,UWORD column)
{

    UWORD x, y;

    struct RastPort *rastport;

    UWORD glyphID;

    /* Fetch display rastport */
    rastport=appContext->ac_DisplayWindow->RPort;

    /* Compute X/Y position for this cell */
    x=THUMBNAIL_HORIZ_SPACING/2+column*(THUMBNAIL_FRAME_WIDTH+THUMBNAIL_HORIZ_SPACING);
    y=THUMBNAIL_VERT_SPACING/2+row*(THUMBNAIL_FRAME_HEIGHT+THUMBNAIL_VERT_SPACING);

    /* If displaying global icon ... */
    if (imageNode==NULL) {

        /* Show Photo CD glyph */
        if (!putGlyph(appContext,GLYPH_PHOTOCD_LOGO,rastport,x,y)) {

#ifdef DEBUG
            printf("putThumbnail(): Error putting Photo CD logo\n");
#endif /* DEBUG */

            return FALSE;

        }

    /* ... else (displaying thumbnail) ... */
    } else {

        glyphID=imageNode->in_IsSlide?
            (imageNode->in_Rotation % 180 ?
                GLYPH_SLIDE_FRAME_VERT : GLYPH_SLIDE_FRAME_HORIZ ) :
            (imageNode->in_Rotation % 180 ?
                GLYPH_IMAGE_FRAME_VERT : GLYPH_IMAGE_FRAME_HORIZ );

        /* Display slide or thumbnail frame */
        if (!putGlyph(appContext,
            glyphID,
            rastport,x,y)) {

#ifdef DEBUG
            printf("putThumbnail(): Error putting frame glyph\n");
#endif /* DEBUG */

            return FALSE;

        }

    }

    /* Success */

}

/****************************************************************************
 *                                                                          *
 *  putThumbnail() - put thumbnail on display                               *
 *                                                                          *
 ****************************************************************************/

BOOL putThumbnail(struct appContext *appContext,struct imageNode *imageNode,
    UWORD row,UWORD column)
{

    UWORD x, y;
    UWORD startX, startY, stopX, stopY;
    UWORD thumbnailWidth, thumbnailHeight;

    struct RastPort *rastport;

    UBYTE *thumbnail;

    UWORD glyphID;

    /* Fetch display rastport */
    rastport=appContext->ac_DisplayWindow->RPort;

    /* Compute X/Y position for this cell */
    x=THUMBNAIL_HORIZ_SPACING/2+column*(THUMBNAIL_FRAME_WIDTH+THUMBNAIL_HORIZ_SPACING);
    y=THUMBNAIL_VERT_SPACING/2+row*(THUMBNAIL_FRAME_HEIGHT+THUMBNAIL_VERT_SPACING);

    /* N.B.: This call forcibly erases the old thumbnail. This is
             necessary because the thumbnail/slide frames are only 4-bitplanes
             deep, while the images within the thumbnails are 8-bitplanes
             deep. Thanks for fredness for pointing this out. */
    EraseRect(
        rastport,
        x, y,
        x+THUMBNAIL_FRAME_WIDTH-1, y+THUMBNAIL_FRAME_HEIGHT-1);

    /* If displaying global icon ... */
    if (imageNode==NULL) {

        /* Show Photo CD glyph */
        if (!putGlyph(appContext,GLYPH_PHOTOCD_LOGO,rastport,x,y)) {

#ifdef DEBUG
            printf("putThumbnail(): Error putting Photo CD logo\n");
#endif /* DEBUG */

            return FALSE;

        }

    /* ... else (displaying thumbnail) ... */
    } else {

        glyphID=imageNode->in_IsSlide?
            (imageNode->in_Rotation % 180 ?
                GLYPH_SLIDE_FRAME_VERT : GLYPH_SLIDE_FRAME_HORIZ ) :
            (imageNode->in_Rotation % 180 ?
                GLYPH_IMAGE_FRAME_VERT : GLYPH_IMAGE_FRAME_HORIZ );

        /* Display slide or thumbnail frame */
        if (!putGlyph(appContext,
            glyphID,
            rastport, x, y)) {

#ifdef DEBUG
            printf("putThumbnail(): Error putting frame glyph\n");
#endif /* DEBUG */

            return FALSE;

        }

        /* Lock thumbnail to prevent expunge */
        lockThumbnail(appContext, imageNode);

        /* Load thumbnail */
        if (!(thumbnail=loadThumbnail(appContext,imageNode))) {

#ifdef DEBUG
            printf("putThumbnail(): Error loading thumbnail\n");
#endif /* DEBUG */

            return FALSE;

        }

        /* Display thumbnail in appropriate orientation */
        thumbnailWidth=(imageNode->in_Rotation%180)?
            THUMBNAIL_HEIGHT:THUMBNAIL_WIDTH;
        thumbnailHeight=(imageNode->in_Rotation%180)?
            THUMBNAIL_WIDTH:THUMBNAIL_HEIGHT;
        startX=x+
            ((imageNode->in_Rotation%180)?
                VERT_THUMBNAIL_OFFSET_X:HORIZ_THUMBNAIL_OFFSET_X);
        startY=y+
            ((imageNode->in_Rotation%180)?
                VERT_THUMBNAIL_OFFSET_Y:HORIZ_THUMBNAIL_OFFSET_Y);
        stopX=startX+thumbnailWidth-1;
        stopY=startY+thumbnailHeight-1;

        WriteChunkyPixels(rastport,
            startX, startY,
            stopX, stopY,
            thumbnail, /* Array */
            thumbnailWidth /* BytesPerRow */);

        /* Unlock thumbnail - may now be expunged */
        unlockThumbnail(appContext, imageNode);

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  thumbnailCursor() - render thumbnail cursor (in display pane)           *
 *                                                                          *
 *  INPUTS                                                                  *
 *      appContext  -   application context                                 *
 *      imageNode   -   image node to render thumbnail cursor for           *
 *      show        -   TRUE to show cursor; FALSE to hide cursor           *
 *                                                                          *
 ****************************************************************************/

BOOL thumbnailCursor(struct appContext *appContext,struct imageNode *imageNode,
    BOOL show)
{

    UWORD minX, minY;
    UWORD maxX, maxY;

    struct RastPort *rastport;

    /* Compute corners */
    thumbnailGridPlace(appContext, imageNode, &minX,&minY);
    minX-=CURSOR_THICKNESS;
    minY-=CURSOR_THICKNESS;
    maxX=minX+THUMBNAIL_FRAME_WIDTH+CURSOR_THICKNESS;
    maxY=minY+THUMBNAIL_FRAME_HEIGHT+CURSOR_THICKNESS;

    /* Fetch display rastport */
    rastport=appContext->ac_DisplayWindow->RPort;

    /* Use two-pen complement drawing mode */
    SetDrMd(rastport,JAM2);

    /* Select pens */
    SetAPen(rastport,show?
        INTERFACE_PEN_CURSOR:
        INTERFACE_PEN_BACKGROUND);
    SetOPen(rastport,show?
        INTERFACE_PEN_CURSOR:
        INTERFACE_PEN_BACKGROUND);

    /* Draw upper edge */
    RectFill(rastport,
        minX, minY,
        maxX+CURSOR_THICKNESS-1, minY+CURSOR_THICKNESS-1);

    /* Draw left edge */
    RectFill(rastport,
        minX,minY,
        minX+CURSOR_THICKNESS-1,maxY+CURSOR_THICKNESS-1);

    /* Draw bottom edge */
    RectFill(rastport,
        minX,maxY,
        maxX+CURSOR_THICKNESS-1,maxY+CURSOR_THICKNESS-1);

    /* Draw right edge */
    RectFill(rastport,
        maxX,minY,
        maxX+CURSOR_THICKNESS-1,maxY+CURSOR_THICKNESS-1);

    /* Success */
    return TRUE;

}


/****************************************************************************
 *                                                                          *
 *  thumbnailScroll() - scroll thumbnail panel to make a slide visible      *
 *                                                                          *
 ****************************************************************************/

BOOL thumbnailScroll(struct appContext *appContext,struct imageNode *imageNode)
{

    struct RastPort *rastport;

    UWORD nRows, nColumns;
    UWORD row, column;

    UWORD firstSlideNumber, lastSlideNumber, thisSlideNumber;

    WORD nScrollRows, nScrollCounter;
    WORD scrollStep;

    UWORD displayMinX, displayMinY, displayMaxX, displayMaxY;

    UWORD nFirstRowSlide, nNewTop;

    struct imageNode *thisImageNode;

    /* Compute number of rows and columns */
    thumbnailGridSize(appContext,&nRows,&nColumns);

    /* Get number of first visible image, last visible image, and this image */
    firstSlideNumber=appContext->ac_Top?
        appContext->ac_Top->in_Sequence:
        0;
    lastSlideNumber=firstSlideNumber+(nRows*nColumns)-1;
    thisSlideNumber=imageNode?
        imageNode->in_Sequence:
        0;

    /* If slide is already visible ... */
    if (thisSlideNumber>=firstSlideNumber &&
        thisSlideNumber<=firstSlideNumber+(nRows*nColumns)-1) {

        /* No action */
        return TRUE;

    }

    /* Fetch RastPort for display window */
    rastport=appContext->ac_DisplayWindow->RPort;

    /* Compute display bounds */
    displayMinX=THUMBNAIL_HORIZ_SPACING/2;
    displayMinY=THUMBNAIL_VERT_SPACING/2;
    displayMaxX=displayMinX+
        nColumns*(THUMBNAIL_FRAME_WIDTH+THUMBNAIL_HORIZ_SPACING);
    displayMaxY=displayMinY+
        (nRows*THUMBNAIL_FRAME_HEIGHT)+((nRows-1)*THUMBNAIL_VERT_SPACING);

    /* Compute number of rows to scroll */
    if (thisSlideNumber<firstSlideNumber) {
        nScrollRows=(thisSlideNumber-firstSlideNumber)/nColumns;

        if ( (thisSlideNumber-firstSlideNumber)%nColumns) {
            nScrollRows-=1;
        }

    } else {

        nScrollRows=(thisSlideNumber-lastSlideNumber)/nColumns;
        if ( ( thisSlideNumber-lastSlideNumber)%nColumns) {
            nScrollRows+=1;
        }

    }

    /* Compute scroll step */
    scrollStep=nScrollRows>=0 ? 1 : -1;

    /* Compute number of first slide on first new row */
    nFirstRowSlide=scrollStep>=0?
        lastSlideNumber+1:
        firstSlideNumber-nColumns;

    /* Compute number of first new top slide */
    nNewTop=firstSlideNumber+(nColumns*scrollStep);

    /* If incremental scrolling or only one line to scroll ... */
    if (
#ifdef THUMBNAIL_INCREMENTAL_SCROLL
        TRUE
#else
        nScrollRows==1 || nScrollRows==-1
#endif /* THUMBNAIL_INCREMENTAL_SCROLL */
    ) {

        /* Loop through rows to scroll */
        for (nScrollCounter=0;
             nScrollCounter!=nScrollRows;
             nScrollCounter+=scrollStep) {

            /* Scroll new row into view */
            ScrollRaster(rastport,
                0, /* deltaX */
                scrollStep*(THUMBNAIL_FRAME_HEIGHT+THUMBNAIL_VERT_SPACING), /* deltaY */
                displayMinX, displayMinY, /* xmin/ymin */
                displayMaxX, displayMaxY /* xmax/ymax */
            );

            /* Set new top slide */
            appContext->ac_Top=imageBySequence(appContext, nNewTop);

            /* Compute row number of this row */
            row=(scrollStep>=0)?
                (nRows-1):
                0;

            /* Render new slides on this row */
            for (column=0;column<nColumns;column++) {

                thisImageNode=imageBySequence(appContext, nFirstRowSlide+column);
                if (thisImageNode || (nFirstRowSlide==0 && column==0)) {

                    putThumbnail(appContext,thisImageNode,row,column);

                }

            }

            /* Update number of first slide on new row */
            nFirstRowSlide+=nColumns*scrollStep;

            /* Update number of new top slide */
            nNewTop+=nColumns*scrollStep;

            /* Remove layers damage */
            BeginRefresh(appContext->ac_DisplayWindow);
            EndRefresh(appContext->ac_DisplayWindow,TRUE);

        }

    /* ... else ... */
    } else {

        /* Compute number of new top slide */
        nNewTop=nColumns*nScrollRows;

        /* Set new top slide */
        appContext->ac_Top=imageBySequence(appContext, nNewTop);

        /* Erase display panel */
        EraseRect(
            appContext->ac_DisplayWindow->RPort, 0, 0,
            appContext->ac_DisplayWindow->Width-1,
            appContext->ac_DisplayWindow->Height-1);

        /* Begin with first visible image */
        thisImageNode=appContext->ac_Top;

        /* Loop through rows */
        for (row=0;row<nRows;row++) {

            /* Loop through columns */
            for (column=0;column<nColumns;column++) {

                /* If not at end of list ... */
                if (thisImageNode==NULL ||
                    thisImageNode->in_Node.mln_Succ) {

                    /* Put thumbnail */
                    putThumbnail(appContext,
                        thisImageNode,
                        row,column);

                    /* If this image node was global ... */
                    if (thisImageNode==NULL) {

                        /* Go to first image */
                        thisImageNode=
                            (struct imageNode *)
                                appContext->ac_ImageList.mlh_Head;

                    /* ... else (not on global icon) ... */
                    } else {

                        /* Go to next image */
                        thisImageNode=
                            (struct imageNode *)
                                thisImageNode->in_Node.mln_Succ;

                    }

                }

            }

        }

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  thumbnailSelect() - select image in thumbnail state                     *
 *                                                                          *
 ****************************************************************************/

BOOL thumbnailSelect(struct appContext *appContext,struct imageNode *newSelection)
{

    /* Erase old cursor */
    thumbnailCursor(appContext,appContext->ac_Selection,FALSE);

    /* Update selection */
    appContext->ac_Selection=newSelection;

    /* Scroll thumbnail display to make new selection visible */
    thumbnailScroll(appContext,newSelection);

    /* Draw new cursor */
    thumbnailCursor(appContext, newSelection, TRUE);

    /* Update status panel */
    updateStatus(appContext);

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  thumbnailNextImage() - go to next image in thumbnail state              *
 *                                                                          *
 ****************************************************************************/

void thumbnailNextImage(struct appContext *appContext)
{

    struct imageNode *newSelection;

    /* Find next image, wrapping around to global */
    if (appContext->ac_Selection==NULL) {

        newSelection=(struct imageNode *) appContext->ac_ImageList.mlh_Head;

    } else {

        newSelection=(struct imageNode *)
            appContext->ac_Selection->in_Node.mln_Succ;
        if (newSelection->in_Node.mln_Succ==NULL) {

            newSelection=NULL;

        }

    }

    /* Update selection */
    thumbnailSelect(appContext,newSelection);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailPreviousImage() - go to previous image in thumbnail state      *
 *                                                                          *
 ****************************************************************************/

void thumbnailPreviousImage(struct appContext *appContext)
{

    struct imageNode *newSelection;

    /* Find previous image, wrapping around to last */
    if (appContext->ac_Selection==NULL) {

        newSelection=
            (struct imageNode *) appContext->ac_ImageList.mlh_TailPred;

    } else {

        newSelection=
            (struct imageNode *) appContext->ac_Selection->in_Node.mln_Pred;

        /* If going before first image ... */
        if (newSelection==(struct imageNode *) &appContext->ac_ImageList) {

            /* Select meta-thumbnail */
            newSelection=NULL;

        }

    }

    /* Update selection */
    thumbnailSelect(appContext,newSelection);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailNextRow() - go to next row in thumbnail state                  *
 *                                                                          *
 ****************************************************************************/

void thumbnailNextRow(struct appContext *appContext)
{

    struct imageNode *newSelection;
    UWORD nSelection;

    UWORD nRows, nColumns;

    UWORD nRow, nColumn;

    /* Compute number of rows and columns */
    thumbnailGridSize(appContext,&nRows,&nColumns);

    /* Compute sequence number of current selection */
    nSelection=appContext->ac_Selection?
        appContext->ac_Selection->in_Sequence:
        0;

    /* Compute logical row and column */
    nRow=nSelection/nColumns;
    nColumn=nSelection%nColumns;

    /* If on last row ... */
    if (nRow==appContext->ac_NImages/nColumns) {

        /* Wrap-around to first row */
        nRow=0;

    /* ... else ... */
    } else {

        /* Go to next row */
        nRow++;

    }

    /* Limit column to number of slides in row */
    if (nRow*nColumns+nColumn>appContext->ac_NImages) {
        nColumn=appContext->ac_NImages-nRow*nColumns;
    }

    /* Compute slide number of new selection,
       wrapping around to start from end */
    nSelection=
        nRow*nColumns+
        nColumn;

    /* Update selection */
    newSelection=imageBySequence(appContext, nSelection);
    thumbnailSelect(appContext, newSelection);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailPreviousRow() - go to previous row in thumbnail state          *
 *                                                                          *
 ****************************************************************************/

void thumbnailPreviousRow(struct appContext *appContext)
{

    struct imageNode *newSelection;
    UWORD nSelection;

    UWORD nRows, nColumns;

    UWORD nRow, nColumn;

    /* Compute number of rows and columns */
    thumbnailGridSize(appContext,&nRows,&nColumns);

    /* Compute sequence number of current selection */
    nSelection=appContext->ac_Selection?
        appContext->ac_Selection->in_Sequence:
        0;

    /* Compute logical row and column */
    nRow=nSelection/nColumns;
    nColumn=nSelection%nColumns;

    /* If on first row ... */
    if (nRow==0) {

        /* Wrap-around to last row */
        nRow=appContext->ac_NImages/nColumns;

    /* ... else ... */
    } else {

        /* Go to previous row */
        nRow--;

    }

    /* Limit column to number of slides in row */
    if (nRow*nColumns+nColumn>appContext->ac_NImages) {
        nColumn=appContext->ac_NImages-nRow*nColumns;
    }

    /* Compute slide number of new selection,
       wrapping around to start from end */
    nSelection=
        nRow*nColumns+
        nColumn;

    /* Update selection */
    newSelection=imageBySequence(appContext, nSelection);
    thumbnailSelect(appContext, newSelection);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailImage() - show selected thumbnail as full-screen image         *
 *                                                                          *
 ****************************************************************************/

void thumbnailImage(struct appContext *appContext)
{

    /* Enter Image state */
    newState(appContext,as_Image);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailDo() - operate on selected thumbnail                           *
 *                                                                          *
 ****************************************************************************/

void thumbnailDo(struct appContext *appContext)
{

    /* Thumbnail control strip */
    static struct controlItem controlArray[] ={

        {
            ct_Button,
            THUMBNAIL_OP_IMAGE,
            thumbnailImage,
            163, 5,
            65, 65,
            GLYPH_IMAGE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            THUMBNAIL_OP_PREV_IMAGE,
            thumbnailPreviousImage,
            234, 5,
            65, 65,
            GLYPH_PREV_IMAGE,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_REVERSE, RAWKEY_PORT1_BUTTON_REVERSE, NULL },
        },
        {
            ct_Button,
            THUMBNAIL_OP_NEXT_IMAGE,
            thumbnailNextImage,
            305, 5,
            65, 65,
            GLYPH_NEXT_IMAGE,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_FORWARD, RAWKEY_PORT1_BUTTON_FORWARD, NULL },
        },
        {
            ct_Button,
            THUMBNAIL_OP_SLIDE,
            slideDo,
            377, 5,
            65, 65,
            GLYPH_SLIDE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            THUMBNAIL_OP_CANCEL,
            thumbnailCancel,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_BLUE, RAWKEY_PORT1_BUTTON_BLUE, RAWKEY_ESC, NULL },
        },
        {
            ct_Button,
            APP_EXIT,
            thumbnailExit,
            449, 5,
            65, 65,
            GLYPH_EXIT,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_End
        }

    };

    /* Hide exit button if exit capability is off or
       not on Global meta-image */
    controlArray[5].cduic_Type=
        (appContext->ac_EnableExit && appContext->ac_Selection==NULL)?
            ct_Button:
            ct_End;

    /* Set controls, with image as default for normal images, or
       slide as default for global meta-image */
    setControls(appContext, controlArray,
        appContext->ac_Selection ?
            &(controlArray[0]):
            &(controlArray[3]));

    /* Thumbnail|Image only enabled for true images (not global meta-image) */
    controlEnable(appContext, THUMBNAIL_OP_IMAGE,
        appContext->ac_Selection?TRUE:FALSE);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailCancel() - cancel thumbnail operations                         *
 *                                                                          *
 ****************************************************************************/

void thumbnailCancel(struct appContext *appContext)
{

    /* Return to normal control panel for state */
    newControl(appContext, appContext->ac_State);

}

/****************************************************************************
 *                                                                          *
 *  thumbnailExit() - exit                                                  *
 *                                                                          *
 ****************************************************************************/

void thumbnailExit(struct appContext *appContext)
{

#ifdef DEBUG
    kprintf("thumbnailExit(appContext=$%08lx)\n", appContext);
    kprintf("   ac_EnableExit=%d\n", appContext->ac_EnableExit);
#endif /* DEBUG */

    /* If exit capability is enabled ... */
    if (appContext->ac_EnableExit) {

        /* Terminate application with OK return code */
        goodbye(appContext, RETURN_OK);

    }

}

/****************************************************************************
 *                                                                          *
 *  thumbnailControls() - set-up thumbnail controls                         *
 *                                                                          *
 ****************************************************************************/

void thumbnailControls(struct appContext *appContext)
{

    /* Control array */
    static struct controlItem controlArray[] ={

        {
            ct_Button,
            THUMBNAIL_PREV_ROW,
            thumbnailPreviousRow,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_UP, RAWKEY_PORT1_JOY_UP, RAWKEY_UP, NULL }
        },
        {
            ct_Button,
            THUMBNAIL_NEXT_ROW,
            thumbnailNextRow,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_DOWN, RAWKEY_PORT1_JOY_DOWN, RAWKEY_DOWN, NULL }
        },
        {
            ct_Button,
            THUMBNAIL_PREV_IMAGE,
            thumbnailPreviousImage,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_LEFT, RAWKEY_PORT1_JOY_LEFT,
              RAWKEY_PORT0_BUTTON_REVERSE, RAWKEY_PORT1_BUTTON_REVERSE,
              RAWKEY_LEFT, NULL }
        },
        {
            ct_Button,
            THUMBNAIL_NEXT_IMAGE,
            thumbnailNextImage,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_RIGHT, RAWKEY_PORT1_JOY_RIGHT,
              RAWKEY_PORT0_BUTTON_FORWARD, RAWKEY_PORT1_BUTTON_FORWARD,
              RAWKEY_RIGHT, NULL }
        },
        {
            ct_Button,
            THUMBNAIL_DO,
            thumbnailDo,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_RED, RAWKEY_PORT1_BUTTON_RED,
                RAWKEY_RETURN, NULL }
        },
        {
            ct_End
        }

    };

    /* Set thumbnail controls */
    setControls(appContext, controlArray, NULL);

}
