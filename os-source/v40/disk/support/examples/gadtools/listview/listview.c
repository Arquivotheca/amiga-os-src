;/*
SC RESOPT PARM=REGISTERS UCHAR CONSTLIB STREQ ANSI NOSTKCHK NOICONS OPT OPTPEEP listview.c
Slink LIB:c.o listview.o TO ListView LIB LIB:sc.lib SC SD
Quit
*/

/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/

/* listview.c - a simple program showing how to use listview rendering
 * callback functions
 */


/****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <utility/hooks.h>
#include <libraries/gadtools.h>
#include <graphics/gfxmacros.h>
#include <string.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>


/****************************************************************************/


typedef unsigned long (*HOOKFUNC)();


/****************************************************************************/


static struct TextAttr topazAttr =
{
    "topaz.font",
     8,
     FS_NORMAL,
     FPF_ROMFONT
};


/*****************************************************************************/


extern struct Library *SysBase;
struct Library        *GfxBase;
struct Library        *IntuitionBase;
struct Library        *GadToolsBase;
struct Library        *UtilityBase;

struct Screen         *screen;
struct Window         *window;
struct DrawInfo       *drawInfo;
APTR                   visualInfo;


/*****************************************************************************/


static UWORD GhostPattern[2] =
{
     0x4444,
     0x1111,
};


/* Apply a ghosting pattern to a given rectangle in a rastport */
static VOID Ghost(struct RastPort *rp, UWORD pen, UWORD x0, UWORD y0, UWORD x1, UWORD y1)
{
    SetABPenDrMd(rp,pen,0,JAM1);
    SetAfPt(rp,GhostPattern,1);
    RectFill(rp,x0,y0,x1,y1);
    SetAfPt(rp,NULL,0);
}


/*****************************************************************************/


/* Erase any part of "oldExtent" which is not covered by "newExtent" */
VOID FillOldExtent(struct RastPort *rp,
                   struct Rectangle *oldExtent,
                   struct Rectangle *newExtent)
{
    if (oldExtent->MinX < newExtent->MinX)
        RectFill(rp,oldExtent->MinX,
                    oldExtent->MinY,
                    newExtent->MinX-1,
                    oldExtent->MaxY);

    if (oldExtent->MaxX > newExtent->MaxX)
        RectFill(rp,newExtent->MaxX+1,
                    oldExtent->MinY,
                    oldExtent->MaxX,
                    oldExtent->MaxY);

    if (oldExtent->MaxY > newExtent->MaxY)
        RectFill(rp,oldExtent->MinX,
                    newExtent->MaxY+1,
                    oldExtent->MaxX,
                    oldExtent->MaxY);

    if (oldExtent->MinY < newExtent->MinY)
        RectFill(rp,oldExtent->MinX,
                    oldExtent->MinY,
                    oldExtent->MaxX,
                    newExtent->MinY-1);
}


/*****************************************************************************/


/* This function is called whenever an item of the listview needs to be drawn
 * by gadtools. The function must fill every pixel of the area described in
 * the LVDrawMsg structure. This function does the exact same rendering as
 * the built-in rendering function in gadtools, except that it render the
 * normal items using the highlight text pen instead of simply text pen.
 */
static ULONG __asm RenderHook(register __a1 struct LVDrawMsg *msg,
                              register __a2 struct Node *node)
{
struct RastPort   *rp;
UBYTE              state;
struct TextExtent  extent;
ULONG              fit;
WORD               x,y;
WORD               slack;
ULONG              apen;
ULONG              bpen;
UWORD             *pens;
STRPTR             name;

    geta4();

    if (msg->lvdm_MethodID != LV_DRAW)
        return(LVCB_UNKNOWN);

    rp    = msg->lvdm_RastPort;
    state = msg->lvdm_State;
    pens  = msg->lvdm_DrawInfo->dri_Pens;

    apen = pens[FILLTEXTPEN];
    bpen = pens[FILLPEN];
    if ((state == LVR_NORMAL) || (state == LVR_NORMALDISABLED))
    {
        apen = pens[HIGHLIGHTTEXTPEN];    /* this is normally TEXTPEN */
        bpen = pens[BACKGROUNDPEN];
    }
    SetABPenDrMd(rp,apen,bpen,JAM2);

    name = node->ln_Name;

    fit = TextFit(rp,name,strlen(name),&extent,NULL,1,
                  msg->lvdm_Bounds.MaxX-msg->lvdm_Bounds.MinX-3,
                  msg->lvdm_Bounds.MaxY-msg->lvdm_Bounds.MinY+1);

    slack = (msg->lvdm_Bounds.MaxY - msg->lvdm_Bounds.MinY) - (extent.te_Extent.MaxY - extent.te_Extent.MinY);

    x = msg->lvdm_Bounds.MinX - extent.te_Extent.MinX + 2;
    y = msg->lvdm_Bounds.MinY - extent.te_Extent.MinY + ((slack+1) / 2);

    extent.te_Extent.MinX += x;
    extent.te_Extent.MaxX += x;
    extent.te_Extent.MinY += y;
    extent.te_Extent.MaxY += y;

    Move(rp,x,y);
    Text(rp,name,fit);

    SetAPen(rp,bpen);
    FillOldExtent(rp,&msg->lvdm_Bounds,&extent.te_Extent);

    if ((state == LVR_NORMALDISABLED) || (state == LVR_SELECTEDDISABLED))
    {
        Ghost(rp,pens[BLOCKPEN],msg->lvdm_Bounds.MinX, msg->lvdm_Bounds.MinY,
                                msg->lvdm_Bounds.MaxX, msg->lvdm_Bounds.MaxY);
    }

    return(LVCB_OK);
}


/*****************************************************************************/


/* Main entry point. Init the universe, and do the event loop */
LONG main(VOID)
{
BOOL                 quit;
struct IntuiMessage *intuiMsg;
struct Gadget       *gadgetList;
struct Hook          renderHook;
struct List          list;
struct Node          nodes[5];
struct NewGadget     ng;

    renderHook.h_Entry = (HOOKFUNC)RenderHook;

    nodes[0].ln_Name = "First Node";
    nodes[1].ln_Name = "Second Node";
    nodes[2].ln_Name = "Third Node";
    nodes[3].ln_Name = "Fourth Node";
    nodes[4].ln_Name = "Fifth Node";
    NewList(&list);
    AddTail(&list,&nodes[0]);
    AddTail(&list,&nodes[1]);
    AddTail(&list,&nodes[2]);
    AddTail(&list,&nodes[3]);
    AddTail(&list,&nodes[4]);

    /* open all basic libraries in case we need 'em later */
    if (IntuitionBase = OpenLibrary("intuition.library", 39))
    {
        GfxBase       = OpenLibrary("graphics.library", 39);
        GadToolsBase  = OpenLibrary("gadtools.library", 39);
        UtilityBase   = OpenLibrary("utility.library", 39);
    }

    /* check if we got everything we need */
    if (IntuitionBase && GfxBase && GadToolsBase && UtilityBase)
    {
        if (screen = LockPubScreen(NULL))
        {
            if (drawInfo = GetScreenDrawInfo(screen))
            {
                if (visualInfo = GetVisualInfoA(screen,NULL))
                {
                    gadgetList = NULL;

                    ng.ng_LeftEdge   = 10;
                    ng.ng_TopEdge    = screen->WBorTop + screen->Font->ta_YSize + 1 + 16;
                    ng.ng_Width      = 200;
                    ng.ng_Height     = 120;
                    ng.ng_GadgetText = "A List Of Things";
                    ng.ng_TextAttr   = &topazAttr;
                    ng.ng_Flags      = 0;
                    ng.ng_VisualInfo = visualInfo;

                    if (CreateGadget(LISTVIEW_KIND,CreateContext(&gadgetList),&ng,
                                     GTLV_Labels,       &list,
                                     GTLV_ShowSelected, NULL,
                                     GTLV_ScrollWidth,  19,
                                     GTLV_ItemHeight,   12,
                                     GTLV_CallBack,     &renderHook,
                                     GTLV_Selected,     0,
                                     GTLV_MaxPen,       255,
                                     TAG_DONE))
                    {
                        if (window = OpenWindowTags(NULL,
                                          WA_InnerWidth,   220,
                                          WA_InnerHeight,  150,
                                          WA_IDCMP,        IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | LISTVIEWIDCMP,
                                          WA_DepthGadget,  TRUE,
                                          WA_DragBar,      TRUE,
                                          WA_CloseGadget,  TRUE,
                                          WA_SimpleRefresh,TRUE,
                                          WA_Activate,     TRUE,
                                          WA_Title,        "ListView Tester",
                                          WA_PubScreen,    screen,
                                          WA_AutoAdjust,   TRUE,
                                          WA_Gadgets,      gadgetList,
                                          TAG_DONE))
                        {
                            GT_RefreshWindow(window,NULL);

                            quit = FALSE;
                            while (!quit)
                            {
                                WaitPort(window->UserPort);
                                if (intuiMsg = (struct IntuiMessage *)GT_GetIMsg(window->UserPort))
                                {
                                    if (intuiMsg->Class == IDCMP_REFRESHWINDOW)
                                    {
                                        GT_BeginRefresh(window);
                                        GT_EndRefresh(window,TRUE);
                                    }
                                    else if (intuiMsg->Class == IDCMP_CLOSEWINDOW)
                                    {
                                        quit = TRUE;
                                    }
                                    GT_ReplyIMsg(intuiMsg);
                                }
                            }

                            CloseWindow(window);
                        }
                    }
                    FreeGadgets(gadgetList);
                    FreeVisualInfo(visualInfo);
                }
                FreeScreenDrawInfo(screen,drawInfo);
            }
            UnlockPubScreen(NULL,screen);
        }
    }

    if (IntuitionBase)
    {
        CloseLibrary(UtilityBase);
        CloseLibrary(GadToolsBase);
        CloseLibrary(GfxBase);
        CloseLibrary(IntuitionBase);
    }

    return(0);
}
