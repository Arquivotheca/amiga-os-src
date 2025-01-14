/*** display.c ***************************************************************
 *
 *  $Id: display.c,v 1.7 94/03/31 16:08:11 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Display Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   display.c,v $
 * Revision 1.7  94/03/31  16:08:11  jjszucs
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
 * Revision 1.6  94/03/16  18:19:38  jjszucs
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
 * Revision 1.5  94/03/08  13:52:20  jjszucs
 * Interlace bit is now set to force all displays to be interlaced,
 * eliminating black lines apparent in image displays on some
 * monitors and the slight jump as the display toggles from interlaced
 * to non-interlaced when the interface panel is hidden/revealed.
 *
 *
 * Revision 1.4  94/03/01  16:09:35  jjszucs
 * Generalized scaling and aspect ratio adjustment code replaced with
 * special cases for NTSC/PAL and portrait/landscape.
 *
 * Revision 1.3  94/02/18  15:55:17  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:02:14  jjszucs
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
 * Revision 1.1  94/01/06  11:56:46  jjszucs
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

#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/modeid.h>
#include <graphics/display.h>

#include <libraries/specialfx.h>

#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/specialfx_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/specialfx_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* Local includes */
#include "global.h"
#include "display.h"
#include "interface.h"
#include "image.h"

/****************************************************************************
 *                                                                          *
 *  openDisplay() - open display environment                                *
 *                                                                          *
 ****************************************************************************/

BOOL openDisplay(struct appContext *appContext)
{

    BOOL status;            /* Return status */

    /* Go to startup display state */
    status=newDisplayMode(appContext,
        DISPLAY_STARTUP_MODE, DISPLAY_STARTUP_DEPTH, DISPLAY_STARTUP_OVERSCAN,
            DISPLAY_STARTUP_FX);
    if (status) {
        appContext->ac_DisplayState=DISPLAY_STARTUP_STATE;
        appContext->ac_DisplayPortrait=FALSE;
    }

    /* Set interlace bit and remake display */
    GfxBase->system_bplcon0|=INTERLACE;
    RemakeDisplay();

    /* Return status */
    return status;

}

/****************************************************************************
 *                                                                          *
 *  closeDisplay() - close display environment                              *
 *                                                                          *
 ****************************************************************************/

void closeDisplay(struct appContext *appContext)
{

    /* Close display window */
    if (appContext->ac_DisplayWindow) {

        CloseWindowSafely(appContext,appContext->ac_DisplayWindow);
        appContext->ac_DisplayWindow=NULL;

    }

    /* Remove display background */
    if (appContext->ac_DisplayBkgd) {

        removeBackground(appContext, appContext->ac_DisplayBkgd);
        appContext->ac_DisplayBkgd=NULL;

    }

    /* Close display screen with protection from shimmer task */
    if (appContext->ac_DisplayScreen) {

        ObtainSemaphore(&appContext->ac_DisplaySemaphore);
        CloseScreen(appContext->ac_DisplayScreen);
        appContext->ac_DisplayScreen=NULL;
        ReleaseSemaphore(&appContext->ac_DisplaySemaphore);

    }

    /* Clear interlace bit and remake display */
    GfxBase->system_bplcon0&=~INTERLACE;
    RemakeDisplay();

}

/****************************************************************************
 *                                                                          *
 *  newDisplay()    -   transition to new display panel                     *
 *                                                                          *
 ****************************************************************************/

BOOL newDisplay(struct appContext *appContext,
    enum appState newState, BOOL newPortrait)
{

    struct imageNode *thisImageNode;
    UWORD nRows, nColumns;
    UWORD row, column;

    /* Change display */
    changeDisplay(appContext, newState, newPortrait);

    /*
     *  Display
     */

    /* Switch on state */
    switch (newState) {

        /* No Disc */
        case as_NoDisc:
        /* Invalid Disc */
        case as_InvalidDisc:

            /* No action */

            break;

        /* Thumbnail */
        case as_Thumbnail:

            /* Get size of thumbnail grid */
            thumbnailGridSize(appContext,&nRows,&nColumns);

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

            break;

        /* Image */
        case as_Image:

            displayImage(appContext,appContext->ac_Selection);
            break;

        /* !!! Other states !!! */

    };

    /*
     *  Exit
     */

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  displayStateProperties()    -   determine display properties for an     *
 *                                  application state                       *
 *                                                                          *
 ****************************************************************************/

BOOL displayStateProperties(struct appContext *appContext,
    enum appState state, BOOL portrait,
    ULONG *pModeID, UWORD *pDepth, UWORD *pOverscan, BOOL *pFX)
{

    ULONG modeID;
    UWORD depth;
    UWORD overscan;
    BOOL fx;

    /* Switch on state */
    switch (state) {

        /* No Disc */
        case as_NoDisc:
        /* Invalid Disc */
        case as_InvalidDisc:
        /* Thumbnail */
        case as_Thumbnail:

            /* Use thumbnail display properties */
            modeID=DISPLAY_THUMBNAIL_MODEID;
            depth=DISPLAY_THUMBNAIL_DEPTH;
            overscan=DISPLAY_THUMBNAIL_OVERSCAN;
            fx=TRUE;
            break;

        /* Image */
        case as_Image:

            /* Use image display properties */
            modeID=portrait?
                DISPLAY_IMAGE_PORTRAIT_MODEID:
                DISPLAY_IMAGE_LANDSCAPE_MODEID;
            depth=DISPLAY_IMAGE_DEPTH;
            overscan=DISPLAY_IMAGE_OVERSCAN;
            fx=FALSE;
            break;

        /* Unrecognized */
        default:

            return FALSE;

    }

    /* Return mode ID and depth */
    *pModeID=modeID;
    *pDepth=depth;
    *pOverscan=overscan;
    *pFX=fx;
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  newDisplayMode()    -   switch to display mode                          *
 *                                                                          *
 ****************************************************************************/

BOOL newDisplayMode(struct appContext *appContext, ULONG modeID,
    UWORD depth, UWORD overscan, BOOL bkgdFX)
{

    /* Default pen array */
    static UWORD pens[] ={
        Largest(UWORD), /* effectively ~0 */
    };

    /* Initial palette load */
    static ULONG paletteLoad[] ={
        RGBLoad(0,1),                           /* Load 1 entry beginning
                                                    at pen 0 */
                                                /* R, G, B values */
        Gun32(  0), Gun32(  0), Gun32(  0),     /* Background (pen 0) */
        0,                                      /* Terminator */
    };

    /* Close old display window */
    if (appContext->ac_DisplayWindow) {

        CloseWindowSafely(appContext,appContext->ac_DisplayWindow);
        appContext->ac_DisplayWindow=NULL;

    }

    /* Remove old background */
    if (appContext->ac_DisplayBkgd) {

        removeBackground(appContext, appContext->ac_DisplayBkgd);
        appContext->ac_DisplayBkgd=NULL;

    }

    /* Close old display screen with protection from shimmer task */
    if (appContext->ac_DisplayScreen) {

        ObtainSemaphore(&appContext->ac_DisplaySemaphore);
        CloseScreen(appContext->ac_DisplayScreen);
        appContext->ac_DisplayScreen=NULL;
        ReleaseSemaphore(&appContext->ac_DisplaySemaphore);

    }

    /* Open new display screen */
    ObtainSemaphore(&appContext->ac_DisplaySemaphore);
    appContext->ac_DisplayScreen=OpenScreenTags(NULL,
        SA_Title, NULL,
        SA_Pens, &pens,
        SA_Overscan, overscan,
        SA_Interleaved, TRUE,
        SA_Depth, depth,
        SA_DisplayID, modeID,
        SA_ColorMapEntries, DISPLAY_CMAP_ENTRIES,
        SA_Colors32, paletteLoad,
        SA_ShowTitle, FALSE,
        SA_MinimizeISG, TRUE,
        SA_Quiet, TRUE,
        TAG_DONE);
    ReleaseSemaphore(&appContext->ac_DisplaySemaphore);
    if (!appContext->ac_DisplayScreen) {

#ifdef DEBUG
        printf("newDisplayMode(): Error opening display screen\n");
#endif /* DEBUG */

        return FALSE;

    }

    /* If interface screen is open and visible ... */
    if (appContext->ac_InterfaceScreen && appContext->ac_InterfaceVisible) {

        /* Restore to frontmost position */
        ScreenToFront(appContext->ac_InterfaceScreen);

    }

    /* Open new display window */
    if ((appContext->ac_DisplayWindow=OpenWindowTags(NULL,
        WA_CustomScreen, appContext->ac_DisplayScreen,
        WA_Title, NULL,
        WA_NoCareRefresh, TRUE,
        WA_Borderless, TRUE,
        WA_Backdrop, TRUE,
        WA_RMBTrap, TRUE,
        WA_Activate, TRUE,
        WA_IDCMP, NULL,
        TAG_DONE))) {

        /* Share IDCMP port with display window */
        if (appContext->ac_InterfaceWindow) {

            appContext->ac_DisplayWindow->UserPort=
                appContext->ac_InterfaceWindow->UserPort;
            ModifyIDCMP(appContext->ac_DisplayWindow,
                IDCMP_RAWKEY|IDCMP_DISKINSERTED|IDCMP_DISKREMOVED|IDCMP_ACTIVEWINDOW);

        }

        /* Blank pointer for this window */
        BlankWindowPointer(appContext,appContext->ac_DisplayWindow);

    } else {

#ifdef DEBUG
        printf("newDisplayMode(): Error opening display window\n");
#endif /* DEBUG */

        return FALSE;

    }

    /* Install new background */
    if (bkgdFX) {

        appContext->ac_DisplayBkgd=
            installBackground(appContext, appContext->ac_DisplayScreen,
                (appContext->ac_DisplayScreen->Height-
                 INTERFACE_PANE_OFFSETY-
                 DISPLAY_INTERSCREEN_GAP)/2
            );

    }

    /* Set new display mode ID */
    appContext->ac_DisplayModeID=modeID;

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  changeDisplay()    -   change display mode without updating content     *
 *                                                                          *
 ****************************************************************************/

BOOL changeDisplay(struct appContext *appContext, enum appState newState, BOOL newPortrait)
{

    ULONG   oldModeID, newModeID;
    UWORD   oldDepth, newDepth;
    UWORD   oldOverscan, newOverscan;
    BOOL    oldFX, newFX;

    int pen;

    struct RastPort *rastport;
    struct ViewPort *viewport;

    /* Palette loading table for graphics.library/LoadRGB32() */
    static ULONG paletteLoad[] ={
        RGBLoad(0,16),                          /* Load 16 entries beginning
                                                    at pen 0 */
                                                /* R, G, B values */
        Gun32(  0), Gun32(  0), Gun32(  0),     /* Background (pen 0) */
        Gun32(238), Gun32(238), Gun32(255),     /* Scratch */
        Gun32(221), Gun32(204), Gun32( 34),     /* Art */
        Gun32(153), Gun32(136), Gun32(  0),     /* Art */
        Gun32(187), Gun32(170), Gun32(  0),     /* Art */
        Gun32(  0), Gun32(255), Gun32(  0),     /* LED green (pen 5) */
        Gun32(  0), Gun32( 51), Gun32(102),     /* Button background (pen 6) */
        Gun32(235), Gun32( 12), Gun32( 12),     /* Warning (pen 7) */
        Gun32(  0), Gun32(  0), Gun32( 17),     /* Grayscale */
        Gun32( 34), Gun32( 34), Gun32( 51),     /* Grayscale - LED segment (pen 9) */
        Gun32( 68), Gun32( 68), Gun32( 85),     /* Grayscale - shadow (pen 10) */
        Gun32(102), Gun32(104), Gun32(125),     /* Grayscale */
        Gun32(130), Gun32(142), Gun32(145),     /* Grayscale */
        Gun32(170), Gun32(170), Gun32(187),     /* Grayscale */
        Gun32(204), Gun32(204), Gun32(221),     /* Grayscale */
        Gun32(238), Gun32(238), Gun32(255),     /* Grayscale - shine (pen 15) */
        0                                       /* Terminator */
    };

    /*
     *  Re-open display (if necessary)
     */

    /* Get properties of old display state */
    oldModeID=Largest(ULONG);
    oldDepth=Largest(UWORD);
    if (appContext->ac_DisplayScreen && appContext->ac_DisplayWindow) {

        displayStateProperties(appContext, appContext->ac_DisplayState,
            appContext->ac_DisplayPortrait,
            &oldModeID, &oldDepth, &oldOverscan, &oldFX);

    }

    /* Get properties of new display state */
    if (!displayStateProperties(appContext, newState, newPortrait,
        &newModeID, &newDepth, &newOverscan, &newFX)) {

#ifdef DEBUG
        printf("newDisplay(): invalid new state %d\n",newState);
#endif /* DEBUG */
        return FALSE;

    }

    /* If mode ID, depth, overscan, or FX are changing ... */
    if (newModeID!=oldModeID || newDepth!=oldDepth ||
        newOverscan!=oldOverscan || newFX!=oldFX) {

        /* Change to new display mode */
        if (!newDisplayMode(appContext,
            newModeID, newDepth, newOverscan, newFX)) {

#ifdef DEBUG
            printf("newDisplay(): Error changing display\n");
#endif /* DEBUG */
            return FALSE;

        }

    }

    /* Update display state */
    appContext->ac_DisplayState=newState;
    appContext->ac_DisplayPortrait=newPortrait;

    /* Fetch display screen viewport and display window rastport */
    viewport=&appContext->ac_DisplayScreen->ViewPort;
    rastport=appContext->ac_DisplayWindow->RPort;

    /*
     *  Palette loading
     */

    /* Initialize palette */
    switch (newState) {

        /* No Disc */
        case as_NoDisc:
        /* Invalid Disc */
        case as_InvalidDisc:
        /* Thumbnail */
        case as_Thumbnail:

            /* Interface palette for pens 0...15 */
            LoadRGB32(viewport,paletteLoad);

            /* Gray-scale palette for pens 16...255 */
            for (pen=0;pen<240;pen++) {
                SetRGB32(viewport,16+pen,
                    Gun32((pen*240)/256), /* R */
                    Gun32((pen*240)/256), /* G */
                    Gun32((pen*240)/256) /* B */
                );
            }
            break;

        /* Image */
        case as_Image:

            /* No action; palette is loaded during image display */
            break;

        /* !!! Other states !!! */

    }

    /*
     *  Display clean-up
     */

    /* Erase display area */
    EraseRect(rastport,0,0,
        appContext->ac_DisplayWindow->Width-1,
        appContext->ac_DisplayWindow->Height-1);

}

/****************************************************************************
 *                                                                          *
 *  installBackground()    -   install background effects                   *
 *                                                                          *
 ****************************************************************************/

struct backgroundData *installBackground(struct appContext *appContext,
    struct Screen *screen, UWORD gradientLines)
{

    struct backgroundData *bgData;
    ULONG fxError;

    struct ColorControl **colorControl;
    UWORD line;
    ULONG r, g, b;
    ULONG rStep, gStep, bStep;

    /* Allocate background data */
    bgData=AllocVec(sizeof(*bgData), MEMF_CLEAR);
    if (bgData) {

        /* Associate with screen */
        bgData->bgd_Screen=screen;

        /* Allocate FX list */
        bgData->bgd_FXHandle=AllocFX(
            &screen->ViewPort,
            SFX_ColorControl,
            gradientLines+1,
            &fxError);
        if (bgData->bgd_FXHandle) {

            /* Set-up background gradient */
            /* N.B.: The last line of the color gradient must not fall
                     within an inter-screen gap. An apparent bug in
                     specialfx.library may cause the copper list for the
                     next screen to be trashed if this occurs. */
            r=GRADIENT_START_R;
            g=GRADIENT_START_G;
            b=GRADIENT_START_B;

            rStep=(GRADIENT_START_R-GRADIENT_END_R)/gradientLines;
            gStep=(GRADIENT_START_G-GRADIENT_END_G)/gradientLines;
            bStep=(GRADIENT_START_B-GRADIENT_END_B)/gradientLines;

            colorControl=(struct ColorControl **) bgData->bgd_FXHandle;

            for (line=0; line<gradientLines; line++) {

                (*colorControl)->cc_Pen=0;
                (*colorControl)->cc_Line=line*2;
                (*colorControl)->cc_Red=r;
                (*colorControl)->cc_Green=g;
                (*colorControl)->cc_Blue=b;
                (*colorControl)->cc_Flags=NULL;

                colorControl++;

                r-=rStep;
                g-=gStep;
                b-=bStep;

            }

            (*colorControl)->cc_Pen=0;
            (*colorControl)->cc_Line=(gradientLines*2)+1;
            (*colorControl)->cc_Red=0;
            (*colorControl)->cc_Green=0;
            (*colorControl)->cc_Blue=0;
            (*colorControl)->cc_Flags=NULL;

            /* Install FX */
            bgData->bgd_DisplayHandle=InstallFX(
                GfxBase->ActiView,
                &(screen->ViewPort),
                SFX_InstallEffect, bgData->bgd_FXHandle,
                TAG_DONE
            );
            if (bgData->bgd_DisplayHandle) {

                /* Re-make display */
                MakeScreen(screen);
                RethinkDisplay();

            } else {

                FreeFX(bgData->bgd_FXHandle);
                FreeVec(bgData);
                return NULL;

            }

        } else {

            FreeVec(bgData);
            return NULL;

        }

    } else {

        return NULL;

    }

    /* Return background data */
    return bgData;

}

/****************************************************************************
 *                                                                          *
 *  removeBackground()    -   remove background effects                     *
 *                                                                          *
 ****************************************************************************/

void removeBackground(struct appContext *appContext,
    struct backgroundData *bgData)
{

    if (bgData) {

        if (bgData->bgd_FXHandle) {

            if (bgData->bgd_DisplayHandle) {

                RemoveFX(bgData->bgd_DisplayHandle);
                MakeScreen(bgData->bgd_Screen);
                RethinkDisplay();

            }

            FreeFX(bgData->bgd_FXHandle);

        }

        FreeVec(bgData);

    }

}
