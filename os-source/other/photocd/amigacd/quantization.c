/*** quantization.c *********************************************************
 *
 *  $Id: quantization.c,v 1.5 94/03/08 13:58:05 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Color Quantization Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
 *
 *  This color quantization code is based on:
 *
 *      Gervautz, Michael and Purgathofer, Werner.
 *      "A Simple Method for Color Quantization: Octree Quantization."
 *      _Graphics Gems_. Glassner, Andrew, ed.
 *
 *  and the public-domain implementation of the algorithm by Wolfgang
 *  Stuerzlinger.
 *
 *  This algorithm was selected due:
 *      o   High speed
 *      o   Minimal memory usage
 *
 */

/*
 *  !!!
 *
 *  This code currently contains many bogons:
 *
 *  o   Non-reentrant
 *
 *  o   Inconsistent coding style
 *
 *  o   No error checking
 *
 *  !!!
 */

/*
$Log:   quantization.c,v $
 * Revision 1.5  94/03/08  13:58:05  jjszucs
 * Color quantization function is now called directly, without an
 * intermediate data-hiding function. This eliminates one function
 * call per pixel.
 *
 * Quantization octree is now exposed in application context. This
 * supports direct calling of the color quantization code.
 * >>
 * Color table is now exposed in the application context. This allows
 * HAM8 color selection function to access color table directly,
 * without overhead of accessor function.
 *
 *
 * Revision 1.4  94/03/01  16:15:00  jjszucs
 * obtainQuantization() and subsidiary functions now accept
 * sampling interval parameters
 *
 * Revision 1.3  94/02/18  15:57:05  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:05:32  jjszucs
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
 * Revision 1.1  94/01/06  11:57:51  jjszucs
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
#include <stdlib.h>

/* Amiga includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>

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
#include "image.h"
#include "quantization.h"

#ifndef DISPLAY_FIXED_PALETTE

/****************************************************************************
 *                                                                          *
 *  Local Definitions                                                       *
 *                                                                          *
 ****************************************************************************/

static UINT size;
static UINT reducelevel;
static UINT leaflevel;
static ULONG colortable[QUANT_MAX_COLORS];
static OCTREE tree;
static OCTREE reducelist[QUANT_MAX_DEPTH + 1];

static UINT quant(OCTREE tree,struct color orig);
static void initcolortable(OCTREE tree,UINT *index);
static void newandinit(OCTREE *tree,UINT depth);
static void freeandclear(OCTREE tree);
static void getreduceable(OCTREE *node);
static void makereduceable(UINT level,OCTREE node);
static void reducetree(OCTREE tree);
static void inserttree(OCTREE *tree,struct color rgb,UINT depth);
static void generateoctree(struct appContext *appContext,OCTREE *tree,unsigned long *pImage,
    int width,int height,int modulo,int xSampling, int ySampling);

/****************************************************************************
 *                                                                          *
 *  Code                                                                    *
 *                                                                          *
 ****************************************************************************/

static UINT quant(OCTREE tree,struct color orig)
    {
    /* This kludge eliminates potential Enforcer hits when a gap in the
        quantization octree is encountered */
    if (!tree) {
        return 0;
    }
    if (tree->leaf)
        return(tree->colorindex);
    else
        return(quant(tree->next[
                        BitTest(orig.r,QUANT_MAX_DEPTH - tree->level) * 4 +
                        BitTest(orig.g,QUANT_MAX_DEPTH - tree->level) * 2 +
                        BitTest(orig.b,QUANT_MAX_DEPTH - tree->level)],orig));
    }

static void initcolortable(OCTREE tree,UINT *index)
    {
    UINT i;

    if (tree != NULL) {
        if (tree->leaf || tree->level == leaflevel) {
            colortable[*index] = RGB(tree->rgbsum.r / tree->colorcount,
                                    tree->rgbsum.g / tree->colorcount,
                                    tree->rgbsum.b / tree->colorcount);
            tree->colorindex = *index;
            tree->leaf = 1;
            *index = *index + 1;
            }
        else {
            for (i = 0; i < 8; i++)
                initcolortable(tree->next[i],index);
            }
        }
    }

static void newandinit(OCTREE *tree,UINT depth)
    {
    *tree = (struct node *) calloc(1,sizeof(struct node));
    if (*tree == NULL) {
        /* !!! Bogus !!! */
        printf("out of memory");
        exit(1);
        }
    (*tree)->level = depth;
    (*tree)->leaf = (depth >= leaflevel);
    if ((*tree)->leaf)
        size++;
    }

static void freeandclear(OCTREE tree)
{

    int i;

    /* Free children */
    for (i=0;i<8;i++) {
        if (tree->next[i]) {
            freeandclear(tree->next[i]);
        }
    }

    /* Free node */
    free(tree);

}

static void getreduceable(OCTREE *node)
    {
    UINT newreducelevel;

    newreducelevel = reducelevel;
    while (reducelist[newreducelevel] == NULL)
        newreducelevel--;
    *node = reducelist[newreducelevel];
    reducelist[newreducelevel] =
                reducelist[newreducelevel]->nextreduceable;
    }

static void makereduceable(UINT level,OCTREE node)
    {
    node->nextreduceable = reducelist[level];
    reducelist[level] = node;
    }

static void reducetree(OCTREE tree)
    {
    OCTREE node;
    UINT depth;

    getreduceable(&node);
    node->leaf = 1;
    size = size - node->children + 1;
    depth = node->level;
    if (depth < reducelevel) {
        reducelevel = depth;
        leaflevel = reducelevel + 1;
        }
    }

static void inserttree(OCTREE *tree,struct color rgb,UINT depth)
    {
    UINT branch;

    if (*tree == NULL)
        newandinit(tree,depth);
    (*tree)->colorcount++;
    (*tree)->rgbsum.r += (ULONG) rgb.r;
    (*tree)->rgbsum.g += (ULONG) rgb.g;
    (*tree)->rgbsum.b += (ULONG) rgb.b;
    if ((*tree)->leaf == FALSE && depth < leaflevel) {
        branch = BitTest(rgb.r,QUANT_MAX_DEPTH - depth) * 4 +
                    BitTest(rgb.g,QUANT_MAX_DEPTH - depth) * 2 +
                    BitTest(rgb.b,QUANT_MAX_DEPTH - depth);
        if ((*tree)->next[branch] == NULL) {
            (*tree)->children++;
            if ((*tree)->children == 2)
                makereduceable(depth,*tree);
            }
        inserttree(&((*tree)->next[branch]),rgb,depth + 1);
        }
    }

static void generateoctree(struct appContext *appContext,
    OCTREE *tree,unsigned long *pImage,
    int width, int height, int modulo, int xSampling, int ySampling)
{

    struct color rgb;
    int x, y;
    unsigned long *pPixel, *pRow;

    size = 0;
    reducelevel = QUANT_MAX_DEPTH;
    leaflevel = reducelevel + 1;
    pRow=pImage;

    for (y=0;y<height;y+=ySampling) {

        pPixel=pRow;

        for (x=0;x<width;x+=xSampling) {

                        rgb.r=(UCHAR) ((*pPixel)>>16)&0xFF;
                        rgb.g=(UCHAR) ((*pPixel)>> 8)&0xFF;
                        rgb.b=(UCHAR) ((*pPixel)>> 0)&0xFF;

                inserttree(tree,rgb,0);

                if (size > QUANT_MAX_COLORS - 1) {
                reducetree(*tree);
            }

            pPixel+=xSampling;

        }

        pRow+=(modulo*ySampling);

        /* Update busy bar */
        interfaceBusyUpdate(appContext, height*3, height+y);

    }

}

/****************************************************************************
 *                                                                          *
 *  obtainQuantization()    -   obtain quantization of image                *
 *                                                                          *
 ****************************************************************************/

BOOL obtainQuantization(struct appContext *appContext,
    struct ViewPort *viewPort,
    ULONG *imageBuffer, UWORD width, UWORD height,
    UWORD modulo, UWORD xSampling, UWORD ySampling)
{

    UINT i, j;

    /* Initialize color table */
    for (i=0;i<QUANT_MAX_COLORS;i++) {
        colortable[i]=0;
    }

    /* Generate quantization octree */
    tree=NULL;
    generateoctree(appContext,&tree,imageBuffer,width,height,modulo,xSampling,ySampling);

    /* Initialize color table, with entry 0 remaining black */
    i=1;
    initcolortable(tree,&i);

    /* Load color table into viewport */
    for (j=0;j<i;j++) {
        SetRGB32(viewPort,j,
            Gun32((colortable[j]>>16)&0xFF),
            Gun32((colortable[j]>> 8)&0xFF),
            Gun32((colortable[j]    )&0xFF)
        );
    }

    /* Store pointer to color table in application context for use
       by other modules */
    appContext->ac_ColorTable=colortable;

    /* Store pointer to quantization octree in application context for use
       by other modules */
    appContext->ac_QuantTree=tree;

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  releaseQuantization()   -   release quantization of image               *
 *                                                                          *
 ****************************************************************************/

void releaseQuantization(struct appContext *appContext)
{

    /* Release quantization octree */
    freeandclear(tree);
    tree=NULL;

    /* Clear color table pointer */
    appContext->ac_ColorTable=NULL;

    /* Clear quantization octree pointer */
    appContext->ac_QuantTree=NULL;

}

/****************************************************************************
 *                                                                          *
 *  quantizeRGB()   -   quantize RGB value                                  *
 *                                                                          *
 ****************************************************************************/

UINT quantizeRGB(struct appContext *appContext, OCTREE thisTree,
    ULONG rgb)
{

    /* This kludge eliminates potential Enforcer hits when a gap in the
        quantization octree is encountered */
    if (!thisTree) {
        return 0;
    }
    if (thisTree->leaf)
        return(thisTree->colorindex);
    else
        return(
            quantizeRGB(appContext,
            thisTree->next[
                    BitTest(RinRGB(rgb), QUANT_MAX_DEPTH - thisTree->level) * 4 +
                    BitTest(GinRGB(rgb), QUANT_MAX_DEPTH - thisTree->level) * 2 +
                    BitTest(BinRGB(rgb), QUANT_MAX_DEPTH - thisTree->level)
                    ],
            rgb)
        );

}

#endif /* DISPLAY_FIXED_PALETTE */
