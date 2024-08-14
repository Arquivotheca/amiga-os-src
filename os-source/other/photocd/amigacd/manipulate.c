/*** manipulate.c ***********************************************************
 *
 *  $Id: manipulate.c,v 1.3 94/03/16 18:21:04 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Image Manipulation Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   manipulate.c,v $
 * Revision 1.3  94/03/16  18:21:04  jjszucs
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
 * Revision 1.2  94/03/01  16:14:41  jjszucs
 * Mask now used for control panel buttons.
 *
 * Revision 1.1  94/02/18  15:56:51  jjszucs
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
#include <math.h>

/* Amiga includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>

#include <libraries/lowlevel.h>

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
#include "interface.h"
#include "image.h"

/****************************************************************************
 *                                                                          *
 *  manipulateDo()  -   image manipulation                                  *
 *                                                                          *
 ****************************************************************************/

void manipulateDo(struct appContext *appContext)
{

    static struct controlItem controlArray[] ={

        {
            ct_Button,
            MANIPULATE_MIRROR,
            manipulateMirror,
            163, 5,
            65, 65,
            GLYPH_MIRROR,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            MANIPULATE_ROTATE,
            manipulateRotate,
            234, 5,
            65, 65,
            GLYPH_ROTATE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            MANIPULATE_ZOOM,
            manipulateZoom,
            305, 5,
            65, 65,
            NULL, /* set at run-time */
            GLYPH_BUTTON_MASK,
            NULL
        },
        {
            ct_Button,
            MANIPULATE_NORMALIZE,
            manipulateNormalize,
            376, 5,
            65, 65,
            GLYPH_NORMALIZE,
            GLYPH_BUTTON_MASK,
            NULL
        },
        {
            ct_Button,
            MANIPULATE_CANCEL,
            manipulateCancel,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_BLUE, RAWKEY_PORT1_BUTTON_BLUE, RAWKEY_ESC, NULL },
        },
        {
            ct_Button,
            MANIPULATE_INTERFACE,
            interfaceToggle,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_GREEN, RAWKEY_PORT1_BUTTON_GREEN, RAWKEY_HELP, NULL },
        },
        {
            ct_End
        },
    };

    /* Set glyph of MANIPULATE_ZOOM depending on magnification of selected
        slide */
    controlArray[2].cduic_Glyph=
        (appContext->ac_Selection->in_Magnification==PHOTOCD_RES_BASE4)?
            GLYPH_ZOOM_IN:
            GLYPH_ZOOM_OUT;

    /* Set controls with mirror as default */
    setControls(appContext, controlArray, &(controlArray[0]));

    /* Enable TOGGLE INTERFACE only in image state */
    controlEnable(appContext, MANIPULATE_INTERFACE,
        appContext->ac_State==as_Image);

    /* Enable NORMALIZE only if image is manipulated */
    controlEnable(appContext, MANIPULATE_NORMALIZE,
        appContext->ac_Selection->in_Manipulated);

}

/****************************************************************************
 *                                                                          *
 *  manipulateMirror()  -   mirror image                                    *
 *                                                                          *
 ****************************************************************************/

void manipulateMirror(struct appContext *appContext)
{

    struct imageNode *imageNode;

    /* Mirror image */
    imageNode=appContext->ac_Selection;
    imageNode->in_Mirror=!imageNode->in_Mirror;

    /* Image is manipulated */
    imageNode->in_Manipulated=TRUE;

    /* Re-display image if necessary */
    if (appContext->ac_State==as_Image) {
        EraseRect(appContext->ac_DisplayWindow->RPort,
            0, 0,
            appContext->ac_DisplayWindow->Width-1,
            appContext->ac_DisplayWindow->Height-1);
        displayImage(appContext, imageNode);
    }

    /* Unload thumbnail, forcing re-loading and rotation when next
       displayed */
    unloadThumbnail(appContext, imageNode);

    /* Enable NORMALIZE button */
    controlEnable(appContext, MANIPULATE_NORMALIZE, TRUE);

}

/****************************************************************************
 *                                                                          *
 *  manipulateRotate()  -  rotate image by 90 degrees CCW                    *
 *                                                                          *
 ****************************************************************************/

void manipulateRotate(struct appContext *appContext)
{

    struct imageNode *imageNode;

    /* Rotate 90o degrees CCW */
    imageNode=appContext->ac_Selection;
    imageNode->in_Rotation+=90;
    if (imageNode->in_Rotation<0) {
        imageNode->in_Rotation=360+imageNode->in_Rotation;
    }
    if (imageNode->in_Rotation>=360) {
        imageNode->in_Rotation-=360;
    }

    /* Image is manipulated */
    imageNode->in_Manipulated=TRUE;

    /* Re-display image if necessary */
    if (appContext->ac_State==as_Image) {
        EraseRect(appContext->ac_DisplayWindow->RPort,
            0, 0,
            appContext->ac_DisplayWindow->Width-1,
            appContext->ac_DisplayWindow->Height-1);
        displayImage(appContext, imageNode);
    }

    /* Unload thumbnail, forcing re-loading and updating when next
       displayed */
    unloadThumbnail(appContext, imageNode);

    /* Enable NORMALIZE button */
    controlEnable(appContext, MANIPULATE_NORMALIZE, TRUE);

}

/****************************************************************************
 *                                                                          *
 *  manipulateZoom()  -  zoom in/out on image                               *
 *                                                                          *
 ****************************************************************************/

void manipulateZoom(struct appContext *appContext)
{

    struct imageNode *imageNode;

#ifdef DEUBG
    kprintf("manipulateZoom(appContext=$%08lx)\n", appContext);
#endif /* DEBUG */

    /* Fetch image; if global meta-image ... */
    imageNode=appContext->ac_Selection;
    if (!imageNode) {

        /* No action */
        return;

    }

    /* If image is not zoomed ... */
    if (imageNode->in_Magnification==PHOTOCD_RES_BASE4) {

#ifdef DEBUG
        kprintf("\tZooming image %ld\n",imageNode->in_Number);
#endif /* DEBUG */

        doZoomIn(appContext);

    } else {

#ifdef DEBUG
        kprintf("\tUnzooming image %ld\n", imageNode->in_Number);
#endif /* DEBUG */

        /* Unzoom image */
        imageNode->in_Magnification=PHOTOCD_RES_BASE4;
        imageNode->in_MagnifyX=0;
        imageNode->in_MagnifyY=0;

        /* Image is manipulated */
        imageNode->in_Manipulated=TRUE;

        /* Re-display image if necessary */
        if (appContext->ac_State==as_Image) {
            EraseRect(appContext->ac_DisplayWindow->RPort,
                0, 0,
                appContext->ac_DisplayWindow->Width-1,
                appContext->ac_DisplayWindow->Height-1);
            displayImage(appContext, imageNode);
        }

        /* Unload thumbnail, forcing re-loading and updating when next
           displayed */
        unloadThumbnail(appContext, imageNode);

        /* Return to manipulation menu */
        manipulateDo(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *   manipulateNormalize() - normalize image                                *
 *                                                                          *
 ****************************************************************************/

void manipulateNormalize(struct appContext *appContext)
{

    struct imageNode *imageNode;
    struct TagItem *imageInfo;

    /* Obtain image information */
    imageNode=appContext->ac_Selection;
    imageInfo=ObtainPhotoCDInfo(appContext->ac_PhotoCDHandle,
        PCD_Image, imageNode->in_Number,
        TAG_DONE);
    if (imageInfo) {

        imageNode->in_Mirror=FALSE;
        imageNode->in_Rotation=GetTagData(PCDImg_Rotation, 0, imageInfo);
        imageNode->in_Magnification=PHOTOCD_RES_BASE4;
        imageNode->in_MagnifyX=0;
        imageNode->in_MagnifyY=0;
        imageNode->in_Manipulated=FALSE;

        ReleasePhotoCDInfo(imageInfo);

        /* Re-display image if necessary */
        if (appContext->ac_State==as_Image) {
            EraseRect(appContext->ac_DisplayWindow->RPort,
                0, 0,
                appContext->ac_DisplayWindow->Width-1,
                appContext->ac_DisplayWindow->Height-1);
            displayImage(appContext, imageNode);
        }

        /* Unload thumbnail, forcing re-loading and updating when next
           displayed */
        unloadThumbnail(appContext, imageNode);

        /* Disable NORMALIZE button */
        controlEnable(appContext, MANIPULATE_NORMALIZE, FALSE);

    }

}

/****************************************************************************
 *                                                                          *
 *  manipulateCancel()  -  cancel manipulation                              *
 *                                                                          *
 ****************************************************************************/

void manipulateCancel(struct appContext *appContext)
{

    /* Switch on state */
    switch (appContext->ac_State) {

        /* Image */
        case as_Image:

            /* Return to image control panel with manipulate selected */
            newControl(appContext, appContext->ac_State);
            controlSelect(appContext, IMAGE_MANIPULATE);

            /* Force interface visible */
            showInterface(appContext);
            break;

    }

}
