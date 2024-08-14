/*** quantization.h ***************************************************************
 *
 *  $Id: quantization.h,v 1.5 94/03/08 14:08:06 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Quantization Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	quantization.h,v $
 * Revision 1.5  94/03/08  14:08:06  jjszucs
 * Changes per 40.10.
 * 
 * Revision 1.4  94/03/01  16:18:34  jjszucs
 * Changes to support sampling interval parameter
 * to quantization functions.
 *
 * Revision 1.3  94/02/18  15:59:36  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:09:10  jjszucs
 * Changes per version 40.3
 *
*/

#ifndef APP_QUANTIZATION_H

#define APP_QUANTIZATION_H

/****************************************************************************
 *                                                                          *
 *  Color Quantization Definitions                                          *
 *                                                                          *
 ****************************************************************************/

#define QUANT_MAX_DEPTH     7       /* Maximum depth */
#define QUANT_MAX_COLORS    64      /* Maximum number of colors */

typedef unsigned char UCHAR;            /* 8 bits */
typedef unsigned short int UINT;        /* 16 bits */

struct color {
    UCHAR r;
    UCHAR g;
    UCHAR b;
    };
struct colorsum {
    ULONG r;
    ULONG g;
    ULONG b;
    };
typedef struct node * OCTREE;
struct node {
    UINT leaf;
    UINT level;
    UINT colorindex;
    UINT children;
    ULONG colorcount;
    struct colorsum rgbsum;
    OCTREE nextreduceable;
    OCTREE next[8];
    };

/****************************************************************************
 *                                                                          *
 *  Prototypes                                                              *
 *                                                                          *
 ****************************************************************************/

/* from quantization.c */
BOOL obtainQuantization(struct appContext *appContext,
    struct ViewPort *viewPort, ULONG *imageBuffer,
    UWORD width, UWORD height,
    UWORD modulo, UWORD xSampling, UWORD ySampling);
void releaseQuantization(struct appContext *appContext);
UINT quantizeRGB(struct appContext *appContext, OCTREE thisTree,
    ULONG rgb);

#endif /* APP_QUANTIZE_H */
