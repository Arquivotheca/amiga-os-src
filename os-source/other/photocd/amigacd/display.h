/*** display.h ****************************************************************
 *
 *  $Id: display.h,v 1.6 94/03/31 16:09:42 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Display Environment Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	display.h,v $
 * Revision 1.6  94/03/31  16:09:42  jjszucs
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
 * Revision 1.5  94/03/16  18:21:39  jjszucs
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
 * Revision 1.4  94/03/01  16:17:05  jjszucs
 * Changes to support special-casing of display modes
 * and quick scaling.
 *
 * Revision 1.3  94/02/18  15:58:55  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:08:28  jjszucs
 * Changes per version 40.3
 *
 * Revision 1.1  94/01/06  11:59:56  jjszucs
 * Initial revision
 *
*/

#ifndef APP_DISPLAY_H

#define APP_DISPLAY_H

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

#ifndef GRAPHICS_MODEID_H
#include <graphics/modeid.h>
#endif /* GRAPHICS_MODEID_H */

/****************************************************************************
 *                                                                          *
 *  Display screen definitions                                              *
 *                                                                          *
 ****************************************************************************/

#define DISPLAY_THUMBNAIL_MODEID    HIRESLACE_KEY   /* Display mode for
                                                       thumbnails */
#define DISPLAY_THUMBNAIL_DEPTH     8                /* Display depth for
                                                       thumbnails */
#define DISPLAY_THUMBNAIL_OVERSCAN  OSCAN_STANDARD

#define DISPLAY_IMAGE_PORTRAIT_MODEID HAM_KEY         /* Display mode
                                                       for portrait images */
#define DISPLAY_IMAGE_LANDSCAPE_MODEID HIRESHAMLACE_KEY /* Display mode for
                                                        landscape images */
#define DISPLAY_IMAGE_DEPTH         8               /* Display depth for
                                                       images */
#define DISPLAY_IMAGE_OVERSCAN      OSCAN_VIDEO     /* Overscan for images */

#define DISPLAY_STARTUP_MODE        DISPLAY_THUMBNAIL_MODEID /* Startup display
                                                                mode ID */
#define DISPLAY_STARTUP_DEPTH       DISPLAY_THUMBNAIL_DEPTH  /* Startup display
                                                                depth */
#define DISPLAY_STARTUP_OVERSCAN    DISPLAY_THUMBNAIL_OVERSCAN /* Startup overscan */

#define DISPLAY_STARTUP_STATE       as_NoDisc       /* Startup display
                                                       state */
#define DISPLAY_STARTUP_FX          TRUE            /* Startup display FX */

#define DISPLAY_INTERSCREEN_GAP     6               /* Interscreen gap between
                                                       display and interface
                                                       screens */
#define DISPLAY_CMAP_ENTRIES        256             /* Number of color map
                                                       entries for display screen */

#define GRADIENT_START_R            0x60606060      /* Gradient start */
#define GRADIENT_START_G            0x6E6E6E6E
#define GRADIENT_START_B            0xC8C8C8C8

#define GRADIENT_END_R              0x12121212      /* Gradient end */
#define GRADIENT_END_G              0x08080808
#define GRADIENT_END_B              0x3C3C3C3C

/****************************************************************************
 *                                                                          *
 *  Thumbnail panel                                                         *
 *                                                                          *
 ****************************************************************************/

#define THUMBNAIL_FRAME_WIDTH   126             /* Thumbnail frame width */
#define THUMBNAIL_FRAME_HEIGHT  126             /* Thumbnail frame height */

#define THUMBNAIL_HORIZ_SPACING  12             /* Horizontal thumbnail spacing */
#define THUMBNAIL_VERT_SPACING   16             /* Vertical thumbnail spacing */

#define HORIZ_THUMBNAIL_OFFSET_X 15             /* X-axis offset of horizontal
                                                   thumbnail in frame */
#define HORIZ_THUMBNAIL_OFFSET_Y 31             /* Y-axis offset of horizontal
                                                   thumbnail in frame */

#define VERT_THUMBNAIL_OFFSET_X  31             /* X-axis offset of vertical
                                                   thumbnail in frame */
#define VERT_THUMBNAIL_OFFSET_Y  16             /* Y-axis offset of vertical
                                                   thumbnail in frame */

/****************************************************************************
 *                                                                          *
 *  Quick scaling constants                                                 *
 *                                                                          *
 ****************************************************************************/

/* Interval of skipped pixels */
#define QUICKSCALE_NTSC          5              /* NTSC */

/* Image dimensions */
#define NTSC_PORTRAIT_WIDTH     (appContext->ac_DisplayWindow->Width)
#define NTSC_PORTRAIT_HEIGHT    205
#define NTSC_LANDSCAPE_WIDTH    256
#define NTSC_LANDSCAPE_HEIGHT   308

#define PAL_PORTRAIT_WIDTH      (appContext->ac_DisplayWindow->Width)
#define PAL_PORTRAIT_HEIGHT     256
#define PAL_LANDSCAPE_WIDTH     256
#define PAL_LANDSCAPE_HEIGHT    384

#ifdef DISPLAY_FIXED_PALETTE

/****************************************************************************
 *                                                                          *
 *  Fixed palette constants                                                 *
 *                                                                          *
 ****************************************************************************/

#define FIXED_PALETTE_GUN_RANGE     4           /* Number of entries per gun */
#define FIXED_PALETTE_GUN_STEP      85          /* Gun value step */
/* was:
#define FIXED_PALETTE_GUN_STEP      74
*/

#define FIXED_PALETTE_R_SHIFT       4           /* Pen shift for red gun */
#define FIXED_PALETTE_G_SHIFT       2           /* Pen shift for green gun */
#define FIXED_PALETTE_B_SHIFT       0           /* Pen shift for blue gun */
#define FIXED_PALETTE_GUN_MASK      0x03        /* Mask for guns */
#define FIXED_PALETTE_GUN_ROUND     43          /* Rounding value added to guns */
/* was:
#define FIXED_PALETTE_GUN_ROUND     37
*/
#define FIXED_PALETTE_GUN_MIN       0
#define FIXED_PALETTE_GUN_MAX       255
/* was:
#define FIXED_PALETTE_GUN_MIN       16
#define FIXED_PALETTE_GUN_MAX       239
*/
#endif /* DISPLAY_FIXED_PALETTE */

#endif /* APP_DISPLAY_H */
