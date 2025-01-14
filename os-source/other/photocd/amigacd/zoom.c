/*** zoom.c ******************************************************************
 *
 *  $Id: zoom.c,v 1.1 94/03/17 16:28:29 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Zoom Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   zoom.c,v $
 * Revision 1.1  94/03/17  16:28:29  jjszucs
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
#include <graphics/rastport.h>
#include <graphics/sprite.h>
#include <graphics/videocontrol.h>

#include <libraries/lowlevel.h>

#include <intuition/intuition.h>
#include <intuition/pointerclass.h>

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
#include "glyphs.h"

/****************************************************************************
 *                                                                          *
 *  Local Prototypes                                                        *
 *                                                                          *
 ****************************************************************************/

static void zoomInLeft(struct appContext *appContext);
static void zoomInRight(struct appContext *appContext);
static void zoomInUp(struct appContext *appContext);
static void zoomInDown(struct appContext *appContext);
static void zoomInSelect(struct appContext *appContext);
static void zoomInCancel(struct appContext *appContext);
static void zoomCleanup(struct appContext *appContext);
static void zoomCursor(struct appContext *appContext);

/****************************************************************************
 *                                                                          *
 *  Local Definitions                                                       *
 *                                                                          *
 ****************************************************************************/

/* Zoom cursor definitions */
#define ZOOM_SPRITE_DEPTH       2               /* Depth of zoom sprite */
#define ZOOM_SPRITE_WIDTH       32              /* Width of zoom sprite */
#define ZOOM_SPRITE_HEIGHT      45              /* Height of zoom sprite */
#define ZOOM_SPRITE_WORD_WIDTH  2               /* Word-width of zoom sprite */
#define ZOOM_SPRITE_XRES        POINTERXRESN_HIRES
#define ZOOM_SPRITE_YRES        POINTERYRESN_HIGH
#define ZOOM_SPRITE_XOFFSET     -14             /* X/Y offset of zoom sprite */
#define ZOOM_SPRITE_YOFFSET     -12

#define SPRITE_COLOR_BASE       64              /* Sprite color base */

/****************************************************************************
 *                                                                          *
 *  doZoomIn() - handle zoom in                                             *
 *                                                                          *
 ****************************************************************************/

void doZoomIn(struct appContext *appContext)
{

    static struct controlItem controlArray[] ={

        {
            ct_Button,
            ZOOM_IN_LEFT,
            zoomInLeft,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_LEFT, RAWKEY_PORT1_JOY_LEFT, RAWKEY_LEFT, NULL },
        },
        {
            ct_Button,
            ZOOM_IN_RIGHT,
            zoomInRight,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_RIGHT, RAWKEY_PORT1_JOY_RIGHT, RAWKEY_RIGHT, NULL },
        },
        {
            ct_Button,
            ZOOM_IN_UP,
            zoomInUp,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_UP, RAWKEY_PORT1_JOY_UP, RAWKEY_UP, NULL },
        },
        {
            ct_Button,
            ZOOM_IN_DOWN,
            zoomInDown,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_JOY_DOWN, RAWKEY_PORT1_JOY_DOWN, RAWKEY_DOWN, NULL },
        },
        {
            ct_Button,
            ZOOM_IN_SELECT,
            zoomInSelect,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_RED, RAWKEY_PORT1_BUTTON_RED, RAWKEY_RETURN, NULL },
        },
        {
            ct_Button,
            ZOOM_IN_CANCEL,
            zoomInCancel,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_BLUE, RAWKEY_PORT1_BUTTON_BLUE, RAWKEY_ESC, NULL },
        },
        {
            ct_Button,
            ZOOM_IN_INTERFACE,
            interfaceToggle,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_GREEN, RAWKEY_PORT1_BUTTON_GREEN, RAWKEY_HELP, NULL },
        },
        {
            ct_End
        }
    };

    struct BitMap *pSpriteBitmap;
    LONG remake;

    struct ViewPort *pViewPort;

    static ULONG spriteColors[] ={
        RGBLoad(SPRITE_COLOR_BASE, 4),  /* Load 4 colors beginning at SPRITE_COLOR_BASE */
        Gun32(0), Gun32(0), Gun32(0),
        Gun32(255), Gun32(255), Gun32(255),
        Gun32(169), Gun32(168), Gun32(190),
        Gun32(97), Gun32(71), Gun32(114),
        0                               /* Terminator */
    };

    /* Fetch display viewport */
    pViewPort=&(appContext->ac_DisplayScreen->ViewPort);

    /* Set sprite colors */
    LoadRGB32(pViewPort,
        spriteColors);

    /* Set even and odd sprite color base
       above HAM8 palette */
    remake=TRUE;
    if (VideoControlTags(pViewPort->ColorMap,
        VTAG_SPODD_BASE_SET, SPRITE_COLOR_BASE,
        VTAG_SPEVEN_BASE_SET, SPRITE_COLOR_BASE,
        VTAG_FULLPALETTE_SET, TRUE,
        VTAG_IMMEDIATE, &remake,
        TAG_DONE)==NULL) {
        if (remake) {
            MakeScreen(appContext->ac_DisplayScreen);
            RethinkDisplay();
        }
    } else {
#ifdef DEBUG
        kprintf("doZoomIn(): VideoControlTags() failure\n");
#endif /* DEBUG */
    }

    /* Get zoom pointer image */
    pSpriteBitmap=loadGlyph(appContext, GLYPH_ZOOM_CURSOR);
    if (pSpriteBitmap) {

        /* Create pointer object */
        appContext->ac_ZoomImage=
            NewObject(
                NULL,"pointerclass",
                POINTERA_BitMap, pSpriteBitmap,
                POINTERA_XOffset, ZOOM_SPRITE_XOFFSET,
                POINTERA_YOffset, ZOOM_SPRITE_YOFFSET,
                POINTERA_WordWidth, ZOOM_SPRITE_WORD_WIDTH,
                POINTERA_XResolution, POINTERXRESN_HIRES,
                POINTERA_YResolution, POINTERYRESN_HIGHASPECT,
                TAG_DONE
            );
        if (appContext->ac_ZoomImage) {

            /* Set pointer */
            SetWindowPointer(appContext->ac_DisplayWindow,
                WA_Pointer, appContext->ac_ZoomImage,
                TAG_DONE);

            /* Set-up zoom in controls */
            setControls(appContext, controlArray, NULL);

            /* Initialize zoom center and bounds */
            appContext->ac_ZoomW=
                appContext->ac_ZoomPicW/2;
            appContext->ac_ZoomH=
                appContext->ac_ZoomPicH/2;
            appContext->ac_ZoomX=
                appContext->ac_ZoomPicW/2;
            appContext->ac_ZoomY=
                appContext->ac_ZoomPicH/2;

            /* Position zoom pointer */
            zoomCursor(appContext);

            /* Activate display window (to make custom pointer visible) */
            ActivateWindow(appContext->ac_DisplayWindow);

        } else {
#ifdef DEBUG
            kprintf("doZoomIn(): Error creating pointerclass object\n");
#endif /* DEBUG */
        }

    } else {
#ifdef DEBUG
        kprintf("doZoomIn(): Error loading bitmap GLYPH_ZOOM_CURSOR\n");
#endif /* DEBUG */
    }

}

/****************************************************************************
 *                                                                          *
 *  zoomInLeft() - move zoom box left                                       *
 *                                                                          *
 ****************************************************************************/

static void zoomInLeft(struct appContext *appContext)
{

    if (appContext->ac_ZoomX>
        (appContext->ac_ZoomW/2)+IMAGE_ZOOM_STEP) {

        appContext->ac_ZoomX-=IMAGE_ZOOM_STEP;
        zoomCursor(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *  zoomInRight() - move zoom box right                                     *
 *                                                                          *
 ****************************************************************************/

static void zoomInRight(struct appContext *appContext)
{

    if (appContext->ac_ZoomX<
        appContext->ac_ZoomPicW-1-(appContext->ac_ZoomW/2)-IMAGE_ZOOM_STEP) {

        appContext->ac_ZoomX+=IMAGE_ZOOM_STEP;
        zoomCursor(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *  zoomInUp() - move zoom box up                                           *
 *                                                                          *
 ****************************************************************************/

static void zoomInUp(struct appContext *appContext)
{

    if (appContext->ac_ZoomY>
        (appContext->ac_ZoomH/2)+IMAGE_ZOOM_STEP) {

        appContext->ac_ZoomY-=IMAGE_ZOOM_STEP;
        zoomCursor(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *  zoomInDown() - move zoom box down                                       *
 *                                                                          *
 ****************************************************************************/

static void zoomInDown(struct appContext *appContext)
{

    if (appContext->ac_ZoomY<
        appContext->ac_ZoomPicH-1-(appContext->ac_ZoomH/2)-IMAGE_ZOOM_STEP) {

        appContext->ac_ZoomY+=IMAGE_ZOOM_STEP;
        zoomCursor(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *  zoomInSelect() - select zoom box                                        *
 *                                                                          *
 ****************************************************************************/

static void zoomInSelect(struct appContext *appContext)
{

    struct imageNode *imageNode;

    LONG zoomX, zoomY;
    LONG zoomW, zoomH;
    LONG dispW, dispH;
    LONG imgW, imgH;
    LONG cropX, cropY;
    LONG magnifyX, magnifyY;
    BOOL fPortrait;

#ifdef DEBUG
    kprintf("zoomInSelect(appContext=$%08lx)\n",appContext);
#endif /* DEBUG */

    /* Cleanup */
    zoomCleanup(appContext);

    /* Fetch image and zoom-related attributes */
    imageNode=appContext->ac_Selection;
    zoomX=appContext->ac_ZoomX;
    zoomY=appContext->ac_ZoomY;
    zoomW=appContext->ac_ZoomW;
    zoomH=appContext->ac_ZoomH;
    dispW=appContext->ac_ZoomPicW;
    dispH=appContext->ac_ZoomPicH;
    imgW=appContext->ac_ZoomImgW;
    imgH=appContext->ac_ZoomImgH;
    cropX=appContext->ac_ZoomCropX;
    cropY=appContext->ac_ZoomCropY;

    /* Is this a portrait image? */
    fPortrait=imageNode->in_Rotation%180?TRUE:FALSE;

#ifdef DEBUG
    kprintf("   zoomX/Y=(%ld,%ld)\n",
        zoomX, zoomY);
    kprintf("   zoomW/H=(%ld,%ld)\n",
        zoomW, zoomH);
    kprintf("   dispW/H=(%ld,%ld)\n",
        dispW, dispH);
    kprintf("   imgW/H=(%ld,%ld)\n",
        imgW, imgH);
    kprintf("   cropX/Y=(%ld,%ld)\n",
        cropX, cropY);
#endif /* DEBUG */

    /* Convert zoom center to image coordinate system */
    magnifyX=cropX+
        (zoomX * ( imgW - cropX*2 ) ) / dispW;
    magnifyY=cropY+
        (zoomY * ( imgH - cropY*2 ) ) / dispH;

#ifdef DEBUG
    kprintf("   Image zoom center X/Y=(%ld,%ld)\n", magnifyX, magnifyY);
#endif /* DEBUG */

    /* Translate zoom center to zoom origin */
    magnifyX-=imgW/4;
    magnifyY-=imgH/4;
#ifdef DEBUG
    kprintf("   Image zoom origin X/Y=(%ld,%ld)\n", magnifyX, magnifyY);
#endif /* DEBUG */

    /* Set magnification */
    imageNode->in_Magnification=PHOTOCD_RES_BASE;
    imageNode->in_MagnifyX=(fPortrait?magnifyY:magnifyX)*2;
    imageNode->in_MagnifyY=(fPortrait?magnifyX:magnifyY)*2;
#ifdef DEBUG
    kprintf("   in_MagnifyX/Y=(%ld,%ld)\n",
        imageNode->in_MagnifyX, imageNode->in_MagnifyY);
#endif /* DEBUG */

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

/****************************************************************************
 *                                                                          *
 *  zoomInCancel() - cancel zoom in                                         *
 *                                                                          *
 ****************************************************************************/

static void zoomInCancel(struct appContext *appContext)
{

#ifdef DEBUG
    kprintf("zoomInCancel(appContext=$%08lx)\n",appContext);
#endif /* DEBUG */


    /* Cleanup */
    zoomCleanup(appContext);

    /* Return to manipulation menu */
    manipulateDo(appContext);

}

/****************************************************************************
 *                                                                          *
 *  zoomCleanup() - zoom cleanup                                            *
 *                                                                          *
 ****************************************************************************/

static void zoomCleanup(struct appContext *appContext)
{

    /* Restore blank pointer */
    BlankWindowPointer(appContext,appContext->ac_DisplayWindow);

    /* Dispose of pointer object */
    DisposeObject(appContext->ac_ZoomImage);
    appContext->ac_ZoomImage=NULL;

}

/****************************************************************************
 *                                                                          *
 *  zoomCursor() - zoom cursor refresh                                      *
 *                                                                          *
 ****************************************************************************/

static void zoomCursor(struct appContext *appContext)
{

    WORD x, y;

    BOOL fPortrait;

    fPortrait=appContext->ac_Selection->in_Rotation%180;

    x=appContext->ac_ZoomPicX+
        (fPortrait?appContext->ac_ZoomX:appContext->ac_ZoomX*2);
    y=appContext->ac_ZoomPicY+
        (fPortrait?appContext->ac_ZoomY:appContext->ac_ZoomY*2);

    moveMouse(appContext, appContext->ac_DisplayScreen, x, y);

}
