/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>

#include <devices/timer.h>

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/timer_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/alib_protos.h>
#include <clib/debug_protos.h>

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

#include "timerint.h"

/****** TimerInt/guiOpen ******************************************
*
*   NAME
*       guiOpen -- open graphical user interface
*
*   SYNOPSIS
*       success=guiOpen();
*
*       BOOL guiOpen(void);
*
*   FUNCTION
*       Open graphical user interface.
*
*   INPUTS
*       None
*
*   RESULT
*       success -- TRUE if success; FALSE if failure
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       guiClose()
*
******************************************************************************
*
*/
BOOL guiOpen(void)
{

    struct NewGadget newGadget;
    struct Gadget *previousGadget;

    static struct TextAttr textAttr ={
        GUI_TYPEFACE, /* ta_Name */
        GUI_TYPESIZE, /* ta_YSize */
        NULL, /* ta_Style */
        NULL  /* ta_Flags */
    };

    static STRPTR modeLabels[] ={
        "One-shot",
        "Continuous",
        NULL
    };

    /*
     * Open window
     */

    /* Open window */
    window=OpenWindowTags(NULL,
        WA_Title, PROGRAM_NAME,
        WA_InnerWidth, 224,
        WA_InnerHeight, 114,
        WA_DragBar, TRUE,
        WA_CloseGadget, TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate, TRUE,
        WA_SimpleRefresh, TRUE,
        WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|
            INTEGERIDCMP|NUMBERIDCMP|CYCLEIDCMP|BUTTONIDCMP,
        TAG_DONE);
    if (!window) {
        return(FALSE);
    }

    /* Fetch visual info */
    visualInfo=GetVisualInfo(window->WScreen,TAG_DONE);
    if (!visualInfo) {
        return(FALSE);
    }

    /* Create gadtools.library context */
    previousGadget=gadgetList=CreateContext(&gadgetList);
    if (!gadgetList) {
        return(FALSE);
    }

    /*
     * Create gadgetry
     */

    /* Interval gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+128;
    newGadget.ng_TopEdge=window->BorderTop+12;
    newGadget.ng_Width=80;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Interval (ms)";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_INTERVAL_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=intervalGadget=CreateGadget(INTEGER_KIND,
        previousGadget,&newGadget,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Deviation gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+128;
    newGadget.ng_TopEdge=window->BorderTop+36;
    newGadget.ng_Width=80;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Deviation (ms)";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_DEVIATION_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=deviationGadget=CreateGadget(NUMBER_KIND,
        previousGadget,&newGadget,
        GTNM_Border, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Mode gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+80;
    newGadget.ng_TopEdge=window->BorderTop+60;
    newGadget.ng_Width=128;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Mode";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_MODE_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=modeGadget=CreateGadget(CYCLE_KIND,
        previousGadget,&newGadget,
        GTCY_Labels, &modeLabels,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Start gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+92;
    newGadget.ng_Width=64;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Start";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_START_ID;
    newGadget.ng_Flags=PLACETEXT_IN;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=startGadget=CreateGadget(BUTTON_KIND,
        previousGadget,&newGadget,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Stop gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+144;
    newGadget.ng_TopEdge=window->BorderTop+92;
    newGadget.ng_Width=64;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Stop";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_STOP_ID;
    newGadget.ng_Flags=PLACETEXT_IN;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=stopGadget=CreateGadget(BUTTON_KIND,
        previousGadget,&newGadget,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /*
     * Attach gadgetry
     */

    /* Add gadget list to window */
    AddGList(window,gadgetList,-1,-1,NULL);

    /* Refresh gadgetry */
    RefreshGList(gadgetList,window,NULL,-1);
    GT_RefreshWindow(window,NULL);

    /*
     * Success
     */

    /* Success */
    return(TRUE);

}

/****** TimerInt/guiClose **************************************************
*
*   NAME
*       guiClose -- close graphical user interface
*
*   SYNOPSIS
*       guiClose();
*
*       void guiClose(void);
*
*   FUNCTION
*       Close graphical user interface.
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
*       guiOpen()
*
******************************************************************************
*
*/
void guiClose(void)
{

    /* If intuition.library and gadtools.library open ... */
    if (IntuitionBase && GadtoolsBase) {

        /* If window open ... */
        if (window) {

            /* If visual info ... */
            if (visualInfo) {

                /* If gadgets created ... */
                if (gadgetList) {

                    /* Remove gadgetry from window */
                    RemoveGList(window,gadgetList,-1);

                    /* Free gadgetry */
                    FreeGadgets(gadgetList);

                }

                /* Free visual info */
                FreeVisualInfo(visualInfo);

            }

            /* Close window */
            CloseWindow(window);

        }

    }

}

/****** TimerInt/guiLoop ***************************************************
*
*   NAME
*       guiLoop -- event loop for graphical user interface
*
*   SYNOPSIS
*       guiLoop();
*
*       void guiLoop(void);
*
*   FUNCTION
*       Event loop for graphical user interface.
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
void guiLoop(void)
{

    ULONG signalFlags;

    struct IntuiMessage *imsg;
    ULONG class;
    USHORT code;
    APTR iaddress;

    /* Endless loop */
    FOREVER {

        /* Wait for ... */
        signalFlags=
            Wait((1<<window->UserPort->mp_SigBit) /* window message ... */
                 | (1<<timerSignal) /* or timer signal */
            );

        /* If window message ... */
        if (signalFlags&(1<<window->UserPort->mp_SigBit)) {

            /* Process window message queue */
            while (imsg=GT_GetIMsg(window->UserPort)) {

                /* Fetch fields from message */
                class=imsg->Class;
                code=imsg->Code;
                iaddress=imsg->IAddress;

                /* Reply to message */
                GT_ReplyIMsg(imsg);

                /* Switch on class */
                switch (class) {

                    /* Window close event */
                    case IDCMP_CLOSEWINDOW:
                        /* Exit event loop */
                        return;

                    /* Window refresh event */
                    case IDCMP_REFRESHWINDOW:
                        /* Refresh window */
                        GT_BeginRefresh(window);
                        GT_EndRefresh(window,TRUE);
                        break;

                    /* Window gadget event */
                    case IDCMP_GADGETUP:
                        {
                            struct Gadget *gadget;

                            gadget=(struct Gadget *) iaddress;
                            switch (gadget->GadgetID) {

                                /* Interval */
                                case GUI_INTERVAL_ID:
                                    /* Fetch interval */
                                    GT_GetGadgetAttrs(intervalGadget,window,NULL,
                                        GTIN_Number, &timerInterval,
                                        TAG_DONE);
                                    break;

                                /* Mode */
                                case GUI_MODE_ID:
                                    /* Mode selection is code */
                                    timerContinuous=
                                        (code==GUI_MODE_CONTINUOUS_CODE)?
                                            TRUE:FALSE;
                                    break;

                                /* Start */
                                case GUI_START_ID:
                                    timerStart();
                                    break;

                                /* Stop */
                                case GUI_STOP_ID:
                                    timerStop();
                                    break;

                            }

                        }
                        break;

                    /* Default */
                    default:
                        /* No action */
                        break;

                }

            }

        }

        /* If timer signal ... */
        if (signalFlags&(1<<timerSignal)) {

            /* Update deviation */
            GT_SetGadgetAttrs(deviationGadget,window,NULL,
                GTNM_Number, deviation,
                TAG_DONE);

        }

    }

}
