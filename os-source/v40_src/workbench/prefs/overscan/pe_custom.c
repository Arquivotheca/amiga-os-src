
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/keymap.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <prefs/locale.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"
#include "edit.h"


#define SysBase ed->ed_SysBase
#define MONITOR_PART(id)	((id) & MONITOR_ID_MASK)


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_OSCN,
};


/*****************************************************************************/


/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,   /* version */
    0,   /* type    */
    0    /* flags   */
};


/*****************************************************************************/


VOID BuildList(EdDataPtr ed)
{
ULONG                modeID;
BOOL                 present;
struct Node         *node;
struct MonitorEntry *me;
struct DimensionInfo dimInfo;
struct MonitorInfo   monInfo;
struct DisplayInfo   dispInfo;
UWORD                len;
DisplayInfoHandle    dh;
/*
ULONG                wbID;
struct Screen       *wb;
*/
ULONG                id;

    NewList(&ed->ed_MonitorList);
/*
    wbID = INVALID_ID;
    if (wb = LockPubScreen("Workbench"))
    {
        wbID = GetVPModeID(&wb->ViewPort);
        UnlockPubScreen(NULL,wb);
    }
*/
    modeID = INVALID_ID;
    while ((modeID = NextDisplayInfo(modeID)) != INVALID_ID)
    {
	if (MONITOR_PART(modeID))
	{
            present = FALSE;
            for (me = (struct MonitorEntry *)ed->ed_MonitorList.lh_Head; me->me_Node.ln_Succ; me = (struct MonitorEntry *)me->me_Node.ln_Succ)
                if (MONITOR_PART(me->me_ID) == MONITOR_PART(modeID))
                    present = TRUE;

            if ((!present)
            &&  (dh = FindDisplayInfo(modeID))
            &&  (GetDisplayInfoData(dh,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID))
            &&  (GetDisplayInfoData(dh,(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,INVALID_ID)))
            {
                if ((!dispInfo.NotAvailable) && (dispInfo.PropertyFlags & DIPF_IS_WB))
		{
		    id = monInfo.PreferredModeID;
/*
		    if (MONITOR_PART(wbID) == MONITOR_PART(id))
		        id = wbID;
*/
                    if ((dh = FindDisplayInfo(id))
                    &&  (GetDisplayInfoData(dh,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID))
                    &&  (GetDisplayInfoData(dh,(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,INVALID_ID))
                    &&  (GetDisplayInfoData(dh,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,INVALID_ID)))
                    {
                        if (me = AllocVec(sizeof(struct MonitorEntry),MEMF_CLEAR|MEMF_PUBLIC))
                        {
                            me->me_ID = id;

                            if (monInfo.Mspc)
                            {
                                if (monInfo.Mspc->ms_Special)
                                {
                                    me->me_HStart = monInfo.Mspc->ms_Special->hsync.asi_Start;
                                    me->me_HStop  = monInfo.Mspc->ms_Special->hsync.asi_Stop;
                                    me->me_VStart = monInfo.Mspc->ms_Special->vsync.asi_Start;
                                    me->me_VStop  = monInfo.Mspc->ms_Special->vsync.asi_Stop;
                                }

                                if (monInfo.Mspc->ms_Node.xln_Name)
                                {
                                    stccpy(me->me_Name,monInfo.Mspc->ms_Node.xln_Name,MENAMELEN);
                                    len = strlen(me->me_Name);
                                    if ((len > 8) && (Stricmp(&me->me_Name[len-8],".monitor")==0))
                                        me->me_Name[len-8] = 0;
                                    me->me_Name[0] = ToUpper(me->me_Name[0]);

                                    if ((Stricmp(me->me_Name,"NTSC") == 0) || (Stricmp(me->me_Name,"PAL") == 0))
                                    {
                                        len = strlen(me->me_Name);
                                        while (len--)
                                            me->me_Name[len] = ToUpper(me->me_Name[len]);
                                    }
                                }
                            }

                            me->me_Node.ln_Name  = me->me_Name;
                            me->me_DisplayInfo   = dispInfo;
                            me->me_DimensionInfo = dimInfo;
                            me->me_MonitorInfo   = monInfo;

                            node = ed->ed_MonitorList.lh_Head;
                            while (node->ln_Succ)
                            {
                                if (Stricmp(node->ln_Name,me->me_Node.ln_Name) >= 0)
                                    break;
                                node = node->ln_Succ;
                            }
                            Insert(&ed->ed_MonitorList,(struct Node *)me,node->ln_Pred);
                        }
                    }
                }
	    }
	}
    }
}


/*****************************************************************************/


BOOL InitPrefs(EdDataPtr ed, struct ExtPrefs *prefs)
{
ULONG        count;
struct Node *node;

    count = 0;
    node = ed->ed_MonitorList.lh_Head;
    while (node->ln_Succ)
    {
        node = node->ln_Succ;
        count++;
    }

    if (count == 0)
        return(TRUE);

    if (prefs->ep_Next = AllocVec(sizeof(struct ExtPrefs)*count,MEMF_PUBLIC|MEMF_CLEAR))
    {
        prefs = prefs->ep_Next;
        node  = ed->ed_MonitorList.lh_Head;
        while (node->ln_Succ)
        {
            prefs->ep_Monitor = *(struct MonitorEntry *)node;
            node = node->ln_Succ;

            if (node->ln_Succ)
            {
                prefs->ep_Next = (struct ExtPrefs *) ((ULONG)prefs + sizeof(struct ExtPrefs));
                prefs = prefs->ep_Next;
            }
        }
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID FitRect(struct Rectangle *container, struct Rectangle *inner)
{
    if (inner->MinX < container->MinX)
        inner->MinX = container->MinX;

    if (inner->MinY < container->MinY)
        inner->MinY = container->MinY;

    if (inner->MaxX > container->MaxX)
        inner->MaxX = container->MaxX;

    if (inner->MaxY > container->MaxY)
        inner->MaxY = container->MaxY;
}


/*****************************************************************************/


VOID GrowRect(struct Rectangle *container, struct Rectangle *inner)
{
    if (inner->MinX > container->MinX)
        inner->MinX = container->MinX;

    if (inner->MinY > container->MinY)
        inner->MinY = container->MinY;

    if (inner->MaxX < container->MaxX)
        inner->MaxX = container->MaxX;

    if (inner->MaxY < container->MaxY)
        inner->MaxY = container->MaxY;
}


/*****************************************************************************/


VOID PrefsToMonitorEntry(EdDataPtr ed, struct OverscanPrefs *op,
                         struct MonitorEntry *me)
{
struct DisplayInfo    dispInfo;
struct MonitorInfo    monInfo;
struct DimensionInfo  dimInfo;
Point                 shift;
Point	              newviewpos;
UWORD                 xscale,yscale;
DisplayInfoHandle     dh;

    if ((dh = FindDisplayInfo(me->me_ID))
    &&  (GetDisplayInfoData(dh,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID))
    &&  (GetDisplayInfoData(dh,(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,INVALID_ID))
    &&  (GetDisplayInfoData(dh,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,INVALID_ID)))
    {
        xscale = monInfo.ViewResolution.x / dispInfo.Resolution.x;
        yscale = monInfo.ViewResolution.y / dispInfo.Resolution.y;

	/* Start with the real live MaxOScan from the graphics database,
	 * and displace it by the amount that the new ViewPosition differs
	 * from the actual ViewPosition (scaled to the appropriate resolution).
	 * Note that we must validate the new ViewPosition because the
	 * addition/deletion of the VGAOnly icon could alter the
	 * ViewPositionRange (isn't persistent data fun?!?)
	 */
	newviewpos = op->os_ViewPos;
        /* make sure the view's horizontal position falls within legal range */
        if (newviewpos.x < monInfo.ViewPositionRange.MinX)
            newviewpos.x = monInfo.ViewPositionRange.MinX;
        else if (newviewpos.x > monInfo.ViewPositionRange.MaxX)
            newviewpos.x = monInfo.ViewPositionRange.MaxX;

        /* make sure the view's vertical position falls within legal range */
        if (newviewpos.y < monInfo.ViewPositionRange.MinY)
            newviewpos.y = monInfo.ViewPositionRange.MinY;
        else if (newviewpos.y > monInfo.ViewPositionRange.MaxY)
            newviewpos.y = monInfo.ViewPositionRange.MaxY;

        shift.x = ((monInfo.ViewPosition.x - newviewpos.x) * monInfo.ViewResolution.x) / dispInfo.Resolution.x;
        shift.y = ((monInfo.ViewPosition.y - newviewpos.y) * monInfo.ViewResolution.y) / dispInfo.Resolution.y;

        ShiftRect(&dimInfo.MaxOScan, shift.x, shift.y);

	/* Set up TxtOScan based on the prefs data.  MinX and MinY
	 * are zero by definition;  the view origin is _defined_ to be
	 * the upper-left corner of the TxtOScan rectangle.  Everything
	 * is quantized to the ViewResolution, which may be coarser
	 * (an even multiple) of the mode's actual resolution.
	 */
        dimInfo.TxtOScan.MinX  = 0;
        dimInfo.TxtOScan.MinY  = 0;
        dimInfo.TxtOScan.MaxX  = op->os_Text.x - Modulo(op->os_Text.x, xscale) - 1;
        dimInfo.TxtOScan.MaxY  = op->os_Text.y - Modulo(op->os_Text.y, yscale) - 1;

	/* Set up StdOScan based on the prefs data, and quantize
	 * to the ViewResolution
	 */
        dimInfo.StdOScan       = op->os_Standard;
        dimInfo.StdOScan.MinX -= Modulo(dimInfo.StdOScan.MinX, xscale);
        dimInfo.StdOScan.MinY -= Modulo(dimInfo.StdOScan.MinY, yscale);
        dimInfo.StdOScan.MaxX -= Modulo((WORD)(dimInfo.StdOScan.MaxX+1), xscale);
        dimInfo.StdOScan.MaxY -= Modulo((WORD)(dimInfo.StdOScan.MaxY+1), yscale);

        monInfo.ViewPosition = newviewpos;

	/* We now must ensure that TxtOScan and StdOScan are legal, and
	 * we ought to do it in the same manner Intuition does.  What
	 * Intuition does is:
	 *
	 *	- ensure that the oscan rectangle fits inside MaxOScan
	 *	- ensure that the result encloses the Nominal rectangle
	 *
	 * It turns out that Intuition chops the rectangle to fit
	 * inside MaxOScan, rather than shift it.
	 */

	FitRect( &dimInfo.MaxOScan, &dimInfo.TxtOScan );
	GrowRect( &dimInfo.Nominal, &dimInfo.TxtOScan );

	FitRect( &dimInfo.MaxOScan, &dimInfo.StdOScan );
	GrowRect( &dimInfo.Nominal, &dimInfo.StdOScan );

        me->me_DimensionInfo = dimInfo;
        me->me_DisplayInfo   = dispInfo;
        me->me_MonitorInfo   = monInfo;

        if (op->os_Magic == OSCAN_MAGIC)
        {
            me->me_HStart = op->os_HStart;
            me->me_HStop  = op->os_HStop;
            me->me_VStart = op->os_VStart;
            me->me_VStop  = op->os_VStop;
            CheckSyncBounds(ed,me);
        }
    }
}


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
struct ExtPrefs *ep;

    BuildList(ed);

    if (InitPrefs(ed,&ed->ed_PrefsDefaults) && InitPrefs(ed,&ed->ed_PrefsInitial) && InitPrefs(ed,&ed->ed_PrefsWork))
    {
        ep = ed->ed_PrefsDefaults.ep_Next;
        while (ep)
        {
            ep->ep_Monitor.me_DimensionInfo.TxtOScan = ep->ep_Monitor.me_DimensionInfo.Nominal;
            ep->ep_Monitor.me_DimensionInfo.StdOScan = ep->ep_Monitor.me_DimensionInfo.Nominal;
            ShiftRect(&ep->ep_Monitor.me_DimensionInfo.MaxOScan,
                       ep->ep_Monitor.me_MonitorInfo.ViewPosition.x-ep->ep_Monitor.me_MonitorInfo.DefaultViewPosition.x,
                       ep->ep_Monitor.me_MonitorInfo.ViewPosition.y-ep->ep_Monitor.me_MonitorInfo.DefaultViewPosition.y);
            ep->ep_Monitor.me_MonitorInfo.ViewPosition = ep->ep_Monitor.me_MonitorInfo.DefaultViewPosition;

            ep = ep->ep_Next;
        }

        return(ES_NORMAL);
    }

    CleanUpEdData(ed);

    return(ES_NO_MEMORY);
}


/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
struct Node *node;

    FreeVec(ed->ed_PrefsDefaults.ep_Next);
    FreeVec(ed->ed_PrefsInitial.ep_Next);
    FreeVec(ed->ed_PrefsWork.ep_Next);

    while (node = RemHead(&ed->ed_MonitorList))
        FreeVec(node);
}


/*****************************************************************************/


VOID CopyPrefs(EdDataPtr ed, struct ExtPrefs *p1, struct ExtPrefs *p2)
{
    while (p1)
    {
        p1->ep_Monitor = p2->ep_Monitor;
        p1 = p1->ep_Next;
        p2 = p2->ep_Next;
    }
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
struct OverscanPrefs  op;
struct ExtPrefs      *ep;
struct DisplayInfo    dispInfo;
ULONG                 id;

    if (cn->cn_ID != ID_OSCN || cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&op,sizeof(struct OverscanPrefs)) == sizeof(struct OverscanPrefs))
    {
        GetDisplayInfoData(FindDisplayInfo(DEFAULT_MONITOR_ID),(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID);
        if (dispInfo.PropertyFlags & DIPF_IS_PAL)
	    id = PAL_MONITOR_ID;
        else
            id = NTSC_MONITOR_ID;

        if (MONITOR_PART(op.os_DisplayID) == DEFAULT_MONITOR_ID)
            op.os_DisplayID = (op.os_DisplayID & ~MONITOR_ID_MASK) | id;

        ep = ed->ed_PrefsWork.ep_Next;
        while (ep)
        {
            if (ep->ep_Monitor.me_ID == op.os_DisplayID)
            {
                PrefsToMonitorEntry(ed,&op,&ep->ep_Monitor);
                break;
            }
            ep = ep->ep_Next;
        }

        return(ES_NORMAL);
    }

    return(ES_IFFERROR);
}


EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
    return(ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
struct ExtPrefs      *ep;
struct OverscanPrefs  op;
struct MonitorEntry  *me;

    ep = ed->ed_PrefsWork.ep_Next;
    while (ep)
    {
        me                = &ep->ep_Monitor;
        op.os_Reserved    = 0;
        op.os_Magic       = OSCAN_MAGIC;
        op.os_HStart      = me->me_HStart;
        op.os_HStop       = me->me_HStop;
        op.os_VStart      = me->me_VStart;
        op.os_VStop       = me->me_VStop;
        op.os_DisplayID   = me->me_ID;
	op.os_ViewPos     = me->me_MonitorInfo.ViewPosition;
	op.os_Text.x      = RectWidth(&me->me_DimensionInfo.TxtOScan);
	op.os_Text.y      = RectHeight(&me->me_DimensionInfo.TxtOScan);
	op.os_Standard    = me->me_DimensionInfo.StdOScan;

        if (!PushChunk(iff,0,ID_OSCN,sizeof(struct OverscanPrefs)))
        {
            if (WriteChunkBytes(iff,&op,sizeof(struct OverscanPrefs)) == sizeof(struct OverscanPrefs))
            {
                if (!PopChunk(iff))
                {
                    ep = ep->ep_Next;
                    continue;
                }
            }
        }
        return(ES_IFFERROR);
    }

    return(ES_NORMAL);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     418
#define NW_HEIGHT    124
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN, 0},
      {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS, 0},
      {NM_ITEM, MSG_NOTHING,                EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},

    {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP, 0},
      {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
      {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED, 0},
      {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE, 0},

    {NM_TITLE,  MSG_OPTIONS_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_OPTIONS_SAVE_ICONS,     EC_SAVEICONS, CHECKIT|MENUTOGGLE},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {
    {BUTTON_KIND,   8,   107, 87,  14, MSG_SAVE_GAD,          EC_SAVE,        0},
    {BUTTON_KIND,   165, 107, 87,  14, MSG_USE_GAD,           EC_USE,         0},
    {BUTTON_KIND,   323, 107, 87,  14, MSG_CANCEL_GAD,        EC_CANCEL,      0},

    {LISTVIEW_KIND, 8,   18,  146, 80, MSG_OSCN_MONITORS_GAD, EC_MONITORLIST, 0},
    {BUTTON_KIND,   170, 66,  240, 14, MSG_OSCN_EDITTEXT_GAD, EC_EDITTEXT,    0},
    {BUTTON_KIND,   170, 83,  240, 14, MSG_OSCN_EDITGFX_GAD,  EC_EDITGFX,     0}
};

/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD               zoomSize[4];
struct DisplayInfo  dispInfo;
ULONG               scrID;
UWORD               currentCnt;
struct ExtPrefs    *ep;

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    scrID = MONITOR_PART(GetVPModeID(&ed->ed_Screen->ViewPort));
    if (!scrID)
    {
        GetDisplayInfoData(FindDisplayInfo(DEFAULT_MONITOR_ID),(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID);
        if (dispInfo.PropertyFlags & DIPF_IS_PAL)
	    scrID = PAL_MONITOR_ID;
        else
            scrID = NTSC_MONITOR_ID;
    }

    currentCnt = 0;
    ep = ed->ed_PrefsWork.ep_Next;
    while (ep)
    {
	if (MONITOR_PART(ep->ep_Monitor.me_ID) == scrID)
	{
	    ed->ed_CurrentMon = &ep->ep_Monitor;
	    break;
	}

        currentCnt++;
        ep = ep->ep_Next;
    }

    ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
    DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);

    DoPrefsGadget(ed,&EG[3],NULL,GTLV_Labels,       &ed->ed_MonitorList,
                                 GTLV_ShowSelected, NULL,
                                 GTLV_Selected,     currentCnt,
                                 GTLV_MakeVisible,  currentCnt,
                                 LAYOUTA_SPACING,   1,
                                 GTLV_ScrollWidth,  18,
                                 TAG_DONE);

    RenderGadgets(ed);

    if ((ed->ed_LastAdded)
    &&  (ed->ed_Menus = CreatePrefsMenus(ed,EM))
    &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,  NW_WIDTH,
                                            WA_InnerHeight, NW_HEIGHT,
                                            WA_MinWidth,    NW_MINWIDTH,
                                            WA_MinHeight,   NW_MINHEIGHT,
                                            WA_MaxWidth,    NW_MAXWIDTH,
                                            WA_MaxHeight,   NW_MAXHEIGHT,
                                            WA_IDCMP,       NW_IDCMP,
                                            WA_Flags,       NW_FLAGS,
                                            WA_Zoom,        zoomSize,
                                            WA_AutoAdjust,  TRUE,
                                            WA_PubScreen,   ed->ed_Screen,
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_OSCN_NAME),
                                            WA_NewLookMenus,TRUE,
                                            WA_Gadgets,     ed->ed_Gadgets,
                                            TAG_DONE)))
    {
        return(TRUE);
    }

    DisposeDisplay(ed);
    return(FALSE);
}


/*****************************************************************************/


VOID DisposeDisplay(EdDataPtr ed)
{
    if (ed->ed_Window)
    {
        ClearMenuStrip(ed->ed_Window);
        CloseWindow(ed->ed_Window);
    }
    FreeMenus(ed->ed_Menus);
    FreeGadgets(ed->ed_Gadgets);
}


/*****************************************************************************/


VOID CenterLine(EdDataPtr ed, struct Window *wp, AppStringsID id,
                UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(wp->RPort,(w-TextLength(wp->RPort,str,len)) / 2 + wp->BorderLeft + x,wp->BorderTop+y);
    Text(wp->RPort,str,len);
}


/*****************************************************************************/


VOID PutSizes(EdDataPtr ed, AppStringsID id, UWORD x, UWORD y, struct Rectangle *rect)
{
char buffer[40];

    sprintf(buffer,GetString(&ed->ed_LocaleInfo,id),RectWidth(rect),RectHeight(rect));

    Move(window->RPort,x+window->BorderLeft,y+window->BorderTop);
    Text(window->RPort,buffer,strlen(buffer));
}


/**************************,***************************************************/


VOID RenderSizes(EdDataPtr ed)
{
    if (window)
    {
        PutSizes(ed,MSG_OSCN_MINSIZE,174,28, &ed->ed_CurrentMon->me_DimensionInfo.Nominal);
        PutSizes(ed,MSG_OSCN_TEXTSIZE,174,37,&ed->ed_CurrentMon->me_DimensionInfo.TxtOScan);
        PutSizes(ed,MSG_OSCN_GFXSIZE,174,46, &ed->ed_CurrentMon->me_DimensionInfo.StdOScan);
        PutSizes(ed,MSG_OSCN_MAXSIZE,174,55, &ed->ed_CurrentMon->me_DimensionInfo.MaxOScan);
    }

    if ((RectWidth(&ed->ed_CurrentMon->me_DimensionInfo.Nominal) == RectWidth(&ed->ed_CurrentMon->me_DimensionInfo.MaxOScan))
    &&  (RectHeight(&ed->ed_CurrentMon->me_DimensionInfo.Nominal) == RectHeight(&ed->ed_CurrentMon->me_DimensionInfo.MaxOScan)))
    {
        ed->ed_EditText = DoPrefsGadget(ed,&EG[4],ed->ed_EditText,GA_Disabled,TRUE,TAG_DONE);
        ed->ed_EditGfx  = DoPrefsGadget(ed,&EG[5],ed->ed_EditGfx,GA_Disabled,TRUE,TAG_DONE);
    }
    else
    {
        ed->ed_EditText = DoPrefsGadget(ed,&EG[4],ed->ed_EditText,GA_Disabled,FALSE,TAG_DONE);
        ed->ed_EditGfx  = DoPrefsGadget(ed,&EG[5],ed->ed_EditGfx,GA_Disabled,FALSE,TAG_DONE);
    }
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}


/**************************,***************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
    SetAPen(window->RPort,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
    SetBPen(window->RPort,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    CenterLine(ed,window,MSG_OSCN_OVERSCAN_DIM_HDR,180,12,230);
    DrawBB(ed,170,18,240,44,GT_VisualInfo, ed->ed_VisualInfo,
                            GTBB_Recessed, TRUE,
                            TAG_DONE);
    RenderSizes(ed);
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
    RenderSizes(ed);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
struct Requester req;
BOOL             bool;
UWORD            icode;
struct ExtPrefs *ep;

    switch (ec)
    {
        case EC_MONITORLIST: icode = ed->ed_CurrentMsg.Code;
                             ep    = ed->ed_PrefsWork.ep_Next;
                             while (icode--)
                                 ep = ep->ep_Next;

                             ed->ed_CurrentMon = &ep->ep_Monitor;
			     break;

        case EC_EDITTEXT   :
        case EC_EDITGFX    : InitRequester(&req);
                             req.Flags = NOISYREQ;
                             bool = Request(&req,window);

                             EditOverscan(ed,ec == EC_EDITTEXT);

                             if (bool)
                                 EndRequest(&req,window);

                             break;

        default            : break;
    }

    RenderSizes(ed);
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
