/*** control.c ***************************************************************
 *
 *  $Id: control.c,v 1.4 94/03/16 18:17:35 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Control Bar Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	control.c,v $
 * Revision 1.4  94/03/16  18:17:35  jjszucs
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
 * Revision 1.3  94/03/01  16:09:08  jjszucs
 * Mask now used for control panel buttons.
 *
 * Revision 1.2  94/02/18  15:54:56  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.1  94/01/06  11:56:35  jjszucs
 * Initial revision
 *
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* Amiga includes */
#include <exec/types.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>

#include <libraries/lowlevel.h>

#include <devices/input.h>
#include <devices/inputevent.h>

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

/****************************************************************************
 *                                                                          *
 *  Local prototypes                                                        *
 *                                                                          *
 ****************************************************************************/

static BOOL controlCursor(struct appContext *appContext,
    struct controlItem *pControlItem,
    BOOL show);
static BOOL selectControl(struct appContext *appContext,
    struct controlItem *pControlItem);
static struct controlItem *findControl(struct appContext *appContext,
    UWORD controlID);
BOOL refreshControl(struct appContext *appContext,
    struct controlItem *pControlItem);

/****************************************************************************
 *                                                                          *
 *  newControl()    -   transition to new control panel                     *
 *                                                                          *
 ****************************************************************************/

BOOL newControl(struct appContext *appContext,enum appState newState)
{

    /* Force interface panel visible for No Disc, Invalid Disc, and
       Thumbnail states */
    if (newState==as_NoDisc ||
        newState==as_InvalidDisc ||
        newState==as_Thumbnail) {

        showInterface(appContext);

    }

    /* Set-up controls */
    switch (newState) {

        case as_Thumbnail:
            thumbnailControls(appContext);
            break;

        case as_Image:
            imageControls(appContext);
            break;

        default:
            clearControls(appContext);
            break;

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  setControls()    -   set controls available in interface panel          *
 *                                                                          *
 *  INPUTS                                                                  *
 *      appContext      -   application context                             *
 *      pControlArray   -   control array                                   *
 *      pActivate       -   initially active control; NULL if no            *
 *                              controls are activatable                    *
 *                                                                          *
 ****************************************************************************/

BOOL setControls(struct appContext *appContext,
    struct controlItem *pControlArray, struct controlItem *pActivate)
{

    struct controlItem *pControlItem;
    BOOL status;

    /*
     *  Initialization
     */

    /* Assume success */
    status=TRUE;

    /*
     *  Clear old controls
     */

    clearControls(appContext);

    /*
     * Set new controls
     */

    /* Loop through controls */
    pControlItem=pControlArray;
    while (pControlItem->cduic_Type!=ct_End) {

        /* Enable control */
        pControlItem->cduic_Flags|=CDUICF_ENABLED;

        /* Perform initial refresh of control */
        if (!refreshControl(appContext, pControlItem)) {

            status=FALSE;

        }

        pControlItem++;

    }

    /*
     *  Set control array pointer
     */

    appContext->ac_ControlArray=pControlArray;

    /*
     *  Activate control
     */

    appContext->ac_ActiveControl=NULL;
    selectControl(appContext, pActivate);

    /*
     * Return
     */

    return status;

}

/****************************************************************************
 *                                                                          *
 *  clearControls()    -   clear controls in interface panel                *
 *                                                                          *
 ****************************************************************************/

void clearControls(struct appContext *appContext)
{

    struct controlItem *pControlItem;

    /* If existing control array ... */
    if (pControlItem=appContext->ac_ControlArray) {

        /* Erase cursor */
        controlCursor(appContext, appContext->ac_ActiveControl, FALSE);

        /* Loop through controls */
        while (pControlItem->cduic_Type!=ct_End) {

            /* Erase control */
            if (pControlItem->cduic_Glyph!=GLYPH_NONE) {

                EraseRect(appContext->ac_InterfaceWindow->RPort,
                    pControlItem->cduic_X, pControlItem->cduic_Y,
                    pControlItem->cduic_X+pControlItem->cduic_Width-1,
                    pControlItem->cduic_Y+pControlItem->cduic_Height-1);

            }

            pControlItem++;

        }

        /* Clear control array */
        appContext->ac_ControlArray=NULL;

        /* Clear active control */
        appContext->ac_ActiveControl=NULL;

    }

}

/****************************************************************************
 *                                                                          *
 *  refreshControl()  - refresh single controls                             *
 *                                                                          *
 ****************************************************************************/

BOOL refreshControl(struct appContext *appContext, struct controlItem
    *pControlItem)
{

    struct RastPort *rastport;

    static USHORT disabledMask[] ={
        0xCCCC,
        0x3333
    };

    /* Fetch RastPort */
    rastport=appContext->ac_InterfaceWindow->RPort;

    /* Refresh control */
    if (pControlItem->cduic_Glyph!=GLYPH_NONE) {

        if (!putGlyphMask(appContext, pControlItem->cduic_Glyph,
                pControlItem->cduic_Mask,
                appContext->ac_InterfaceWindow->RPort,
                pControlItem->cduic_X, pControlItem->cduic_Y)) {

            return FALSE;

        }

        /* If control is disabled ... */
        if (!(pControlItem->cduic_Flags&CDUICF_ENABLED)) {

            /* Cover with 50% background mask */
            SetAfPt(rastport,
                disabledMask,
                1);
            SetAPen(rastport, INTERFACE_PEN_BACKGROUND);
            SetOPen(rastport, INTERFACE_PEN_BACKGROUND);
            SetDrMd(rastport, JAM1);
            RectFill(rastport,
                pControlItem->cduic_X, pControlItem->cduic_Y,
                pControlItem->cduic_X+pControlItem->cduic_Width-1,
                pControlItem->cduic_Y+pControlItem->cduic_Height-1);
            SetAfPt(rastport, NULL, 0);
            SetDrMd(rastport, JAM2);

        }

    }

    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  refreshControls()  - refresh all controls                               *
 *                                                                          *
 ****************************************************************************/

BOOL refreshControls(struct appContext *appContext)
{

    struct controlItem *pControlItem;
    BOOL status;

    /* Assume success */
    status=TRUE;

    /* Loop through controls */
    pControlItem=appContext->ac_ControlArray;
    while (pControlItem->cduic_Type!=ct_End) {

        /* Refresh controls */
        if (!refreshControl(appContext, pControlItem)) {

            status=FALSE;

        }

        pControlItem++;

    }

    /* Refresh cursor */
    if (appContext->ac_ActiveControl) {

        controlCursor(appContext, appContext->ac_ActiveControl, TRUE);

    }

    return status;

}

/****************************************************************************
 *                                                                          *
 *  controlNext()    -   select next control                                *
 *                                                                          *
 ****************************************************************************/

BOOL controlNext(struct appContext *appContext)
{

    struct controlItem *newControl;

    /* Advance to next control; wrapping around to first and skipping
        hidden controls */
    newControl=appContext->ac_ActiveControl;
    do {

        if (newControl->cduic_Type==ct_End) {

            newControl=appContext->ac_ControlArray;

        } else {

            newControl++;

        }

    } while (newControl->cduic_Type==ct_End ||
             newControl->cduic_Glyph==GLYPH_NONE ||
             !(newControl->cduic_Flags&CDUICF_ENABLED));

    /* Select new control */
    selectControl(appContext, newControl);

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  controlPrevious()    -   select previous control                        *
 *                                                                          *
 ****************************************************************************/

BOOL controlPrevious(struct appContext *appContext)
{

    struct controlItem *newControl;

    /* Go to previous control; wrapping around to last and skipping
        hidden controls */
    newControl=appContext->ac_ActiveControl;
    do {

        if (newControl==appContext->ac_ControlArray) {

            while (newControl->cduic_Type!=ct_End) {

                newControl++;

            }

        }

        newControl--;

    } while (newControl->cduic_Glyph==GLYPH_NONE ||
             !(newControl->cduic_Flags&CDUICF_ENABLED));

    /* Select new control */
    selectControl(appContext, newControl);

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  controlActivate()    -   activate control                               *
 *                                                                          *
 ****************************************************************************/

BOOL controlActivate(struct appContext *appContext)
{

    /* Call client function */
    (*(appContext->ac_ActiveControl->cduic_Callback))(appContext);

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  controlCursor()    -   render control cursor                            *
 *                                                                          *
 ****************************************************************************/

static BOOL controlCursor(struct appContext *appContext,
    struct controlItem *pControl,
    BOOL show)
{

    UWORD minX, minY;
    UWORD maxX, maxY;

    struct RastPort *rastport;

    /* If no cursor ... */
    if (pControl==NULL) {

        /* No action */
        return TRUE;

    }

    /* Compute corners */
    minX=pControl->cduic_X-CURSOR_THICKNESS;
    minY=pControl->cduic_Y-CURSOR_THICKNESS;
    maxX=minX+pControl->cduic_Width-1+CURSOR_THICKNESS;
    maxY=minY+pControl->cduic_Height-1+CURSOR_THICKNESS;

    /* Fetch interface rastport */
    rastport=appContext->ac_InterfaceWindow->RPort;

    /* Use two-pen complement drawing mode */
    SetDrMd(rastport,JAM2);

    if (show) {

        /* Set pens */
        SetAPen(rastport, INTERFACE_PEN_CURSOR);
        SetOPen(rastport, INTERFACE_PEN_CURSOR);

        /* Draw upper edge */
        RectFill(rastport,
            minX, minY,
            maxX+CURSOR_THICKNESS-1, minY+CURSOR_THICKNESS-1);

        /* Draw left edge */
        RectFill(rastport,
            minX,minY,
            minX+CURSOR_THICKNESS-1,maxY+CURSOR_THICKNESS-1);

        /* Draw bottom edge */
        RectFill(rastport,
            minX,maxY,
            maxX+CURSOR_THICKNESS-1,maxY+CURSOR_THICKNESS-1);

        /* Draw right edge */
        RectFill(rastport,
            maxX,minY,
            maxX+CURSOR_THICKNESS-1,maxY+CURSOR_THICKNESS-1);

    } else {

        /* Erase upper edge */
        EraseRect(rastport,
            minX, minY,
            maxX+CURSOR_THICKNESS-1, minY+CURSOR_THICKNESS-1);

        /* Erase left edge */
        EraseRect(rastport,
            minX, minY,
            minX+CURSOR_THICKNESS-1, maxY+CURSOR_THICKNESS-1);

        /* Erase bottom edge */
        EraseRect(rastport,
            minX, maxY,
            maxX+CURSOR_THICKNESS-1, maxY+CURSOR_THICKNESS-1);

        /* Erase right edge */
        EraseRect(rastport,
            maxX, minY,
            maxX+CURSOR_THICKNESS-1, maxY+CURSOR_THICKNESS-1);

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  selectControl()    -   select control                                   *
 *                                                                          *
 ****************************************************************************/

static BOOL selectControl(struct appContext *appContext,
    struct controlItem *pControlItem)
{

    /* Erase old cursor */
    if (appContext->ac_ActiveControl) {

        controlCursor(appContext, appContext->ac_ActiveControl, FALSE);

    }

    /* Set active control */
    appContext->ac_ActiveControl=pControlItem;

    /* Draw new cursor */
    if (pControlItem) {

        controlCursor(appContext, appContext->ac_ActiveControl, TRUE);

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  findControl()    -   find control by identifier                         *
 *                                                                          *
 ****************************************************************************/

static struct controlItem *findControl(struct appContext *appContext, UWORD controlID)
{

    struct controlItem *pControlItem;

    pControlItem=appContext->ac_ControlArray;
    if (!pControlItem) {
        return NULL;
    }

    while (pControlItem->cduic_Type!=ct_End) {

        if (pControlItem->cduic_ID==controlID) {
            return pControlItem;
        }

        pControlItem++;

    }

    return NULL;

}

/****************************************************************************
 *                                                                          *
 *  controlChangeGlyph()    -   change glyph of control                     *
 *                                                                          *
 ****************************************************************************/

BOOL controlChangeGlyph(struct appContext *appContext, UWORD controlID,
    UWORD glyphID)
{

    struct controlItem *pControlItem;

    /* Find control */
    pControlItem=findControl(appContext, controlID);
    if (!pControlItem) {
        return FALSE;
    }

    /* Set new glyph */
    pControlItem->cduic_Glyph=glyphID;

    /* Refresh control */
    refreshControl(appContext, pControlItem);

    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  controlSelect()    -   select control                                   *
 *                                                                          *
 ****************************************************************************/

BOOL controlSelect(struct appContext *appContext, UWORD controlID)
{

    struct controlItem *pControlItem;

    /* Find control */
    pControlItem=findControl(appContext, controlID);
    if (!pControlItem) {
        return FALSE;
    }

    /* Select control */
    selectControl(appContext, pControlItem);

    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  controlEnable()    -   enable/disable control                           *
 *                                                                          *
 ****************************************************************************/

BOOL controlEnable(struct appContext *appContext, UWORD controlID, BOOL enable)
{

    struct controlItem *pControlItem;

    /* Find control item */
    pControlItem=findControl(appContext, controlID);
    if (!pControlItem) {

        return FALSE;

    }

    /* Enable/disable control */
    if (enable) {

        pControlItem->cduic_Flags|=CDUICF_ENABLED;

    } else {

        pControlItem->cduic_Flags&=~CDUICF_ENABLED;

    }

    /* Refresh control */
    refreshControl(appContext, pControlItem);

    /* If active control is disabled ... */
    if (pControlItem==appContext->ac_ActiveControl && (!enable)) {

        /* Go to next control */
        controlNext(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *  moveMouse()    -   move mouse pointer under software control            *
 *                                                                          *
 ****************************************************************************/

BOOL moveMouse(struct appContext *appContext, struct Screen *screen, WORD x, WORD y)
{

    struct InputEvent ie;

    appContext->ac_InputRequest->io_Command=IND_WRITEEVENT;
    appContext->ac_InputRequest->io_Flags=NULL;
    appContext->ac_InputRequest->io_Length=sizeof(ie);
    appContext->ac_InputRequest->io_Data=&ie;

    ie.ie_NextEvent=NULL;
    ie.ie_Class=IECLASS_POINTERPOS;
    ie.ie_SubClass=IESUBCLASS_COMPATIBLE;
    ie.ie_Code=NULL;
    ie.ie_Qualifier=NULL;
    ie.ie_X=x;
    ie.ie_Y=y;

    if (DoIO(appContext->ac_InputRequest)) {
        return FALSE;
    }

    return TRUE;

}
