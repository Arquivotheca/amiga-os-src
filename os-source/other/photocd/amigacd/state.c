/*** state.c *****************************************************************
 *
 *  $Id: state.c,v 1.5 94/03/31 16:09:28 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  State Transition Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	state.c,v $
 * Revision 1.5  94/03/31  16:09:28  jjszucs
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
 * Revision 1.4  94/03/01  16:16:17  jjszucs
 * Changes to support special-casing of display modes.
 *
 * Revision 1.3  94/02/18  15:58:11  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:06:37  jjszucs
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
 * Revision 1.1  94/01/06  11:58:18  jjszucs
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

#include <clib/exec_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>

/* Local includes */
#include "global.h"
#include "interface.h"
#include "image.h"

/****************************************************************************
 *                                                                          *
 *  newState()   -   transition to new state                                *
 *                                                                          *
 ****************************************************************************/

BOOL newState(struct appContext *appContext,enum appState newState)
{

    enum appState oldState;

    /* Save old state */
    oldState=appContext->ac_State;

    /* Set state in application context */
    appContext->ac_State=newState;

    /* Immediately clear control panel to prevent dead control panel
       from remaining on-screen */
    clearControls(appContext);

    /* If going to non-Image state ... */
    if (appContext->ac_State!=as_Image) {

        /* Force interface panel visible */
        showInterface(appContext);

    }

    /* Handle state transitions */
    switch (appContext->ac_State) {

        /* No Disc */
        case as_NoDisc:
        /* Invalid Disc */
        case as_InvalidDisc:

            /* End session */
            endSession(appContext);
            break;

        /* Thumbnail */
        case as_Thumbnail:
        /* Image */
        case as_Image:

            /* Begin session */
            if (!beginSession(appContext)) {

                fatalError(appContext,"Error opening Photo CD session");

            }
            break;

        /* !!! Other states !!! */

    }

    /* Update display pane */
    newDisplay(appContext, newState,
        appContext->ac_Selection?
            (appContext->ac_Selection->in_Rotation%180 ? TRUE : FALSE)
            : FALSE);

    /* Update status panel */
    newStatus(appContext,newState);

    /* Update control bar */
    newControl(appContext,newState);

    /* Start state */
    switch (appContext->ac_State) {

        /* No Disc */
        case as_NoDisc:
        /* Invalid Disc */
        case as_InvalidDisc:

            /* No cursor */
            break;

        /* Thumbnail */
        case as_Thumbnail:

            /* Select image */
            thumbnailSelect(appContext, appContext->ac_Selection);
            break;

        /* Image */
        case as_Image:

            /* Select image */
            imageSelect(appContext, appContext->ac_Selection);
            break;

        /* !!! Other states !!! */

    }

    /* Success */
    return TRUE;

}
