/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>

#include <libraries/lowlevel.h>

#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/lowlevel_pragmas.h>

/*
 * ANSI includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes
 */

#include "readjoyport.h"

/****** ReadJoyPort/pollPort ******************************************
*
*   NAME
*       pollPort -- poll game controller port
*
*   SYNOPSIS
*       pollPort();
*
*       void pollPort(void);
*
*   FUNCTION
*       Poll game controller port. If any state change, update display.
*
*   INPUTS
*       None
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/
void pollPort(void)
{

    ULONG newPortState;

    char *type;

    /* Read port state */
    newPortState=ReadJoyPort(portNumber);

    /* If port state unchanged ... */
    if (portState==newPortState) {
        /* No action */
        return;
    }

    /* Save port state */
    portState=newPortState;

    /* Update type */
    switch (portState&JP_TYPE_MASK) {
        case JP_TYPE_NOTAVAIL:
            type="Not available";
            break;
        case JP_TYPE_GAMECTLR:
            type="Game controller";
            break;
        case JP_TYPE_MOUSE:
            type="Mouse";
            break;
        case JP_TYPE_JOYSTK:
            type="Joystick";
            break;
        case JP_TYPE_UNKNOWN:
            type="Unknown";
            break;
        default:
            type="Unexpected";
            break;
    }
    GT_SetGadgetAttrs(typeGadget,window,NULL,
        GTTX_Text, type,
        TAG_DONE);

    /* Switch on type */
    switch (portState&JP_TYPE_MASK) {

        /* Game controller */
        case JP_TYPE_GAMECTLR:

            /* Update button 1 */
            GT_SetGadgetAttrs(btn1Gadget,window,NULL,
                sticky?(portState&JPF_BTN1?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN1,
                TAG_DONE);

            /* Update button 2 */
            GT_SetGadgetAttrs(btn2Gadget,window,NULL,
                sticky?(portState&JPF_BTN2?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN2,
                TAG_DONE);

            /* Update button 3 */
            GT_SetGadgetAttrs(btn3Gadget,window,NULL,
                sticky?(portState&JPF_BTN3?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN3,
                TAG_DONE);

            /* Update button 4 */
            GT_SetGadgetAttrs(btn4Gadget,window,NULL,
                sticky?(portState&JPF_BTN4?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN4,
                TAG_DONE);

            /* Update button 5 */
            GT_SetGadgetAttrs(btn5Gadget,window,NULL,
                sticky?(portState&JPF_BTN5?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN5,
                TAG_DONE);

            /* Update button 6 */
            GT_SetGadgetAttrs(btn6Gadget,window,NULL,
                sticky?(portState&JPF_BTN6?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN6,
                TAG_DONE);

            /* Update button 7 */
            GT_SetGadgetAttrs(btn7Gadget,window,NULL,
                sticky?(portState&JPF_BTN7?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN7,
                TAG_DONE);

            /* Update up */
            GT_SetGadgetAttrs(upGadget,window,NULL,
                sticky?(portState&JPF_UP?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_UP,
                TAG_DONE);

            /* Update down */
            GT_SetGadgetAttrs(downGadget,window,NULL,
                sticky?(portState&JPF_DOWN?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_DOWN,
                TAG_DONE);

            /* Update left */
            GT_SetGadgetAttrs(leftGadget,window,NULL,
                sticky?(portState&JPF_LEFT?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_LEFT,
                TAG_DONE);

            /* Update right */
            GT_SetGadgetAttrs(rightGadget,window,NULL,
                sticky?(portState&JPF_RIGHT?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_RIGHT,
                TAG_DONE);

            /* If game controller error ... */
            if (!gameCtlrCheck(portState)) {
                /* Beep display */
                DisplayBeep(NULL);
            }

            break;

        /* Mouse */
        case JP_TYPE_MOUSE:

            /* Update button 1 */
            GT_SetGadgetAttrs(btn1Gadget,window,NULL,
                sticky?(portState&JPF_BTN1?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN1,
                TAG_DONE);

            /* Update button 2 */
            GT_SetGadgetAttrs(btn2Gadget,window,NULL,
                sticky?(portState&JPF_BTN2?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN2,
                TAG_DONE);

            /* Update button 7 */
            GT_SetGadgetAttrs(btn7Gadget,window,NULL,
                sticky?(portState&JPF_BTN7?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN7,
                TAG_DONE);

            /* Update vertical */
            GT_SetGadgetAttrs(verticalGadget,window,NULL,
                GTNM_Number, portState&JP_MVERT_MASK,
                TAG_DONE);

            /* Update horizontal */
            GT_SetGadgetAttrs(horizontalGadget,window,NULL,
                GTNM_Number, portState&JP_MHORZ_MASK,
                TAG_DONE);

            break;

        /* Joystick */
        case JP_TYPE_JOYSTK:

            /* Update button 1 */
            GT_SetGadgetAttrs(btn1Gadget,window,NULL,
                sticky?(portState&JPF_BTN1?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN1,
                TAG_DONE);

            /* Update button 2 */
            GT_SetGadgetAttrs(btn2Gadget,window,NULL,
                sticky?(portState&JPF_BTN2?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_BTN2,
                TAG_DONE);

            /* Update up */
            GT_SetGadgetAttrs(upGadget,window,NULL,
                sticky?(portState&JPF_UP?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_UP,
                TAG_DONE);

            /* Update down */
            GT_SetGadgetAttrs(downGadget,window,NULL,
                sticky?(portState&JPF_DOWN?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_DOWN,
                TAG_DONE);

            /* Update left */
            GT_SetGadgetAttrs(leftGadget,window,NULL,
                sticky?(portState&JPF_LEFT?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_LEFT,
                TAG_DONE);

            /* Update right */
            GT_SetGadgetAttrs(rightGadget,window,NULL,
                sticky?(portState&JPF_RIGHT?GTCB_Checked:TAG_IGNORE):GTCB_Checked,
                    portState&JPF_RIGHT,
                TAG_DONE);

            /* If joystick error ... */
            if (!joystickCheck(portState)) {
                /* Beep display */
                DisplayBeep(NULL);
            }

            break;

        /* Default */
        default:
            /* No action */
            break;

    }

}

/****** ReadJoyPort/gameCtlrCheck ******************************************
*
*   NAME
*       gameCtlrCheck -- check for error condition for game controller
*
*   SYNOPSIS
*       success=gameCtlrCheck(gcValue);
*
*       BOOL gameCtlrCheck(ULONG gcValue);
*
*   FUNCTION
*       Check for error condition for game controller.
*
*       Tests for following error conditions:
*       o   Mutually-exclusive directions
*           (i.e., left and right, up and down)
*
*   INPUTS
*       gcValue -- game controller port value
*
*   RESULT
*       success -- TRUE if good; FALSE if error
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

BOOL gameCtlrCheck(ULONG gcValue)
{

    /*
     * Check for mutually-exclusive directions
     */

    /* Up and down */
    if (gcValue&JPF_UP && gcValue&JPF_DOWN) {
        return(FALSE);
    }

    /* Left and right */
    if (gcValue&JPF_LEFT && gcValue&JPF_RIGHT) {
        return(FALSE);
    }

    /*
     * Pass
     */
    return(TRUE);

}

/****** ReadJoyPort/joystickCheck ******************************************
*
*   NAME
*       joystickCheck -- check for error condition for joystick
*
*   SYNOPSIS
*       success=joystickCheck(jsValue);
*
*       BOOL joystickCheck(ULONG jsValue);
*
*   FUNCTION
*       Check for error condition for joystick.
*
*       Tests for following error conditions:
*       o   Mutually-exclusive directions
*           (i.e., left and right, up and down)
*
*   INPUTS
*       gcValue -- game controller port value
*
*   RESULT
*       success -- TRUE if good; FALSE if error
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

BOOL joystickCheck(ULONG jsValue)
{

    /*
     * Check for mutually-exclusive directions
     */

    /* Up and down */
    if (jsValue&JPF_UP && jsValue&JPF_DOWN) {
        return(FALSE);
    }

    /* Left and right */
    if (jsValue&JPF_LEFT && jsValue&JPF_RIGHT) {
        return(FALSE);
    }

    /*
     * Pass
     */
    return(TRUE);

}
