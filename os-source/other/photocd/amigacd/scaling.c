/*** scaling.c ***************************************************************
 *
 *  $Id: scaling.c,v 1.3 94/03/01 16:15:28 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Image Scaling Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*

A Simple Algorithm for Scaling of Images
by  John J. Szucs (jjszucs@commodore.com)
and Darren M. Greenwald (darren@commodore.com)

Given:

    srcImage    =   source image buffer (assumed to be 24-bit RGB)
    srcWidth    =   width of source image
    srcHeight   =   height of source image

    destImage   =   destination image buffer (in same format as srcImage)
    destWidth   =   width of destination image
    destHeight  =   height of destination image

1)  Each destination pixel must contain srcWidth horizontal units
    and srcHeight destination units.

2)  A complete horizontal source pixel contributes destWidth
    horizontal units. A partial horizontal source pixel contributes
    0 < contribX < destWidth horizontal units, where contribX is
    the horizontal contribution of the source pixel.

3)  A complete vertical source pixel contributes destHeight
    vertical units. A partial vertical source pixel contributes
    0 < contribY < destHeight horizontal units, where contribY is
    the vertical contribution of the source pixel.

4)  In the general case, eight source pixels contribute to each
    destination pixel:

    s   s   s

    s   d   s

    s   s   s

    where s indicates a source pixel and d indicates a destination pixel.

    Special cases, with lesser numbers of source pixels contributing
    to a destination pixel, occur at the edges of the destination image.

5)  The weight of each component of a source pixel in the resulting
    component of the destination pixel is:

         contribX*contribY
        --------------------
         srcWidth*srcHeight

    The denominator (srcWidth*srcHeight) may be pre-calculated and
    cached, reducing the number of multiplication operations required
    to process each pixel.

6)  This algorithm may be executed in place (srcImage==destImge) for
    reduction operations, since a source pixel is over-written by a
    destination pixel only when the pixel is no longer needed to compute
    later destination pixels.

*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* Amiga includes */
#include <exec/types.h>

/* Application includes */
#include "global.h"
#include "image.h"
#include "scaling.h"

/* ANSI includes */
#include <math.h>

#ifndef DISPLAY_QUICK_SCALE

/****************************************************************************
 *                                                                          *
 *  scale() - scale 24-bit RGB image                                        *
 *                                                                          *
 ****************************************************************************/

void scale(struct appContext *appContext,
           ULONG *srcBuffer, UWORD srcWidth, UWORD srcHeight,
           ULONG *destBuffer, UWORD destWidth, UWORD destHeight,
           UWORD destModulo)
{

    ULONG   destX, destY;
    ULONG   srcX, srcY;

    UWORD   needX, needY;
    UWORD   availX, availY;

    UWORD   contribX, contribY;
    ULONG   contribWeight;

    ULONG   *pSrcStart;
    ULONG   startAvailX;

    ULONG   *pThisSrc;

    ULONG   *pDestStart;
    ULONG   *pThisDest;

    register ULONG   srcRGB;
    UBYTE   srcR, srcG, srcB;
    ULONG   rSum, gSum, bSum;
    UBYTE   destR, destG, destB;

    ULONG   srcArea;

    /* Compute source area */
    srcArea=srcWidth*srcHeight;

    /* Begin with first destination row */
    pDestStart=destBuffer;

    /* Loop through destination rows */
    for (destY=0; destY<destHeight; destY++) {

        /* Begin with first destination pixel */
        pThisDest=pDestStart;

        /* Loop through destination columns */
        for (destX=0; destX<destWidth; destX++) {

            /* Compute position and availability of source pixel (+0, +0) */
            srcX=(destX*srcWidth)/destWidth;
            srcY=(destY*srcHeight)/destHeight;
            startAvailX=availX=destWidth-((destX*srcWidth)%destWidth);
            availY=destHeight-((destY*srcHeight)%destHeight);
            pSrcStart=srcBuffer+(srcY*srcWidth)+srcX;

            /* Begin with first source pixel associated
               with this destination pixel */
            pThisSrc=pSrcStart;

            /* Clear component sums */
            rSum=gSum=bSum=0;

            /* Need srcHeight vertical units for this pixel. */
            needY=srcHeight;

            /* While vertical need is not satisfied ... */
            while (needY) {

                /* Vertical contribution of this row is
                   lesser of need and availability */
                contribY=min(needY,availY);

                /* Need srcWidth horizontal units for this pixel. */
                needX=srcWidth;

                /* While horizontal need is not satisfied ... */
                while (needX) {

                    /* Horizontal contribution of this column is
                       lesser of need and availability */
                    contribX=min(needX,availX);

                    /* Add weighted components of source pixel
                       to component sums */
                    srcRGB=*pThisSrc;
                    srcR=RfromRGB(srcRGB);
                    srcG=GfromRGB(srcRGB);
                    srcB=BfromRGB(srcRGB);
                    contribWeight=contribX*contribY;
                    rSum+=srcR*contribWeight;
                    gSum+=srcG*contribWeight;
                    bSum+=srcB*contribWeight;

                    /* Subtract horizontal contribution of this
                       source column from need */
                    needX-=contribX;

                    /* Update available horizontal portion of this
                       source column */
                    availX-=contribX;

                    /* If column is completely used ... */
                    if (availX==0) {

                        /* Advance to next source column */
                        pThisSrc++;

                        /* Column is fully available */
                        availX=destWidth;

                    }

                }

                /* Subtract vertical contribution of this
                   source row from need */
                needY-=contribY;

                /* Update available vertical portion of this
                   source row */
                availY-=contribY;

                /* If row is completely used ... */
                if (availY==0) {

                    /* Advance to next source row */
                    pThisSrc=pSrcStart+srcWidth;

                    /* Row is fully available */
                    availY=destHeight;

                    /* Use horizontal availability of first source column
                       associated with this destination pixel */
                    availX=startAvailX;

                }

            }

            /* Destination component values are
               component sums divided by source area */
            destR=rSum/srcArea;
            destG=gSum/srcArea;
            destB=bSum/srcArea;

            /* Write destination pixel */
            *(pThisDest)=RGB(destR,destG,destB);

            /* Advance to next destination column */
            pThisDest++;

        }

        /* Advance to next destination row */
        pDestStart+=destModulo;

        /* Update busy bar */
        interfaceBusyUpdate(appContext, destHeight*4, destHeight+destY);

    }

}

#endif /* DISPLAY_QUICK_SCALE */
