
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <graphics/text.h>
#include <string.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/locale_protos.h>
#include <clib/dos_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "asl.h"
#include "aslbase.h"
#include "aslutils.h"
#include "layout.h"
#include "requtils.h"


/*****************************************************************************/


#define LocaleBase ri->ri_LocaleInfo.li_LocaleBase
#define catalog    ri->ri_LocaleInfo.li_Catalog


/*****************************************************************************/


VOID SaveCoords(struct ReqInfo *ri)
{
    ri->ri_Coords.Left   = ri->ri_Window->LeftEdge;
    ri->ri_Coords.Top    = ri->ri_Window->TopEdge;
    ri->ri_Coords.Width  = ri->ri_Window->Width;
    ri->ri_Coords.Height = ri->ri_Window->Height;
}


/*****************************************************************************/


/* This is the message packet passed by layers.library to a backfill hook.
 * It contains a pointer to the layer that has been damaged, a Rectangle
 * structure that defines the bounds of the damage. No rendering can occur
 * outside of these coordinates.
 *
 * The backfill hook is also passed a RastPort in which the rendering
 * should be performed.
 */
struct BackFillMsg
{
    struct Layer     *bf_Layer;
    struct Rectangle  bf_Bounds;
    LONG              bf_OffsetX;
    LONG              bf_OffsetY;
};


#define offsetof(type,memb)  ((ULONG)&(((type *) 0L)->memb))
typedef unsigned long (*HOOKFUNC)();

#undef AslBase

VOID __asm BackFillHook(register __a0 struct Hook *hook,
                        register __a2 struct RastPort    *rp,
                        register __a1 struct BackFillMsg *bfm)
{
struct RastPort  crp;
struct ASLLib   *AslBase = hook->h_Data;
struct ReqInfo  *ri;

    ri = (struct ReqInfo *)((ULONG)hook - offsetof(struct ReqInfo,ri_BackFill));

    crp       = *rp;
    crp.Layer = NULL;

    SetAPen(&crp,ri->ri_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    SetDrMd(&crp,JAM2);
    RectFill(&crp,bfm->bf_Bounds.MinX,
                  bfm->bf_Bounds.MinY,
                  bfm->bf_Bounds.MaxX,
                  bfm->bf_Bounds.MaxY);
}

#define AslBase       ((struct ASLLib *)getreg(REG_A6))


/*****************************************************************************/


APTR PrepareRequester(struct ReqInfo *ri, APTR templates, ULONG templateSize,
                      ULONG memorySize)
{
APTR memory;

    if (ri->ri_Template = AllocVec(templateSize,MEMF_ANY))
    {
        if (memory = AllocVec(memorySize,MEMF_CLEAR|MEMF_ANY))
        {
            CopyMem(templates,ri->ri_Template,templateSize);
            return(memory);
        }
        FreeVec(ri->ri_Template);
    }

    SetIoErr(ERROR_NO_FREE_STORE);
    return(NULL);
}


/*****************************************************************************/


VOID CleanupRequester(struct ReqInfo *ri, APTR memory)
{
    if (LocaleBase)
    {
        if (!ri->ri_UserLocale)
            CloseLocale(ri->ri_LocaleInfo.li_Locale);

        CloseCatalog(catalog);
        CloseLibrary(LocaleBase);
    }

    FreeVec(ri->ri_Template);
    FreeVec(memory);
}


/*****************************************************************************/


VOID PrepareLocale(struct ReqInfo *ri)
{
    if (LocaleBase = (APTR)OpenLibrary("locale.library",38))
    {
        if (ri->ri_UserLocale)
            ri->ri_LocaleInfo.li_Locale = ri->ri_UserLocale;
        else
            ri->ri_LocaleInfo.li_Locale = OpenLocale(NULL);

        catalog = OpenCatalogA(ri->ri_LocaleInfo.li_Locale,"sys/libs.catalog",NULL);
    }
    else
    {
        ri->ri_LocaleInfo.li_Locale = NULL;
    }
}


/*****************************************************************************/


BOOL LayoutASLMenus(struct ReqInfo *ri, struct Menu *menus, ULONG tag1, ...)
{
    return(LayoutMenusA(menus,ri->ri_VisualInfo,(struct TagItem *) &tag1));
}


/*****************************************************************************/


struct Menu *CreateASLMenus(struct ReqInfo *ri,
                            struct ASLMenu *am)
{
UWORD           i;
struct NewMenu *nm;
struct Menu    *menus;

    i = 0;
    while (am[i++].am_Type != NM_END) {}

    if (!(nm = AllocVec(sizeof(struct NewMenu)*i,MEMF_CLEAR|MEMF_ANY)))
        return(NULL);

    while (i--)
    {
        nm[i].nm_Type        = am[i].am_Type;
        nm[i].nm_UserData    = (APTR)am[i].am_Command;

        if (am[i].am_Type == NM_TITLE)
        {
            nm[i].nm_Label   = GetString(&ri->ri_LocaleInfo,am[i].am_Label);
        }
        else if (am[i].am_Label == (ULONG)NM_BARLABEL)
        {
            nm[i].nm_Label   = NM_BARLABEL;
        }
        else if (am[i].am_Type != NM_END)
        {
            nm[i].nm_CommKey = GetString(&ri->ri_LocaleInfo,am[i].am_Label);
            nm[i].nm_Label   = &nm[i].nm_CommKey[2];
            if (nm[i].nm_CommKey[0] == ' ')
            {
                nm[i].nm_CommKey = NULL;
            }
        }
    }

    if (menus = CreateMenusA(nm,NULL))
    {
        if (!(LayoutASLMenus(ri,menus,GTMN_NewLookMenus,TRUE,
                                      TAG_DONE)))
        {
            FreeMenus(menus);
	    menus = NULL;
	}
    }

    FreeVec(nm);

    return(menus);
}


/*****************************************************************************/


VOID ClearReqInfo(struct ReqInfo *ri)
{
    ri->ri_Window     = NULL;
    ri->ri_Screen     = NULL;
    ri->ri_Menus      = NULL;
    ri->ri_Gadgets    = NULL;
    ri->ri_DrawInfo   = NULL;
    ri->ri_VisualInfo = NULL;
    ri->ri_DFont      = NULL;
    ri->ri_DTextAttr  = NULL;
    ri->ri_Font       = NULL;
    ri->ri_TextAttr   = NULL;
    ri->ri_RastPort   = NULL;
    ri->ri_LastAdded  = NULL;
    ri->ri_ASleep     = FALSE;
}


/*****************************************************************************/


VOID SleepRequester(struct ReqInfo *ri)
{
struct TagItem tags[3];

    tags[0].ti_Tag  = WA_BusyPointer;
    tags[0].ti_Data = TRUE;
    tags[1].ti_Tag  = WA_PointerDelay;
    tags[1].ti_Data = TRUE;
    tags[2].ti_Tag  = TAG_DONE;

    InitRequester(&ri->ri_LocalSleepReq);
    ri->ri_LocalASleep = Request(&ri->ri_LocalSleepReq,ri->ri_Window);
    SetWindowPointerA(ri->ri_Window,tags);
    WindowLimits(ri->ri_Window,ri->ri_Window->Width,ri->ri_Window->Height,ri->ri_Window->Width,ri->ri_Window->Height);
}


/*****************************************************************************/


VOID WakeupRequester(struct ReqInfo *ri)
{
    if (ri->ri_LocalASleep)
        EndRequest(&ri->ri_LocalSleepReq,ri->ri_Window);

    ClearPointer(ri->ri_Window);
    WindowLimits(ri->ri_Window,ri->ri_MinWidth,ri->ri_MinHeight,1024,1024);
}


/*****************************************************************************/


BOOL OpenRequester(struct ReqInfo *ri, struct ASLMenu *menus)
{
struct Screen *screen = NULL;
STRPTR         title;
BOOL           unlock = FALSE;
struct TagItem tags[2];

    ri->ri_SharedIDCMP = FALSE;
    ri->ri_ASleep      = FALSE;
    ri->ri_Gadgets     = NULL;

    ri->ri_BackFill.h_Entry = (HOOKFUNC)BackFillHook;
    ri->ri_BackFill.h_Data  = AslBase;

    if (ri->ri_UserWindow)
        screen = ri->ri_UserWindow->WScreen;

    if (ri->ri_UserScreenName)
    {
        screen = LockPubScreen(ri->ri_UserScreenName);
        unlock = TRUE;
    }

    if (ri->ri_UserScreen)
    {
        if (unlock)
            UnlockPubScreen(NULL,screen);
        unlock = FALSE;
        screen = ri->ri_UserScreen;
    }

    if ((!screen) && (!ri->ri_UserScreenName))
    {
        screen = LockPubScreen(NULL);
        unlock = TRUE;
    }

    if (screen)
    {
        if (!(ri->ri_DTextAttr = ri->ri_UserAttr))
	    ri->ri_DTextAttr = screen->Font;

        if (ri->ri_DFont = OpenFont (ri->ri_DTextAttr))
        {
            if (ri->ri_VisualInfo = GetVisualInfoA(screen,NULL))
            {
                if (ri->ri_DrawInfo = GetScreenDrawInfo(screen))
                {
                    if (ri->ri_Menus = CreateASLMenus(ri,menus))
                    {
                        title = GetString(&ri->ri_LocaleInfo,ri->ri_TitleID);
                        if (ri->ri_TitleText)
                            title = ri->ri_TitleText;

			ri->ri_Screen = screen;

			LayoutGadgets(ri,LGM_SETMIN);

                        if (ri->ri_Window = AslOpenWindow(
                                              WA_Left,        ri->ri_Coords.Left,
                                              WA_Top,         ri->ri_Coords.Top,
                                              WA_Width,       ri->ri_Coords.Width,
                                              WA_Height,      ri->ri_Coords.Height,
                                              WA_Flags,       ri->ri_WindowFlags,
                                              WA_AutoAdjust,  TRUE,
                                              WA_CustomScreen,screen,
                                              WA_Title,       title,
                                              WA_NewLookMenus,TRUE,
                                              WA_MinWidth,    ri->ri_MinWidth,
                                              WA_MaxWidth,    1024,
                                              WA_MinHeight,   ri->ri_MinHeight,
                                              WA_MaxHeight,   1024,
                                              WA_HelpGroupWindow, ri->ri_UserWindow,
                                              WA_BackFill,    &ri->ri_BackFill,
                                              TAG_DONE))
                        {
                            ri->ri_RastPort = ri->ri_Window->RPort;
                            SetFont(ri->ri_RastPort,ri->ri_Font);
                            SetMenuStrip(ri->ri_Window,ri->ri_Menus);
			    CalculatePaneSize(ri);

                            if (ri->ri_MakeSleep && ri->ri_UserWindow)
                            {
                                InitRequester(&ri->ri_SleepReq);
                                ri->ri_ASleep = Request(&ri->ri_SleepReq,ri->ri_UserWindow);

                                tags[0].ti_Tag  = WA_BusyPointer;
                                tags[0].ti_Data = TRUE;
                                tags[1].ti_Tag  = TAG_DONE;
                                SetWindowPointerA(ri->ri_UserWindow,tags);
                            }

                            if ((!ri->ri_PrivateIDCMP) && (ri->ri_UserWindow) && (ri->ri_UserWindow->UserPort))
                            {
                                ri->ri_Window->UserPort = ri->ri_UserWindow->UserPort;
                                ri->ri_SharedIDCMP = TRUE;
                            }

                            if (ModifyIDCMP(ri->ri_Window,ri->ri_IDCMPFlags))
                            {
                                if (unlock)
                                    UnlockPubScreen(NULL,screen);

                                return(TRUE);
                            }

                            if (ri->ri_ASleep)
                                EndRequest(&ri->ri_SleepReq,ri->ri_Window);

			    SaveCoords(ri);

                            AslCloseWindow(ri->ri_Window,ri->ri_SharedIDCMP);
                        }
                        FreeLayoutGadgets(ri,FALSE);
                        FreeMenus(ri->ri_Menus);
                    }
                    FreeScreenDrawInfo(screen,ri->ri_DrawInfo);
                }
                FreeVisualInfo(ri->ri_VisualInfo);
            }
            CloseFont(ri->ri_DFont);
        }

        if (unlock)
            UnlockPubScreen(NULL,screen);
    }

    ClearReqInfo(ri);

    return(FALSE);
}


/*****************************************************************************/


VOID CloseRequester(struct ReqInfo *ri)
{
    if (ri->ri_ASleep)
    {
        EndRequest(&ri->ri_SleepReq,ri->ri_UserWindow);
        ClearPointer(ri->ri_UserWindow);
    }

    CalculatePaneSize(ri);

    FreeScreenDrawInfo(ri->ri_Screen,ri->ri_DrawInfo);
    SaveCoords(ri);
    AslCloseWindow(ri->ri_Window,ri->ri_SharedIDCMP);
    CloseFont(ri->ri_DFont);
    FreeLayoutGadgets(ri,FALSE);
    FreeMenus(ri->ri_Menus);
    FreeVisualInfo(ri->ri_VisualInfo);

    ClearReqInfo(ri);
}


/*****************************************************************************/


struct IntuiMessage *GetReqMsg(struct ReqInfo *ri, APTR object,
			       struct Window *aux, ULONG extraSigs)
{
ULONG                mask;
ULONG                sigs;
struct IntuiMessage *intuiMsg;
struct Window       *req;
struct Window       *parent;

    req    = ri->ri_Window;
    parent = ri->ri_UserWindow;

    mask = (1<<req->UserPort->mp_SigBit) + extraSigs;
    if (parent && parent->UserPort)
        mask |= (1<<parent->UserPort->mp_SigBit);

    while (TRUE)
    {
        do
        {
            intuiMsg = GT_GetIMsg(req->UserPort);
            if (!intuiMsg && parent && parent->UserPort)
                intuiMsg = GT_GetIMsg(parent->UserPort);

            if (!intuiMsg)
            {
                sigs = Wait(mask);
                if (sigs & extraSigs)
                    return(NULL);
            }
        } while (!intuiMsg);

        if (intuiMsg->IDCMPWindow == req)
            return(intuiMsg);

        if (intuiMsg->IDCMPWindow == aux)
            return(intuiMsg);

        if (ri->ri_CallOldHook && ri->ri_OldHook)
        {
            if ((struct IntuiMessage *)CallFunc(ri->ri_OldHook,FRF_INTUIFUNC,(ULONG)intuiMsg,(ULONG)object,ri->ri_A4) == intuiMsg)
                GT_ReplyIMsg(intuiMsg);
        }
        else
        {
            if (ri->ri_IntuiHook)
                CallHookPkt(ri->ri_IntuiHook,object,intuiMsg);

            GT_ReplyIMsg(intuiMsg);
        }
    }
}


/*****************************************************************************/


VOID ProcessStdTag(struct ReqInfo *ri, struct TagItem *tag)
{
ULONG data;

    data = tag->ti_Data;
    switch (tag->ti_Tag)
    {
        case ASL_InitialLeftEdge: ri->ri_Coords.Left = (WORD)data;
                                  break;

        case ASL_InitialTopEdge : ri->ri_Coords.Top = (WORD)data;
                                  break;

        case ASL_InitialWidth   : ri->ri_Coords.Width = (WORD)data;
                                  break;

        case ASL_InitialHeight  : ri->ri_Coords.Height = (WORD)data;
                                  break;

        case ASL_Window         : ri->ri_UserWindow = (struct Window *)data;
                                  break;

        case ASL_Screen         : ri->ri_UserScreen = (struct Screen *)data;
                                  break;

        case ASL_PubScreenName  : ri->ri_UserScreenName = (STRPTR)data;
                                  break;

        case ASL_TitleText      : ri->ri_TitleText = (STRPTR)data;
                                  break;

        case ASL_PositiveText   : ri->ri_PositiveText = (STRPTR)data;
                                  break;

        case ASL_NegativeText   : ri->ri_NegativeText = (STRPTR)data;
                                  break;

        case ASL_TextAttr       : ri->ri_UserAttr = (struct TextAttr *)data;
                                  break;

        case ASL_Locale         : ri->ri_UserLocale = (struct Locale *)data;
                                  break;

        case ASL_SleepWindow    : ri->ri_MakeSleep = (BOOL)data;
                                  break;

        case ASL_IntuiMsgFunc   : ri->ri_IntuiHook = (struct Hook *)data;
                                  break;

        case ASL_HookFunc       : ri->ri_OldHook = (APTR)data;
                                  break;

        case ASL_PrivateIDCMP   : ri->ri_PrivateIDCMP = (BOOL)data;
                                  break;

        default                 : break;
    }
}


/*****************************************************************************/


ULONG CallFunc(ULONG __stdargs  (*oldHook)(ULONG,ULONG,ULONG),
               ULONG arg1, ULONG arg2, ULONG arg3, ULONG a4)
{
ULONG oldA4;
ULONG result;

    oldA4 = getreg(REG_A4);
    putreg(REG_A4,a4);
    result = oldHook(arg1,arg2,arg3);
    putreg(REG_A4,oldA4);

    return(result);
}
