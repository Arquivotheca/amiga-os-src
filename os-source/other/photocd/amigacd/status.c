/*** status.c ****************************************************************
 *
 *  $Id: status.c,v 1.2 94/02/18 15:58:19 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Status Panel Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	status.c,v $
 * Revision 1.2  94/02/18  15:58:19  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 * 
 * Revision 1.1  94/01/06  11:58:30  jjszucs
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

#include <graphics/gfx.h>

#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* Local includes */
#include "global.h"
#include "interface.h"
#include "glyphs.h"
#include "image.h"

/****************************************************************************
 *                                                                          *
 *  newStatus()   -   transition to new status display                      *
 *                                                                          *
 ****************************************************************************/

BOOL newStatus(struct appContext *appContext,enum appState newState)
{

    UWORD glyphID;
    struct RastPort *rastport;

    /* Fetch RastPort */
    rastport=appContext->ac_InterfaceWindow->RPort;

    /* Load bitmap */
    switch (newState) {

        /* No Disc */
        case as_NoDisc:

            /* Load No Disc bitmap */
            glyphID=GLYPH_NO_DISC;
            break;

        /* Invalid Disc */
        case as_InvalidDisc:

            /* Load Invalid Disc bitmap */
            glyphID=GLYPH_INVALID_DISC;
            break;

        /* Thumbnail */
        case as_Thumbnail:
        /* Image */
        case as_Image:

            /* Load Status Panel bitmap */
            glyphID=GLYPH_STATUS_PANEL;
            break;

        /* !!! Not yet finished !!! */

        /* Unrecognized state */
        default:

#ifdef DEBUG
            printf("newStatus(): Unknown state %ld\n",newState);
#endif /* DEBUG */

            return FALSE;

            break;

    }

    if (!putGlyph(appContext,glyphID,
        rastport, STATUS_PANEL_LEFT, STATUS_PANEL_TOP)) {

#ifdef DEBUG
        printf("newStatus(): Error putting panel glyph ID %lu\n",glyphID);
#endif /* DEBUG */

        return FALSE;

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  updateStatus()   -   update status display                              *
 *                                                                          *
 ****************************************************************************/

BOOL updateStatus(struct appContext *appContext)
{

    struct imageNode *imageNode;

    /* Switch on state */
    switch (appContext->ac_State) {

        /* Thumbnail */
        case as_Thumbnail:
        /* Image */
        case as_Image:

            /* Get selected image */
            imageNode=appContext->ac_Selection;

            /* If selection is not image ... */
            if (!imageNode) {

                /* Clear slide LED */
                clearLED(appContext, appContext->ac_InterfaceWindow->RPort,
                    SLIDE_LED_X,SLIDE_LED_Y,3,TRUE);
                displayLEDSymbol(appContext, appContext->ac_InterfaceWindow->RPort,
                    '/', SLIDE_SEP_X, SLIDE_SEP_Y, FALSE);
                clearLED(appContext, appContext->ac_InterfaceWindow->RPort,
                    SLIDE_OF_LED_X,SLIDE_OF_LED_Y,3,TRUE);

                /* Clear image LED */
                clearLED(appContext, appContext->ac_InterfaceWindow->RPort,
                    IMAGE_LED_X,IMAGE_LED_Y,3,TRUE);
                displayLEDSymbol(appContext, appContext->ac_InterfaceWindow->RPort,
                    '/', IMAGE_SEP_X, IMAGE_SEP_Y, FALSE);
                clearLED(appContext, appContext->ac_InterfaceWindow->RPort,
                    IMAGE_OF_LED_X,IMAGE_OF_LED_Y,3,TRUE);

            /* ... else ... */
            } else {

                /* Display slide number */
                displayLEDNumber(appContext, appContext->ac_InterfaceWindow->RPort,
                    imageNode->in_IsSlide ? imageNode->in_Slide : Largest(UWORD),
                    SLIDE_LED_X, SLIDE_LED_Y, 3, TRUE);
                displayLEDSymbol(appContext, appContext->ac_InterfaceWindow->RPort,
                    '/', SLIDE_SEP_X, SLIDE_SEP_Y, TRUE);
                displayLEDNumber(appContext, appContext->ac_InterfaceWindow->RPort,
                    appContext->ac_NSlides,
                    SLIDE_OF_LED_X, SLIDE_OF_LED_Y, 3, TRUE);

                /* Display image number */
                displayLEDNumber(appContext, appContext->ac_InterfaceWindow->RPort,
                    imageNode->in_Number,
                    IMAGE_LED_X,IMAGE_LED_Y,3,TRUE);
                displayLEDSymbol(appContext, appContext->ac_InterfaceWindow->RPort,
                    '/', IMAGE_SEP_X, IMAGE_SEP_Y, TRUE);
                displayLEDNumber(appContext, appContext->ac_InterfaceWindow->RPort,
                    appContext->ac_NImages,
                    IMAGE_OF_LED_X,IMAGE_OF_LED_Y,3,TRUE);

            }

            break;

        /* !!! Other states */

    }

    /* Success */
    return TRUE;

}
