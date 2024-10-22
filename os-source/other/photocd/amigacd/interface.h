/*** interface.h *************************************************************
 *
 *  $Id: interface.h,v 1.7 94/03/31 16:10:33 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  User Interface Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   interface.h,v $
 * Revision 1.7  94/03/31  16:10:33  jjszucs
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
 * Revision 1.6  94/03/17  16:29:28  jjszucs
 * Changes per 40.13
 *
 * Revision 1.5  94/03/16  18:22:30  jjszucs
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
 * Revision 1.4  94/03/09  17:16:20  jjszucs
 * Changes per version 40.11.
 *
 * Revision 1.3  94/03/01  16:18:18  jjszucs
 * Changes to support masks for control panel buttons.
 *
 * Revision 1.2  94/02/18  15:59:27  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.1  94/01/06  12:00:27  jjszucs
 * Initial revision
 *
*/

#ifndef APP_INTERFACE_H

#define APP_INTERFACE_H

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
 *  User interface screen definitions                                       *
 *                                                                          *
 ****************************************************************************/

#define INTERFACE_PANE_MODEID   HIRESLACE_KEY   /* Display mode */
#define INTERFACE_PANE_OFFSETY  112             /* Offset from bottom of screen */
#define INTERFACE_PANE_HEIGHT   76              /* Height */
#define INTERFACE_PANE_DEPTH    4               /* Depth (bitplanes) */

/* Pen definitions */
#define INTERFACE_PEN_BACKGROUND    0           /* Background */
#define INTERFACE_PEN_CURSOR        1           /* Cursor */
#define INTERFACE_PEN_LED_LIGHT     5           /* Lighted LED segment */
#define INTERFACE_PEN_BUTTON_BKGD   6           /* Button background */
#define INTERFACE_PEN_WARNING       7           /* Warning */
#define INTERFACE_PEN_LED_DARK      9           /* Dark LED segment */

/* Status panel */
#define STATUS_PANEL_LEFT           0           /* Left edge */
#define STATUS_PANEL_TOP            0           /* Top edge */
#define STATUS_PANEL_WIDTH          156         /* Width */
#define STATUS_PANEL_HEIGHT         76          /* Height */

/* Cursor */
#define CURSOR_THICKNESS            4           /* Thickness of cursor */

/* Raw key codes */
#define RAWKEY_LEFT         0x4F        /* <Left> */
#define RAWKEY_RIGHT        0x4E        /* <Right> */
#define RAWKEY_UP           0x4C        /* <Up> */
#define RAWKEY_DOWN         0x4D        /* <Down> */
#define RAWKEY_RETURN       0x44        /* <Return> */
#define RAWKEY_BACKSPACE    0x41        /* <Backspace> */
#define RAWKEY_ESC          0x45        /* <Esc> */
#define RAWKEY_HELP         0x5F        /* <Help> */

/* Status panel LED positions */
#define SLIDE_LED_X                 44          /* Slide number X */
#define SLIDE_LED_Y                 14          /* Slide number Y */
#define SLIDE_SEP_X                 89          /* Slide '/' X */
#define SLIDE_SEP_Y                 18          /* Slide '/' Y */
#define SLIDE_OF_LED_X              101         /* Number of slides X */
#define SLIDE_OF_LED_Y              14          /* Number of slides Y */
#define IMAGE_LED_X                 44          /* Image number X */
#define IMAGE_LED_Y                 43          /* Image number Y */
#define IMAGE_SEP_X                 89          /* Image '/' X */
#define IMAGE_SEP_Y                 47          /* Image '/' Y */
#define IMAGE_OF_LED_X              101         /* Number of images X */
#define IMAGE_OF_LED_Y              43          /* Number of images Y */
#define LED_DIGIT_WIDTH             15          /* LED digit width */
#define LED_DIGIT_HEIGHT            21          /* LED digit height */
#define LED_DIGIT_NARROW_WIDTH      3           /* Narrow LED digit width */
#define LED_DIGIT_SPACE             19          /* Spacing between LED digits */
#define LED_DIGIT_NARROW_SPACE      7           /* Spacing between narrow
                                                   LED digits */

/* Status panel busy bar */
#define BUSY_GLYPH_X                15          /* Busy glyph X */
#define BUSY_GLYPH_Y                18          /* Busy glpyh Y */
#define BUSY_GLYPH_WIDTH            53          /* Busy glyph width */
#define BUSY_GLYPH_HEIGHT           42          /* Busy glyph height */
#define BUSY_BAR_X                  74          /* Busy bar X */
#define BUSY_BAR_Y                  15          /* Busy bar Y */
#define BUSY_BAR_WIDTH              67          /* Busy bar width */
#define BUSY_BAR_HEIGHT             48          /* Busy bar height */

/****************************************************************************
 *                                                                          *
 *  Control definitions                                                     *
 *                                                                          *
 ****************************************************************************/

/* Control types */
enum controlType {
    ct_End,         /* End of control list */
    ct_Button       /* Button */
};

/* Control item */
struct controlItem {

    enum controlType    cduic_Type;             /* Type */

    UWORD               cduic_ID;               /* Identifier */

    void                (*cduic_Callback)(struct appContext *appContext);
                                                /* Callback */

    UWORD               cduic_X,                /* Position */
                        cduic_Y;
    UWORD               cduic_Width,            /* Dimensions */
                        cduic_Height;

    UWORD               cduic_Glyph;            /* Glyph */
    UWORD               cduic_Mask;             /* Mask for glyph */

#define CONTROL_SHORTCUT_COUNT  8
    USHORT              cduic_KeyCode[CONTROL_SHORTCUT_COUNT];
                                                /* Shortcut keycode(s) */

    ULONG               cduic_Flags;            /* Flags - do not directly
                                                   set or manipulate! */

};

#define CDUICB_ENABLED   0                  /* Enable control */
#define CDUICF_ENABLED (1<<CDUICB_ENABLED)

/****************************************************************************
 *                                                                          *
 *  Control Identifiers                                                     *
 *                                                                          *
 ****************************************************************************/

/* Thumbnail state */
#define THUMBNAIL_PREV_ROW      1
#define THUMBNAIL_NEXT_ROW      2
#define THUMBNAIL_PREV_IMAGE    3
#define THUMBNAIL_NEXT_IMAGE    4
#define THUMBNAIL_DO            5
#define THUMBNAIL_EXIT          6

/* Thumbnail operations */
#define THUMBNAIL_OP_IMAGE      1
#define THUMBNAIL_OP_PREV_IMAGE 2
#define THUMBNAIL_OP_NEXT_IMAGE 3
#define THUMBNAIL_OP_SLIDE      4
#define THUMBNAIL_OP_CANCEL     5

/* Image state */
#define IMAGE_THUMBNAIL         1
#define IMAGE_PREV_IMAGE        2
#define IMAGE_NEXT_IMAGE        3
#define IMAGE_MANIPULATE        4
#define IMAGE_SLIDE             5
#define IMAGE_INTERFACE         6

/* Slide operations */
#define SLIDE_INCLUSION         1
#define SLIDE_SWAP_PREV         2
#define SLIDE_SWAP_NEXT         3
#define SLIDE_PREV_IMAGE        4
#define SLIDE_NEXT_IMAGE        5
#define SLIDE_CANCEL            6
#define SLIDE_INTERFACE         7
#define SLIDE_INCLUDE_ALL       8
#define SLIDE_EXCLUDE_ALL       9
#define SLIDE_SAVE              10

/* Slideshow operations */
/* N.B.: The 10000 series of control identifiers are used for slideshow
         operations that are valid in multiple contexts, allowing the
         controls to be addressed unambigously in all applicable
         contexts. */
#define SLIDESHOW_PLAY          10001
#define SLIDESHOW_REPEAT        10002
#define SLIDESHOW_PREV_IMAGE    3
#define SLIDESHOW_NEXT_IMAGE    4
#define SLIDESHOW_STOP          5
#define SLIDESHOW_INTERFACE     6

/* Manipulation operations */
#define MANIPULATE_MIRROR       1
#define MANIPULATE_ROTATE       2
#define MANIPULATE_ZOOM         3
#define MANIPULATE_NORMALIZE    4
#define MANIPULATE_CANCEL       5
#define MANIPULATE_INTERFACE    6

/* Zoom in */
#define ZOOM_IN_LEFT    1
#define ZOOM_IN_RIGHT   2
#define ZOOM_IN_UP      3
#define ZOOM_IN_DOWN    4
#define ZOOM_IN_SELECT  5
#define ZOOM_IN_CANCEL  6
#define ZOOM_IN_INTERFACE 7

/* Application */
#define APP_EXIT        99

/****************************************************************************
 *                                                                          *
 *  Command-line interface                                                  *
 *                                                                          *
 ****************************************************************************/

#define CLI_TEMPLATE        "NOEXIT/S"

struct cliArguments {

    LONG        noExit;         /* NOEXIT */

};

#endif /* APP_INTERFACE_H */
