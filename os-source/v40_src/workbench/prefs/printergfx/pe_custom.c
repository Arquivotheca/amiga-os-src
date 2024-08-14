
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
#include <prefs/printergfx.h>
#include <dos/dos.h>
#include <string.h>

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
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"


#define SysBase ed->ed_SysBase


/*****************************************************************************/


ULONG GetColors(EdDataPtr ed);
ULONG GetMM(EdDataPtr ed, ULONG tenths);
ULONG GetTenths(EdDataPtr ed, ULONG mm);


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_PGFX,
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


#define NEW_PS_COLOR       PS_GREY_SCALE2
#define NEW_PS_GREY_SCALE2 PS_COLOR


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
struct Locale *locale;

    ed->ed_PrefsDefaults.pg_Aspect         = PA_HORIZONTAL;
    ed->ed_PrefsDefaults.pg_Shade          = PS_BW;
    ed->ed_PrefsDefaults.pg_Image          = PI_POSITIVE;
    ed->ed_PrefsDefaults.pg_Threshold      = 7;
    ed->ed_PrefsDefaults.pg_ColorCorrect   = 0;
    ed->ed_PrefsDefaults.pg_Dimensions     = PD_IGNORE;
    ed->ed_PrefsDefaults.pg_Dithering      = PD_ORDERED;
    ed->ed_PrefsDefaults.pg_GraphicFlags   = 0;
    ed->ed_PrefsDefaults.pg_PrintDensity   = 1;
    ed->ed_PrefsDefaults.pg_PrintMaxWidth  = 0;
    ed->ed_PrefsDefaults.pg_PrintMaxHeight = 0;
    ed->ed_PrefsDefaults.pg_PrintXOffset   = 0;
    ed->ed_PrefsDefaults.pg_PrintYOffset   = 0;

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    ed->ed_Metric       = FALSE;

    if (LocaleBase)
    {
        if (locale = OpenLocale(NULL))
        {
            ed->ed_Metric = (locale->loc_MeasuringSystem == MS_ISO);
            CloseLocale(locale);
        }
    }

    return(ES_NORMAL);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (cn->cn_ID != ID_PGFX || cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PrinterGfxPrefs)) == sizeof(struct PrinterGfxPrefs))
    {
        if (ed->ed_PrefsWork.pg_Shade == PS_COLOR)
        {
            ed->ed_PrefsWork.pg_Shade = NEW_PS_COLOR;
        }
        else if (ed->ed_PrefsWork.pg_Shade == PS_GREY_SCALE2)
        {
             ed->ed_PrefsWork.pg_Shade = NEW_PS_GREY_SCALE2;
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
EdStatus result;

    if (ed->ed_PrefsWork.pg_Shade == NEW_PS_COLOR)
    {
        ed->ed_PrefsWork.pg_Shade = PS_COLOR;
    }
    else if (ed->ed_PrefsWork.pg_Shade == NEW_PS_GREY_SCALE2)
    {
        ed->ed_PrefsWork.pg_Shade = PS_GREY_SCALE2;
    }

    result = ES_IFFERROR;
    if (!PushChunk(iff,0,ID_PGFX,sizeof(struct PrinterGfxPrefs)))
        if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PrinterGfxPrefs)) == sizeof(struct PrinterGfxPrefs))
            if (!PopChunk(iff))
                result = ES_NORMAL;

    if (ed->ed_PrefsWork.pg_Shade == PS_COLOR)
    {
         ed->ed_PrefsWork.pg_Shade = NEW_PS_COLOR;
    }
    else if (ed->ed_PrefsWork.pg_Shade == PS_GREY_SCALE2)
    {
         ed->ed_PrefsWork.pg_Shade = NEW_PS_GREY_SCALE2;
    }

    return(result);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     556
#define NW_HEIGHT    176
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP)
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
      {NM_ITEM, MSG_PGFX_OPTIONS_METRIC,         EC_METRIC,    CHECKIT|MENUTOGGLE},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {
    {BUTTON_KIND,   8,   159,  87,  14, MSG_SAVE_GAD,          EC_SAVE,    0},
    {BUTTON_KIND,   234, 159,  87,  14, MSG_USE_GAD,           EC_USE,     0},
    {BUTTON_KIND,   461, 159,  87,  14, MSG_CANCEL_GAD,        EC_CANCEL,  0},

    {CHECKBOX_KIND, 425, 18,   87,  14, MSG_PGFX_RED_GAD,           EC_RED,     0},
    {CHECKBOX_KIND, 425, 30,   87,  14, MSG_PGFX_GREEN_GAD,         EC_GREEN,   0},
    {CHECKBOX_KIND, 425, 42,   87,  14, MSG_PGFX_BLUE_GAD,          EC_BLUE,    0},
    {NUMBER_KIND,   490, 33,   36,  13, MSG_PGFX_COLORS_GAD,        EC_NOP,     PLACETEXT_ABOVE},

    {CYCLE_KIND,    127, 8,    176, 14, MSG_PGFX_DITHERING_GAD,     EC_DITHERING, 0},
    {CYCLE_KIND,    127, 22,   176, 14, MSG_PGFX_SCALING_GAD,       EC_SCALING,   0},
    {CYCLE_KIND,    127, 36,   176, 14, MSG_PGFX_IMAGE_GAD,         EC_IMAGE,     0},
    {CYCLE_KIND,    127, 50,   176, 14, MSG_PGFX_ASPECT_GAD,        EC_ASPECT,    0},
    {CYCLE_KIND,    127, 64,   176, 14, MSG_PGFX_SHADE_GAD,         EC_SHADE,     0},

    {SLIDER_KIND,   151, 84,   152, 11, MSG_PGFX_THRESHOLD_GAD,     EC_THRESHOLD, 0},
    {SLIDER_KIND,   151, 97,   152, 11, MSG_PGFX_DENSITY_GAD,       EC_DENSITY,   0},

    {CHECKBOX_KIND, 127, 117,  87,  14, MSG_PGFX_SMOOTHING_GAD,     EC_SMOOTHING,     PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 127, 131,  87,  14, MSG_PGFX_CENTERPICTURE_GAD, EC_CENTERPICTURE, PLACETEXT_RIGHT},

    {INTEGER_KIND,  494, 65,   54,  14, MSG_NOTHING,           EC_LEFTOFFSET,    0},

    {CYCLE_KIND,    412, 99,   136, 14, MSG_PGFX_TYPE_GAD,          EC_TYPE,     0},
    {INTEGER_KIND,  494, 115,  54,  14, MSG_NOTHING,           EC_WIDTH,    0},
    {INTEGER_KIND,  494, 131,  54,  14, MSG_NOTHING,           EC_HEIGHT,   0}
};


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD              zoomSize[4];
struct LocaleInfo *li;

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    li = &ed->ed_LocaleInfo;
    ed->ed_DitheringLabels[0] = GetString(li,MSG_PGFX_ORDERED);
    ed->ed_DitheringLabels[1] = GetString(li,MSG_PGFX_HALFTONE);
    ed->ed_DitheringLabels[2] = GetString(li,MSG_PGFX_FLOYDSTEINBERG);
    ed->ed_ScalingLabels[0]   = GetString(li,MSG_PGFX_FRACTION);
    ed->ed_ScalingLabels[1]   = GetString(li,MSG_PGFX_INTEGER);
    ed->ed_ImageLabels[0]     = GetString(li,MSG_PGFX_POSITIVE);
    ed->ed_ImageLabels[1]     = GetString(li,MSG_PGFX_NEGATIVE);
    ed->ed_AspectLabels[0]    = GetString(li,MSG_PGFX_HORIZONTAL);
    ed->ed_AspectLabels[1]    = GetString(li,MSG_PGFX_VERTICAL);
    ed->ed_ShadeLabels[0]     = GetString(li,MSG_PGFX_BLACK_AND_WHITE);
    ed->ed_ShadeLabels[1]     = GetString(li,MSG_PGFX_GREY_SCALE_1);
    ed->ed_ShadeLabels[2]     = GetString(li,MSG_PGFX_GREY_SCALE_2);
    ed->ed_ShadeLabels[3]     = GetString(li,MSG_PGFX_COLOR);
    ed->ed_TypeLabels[0]      = GetString(li,MSG_PGFX_IGNORE);
    ed->ed_TypeLabels[1]      = GetString(li,MSG_PGFX_BOUNDED);
    ed->ed_TypeLabels[2]      = GetString(li,MSG_PGFX_ABSOLUTE);
    ed->ed_TypeLabels[3]      = GetString(li,MSG_PGFX_PIXELS);
    ed->ed_TypeLabels[4]      = GetString(li,MSG_PGFX_MULTIPLY);

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
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_PGFX_NAME),
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


ULONG GetColors(EdDataPtr ed)
{
UWORD count;

    count = 0;
    if (ed->ed_PrefsWork.pg_ColorCorrect & PCCF_RED)
        count++;

    if (ed->ed_PrefsWork.pg_ColorCorrect & PCCF_GREEN)
        count++;

    if (ed->ed_PrefsWork.pg_ColorCorrect & PCCF_BLUE)
        count++;

    return((ULONG)(4096-count*308));
}


/*****************************************************************************/


ULONG GetMM(EdDataPtr ed, ULONG tenths)
{
    if (!ed->ed_Metric)
        return(tenths);

    return((tenths*3050000)/1200000);
}


/*****************************************************************************/


ULONG GetTenths(EdDataPtr ed, ULONG mm)
{
    if (!ed->ed_Metric)
        return(mm);

    return((mm*1200000+3049999)/3050000);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y,
                                     SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID PutLine(EdDataPtr ed, struct RastPort *rp, AppStringsID id,
             UWORD x, UWORD y, UWORD w, BOOL center)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    if (center)
        Move(rp,(w-TextLength(rp,str,len)) / 2 + window->BorderLeft + x,
                window->BorderTop+y);
    else
        Move(rp,w-TextLength(rp,str,len) + window->BorderLeft + x,
                window->BorderTop+y);

    Text(rp,str,len);
}


/*****************************************************************************/


VOID RenderWidthHeight(EdDataPtr ed)
{
AppStringsID      width,left;
struct RastPort  *rp;

    rp = window->RPort;
    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
    SetBPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    left = MSG_PGFX_LEFTOFFSET0_GAD;
    if (!ed->ed_Metric)
        left = MSG_PGFX_LEFTOFFSET1_GAD;

    PutLine(ed,rp,left,0,74,486,FALSE);

    if ((ed->ed_PrefsWork.pg_Dimensions == PD_BOUNDED) || (ed->ed_PrefsWork.pg_Dimensions == PD_ABSOLUTE))
    {
        if (ed->ed_Metric)
            width = MSG_PGFX_WIDTH1_GAD;
        else
            width = MSG_PGFX_WIDTH2_GAD;
    }
    else
    {
        width = MSG_PGFX_WIDTH0_GAD+(ed->ed_PrefsWork.pg_Dimensions*2);
    }

    PutLine(ed,rp,width,0,124,486,FALSE);
    PutLine(ed,rp,width+1,0,140,486,FALSE);
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
struct RastPort *rp;

    rp = window->RPort;
    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]);
    SetBPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    PutLine(ed,rp,MSG_PGFX_COLORCORRECT_HDR,307,10,245,TRUE);

    DrawBB(ed,464,33,84,13,GT_VisualInfo,ed->ed_VisualInfo,
                           GTBB_Recessed,TRUE,
                           TAG_DONE);

    PutLine(ed,rp,MSG_PGFX_LIMITS_HDR,307,94,245,TRUE);

    RenderWidthHeight(ed);
}


/*****************************************************************************/


/* gadgets that are affected by the state of the Shade gadget */
VOID RenderShadeGadgets(EdDataPtr ed)
{
    ed->ed_Red = DoPrefsGadget(ed,&EG[3],ed->ed_Red,
                                         GTCB_Checked, ed->ed_PrefsWork.pg_ColorCorrect & PCCF_RED,
                                         GA_Disabled,  ed->ed_PrefsWork.pg_Shade != NEW_PS_COLOR,
                                         TAG_DONE);

    ed->ed_Green = DoPrefsGadget(ed,&EG[4],ed->ed_Green,
                                           GTCB_Checked, ed->ed_PrefsWork.pg_ColorCorrect & PCCF_GREEN,
                                           GA_Disabled,  ed->ed_PrefsWork.pg_Shade != NEW_PS_COLOR,
                                           TAG_DONE);

    ed->ed_Blue = DoPrefsGadget(ed,&EG[5],ed->ed_Blue,
                                          GTCB_Checked, ed->ed_PrefsWork.pg_ColorCorrect & PCCF_BLUE,
                                          GA_Disabled,  ed->ed_PrefsWork.pg_Shade != NEW_PS_COLOR,
                                         TAG_DONE);

    ed->ed_Threshold = DoPrefsGadget(ed,&EG[12],ed->ed_Threshold,
                                                GTSL_Level,       ed->ed_PrefsWork.pg_Threshold,
                                                GTSL_Min,         1,
                                                GTSL_Max,         15,
                                                GTSL_MaxLevelLen, 2,
                                                GTSL_LevelFormat, "%2lu",
                                                GA_RelVerify,     TRUE,
                                                GA_Immediate,     TRUE,
                                                TAG_DONE);
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
ULONG num;

    RenderShadeGadgets(ed);

    ed->ed_Colors = DoPrefsGadget(ed,&EG[6],ed->ed_Colors,
                                            GTNM_Number,GetColors(ed),
                                            TAG_DONE);

    ed->ed_Dithering = DoPrefsGadget(ed,&EG[7],ed->ed_Dithering,
                                               GTCY_Active, ed->ed_PrefsWork.pg_Dithering,
                                               GTCY_Labels, ed->ed_DitheringLabels,
					       TAG_DONE);

    ed->ed_Scaling = DoPrefsGadget(ed,&EG[8],ed->ed_Scaling,
                                             GTCY_Active, (ed->ed_PrefsWork.pg_GraphicFlags & PGFF_INTEGER_SCALING) >> 1,
                                             GTCY_Labels, ed->ed_ScalingLabels,
					     TAG_DONE);

    ed->ed_Image = DoPrefsGadget(ed,&EG[9],ed->ed_Image,
					   GTCY_Active, ed->ed_PrefsWork.pg_Image,
                                           GTCY_Labels, ed->ed_ImageLabels,
                                           TAG_DONE);

    ed->ed_Aspect = DoPrefsGadget(ed,&EG[10],ed->ed_Aspect,
                                             GTCY_Active, ed->ed_PrefsWork.pg_Aspect,
                                             GTCY_Labels, ed->ed_AspectLabels,
					     TAG_DONE);

    ed->ed_Shade = DoPrefsGadget(ed,&EG[11],ed->ed_Shade,
                                            GTCY_Active, ed->ed_PrefsWork.pg_Shade,
                                            GTCY_Labels, ed->ed_ShadeLabels,
					    TAG_DONE);

    ed->ed_Density = DoPrefsGadget(ed,&EG[13],ed->ed_Density,
                                              GTSL_Level,       ed->ed_PrefsWork.pg_PrintDensity,
    					      GTSL_Min,         1,
    					      GTSL_Max,         7,
                                              GTSL_MaxLevelLen, 2,
                                              GTSL_LevelFormat, "%2lu",
                                              GA_RelVerify,     TRUE,
                                              GA_Immediate,     TRUE,
                                              TAG_DONE);

    ed->ed_Smoothing = DoPrefsGadget(ed,&EG[14],ed->ed_Smoothing,
                                                GTCB_Checked, ed->ed_PrefsWork.pg_GraphicFlags & PGFF_ANTI_ALIAS,
                                                GA_Disabled,  ed->ed_PrefsWork.pg_Dithering == PD_FLOYD,
                                                TAG_DONE);

    ed->ed_CenterPicture = DoPrefsGadget(ed,&EG[15],ed->ed_CenterPicture,
                                                    GTCB_Checked,ed->ed_PrefsWork.pg_GraphicFlags & PGFF_CENTER_IMAGE,
                                                    TAG_DONE);

    ed->ed_LeftOffset = DoPrefsGadget(ed,&EG[16],ed->ed_LeftOffset,
                                                 GTIN_Number,   GetMM(ed,ed->ed_PrefsWork.pg_PrintXOffset),
                                                 GA_Disabled,   ed->ed_PrefsWork.pg_GraphicFlags & PGFF_CENTER_IMAGE,
                                                 GTIN_MaxChars, 4,
						 TAG_DONE);

    ed->ed_Type = DoPrefsGadget(ed,&EG[17],ed->ed_Type,
                                           GTCY_Active, ed->ed_PrefsWork.pg_Dimensions,
                                           GTCY_Labels, ed->ed_TypeLabels,
                                           TAG_DONE);

    num = ed->ed_PrefsWork.pg_PrintMaxWidth;
    if ((ed->ed_PrefsWork.pg_Dimensions == PD_BOUNDED) || (ed->ed_PrefsWork.pg_Dimensions == PD_ABSOLUTE))
        num = GetMM(ed,num);

    ed->ed_Width = DoPrefsGadget(ed,&EG[18],ed->ed_Width,
                                            GTIN_Number,   num,
                                            GA_Disabled,   ed->ed_PrefsWork.pg_Dimensions == PD_IGNORE,
                                            GTIN_MaxChars, 4,
                                            TAG_DONE);

    num = ed->ed_PrefsWork.pg_PrintMaxHeight;
    if ((ed->ed_PrefsWork.pg_Dimensions == PD_BOUNDED) || (ed->ed_PrefsWork.pg_Dimensions == PD_ABSOLUTE))
        num = GetMM(ed,num);

    ed->ed_Height = DoPrefsGadget(ed,&EG[19],ed->ed_Height,
                                             GTIN_Number,   num,
                                             GA_Disabled,   ed->ed_PrefsWork.pg_Dimensions == PD_IGNORE,
                                             GTIN_MaxChars, 4,
                                             TAG_DONE);
}


/*****************************************************************************/


VOID ProcessTextGadget(EdDataPtr ed, EdCommand ec, BOOL screenStuff)
{
BOOL           shift;
LONG           num;
struct Gadget *act;
BOOL           beep;

    shift = (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT) & ed->ed_CurrentMsg.Qualifier;
    beep  = FALSE;

    switch (ec)
    {
        case EC_LEFTOFFSET     : num = ((struct StringInfo *)ed->ed_LeftOffset->SpecialInfo)->LongInt;
                                 if (num < 0)
                                 {
                                     num  = 0;
                                     act  = ed->ed_LeftOffset;
                                     beep = TRUE;
                                 }
                                 else if (num > 2539)
                                 {
                                     num  = 2539;
                                     act  = ed->ed_LeftOffset;
                                     beep = TRUE;
                                 }
                                 else
                                 {
                                     if (shift)
                                         act = ed->ed_Height;
                                     else
                                         act = ed->ed_Width;
                                 }

                                 ed->ed_PrefsWork.pg_PrintXOffset = num;

				 if (screenStuff)
                                     SetGadgetAttr(ed,ed->ed_LeftOffset,GTIN_Number,GetMM(ed,num),
                                                                        TAG_DONE);
                                 break;

            case EC_WIDTH      : num = ((struct StringInfo *)ed->ed_Width->SpecialInfo)->LongInt;
                                 if (num < 0)
                                 {
                                     num  = 0;
                                     act  = ed->ed_Width;
                                     beep = TRUE;
                                 }
                                 else if (num > 2539)
                                 {
                                     num  = 2539;
                                     act  = ed->ed_Width;
                                     beep = TRUE;
                                 }
                                 else
                                 {
                                     if (shift)
                                         act = ed->ed_LeftOffset;
                                     else
                                         act = ed->ed_Height;
                                 }

                                 if ((ed->ed_PrefsWork.pg_Dimensions == PD_BOUNDED) || (ed->ed_PrefsWork.pg_Dimensions == PD_ABSOLUTE))
                                 {
                                     num = GetTenths(ed,num);
                                     if (screenStuff)
                                         SetGadgetAttr(ed,ed->ed_Width,GTIN_Number,GetMM(ed,num),
                                                                       TAG_DONE);
                                 }
                                 else
                                 {
                                     if (screenStuff)
                                         SetGadgetAttr(ed,ed->ed_Width,GTIN_Number,num,
                                                                       TAG_DONE);
                                 }
                                 ed->ed_PrefsWork.pg_PrintMaxWidth = num;
                                 break;

        case EC_HEIGHT         : num = ((struct StringInfo *)ed->ed_Height->SpecialInfo)->LongInt;
                                 if (num < 0)
                                 {
                                     num  = 0;
                                     act  = ed->ed_Height;
                                     beep = TRUE;
                                 }
                                 else if (num > 2539)
                                 {
                                     num  = 2539;
                                     act  = ed->ed_Height;
                                     beep = TRUE;
                                 }
                                 else
                                 {
                                     if (shift)
                                         act = ed->ed_Width;
                                     else
                                         act = ed->ed_LeftOffset;
                                 }

                                 if ((ed->ed_PrefsWork.pg_Dimensions == PD_BOUNDED) || (ed->ed_PrefsWork.pg_Dimensions == PD_ABSOLUTE))
                                 {
                                     num = GetTenths(ed,num);
                                     if (screenStuff)
                                         SetGadgetAttr(ed,ed->ed_Height,GTIN_Number,GetMM(ed,num),
                                                                        TAG_DONE);
                                 }
                                 else
                                 {
                                     if (screenStuff)
                                         SetGadgetAttr(ed,ed->ed_Height,GTIN_Number,num,
                                                                        TAG_DONE);
                                 }
                                 ed->ed_PrefsWork.pg_PrintMaxHeight = num;
                                 break;
    }

    if (screenStuff)
    {
        if (!(act->Flags & GFLG_DISABLED))
        {
            ActivateGadget(act,window,NULL);
        }

        if (beep)
        {
            DisplayBeep(window->WScreen);
        }
    }
}


/*****************************************************************************/


VOID SyncTextGadgets(EdDataPtr ed)
{
    ProcessTextGadget(ed,EC_LEFTOFFSET,FALSE);
    ProcessTextGadget(ed,EC_WIDTH,FALSE);
    ProcessTextGadget(ed,EC_HEIGHT,FALSE);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD          icode;
struct Gadget *gadget;

    if (ed->ed_CurrentMsg.Class == IDCMP_MOUSEMOVE)
        return;

    icode  = ed->ed_CurrentMsg.Code;
    gadget = ed->ed_CurrentMsg.IAddress;

    switch (ec)
    {
	case EC_RED            : ed->ed_PrefsWork.pg_ColorCorrect &= ~(PCCF_RED);
	                         if (gadget->Flags & SELECTED)
                                     ed->ed_PrefsWork.pg_ColorCorrect |= PCCF_RED;

                                 SetGadgetAttr(ed,ed->ed_Colors,GTNM_Number,GetColors(ed),
                                                                TAG_DONE);
                                 break;

	case EC_GREEN          : ed->ed_PrefsWork.pg_ColorCorrect &= ~(PCCF_GREEN);
	                         if (gadget->Flags & SELECTED)
                                     ed->ed_PrefsWork.pg_ColorCorrect |= PCCF_GREEN;

                                 SetGadgetAttr(ed,ed->ed_Colors,GTNM_Number,GetColors(ed),
                                                                TAG_DONE);
                                 break;

	case EC_BLUE           : ed->ed_PrefsWork.pg_ColorCorrect &= ~(PCCF_BLUE);
	                         if (gadget->Flags & SELECTED)
                                     ed->ed_PrefsWork.pg_ColorCorrect |= PCCF_BLUE;

                                 SetGadgetAttr(ed,ed->ed_Colors,GTNM_Number,GetColors(ed),
                                                                TAG_DONE);
                                 break;

        case EC_DITHERING      : ed->ed_PrefsWork.pg_Dithering = icode;
                                 if (icode == PD_FLOYD)
                                 {
                                     ed->ed_PrefsWork.pg_GraphicFlags &= ~(PGFF_ANTI_ALIAS);
                                 }

                                 SetGadgetAttr(ed,ed->ed_Smoothing,GA_Disabled,icode == PD_FLOYD,TAG_DONE);
                                 break;

        case EC_SCALING        : if (icode == 0)
                                     ed->ed_PrefsWork.pg_GraphicFlags &= ~PGFF_INTEGER_SCALING;
                                 else
                                     ed->ed_PrefsWork.pg_GraphicFlags |= PGFF_INTEGER_SCALING;
                                 break;

        case EC_IMAGE          : ed->ed_PrefsWork.pg_Image = icode;
                                 break;

        case EC_ASPECT         : ed->ed_PrefsWork.pg_Aspect = icode;
                                 break;

        case EC_SHADE          : ed->ed_PrefsWork.pg_Shade = icode;
                                 RenderShadeGadgets(ed);
                                 break;

        case EC_THRESHOLD      : ed->ed_PrefsWork.pg_Threshold = icode;
                                 break;

        case EC_DENSITY        : ed->ed_PrefsWork.pg_PrintDensity = icode;
                                 break;

        case EC_SMOOTHING      : ed->ed_PrefsWork.pg_GraphicFlags &= ~(PGFF_ANTI_ALIAS);
                                 if (gadget->Flags & SELECTED)
                                     ed->ed_PrefsWork.pg_GraphicFlags |= PGFF_ANTI_ALIAS;
                                 break;

        case EC_CENTERPICTURE  : ed->ed_PrefsWork.pg_GraphicFlags &= ~(PGFF_CENTER_IMAGE);
                                 if (gadget->Flags & SELECTED)
                                     ed->ed_PrefsWork.pg_GraphicFlags |= PGFF_CENTER_IMAGE;

                                 SetGadgetAttr(ed,ed->ed_LeftOffset,GA_Disabled,ed->ed_PrefsWork.pg_GraphicFlags & PGFF_CENTER_IMAGE,TAG_DONE);
                                 break;

        case EC_LEFTOFFSET     :
        case EC_WIDTH          :
        case EC_HEIGHT         : ProcessTextGadget(ed,ec,TRUE);
                                 break;

        case EC_TYPE           : ed->ed_PrefsWork.pg_Dimensions = icode;
    				 SetGadgetAttr(ed,ed->ed_Width,GA_Disabled,ed->ed_PrefsWork.pg_Dimensions == PD_IGNORE,TAG_DONE);
    				 SetGadgetAttr(ed,ed->ed_Height,GA_Disabled,ed->ed_PrefsWork.pg_Dimensions == PD_IGNORE,TAG_DONE);
                                 RenderWidthHeight(ed);
                                 break;

        case EC_METRIC         : ed->ed_Metric = !ed->ed_Metric;
        			 RenderWidthHeight(ed);
        			 RenderGadgets(ed);
				 break;

        default                : break;
    }
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;

    if (ec == EC_METRIC)
        state->cs_Selected = ed->ed_Metric;
}
