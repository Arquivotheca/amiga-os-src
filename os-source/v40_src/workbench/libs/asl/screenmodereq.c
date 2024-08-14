
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/screens.h>
#include <libraries/gadtools.h>
#include <string.h>
#include <math.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "asl.h"
#include "aslbase.h"
#include "screenmodereq.h"
#include "aslutils.h"
#include "asllists.h"
#include "texttable.h"
#include "layout.h"
#include "requtils.h"


/*****************************************************************************/


#define LocaleBase ri->ri_LocaleInfo.li_LocaleBase
#define catalog    ri->ri_LocaleInfo.li_Catalog


/*****************************************************************************/


typedef enum SMCommands
{
    SM_NOP,

    SM_OK,
    SM_CANCEL,

    SM_DMLIST,
    SM_PROPLIST,
    SM_WIDTH,
    SM_HEIGHT,
    SM_COLORS,
    SM_HAMCOLORS,
    SM_OVERSCAN,
    SM_INFO,
    SM_AUTOSCROLL,

    SM_PREVMODE,
    SM_NEXTMODE,
    SM_RESTORE
};


/*****************************************************************************/


/* Property ID's */
#define INTERLACE_PID   0
#define HAM_PID         1
#define EHB_PID         2
#define ECS_PID         3
#define GENLOCK_PID     4
#define NOGENLOCK_PID   5
#define WORKBENCH_PID   6
#define NOWORKBENCH_PID 7
#define DRAGGABLE_PID   8
#define NODRAGGABLE_PID 9
#define DUALPF_PID      10
#define PF2PRI_PID      11
#define SCANRATES_PID   12

#define MONITOR_PART(id) ((id) & MONITOR_ID_MASK)

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<=(b)?(a):(b))
#endif


static struct ASLMenu far AM[] =
{
    {NM_TITLE,  SM_NOP,      MSG_ASL_CONTROL_MENU},
      {NM_ITEM, SM_PREVMODE, MSG_ASL_CONTROL_LASTMODE},
      {NM_ITEM, SM_NEXTMODE, MSG_ASL_CONTROL_NEXTMODE},
      {NM_ITEM, SM_NOP,      MSG_BARLABEL},
      {NM_ITEM, SM_INFO,     MSG_ASL_CONTROL_PROPS},
      {NM_ITEM, SM_RESTORE,  MSG_ASL_CONTROL_RESTORE},
      {NM_ITEM, SM_NOP,      MSG_BARLABEL},
      {NM_ITEM, SM_OK,       MSG_ASL_CONTROL_OK},
      {NM_ITEM, SM_CANCEL,   MSG_ASL_CONTROL_CANCEL},

    {NM_END,    SM_NOP,      MSG_BARLABEL}
};


/*****************************************************************************/


#define	POSGAD		1
#define	NEGGAD		2
#define	OVERSCANTYPEGAD	4
#define	WIDTHGAD	5
#define	HEIGHTGAD	6
#define	NUMCOLORSGAD	7
#define	COLORSGAD	8
#define	AUTOSCROLLGAD	9
#define	MODELISTGAD	10

static struct ASLGadget far AG[] =
{
    {HGROUP_KIND,   SM_NOP,       0, MSG_NOTHING,               {  4,-16, -8, 14}, 1,  0},
    {BUTTON_KIND,   SM_OK,        0, MSG_ASL_OK_GAD,            {  0,  0,  0,  0}, 1,  0, },
    {BUTTON_KIND,   SM_CANCEL,    0, MSG_ASL_CANCEL_GAD,        {  0,  0,  0,  0}, 1,  0, },

    {VGROUP_KIND,   SM_NOP,       0, MSG_NOTHING,               {  4,-86, -8, 74}, 2,  0},
    {CYCLE_KIND,    SM_OVERSCAN,  0, MSG_ASL_SM_OVERSCAN_GAD,   {  0,  0,  0,  0}, 2,  0,  },
    {INTEGER_KIND,  SM_WIDTH,     0, MSG_ASL_SM_WIDTH_GAD,      {  0,  0, 50,  0}, 2,  0,  },
    {INTEGER_KIND,  SM_HEIGHT,    0, MSG_ASL_SM_HEIGHT_GAD,     {  0,  0, 50,  0}, 2, -1,  },
    {TEXT_KIND,     SM_COLORS,    0, MSG_ASL_SM_COLORS_GAD,     {  0,  0, 50,  0}, 2,  0,  },
    {SLIDER_KIND,   SM_COLORS,    0, MSG_NOTHING,               {  0,  0,  0,  0}, 2, -1,  },
    {CHECKBOX_KIND, SM_AUTOSCROLL,0, MSG_ASL_SM_AUTOSCROLL_GAD, {  0,  0,  0,  0}, 2,  0,  },

    {LISTVIEW_KIND, SM_DMLIST,    0, MSG_NOTHING,               {  4,  2, -8,-90}, 3,  0, },

    {END_KIND,},
};


/*****************************************************************************/


static VOID FreeDisplayModes(struct ExtSMReq *smr)
{
struct DisplayMode *node;

    while (node = (struct DisplayMode *)RemHead(&smr->sm_DisplayModes))
    {
        if (((node->dm_DimensionInfo.Header.DisplayID) & 0xFFFF0000) != 0xFFFF0000)
        {
            FreeVec(node);
        }
        else if (smr->sm_CustomSMList)
        {
            AddTail(smr->sm_CustomSMList,(struct Node *)node);
        }
    }
}


/*****************************************************************************/


static BOOL GetDisplayModes(struct ExtSMReq *smr)
{
struct List          *list = &smr->sm_DisplayModes;
struct DisplayMode   *node;
ULONG                 modeID;
struct NameInfo       nameInfo;
struct DisplayInfo    dispInfo;
struct DimensionInfo  dimInfo;
struct MonitorInfo    monInfo;
char                  buffer[128];
char                  name[DISPLAYNAMELEN+1];
UWORD                 len;
DisplayInfoHandle     dh;

    if (smr->sm_CustomSMList != NULL)
        while (node = (struct DisplayMode *)RemHead(smr->sm_CustomSMList))
            EnqueueAlpha(list,(struct Node *)node);

    modeID = INVALID_ID;
    while ((modeID = NextDisplayInfo(modeID)) != INVALID_ID)
    {
        if (MONITOR_PART(modeID))    /* ignore "default" monitor */
        {
	    dh = FindDisplayInfo (modeID);
            if (GetDisplayInfoData(dh,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,INVALID_ID))
            {
                if (!(dispInfo.NotAvailable))
                {
                    if (GetDisplayInfoData(dh,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,INVALID_ID))
                    {
                        if ((dispInfo.PropertyFlags & smr->sm_PropertyMask) == (smr->sm_PropertyFlags & smr->sm_PropertyMask))
                        {
                            if (   (smr->sm_MinWidth  <= dimInfo.MaxRasterWidth)
                                && (smr->sm_MaxWidth  >= dimInfo.MinRasterWidth)
                                && (smr->sm_MinHeight <= dimInfo.MaxRasterHeight)
                                && (smr->sm_MaxHeight >= dimInfo.MinRasterHeight)
                                && (smr->sm_MinDepth  <= dimInfo.MaxDepth))
                            {
                                if ((!smr->sm_FilterFunc) || (CallHookPkt(smr->sm_FilterFunc,PUBLIC_SMR(smr),(APTR)modeID)))
                                {
                                    /* Get name or make one if no name available */
                                    if (GetDisplayInfoData(dh,(APTR)&nameInfo,sizeof(struct NameInfo),DTAG_NAME,INVALID_ID))
                                    {
                                        node = AllocNamedNode(sizeof(struct DisplayMode),nameInfo.Name);
                                    }
                                    else
                                    {
                                        name[0] = 0;
                                        if (GetDisplayInfoData(NULL,(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,modeID))
                                        {
                                            if ((monInfo.Mspc) && (monInfo.Mspc->ms_Node.xln_Name))
                                            {
                                                strcpy(name,monInfo.Mspc->ms_Node.xln_Name);
                                                StripExtension(name,".monitor");
                                                strcat(name,":");
                                                len = strlen(name);
                                                while (len>0)
                                                    name[--len] = ToUpper(name[len]);
                                            }
                                        }

                                        sprintf(buffer,"%s%lu x %lu %s%s%s",
                                                name,
                                                (dimInfo.Nominal.MaxX - dimInfo.Nominal.MinX + 1),
                                                (dimInfo.Nominal.MaxY - dimInfo.Nominal.MinY + 1),
                                                (dispInfo.PropertyFlags & DIPF_IS_HAM) ? "HAM " :
                                                (dispInfo.PropertyFlags & DIPF_IS_EXTRAHALFBRITE) ? "EHB " : "",
                                                (dispInfo.PropertyFlags & DIPF_IS_PF2PRI) ? "DPF2 " :
                                                (dispInfo.PropertyFlags & DIPF_IS_DUALPF) ? "DPF " : "",
                                                (dispInfo.PropertyFlags & DIPF_IS_LACE) ? GetString(&smr->sm_ReqInfo.ri_LocaleInfo,MSG_ASL_SM_INTERLACED) : "" , "");

                                        node = AllocNamedNode(sizeof(struct DisplayMode),buffer);
                                    }

                                    if (!node)
                                    {
                                        FreeDisplayModes(smr);
                                        return(FALSE);
                                    }

                                    node->dm_PropertyFlags = dispInfo.PropertyFlags;
                                    node->dm_DimensionInfo = dimInfo;
                                    EnqueueAlpha(list,(struct Node *)node);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return(TRUE);
}


/*****************************************************************************/


static struct DisplayMode *GetCurrentMode(struct ExtSMReq *smr, ULONG displayID)
{
struct DisplayMode *node;
ULONG               foundID;

    node = (struct DisplayMode *)smr->sm_DisplayModes.lh_Head;
    while (node->dm_Node.ln_Succ)
    {
        foundID = node->dm_DimensionInfo.Header.DisplayID;

        if (MONITOR_PART(displayID))
        {
            if (displayID == foundID)
                return(node);
        }
        else if (displayID == (foundID & ~MONITOR_ID_MASK))
        {
            return(node);
        }

        node = (struct DisplayMode *)node->dm_Node.ln_Succ;
    }

    return((struct DisplayMode *)smr->sm_DisplayModes.lh_Head);
}


/*****************************************************************************/


static VOID UpdatePropList(struct ExtSMReq *smr)
{
struct TagItem      tags[2];
struct List        *list = &smr->sm_Properties;
ULONG               properties;
ULONG               vscan;
ULONG               hscan;
ULONG               hscandec;
STRPTR              decPoint;
struct MonitorSpec *mspec;
struct DisplayMode *node;
struct MonitorInfo  monInfo;
DisplayInfoHandle   dh;

    if (smr->sm_InfoWindow)
    {
        tags[0].ti_Tag  = GTLV_Labels;
        tags[0].ti_Data = ~0;
        tags[1].ti_Tag  = TAG_DONE;
        GT_SetGadgetAttrsA(smr->sm_PropertyListGad,smr->sm_InfoWindow,NULL,tags);

        NewList(list);

        node = smr->sm_CurrentMode;
        properties = node->dm_PropertyFlags;

        if (properties & DIPF_IS_HAM)
            AddTail(list,&smr->sm_PropertyNodes[HAM_PID]);

        if (properties & DIPF_IS_EXTRAHALFBRITE)
            AddTail(list,&smr->sm_PropertyNodes[EHB_PID]);

        if (properties & DIPF_IS_LACE)
            AddTail(list,&smr->sm_PropertyNodes[INTERLACE_PID]);

        if (properties & DIPF_IS_ECS)
            AddTail(list,&smr->sm_PropertyNodes[ECS_PID]);

        if (properties & DIPF_IS_WB)
            AddTail(list,&smr->sm_PropertyNodes[WORKBENCH_PID]);
        else
            AddTail(list,&smr->sm_PropertyNodes[NOWORKBENCH_PID]);

        if (properties & DIPF_IS_GENLOCK)
            AddTail(list,&smr->sm_PropertyNodes[GENLOCK_PID]);
        else
            AddTail(list,&smr->sm_PropertyNodes[NOGENLOCK_PID]);

        if (properties & DIPF_IS_DRAGGABLE)
            AddTail(list,&smr->sm_PropertyNodes[DRAGGABLE_PID]);
        else
            AddTail(list,&smr->sm_PropertyNodes[NODRAGGABLE_PID]);

        if (properties & DIPF_IS_PF2PRI)
            AddTail(list,&smr->sm_PropertyNodes[PF2PRI_PID]);
        else if (properties & DIPF_IS_DUALPF)
            AddTail(list,&smr->sm_PropertyNodes[DUALPF_PID]);

        decPoint = ".";
        if (smr->sm_ReqInfo.ri_LocaleInfo.li_Locale)
            decPoint = smr->sm_ReqInfo.ri_LocaleInfo.li_Locale->loc_DecimalPoint;

        if (((node->dm_DimensionInfo.Header.DisplayID) & 0xFFFF0000) != 0xFFFF0000)
        {
            if (dh = FindDisplayInfo(node->dm_DimensionInfo.Header.DisplayID))
            {
                if (GetDisplayInfoData(dh,(APTR)&monInfo,sizeof(struct MonitorInfo),DTAG_MNTR,INVALID_ID))
                {
                    mspec = monInfo.Mspc;
                    vscan = 1000000000 / ((ULONG)mspec->total_colorclocks * 280 * (ULONG)mspec->total_rows);
                    hscan = vscan * mspec->total_rows;
                    hscandec = (hscan % 1000) / 10;
                    hscan = hscan / 1000;
                    sprintf(smr->sm_ScanRates,"%luHz, %lu%s%02lukHz",vscan,hscan,decPoint,hscandec);
                    AddTail(list,&smr->sm_PropertyNodes[SCANRATES_PID]);
                }
            }
        }

        tags[0].ti_Data = (ULONG)list;
        GT_SetGadgetAttrsA(smr->sm_PropertyListGad,smr->sm_InfoWindow,NULL,tags);
    }
}


/*****************************************************************************/


static VOID CloseInfoWindow(struct ExtSMReq *smr)
{
    if (smr->sm_InfoWindow)
    {
        smr->sm_InfoCoords.Left   = smr->sm_InfoWindow->LeftEdge-smr->sm_ReqInfo.ri_Window->LeftEdge;
        smr->sm_InfoCoords.Top    = smr->sm_InfoWindow->TopEdge-smr->sm_ReqInfo.ri_Window->TopEdge;
        smr->sm_InfoCoords.Width  = smr->sm_ReqInfo.ri_Window->Width;
        smr->sm_InfoCoords.Height = smr->sm_ReqInfo.ri_Window->Height;

        AslCloseWindow(smr->sm_InfoWindow,TRUE);
        FreeGadgets(smr->sm_InfoGadgets);

        smr->sm_InfoWindow  = NULL;
        smr->sm_InfoGadgets = NULL;
    }
}


/*****************************************************************************/


static VOID OpenInfoWindow(struct ExtSMReq *smr)
{
struct Gadget    *gad;
struct NewGadget  ng;
struct ReqInfo   *ri;
struct TagItem    tags[3];

    if (smr->sm_InfoWindow)
    {
        WindowToFront(smr->sm_InfoWindow);
    }
    else
    {
        ri = &smr->sm_ReqInfo;
        if (smr->sm_InfoWindow = AslOpenWindow(WA_Left,         smr->sm_ReqInfo.ri_Window->LeftEdge + smr->sm_InfoCoords.Left,
                                               WA_Top,          smr->sm_ReqInfo.ri_Window->TopEdge + smr->sm_InfoCoords.Top,
                                               WA_Width,        smr->sm_InfoCoords.Width,
                                               WA_Height,       (ri->ri_TextAttr->ta_YSize+1)*7 + smr->sm_ReqInfo.ri_Window->BorderTop + smr->sm_ReqInfo.ri_Window->BorderBottom + 4,
                                               WA_AutoAdjust,   TRUE,
                                               WA_Flags,        WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_SIMPLE_REFRESH,
                                               WA_Title,        GetString(&smr->sm_ReqInfo.ri_LocaleInfo,MSG_ASL_SMINFO_TITLE),
                                               WA_CustomScreen, smr->sm_ReqInfo.ri_Screen,
                                               WA_NewLookMenus, TRUE,
                                               WA_HelpGroupWindow, smr->sm_ReqInfo.ri_Window,
                                               TAG_DONE))
        {
            ng.ng_LeftEdge   = 4+smr->sm_InfoWindow->BorderLeft;
            ng.ng_TopEdge    = 4+smr->sm_InfoWindow->BorderTop;
            ng.ng_Width      = smr->sm_InfoWindow->GZZWidth-8;
            ng.ng_Height     = smr->sm_InfoWindow->GZZHeight-8;
            ng.ng_GadgetText = NULL;
            ng.ng_TextAttr   = ri->ri_TextAttr;
            ng.ng_Flags      = 0;
            ng.ng_VisualInfo = ri->ri_VisualInfo;
            ng.ng_GadgetID   = SM_PROPLIST;
            ng.ng_UserData   = (APTR)smr;

            tags[0].ti_Tag   = GTLV_ReadOnly;
            tags[0].ti_Data  = TRUE;
            tags[1].ti_Tag   = LAYOUTA_SPACING;
            tags[1].ti_Data  = 1;
            tags[2].ti_Tag   = TAG_DONE;

            gad = CreateContext(&smr->sm_InfoGadgets);
            if (smr->sm_PropertyListGad = CreateGadgetA(LISTVIEW_KIND,gad,&ng,tags))
            {
                smr->sm_InfoWindow->UserPort = smr->sm_ReqInfo.ri_Window->UserPort;
                ModifyIDCMP(smr->sm_InfoWindow,IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | LISTVIEWIDCMP);

                AddGList(smr->sm_InfoWindow, smr->sm_InfoGadgets,-1,-1,NULL);
                RefreshGList(smr->sm_InfoGadgets, smr->sm_InfoWindow,NULL,-1);
                GT_RefreshWindow(smr->sm_InfoWindow, NULL);
                UpdatePropList(smr);
                return;
            }
        }

        CloseInfoWindow(smr);
        DisplayBeep(smr->sm_ReqInfo.ri_Window->WScreen);
    }
}


/*****************************************************************************/


static VOID DepthToColors(struct ExtSMReq *smr, struct Gadget *gadget, WORD level, STRPTR result)
{
ULONG  num;

    num = (ULONG)(1L << level);
    if (smr->sm_CurrentMode->dm_PropertyFlags & DIPF_IS_HAM)
    {
        if (level == 6)
        {
            num = 4096;
        }
        else
        {
            num = 16777216;
        }

        if (gadget)
            gadget->GadgetID = SM_HAMCOLORS;
    }
    else
    {
        if (gadget)
            gadget->GadgetID = SM_COLORS;
    }

    if (num < (1<<15))
        sprintf(result,"%lU",num);

    else if (num < (1 << 20))
        sprintf(result,"%luK",num / 1024);

    else
        sprintf(result,"%luM",num / (1024*1024));
}


/*****************************************************************************/


static UWORD MaxDigits(struct ExtSMReq *smr, struct Gadget *gadget, WORD maxDepth)
{
ULONG  num;
char   stash[32];
UWORD  maxLen;

    maxLen = 0;
    while (maxDepth)
    {
        num = (ULONG)(1L << maxDepth--);

        if (num < (1<<14))
            sprintf(stash,"%lU",num);

        else if (num < (1 << 20))
            sprintf(stash,"%luK",num / 1024);

        else
            sprintf(stash,"%luM",num / (1024*1024));

        if (strlen(stash) > maxLen)
            maxLen = strlen(stash);
    }

    return(maxLen);
}


/*****************************************************************************/


static VOID RenderSMDisplay(struct ExtSMReq *smr)
{
struct ReqInfo  *ri = &smr->sm_ReqInfo;
struct RastPort *rp;
struct Gadget   *gad;
UWORD            len;
UWORD            plen;
char             str[20];

    rp  = ri->ri_RastPort;
    if (gad = ri->ri_Template[NUMCOLORSGAD].ag_Gadget)
    {
        DepthToColors(smr,ri->ri_Template[COLORSGAD].ag_Gadget,smr->sm_DisplayDepth,str);

        len  = strlen(str);
        plen = TextLength(rp,str,len);

        SetAPen(rp,ri->ri_DrawInfo->dri_Pens[BACKGROUNDPEN]);
        RectFill(rp,gad->LeftEdge,gad->TopEdge+2,gad->LeftEdge+gad->Width-1-plen,gad->TopEdge+rp->TxHeight-1+2);

        SetAPen(rp,ri->ri_DrawInfo->dri_Pens[TEXTPEN]);
        Move(rp,gad->LeftEdge+gad->Width-plen,gad->TopEdge+rp->TxBaseline+2);
        Text(rp,str,len);
    }
}


/*****************************************************************************/


static struct Rectangle *GetRect(struct ExtSMReq *smr)
{
struct DisplayMode   *node;
struct DimensionInfo *dimInfo;
struct Rectangle     *rect;

    node    = smr->sm_CurrentMode;
    dimInfo = &node->dm_DimensionInfo;

    switch (smr->sm_OverscanType)
    {
        case OSCAN_STANDARD: rect = &dimInfo->StdOScan;
                             break;

        case OSCAN_MAX     : rect = &dimInfo->MaxOScan;
                             break;

        case OSCAN_VIDEO   : rect = &dimInfo->VideoOScan;
                             break;

        default            : rect = &dimInfo->TxtOScan;
                             break;
    }

    return(rect);
}


/*****************************************************************************/


static VOID DoSMGadgets(struct ExtSMReq *smr)
{
struct ReqInfo       *ri;
struct DisplayMode   *node;
struct DimensionInfo *dimInfo;
WORD                  curWidth,curHeight;
WORD                  minWidth,maxWidth;
WORD                  minHeight,maxHeight;
WORD                  minDepth,maxDepth;
ULONG                 nodeNum;

    ri         = &smr->sm_ReqInfo;
    node       = smr->sm_CurrentMode;
    dimInfo    = &node->dm_DimensionInfo;
    minWidth   = dimInfo->MinRasterWidth;
    maxWidth   = dimInfo->MaxRasterWidth;
    minHeight  = dimInfo->MinRasterHeight;
    maxHeight  = dimInfo->MaxRasterHeight;
    minDepth   = 1;  /* dimInfo->MinDepth; doesn't exist! */
    maxDepth   = dimInfo->MaxDepth;

    if (node->dm_PropertyFlags & (DIPF_IS_HAM | DIPF_IS_EXTRAHALFBRITE))
        minDepth = 6;

    if (minWidth < smr->sm_MinWidth)
        minWidth = smr->sm_MinWidth;

    if (maxWidth > smr->sm_MaxWidth)
        maxWidth = smr->sm_MaxWidth;

    if (minHeight < smr->sm_MinHeight)
        minHeight = smr->sm_MinHeight;

    if (maxHeight > smr->sm_MaxHeight)
        maxHeight = smr->sm_MaxHeight;

    if (minDepth < smr->sm_MinDepth)
        minDepth = smr->sm_MinDepth;

    if (maxDepth > smr->sm_MaxDepth)
        maxDepth = smr->sm_MaxDepth;

    curWidth  = smr->sm_DisplayWidth;
    curHeight = smr->sm_DisplayHeight;

    if (curWidth < minWidth)
        curWidth = minWidth;

    if (curWidth > maxWidth)
        curWidth = maxWidth;

    if (curHeight < minHeight)
        curHeight = minHeight;

    if (curHeight > maxHeight)
        curHeight = maxHeight;

    if ((node->dm_PropertyFlags & DIPF_IS_HAM) && (smr->sm_DisplayDepth == 7))
        smr->sm_DisplayDepth = 6;

    if (smr->sm_DisplayDepth < minDepth)
        smr->sm_DisplayDepth = minDepth;

    if (smr->sm_DisplayDepth > maxDepth)
        smr->sm_DisplayDepth = maxDepth;

    smr->sm_DisplayWidth  = curWidth;
    smr->sm_DisplayHeight = curHeight;

    if (!smr->sm_ReqInfo.ri_Template[MODELISTGAD].ag_Gadget)
    {
        nodeNum = FindNodeNum(&smr->sm_DisplayModes,smr->sm_CurrentMode);
        SetGadgetAttr(ri,MODELISTGAD,
                      GTLV_ShowSelected, NULL,
                      GTLV_Labels,       &smr->sm_DisplayModes,
                      LAYOUTA_SPACING,   1,
                      GTLV_ScrollWidth,  18,
                      GTLV_Selected,     nodeNum,
                      GTLV_MakeVisible,  nodeNum,
                      TAG_DONE);
    }

    if (SMF_DOWIDTH & smr->sm_Flags)
    {
        SetGadgetAttr(ri,WIDTHGAD,GTIN_Number,   smr->sm_DisplayWidth,
                                  GTIN_MaxChars, 4,
                                  TAG_DONE);
    }

    if (SMF_DOHEIGHT & smr->sm_Flags)
    {
        SetGadgetAttr(ri,HEIGHTGAD,GTIN_Number,   smr->sm_DisplayHeight,
                                   GTIN_MaxChars, 4,
                                   TAG_DONE);
    }

    if (SMF_DODEPTH & smr->sm_Flags)
    {
        if (node->dm_PropertyFlags & DIPF_IS_HAM)
        {
            SetGadgetAttr(ri,COLORSGAD,GTSL_Level,   (smr->sm_DisplayDepth / 2) - 3,   /* 6 becomes 0, 8 becomes 1 */
                                       GTSL_Min,     0,
                                       GTSL_Max,     (maxDepth-minDepth) / 2,
                                       GA_RelVerify, TRUE,
                                       GA_Immediate, TRUE,
                                       TAG_DONE);

        }
        else
        {
            SetGadgetAttr(ri,COLORSGAD,GTSL_Level,   smr->sm_DisplayDepth,
                                       GTSL_Min,     minDepth,
                                       GTSL_Max,     maxDepth,
                                       GA_RelVerify, TRUE,
                                       GA_Immediate, TRUE,
                                       TAG_DONE);
        }

        SetGadgetAttr(ri,NUMCOLORSGAD,GTIN_MaxChars, MaxDigits(smr,ri->ri_Template[COLORSGAD].ag_Gadget,smr->sm_MaxDepth),
                                      /* Above tag is for special use by layout engine */
                                      TAG_DONE);

        if (ri->ri_Template[COLORSGAD].ag_Gadget)
            RenderSMDisplay(smr);
    }

    if (SMF_DOAUTOSCROLL & smr->sm_Flags)
    {
        SetGadgetAttr(ri,AUTOSCROLLGAD,GTCB_Checked, smr->sm_AutoScroll,
                                       GTCB_Scaled,  TRUE,
                                       TAG_DONE);
    }

    if (SMF_DOOVERSCAN & smr->sm_Flags)
    {
        SetGadgetAttr(ri,OVERSCANTYPEGAD,GTCY_Labels, (ULONG)smr->sm_OverscanLabels,
                                         GTCY_Active, smr->sm_OverscanType-1,
                                         TAG_DONE);
    }
}


/*****************************************************************************/


static VOID SetDefSizes(struct ExtSMReq *smr)
{
struct Rectangle *rect;

    rect = GetRect(smr);

    smr->sm_DisplayWidth  = rect->MaxX - rect->MinX + 1;
    smr->sm_DisplayHeight = rect->MaxY - rect->MinY + 1;

    DoSMGadgets(smr);
}


/*****************************************************************************/


static BOOL CreateSMGadgets(struct ExtSMReq *smr)
{
struct ReqInfo *ri = &smr->sm_ReqInfo;

    FreeLayoutGadgets(ri,TRUE);

    DoSMGadgets(smr);

    if (LayoutGadgets(ri,LGM_CREATE))
    {
        AddGList(ri->ri_Window,ri->ri_Gadgets,-1,-1,NULL);
        RefreshGList(ri->ri_Gadgets,ri->ri_Window,NULL,-1);
        GT_RefreshWindow(ri->ri_Window,NULL);

        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


static VOID NewMode(struct ExtSMReq *smr, struct DisplayMode *new,
                    BOOL shuffle, BOOL force)
{
ULONG nodeNum;

    if ((new != smr->sm_CurrentMode) || force)
    {
        if (shuffle)
        {
            nodeNum = FindNodeNum(&smr->sm_DisplayModes,new);
            SetGadgetAttr(&smr->sm_ReqInfo,MODELISTGAD,
                          GTLV_Selected,    nodeNum,
                          GTLV_MakeVisible, nodeNum,
                          TAG_DONE);
        }

        smr->sm_CurrentMode = new;
        smr->sm_DisplayID   = new->dm_DimensionInfo.Header.DisplayID;
        DoSMGadgets(smr);
        UpdatePropList(smr);
    }
}


/*****************************************************************************/


static VOID SMPreserve(struct ExtSMReq *smr)
{
    smr->sm_OriginalDisplayID     = smr->sm_DisplayID;
    smr->sm_OriginalOverscanType  = smr->sm_OverscanType;
    smr->sm_OriginalAutoScroll    = smr->sm_AutoScroll;
    smr->sm_OriginalDisplayWidth  = smr->sm_DisplayWidth;
    smr->sm_OriginalDisplayHeight = smr->sm_DisplayHeight;
    smr->sm_OriginalDisplayDepth  = smr->sm_DisplayDepth;
}


/*****************************************************************************/


static VOID SMRestore(struct ExtSMReq *smr)
{
    smr->sm_DisplayID     = smr->sm_OriginalDisplayID;
    smr->sm_OverscanType  = smr->sm_OriginalOverscanType;
    smr->sm_AutoScroll    = smr->sm_OriginalAutoScroll;
    smr->sm_DisplayWidth  = smr->sm_OriginalDisplayWidth;
    smr->sm_DisplayHeight = smr->sm_OriginalDisplayHeight;
    smr->sm_DisplayDepth  = smr->sm_OriginalDisplayDepth;
}


/*****************************************************************************/


static BOOL HandleSMIDCMP(struct ExtSMReq *smr)
{
struct ReqInfo       *ri = &smr->sm_ReqInfo;
struct IntuiMessage  *intuiMsg;
ULONG                 class;
UWORD		      icode;
struct Gadget        *gadget;
struct Window        *window;
struct MenuItem      *menuItem;
UWORD                 oldICode;
ULONG                 micros, oldMicros;
ULONG                 seconds, oldSeconds;

    oldICode   = 0xffff;
    oldSeconds = 0;
    oldMicros  = 0;

    while (TRUE)
    {
        intuiMsg = GetReqMsg(&smr->sm_ReqInfo,PUBLIC_SMR(smr),smr->sm_InfoWindow,0);

        class   = intuiMsg->Class;
        icode   = intuiMsg->Code;
        gadget  = (struct Gadget *) intuiMsg->IAddress;
        window  = intuiMsg->IDCMPWindow;
        seconds = intuiMsg->Seconds;
        micros  = intuiMsg->Micros;
        GT_ReplyIMsg(intuiMsg);

        switch (class)
        {
            case IDCMP_REFRESHWINDOW: GT_BeginRefresh(window);
                                      GT_EndRefresh(window,TRUE);
                                      RenderSMDisplay(smr);
                                      break;

            case IDCMP_CLOSEWINDOW  : if (window == smr->sm_InfoWindow)
                                      {
                                          CloseInfoWindow(smr);
                                      }
                                      else
                                      {
                                          return(FALSE);
                                      }
                                      break;

            case IDCMP_MENUPICK     : while (icode != MENUNULL)
                                      {
                                          menuItem = ItemAddress(smr->sm_ReqInfo.ri_Menus,icode);
                                          switch ((UWORD)MENU_USERDATA(menuItem))
                                          {
                                              case SM_CANCEL  : return(FALSE);

                                              case SM_OK      : if (ri->ri_Template[WIDTHGAD].ag_Gadget)
                                                                    smr->sm_DisplayWidth  = ((struct StringInfo *) (ri->ri_Template[WIDTHGAD].ag_Gadget->SpecialInfo))->LongInt;
                                                                if (ri->ri_Template[HEIGHTGAD].ag_Gadget)
                                                                    smr->sm_DisplayHeight = ((struct StringInfo *) (ri->ri_Template[HEIGHTGAD].ag_Gadget->SpecialInfo))->LongInt;
                                                                /*!!! validate values !!!*/
                                                                return(TRUE);

                                              case SM_INFO    : OpenInfoWindow(smr);
                                                                break;

                                              case SM_NEXTMODE: if (smr->sm_CurrentMode != (struct DisplayMode *)smr->sm_DisplayModes.lh_TailPred)
                                                                    NewMode(smr,(struct DisplayMode *)smr->sm_CurrentMode->dm_Node.ln_Succ,TRUE,FALSE);
                                                                break;

                                              case SM_PREVMODE: if (smr->sm_CurrentMode != (struct DisplayMode *)smr->sm_DisplayModes.lh_Head)
                                                                    NewMode(smr,(struct DisplayMode *)smr->sm_CurrentMode->dm_Node.ln_Pred,TRUE,FALSE);
                                                                break;

                                              case SM_RESTORE : SMRestore(smr);
                                                                NewMode(smr,GetCurrentMode(smr,smr->sm_DisplayID),TRUE,TRUE);
                                                                break;
                                          }
                                          icode = menuItem->NextSelect;
                                      }
                                      break;

            case IDCMP_MOUSEMOVE    :
            case IDCMP_GADGETDOWN   :
            case IDCMP_GADGETUP     : switch ((UWORD)gadget->GadgetID)
                                      {
                                          case SM_CANCEL  : return(FALSE);

                                          case SM_DMLIST  : if ((icode != oldICode) || !DoubleClick(oldSeconds,oldMicros,seconds,micros))
                                                            {
                                                                NewMode(smr,(struct DisplayMode *)FindNum((struct List *)&smr->sm_DisplayModes,icode),FALSE,TRUE);
                                                                SetDefSizes(smr);

                                                                oldICode   = icode;
                                                                oldSeconds = seconds;
                                                                oldMicros  = micros;
                                                                break;
                                                            }

                                          case SM_OK      : if (ri->ri_Template[WIDTHGAD].ag_Gadget)
                                                                smr->sm_DisplayWidth  = ((struct StringInfo *) (ri->ri_Template[WIDTHGAD].ag_Gadget->SpecialInfo))->LongInt;
                                                            if (ri->ri_Template[HEIGHTGAD].ag_Gadget)
                                                                smr->sm_DisplayHeight = ((struct StringInfo *) (ri->ri_Template[HEIGHTGAD].ag_Gadget->SpecialInfo))->LongInt;
                                                            /*!!! validate values */
                                                            return(TRUE);

                                          case SM_WIDTH   : smr->sm_DisplayWidth = ((struct StringInfo *) (gadget->SpecialInfo))->LongInt;
                                                            DoSMGadgets(smr);
                                                            if (ri->ri_Template[HEIGHTGAD].ag_Gadget)
                                                                ActivateGadget(ri->ri_Template[HEIGHTGAD].ag_Gadget,window,NULL);
                                                            break;

                                          case SM_HEIGHT  : smr->sm_DisplayHeight = ((struct StringInfo *) (gadget->SpecialInfo))->LongInt;
                                                            DoSMGadgets(smr);
                                                            if (ri->ri_Template[WIDTHGAD].ag_Gadget)
                                                                ActivateGadget(ri->ri_Template[WIDTHGAD].ag_Gadget,window,NULL);
                                                            break;

                                          case SM_HAMCOLORS: smr->sm_DisplayDepth = (icode+3)*2;  /* 0 becomes 6, 1 becomes 8 */
                                                             RenderSMDisplay(smr);
                                                             break;


                                          case SM_COLORS  : smr->sm_DisplayDepth = icode;
                                                            RenderSMDisplay(smr);
                                                            break;

                                          case SM_OVERSCAN: smr->sm_OverscanType = icode+1;
                                                            SetDefSizes(smr);
                                                            break;

                                          case SM_AUTOSCROLL: smr->sm_AutoScroll = (SELECTED & gadget->Flags);
                                                              break;
                                      }
                                      break;

            case IDCMP_NEWSIZE      : if (!CreateSMGadgets(smr))
                                          return(FALSE);
                                      break;
        }
    }
}


/*****************************************************************************/


static VOID ProcessSMTags(struct ExtSMReq *smr, struct TagItem *tagList)
{
struct TagItem *tag;
struct TagItem *tags = tagList;
ULONG           data;

    while (tag = NextTagItem(&tags))
    {
        data = tag->ti_Data;
        switch (tag->ti_Tag)
        {
            case ASLSM_InitialDisplayID    : smr->sm_DisplayID = (ULONG)data;
                                             break;

            case ASLSM_InitialDisplayWidth : smr->sm_DisplayWidth = (ULONG)data;
                                             break;

            case ASLSM_InitialDisplayHeight: smr->sm_DisplayHeight = (ULONG)data;
                                             break;

            case ASLSM_InitialDisplayDepth : smr->sm_DisplayDepth = (UWORD)data;
                                             break;

            case ASLSM_InitialOverscanType : if (data < OSCAN_TEXT)
                                                 data = OSCAN_TEXT;

                                             smr->sm_OverscanType = (UWORD)data;
                                             break;

            case ASLSM_InitialAutoScroll   : smr->sm_AutoScroll = (BOOL)data;
                                             break;

            case ASLSM_PropertyFlags       : smr->sm_PropertyFlags = (ULONG)data;
                                             break;

            case ASLSM_PropertyMask        : smr->sm_PropertyMask = (ULONG)data;
                                             break;

            case ASLSM_MinWidth            : smr->sm_MinWidth = (ULONG)data;
                                             break;

            case ASLSM_MaxWidth            : smr->sm_MaxWidth = (ULONG)data;
                                             break;

            case ASLSM_MinHeight           : smr->sm_MinHeight = (ULONG)data;
                                             break;

            case ASLSM_MaxHeight           : smr->sm_MaxHeight = (ULONG)data;
                                             break;

            case ASLSM_MinDepth            : smr->sm_MinDepth = (UWORD)data;
                                             break;

            case ASLSM_MaxDepth            : smr->sm_MaxDepth = (UWORD)data;
                                             break;

            case ASLSM_FilterFunc          : smr->sm_FilterFunc = (struct Hook *)data;
                                             break;

            case ASLSM_CustomSMList        : smr->sm_CustomSMList = (struct List *)data;
                                             break;

            case ASLSM_InitialInfoOpened   : smr->sm_InfoOpened = (BOOL)data;
                                             break;

            case ASLSM_InitialInfoLeftEdge : smr->sm_InfoCoords.Left = (WORD)data;
                                             break;

            case ASLSM_InitialInfoTopEdge  : smr->sm_InfoCoords.Top = (WORD)data;
                                             break;

            case ASLSM_UserData            : smr->sm_UserData = (APTR)data;
                                             break;

            default                        : ProcessStdTag(&smr->sm_ReqInfo,tag);
                                             break;
        }
    }

    smr->sm_Flags = AslPackBoolTags(smr->sm_Flags,tagList,
                                    ASLSM_DoOverscanType, SMF_DOOVERSCAN,
                                    ASLSM_DoAutoScroll,   SMF_DOAUTOSCROLL,
                                    ASLSM_DoWidth,        SMF_DOWIDTH,
                                    ASLSM_DoHeight,       SMF_DOHEIGHT,
                                    ASLSM_DoDepth,        SMF_DODEPTH,
                                    TAG_DONE);
}


/*****************************************************************************/


struct ExtSMReq *AllocSMRequest(struct TagItem *tagList)
{
struct ExtSMReq *smr;

    if (smr = AllocVec(sizeof(struct ExtSMReq), MEMF_CLEAR | MEMF_ANY))
    {
        smr->sm_ReqInfo.ri_Coords.Left   = 30;
        smr->sm_ReqInfo.ri_Coords.Top    = 20;
        smr->sm_ReqInfo.ri_Coords.Width  = 318;
        smr->sm_ReqInfo.ri_Coords.Height = 198;

        smr->sm_ReqInfo.ri_TitleID     = MSG_ASL_SM_TITLE;
        smr->sm_ReqInfo.ri_IDCMPFlags  = IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | LISTVIEWIDCMP | SCROLLERIDCMP | BUTTONIDCMP | CYCLEIDCMP;
        smr->sm_ReqInfo.ri_WindowFlags = WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIMPLE_REFRESH | WFLG_ACTIVATE | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM;
        smr->sm_DisplayWidth           = 640;
        smr->sm_DisplayHeight          = 200;
        smr->sm_DisplayDepth           = 2;
        smr->sm_OverscanType           = OSCAN_TEXT;
        smr->sm_AutoScroll             = TRUE;
        smr->sm_PropertyFlags          = DIPF_IS_WB;
        smr->sm_PropertyMask           = DIPF_IS_WB;
        smr->sm_MinDepth               = 1;
        smr->sm_MaxDepth               = 24;
        smr->sm_MinWidth               = 16;
        smr->sm_MaxWidth               = 16368;
        smr->sm_MinHeight              = 16;
        smr->sm_MaxHeight              = 16384;

        smr->sm_InfoCoords.Left        = 16;
        smr->sm_InfoCoords.Top         = 25;
        smr->sm_InfoCoords.Width       = 280;
        smr->sm_InfoCoords.Height      = 84;

        NewList(&smr->sm_DisplayModes);

        ProcessSMTags(smr,tagList);

        smr->sm_Coords = smr->sm_ReqInfo.ri_Coords;
    }

    return (smr);
}


/*****************************************************************************/


APTR SMRequest(struct ExtSMReq *smr, struct TagItem *tagList)
{
struct ReqInfo *ri     = &smr->sm_ReqInfo;
LONG            error  = ERROR_NO_FREE_STORE;
BOOL            result = FALSE;
UWORD           i;
APTR            memory;
ULONG           round;

    if (memory = PrepareRequester(ri,AG,sizeof(AG),8))
    {
        ProcessSMTags(smr,tagList);
        PrepareLocale(&smr->sm_ReqInfo);

        if (!(SMF_DOWIDTH & smr->sm_Flags))
        {
            ri->ri_Template[WIDTHGAD].ag_LOGroup = 0;
            ri->ri_Template[HEIGHTGAD].ag_LOSub = 0;
        }

        if (!(SMF_DOHEIGHT & smr->sm_Flags))
            ri->ri_Template[HEIGHTGAD].ag_LOGroup = 0;

        if (!(SMF_DODEPTH & smr->sm_Flags))
        {
            ri->ri_Template[NUMCOLORSGAD].ag_LOGroup = 0;
            ri->ri_Template[COLORSGAD].ag_LOGroup = 0;
        }

        if (!(SMF_DOAUTOSCROLL & smr->sm_Flags))
        {
            ri->ri_Template[AUTOSCROLLGAD].ag_LOGroup = 0;
        }

        for (i=0; i<3; i++)
            smr->sm_OverscanLabels[i] = GetString(&ri->ri_LocaleInfo,i+MSG_ASL_SM_TEXT);
        smr->sm_OverscanLabels[3] = GetString(&ri->ri_LocaleInfo,MSG_ASL_SM_REALVIDEO);

        if (SMF_DOOVERSCAN & smr->sm_Flags)
        {
            /* we need this here in order to get the tags cloned */
            SetGadgetAttr(&smr->sm_ReqInfo,OVERSCANTYPEGAD,GTCY_Labels, (ULONG)smr->sm_OverscanLabels,
                                                           GTCY_Active, smr->sm_OverscanType,
                                                           TAG_DONE);
        }
        else
        {
            ri->ri_Template[OVERSCANTYPEGAD].ag_LOGroup = 0;
        }

        SMPreserve(smr);

        if (GetDisplayModes(smr))
        {
            error = ERROR_NO_MORE_ENTRIES;
            if (!IsListEmpty(&smr->sm_DisplayModes))
            {
                error = ERROR_NO_FREE_STORE;
                if (OpenRequester(&smr->sm_ReqInfo,AM))
                {
                    for (i = 0; i<12; i++)
                        smr->sm_PropertyNodes[i].ln_Name = GetString(&ri->ri_LocaleInfo,i+MSG_ASL_SM_INTERLACED);
                    smr->sm_PropertyNodes[SCANRATES_PID].ln_Name = smr->sm_ScanRates;

                    smr->sm_CurrentMode = GetCurrentMode(smr,smr->sm_DisplayID);
                    smr->sm_DisplayID   = smr->sm_CurrentMode->dm_DimensionInfo.Header.DisplayID;
                    SMPreserve(smr);

                    if (CreateSMGadgets(smr))
                    {
                        RenderSMDisplay(smr);
                        error = 0;

                        if (smr->sm_InfoOpened)
                            OpenInfoWindow(smr);

                        if (result = HandleSMIDCMP(smr))
                        {
                            round = 16 << GfxBase->bwshifts[GfxBase->MemType];
                            smr->sm_BitMapWidth = (smr->sm_BitMapWidth + round - 1) & (~(round-1));
                            smr->sm_BitMapHeight = smr->sm_DisplayHeight;
                        }
                    }

                    smr->sm_InfoOpened = (smr->sm_InfoWindow != NULL);
                    CloseInfoWindow(smr);
                    CloseRequester(&smr->sm_ReqInfo);
                }
            }
            FreeDisplayModes(smr);
        }

        smr->sm_Coords = ri->ri_Coords;

        if (!result)
            SMRestore(smr);

        CleanupRequester(ri,memory);
    }

    SetIoErr(error);

    return((APTR)result);
}


/*****************************************************************************/


VOID FreeSMRequest(struct ExtSMReq *smr)
{
    FreeVec(smr);
}
