/*** ham8.c *****************************************************************
 *
 *  $Id: ham8.c,v 1.6 94/03/31 16:08:32 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  HAM8 Display Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   ham8.c,v $
 * Revision 1.6  94/03/31  16:08:32  jjszucs
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
 * Revision 1.5  94/03/16  18:20:03  jjszucs
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
 * Revision 1.4  94/03/08  13:54:18  jjszucs
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
 * Revision 1.3  94/03/01  16:10:31  jjszucs
 * Uses of abs(x-y) replaced with difference() macro.
 * RGB component values now extracted with an index-based macro.
 * RGB component values changed from WORD to UWORD to eliminate
 * sign extensions.
 * Various other optimizations in HAM8 color selection.
 *
 * Revision 1.1  94/01/13  17:04:03  jjszucs
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
#include <math.h>

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
#include "display.h"
#include "image.h"
#include "quantization.h"

/****************************************************************************
 *                                                                          *
 *  Local Macros                                                            *
 *                                                                          *
 ****************************************************************************/

/* HAM8 modification bits */
#define HAM8_MODIFY_R   0x80
#define HAM8_MODIFY_G   0xC0
#define HAM8_MODIFY_B   0x40

/* HAM8 modified component mask and shift values */
#define HAM8_COMPONENT_MASK     0xFC
#define HAM8_COMPONENT_SHIFT    2

#ifdef DISPLAY_FIXED_PALETTE

/****************************************************************************
 *                                                                          *
 *  initHAM8()  - initialize HAM8 color quantization tables                 *
 *                                                                          *
 ****************************************************************************/

void initHAM8(struct appContext *appContext)
{

    UWORD i, j;

    /* Initialize gun division table */
    for (i=0; i<256; i++) {

        j=max(
            min(i+FIXED_PALETTE_GUN_ROUND, FIXED_PALETTE_GUN_MAX),
            FIXED_PALETTE_GUN_MIN
          )-FIXED_PALETTE_GUN_MIN;
        j/=FIXED_PALETTE_GUN_STEP;
        appContext->ac_GunDivTable[i]=j;

    }

    /* Initialize gun multiplication table */
    for (i=0; i<FIXED_PALETTE_GUN_RANGE; i++) {

        j=FIXED_PALETTE_GUN_MIN+(i*FIXED_PALETTE_GUN_STEP);
        appContext->ac_GunMulTable[i]=j;

    }

}

/****************************************************************************
 *                                                                          *
 *  quantizeFixedPal()  - quantize RGB to fixed palette pen                 *
 *                                                                          *
 ****************************************************************************/

UBYTE quantizeFixedPal(struct appContext *appContext, ULONG rgb)
{

    register UWORD r, g, b;
    UWORD pen;

/*
    r=RinRGB(rgb)+FIXED_PALETTE_GUN_ROUND;
    g=GinRGB(rgb)+FIXED_PALETTE_GUN_ROUND;
    b=BinRGB(rgb)+FIXED_PALETTE_GUN_ROUND;

    pen=
        ((r/FIXED_PALETTE_GUN_STEP)<<FIXED_PALETTE_R_SHIFT)|
        ((g/FIXED_PALETTE_GUN_STEP)<<FIXED_PALETTE_G_SHIFT)|
        ((b/FIXED_PALETTE_GUN_STEP)<<FIXED_PALETTE_B_SHIFT);

*/

    r=RinRGB(rgb);
    g=GinRGB(rgb);
    b=BinRGB(rgb);

    r=appContext->ac_GunDivTable[r];
    g=appContext->ac_GunDivTable[g];
    b=appContext->ac_GunDivTable[b];
    pen=
        (r<<FIXED_PALETTE_R_SHIFT)|
        (g<<FIXED_PALETTE_G_SHIFT)|
        (b<<FIXED_PALETTE_B_SHIFT);

    return ((UBYTE) pen);

}

/****************************************************************************
 *                                                                          *
 *  fixedPalRGB()   -   get RGB value for fixed palette pen                 *
 *                                                                          *
 ****************************************************************************/

ULONG fixedPalRGB(struct appContext *appContext, UBYTE pen)
{

    register UBYTE r, g, b;
    ULONG rgb;

    r=(pen>>FIXED_PALETTE_R_SHIFT)&FIXED_PALETTE_GUN_MASK;
    g=(pen>>FIXED_PALETTE_G_SHIFT)&FIXED_PALETTE_GUN_MASK;
    b=(pen>>FIXED_PALETTE_B_SHIFT)&FIXED_PALETTE_GUN_MASK;

    rgb=0L;
/*
    RinRGB(rgb)=FIXED_PALETTE_GUN_MIN+(r*FIXED_PALETTE_GUN_STEP);
    GinRGB(rgb)=FIXED_PALETTE_GUN_MIN+(g*FIXED_PALETTE_GUN_STEP);
    BinRGB(rgb)=FIXED_PALETTE_GUN_MIN+(b*FIXED_PALETTE_GUN_STEP);
*/
    RinRGB(rgb)=appContext->ac_GunMulTable[r];
    GinRGB(rgb)=appContext->ac_GunMulTable[g];
    BinRGB(rgb)=appContext->ac_GunMulTable[b];

    return rgb;

}

/****************************************************************************
 *                                                                          *
 *  ham8QuantizeFixedPal()  - quantize RGB to fixed palette pen             *
 *                            (for internal use by HAM8 color selection)    *
 *                                                                          *
 ****************************************************************************/

static UBYTE ham8QuantizeFixedPal(struct appContext *appContext,
    ULONG rgb, ULONG *pResultRGB)
{

    UWORD r, g, b;
    UWORD pen;
    ULONG resultRGB;

    r=RinRGB(rgb);
    g=GinRGB(rgb);
    b=BinRGB(rgb);

/*
    r=max(
        min(r+FIXED_PALETTE_GUN_ROUND, FIXED_PALETTE_GUN_MAX),
        FIXED_PALETTE_GUN_MIN
      )-FIXED_PALETTE_GUN_MIN;
    g=max(
        min(g+FIXED_PALETTE_GUN_ROUND, FIXED_PALETTE_GUN_MAX),
        FIXED_PALETTE_GUN_MIN
      )-FIXED_PALETTE_GUN_MIN;
    b=max(
        min(b+FIXED_PALETTE_GUN_ROUND, FIXED_PALETTE_GUN_MAX),
        FIXED_PALETTE_GUN_MIN
      )-FIXED_PALETTE_GUN_MIN;

    r/=FIXED_PALETTE_GUN_STEP;
    g/=FIXED_PALETTE_GUN_STEP;
    b/=FIXED_PALETTE_GUN_STEP;

*/

    r=appContext->ac_GunDivTable[r];
    g=appContext->ac_GunDivTable[g];
    b=appContext->ac_GunDivTable[b];

    pen=
        (r<<FIXED_PALETTE_R_SHIFT)|
        (g<<FIXED_PALETTE_G_SHIFT)|
        (b<<FIXED_PALETTE_B_SHIFT);

    r=appContext->ac_GunMulTable[r];
    g=appContext->ac_GunMulTable[g];
    b=appContext->ac_GunMulTable[b];
/*
    r*=FIXED_PALETTE_GUN_STEP;
    g*=FIXED_PALETTE_GUN_STEP;
    b*=FIXED_PALETTE_GUN_STEP;
*/

    resultRGB=0L;
/*
    RinRGB(resultRGB)=FIXED_PALETTE_GUN_MIN+r;
    GinRGB(resultRGB)=FIXED_PALETTE_GUN_MIN+g;
    BinRGB(resultRGB)=FIXED_PALETTE_GUN_MIN+b;
*/
    RinRGB(resultRGB)=r;
    GinRGB(resultRGB)=g;
    BinRGB(resultRGB)=b;
    *pResultRGB=resultRGB;

    return ((UBYTE) pen);

}

#endif /* DISPLAY_FIXED_PALETTE */

/****************************************************************************
 *                                                                          *
 *  ham8Pick - select color value for HAM8 display mode                     *
 *                                                                          *
 ****************************************************************************/

UBYTE ham8Pick(struct appContext *appContext, ULONG thisRGB, ULONG *pLastRGB)
{

    UWORD   thisR, thisG, thisB;

    UBYTE   clutValue;
    ULONG   clutRGB;
    UWORD   clutR, clutG, clutB;
    UWORD   clutErrorR, clutErrorG, clutErrorB;
    UWORD   clutErrorSum;

    ULONG   lastRGB;
    UWORD   lastR, lastG, lastB;
    UWORD   lastErrorR, lastErrorG, lastErrorB;

    UBYTE   hamValue;
    ULONG   hamRGB;
    UWORD   hamR, hamG, hamB;
    UWORD   hamErrorR, hamErrorG, hamErrorB;
    UWORD   hamErrorSum;

    /* Extract components of target RGB value */
    thisR=RinRGB(thisRGB);
    thisG=GinRGB(thisRGB);
    thisB=BinRGB(thisRGB);

    /* Quantize 24-bit RGB to CLUT value */
#ifdef DISPLAY_FIXED_PALETTE
    clutValue=ham8QuantizeFixedPal(
        appContext, thisRGB, &clutRGB);
#else
    clutValue=quantizeRGB(
        appContext,
        appContext->ac_QuantTree,
        thisRGB);
#endif /* DISPLAY_FIXED_PALETTE */

    /* Determine CLUT error factors */
#ifndef DISPLAY_FIXED_PALETTE
    clutRGB=appContext->ac_ColorTable[clutValue];
#endif /* DISPLAY_FIXED_PALETTE */
    clutR=RinRGB(clutRGB);
    clutG=GinRGB(clutRGB);
    clutB=BinRGB(clutRGB);

    clutErrorR=difference(clutR,thisR);
    clutErrorG=difference(clutG,thisG);
    clutErrorB=difference(clutB,thisB);
    clutErrorSum=clutErrorR+clutErrorG+clutErrorB;

    /* Determine change from last RGB value */
    lastRGB=*pLastRGB;
    lastR=RinRGB(lastRGB);
    lastG=GinRGB(lastRGB);
    lastB=BinRGB(lastRGB);

    lastErrorR=difference(lastR,thisR);
    lastErrorG=difference(lastG,thisG);
    lastErrorB=difference(lastB,thisB);

    /* Modify component with greatest error */
    if (lastErrorR>lastErrorG && lastErrorR>lastErrorB) {

        /* Modify red component */
        hamValue=
            HAM8_MODIFY_R |
            (thisR>>HAM8_COMPONENT_SHIFT);
        hamRGB=0L;
        RinRGB(hamRGB)=hamR=thisR&HAM8_COMPONENT_MASK;
        GinRGB(hamRGB)=hamG=lastG;
        BinRGB(hamRGB)=hamB=lastB;

    } else if (lastErrorB>lastErrorR && lastErrorB>lastErrorG) {

        /* Modify blue component */
        hamValue=
            HAM8_MODIFY_B | /* Modify blue */
            (thisB>>HAM8_COMPONENT_SHIFT);
        hamRGB=0L;
        RinRGB(hamRGB)=hamR=lastR;
        GinRGB(hamRGB)=hamG=lastG;
        BinRGB(hamRGB)=hamB=thisB&HAM8_COMPONENT_MASK;

    /* ... else ... */
    } else {

        /* Modify green component (most perceptible) */
        hamValue=
            HAM8_MODIFY_G | /* Modify green */
            (thisG>>HAM8_COMPONENT_SHIFT);
        hamRGB=0L;
        RinRGB(hamRGB)=hamR=lastR;
        GinRGB(hamRGB)=hamG=thisG&HAM8_COMPONENT_MASK;
        BinRGB(hamRGB)=hamB=lastB;

    }

    /* Determine HAM error factors */
    hamErrorR=difference(hamR,thisR);
    hamErrorG=difference(hamG,thisG);
    hamErrorB=difference(hamB,thisB);
    hamErrorSum=hamErrorR+hamErrorG+hamErrorB;


    /* If HAM error sum is less than CLUT error sum ... */
    if (hamErrorSum<clutErrorSum) {

        /* Use HAM for this pixel */
        *pLastRGB=hamRGB;
        return hamValue;

    /* ... else ... */
    }

    /* Use CLUT for this pixel */
    *pLastRGB=clutRGB;
    return clutValue;

}
