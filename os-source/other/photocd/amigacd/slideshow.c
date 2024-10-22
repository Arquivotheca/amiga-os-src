/*** slideshow.c ************************************************************
 *
 *  $Id: slideshow.c,v 1.2 94/03/09 17:11:37 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Slideshow Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   slideshow.c,v $
 * Revision 1.2  94/03/09  17:11:37  jjszucs
 * Slideshow playback code implemented.
 *
 * Slideshow control panel implemented.
 *
 * Revision 1.1  94/02/18  15:58:00  jjszucs
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
 *  Local Definitions                                                       *
 *                                                                          *
 ****************************************************************************/

/* Slideshow interval */
#define SLIDESHOW_INTERVAL_SECS         20      /* Interval (secs) between
                                                   auto-advance */

/****************************************************************************
 *                                                                          *
 *  Local Prototypes                                                        *
 *                                                                          *
 ****************************************************************************/

static void slideshowStartTimer(struct appContext *appContext);
static void slideshowStopTimer(struct appContext *appContext);

/****************************************************************************
 *                                                                          *
 *  slideshowPlay() - play slideshow                                        *
 *                                                                          *
 ****************************************************************************/

void slideshowPlay(struct appContext *appContext)
{

    static struct controlItem controlArray[] ={

        {
            ct_Button,
            SLIDESHOW_PREV_IMAGE,
            slideshowPrevious,
            163, 5,
            65, 65,
            GLYPH_PREV_IMAGE,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_REVERSE, RAWKEY_PORT1_BUTTON_REVERSE, NULL },
        },
        {
            ct_Button,
            SLIDESHOW_PLAY,
            slideshowPlay,
            234, 5,
            65, 65,
            GLYPH_PLAY_ON,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_PLAY, RAWKEY_PORT1_BUTTON_PLAY, NULL },
        },
        {
            ct_Button,
            SLIDESHOW_NEXT_IMAGE,
            slideshowNext,
            305, 5,
            65, 65,
            GLYPH_NEXT_IMAGE,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_FORWARD, RAWKEY_PORT1_BUTTON_FORWARD, NULL },
        },
        {
            ct_Button,
            SLIDESHOW_STOP,
            slideshowStop,
            376, 5,
            65, 65,
            GLYPH_STOP,
            GLYPH_BUTTON_MASK,
            { RAWKEY_PORT0_BUTTON_BLUE, RAWKEY_PORT1_BUTTON_BLUE, RAWKEY_ESC, NULL },
        },
        {
            ct_Button,
            SLIDESHOW_INTERFACE,
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

#ifdef DEBUG
    kprintf("slideshowPlay(appContext=$%08lx)\n", appContext);
#endif /* DEBUG */

    /* If not already playing ... */
    if (!appContext->ac_SlideshowPlay) {

#ifdef DEBUG
        kprintf("   Starting play\n");
#endif /* DEBUG */

        /* If no slides ... */
        if (appContext->ac_NSlides==0) {

            /* No action */
            return;

        }

        /* Set controls with PLAY/PAUSE as default */
        setControls(appContext, controlArray, &(controlArray[1]));

        /* Set slideshow play */
        appContext->ac_SlideshowPlay=TRUE;

        /* Clear pause */
        appContext->ac_SlideshowPause=FALSE;

        /* Preserve state and selection on entry */
        appContext->ac_SlideshowSelect=appContext->ac_Selection;
        appContext->ac_SlideshowState=appContext->ac_State;

        /* Go to first slide */
        appContext->ac_Selection=NULL;
        slideshowNext(appContext);

    /* ... else ... */
    } else {

#ifdef DEBUG
        kprintf("   Toggling pause\n");
#endif /* DEBUG */

        /* Already playing; toggle pause */
        appContext->ac_SlideshowPause=!appContext->ac_SlideshowPause;

        /* If pausing ... */
        if (appContext->ac_SlideshowPause) {

            /* Stop timer */
            slideshowStopTimer(appContext);

        /* ... else (unpausing) ... */
        } else {

            /* Re-start timer */
            slideshowStartTimer(appContext);

        }

        /* Update SLIDESHOW_PLAY glyph */
        controlChangeGlyph(appContext, SLIDESHOW_PLAY,
            appContext->ac_SlideshowPause?
                GLYPH_PLAY_PAUSE: GLYPH_PLAY_ON);

    }

}

/****************************************************************************
 *                                                                          *
 *  slideshowRepeat() - toggle repeat state of slideshow                    *
 *                                                                          *
 ****************************************************************************/

void slideshowRepeat(struct appContext *appContext)
{

#ifdef DEBUG
    kprintf("slideshowRepeat(appContext=$%08lx)\n",appContext);
#endif /* DEBUG */

    /* Toggle slideshow repeat */
    appContext->ac_SlideshowRepeat=!appContext->ac_SlideshowRepeat;
#ifdef DEBUG
    kprintf("   Repeat turned %s\n", appContext->ac_SlideshowRepeat?"ON":"OFF");
#endif /* DEBUG */

    /* Update SLIDESHOW_REPEAT control */
    controlChangeGlyph(appContext, SLIDESHOW_REPEAT,
        appContext->ac_SlideshowRepeat?
        GLYPH_REPEAT_ON:
        GLYPH_REPEAT_OFF);

}

/****************************************************************************
 *                                                                          *
 *  slideshowNext() - go to next slide                                      *
 *                                                                          *
 ****************************************************************************/

void slideshowNext(struct appContext *appContext)
{

    struct imageNode *imageNode;

#ifdef DEBUG
    kprintf("slideshowNext(appContext=$%08lx)\n",appContext);
#endif /* DEBUG */

    /* Abort pending timer request */
    slideshowStopTimer(appContext);

    /* If no selection ... */
    if (appContext->ac_Selection==NULL) {

        /* Begin with first image */
        imageNode=(struct imageNode *) appContext->ac_ImageList.mlh_Head;

    /* ... else ... */
    } else {

        /* Begin with next image */
        imageNode=(struct imageNode *) appContext->ac_Selection->in_Node.mln_Succ;

    }

    /* Loop through images ... */
    while (imageNode->in_Node.mln_Succ) {

        /* If this is a slide ... */
        if (imageNode->in_IsSlide) {

            /* Show this slide */
            slideshowShow(appContext, imageNode);
            return;

        }

        imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ;

    }

    /* On last slide; if in repeat mode ... */
    if (appContext->ac_SlideshowRepeat) {

        /* Repeat */
        appContext->ac_Selection=NULL;
        slideshowNext(appContext);

    /* ... else ... */
    } else {

        /* Stop */
        slideshowStop(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *  slideshowPrevious() - go to previous slide                              *
 *                                                                          *
 ****************************************************************************/

void slideshowPrevious(struct appContext *appContext)
{

    struct imageNode *imageNode;

#ifdef DEBUG
    kprintf("slideshowPrevious(appContext=$%08lx)\n", appContext);
#endif /* DEBUG */

    /* Abort pending timer request */
    slideshowStopTimer(appContext);

    /* If no selection ... */
    if (appContext->ac_Selection==NULL) {

        /* Begin with last image */
        imageNode=
            (struct imageNode *) appContext->ac_ImageList.mlh_TailPred;

    /* ... else */
    } else {

        /* Begin with previous image */
        imageNode=
            (struct imageNode *) appContext->ac_Selection->in_Node.mln_Pred;

        /* If going before first image ... */
        if (imageNode==(struct imageNode *) &appContext->ac_ImageList) {

            /* Go to last image */
            imageNode=
                (struct imageNode *) appContext->ac_ImageList.mlh_TailPred;

        }

    }

    /* Loop through images ... */
    while (imageNode!=(struct imageNode *) &appContext->ac_ImageList) {

        /* If this is a slide ... */
        if (imageNode->in_IsSlide) {

            /* Show this slide */
            slideshowShow(appContext, imageNode);
            return;

        }

        imageNode=(struct imageNode *) imageNode->in_Node.mln_Succ;

    }

}

/****************************************************************************
 *                                                                          *
 *  slideshowStop() - stop slideshow                                        *
 *                                                                          *
 ****************************************************************************/

void slideshowStop(struct appContext *appContext)
{

#ifdef DEBUG
    kprintf("slideshowStop(appContext=$%08lx)\n", appContext);
#endif /* DEBUG */

    /* Stop timer */
    slideshowStopTimer(appContext);

    /* Clear slideshow playing and paused flags */
    appContext->ac_SlideshowPlay=FALSE;
    appContext->ac_SlideshowPause=FALSE;

    /* Restore original selection */
    appContext->ac_Selection=appContext->ac_SlideshowSelect;

    /* Restore original state */
    newState(appContext, appContext->ac_SlideshowState);

}

/****************************************************************************
 *                                                                          *
 *  slideshowShow() - show a slide                                          *
 *                                                                          *
 ****************************************************************************/

void slideshowShow(struct appContext *appContext, struct imageNode *imageNode)
{

#ifdef DEBUG
    kprintf("slideshowShow(appContext=$%08lx, imageNode=$%08lx)\n",
        appContext, imageNode);
    kprintf("   Showing image %ld\n",imageNode->in_Number);
#endif /* DEBUG */

    /* Select image */
    appContext->ac_Selection=imageNode;

    /* Force into image state */
    appContext->ac_State=as_Image;

    /* Update display pane */
    newDisplay(appContext, as_Image,
        imageNode?
            (imageNode->in_Rotation%180 ? TRUE:FALSE):
            FALSE
    );

    /* Update status panel */
    newStatus(appContext, as_Image);

    /* Update display */
    imageSelect(appContext, imageNode);

    /* Start timer */
    slideshowStartTimer(appContext);

}

/****************************************************************************
 *                                                                          *
 *  slideshowTimer() - handle timer event in slideshow                      *
 *                                                                          *
 ****************************************************************************/

void slideshowTimer(struct appContext *appContext)
{

#ifdef DEBUG
    kprintf("slideshowTimer(appContext=$%08lx)\n", appContext);
#endif /* DEBUG */

    /* Go to next image */
    slideshowNext(appContext);

}

/****************************************************************************
 *                                                                          *
 *  slideshowStartTimer() - start slideshow timer                           *
 *                                                                          *
 ****************************************************************************/

static void slideshowStartTimer(struct appContext *appContext)
{

    /* Abort any active timer request */
    slideshowStopTimer(appContext);

    /* Clear timer signal, to handle contingency of signal
       remaining on after WaitIO(). For more information,
       see exec.library/WaitIO() documentation. */
    SetSignal(
        0L,
        1<<appContext->ac_TimerPort->mp_SigBit);

    /* Issue new timer request */
    appContext->ac_TimerRequest->tr_node.io_Command=TR_ADDREQUEST;
    appContext->ac_TimerRequest->tr_time.tv_secs=SLIDESHOW_INTERVAL_SECS;
    appContext->ac_TimerRequest->tr_time.tv_micro=0L;
    SendIO((struct IORequest *) appContext->ac_TimerRequest);

    /* Timer is now active */
    appContext->ac_TimerActive=TRUE;

}

/****************************************************************************
 *                                                                          *
 *  slideshowStopTimer() - stop slideshow timer                             *
 *                                                                          *
 ****************************************************************************/

static void slideshowStopTimer(struct appContext *appContext)
{

    /* If timer I/O request is active ... */
    if (appContext->ac_TimerActive) {

        /* Abort timer I/O request */
        AbortIO(appContext->ac_TimerRequest);

        /* Wait for abort to return */
        WaitIO(appContext->ac_TimerRequest);

    }

    appContext->ac_TimerActive=FALSE;

}
