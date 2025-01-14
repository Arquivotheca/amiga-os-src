/*** glyphs.c ***************************************************************
 *
 *  $Id: glyphs.c,v 1.7 94/03/17 16:26:01 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Glyphs Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   glyphs.c,v $
 * Revision 1.7  94/03/17  16:26:01  jjszucs
 * Slide|Save glyph added
 *
 * Revision 1.6  94/03/16  18:19:52  jjszucs
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
 * Revision 1.5  94/03/09  17:03:40  jjszucs
 * Seperate Play and Puase combined into tri-state glyph
 * similar to that used by the CD multiplayer.
 *
 * Stop glyph corrected; had an inadvertent raised appearance.
 *
 * "Easter egg" screen improved - text now glows and is better
 * centered :-).
 *
 * Revision 1.4  94/03/08  13:54:02  jjszucs
 * Added credits glyph.
 *
 * Revision 1.3  94/03/01  16:10:14  jjszucs
 * Mask now used for control panel buttons.
 *
 * Revision 1.2  94/02/18  15:55:45  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.1  94/01/06  11:57:12  jjszucs
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

#include <graphics/gfx.h>

#include <hardware/blit.h>

#include <libraries/debox.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/debox_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/debox_pragmas.h>

/* Local includes */
#include "global.h"
#include "glyphs.h"

/****************************************************************************
 *                                                                          *
 *  Compressed glyph references                                             *
 *                                                                          *
 ****************************************************************************/

extern UBYTE __far  *boxStatusPanel,
                    *boxNoDisc,
                    *boxInvalidDisc,
                    *boxPhotoCDLogo,
                    *boxImageFrameHoriz,
                    *boxImageFrameVert,
                    *boxSlideFrameHoriz,
                    *boxSlideFrameVert,
                    *boxPause,
                    *boxStop,
                    *boxRepeatOff,
                    *boxRepeatOn,
                    *boxNormalize,
                    *boxPrevImage,
                    *boxNextImage,
                    *boxImage,
                    *boxThumbnail,
                    *boxPlayOff,
                    *boxPlayOn,
                    *boxMirror,
                    *boxRotate,
                    *boxPan,
                    *boxZoomIn,
                    *boxZoomOut,
                    *boxSwapNext,
                    *boxSwapPrevious,
                    *boxRemoveSlide,
                    *boxAddSlide,
                    *boxInterval,

                    *boxLEDSeg,
                    *boxLEDSegNarrow,
                    *boxLED1Narrow,
                    *boxLEDDigitDash,

                    *boxLED0,
                    *boxLED1,
                    *boxLED2,
                    *boxLED3,
                    *boxLED4,
                    *boxLED5,
                    *boxLED6,
                    *boxLED7,
                    *boxLED8,
                    *boxLED9,

                    *boxLEDSlash,
                    *boxLEDSlashSeg,

                    *boxSlide,
                    *boxManipulate,

                    *boxBusy,

                    *boxRaisedButton,
                    *boxDepressedButton,

                    *boxPanelBkgd,

                    *boxMaskButton,

                    *boxCredits,

                    *boxPlayPause,

                    *boxZoomCursor,

                    *boxBusyThumbnail,
                    *boxBusyImage,

                    *boxExit,
                    *boxSave;

static void *boxTable[] ={

    &boxStatusPanel,
    &boxNoDisc,
    &boxInvalidDisc,
    &boxPhotoCDLogo,
    &boxImageFrameHoriz,
    &boxImageFrameVert,
    &boxSlideFrameHoriz,
    &boxSlideFrameVert,
    &boxPause,
    &boxStop,
    &boxRepeatOff,
    &boxRepeatOn,
    &boxNormalize,
    &boxPrevImage,
    &boxNextImage,
    &boxImage,
    &boxThumbnail,
    &boxPlayOff,
    &boxPlayOn,
    &boxMirror,
    &boxRotate,
    &boxPan,
    &boxZoomIn,
    &boxZoomOut,
    &boxSwapNext,
    &boxSwapPrevious,
    &boxRemoveSlide,
    &boxAddSlide,
    &boxInterval,

    &boxLEDSeg,
    &boxLEDSegNarrow,
    &boxLED1Narrow,
    &boxLEDDigitDash,

    &boxLED0,
    &boxLED1,
    &boxLED2,
    &boxLED3,
    &boxLED4,
    &boxLED5,
    &boxLED6,
    &boxLED7,
    &boxLED8,
    &boxLED9,

    &boxLEDSlash,
    &boxLEDSlashSeg,

    &boxSlide,
    &boxManipulate,

    &boxBusy,

    &boxRaisedButton,
    &boxDepressedButton,

    &boxPanelBkgd,

    &boxMaskButton,

    &boxCredits,

    &boxPlayPause,

    &boxZoomCursor,

    &boxBusyThumbnail,
    &boxBusyImage,

    &boxExit,

    &boxSave

};

/****************************************************************************
 *                                                                          *
 *  loadGlyph()     -   load glyph                                          *
 *                                                                          *
 ****************************************************************************/

struct BitMap *loadGlyph(struct appContext *appContext,UWORD glyphID)
{

    void *sourceData;

    struct BMInfo *pBMInfo;

    struct BitMap *pBitmap;

    /* If glyph is not loaded ... */
    pBitmap=appContext->ac_Glyphs[glyphID];
    if (!pBitmap) {

        /* Obtain source data */
        sourceData=boxTable[glyphID];

        /* Load bitmap info */
        pBMInfo=
            DecompBMInfo(NULL,NULL,sourceData);
        if (pBMInfo) {

            /* Allocate bitmap */
            if (pBitmap=AllocBitMap(pBMInfo->bmi_Width,pBMInfo->bmi_Height,
                pBMInfo->bmi_Depth,BMF_CLEAR,NULL)) {

                /* Decompress bitmap */
                if (!DecompBitMap(NULL,sourceData,pBitmap,NULL)) {

                    appContext->ac_Glyphs[glyphID]=pBitmap;

                    /* Store dimensions */
                    appContext->ac_GlyphWidth[glyphID]=pBMInfo->bmi_Width;
                    appContext->ac_GlyphHeight[glyphID]=pBMInfo->bmi_Height;

                } else {

#ifdef DEBUG
                    printf("loadGlyph(): Error decompressing bitmap\n");
#endif /* DEBUG */

                    FreeBitMap(pBitmap);
                    pBitmap=NULL;

                }

            } else {

#ifdef DEBUG
                printf("loadGlyph(): Error allocating bitmap\n");
#endif /* DEBUG */

            }

            FreeBMInfo(pBMInfo);

        } else {

#ifdef DEBUG
            printf("loadGlyph(): Error getting BitMap info\n");
#endif /* DEBUG */

        }

    }

    /* Return bitmap (NULL for failure) */
    return pBitmap;

}

/****************************************************************************
 *                                                                          *
 *  unloadGlyph()     -   unload glyph                                      *
 *                                                                          *
 ****************************************************************************/

void unloadGlyph(struct appContext *appContext,UWORD glyphID)
{

    /* Release bitmap */
    if (appContext->ac_Glyphs[glyphID]) {

        FreeBitMap(appContext->ac_Glyphs[glyphID]);

        /* Clear bitmap pointer */
        appContext->ac_Glyphs[glyphID]=NULL;

    }

}

/****************************************************************************
 *                                                                          *
 *  putGlyph()     -   put glyph on display                                 *
 *                                                                          *
 ****************************************************************************/

BOOL putGlyph(struct appContext *appContext,UWORD glyphID,
    struct RastPort *rastport,UWORD x,UWORD y)
{

    struct BitMap *pBitMap;

    /* Load glyph bitmap */
    pBitMap=loadGlyph(appContext,glyphID);
    if (!pBitMap) {

#ifdef DEBUG
        printf("putGlpyh(): Error loading glyph ID %lu\n",glyphID);
#endif /* DEBUG */

        return FALSE;

    }

    /* Write bitmap to rastport */
    BltBitMapRastPort(pBitMap, 0, 0,
        rastport, x, y,
        appContext->ac_GlyphWidth[glyphID], appContext->ac_GlyphHeight[glyphID],
        MINTERM_COPY);

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  putGlyphMask()     -   put glyph on display with mask                   *
 *                                                                          *
 ****************************************************************************/

BOOL putGlyphMask(struct appContext *appContext, UWORD glyphID, UWORD maskID,
    struct RastPort *rastport,UWORD x,UWORD y)
{

    struct BitMap *pBitMap;
    struct BitMap *pMask;

    /* Load glyph bitmap */
    pBitMap=loadGlyph(appContext,glyphID);
    if (!pBitMap) {

#ifdef DEBUG
        printf("putGlyph(): Error loading glyph ID %lu\n",glyphID);
#endif /* DEBUG */

        return FALSE;

    }

    if (maskID!=GLYPH_NONE) {

        /* Load mask bitmap */
        pMask=loadGlyph(appContext, maskID);
        if (!pMask) {

#ifdef DEBUG
            printf("putGlyph(): Error loading mask ID %lu\n",maskID);
#endif /* DEBUG */

            return FALSE;

        }

        /* Write bitmap to rastport with mask */
        BltMaskBitMapRastPort(pBitMap, 0, 0,
            rastport, x, y,
            appContext->ac_GlyphWidth[glyphID], appContext->ac_GlyphHeight[glyphID],
            (ABC|ABNC|ANBC),
            pMask->Planes[0]);

    } else {

        /* Write bitmap to rastport */
        BltBitMapRastPort(pBitMap, 0, 0,
            rastport, x, y,
            appContext->ac_GlyphWidth[glyphID], appContext->ac_GlyphHeight[glyphID],
            MINTERM_COPY);

    }

    /* Success */
    return TRUE;

}
