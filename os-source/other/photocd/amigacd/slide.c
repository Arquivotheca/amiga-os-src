/*** slide.c *****************************************************************
 *
 *  $Id: slide.c,v 1.4 94/03/17 16:27:24 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Slide Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   slide.c,v $
 * Revision 1.4  94/03/17  16:27:24  jjszucs
 * Slide|Save implemented
 * Global meta-image functions implemented
 *
 * Revision 1.3  94/03/09  17:10:55  jjszucs
 * Slideshow playback code implemented.
 *
 * Slideshow Repeat, Play implemented.
 *
 * Revision 1.2  94/03/01  16:16:05  jjszucs
 * Generalized scaling algorithm now replaced with quick and simple
 * averaging of every 5th column or row into previous column or
 * row (for NTSC systems only).
 *
 * Revision 1.1  94/02/18  15:57:46  jjszucs
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
 *  slideDo()  -   slide manipulation                                       *
 *                                                                          *
 ****************************************************************************/

void slideDo(struct appContext *appContext)
{

    static struct controlItem controlArray[] ={

        {
            ct_Button,
            SLIDE_INCLUSION,
            slideInclusion,
            163, 5,
            65, 65,
            NULL, /* set at run-time */
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDE_SWAP_PREV,
            slideSwapPrevious,
            234, 5,
            65, 65,
            GLYPH_SWAP_PREVIOUS,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDE_SWAP_NEXT,
            slideSwapNext,
            305, 5,
            65, 65,
            GLYPH_SWAP_NEXT,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDESHOW_PLAY,
            slideshowPlay,
            382, 5,
            65, 65,
            NULL, /* set at run-time */
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDESHOW_REPEAT,
            slideshowRepeat,
            453, 5,
            65, 65,
            NULL, /* set at run-time */
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDE_SAVE,
            slideSave,
            530, 5,
            65, 65,
            GLYPH_SAVE, /* set at run-time */
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDE_PREV_IMAGE,
            slidePreviousImage,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_REVERSE, RAWKEY_PORT1_BUTTON_REVERSE, NULL },
        },
        {
            ct_Button,
            SLIDE_NEXT_IMAGE,
            slideNextImage,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_FORWARD, RAWKEY_PORT1_BUTTON_FORWARD, NULL },
        },
        {
            ct_Button,
            SLIDE_CANCEL,
            slideCancel,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_BLUE, RAWKEY_PORT1_BUTTON_BLUE, RAWKEY_ESC, NULL },
        },
        {
            ct_Button,
            SLIDE_INTERFACE,
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

    static struct controlItem globalControlArray[] ={
        {
            ct_Button,
            SLIDE_INCLUDE_ALL,
            slideIncludeAll,
            163, 5,
            65, 65,
            GLYPH_ADD_SLIDE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDE_EXCLUDE_ALL,
            slideExcludeAll,
            234, 5,
            65, 65,
            GLYPH_REMOVE_SLIDE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDESHOW_PLAY,
            slideshowPlay,
            311, 5,
            65, 65,
            NULL, /* set at run-time */
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDESHOW_REPEAT,
            slideshowRepeat,
            382, 5,
            65, 65,
            NULL, /* set at run-time */
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDE_SAVE,
            slideSave,
            459, 5,
            65, 65,
            GLYPH_SAVE,
            GLYPH_BUTTON_MASK,
            NULL,
        },
        {
            ct_Button,
            SLIDE_PREV_IMAGE,
            slidePreviousImage,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_REVERSE, RAWKEY_PORT1_BUTTON_REVERSE, NULL },
        },
        {
            ct_Button,
            SLIDE_NEXT_IMAGE,
            slideNextImage,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_FORWARD, RAWKEY_PORT1_BUTTON_FORWARD, NULL },
        },
        {
            ct_Button,
            SLIDE_CANCEL,
            slideCancel,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_BLUE, RAWKEY_PORT1_BUTTON_BLUE, RAWKEY_ESC, NULL },
        },
        {
            ct_Button,
            SLIDE_INTERFACE,
            interfaceToggle,
            0, 0,
            0, 0,
            GLYPH_NONE,
            GLYPH_NONE,
            { RAWKEY_PORT0_BUTTON_GREEN, RAWKEY_PORT1_BUTTON_GREEN, NULL },
        },
        {
            ct_End
        }
    };

    /* If on normal image ... */
    if (appContext->ac_Selection) {

        /* Set glyph of SLIDE_INCLUSION depending on inclusion/exclusion of
           selected slide */
        controlArray[0].cduic_Glyph=
            appContext->ac_Selection->in_IsSlide?
                GLYPH_REMOVE_SLIDE :
                GLYPH_ADD_SLIDE;

        /* Set glyph of SLIDESHOW_PLAY depending on state of play mode */
        controlArray[3].cduic_Glyph=
            appContext->ac_SlideshowPlay?
                GLYPH_PLAY_ON:
                GLYPH_PLAY_OFF;

        /* Set glyph of SLIDESHOW_REPEAT depending on state of repeat mode */
        controlArray[4].cduic_Glyph=
            appContext->ac_SlideshowRepeat?
                GLYPH_REPEAT_ON:
                GLYPH_REPEAT_OFF;

        /* Set controls with image as default */
        setControls(appContext, controlArray, &(controlArray[0]));

    } else {

        /* Set glyph of SLIDESHOW_PLAY depending on state of play mode */
        globalControlArray[2].cduic_Glyph=
            appContext->ac_SlideshowPlay?
                GLYPH_PLAY_ON:
                GLYPH_PLAY_OFF;

        /* Set glyph of SLIDESHOW_REPEAT depending on state of repeat mode */
        globalControlArray[3].cduic_Glyph=
            appContext->ac_SlideshowRepeat?
                GLYPH_REPEAT_ON:
                GLYPH_REPEAT_OFF;

        /* Set controls with image as default */
        setControls(appContext, globalControlArray, &(globalControlArray[0]));

    }

    /* Enable SLIDESHOW_PLAY only if slide show is not empty */
    controlEnable(appContext, SLIDESHOW_PLAY,
        appContext->ac_NImages!=0);

    /* Enable TOGGLE INTERFACE only in image state */
    controlEnable(appContext, SLIDE_INTERFACE,
        appContext->ac_State==as_Image);

}

/****************************************************************************
 *                                                                          *
 *  slideInclusion()  -   slide inclusion                                   *
 *                                                                          *
 ****************************************************************************/

void slideInclusion(struct appContext *appContext)
{

    UWORD row, column;

    struct imageNode *imageNode, *thisImageNode;

    /* Get image node */
    imageNode=appContext->ac_Selection;

    /* Toggle inclusion/exclusion of current slide */
    imageNode->in_IsSlide=!imageNode->in_IsSlide;

    /* Update slide count */
    appContext->ac_NSlides+=imageNode->in_IsSlide?1:-1;

    /* Update slide numbers of subsequent slides */
    for (thisImageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
         thisImageNode->in_Node.mln_Succ;
         thisImageNode=(struct imageNode *) thisImageNode->in_Node.mln_Succ) {

        if (thisImageNode->in_Slide>=imageNode->in_Slide &&
            thisImageNode!=imageNode) {

            thisImageNode->in_Slide+=imageNode->in_IsSlide?1:-1;

        }

    }

    /* If in thumbnail state ... */
    if (appContext->ac_State==as_Thumbnail) {

        /* Update thumbnail display */
        thumbnailGridPosition(appContext, imageNode, &row, &column);
        putThumbnailFrame(appContext, imageNode, row, column);

    }

    /* Update status */
    updateStatus(appContext);

    /* Update SLIDE_INCLUSION control */
    controlChangeGlyph(appContext, SLIDE_INCLUSION,
        imageNode->in_IsSlide ? GLYPH_REMOVE_SLIDE : GLYPH_ADD_SLIDE);

    /* Enable SLIDESHOW_PLAY only if slide show is not empty */
    controlEnable(appContext, SLIDESHOW_PLAY,
        appContext->ac_NImages!=0);

}

/****************************************************************************
 *                                                                          *
 *  slideIncludeAll()  -   include all slides                               *
 *                                                                          *
 ****************************************************************************/

void slideIncludeAll(struct appContext *appContext)
{

    struct imageNode *imageNode;
    UWORD added;

    /* Clear counter of added images */
    added=0;

    /* Loop through all images */
    imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
    while (imageNode->in_Node.mln_Succ) {

        /* Update slide number */
        imageNode->in_Slide+=added;

        /* If this image is excluded ... */
        if (!imageNode->in_IsSlide) {

            /* Include */
            imageNode->in_IsSlide=TRUE;
            added++;

        }

        imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ;

    }

    /* Update thumbnail display */
    if (appContext->ac_State==as_Thumbnail) {

        newDisplay(appContext, as_Thumbnail, FALSE);

    }

    /* Update status */
    updateStatus(appContext);

    /* Restore cursor */
    thumbnailCursor(appContext, appContext->ac_Selection, TRUE);

    /* Enable SLIDESHOW_PLAY only if slide show is not empty */
    controlEnable(appContext, SLIDESHOW_PLAY,
        appContext->ac_NImages!=0);

}

/****************************************************************************
 *                                                                          *
 *  slideExcludeAll()  -   exclude all slides                               *
 *                                                                          *
 ****************************************************************************/

void slideExcludeAll(struct appContext *appContext)
{

    struct imageNode *imageNode;
    UWORD removed;

    /* Clear counter of removed images */
    removed=0;

    /* Loop through all images */
    imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
    while (imageNode->in_Node.mln_Succ) {

        /* Update slide number */
        imageNode->in_Slide-=removed;

        /* If this image is included ... */
        if (imageNode->in_IsSlide) {

            /* Exclude */
            imageNode->in_IsSlide=FALSE;
            removed++;

        }

        imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ;

    }

    /* Update thumbnail display */
    if (appContext->ac_State==as_Thumbnail) {

        newDisplay(appContext, as_Thumbnail, FALSE);

    }

    /* Update status */
    updateStatus(appContext);

    /* Restore cursor */
    thumbnailCursor(appContext, appContext->ac_Selection, TRUE);

    /* Enable SLIDESHOW_PLAY only if slide show is not empty */
    controlEnable(appContext, SLIDESHOW_PLAY,
        appContext->ac_NImages!=0);

}

/****************************************************************************
 *                                                                          *
 *  slideSwapPrevious()  -   swap slide with previous                       *
 *                                                                          *
 ****************************************************************************/

void slideSwapPrevious(struct appContext *appContext)
{

    struct imageNode *thisImageNode, *previousImageNode;

    UWORD row, column;
    UWORD swap;
    BOOL firstSlide;

    /* Get this image node */
    thisImageNode=appContext->ac_Selection;

    /* Find previous image node */
    previousImageNode=(struct imageNode *) thisImageNode->in_Node.mln_Pred;
    if (previousImageNode==(struct imageNode *) &appContext->ac_ImageList) {

        previousImageNode=
            (struct imageNode *) appContext->ac_ImageList.mlh_TailPred;
        firstSlide=TRUE;

    } else {

        firstSlide=FALSE;

    }

    /* If in thumbnail state ... */
    if (appContext->ac_State==as_Thumbnail) {

        /* Erase thumbnail cursor */
        thumbnailCursor(appContext, thisImageNode, FALSE);

    }

    /* Move previous image node after this image node */
    if (firstSlide) {

        Remove((struct Node *) thisImageNode);

        Remove((struct Node *) previousImageNode);

        AddHead((struct List *) &appContext->ac_ImageList,
            (struct Node *) previousImageNode);
        AddTail((struct List *) &appContext->ac_ImageList,
            (struct Node *) thisImageNode);

    } else {

        Remove((struct Node *) previousImageNode);

        Insert(
            (struct List *) &appContext->ac_ImageList,
            (struct Node *) previousImageNode,
            (struct Node *) thisImageNode);

    }

    /* Update sequence and slide numbers */
    swap=thisImageNode->in_Sequence;
    thisImageNode->in_Sequence=previousImageNode->in_Sequence;
    previousImageNode->in_Sequence=swap;

    swap=thisImageNode->in_Slide;
    thisImageNode->in_Slide=previousImageNode->in_Slide;
    previousImageNode->in_Slide=swap;

    /* If in thumbnail state ... */
    if (appContext->ac_State==as_Thumbnail) {

        /* Update thumbnail display */
        thumbnailGridPosition(appContext, thisImageNode, &row, &column);
        putThumbnail(appContext, thisImageNode, row, column);

        thumbnailGridPosition(appContext, previousImageNode, &row, &column);
        putThumbnail(appContext, previousImageNode, row, column);

        /* Scroll to reveal moved slide */
        thumbnailScroll(appContext, thisImageNode);

        /* Redraw cursor */
        thumbnailCursor(appContext, thisImageNode, TRUE);

    }

    /* Update status */
    updateStatus(appContext);

}

/****************************************************************************
 *                                                                          *
 *  slideSwapNext()  -   swap slide with next                               *
 *                                                                          *
 ****************************************************************************/

void slideSwapNext(struct appContext *appContext)
{

    struct imageNode *thisImageNode, *nextImageNode;

    UWORD row, column;
    UWORD swap;
    BOOL lastSlide;

    /* Get this image node */
    thisImageNode=appContext->ac_Selection;

    /* Find next image node */
    nextImageNode=(struct imageNode *) thisImageNode->in_Node.mln_Succ;
    if (nextImageNode->in_Node.mln_Succ==NULL) {

        nextImageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;
        lastSlide=TRUE;

    } else {

        lastSlide=FALSE;

    }

    /* If in thumbnail state ... */
    if (appContext->ac_State==as_Thumbnail) {

        /* Erase thumbnail cursor */
        thumbnailCursor(appContext, thisImageNode, FALSE);

    }

    /* Move this image node after next image node */
    if (lastSlide) {

        Remove((struct Node *) thisImageNode);
        Remove((struct Node *) nextImageNode);

        AddHead((struct List *) &appContext->ac_ImageList,
            (struct Node *) thisImageNode);
        AddTail((struct List *) &appContext->ac_ImageList,
            (struct Node *) nextImageNode);

    } else {

        Remove((struct Node *) thisImageNode);

        Insert(
            (struct List *) &appContext->ac_ImageList,
            (struct Node *) thisImageNode,
            (struct Node *) nextImageNode);

    }

    /* Update sequence and slide numbers */
    swap=thisImageNode->in_Sequence;
    thisImageNode->in_Sequence=nextImageNode->in_Sequence;
    nextImageNode->in_Sequence=swap;

    swap=thisImageNode->in_Slide;
    thisImageNode->in_Slide=nextImageNode->in_Slide;
    nextImageNode->in_Slide=swap;

    /* If in thumbnail state ... */
    if (appContext->ac_State==as_Thumbnail) {

        /* Update thumbnail display */
        thumbnailGridPosition(appContext, thisImageNode, &row, &column);
        putThumbnail(appContext, thisImageNode, row, column);

        thumbnailGridPosition(appContext, nextImageNode, &row, &column);
        putThumbnail(appContext, nextImageNode, row, column);

        /* Scroll to reveal moved slide */
        thumbnailScroll(appContext, thisImageNode);

        /* Redraw cursor */
        thumbnailCursor(appContext, thisImageNode, TRUE);

    }

    /* Update status */
    updateStatus(appContext);

}

/****************************************************************************
 *                                                                          *
 *  slideCancel()  -   cancel slide operations                              *
 *                                                                          *
 ****************************************************************************/

void slideCancel(struct appContext *appContext)
{

    /* Switch on state */
    switch (appContext->ac_State) {

        /* Thumbnail */
        case as_Thumbnail:
            /* Return to thumbnail ops control panel with slide selected */
            thumbnailDo(appContext);
            controlSelect(appContext, THUMBNAIL_OP_SLIDE);
            break;

        case as_Image:
            /* Return to image control panel with slide selected */
            newControl(appContext, appContext->ac_State);
            controlSelect(appContext, IMAGE_SLIDE);
            /* Force interface visible */
            showInterface(appContext);
            break;

    }

}

/****************************************************************************
 *                                                                          *
 *  slidePreviousImage()  -   go to previous image                          *
 *                                                                          *
 ****************************************************************************/

void slidePreviousImage(struct appContext *appContext)
{

    /* Go to next image */
    switch (appContext->ac_State) {

        case as_Thumbnail:
            thumbnailPreviousImage(appContext);
            break;

        case as_Image:
            imagePreviousImage(appContext);
            break;

    }

    /* Update SLIDE_INCLUSION control */
    controlChangeGlyph(appContext, SLIDE_INCLUSION,
        appContext->ac_Selection->in_IsSlide ?
            GLYPH_REMOVE_SLIDE : GLYPH_ADD_SLIDE);

}

/****************************************************************************
 *                                                                          *
 *  slideNextImage()  -   go to next image                                  *
 *                                                                          *
 ****************************************************************************/

void slideNextImage(struct appContext *appContext)
{

    /* Go to next image */
    switch (appContext->ac_State) {

        case as_Thumbnail:
            thumbnailNextImage(appContext);
            break;

        case as_Image:
            imageNextImage(appContext);
            break;

    }

    /* Update SLIDE_INCLUSION control */
    controlChangeGlyph(appContext, SLIDE_INCLUSION,
        appContext->ac_Selection->in_IsSlide ?
            GLYPH_REMOVE_SLIDE : GLYPH_ADD_SLIDE);

}

/****************************************************************************
 *                                                                          *
 *  slideSave()  -   save slide program                                     *
 *                                                                          *
 ****************************************************************************/

void slideSave(struct appContext *appContext)
{

#ifdef DEBUG
    kprintf("slideSave(appContext=$%08lx)\n", appContext);
#endif /* DEBUG */

    /* Save program */
    saveProgram(appContext);

}
