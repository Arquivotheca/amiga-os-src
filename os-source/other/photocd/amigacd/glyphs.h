/*** glpyhs.h ***************************************************************
 *
 *  $Id: glyphs.h,v 1.6 94/03/17 16:29:03 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Glyphs Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   glyphs.h,v $
 * Revision 1.6  94/03/17  16:29:03  jjszucs
 * Slide|Save glyph added.
 *
 * Revision 1.5  94/03/16  18:22:01  jjszucs
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
 * Revision 1.4  94/03/09  17:14:23  jjszucs
 * Changes per version 40.11.
 *
 * Revision 1.3  94/03/08  14:07:41  jjszucs
 * Changes per 40.10.
 *
 * Revision 1.2  94/03/01  16:17:51  jjszucs
 * Changes to support special-casing of display modes
 * and quick scaling.
 *
 * Revision 1.1  94/01/06  12:00:14  jjszucs
 * Initial revision
 *
*/

#ifndef APP_GLYPHS_H

#define APP_GLYPHS_H

/****************************************************************************
 *                                                                          *
 *  Glyph Identifiers                                                       *
 *                                                                          *
 ****************************************************************************/

#define GLYPH_STATUS_PANEL      0
#define GLYPH_NO_DISC           1
#define GLYPH_INVALID_DISC      2
#define GLYPH_PHOTOCD_LOGO      3
#define GLYPH_IMAGE_FRAME_HORIZ 4
#define GLYPH_IMAGE_FRAME_VERT  5
#define GLYPH_SLIDE_FRAME_HORIZ 6
#define GLYPH_SLIDE_FRAME_VERT  7
#define GLYPH_PAUSE             8
#define GLYPH_STOP              9
#define GLYPH_REPEAT_OFF        10
#define GLYPH_REPEAT_ON         11
#define GLYPH_NORMALIZE         12
#define GLYPH_PREV_IMAGE        13
#define GLYPH_NEXT_IMAGE        14
#define GLYPH_IMAGE             15
#define GLYPH_THUMBNAIL         16
#define GLYPH_PLAY_OFF          17
#define GLYPH_PLAY_ON           18
#define GLYPH_MIRROR            19
#define GLYPH_ROTATE            20
#define GLYPH_PAN               21
#define GLYPH_ZOOM_IN           22
#define GLYPH_ZOOM_OUT          23
#define GLYPH_SWAP_NEXT         24
#define GLYPH_SWAP_PREVIOUS     25
#define GLYPH_REMOVE_SLIDE      26
#define GLYPH_ADD_SLIDE         27
#define GLYPH_INTERVAL          28

#define GLYPH_LEDSEG            29
#define GLYPH_LEDSEGNARROW      30
#define GLYPH_LED1NARROW        31
#define GLYPH_LEDDIGITDASH      32

/* N.B.: displayLED() depends on the fact that GLYPH_LED0 ... GLYPH_LED9
         are sequential */
#define GLYPH_LED0              33
#define GLYPH_LED1              34
#define GLYPH_LED2              35
#define GLYPH_LED3              36
#define GLYPH_LED4              37
#define GLYPH_LED5              38
#define GLYPH_LED6              39
#define GLYPH_LED7              40
#define GLYPH_LED8              41
#define GLYPH_LED9              42

#define GLYPH_LEDSLASH          43
#define GLYPH_LEDSLASHSEG       44

#define GLYPH_SLIDE             45
#define GLYPH_MANIPULATE        46
#define GLYPH_BUSY              47

#define GLYPH_RAISED_BUTTON     48
#define GLYPH_DEPRESSED_BUTTON  49

#define GLYPH_PANEL_BKGD        50

#define GLYPH_BUTTON_MASK       51

#define GLYPH_CREDITS           52

#define GLYPH_PLAY_PAUSE        53

#define GLYPH_ZOOM_CURSOR       54

#define GLYPH_BUSY_THUMBNAIL    55
#define GLYPH_BUSY_IMAGE        56

#define GLYPH_EXIT              57

#define GLYPH_SAVE              58

#define GLYPH_COUNT             59  /* Number of glyphs */

#define GLYPH_NONE              Largest(UWORD)  /* Null glyph */

#endif /* APP_GLYPHS_H */
