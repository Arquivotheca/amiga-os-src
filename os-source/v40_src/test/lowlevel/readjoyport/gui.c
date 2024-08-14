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

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
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

/****** ReadJoyPort/guiOpen ******************************************
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

    static UBYTE *modeState[] ={
        "Game Controller",
        "Mouse",
        "Joystick",
        "Autosense",
        NULL
    };

    /*
     * Open window
     */

    /* Open window */
    window=OpenWindowTags(NULL,
        WA_Title, PROGRAM_NAME,
        WA_InnerWidth, 386,
        WA_InnerHeight, 168,
        WA_DragBar, TRUE,
        WA_CloseGadget, TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate, TRUE,
        WA_SimpleRefresh, TRUE,
        WA_RMBTrap, TRUE,
        WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|
            INTEGERIDCMP|NUMBERIDCMP|BUTTONIDCMP|TEXTIDCMP|CHECKBOXIDCMP|CYCLEIDCMP,
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

    /* Port gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+48;
    newGadget.ng_TopEdge=window->BorderTop+8;
    newGadget.ng_Width=32;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Port";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_PORT_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=portGadget=CreateGadget(INTEGER_KIND,
        previousGadget,&newGadget,
        GTIN_Number, portNumber,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Type gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+128;
    newGadget.ng_TopEdge=window->BorderTop+8;
    newGadget.ng_Width=248;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Type";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_TYPE_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=typeGadget=CreateGadget(TEXT_KIND,
        previousGadget,&newGadget,
        GTTX_Border, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Sticky gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+44;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Sticky";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_STICKY_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=stickyGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GTCB_Checked, sticky,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Reset gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+96;
    newGadget.ng_TopEdge=window->BorderTop+42;
    newGadget.ng_Width=80;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Reset";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_RESET_ID;
    newGadget.ng_Flags=PLACETEXT_IN;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=resetGadget=CreateGadget(BUTTON_KIND,
        previousGadget, &newGadget,
        TAG_DONE);

    /* Mode gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+224;
    newGadget.ng_TopEdge=window->BorderTop+42;
    newGadget.ng_Width=152;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Mode";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_MODE_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    previousGadget=modeGadget=CreateGadget(CYCLE_KIND,
        previousGadget, &newGadget,
        GTCY_Labels, modeState,
        GTCY_Active, mode,
        TAG_DONE);

    /* Btn 1 gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+304;
    newGadget.ng_TopEdge=window->BorderTop+112;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Btn 1";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_BTN1_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=btn1Gadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }


    /* Btn 2 gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+224;
    newGadget.ng_TopEdge=window->BorderTop+112;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Btn 2";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_BTN2_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=btn2Gadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Btn 3 gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+304;
    newGadget.ng_TopEdge=window->BorderTop+88;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Btn 3";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_BTN3_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=btn3Gadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Btn 4 gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+224;
    newGadget.ng_TopEdge=window->BorderTop+88;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Btn 4";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_BTN4_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=btn4Gadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Btn 5 gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+264;
    newGadget.ng_TopEdge=window->BorderTop+64;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Btn 5";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_BTN5_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=btn5Gadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Btn 6 gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+48;
    newGadget.ng_TopEdge=window->BorderTop+64;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Btn 6";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_BTN6_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=btn6Gadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Btn 7 gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+144;
    newGadget.ng_TopEdge=window->BorderTop+120;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Btn 7";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_BTN6_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=btn7Gadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Up gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+48;
    newGadget.ng_TopEdge=window->BorderTop+88;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Up";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_UP_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=upGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Down gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+48;
    newGadget.ng_TopEdge=window->BorderTop+120;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Down";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_DOWN_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=downGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Left gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+104;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Left";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_LEFT_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=leftGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Right gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+80;
    newGadget.ng_TopEdge=window->BorderTop+104;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Right";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_UP_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=rightGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Vertical gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+80;
    newGadget.ng_TopEdge=window->BorderTop+144;
    newGadget.ng_Width=96;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Vertical";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_VERTICAL_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=verticalGadget=CreateGadget(NUMBER_KIND,
        previousGadget,&newGadget,
        GTNM_Border, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Horizontal gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+280;
    newGadget.ng_TopEdge=window->BorderTop+144;
    newGadget.ng_Width=96;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Horizontal";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_HORIZONTAL_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=horizontalGadget=CreateGadget(NUMBER_KIND,
        previousGadget,&newGadget,
        GTNM_Border, TRUE,
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

/****** ReadJoyPort/guiClose ******************************************
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

/****** ReadJoyPort/guiLoop ******************************************
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

    struct IntuiMessage *imsg;
    ULONG class;
    USHORT code;
    APTR iaddress;

    if (debugMode) {
        Printf("ReadJoyPort/guiLoop(): Entry\n");
    }

    /* Endless loop */
    FOREVER {

        /* Wait for top of frame */
        WaitTOF();

        /* Poll game controller port */
        pollPort();

        /* Process window message queue (if any) */
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
                    if (debugMode) {
                        Printf("ReadJoyPort/guiLoop(): IDCMP_CLOSEWINDOW\n");
                        Printf("ReadJoyPort/guiLoop(): Exiting\n");
                    }
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

                        /* Fetch gadget address */
                        gadget=iaddress;

                        /* Switch on gadget ID */
                        switch (gadget->GadgetID) {

                            /* Port gadget */
                            case GUI_PORT_ID:

                                /* Get port number */
                                GT_GetGadgetAttrs(portGadget,window,NULL,
                                    GTIN_Number, &portNumber,
                                    TAG_DONE);
                                /* Reset port state */
                                portState=~0UL;

                                break;

                            /* Sticky gadget */
                            case GUI_STICKY_ID:

                                /* Get sticky state */
                                GT_GetGadgetAttrs(stickyGadget,window,NULL,
                                    GTCB_Checked, &sticky,
                                    TAG_DONE);
                                break;

                            /* Mode gadget */
                            case GUI_MODE_ID:

                                /* Get mode */
                                GT_GetGadgetAttrs(modeGadget,window,NULL,
                                    GTCY_Active, &mode,
                                    TAG_DONE);

                                /* Set joystick port attributes */
                                switch (mode) {

                                    case GUI_MODE_GAME_CONTROLLER:
                                        SetJoyPortAttrs(portNumber,
                                            SJA_Type, SJA_TYPE_GAMECTLR,
                                            TAG_DONE);
                                        break;

                                    case GUI_MODE_JOYSTICK:
                                        SetJoyPortAttrs(portNumber,
                                            SJA_Type, SJA_TYPE_JOYSTK,
                                            TAG_DONE);
                                        break;

                                    case GUI_MODE_MOUSE:
                                        SetJoyPortAttrs(portNumber,
                                            SJA_Type, SJA_TYPE_MOUSE,
                                            TAG_DONE);
                                        break;

                                    case GUI_MODE_AUTOSENSE:
                                        SetJoyPortAttrs(portNumber,
                                            SJA_Type, SJA_TYPE_AUTOSENSE,
                                            TAG_DONE);
                                        break;

                                }

                                break;

                            /* Reset gadget */
                            case GUI_RESET_ID:

                                /* Clear button status */
                                GT_SetGadgetAttrs(btn1Gadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(btn2Gadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(btn3Gadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(btn4Gadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(btn5Gadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(btn6Gadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(btn7Gadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);

                                /* Clear direction status */
                                GT_SetGadgetAttrs(upGadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(downGadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(leftGadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);
                                GT_SetGadgetAttrs(rightGadget,window,NULL,
                                    GTCB_Checked, FALSE,
                                    TAG_DONE);

                            /* Default */
                            default:
                                /* No action */
                                break;

                        }

                    }

                /* Default */
                default:
                    /* No action */
                    break;

            }

        }

    }

}
