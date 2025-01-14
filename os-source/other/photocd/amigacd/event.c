/*** event.c *****************************************************************
 *
 *  $Id: event.c,v 1.4 94/03/09 17:02:25 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Event Handling Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   event.c,v $
 * Revision 1.4  94/03/09  17:02:25  jjszucs
 * Timer event handling added
 *
 * Revision 1.3  94/03/08  13:53:39  jjszucs
 * Interlace bit is now set to force all displays to be interlaced,
 * eliminating black lines apparent in image displays on some
 * monitors and the slight jump as the display toggles from interlaced
 * to non-interlaced when the interface panel is hidden/revealed.
 *
 *
 * Revision 1.2  94/02/18  15:55:36  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.1  94/01/06  11:57:02  jjszucs
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

#include <dos/dos.h>

#include <libraries/lowlevel.h>
#include <libraries/photocd.h>

#include <graphics/modeid.h>

#include <devices/cd.h>
#include <devices/inputevent.h>

#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/intuition_protos.h>
#include <clib/debug_protos.h>
#include <clib/photocd_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/photocd_pragmas.h>

/* Local includes */
#include "global.h"
#include "interface.h"

/****************************************************************************
 *                                                                          *
 *  rawkeyEvent()   -   handle raw key event                                *
 *                                                                          *
 ****************************************************************************/

BOOL rawkeyEvent(struct appContext *appContext, USHORT code, UWORD qualifier)
{

    struct controlItem *pControlItem;
    USHORT *pControlKey;

    /* Easter egg code sequence;
        PORT1_BUTTON_FORWARD for constanants,
        PORT1_BUTTON_REVERSE for vowels
    */
    static USHORT eggCode[] ={
        RAWKEY_PORT1_BUTTON_FORWARD, /* C */
        RAWKEY_PORT1_BUTTON_REVERSE, /* o */
        RAWKEY_PORT1_BUTTON_FORWARD, /* m */
        RAWKEY_PORT1_BUTTON_FORWARD, /* m */
        RAWKEY_PORT1_BUTTON_REVERSE, /* o */
        RAWKEY_PORT1_BUTTON_FORWARD, /* d */
        RAWKEY_PORT1_BUTTON_REVERSE, /* o */
        RAWKEY_PORT1_BUTTON_FORWARD, /* r */
        RAWKEY_PORT1_BUTTON_REVERSE, /* e */
        NULL
    };
    static USHORT eggIndex=0;

    /* If in thumbnail state ... */
    if (appContext->ac_State==as_Thumbnail) {

        /* Check for "Easter Egg" sequence ... */
        if (code==eggCode[eggIndex]) {

            eggIndex++;
            if (eggCode[eggIndex]==NULL) {

                showCredits(appContext);
                appContext->ac_Credits=TRUE;
                eggIndex=0;

                /* Do not process this key further */
                return TRUE;

            }

        /* ... else ... */
        } else if (!(code&IECODE_UP_PREFIX)) {

            /* Clear Easter Egg sequence */
            eggIndex=0;

            /* If credits were displayed ... */
            if (appContext->ac_Credits) {

                /* Restore normal thumbnail display */
                newState(appContext, as_Thumbnail);

                /* Clear credits flag */
                appContext->ac_Credits=FALSE;

                /* Do not process this key further */
                return TRUE;

            }

        }

    } else {

        eggIndex=0;

    }

    /* Ignore repeated <Select> or <Cancel> events */
    if (
        (code==RAWKEY_PORT0_BUTTON_RED || code==RAWKEY_PORT1_BUTTON_RED ||
         code==RAWKEY_PORT0_BUTTON_BLUE || code==RAWKEY_PORT1_BUTTON_BLUE ||
         code==RAWKEY_ESC || code==RAWKEY_RETURN) &&
        (qualifier&IEQUALIFIER_REPEAT)
       ) {

        return TRUE;

    }

    /* Loop through controls */
    pControlItem=appContext->ac_ControlArray;
    while (pControlItem && pControlItem->cduic_Type!=ct_End) {

        /* If key code matches any key code of control ... */
        pControlKey=pControlItem->cduic_KeyCode;
        while (*pControlKey) {

            if (*pControlKey==code) {

                /* Activate control */
                (*(pControlItem->cduic_Callback))(appContext);

                /* Success */
                return TRUE;

            }

            pControlKey++;

        }

        pControlItem++;

    }

    /* If activable controls and interface pane is visible ... */
    if (appContext->ac_ActiveControl && appContext->ac_InterfaceVisible) {

        /* Handle user interface keys */
        switch (code) {

            /* Left */
            case RAWKEY_PORT0_JOY_LEFT:
            case RAWKEY_PORT1_JOY_LEFT:
            case RAWKEY_LEFT:
                controlPrevious(appContext);
                break;

            /* Right */
            case RAWKEY_PORT0_JOY_RIGHT:
            case RAWKEY_PORT1_JOY_RIGHT:
            case RAWKEY_RIGHT:
                controlNext(appContext);
                break;

            /* Select */
            case RAWKEY_PORT0_BUTTON_RED:
            case RAWKEY_PORT1_BUTTON_RED:
            case RAWKEY_RETURN:
                controlActivate(appContext);
                break;

        }

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  diskInsertEvent()   -   handle disk insertion event                     *
 *                                                                          *
 ****************************************************************************/

BOOL diskInsertEvent(struct appContext *appContext)
{

    struct CDInfo       cdInfo;

    /* Switch on state */
    switch (appContext->ac_State) {

        /* In No Disc state ... */
        case as_NoDisc:

            /* Get cd.device status */
            appContext->ac_CDRequest->io_Command=CD_INFO;
            appContext->ac_CDRequest->io_Data=&cdInfo;
            appContext->ac_CDRequest->io_Length=sizeof(cdInfo);
            if (DoIO(appContext->ac_CDRequest)!=0) {

#ifdef DEBUG
                printf("diskInsertedEvent(): Error in CD_INFO\n");
#endif /* DEBUG */

                return FALSE;
            }

            /* If disc is in drive ... */
            if (cdInfo.Status&CDSTSF_DISK) {

                /* If disc is Photo CD ... */
                if (IsPhotoCD()) {

                    /* Enter Thumbnail state */
                    newState(appContext,as_Thumbnail);

                /* ... else (not Photo CD) ... */
                } else {

                    /* Enter Invalid Disc state */
                    newState(appContext,as_InvalidDisc);

                }

            }
            break;

        /* Default */
        default:
            /* No action */
            break;

    }


    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  diskRemoveEvent()   -   handle disk removal event                       *
 *                                                                          *
 ****************************************************************************/

BOOL diskRemoveEvent(struct appContext *appContext)
{

    struct CDInfo       cdInfo;

    /* Get cd.device status */
    appContext->ac_CDRequest->io_Command=CD_INFO;
    appContext->ac_CDRequest->io_Data=&cdInfo;
    appContext->ac_CDRequest->io_Length=sizeof(cdInfo);
    if (DoIO(appContext->ac_CDRequest)!=0) {

#ifdef DEBUG
        printf("diskRemoveEvent(): Error in CD_INFO\n");
#endif /* DEBUG */

        return FALSE;

    }

    /* If disc is not in drive ... */
    if (!(cdInfo.Status&CDSTSF_DISK)) {

        /* Enter No disc state */
        newState(appContext,as_NoDisc);

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  timerEvent()   -   handle timer event                                   *
 *                                                                          *
 ****************************************************************************/

BOOL timerEvent(struct appContext *appContext)
{

    /* Complete I/O request */
    WaitIO((struct IORequest *) appContext->ac_TimerRequest);

    /* Clear timer active flag */
    appContext->ac_TimerActive=FALSE;

    /* If in image state and running slideshow ... */
    if (appContext->ac_State==as_Image &&
        appContext->ac_SlideshowPlay) {

        /* Handle slideshow timer event */
        slideshowTimer(appContext);

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  activeWindowEvent()   -   window activation event                       *
 *                                                                          *
 ****************************************************************************/

BOOL activeWindowEvent(struct appContext *appContext)
{

#ifdef DEBUG
    kprintf("activeWindowEvent(appContext=$%08lx)\n", appContext);
#endif /* DEBUG */

    /* Move display screen to front */
    if (appContext->ac_DisplayScreen) {

        ScreenToFront(appContext->ac_DisplayScreen);

    }

    /* If interface panel is visible ... */
    if (appContext->ac_InterfaceVisible &&
        appContext->ac_InterfaceScreen) {

        ScreenToFront(appContext->ac_InterfaceScreen);

    }

    /* Success */
    return TRUE;

}
