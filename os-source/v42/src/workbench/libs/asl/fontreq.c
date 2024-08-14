
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/screens.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <dos/dos.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/layers_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "asl.h"
#include "aslbase.h"
#include "fontreq.h"
#include "aslutils.h"
#include "asllists.h"
#include "layout.h"
#include "requtils.h"


/*****************************************************************************/


#define LocaleBase ri->ri_LocaleInfo.li_LocaleBase
#define catalog    ri->ri_LocaleInfo.li_Catalog


/*****************************************************************************/


struct CacheNode
{
    struct MinNode   cn_Node;
    ULONG            cn_Key;
    struct DateStamp cn_Date;
};


struct FontNode
{
    struct Node    fn_Node;
    struct MinList fn_Sizes;
};


struct SizeNode
{
    struct Node sn_Node;
    UWORD       sn_Size;
};


/*****************************************************************************/


typedef enum FOCommands
{
    FO_NOP,

    FO_OK,
    FO_CANCEL,

    FO_NAMELIST,
    FO_NAME,
    FO_SIZELIST,
    FO_SIZE,

    FO_PLAIN,
    FO_BOLD,
    FO_ITALIC,
    FO_UNDERLINED,

    FO_TEXT,
    FO_FIELD,

    FO_MODE,

    FO_PREVFONT,
    FO_NEXTFONT,
    FO_RESTORE,
    FO_RESCAN
};


/*****************************************************************************/


static struct ASLMenu far AM[] =
{
    {NM_TITLE,  FO_NOP,      MSG_ASL_CONTROL_MENU},
      {NM_ITEM, FO_PREVFONT, MSG_ASL_CONTROL_LASTFONT},
      {NM_ITEM, FO_NEXTFONT, MSG_ASL_CONTROL_NEXTFONT},
      {NM_ITEM, FO_NOP,      MSG_BARLABEL},
      {NM_ITEM, FO_RESTORE,  MSG_ASL_CONTROL_RESTORE},
      {NM_ITEM, FO_RESCAN,   MSG_ASL_CONTROL_RESCAN},
      {NM_ITEM, FO_NOP,      MSG_BARLABEL},
      {NM_ITEM, FO_OK,       MSG_ASL_CONTROL_OK},
      {NM_ITEM, FO_CANCEL,   MSG_ASL_CONTROL_CANCEL},

    {NM_END,    FO_NOP,      MSG_BARLABEL}
};


/*****************************************************************************/


#define	POSGAD		1
#define	NEGGAD		2
#define	DISPLAYGAD	3
#define	PLAINGAD	5
#define	BOLDGAD		6
#define	ITALICGAD	7
#define	UNDERLINEDGAD	8
#define	TEXTGAD		9
#define	FIELDGAD	10
#define	MODEGAD		11
#define	NAMEGAD		13
#define	NAMELISTGAD	14
#define	SIZEGAD		15
#define	SIZELISTGAD	16

/* main display gadgets */
static struct ASLGadget far AG[] =
{
    {HGROUP_KIND,   FO_NOP,       0,               MSG_NOTHING,              {  4,-16,  -8,  14}, 1,},
    {BUTTON_KIND,   FO_OK,        0,               MSG_ASL_OK_GAD,           {  4,-16,  70,  14}, 1, 0, },
    {BUTTON_KIND,   FO_CANCEL,    0,               MSG_ASL_CANCEL_GAD,       {-74,-16,  70,  14}, 1, 0, },

    {GENERIC_KIND,  FO_NOP,       0,               MSG_NOTHING,              {  4,-46,  -8,  26}, 2, 0, },

    {VGROUP_KIND,   FO_NOP,       0,               MSG_NOTHING,              {  4,-98,  -8,  50}, 3, },
    {CHECKBOX_KIND, FO_PLAIN,     PLACETEXT_RIGHT, MSG_ASL_FO_PLAIN_GAD,     {  0,  0,   0,   0}, 3, 0,  },
    {CHECKBOX_KIND, FO_BOLD,      PLACETEXT_RIGHT, MSG_ASL_FO_BOLD_GAD,      {  0,  0,   0,   0}, 3, 0,  },
    {CHECKBOX_KIND, FO_ITALIC,    PLACETEXT_RIGHT, MSG_ASL_FO_ITALIC_GAD,    {  0,  0,   0,   0}, 3, 0,  },
    {CHECKBOX_KIND, FO_UNDERLINED,PLACETEXT_RIGHT, MSG_ASL_FO_UNDERLINED_GAD,{  0,  0,   0,   0}, 3, 0,  },
    {PALETTE_KIND,  FO_TEXT,      PLACETEXT_LEFT,  MSG_ASL_FO_TEXT_GAD,      {  0,  0,   0,   0}, 3, 1,  },
    {PALETTE_KIND,  FO_FIELD,     PLACETEXT_LEFT,  MSG_ASL_FO_FIELD_GAD,     {  0,  0,   0,   0}, 3, 1,  },
    {CYCLE_KIND,    FO_MODE,      0,               MSG_ASL_FO_MODE_GAD,      {  0,  0,   0,   0}, 3, 1, },

    {DGROUP_KIND,   FO_NOP,       0,               MSG_NOTHING,              {  4,  2,  -8,-102}, 4, },
    {STRING_KIND,   FO_NAME,      0,               MSG_NOTHING,              {  0,  0, -58,   0}, 4, 0, },
    {LISTVIEW_KIND, FO_NAMELIST,  0,               MSG_NOTHING,              {  0,  0, -58,  -6}, 4, 0, },
    {INTEGER_KIND,  FO_SIZE,      0,               MSG_NOTHING,              {-54,  0,  54,   0}, 4, 0, },
    {LISTVIEW_KIND, FO_SIZELIST,  0,               MSG_NOTHING,              {-54,  0,  54,  -6}, 4, 0, },

    {END_KIND,},
};


/*****************************************************************************/


VOID SetNewFontSize(struct ExtFontReq *fr, LONG newSize);


/*****************************************************************************/


static VOID StripDotFont(struct ASLLib *AslBase, STRPTR name)
{
    StripExtension(AslBase,name,".font");
}


/*****************************************************************************/


static VOID AppendDotFont(struct ASLLib *AslBase, STRPTR name)
{
    StripDotFont(AslBase,name);
    strcat(name,".font");
}


/*****************************************************************************/


static VOID DisposeCacheList(struct ASLLib *AslBase, struct MinList *list)
{
    FreeList(AslBase,(struct List *)list);
}


/*****************************************************************************/


static VOID CreateCacheList(struct ASLLib *AslBase, struct MinList *list)
{
struct CacheNode               *new;
struct DevProc                 *dvp;
BPTR                            lock;
struct FileInfoBlock __aligned  fib;

    dvp = NULL;
    while (dvp = GetDeviceProc("FONTS:",dvp))
    {
        new = NULL;
        fib.fib_DiskKey = NULL;  /* used as NULL BSTR in following call */
        if (lock = DoPkt3(dvp->dvp_Port,ACTION_LOCATE_OBJECT,dvp->dvp_Lock,MKBADDR(&fib.fib_DiskKey),SHARED_LOCK))
        {
            if (Examine(lock,&fib))
            {
                if (new = AllocVec(sizeof(struct CacheNode),MEMF_ANY))
                {
                    new->cn_Key  = fib.fib_DiskKey;
                    new->cn_Date = fib.fib_Date;
                    AddTail((struct List *)list,(struct Node *)new);
                }
            }
            UnLock(lock);
        }

        if (!new)
        {
            FreeDeviceProc(dvp);
            DisposeCacheList(AslBase,list);
            return;
        }
    }
}


/*****************************************************************************/


static BOOL ValidateCacheList(struct ASLLib *AslBase, struct MinList *list)
{
BOOL                            result;
struct DevProc                 *dvp;
BPTR                            lock;
struct CacheNode               *node;
struct FileInfoBlock __aligned  fib;

    node   = (struct CacheNode *)list->mlh_Head;
    dvp    = NULL;
    result = TRUE;

    while ((dvp = GetDeviceProc("FONTS:",dvp)) && node->cn_Node.mln_Succ && result)
    {
        result = FALSE;
        fib.fib_DiskKey = NULL;  /* used as NULL BSTR in following call */
        if (lock = DoPkt3(dvp->dvp_Port,ACTION_LOCATE_OBJECT,dvp->dvp_Lock,MKBADDR(&fib.fib_DiskKey),SHARED_LOCK))
        {
            result = (Examine(lock,&fib) && (fib.fib_DiskKey == node->cn_Key) && (CompareDates(&fib.fib_Date,&node->cn_Date) == 0));
            UnLock(lock);
        }
        node = (struct CacheNode *)node->cn_Node.mln_Succ;
    }

    FreeDeviceProc(dvp);

    if ((!dvp && node->cn_Node.mln_Succ) || (dvp && !node->cn_Node.mln_Succ))
        result = FALSE;

    return(result);
}


/*****************************************************************************/


static VOID DisposeFontList(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct FontNode *node;

    node = (struct FontNode *)fr->fo_Fonts.mlh_Head;
    while (node->fn_Node.ln_Succ)
    {
        FreeList(AslBase,(struct List *)&node->fn_Sizes);
        node = (struct FontNode *)node->fn_Node.ln_Succ;
    }

    FreeList(AslBase,(struct List *)&fr->fo_Fonts);
}


/*****************************************************************************/


static BOOL CreateFontList(struct ASLLib *AslBase, struct ExtFontReq *fr, struct AvailFontsHeader *afh)
{
struct AvailFonts *af;
struct FontNode   *font;
struct SizeNode   *size;
struct SizeNode   *node;
char               sizeStr[20];
UWORD              num;
STRPTR             name;

    num = afh->afh_NumEntries;
    af  = (struct AvailFonts *)((ULONG)afh+2);

    while (num--)
    {
        name = af->af_Attr.ta_Name;
        AppendDotFont(AslBase,name);

        if ((!((fr->fo_Options & FOF_FIXEDWIDTHONLY) && (af->af_Attr.ta_Flags & FPF_PROPORTIONAL)))
        &&  (!fr->fo_CallOldFilter || (fr->fo_ReqInfo.ri_OldHook && CallFunc(AslBase,fr->fo_ReqInfo.ri_OldHook,FOF_FILTERFUNC,(ULONG)&af->af_Attr,(ULONG)PUBLIC_FO(fr),fr->fo_ReqInfo.ri_A4)))
        &&  (!fr->fo_FilterFunc || CallHookPkt(fr->fo_FilterFunc,PUBLIC_FO(fr),&af->af_Attr)))
        {
            StripDotFont(AslBase,name);

            font = (struct FontNode *)FindNameNC(AslBase,(struct List *)&fr->fo_Fonts,name);
            if (!font)
            {
                if (!(font = AllocNamedNode(AslBase,sizeof(struct FontNode),name)))
                {
                    DisposeFontList(AslBase,fr);
                    return(FALSE);
                }

                NewList((struct List *)&font->fn_Sizes);
                EnqueueAlpha(AslBase,(struct List *)&fr->fo_Fonts,(struct Node *)font);
            }

            if ((af->af_Attr.ta_YSize >= fr->fo_MinHeight)
            &&  (af->af_Attr.ta_YSize <= fr->fo_MaxHeight))
            {
                size = (struct SizeNode *)font->fn_Sizes.mlh_Head;
                while (size->sn_Node.ln_Succ)
                {
                    if (size->sn_Size == af->af_Attr.ta_YSize)
                        break;
                    size = (struct SizeNode *)size->sn_Node.ln_Succ;
                }

                if (!size->sn_Node.ln_Succ)
                {
                    sprintf(sizeStr,"%lu",af->af_Attr.ta_YSize);
                    if (!(size = AllocNamedNode(AslBase,sizeof(struct SizeNode),sizeStr)))
                    {
                        DisposeFontList(AslBase,fr);
                        return(FALSE);
                    }

                    size->sn_Size = af->af_Attr.ta_YSize;

                    node = (struct SizeNode *)font->fn_Sizes.mlh_Head;
                    while (node->sn_Node.ln_Succ)
                    {
                        if (size->sn_Size <= node->sn_Size)
                            break;
                        node = (struct SizeNode *)node->sn_Node.ln_Succ;
                    }
                    Insert((struct List *)&font->fn_Sizes,size,node->sn_Node.ln_Pred);
                }
            }
        }

        af = (struct AvailFonts *)((ULONG)af+sizeof(struct AvailFonts));
    }

    return(TRUE);
}


/*****************************************************************************/


static BOOL GetFontList(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct AvailFontsHeader *memAFH;
ULONG                    bufSize;
ULONG                    missing;
BOOL                     result;
struct ASLLib           *asl = AslBase;

    SleepRequester(AslBase,&fr->fo_ReqInfo);

    bufSize = 1024;
    while (memAFH = AllocVec(bufSize,MEMF_ANY))
    {
        if (missing = AvailFonts((APTR)memAFH,bufSize,AFF_MEMORY|AFF_SCALED))
            bufSize += missing;
        else
            break;

        FreeVec(memAFH);
    }

    result = FALSE;
    if (memAFH)
    {
        DisposeFontList(AslBase,fr);
        if (CreateFontList(AslBase,fr,memAFH))
        {
            FreeVec(memAFH);
            ObtainSemaphore(&asl->ASL_FontCacheLock);

            if (!ValidateCacheList(AslBase,&AslBase->ASL_FontCache))
            {
                FreeVec(asl->ASL_AFH);

                DisposeCacheList(AslBase,&AslBase->ASL_FontCache);
                CreateCacheList(AslBase,&AslBase->ASL_FontCache);

                bufSize = 4096;
                while (AslBase->ASL_AFH = AllocVec(bufSize,MEMF_ANY))
                {
                    if (missing = AvailFonts((APTR)AslBase->ASL_AFH,bufSize,AFF_DISK|AFF_SCALED))
                        bufSize += missing;
                    else
                        break;

                    FreeVec(asl->ASL_AFH);
                }
            }

            if (AslBase->ASL_AFH)
                if (CreateFontList(AslBase,fr,AslBase->ASL_AFH))
                    result = TRUE;

            ReleaseSemaphore(&asl->ASL_FontCacheLock);
        }
        else
        {
            FreeVec(memAFH);
        }
    }

    WakeupRequester(AslBase,&fr->fo_ReqInfo);

    return(result);
}


/*****************************************************************************/


static VOID RenderFOFont(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct ReqInfo   *ri = &fr->fo_ReqInfo;
struct TextFont  *font;
struct Region    *oldCR;
struct RastPort  *rp;
WORD              y;
struct Rectangle  rect;
struct Region    *region;
struct TTextAttr  textAttr;

    SetNewFontSize(fr,((struct StringInfo *) (ri->ri_Template[SIZEGAD].ag_Gadget->SpecialInfo))->LongInt);

    if (region = NewRegion())
    {
        rect.MinX = ri->ri_Template[DISPLAYGAD].ag_Gadget->LeftEdge + 3;
        rect.MinY = ri->ri_Template[DISPLAYGAD].ag_Gadget->TopEdge + 2;
        rect.MaxX = ri->ri_Template[DISPLAYGAD].ag_Gadget->LeftEdge + ri->ri_Template[DISPLAYGAD].ag_Gadget->Width - 3 - 1;
        rect.MaxY = ri->ri_Template[DISPLAYGAD].ag_Gadget->TopEdge + ri->ri_Template[DISPLAYGAD].ag_Gadget->Height - 2 - 1;

        if (OrRectRegion(region,&rect))
        {
            SleepRequester(AslBase,ri);

            rp    = ri->ri_RastPort;
            oldCR = InstallClipRegion(rp->Layer,region);

            SetABPenDrMd(rp,fr->fo_FrontPen,0,JAM1);
            SetRast(rp,fr->fo_BackPen);

            AppendDotFont(AslBase,fr->fo_TAttr.tta_Name);
            textAttr = fr->fo_TAttr;
            textAttr.tta_Flags &= ~(FPF_DESIGNED);

            if (font = OpenDiskFont(&textAttr))
            {
                SetFont(rp,font);
                SetSoftStyle(rp,fr->fo_TAttr.tta_Style,FSF_BOLD|FSF_UNDERLINED|FSF_ITALIC);

                if (font->tf_YSize < (ri->ri_Template[DISPLAYGAD].ag_Gadget->Height - 4))
                    y = (ri->ri_Template[DISPLAYGAD].ag_Gadget->Height - font->tf_YSize - 4) / 2 + font->tf_Baseline;
                else
                    y = font->tf_Baseline;

                Move(rp, ri->ri_Template[DISPLAYGAD].ag_Gadget->LeftEdge + 3, ri->ri_Template[DISPLAYGAD].ag_Gadget->TopEdge + 2 + y);
                Text(rp,"123 AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz!@#$%^&*()",66);

                fr->fo_TAttr.tta_Flags = font->tf_Flags;
                fr->fo_TAttr.tta_Style &= ~(FSF_COLORFONT);

                if (font->tf_Style & FSF_COLORFONT)
                    fr->fo_TAttr.tta_Style |= FSF_COLORFONT;

                CloseFont(font);
            }

            StripDotFont(AslBase,fr->fo_TAttr.tta_Name);

            InstallClipRegion(rp->Layer,oldCR);

            WakeupRequester(AslBase,ri);

        }
        DisposeRegion(region);
    }
}


/*****************************************************************************/


static VOID DoFOGadgets(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct ReqInfo *ri = &fr->fo_ReqInfo;
ULONG           numColors;

    SetGadgetAttr(AslBase,ri,NAMEGAD,GTST_MaxChars, 25,
                             GTST_String,   fr->fo_Name,
                             TAG_DONE);

    SetGadgetAttr(AslBase,ri,NAMELISTGAD,GTLV_ShowSelected, ~0,  /* ~0 is special for the layout code */
                                 GTLV_Labels,       &fr->fo_Fonts,
                                 LAYOUTA_SPACING,   1,
                                 TAG_DONE);

    SetGadgetAttr(AslBase,ri,SIZEGAD,GTST_MaxChars, 5,
                             GTIN_Number,   fr->fo_TAttr.tta_YSize,
                             TAG_DONE);

    SetGadgetAttr(AslBase,ri,SIZELISTGAD,GTLV_ShowSelected, ~0,  /* ~0 is special for the layout code */
                                 GTLV_Labels,       fr->fo_CurrentSizes,
                                 LAYOUTA_SPACING,   1,
                                 TAG_DONE);

    if (FOF_DOSTYLE & fr->fo_Options)
    {
        SetGadgetAttr(AslBase,ri,PLAINGAD,GTCB_Checked,(BOOL)(!(fr->fo_TAttr.tta_Style & (FSF_BOLD | FSF_ITALIC | FSF_UNDERLINED))),
                                  GTCB_Scaled, TRUE,
                                  TAG_DONE);

        SetGadgetAttr(AslBase,ri,BOLDGAD,GTCB_Checked,(BOOL)(fr->fo_TAttr.tta_Style & FSF_BOLD),
                                 GTCB_Scaled, TRUE,
                                 TAG_DONE);

        SetGadgetAttr(AslBase,ri,ITALICGAD,GTCB_Checked,(BOOL)(fr->fo_TAttr.tta_Style & FSF_ITALIC),
                                   GTCB_Scaled, TRUE,
                                   TAG_DONE);

        SetGadgetAttr(AslBase,ri, UNDERLINEDGAD,GTCB_Checked,(BOOL)(fr->fo_TAttr.tta_Style & FSF_UNDERLINED),
                                        GTCB_Scaled, TRUE,
                                        TAG_DONE);
    }

    numColors = 2;
    if (ri->ri_Screen)
        numColors = (1 << ri->ri_Screen->BitMap.Depth);

    if (FOF_DOFRONTPEN & fr->fo_Options)
    {
        SetGadgetAttr(AslBase,ri,TEXTGAD,GTPA_NumColors,      numColors > fr->fo_MaxFrontPen ? fr->fo_MaxFrontPen : numColors,
                                 GTPA_IndicatorWidth, 20,
                                 GTPA_Color,          fr->fo_FrontPen,
                                 GTPA_ColorTable,     fr->fo_FrontPens,
                                 TAG_DONE);
    }

    if (FOF_DOBACKPEN & fr->fo_Options)
    {
        SetGadgetAttr(AslBase,ri,FIELDGAD,GTPA_NumColors,      numColors > fr->fo_MaxBackPen ? fr->fo_MaxBackPen : numColors,
                                  GTPA_IndicatorWidth, 20,
                                  GTPA_Color,          fr->fo_BackPen,
                                  GTPA_ColorTable,     fr->fo_BackPens,
                                  TAG_DONE);
    }

    if (FOF_DODRAWMODE & fr->fo_Options)
    {
        SetGadgetAttr(AslBase,ri,MODEGAD,GTCY_Labels, (LONG)&fr->fo_ModeLabels[1],
                                 GTCY_Active, fr->fo_DrawMode,
                                 TAG_DONE);
    }
}


/*****************************************************************************/


static BOOL CreateFOGadgets(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct ReqInfo *ri = &fr->fo_ReqInfo;
struct Gadget  *gad;

    FreeLayoutGadgets(AslBase,ri,TRUE);

    DoFOGadgets(AslBase,fr);

    if (LayoutGadgets(AslBase,ri,LGM_CREATE))
    {
        gad                = ri->ri_Template[DISPLAYGAD].ag_Gadget;
        gad->Flags         = GFLG_GADGHNONE;
        gad->GadgetType   |= GTYP_BOOLGADGET;
        gad->GadgetID      = FO_NOP;

        AddGList(ri->ri_Window,ri->ri_Gadgets,-1,-1,NULL);
        RefreshGList(ri->ri_Gadgets,ri->ri_Window,NULL,-1);
        GT_RefreshWindow(ri->ri_Window,NULL);

        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


static VOID RenderFODisplay(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct ReqInfo *ri = &fr->fo_ReqInfo;
struct Gadget  *gad;

    gad = ri->ri_Template[DISPLAYGAD].ag_Gadget;
    AslDrawBevelBox(AslBase,ri->ri_Window,gad->LeftEdge - ri->ri_Window->BorderLeft,
                                  gad->TopEdge - ri->ri_Window->BorderTop,
                                  gad->Width,
                                  gad->Height,
                                  GT_VisualInfo, ri->ri_VisualInfo,
                                  GTBB_Recessed, TRUE,
                                  TAG_DONE);
}


/*****************************************************************************/


static VOID NewFontName(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct FontNode *font;
struct SizeNode *size;
ULONG            num;

    if (font = (struct FontNode *)FindNameNC(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name))
    {
        fr->fo_CurrentSizes = &font->fn_Sizes;
        size = (struct SizeNode *)fr->fo_CurrentSizes->mlh_Head;
        num  = 0;
        while (size->sn_Node.ln_Succ)
        {
            if (size->sn_Size >= fr->fo_TAttr.tta_YSize)
                break;
            num++;
            size = (struct SizeNode *)size->sn_Node.ln_Succ;
        }

        SetGadgetAttr(AslBase,&fr->fo_ReqInfo,SIZELISTGAD,GTLV_Labels,      fr->fo_CurrentSizes,
                                                  GTLV_Selected,    num,
                                                  GTLV_MakeVisible, num,
                                                  TAG_DONE);
    }
    else
    {
        fr->fo_CurrentSizes = NULL;
        SetGadgetAttr(AslBase,&fr->fo_ReqInfo,SIZELISTGAD,GTLV_Labels, NULL,
                                                  TAG_DONE);
    }
}


/*****************************************************************************/


VOID SetNewFontSize(struct ExtFontReq *fr, LONG newSize)
{
    if (newSize < (LONG)fr->fo_MinHeight)
        newSize = fr->fo_MinHeight;
    else if (newSize > (LONG)fr->fo_MaxHeight)
        newSize = fr->fo_MaxHeight;

    fr->fo_TAttr.tta_YSize = newSize;
}


/*****************************************************************************/


static VOID NewFontSize(struct ASLLib *AslBase, struct ExtFontReq *fr, LONG newSize)
{
    SetNewFontSize(fr,newSize);

    SetGadgetAttr(AslBase, &fr->fo_ReqInfo,SIZEGAD,GTIN_Number, fr->fo_TAttr.tta_YSize,
                                          TAG_DONE);

    NewFontName(AslBase,fr);
}


/*****************************************************************************/


static VOID FindBestName(struct ASLLib *AslBase, struct ExtFontReq *fr, BOOL newList, BOOL newSelected)
{
struct FontNode *font;
ULONG            num;
ULONG            tag;

    num = 0;
    if (font = (struct FontNode *)FindClosest(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name))
        num = FindNodeNum((struct List *)&fr->fo_Fonts,font);

    if (newList)
    {
        SetGadgetAttr(AslBase,&fr->fo_ReqInfo,NAMELISTGAD,GTLV_Labels,     &fr->fo_Fonts,
                                                  GTLV_MakeVisible,num,
                                                  GTLV_Selected,   num,
                                                  TAG_DONE);
    }
    else
    {
        tag = TAG_IGNORE;
        if (newSelected)
            tag = GTLV_Selected;

        SetGadgetAttr(AslBase,&fr->fo_ReqInfo,NAMELISTGAD,GTLV_MakeVisible, num,
                                                  tag,              num,
                                                  TAG_DONE);
    }

    if (newSelected)
        NewFontName(AslBase,fr);
}


/*****************************************************************************/


static VOID FOPreserve(struct ExtFontReq *fr)
{
    strcpy(fr->fo_OriginalName,fr->fo_TAttr.tta_Name);
    fr->fo_OriginalYSize    = fr->fo_TAttr.tta_YSize;
    fr->fo_OriginalStyle    = fr->fo_TAttr.tta_Style;
    fr->fo_OriginalFlags    = fr->fo_TAttr.tta_Flags;
    fr->fo_OriginalFrontPen = fr->fo_FrontPen;
    fr->fo_OriginalBackPen  = fr->fo_BackPen;
    fr->fo_OriginalDrawMode = fr->fo_DrawMode;
}


/*****************************************************************************/


static VOID FORestore(struct ExtFontReq *fr)
{
    strcpy(fr->fo_Name,fr->fo_OriginalName);
    fr->fo_TAttr.tta_YSize = fr->fo_OriginalYSize;
    fr->fo_TAttr.tta_Style = fr->fo_OriginalStyle;
    fr->fo_TAttr.tta_Flags = fr->fo_OriginalFlags;
    fr->fo_FrontPen        = fr->fo_OriginalFrontPen;
    fr->fo_BackPen         = fr->fo_OriginalBackPen;
    fr->fo_DrawMode        = fr->fo_OriginalDrawMode;
}


/*****************************************************************************/


static BOOL HandleFOIDCMP(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
struct ReqInfo       *ri = &fr->fo_ReqInfo;
struct IntuiMessage  *intuiMsg;
ULONG                 class;
UWORD                 icode;
BOOL                  render;
struct Gadget        *gadget;
struct Gadget        *act;
struct Window        *window;
struct MenuItem      *menuItem;
struct SizeNode      *sizeNode;
struct FontNode      *font;
ULONG                 num;
ULONG                 tag;

    while (TRUE)
    {
        intuiMsg = GetReqMsg(AslBase,&fr->fo_ReqInfo,PUBLIC_FO(fr),NULL,0);

        class  = intuiMsg->Class;
        icode  = intuiMsg->Code;
        gadget = (struct Gadget *)intuiMsg->IAddress;
        window = intuiMsg->IDCMPWindow;
        GT_ReplyIMsg(intuiMsg);

        switch (class)
        {
            case IDCMP_REFRESHWINDOW: GT_BeginRefresh(window);
                                      GT_EndRefresh(window,TRUE);
                                      RenderFODisplay(AslBase,fr);
                                      RenderFOFont(AslBase,fr);
                                      break;

            case IDCMP_CLOSEWINDOW  : return(FALSE);

            case IDCMP_MENUPICK     : while (icode != MENUNULL)
                                      {
                                          menuItem = ItemAddress(fr->fo_ReqInfo.ri_Menus,icode);
                                          switch ((UWORD)MENU_USERDATA(menuItem))
                                          {
                                              case FO_CANCEL  : return(FALSE);

                                              case FO_OK      : strcpy(fr->fo_Name,((struct StringInfo *)ri->ri_Template[NAMEGAD].ag_Gadget->SpecialInfo)->Buffer);
                                                                fr->fo_TAttr.tta_YSize = ((struct StringInfo *) (ri->ri_Template[SIZEGAD].ag_Gadget->SpecialInfo))->LongInt;
                                                                /*!!! validate value !!!*/
                                                                return(TRUE);

                                              case FO_RESTORE : FORestore(fr);
                                                                DoFOGadgets(AslBase,fr);
                                                                FindBestName(AslBase,fr,FALSE,TRUE);
                                                                NewFontSize(AslBase,fr,fr->fo_OriginalYSize);
                                                                RenderFOFont(AslBase,fr);
                                                                break;

                                              case FO_RESCAN  : ObtainSemaphore(&AslBase->ASL_FontCacheLock);
                                                                DisposeCacheList(AslBase,&AslBase->ASL_FontCache);
                                                                ReleaseSemaphore(&AslBase->ASL_FontCacheLock);
                                                                SetGadgetAttr(AslBase,&fr->fo_ReqInfo,NAMELISTGAD,GTLV_Labels, NULL,
                                                                                                          TAG_DONE);
                                                                SetGadgetAttr(AslBase,&fr->fo_ReqInfo,SIZELISTGAD,GTLV_Labels, NULL,
                                                                                                          TAG_DONE);
                                                                DisposeFontList(AslBase,fr);
                                                                GetFontList(AslBase,fr);
                                                                FindBestName(AslBase,fr,TRUE,TRUE);
                                                                RenderFOFont(AslBase,fr);
                                                                break;

                                              case FO_PREVFONT: font = (struct FontNode *)FindClosest(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name);
                                                                if (font != (struct FontNode *)fr->fo_Fonts.mlh_Head)
                                                                {
                                                                    font = (struct FontNode *)font->fn_Node.ln_Pred;
                                                                    strcpy(fr->fo_Name,font->fn_Node.ln_Name);
                                                                    FindBestName(AslBase,fr,FALSE,TRUE);
                                                                    RenderFOFont(AslBase,fr);
                                                                }
                                                                break;

                                              case FO_NEXTFONT: font = (struct FontNode *)FindClosest(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name);
                                                                if (font != (struct FontNode *)fr->fo_Fonts.mlh_TailPred)
                                                                {
                                                                    font = (struct FontNode *)font->fn_Node.ln_Succ;
                                                                    strcpy(fr->fo_Name,font->fn_Node.ln_Name);
                                                                    FindBestName(AslBase,fr,FALSE,TRUE);
                                                                    RenderFOFont(AslBase,fr);
                                                                }
                                                                break;

                                              default         : break;
                                          }
                                          icode = menuItem->NextSelect;
                                      }
                                      break;

            case IDCMP_RAWKEY       : if (icode == CURSORUP)
                                      {
                                          font = (struct FontNode *)FindClosest(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name);
                                          if (font != (struct FontNode *)fr->fo_Fonts.mlh_Head)
                                          {
                                              font = (struct FontNode *)font->fn_Node.ln_Pred;
                                              strcpy(fr->fo_Name,font->fn_Node.ln_Name);
                                              FindBestName(AslBase,fr,FALSE,TRUE);
                                              RenderFOFont(AslBase,fr);
                                          }
                                      }
                                      else if (icode == CURSORDOWN)
                                      {
                                          font = (struct FontNode *)FindClosest(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name);
                                          if (font != (struct FontNode *)fr->fo_Fonts.mlh_TailPred)
                                          {
                                              font = (struct FontNode *)font->fn_Node.ln_Succ;
                                              strcpy(fr->fo_Name,font->fn_Node.ln_Name);
                                              FindBestName(AslBase,fr,FALSE,TRUE);
                                              RenderFOFont(AslBase,fr);
                                          }
                                      }
                                      break;

            case IDCMP_GADGETDOWN   :
            case IDCMP_GADGETUP     : act    = NULL;
                                      render = TRUE;
                                      switch ((UWORD)gadget->GadgetID)
                                      {
                                          case FO_CANCEL    : return(FALSE);

                                          case FO_OK        : strcpy(fr->fo_Name,((struct StringInfo *)ri->ri_Template[NAMEGAD].ag_Gadget->SpecialInfo)->Buffer);
                                                              fr->fo_TAttr.tta_YSize = ((struct StringInfo *) (ri->ri_Template[SIZEGAD].ag_Gadget->SpecialInfo))->LongInt;
                                                              /*!!! validate value !!!*/
                                                              return(TRUE);

                                          case FO_NAME      : strcpy(fr->fo_Name,((struct StringInfo *)ri->ri_Template[NAMEGAD].ag_Gadget->SpecialInfo)->Buffer);
                                                              FindBestName(AslBase,fr,FALSE,TRUE);
                                                              act = ri->ri_Template[SIZEGAD].ag_Gadget;
                                                              break;

                                          case FO_NAMELIST  : strcpy(fr->fo_Name,((struct StringInfo *)ri->ri_Template[NAMEGAD].ag_Gadget->SpecialInfo)->Buffer);
                                                              NewFontName(AslBase,fr);
                                                              break;

                                          case FO_SIZE      : NewFontSize(AslBase,fr,((struct StringInfo *) (gadget->SpecialInfo))->LongInt);
                                                              act = ri->ri_Template[NAMEGAD].ag_Gadget;
                                                              break;

                                          case FO_SIZELIST  : sizeNode = (struct SizeNode *)FindNum((struct List *)fr->fo_CurrentSizes,icode);
                                                              NewFontSize(AslBase,fr,sizeNode->sn_Size);
                                                              break;

                                          case FO_TEXT      : fr->fo_FrontPen = icode;
                                                              break;

                                          case FO_FIELD     : fr->fo_BackPen = icode;
                                                              break;

                                          case FO_PLAIN     : fr->fo_TAttr.tta_Style &= ~(FSF_BOLD | FSF_ITALIC | FSF_UNDERLINED);
                                                              SetGadgetAttr(AslBase,ri,PLAINGAD,GTCB_Checked,TRUE,TAG_DONE);
                                                              SetGadgetAttr(AslBase,ri,BOLDGAD,GTCB_Checked,FALSE,TAG_DONE);
                                                              SetGadgetAttr(AslBase,ri,ITALICGAD,GTCB_Checked,FALSE,TAG_DONE);
                                                              SetGadgetAttr(AslBase,ri,UNDERLINEDGAD,GTCB_Checked,FALSE,TAG_DONE);
                                                              break;

                                          case FO_BOLD      :
                                          case FO_ITALIC    :
                                          case FO_UNDERLINED: fr->fo_TAttr.tta_Style &= ~(FSF_BOLD | FSF_ITALIC | FSF_UNDERLINED);
                                                              if (SELECTED & ri->ri_Template[BOLDGAD].ag_Gadget->Flags)
                                                                  fr->fo_TAttr.tta_Style |= FSF_BOLD;

                                                              if (SELECTED & ri->ri_Template[ITALICGAD].ag_Gadget->Flags)
                                                                  fr->fo_TAttr.tta_Style |= FSF_ITALIC;

                                                              if (SELECTED & ri->ri_Template[UNDERLINEDGAD].ag_Gadget->Flags)
                                                                  fr->fo_TAttr.tta_Style |= FSF_UNDERLINED;

                                                              SetGadgetAttr(AslBase,ri,PLAINGAD,GTCB_Checked,(BOOL)(!(fr->fo_TAttr.tta_Style & (FSF_BOLD | FSF_ITALIC | FSF_UNDERLINED))),
                                                                                        TAG_DONE);
                                                              break;

                                          case FO_MODE      : fr->fo_DrawMode = icode;
                                                              render          = FALSE;
                                                              break;

                                      }

                                      if (render)
                                          RenderFOFont(AslBase,fr);

                                      if (act)
                                          ActivateGadget(act,window,NULL);

                                      break;

            case IDCMP_NEWSIZE      : if (!CreateFOGadgets(AslBase,fr))
                                          return(FALSE);

                                      SetGadgetAttr(AslBase,&fr->fo_ReqInfo,NAMEGAD,
                                                    GTST_String, fr->fo_Name,
                                                    TAG_DONE);

                                      if (font = (struct FontNode *)FindNameNC(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name))
                                      {
                                          num = FindNodeNum((struct List *)&fr->fo_Fonts,font);
                                          tag = GTLV_Selected;
                                      }
                                      else
                                      {
                                          tag = TAG_IGNORE;
                                          num = 0;
                                          if (font = (struct FontNode *)FindClosest(AslBase,(struct List *)&fr->fo_Fonts,fr->fo_Name))
                                              num = FindNodeNum((struct List *)&fr->fo_Fonts,font);
                                      }

                                      SetGadgetAttr(AslBase,&fr->fo_ReqInfo,NAMELISTGAD,
                                                    GTLV_MakeVisible, num,
                                                    tag,              num,
                                                    TAG_DONE);

                                      NewFontSize(AslBase,fr, ((struct StringInfo *) (ri->ri_Template[SIZEGAD].ag_Gadget->SpecialInfo))->LongInt);
                                      break;

            case IDCMP_INTUITICKS   : if (SELECTED & ri->ri_Template[NAMEGAD].ag_Gadget->Flags)
                                      {
                                          strcpy(fr->fo_Name,((struct StringInfo *)ri->ri_Template[NAMEGAD].ag_Gadget->SpecialInfo)->Buffer);
                                          FindBestName(AslBase,fr,FALSE,FALSE);
                                      }
                                      break;
        }
    }
}


/*****************************************************************************/


static VOID ProcessFOTags(struct ASLLib *AslBase, struct ExtFontReq *fr, struct TagItem *tagList)
{
struct TagItem *tag;
struct TagItem *tags = tagList;
ULONG           data;

    while (tag = NextTagItem(&tags))
    {
        data = tag->ti_Data;
        switch(tag->ti_Tag)
        {
            case ASLFO_InitialName     : strcpy(fr->fo_Name,(STRPTR)data);
                                         break;

            case ASLFO_InitialSize     : fr->fo_TAttr.tta_YSize = (UWORD)data;
                                         break;

            case ASLFO_InitialStyle    : fr->fo_TAttr.tta_Style = (UBYTE)data;
                                         break;

            case ASLFO_InitialFlags    : fr->fo_TAttr.tta_Flags = (UBYTE)data;
                                         break;

            case ASLFO_InitialFrontPen : fr->fo_FrontPen = (UBYTE)data;
                                         break;

            case ASLFO_InitialBackPen  : fr->fo_BackPen = (UBYTE)data;
                                         break;

            case ASLFO_InitialDrawMode : fr->fo_DrawMode = (UBYTE)data;
                                         break;

            case ASLFO_HookFunc        : fr->fo_ReqInfo.ri_OldHook = (APTR)data;
                                         break;

            case ASLFO_ModeList        : fr->fo_UserModes = (STRPTR *)data;
                                         break;

            case ASLFO_FilterFunc      : fr->fo_FilterFunc = (struct Hook *)data;
                                         break;

            case ASLFO_MinHeight       : fr->fo_MinHeight = (UWORD)data;
                                         break;

            case ASLFO_MaxHeight       : fr->fo_MaxHeight = (UWORD)data;
                                         break;

            case ASLFO_Flags           : fr->fo_Options                  = (ULONG)data;
                                         fr->fo_ReqInfo.ri_CallOldHook   = (BOOL)(data & FOF_INTUIFUNC);
                                         fr->fo_CallOldFilter            = (BOOL)(data & FOF_FILTERFUNC);
                                         fr->fo_ReqInfo.ri_PrivateIDCMP  = (BOOL)(data & FOF_PRIVATEIDCMP);
                                         break;

            case ASLFO_UserData        : fr->fo_UserData = (APTR)data;
                                         break;

            case ASLFO_MaxFrontPen     : fr->fo_MaxFrontPen = (UWORD)data;
                                         break;

            case ASLFO_MaxBackPen      : fr->fo_MaxBackPen = (UWORD)data;
                                         break;

            case ASLFO_FrontPens       : fr->fo_FrontPens = (UBYTE *)data;
                                         break;

            case ASLFO_BackPens        : fr->fo_BackPens = (UBYTE *)data;
                                         break;

            default                    : ProcessStdTag(AslBase,&fr->fo_ReqInfo,tag);
                                         break;
        }
    }

    fr->fo_Options = AslPackBoolTags(AslBase,fr->fo_Options,tagList,
                                     ASLFO_DoStyle,       FOF_DOSTYLE,
                                     ASLFO_DoFrontPen,    FOF_DOFRONTPEN,
                                     ASLFO_DoBackPen,     FOF_DOBACKPEN,
                                     ASLFO_DoDrawMode,    FOF_DODRAWMODE,
                                     ASLFO_FixedWidthOnly,FOF_FIXEDWIDTHONLY,
                                     TAG_DONE);
}


/*****************************************************************************/


struct ExtFontReq *AllocFontRequest(struct ASLLib *AslBase, struct TagItem *tagList)
{
struct ExtFontReq *fr;

    if (fr = AllocVec(sizeof(struct ExtFontReq), MEMF_CLEAR | MEMF_ANY))
    {
        fr->fo_ReqInfo.ri_Coords.Left    = 30;
        fr->fo_ReqInfo.ri_Coords.Top     = 20;
        fr->fo_ReqInfo.ri_Coords.Width   = 318;
        fr->fo_ReqInfo.ri_Coords.Height  = 198;

        fr->fo_ReqInfo.ri_MinWidth    = 313;
        fr->fo_ReqInfo.ri_MinHeight   = 135;

        fr->fo_ReqInfo.ri_TitleID     = MSG_ASL_FO_TITLE;
        fr->fo_ReqInfo.ri_IDCMPFlags  = IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW | IDCMP_INTUITICKS | IDCMP_NEWSIZE | LISTVIEWIDCMP | SCROLLERIDCMP | BUTTONIDCMP | CYCLEIDCMP;
        fr->fo_ReqInfo.ri_WindowFlags = WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIMPLE_REFRESH | WFLG_ACTIVATE | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM;

        fr->fo_FrontPen        = 1;
        fr->fo_BackPen         = 0;
        fr->fo_DrawMode        = JAM1;
        fr->fo_MinHeight       = 5;
        fr->fo_MaxHeight       = 24;
        fr->fo_MaxFrontPen     = 255;
        fr->fo_MaxBackPen      = 255;
        fr->fo_TAttr.tta_Name  = fr->fo_Name;
        fr->fo_TAttr.tta_YSize = 8;
        fr->fo_TAttr.tta_Style = FS_NORMAL | FSF_TAGGED;
        fr->fo_TAttr.tta_Flags = FPF_ROMFONT;

        strcpy(fr->fo_Name,"topaz");

        NewList((struct List *)&fr->fo_Fonts);

        ProcessFOTags(AslBase,fr,tagList);

        fr->fo_Attr.ta_Name  = fr->fo_Name;
        fr->fo_Attr.ta_YSize = fr->fo_TAttr.tta_YSize;
        fr->fo_Attr.ta_Style = fr->fo_TAttr.tta_Style;
        fr->fo_Attr.ta_Flags = fr->fo_TAttr.tta_Flags & ~(FSF_TAGGED);
        fr->fo_Coords        = fr->fo_ReqInfo.ri_Coords;
    }

    return(fr);
}


/*****************************************************************************/


APTR FontRequest(struct ASLLib *AslBase, struct ExtFontReq *fr, struct TagItem *tagList)
{
struct ReqInfo   *ri     = &fr->fo_ReqInfo;
struct ASLLib    *asl    = AslBase;
LONG              error  = ERROR_NO_FREE_STORE;
BOOL              result = FALSE;
STRPTR           *name;
UWORD             i;
APTR              memory;

    if (memory = PrepareRequester(AslBase,ri,AG,sizeof(AG),8))
    {
        ProcessFOTags(AslBase,fr,tagList);
        PrepareLocale(AslBase,&fr->fo_ReqInfo);

        if (!(FOF_DOSTYLE & fr->fo_Options))
        {
            ri->ri_Template[PLAINGAD].ag_LOGroup      = 0;
            ri->ri_Template[BOLDGAD].ag_LOGroup       = 0;
            ri->ri_Template[ITALICGAD].ag_LOGroup     = 0;
            ri->ri_Template[UNDERLINEDGAD].ag_LOGroup = 0;
        }
	else if (!((FOF_DOFRONTPEN | FOF_DOBACKPEN | FOF_DODRAWMODE) & fr->fo_Options))
	{
            ri->ri_Template[ITALICGAD].ag_LOSub       = 1;
            ri->ri_Template[UNDERLINEDGAD].ag_LOSub   = 1;
	}

        if (!(FOF_DOFRONTPEN & fr->fo_Options))
        {
            ri->ri_Template[TEXTGAD].ag_LOGroup = 0;
        }

        if (!(FOF_DOBACKPEN & fr->fo_Options))
        {
            ri->ri_Template[FIELDGAD].ag_LOGroup = 0;
        }

        if (fr->fo_UserModes)
        {
            i    = 0;
            name = fr->fo_UserModes;

            while ((i < 11) && name[i])
            {
                fr->fo_ModeLabels[i] = name[i];
                i++;
            }
            fr->fo_ModeLabels[i] = NULL;
        }
        else
        {
            fr->fo_ModeLabels[0] = GetString(&fr->fo_ReqInfo.ri_LocaleInfo,MSG_ASL_FO_MODE_GAD);
            fr->fo_ModeLabels[4] = NULL;
            for (i=1; i<4; i++)
                fr->fo_ModeLabels[i] = GetString(&fr->fo_ReqInfo.ri_LocaleInfo,i+MSG_ASL_FO_TEXT-1);
        }

        if (FOF_DODRAWMODE & fr->fo_Options)
        {
            /* this is needed in order to get the tags cloned */
            SetGadgetAttr(AslBase,ri,MODEGAD,GTCY_Labels, (LONG)&fr->fo_ModeLabels[1],
                                     GTCY_Active, fr->fo_DrawMode,
                                     TAG_DONE);
        }
        else
        {
            ri->ri_Template[MODEGAD].ag_LOGroup = 0;
        }

        StripDotFont(AslBase,fr->fo_Name);

        FOPreserve(fr);
        fr->fo_CurrentSizes = NULL;

        error = ERROR_INVALID_RESIDENT_LIBRARY;
        if (asl->ASL_DiskfontBase = OpenLibrary("diskfont.library",36))
        {
            error = ERROR_NO_FREE_STORE;
            if (OpenRequester(AslBase,&fr->fo_ReqInfo,AM))
            {
                if (CreateFOGadgets(AslBase,fr))
                {
                    RenderFODisplay(AslBase,fr);

                    if (GetFontList(AslBase,fr))
                    {
                        error = 0;
                        FindBestName(AslBase,fr,TRUE,TRUE);
                        NewFontSize(AslBase,fr,fr->fo_TAttr.tta_YSize);
                        RenderFOFont(AslBase,fr);
                        ActivateGadget(ri->ri_Template[NAMEGAD].ag_Gadget,fr->fo_ReqInfo.ri_Window,NULL);
                        result = HandleFOIDCMP(AslBase,fr);
                        DisposeFontList(AslBase,fr);
                    }
                }
                CloseRequester(AslBase,&fr->fo_ReqInfo);
            }
            CloseLibrary(asl->ASL_DiskfontBase);
        }

        AppendDotFont(AslBase,fr->fo_TAttr.tta_Name);

        fr->fo_Attr.ta_YSize = fr->fo_TAttr.tta_YSize;
        fr->fo_Attr.ta_Style = fr->fo_TAttr.tta_Style & ~(FSF_TAGGED);
        fr->fo_Attr.ta_Flags = fr->fo_TAttr.tta_Flags;

        fr->fo_Coords = ri->ri_Coords;

        if (!result)
            FORestore(fr);

        CleanupRequester(AslBase,ri,memory);
    }

    SetIoErr(error);

    /* kludge to keep old sw running. Turns out old versions of ASL were
     * returning the font name in D0 instead of a BOOL. We keep doing this...
     */
    if (result)
        return(fr->fo_TAttr.tta_Name);

    return(NULL);
}


/*****************************************************************************/


VOID FreeFontRequest(struct ASLLib *AslBase, struct ExtFontReq *fr)
{
    FreeVec(fr);
}


/*****************************************************************************/


VOID FlushFontCaches(struct ASLLib *AslBase)
{
struct ASLLib *asl = AslBase;

    if (AttemptSemaphore(&asl->ASL_FontCacheLock))
    {
        DisposeCacheList(AslBase,&AslBase->ASL_FontCache);
        FreeVec(asl->ASL_AFH);
        AslBase->ASL_AFH = NULL;
        ReleaseSemaphore(&asl->ASL_FontCacheLock);
    }
}
