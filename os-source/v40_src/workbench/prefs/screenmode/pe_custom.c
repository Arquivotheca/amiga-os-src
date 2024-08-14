
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
#include <graphics/displayinfo.h>
#include <graphics/monitor.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <prefs/screenmode.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <string.h>
#include <stdio.h>
#include "V39:src/kickstart/graphics/displayinfo_internal.h"

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
#include <clib/icon_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"


#define SysBase          ed->ed_SysBase
#define MONITOR_PART(id) ((id) & MONITOR_ID_MASK)


/*****************************************************************************/


struct DispMode
{
    struct Node dm_Node;
    ULONG       dm_ID;
    char        dm_Name[DISPLAYNAMELEN*2];
};

#define VISIBLESIZE_PID   0
#define MAXIMUMSIZE_PID   1
#define MINIMUMSIZE_PID   2
#define MAXIMUMCOLORS_PID 3
#define INTERLACE_PID     4
#define ECS_PID           5
#define GENLOCK_PID       6
#define NOGENLOCK_PID     7
#define DRAGGABLE_PID     8
#define NODRAGGABLE_PID   9
#define SPLIT_PID         10
#define SCANRATES_PID     11


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_SCRM,
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


/* private calls in gfx lib */
#pragma libcall GfxBase SetDisplayInfoData 2ee 2109805
#pragma libcall GfxBase AddDisplayInfoDataA 2e8 2109805

ULONG SetDisplayInfoData( DisplayInfoHandle handle, UBYTE * buf, ULONG size, ULONG tagID, ULONG displayID );
ULONG AddDisplayInfoDataA( DisplayInfoHandle handle, UBYTE * buf, ULONG size, ULONG tagID, ULONG displayID );


/*****************************************************************************/


VOID ProcessArgs(EdDataPtr ed, struct DiskObject *diskObj)
{
    ed->ed_PubScreenName = "Workbench";
}


/*****************************************************************************/


BOOL AddMode(EdDataPtr ed, struct List *list, ULONG modeID,
             struct DisplayInfo *dispInfo, struct DimensionInfo *dimInfo,
             struct NameInfo *nameInfo)
{
struct MonitorInfo  monInfo;
struct DispMode    *dm;
STRPTR              str1,str2;
ULONG               nomwidth, nomheight;
char                name[DISPLAYNAMELEN+1];
UWORD               len;
struct Node        *node;

    if (!(dm = AllocRemember(&ed->ed_Tracker,sizeof(struct DispMode),MEMF_CLEAR)))
	return(FALSE);

    dm->dm_ID = modeID;
    strncpy(dm->dm_Name,nameInfo->Name,DISPLAYNAMELEN);
    dm->dm_Node.ln_Name = dm->dm_Name;

    if (*dm->dm_Name == '\0')
    {
        name[0] = 0;
        if (GetDisplayInfoData(FindDisplayInfo(modeID),(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,INVALID_ID))
        {
            if ((monInfo.Mspc) && (monInfo.Mspc->ms_Node.xln_Name))
            {
                strcpy(name,monInfo.Mspc->ms_Node.xln_Name);
                len = strlen(name);
                if ((len > 8) && (Stricmp(&name[len-8],".monitor")==0))
                    name[len-8] = 0;
                strcat(name,":");
                while (len>0)
                    name[--len] = ToUpper(name[len]);
            }
        }

	nomwidth  = dimInfo->Nominal.MaxX - dimInfo->Nominal.MinX + 1;
	nomheight = dimInfo->Nominal.MaxY - dimInfo->Nominal.MinY + 1;

	if (dispInfo->PropertyFlags & DIPF_IS_LACE)
        {
            str1 = " ";
	    str2 = GetString(&ed->ed_LocaleInfo,MSG_SM_INTERLACE_PROP);
	}
	else
	{
	    str1 = "";
	    str2 = "";
        }

        /* we need a length limited sprintf()! */
	sprintf(dm->dm_Name,"%s%lu x %lu%s%s",name,nomwidth,nomheight,str1,str2);
    }

    node = list->lh_Head;
    while (node->ln_Succ)
    {
        if (Stricmp(node->ln_Name,dm->dm_Node.ln_Name) >= 0)
            break;
        node = node->ln_Succ;
    }
    Insert(list,(struct Node *)dm,node->ln_Pred);

    return(TRUE);
}


/*****************************************************************************/


EdStatus BuildList(EdDataPtr ed, struct List *list)
{
ULONG                modeID;
struct NameInfo      nameInfo;
struct DimensionInfo dimInfo;
struct DisplayInfo   dispInfo;
DisplayInfoHandle    dh;

    NewList(list);

    modeID = INVALID_ID;
    while ((modeID = NextDisplayInfo(modeID)) != INVALID_ID)
    {
	if (MONITOR_PART(modeID) != DEFAULT_MONITOR_ID)
	{
            dh = FindDisplayInfo(modeID);

            nameInfo.Name[0] = '\0';
            GetDisplayInfoData(dh,(APTR)&nameInfo,sizeof(struct NameInfo),DTAG_NAME,INVALID_ID);

            if (GetDisplayInfoData(dh,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID)
	    &&  GetDisplayInfoData(dh,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,INVALID_ID))
	    {
                if ((dispInfo.PropertyFlags & DIPF_IS_WB) && (!dispInfo.NotAvailable))
                {
		    if (!AddMode(ed,list,modeID,&dispInfo,&dimInfo,&nameInfo))
		    {
			return(ES_NO_MEMORY);
		    }
		}
	    }
	}
    }

    return(ES_NORMAL);
}


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
UWORD i;

    for (i=0; i<10; i++)
        ed->ed_PropNodes[i].ln_Name = GetString(&ed->ed_LocaleInfo,i+MSG_SM_VISIBLESIZE_PROP);

    ed->ed_PropNodes[VISIBLESIZE_PID].ln_Name   = ed->ed_VisibleSize;
    ed->ed_PropNodes[MAXIMUMSIZE_PID].ln_Name   = ed->ed_MaximumSize;
    ed->ed_PropNodes[MINIMUMSIZE_PID].ln_Name   = ed->ed_MinimumSize;
    ed->ed_PropNodes[MAXIMUMCOLORS_PID].ln_Name = ed->ed_MaximumColors;
    ed->ed_PropNodes[SCANRATES_PID].ln_Name     = ed->ed_ScanRates;

    if (GfxBase->DisplayFlags & NTSC)
        ed->ed_PrefsDefaults.smp_DisplayID = HIRES_KEY | NTSC_MONITOR_ID;
    else if (GfxBase->DisplayFlags & PAL)
        ed->ed_PrefsDefaults.smp_DisplayID = HIRES_KEY | PAL_MONITOR_ID;

    ed->ed_PrefsDefaults.smp_Width     = -1;
    ed->ed_PrefsDefaults.smp_Height    = -1;
    ed->ed_PrefsDefaults.smp_Depth     = 2;
    ed->ed_PrefsDefaults.smp_Control   = 1;

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    NewList(&ed->ed_Properties);

    return(BuildList(ed,&ed->ed_DisplayModes));
}


/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreeRemember(&ed->ed_Tracker,TRUE);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (cn->cn_ID != ID_SCRM || cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct ScreenModePrefs)) == sizeof(struct ScreenModePrefs))
    {
        /* we do this for compatibility with <V39 prefs files */
        if (MONITOR_PART(ed->ed_PrefsWork.smp_DisplayID) == DEFAULT_MONITOR_ID)
        {
            if (GfxBase->DisplayFlags & NTSC)
                ed->ed_PrefsWork.smp_DisplayID |= NTSC_MONITOR_ID;
            else if (GfxBase->DisplayFlags & PAL)
                ed->ed_PrefsWork.smp_DisplayID |= PAL_MONITOR_ID;
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
    if (!PushChunk(iff,0,ID_SCRM,sizeof(struct ScreenModePrefs)))
        if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct ScreenModePrefs)) == sizeof(struct ScreenModePrefs))
            if (!PopChunk(iff))
                return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


ULONG __stdargs DepthToColors(struct Gadget *gadget, WORD level)
{
    return ((ULONG)(1L << level));
}


/*****************************************************************************/


#define NW_WIDTH     596
#define NW_HEIGHT    182
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP,    0},
      {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN,   0},
      {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS, 0},
      {NM_ITEM, MSG_NOTHING,                EC_NOP,    0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},

    {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP,      0},
      {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
      {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED,0},
      {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE,  0},

    {NM_TITLE,  MSG_OPTIONS_MENU,           EC_NOP,       0},
      {NM_ITEM, MSG_OPTIONS_SAVE_ICONS,     EC_SAVEICONS, CHECKIT|MENUTOGGLE},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {
    {BUTTON_KIND,   8,   163,  87,  14, MSG_SAVE_GAD,           EC_SAVE,     0},
    {BUTTON_KIND,   254, 163,  87,  14, MSG_USE_GAD,            EC_USE,      0},
    {BUTTON_KIND,   501, 163,  87,  14, MSG_CANCEL_GAD,         EC_CANCEL,   0},

    {LISTVIEW_KIND, 8,   18,  296, 78, MSG_SM_DISPLAY_MODE_GAD, EC_MODELIST,  0},
    {INTEGER_KIND,  106, 95,  68,  14, MSG_SM_WIDTH_GAD,        EC_WIDTH,     0},
    {INTEGER_KIND,  106, 111, 68,  14, MSG_SM_HEIGHT_GAD,       EC_HEIGHT,    0},
    {CHECKBOX_KIND, 182, 97,  90,  11, MSG_SM_DEFAULT_GAD,      EC_DEFWIDTH,  PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 182, 113, 90,  11, MSG_SM_DEFAULT_GAD,      EC_DEFHEIGHT, PLACETEXT_RIGHT},
    {SLIDER_KIND,   138, 129, 166, 11, MSG_SM_COLORS_GAD,       EC_COLORS,    0},
    {CHECKBOX_KIND, 106, 143, 90,  11, MSG_SM_AUTOSCROLL_GAD,   EC_AUTOSCROLL,0}
};


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
    DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);

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
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_SM_NAME),
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


VOID CenterLine(EdDataPtr ed, struct RastPort *rp, AppStringsID id,
                UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(rp,(w-TextLength(rp,str,len)) / 2 + window->BorderLeft + x,
            window->BorderTop+y);
    Text(rp,str,len);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID RenderProps(EdDataPtr ed)
{
struct DispMode *node;
UWORD            y;

    y    = 30+window->BorderTop;
    node = (struct DispMode *)ed->ed_Properties.lh_Head;
    while (y < 150)
    {
        Move(window->RPort,325+window->BorderLeft,y);

        if (node->dm_Node.ln_Succ)
        {
            if (node->dm_Node.ln_Name)
                Text(window->RPort,node->dm_Node.ln_Name,strlen(node->dm_Node.ln_Name));
            node = (struct DispMode *)node->dm_Node.ln_Succ;
        }

        if (window->RPort->cp_x < 585+window->BorderLeft)
        {
            SetAPen(window->RPort,0);
            RectFill(window->RPort,window->RPort->cp_x,    y-6,
                                   585+window->BorderLeft, y+1);
            SetAPen(window->RPort,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
        }

        y += 9;
    }
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
    SetAPen(window->RPort,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
    SetBPen(window->RPort,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    SetDrMd(window->RPort,JAM2);

    CenterLine(ed,window->RPort,MSG_SM_PROPS_GAD,318,12,270);
    DrawBB(ed,318,18,270,135,GTBB_Recessed, TRUE,
                             GT_VisualInfo, ed->ed_VisualInfo,
                             TAG_DONE);
    RenderProps(ed);
}


/*****************************************************************************/


VOID DoRenderGadgets(EdDataPtr ed, BOOL full)
{
struct DispMode      *node;
struct DimensionInfo  dimInfo;
struct DisplayInfo    dispInfo;
struct MonitorInfo    monInfo;
struct List          *list;
WORD                  minWidth,maxWidth,defWidth,curWidth;
WORD                  minHeight,maxHeight,defHeight,curHeight;
WORD                  maxDepth;
WORD                  nodeNum;
ULONG                 props;
DisplayInfoHandle     dh;
struct MonitorSpec   *mspec;
ULONG                 vscan;
ULONG                 hscan;
ULONG                 hscandec;
ULONG                 visTag;
STRPTR                decPoint;
struct Locale        *locale;

    nodeNum = 0;
    node    = (struct DispMode *)ed->ed_DisplayModes.lh_Head;
    while (node->dm_Node.ln_Succ && (node->dm_ID != ed->ed_PrefsWork.smp_DisplayID))
    {
        node = (struct DispMode *)node->dm_Node.ln_Succ;
        nodeNum++;
    }

    if (!node->dm_Node.ln_Succ)
    {
        nodeNum                        = 0;
        node                           = (struct DispMode *)ed->ed_DisplayModes.lh_Head;
        ed->ed_PrefsWork.smp_DisplayID = node->dm_ID;
    }

    dh = FindDisplayInfo(node->dm_ID);
    GetDisplayInfoData(dh,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,INVALID_ID);
    GetDisplayInfoData(dh,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID);
    GetDisplayInfoData(dh,(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,INVALID_ID);

    defWidth    = dimInfo.TxtOScan.MaxX - dimInfo.TxtOScan.MinX + 1;
    defHeight   = dimInfo.TxtOScan.MaxY - dimInfo.TxtOScan.MinY + 1;
    minWidth    = dimInfo.MinRasterWidth;
    maxWidth    = dimInfo.MaxRasterWidth;
    minHeight   = dimInfo.MinRasterHeight;
    maxHeight   = dimInfo.MaxRasterHeight;
    maxDepth    = dimInfo.MaxDepth;
    curWidth    = defWidth;
    curHeight   = defHeight;

    if (ed->ed_PrefsWork.smp_Width != -1)
        curWidth = ed->ed_PrefsWork.smp_Width;

    if (ed->ed_PrefsWork.smp_Height != -1)
        curHeight = ed->ed_PrefsWork.smp_Height;

    if (minWidth < 640)
        minWidth = 640;

    if (minHeight < 200)
        minHeight = 200;

    if (curWidth < minWidth)
        curWidth = minWidth;

    if (curWidth > maxWidth)
        curWidth = maxWidth;

    if (curHeight < minHeight)
        curHeight = minHeight;

    if (curHeight > maxHeight)
        curHeight = maxHeight;

    if (ed->ed_PrefsWork.smp_Depth > maxDepth)
        ed->ed_PrefsWork.smp_Depth = maxDepth;

    if ((node != ed->ed_LastMode) || (!ed->ed_ModeList))
    {
        if (ed->ed_ModeList)
        {
            visTag = GTLV_MakeVisible;
            if (!full)
                visTag = TAG_IGNORE;

            DoPrefsGadget(ed,&EG[3],ed->ed_ModeList,GTLV_ShowSelected, NULL,
                                                    GTLV_Labels,       &ed->ed_DisplayModes,
                                                    LAYOUTA_SPACING,   1,
                                                    GTLV_ScrollWidth,  18,
                                                    GTLV_Selected,     nodeNum,
                                                    visTag,            nodeNum,
                                                    TAG_DONE);
        }
        else
        {
            ed->ed_ModeList = DoPrefsGadget(ed,&EG[3],ed->ed_ModeList,
                                                      GTLV_ShowSelected, NULL,
                                                      GTLV_Labels,       &ed->ed_DisplayModes,
                                                      LAYOUTA_SPACING,   1,
                                                      GTLV_ScrollWidth,  18,
                                                      GTLV_Selected,     nodeNum,
                                                      GTLV_MakeVisible,  nodeNum,
                                                      TAG_DONE);
        }

        ed->ed_LastMode = node;

        sprintf(ed->ed_VisibleSize,  GetString(&ed->ed_LocaleInfo,MSG_SM_VISIBLESIZE_PROP),(ULONG)defWidth,(ULONG)defHeight);
        sprintf(ed->ed_MaximumSize,  GetString(&ed->ed_LocaleInfo,MSG_SM_MAXSIZE_PROP),(ULONG)maxWidth,(ULONG)maxHeight);
        sprintf(ed->ed_MinimumSize,  GetString(&ed->ed_LocaleInfo,MSG_SM_MINSIZE_PROP),(ULONG)minWidth,(ULONG)minHeight);
        sprintf(ed->ed_MaximumColors,GetString(&ed->ed_LocaleInfo,MSG_SM_MAXCOLORS_PROP),(ULONG)1<<maxDepth);

        list  = &ed->ed_Properties;
        props = dispInfo.PropertyFlags;
        NewList(list);
        AddTail(list,&ed->ed_PropNodes[VISIBLESIZE_PID]);
        AddTail(list,&ed->ed_PropNodes[MINIMUMSIZE_PID]);
        AddTail(list,&ed->ed_PropNodes[MAXIMUMSIZE_PID]);
        AddTail(list,&ed->ed_PropNodes[MAXIMUMCOLORS_PID]);
        AddTail(list,&ed->ed_PropNodes[SPLIT_PID]);

        if (props & DIPF_IS_LACE)
            AddTail(list,&ed->ed_PropNodes[INTERLACE_PID]);

        if (props & DIPF_IS_ECS)
            AddTail(list,&ed->ed_PropNodes[ECS_PID]);

        if (props & DIPF_IS_GENLOCK)
            AddTail(list,&ed->ed_PropNodes[GENLOCK_PID]);
        else
            AddTail(list,&ed->ed_PropNodes[NOGENLOCK_PID]);

        if (props & DIPF_IS_DRAGGABLE)
            AddTail(list,&ed->ed_PropNodes[DRAGGABLE_PID]);
        else
            AddTail(list,&ed->ed_PropNodes[NODRAGGABLE_PID]);

        decPoint = ".";
        locale   = NULL;
        if (LocaleBase)
        {
            locale = OpenLocale(NULL);
            decPoint = locale->loc_DecimalPoint;
        }

        mspec = monInfo.Mspc;
        vscan = 1000000000 / ((ULONG)mspec->total_colorclocks * 280 * (ULONG)mspec->total_rows);
        hscan = vscan * mspec->total_rows;
        hscandec = (hscan % 1000) / 10;
        hscan = hscan / 1000;
        sprintf(ed->ed_ScanRates,"%luHz, %lu%s%02lukHz",vscan,hscan,decPoint,hscandec);
        AddTail(list,&ed->ed_PropNodes[SCANRATES_PID]);

        if (locale)
            CloseLocale(locale);

        if (ed->ed_Window)
            RenderProps(ed);
    }

    ed->ed_Width = DoPrefsGadget(ed,&EG[4],ed->ed_Width,
                                           GTIN_MaxChars, 6,
                                           GTIN_Number,   curWidth,
                                           GA_Disabled,   (ed->ed_PrefsWork.smp_Width == -1),
                                           TAG_DONE);

    ed->ed_Height = DoPrefsGadget(ed,&EG[5],ed->ed_Height,
                                            GTIN_MaxChars, 6,
                                            GTIN_Number,   curHeight,
                                            GA_Disabled,   (ed->ed_PrefsWork.smp_Height == -1),
                                            TAG_DONE);

    ed->ed_DefWidth = DoPrefsGadget(ed,&EG[6],ed->ed_DefWidth,
                                              GTCB_Checked,(ed->ed_PrefsWork.smp_Width == -1),
                                              TAG_DONE);

    ed->ed_DefHeight = DoPrefsGadget(ed,&EG[7],ed->ed_DefHeight,
                                               GTCB_Checked,(ed->ed_PrefsWork.smp_Height == -1),
                                               TAG_DONE);

    ed->ed_Colors = DoPrefsGadget(ed,&EG[8],ed->ed_Colors,
                                            GTSL_MaxLevelLen, 10,
                                            GTSL_LevelFormat, "%3lu",
                                            GTSL_DispFunc,    (ULONG)DepthToColors,
                                            GTSL_Level,       (WORD)ed->ed_PrefsWork.smp_Depth,
                                            GTSL_Min,         1,
                                            GTSL_Max,         maxDepth,
                                            GA_Immediate,     TRUE,
                                            TAG_DONE);

    ed->ed_AutoScroll = DoPrefsGadget(ed,&EG[9],ed->ed_AutoScroll,
                                                GTCB_Checked,ed->ed_PrefsWork.smp_Control & 1,
                                                TAG_DONE);
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
    DoRenderGadgets(ed,TRUE);
}


/*****************************************************************************/


VOID ProcessTextGadget(EdDataPtr ed, EdCommand ec, BOOL screenStuff)
{
LONG                  num;
struct Gadget        *act;
BOOL                  beep;
struct DimensionInfo  dimInfo;
WORD                  maxWidth, maxHeight;
WORD                  minWidth, minHeight;

    GetDisplayInfoData(FindDisplayInfo(ed->ed_PrefsWork.smp_DisplayID),(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,INVALID_ID);

    minWidth  = dimInfo.MinRasterWidth;
    maxWidth  = dimInfo.MaxRasterWidth;
    minHeight = dimInfo.MinRasterHeight;
    maxHeight = dimInfo.MaxRasterHeight;

    if (minWidth < 640)
        minWidth = 640;

    if (minHeight < 200)
        minHeight = 200;

    beep = FALSE;
    act  = NULL;
    switch (ec)
    {
        case EC_WIDTH      : num = ((struct StringInfo *)ed->ed_Width->SpecialInfo)->LongInt;
                             if (num < minWidth)
                             {
                                 num  = minWidth;
                                 act  = ed->ed_Width;
                                 beep = TRUE;
                             }
                             else if (num > maxWidth)
                             {
                                 num  = maxWidth;
                                 act  = ed->ed_Width;
                                 beep = TRUE;
                             }
                             else if (ed->ed_PrefsWork.smp_Height != -1)
                             {
                                 act = ed->ed_Height;
                             }

                             if (screenStuff)
                             {
                                 ed->ed_PrefsWork.smp_Width = num;
                                 if (beep)
                                 {
                                     SetGadgetAttr(ed,ed->ed_Width,GTIN_Number,num,
                                                                   TAG_DONE);
                                 }
                             }
                             else if (ed->ed_PrefsWork.smp_Width != -1)
                             {
                                 ed->ed_PrefsWork.smp_Width = num;
                             }
                             break;

        case EC_HEIGHT     : num = ((struct StringInfo *)ed->ed_Height->SpecialInfo)->LongInt;
                             if (num < minHeight)
                             {
                                 num  = minHeight;
                                 act  = ed->ed_Height;
                                 beep = TRUE;
                             }

                             else if (num > maxHeight)
                             {
                                 num  = maxHeight;
                                 act  = ed->ed_Height;
                                 beep = TRUE;
                             }
                             else if (ed->ed_PrefsWork.smp_Width != -1)
                             {
                                 act = ed->ed_Width;
                             }

                             if (screenStuff)
                             {
                                 ed->ed_PrefsWork.smp_Height = num;
                                 if (beep)
                                 {
                                     SetGadgetAttr(ed,ed->ed_Height,GTIN_Number,num,
                                                                    TAG_DONE);
                                 }
                             }
                             else if (ed->ed_PrefsWork.smp_Height != -1)
                             {
                                 ed->ed_PrefsWork.smp_Height = num;
                             }
                             break;
    }

    if (screenStuff)
    {
        if (act)
            ActivateGadget(act,window,NULL);

        if (beep)
            DisplayBeep(window->WScreen);
    }
}


/*****************************************************************************/


VOID SyncTextGadgets(EdDataPtr ed)
{
    ProcessTextGadget(ed,EC_WIDTH,FALSE);
    ProcessTextGadget(ed,EC_HEIGHT,FALSE);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD            icode;
struct DispMode *node;
struct Gadget   *gadget;

    icode  = ed->ed_CurrentMsg.Code;
    gadget = ed->ed_CurrentMsg.IAddress;

    switch (ec)
    {
        case EC_MODELIST   : node = (struct DispMode *)ed->ed_DisplayModes.lh_Head;
                             while (icode--)
                                 node = (struct DispMode *)node->dm_Node.ln_Succ;
                             ed->ed_PrefsWork.smp_DisplayID = node->dm_ID;
                             DoRenderGadgets(ed,FALSE);
                             break;

        case EC_COLORS     : ed->ed_PrefsWork.smp_Depth = icode;
                             break;

        case EC_WIDTH      :
        case EC_HEIGHT     : ProcessTextGadget(ed,ec,TRUE);
                             break;

        case EC_DEFWIDTH   : if (SELECTED & gadget->Flags)
                             {
                                 ed->ed_PrefsWork.smp_Width = -1;
                                 DoRenderGadgets(ed,FALSE);
                             }
                             else
                             {
                                 ed->ed_PrefsWork.smp_Width = ((struct StringInfo *) (ed->ed_Width->SpecialInfo))->LongInt;
                                 SetGadgetAttr(ed,ed->ed_Width,GA_Disabled,FALSE,
                                                               TAG_DONE);
                                 ActivateGadget(ed->ed_Width,window,NULL);
                             }
                             break;

        case EC_DEFHEIGHT  : if (SELECTED & gadget->Flags)
                             {
                                 ed->ed_PrefsWork.smp_Height = -1;
                                 DoRenderGadgets(ed,FALSE);
                             }
                             else
                             {
                                 ed->ed_PrefsWork.smp_Height = ((struct StringInfo *) (ed->ed_Height->SpecialInfo))->LongInt;
                                 SetGadgetAttr(ed,ed->ed_Height,GA_Disabled,FALSE,
                                                                TAG_DONE);
                                 ActivateGadget(ed->ed_Height,window,NULL);
                             }
                             break;

        case EC_AUTOSCROLL : ed->ed_PrefsWork.smp_Control &= ~1;
                             if (ed->ed_AutoScroll->Flags & SELECTED)
                             {
                                 ed->ed_PrefsWork.smp_Control |= 1;
                             }
                             break;

        default            : break;
    }
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
