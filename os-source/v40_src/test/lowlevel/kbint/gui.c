/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>

#include <devices/inputevent.h>

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/keymap_protos.h>
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

#include "kbint.h"

/****** KBInt/guiOpen ******************************************
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

    /*
     * Open window
     */

    /* Open window */
    window=OpenWindowTags(NULL,
        WA_Title, PROGRAM_NAME,
        WA_InnerWidth, 340,
        WA_InnerHeight, 116,
        WA_DragBar, TRUE,
        WA_CloseGadget, TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate, TRUE,
        WA_SimpleRefresh, TRUE,
        WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|
            NUMBERIDCMP|TEXTIDCMP|CHECKBOXIDCMP,
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

    /* Key gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+40;
    newGadget.ng_TopEdge=window->BorderTop+12;
    newGadget.ng_Width=24;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Key";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_KEY_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=keyGadget=CreateGadget(TEXT_KIND,
        previousGadget,&newGadget,
        GTTX_Border, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Code gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+128;
    newGadget.ng_TopEdge=window->BorderTop+12;
    newGadget.ng_Width=32;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Code";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_CODE_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=codeGadget=CreateGadget(NUMBER_KIND,
        previousGadget,&newGadget,
        GTNM_Border, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Transition gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+264;
    newGadget.ng_TopEdge=window->BorderTop+12;
    newGadget.ng_Width=64;
    newGadget.ng_Height=16;
    newGadget.ng_GadgetText="Transition";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_KEY_ID;
    newGadget.ng_Flags=PLACETEXT_LEFT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=transitionGadget=CreateGadget(TEXT_KIND,
        previousGadget,&newGadget,
        GTTX_Border, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* LShift gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+36;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="LShift";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_LSHIFT_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=lShiftGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* RShift gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+248;
    newGadget.ng_TopEdge=window->BorderTop+36;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="RShift";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_RSHIFT_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=rShiftGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Caps Lock gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+52;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Caps Lock";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_CAPSLOCK_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=capsLockGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* Control gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+68;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="Control";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_CONTROL_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=controlGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* LAlt gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+84;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="LAlt";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_LALT_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=lAltGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* RAlt gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+248;
    newGadget.ng_TopEdge=window->BorderTop+84;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="RAlt";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_RALT_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=rAltGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* LAmiga gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+8;
    newGadget.ng_TopEdge=window->BorderTop+100;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="LAmiga";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_LAMIGA_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=lAmigaGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
        TAG_DONE);
    if (!previousGadget) {
        return(FALSE);
    }

    /* RAmiga gadget */
    newGadget.ng_LeftEdge=window->BorderLeft+248;
    newGadget.ng_TopEdge=window->BorderTop+100;
    newGadget.ng_Width=26;
    newGadget.ng_Height=11;
    newGadget.ng_GadgetText="RAmiga";
    newGadget.ng_TextAttr=&textAttr;
    newGadget.ng_GadgetID=GUI_RAMIGA_ID;
    newGadget.ng_Flags=PLACETEXT_RIGHT;
    newGadget.ng_VisualInfo=visualInfo;
    newGadget.ng_UserData=NULL;
    previousGadget=rAmigaGadget=CreateGadget(CHECKBOX_KIND,
        previousGadget,&newGadget,
        GA_Disabled, TRUE,
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

/****** KBInt/guiClose **************************************************
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

/****** KBInt/guiLoop ***************************************************
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
                 | (1<<kbSignal)                  /* or keyboard signal */
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

                    /* Default */
                    default:
                        /* No action */
                        break;

                }

            }

        }

        /* If keyboard signal ... */
        if (signalFlags&(1<<kbSignal)) {

            /* Update keyboard display */
            kbUpdate();

        }

    }

}
