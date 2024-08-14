/*** image.h *****************************************************************
 *
 *  $Id: image.h,v 1.6 94/03/17 16:29:15 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Image Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   image.h,v $
 * Revision 1.6  94/03/17  16:29:15  jjszucs
 * Changes per 40.13
 *
 * Revision 1.5  94/03/16  18:22:21  jjszucs
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
 * Revision 1.4  94/03/01  16:18:08  jjszucs
 * Changes to support special-casing of display modes
 * and quick scaling.
 *
 * Revision 1.3  94/02/18  15:59:18  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:09:02  jjszucs
 * Changes per version 40.3
 *
 * Revision 1.1  94/01/06  12:00:20  jjszucs
 * Initial revision
 *
*/

#ifndef APP_IMAGE_H
#define APP_IMAGE_H

/****************************************************************************
 *                                                                          *
 *  Image Node                                                              *
 *                                                                          *
 ****************************************************************************/

struct imageNode {

    /* Header */
    struct MinNode                  in_Node;        /* Minimal exec.library
                                                       list node */

    /* Properties */
    UWORD                           in_Number;      /* Image number */
    UWORD                           in_Slide;       /* Slide number */
    UWORD                           in_Sequence;    /* Sequence number */
    BOOL                            in_IsSlide;     /* Included in program? */
    BOOL                            in_Mirror;      /* Mirror? */
    USHORT                          in_Rotation;    /* Rotation (degrees CCW) */
    UBYTE                           in_Magnification;/* Magnification
                                                        (one of PHOTOCD_RES_*) */
    UWORD                           in_MagnifyX,    /* Magnification origin (X/Y) */
                                    in_MagnifyY;
    BOOL                            in_Manipulated; /* Any manipulations
                                                       applied by user? */

    /* Thumbnail */
    UBYTE *                         in_Thumbnail;   /* Chunky thumbnail bitmap */
    BOOL                            in_ThumbnailLock;/* Thumbnail bitmap locked? */

};

/****************************************************************************
 *                                                                          *
 *  Storage Node                                                            *
 *                                                                          *
 ****************************************************************************/

struct storageNode {

    BOOL                            sn_EOF;         /* EOF flag */
    UBYTE                           sn_Number;      /* Image number */
    UBYTE                           sn_Slide;       /* Slide number */
    UBYTE                           sn_Sequence;    /* Sequence number */
    UBYTE                           sn_Flags;       /* Flags */
    UBYTE                           sn_MagnifyX,    /* Magnification origin */
                                    sn_MagnifyY;    /*    (/2) */

};

#define STORAGEB_ISSLIDE 4
#define STORAGEB_MIRROR 3
#define STORAGEB_MAGNIFY 0

#define STORAGEF_ISSLIDE (1<<STORAGEB_ISSLIDE)
#define STORAGEF_MIRROR (1<<STORAGEB_MIRROR)
#define STORAGEF_MAGNIFY (1<<STORAGEB_MAGNIFY)

#define STORAGEB_ROTATE     1
#define STORAGEM_ROTATE     0x03

/****************************************************************************
 *                                                                          *
 *  Thumbnail Definitions                                                   *
 *                                                                          *
 ****************************************************************************/

/* Dimensions */
#define THUMBNAIL_WIDTH             96
#define THUMBNAIL_HEIGHT            64

/****************************************************************************
 *                                                                          *
 *  Image Definitions                                                       *
 *                                                                          *
 ****************************************************************************/

#define IMAGE_PAGE_WIDTH            384             /* Image page width */
#define IMAGE_PAGE_HEIGHT           256             /* Image page height */

#define MAX_GUN_VALUE               255             /* Maximum value for
                                                       color gun */

#define IMAGE_QUANT_X_SAMPLE        2               /* X-axis sampling interval
                                                       for quantization */
#define IMAGE_QUANT_Y_SAMPLE        2               /* Y-axis sampling interval
                                                       for quantization */

#define IMAGE_ZOOM_STEP             4               /* Zoom cursor step */

#define IMAGE_ALLOC_RES             PHOTOCD_RES_BASE    /* Resolution for
                                                            allocating image
                                                            buffer */
#define IMAGE_ALLOC_LINES           256                 /* Number of lines
                                                           to allocate in
                                                           image buffer */

#endif /* APP_IMAGE_H */
