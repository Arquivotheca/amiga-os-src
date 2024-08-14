
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
#include <graphics/gfxmacros.h>
#include <graphics/layers.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <prefs/printerps.h>
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
#include <clib/locale_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>
#include <clib/icon_protos.h>
#include <clib/layers_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/layers_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"
#include "conversion.h"
#include "edithook.h"


#define SysBase	ed->ed_SysBase


/*****************************************************************************/


struct PaperSize
{
    LONG Width;
    LONG Height;
};

struct PaperSize sizes[] =
{
    {85 * 72 * 100,      11 * 72 * 1000},	/* 0 : US Letter */
    {85 * 72 * 100,      14 * 72 * 1000},	/* 1 : US Legal */
    {595276,             841890},		/* 2 : DIN A4, 210mm x 297mm */
};


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_PSPD,
};


/*****************************************************************************/


/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,				/* version */
    0,				/* type    */
    0				/* flags   */
};


/*****************************************************************************/


#ifndef GTST_EditHook
#define GTST_EditHook GT_TagBase+55
#endif

#ifndef GTIN_EditHook
#define GTIN_EditHook GT_TagBase+55
#endif


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
EdStatus       result = ES_NO_MEMORY;
struct Locale *locale;

    if (ed->ed_PlanePtr = AllocRaster(RASWIDTH,RASHEIGHT))
    {
	InitArea(&ed->ed_AreaInfo,ed->ed_AreaBuffer,NUMVECTORS);
	InitTmpRas(&ed->ed_TmpRas, ed->ed_PlanePtr,RASSIZE(RASWIDTH,RASHEIGHT));

	ed->ed_PrefsDefaults.ps_DriverMode    = DM_POSTSCRIPT;
	ed->ed_PrefsDefaults.ps_PaperFormat   = 0;		/* Letter */
	ed->ed_PrefsDefaults.ps_Copies        = 1;
	ed->ed_PrefsDefaults.ps_PaperWidth    = 85 * 72 * 100;	/* 8.5 inches */
	ed->ed_PrefsDefaults.ps_PaperHeight   = 11 * 72 * 1000;	/* 11 inches  */
	ed->ed_CurPaperWidth                  = 85 * 72 * 100;	/* 8.5 inches */
	ed->ed_CurPaperHeight                 = 11 * 72 * 1000;	/* 11 inches  */
	ed->ed_PrefsDefaults.ps_HorizontalDPI = 300;
	ed->ed_PrefsDefaults.ps_VerticalDPI   = 300;
	ed->ed_PrefsDefaults.ps_Font          = FONT_COURIER;
	ed->ed_PrefsDefaults.ps_Pitch         = PITCH_NORMAL;
	ed->ed_PrefsDefaults.ps_Orientation   = ORIENT_PORTRAIT;
	ed->ed_PrefsDefaults.ps_Tab           = TAB_INCH;
	ed->ed_PrefsDefaults.ps_LeftMargin    = 10 * 72 * 100;	/* 1.0 inches */
	ed->ed_PrefsDefaults.ps_RightMargin   = 10 * 72 * 100;	/* 1.0 inches */
	ed->ed_PrefsDefaults.ps_TopMargin     = 10 * 72 * 100;	/* 1.0 inches */
	ed->ed_PrefsDefaults.ps_BottomMargin  = 10 * 72 * 100;	/* 1.0 inches */
	ed->ed_PrefsDefaults.ps_FontPointSize = 10 * 1000;
	ed->ed_PrefsDefaults.ps_Leading       = 2  * 1000;
	ed->ed_PrefsDefaults.ps_Aspect        = ASP_HORIZ;
	ed->ed_PrefsDefaults.ps_ScalingType   = ST_ASPECT_BOTH;
	ed->ed_PrefsDefaults.ps_ScalingMath   = SM_FRACTIONAL;
	ed->ed_PrefsDefaults.ps_Centering     = CENT_BOTH;
	ed->ed_PrefsDefaults.ps_LeftEdge      = 10 * 72 * 100;	/* 1.0 inches */
	ed->ed_PrefsDefaults.ps_TopEdge       = 10 * 72 * 100;	/* 1.0 inches */
	ed->ed_PrefsDefaults.ps_Width         = 55 * 72 * 100;	/* 6.5 inches */
	ed->ed_PrefsDefaults.ps_Height        = 90 * 72 * 100;	/* 9 inches */
	ed->ed_PrefsDefaults.ps_Image         = IM_POSITIVE;
	ed->ed_PrefsDefaults.ps_Shading       = SHAD_GREYSCALE;
	ed->ed_PrefsDefaults.ps_Dithering     = DITH_DEFAULT;
	ed->ed_PrefsDefaults.ps_Transparency  = TRANS_WHITE;

	ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
	ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

	ed->ed_FloatHook.h_Entry = FloatHook;

	if (LocaleBase && (locale = OpenLocale (NULL)))
	{
	    ed->ed_CurrentSystem = (locale->loc_MeasuringSystem != MS_ISO) ? 1 : 0;
	    CloseLocale(locale);
	}

	result = ES_NORMAL;
    }

    return(result);
}


/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreeRaster(ed->ed_PlanePtr,RASWIDTH,RASHEIGHT);
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (cn->cn_ID != ID_PSPD || cn->cn_Type != ID_PREF)
	return (ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PrinterPSPrefs)) == sizeof(struct PrinterPSPrefs))
    {
	ed->ed_CurPaperWidth = ed->ed_PrefsDefaults.ps_PaperWidth;
	ed->ed_CurPaperHeight = ed->ed_PrefsDefaults.ps_PaperHeight;
	return(ES_NORMAL);
    }

    return (ES_IFFERROR);
}


EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{

    return (ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    if (!PushChunk(iff,0,ID_PSPD,sizeof(struct PrinterPSPrefs)))
	if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PrinterPSPrefs)) == sizeof(struct PrinterPSPrefs))
	    if (!PopChunk(iff))
		return(ES_NORMAL);

    return(ES_IFFERROR);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{

    return (WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     656
#define NW_HEIGHT    220
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] =
{
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP,       0},
      {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN,      0},
      {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS,    0},
      {NM_ITEM, MSG_NOTHING,                EC_NOP,       0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL,    0},

    {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP,       0},
      {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL,  0},
      {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED, 0},
      {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE,   0},

    {NM_TITLE,   MSG_OPTIONS_MENU,          EC_NOP,       0},
      {NM_ITEM,  MSG_OPTIONS_SAVE_ICONS,    EC_SAVEICONS, CHECKIT | MENUTOGGLE},
      {NM_ITEM,  MSG_PS_OPTIONS_SYSTEM,     EC_CM,        0},
        {NM_SUB, MSG_PS_OPTIONS_CM,         EC_CM,        CHECKIT},
        {NM_SUB, MSG_PS_OPTIONS_INCHES,     EC_INCHES,    CHECKIT},
        {NM_SUB, MSG_PS_OPTIONS_POINTS,     EC_POINTS,    CHECKIT},

    {NM_END, MSG_NOTHING, EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] =
{
    {BUTTON_KIND,  8,   194, 87, 20, MSG_SAVE_GAD,             EC_SAVE,        0},
    {BUTTON_KIND,  279, 194, 87, 20, MSG_USE_GAD,              EC_USE,         0},
    {BUTTON_KIND,  541, 194, 87, 20, MSG_CANCEL_GAD,           EC_CANCEL,      0},

    {CYCLE_KIND,   152, 8,   167, 18, MSG_PS_DRIVERMODE_GAD,   EC_DRIVERMODE,  0},
    {SLIDER_KIND,  176, 28,  111, 11, MSG_PS_COPIES_GAD,       EC_COPIES,      0},
    {CYCLE_KIND,   152, 43,  135, 18, MSG_PS_PAPERFORMAT_GAD,  EC_PAPERFORMAT, 0},
    {STRING_KIND,  152, 63,  135, 20, MSG_PS_PAPERWIDTH_GAD,   EC_PAPERWIDTH,  0},
    {STRING_KIND,  152, 84,  135, 20, MSG_PS_PAPERHEIGHT_GAD,  EC_PAPERHEIGHT,  0},
    {INTEGER_KIND, 152, 105, 135, 20, MSG_PS_HORIZDPI_GAD,     EC_HORIZONTALDPI,0},
    {INTEGER_KIND, 152, 126, 135, 20, MSG_PS_VERTDPI_GAD,      EC_VERTICALDPI, 0},
    {CYCLE_KIND,   381, 8,   223, 18, MSG_NOTHING,             EC_PANEL,       0},

    {CYCLE_KIND,  456, 28,  163, 18, MSG_PS_FONT_GAD,          EC_PRINTFONT,   0},
    {CYCLE_KIND,  456, 47,  163, 18, MSG_PS_PITCH_GAD,         EC_PITCH,       0},
    {CYCLE_KIND,  456, 66,  163, 18, MSG_PS_ORIENTATION_GAD,   EC_ORIENTATION, 0},
    {CYCLE_KIND,  456, 85,  163, 18, MSG_PS_TAB_GAD,           EC_TAB,         0},

    {STRING_KIND, 516, 28,  103, 20, MSG_PS_LEFTMARGIN_GAD,    EC_LEFTMARGIN,  0},
    {STRING_KIND, 516, 49,  103, 20, MSG_PS_RIGHTMARGIN_GAD,   EC_RIGHTMARGIN, 0},
    {STRING_KIND, 516, 70,  103, 20, MSG_PS_TOPMARGIN_GAD,     EC_TOPMARGIN,   0},
    {STRING_KIND, 516, 91,  103, 20, MSG_PS_BOTTOMMARGIN_GAD,  EC_BOTTOMMARGIN, 0},
    {STRING_KIND, 516, 112,  103, 20, MSG_PS_FONTPOINTSIZE_GAD, EC_FONTPOINTSIZE,0},
    {STRING_KIND, 516, 133, 103, 20, MSG_PS_LINELEADING_GAD,   EC_LINELEADING, 0},
    {TEXT_KIND,   516, 154, 103, 18, MSG_PS_LINESPERINCH_GAD,  EC_LINESPERINCH,0},
    {TEXT_KIND,   516, 173, 103, 18, MSG_PS_LINESPERPAGE_GAD,  EC_LINESPERPAGE,0},

    {STRING_KIND, 516, 28,  103, 20, MSG_PS_LEFTEDGE_GAD,      EC_LEFTEDGE,    0},
    {STRING_KIND, 516, 49,  103, 20, MSG_PS_TOPEDGE_GAD,       EC_TOPEDGE,     0},
    {STRING_KIND, 516, 70,  103, 20, MSG_PS_WIDTH_GAD,         EC_WIDTH,       0},
    {STRING_KIND, 516, 91,  103, 20, MSG_PS_HEIGHT_GAD,        EC_HEIGHT,      0},
    {CYCLE_KIND,  456, 112,  163, 18, MSG_PS_IMAGE_GAD,         EC_IMAGE,       0},
    {CYCLE_KIND,  456, 131, 163, 18, MSG_PS_SHADING_GAD,       EC_SHADING,     0},
    {CYCLE_KIND,  456, 150, 163, 18, MSG_PS_DITHERING_GAD,     EC_DITHERING,   0},
    {CYCLE_KIND,  456, 169, 163, 18, MSG_PS_TRANSPARENT_GAD,   EC_TRANSPARENT, 0},

    {CYCLE_KIND,  456, 28, 163, 18, MSG_PS_ASPECT_GAD,         EC_ASPECT,      0},
    {CYCLE_KIND,  456, 47, 163, 18, MSG_PS_SCALINGTYPE_GAD,    EC_SCALINGTYPE, 0},
    {CYCLE_KIND,  456, 66, 163, 18, MSG_PS_SCALINGMATH_GAD,    EC_SCALINGMATH, 0},
    {CYCLE_KIND,  456, 85, 163, 18, MSG_PS_CENTERING_GAD,      EC_CENTERING,   0}
};


/*****************************************************************************/


VOID InitLabels(EdDataPtr ed, STRPTR *ptr, ULONG arraySize, ULONG start)
{
    arraySize = (arraySize / 4);
    while (--arraySize)
	*ptr++ = GetString (&ed->ed_LocaleInfo, start++);
}


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD            zoomSize[4];
struct MenuItem *item1;
struct MenuItem *item2;
struct MenuItem *item3;

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    InitLabels(ed,&ed->ed_DriverModeLabels[0],  sizeof(ed->ed_DriverModeLabels),  MSG_PS_MODE_1);
    InitLabels(ed,&ed->ed_PaperFormatLabels[0], sizeof(ed->ed_PaperFormatLabels), MSG_PS_FORMAT_1);
    InitLabels(ed,&ed->ed_PanelLabels[0],       sizeof(ed->ed_PanelLabels),       MSG_PS_PANEL_1);

    InitLabels(ed,&ed->ed_PrintFontLabels[0],   sizeof(ed->ed_PrintFontLabels),   MSG_PS_FONT_1);
    InitLabels(ed,&ed->ed_PitchLabels[0],       sizeof(ed->ed_PitchLabels),       MSG_PS_PITCH_1);
    InitLabels(ed,&ed->ed_OrientationLabels[0], sizeof(ed->ed_OrientationLabels), MSG_PS_ORIENTATION_1);
    InitLabels(ed,&ed->ed_TabLabels[0],         sizeof(ed->ed_TabLabels),         MSG_PS_TAB_1);

    InitLabels(ed,&ed->ed_AspectLabels[0],      sizeof(ed->ed_AspectLabels),      MSG_PS_ASPECT_1);
    InitLabels(ed,&ed->ed_ScalingTypeLabels[0], sizeof(ed->ed_ScalingTypeLabels), MSG_PS_SCALINGTYPE_1);
    InitLabels(ed,&ed->ed_ScalingMathLabels[0], sizeof(ed->ed_ScalingMathLabels), MSG_PS_SCALINGMATH_1);
    InitLabels(ed,&ed->ed_CenteringLabels[0],   sizeof(ed->ed_CenteringLabels),   MSG_PS_CENTERING_1);

    InitLabels(ed,&ed->ed_ImageLabels[0],       sizeof(ed->ed_ImageLabels),       MSG_PS_IMAGE_1);
    InitLabels(ed,&ed->ed_ShadingLabels[0],     sizeof(ed->ed_ShadingLabels),     MSG_PS_SHADING_1);
    InitLabels(ed,&ed->ed_DitheringLabels[0],   sizeof(ed->ed_DitheringLabels),   MSG_PS_DITHERING_1);
    InitLabels(ed,&ed->ed_TransparentLabels[0], sizeof(ed->ed_TransparentLabels), MSG_PS_TRANSPARENT_1);

    RenderGadgets(ed);

    if ((ed->ed_LastAdded)
    && (ed->ed_Menus = CreatePrefsMenus(ed,EM))
    && (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,   NW_WIDTH,
                                           WA_InnerHeight,  NW_HEIGHT,
                                           WA_MinWidth,     NW_MINWIDTH,
                                           WA_MinHeight,    NW_MINHEIGHT,
                                           WA_MaxWidth,     NW_MAXWIDTH,
                                           WA_MaxHeight,    NW_MAXHEIGHT,
                                           WA_IDCMP,        NW_IDCMP,
                                           WA_Flags,        NW_FLAGS,
                                           WA_Zoom,         zoomSize,
                                           WA_AutoAdjust,   TRUE,
                                           WA_PubScreen,    ed->ed_Screen,
                                           WA_Title,        GetString(&ed->ed_LocaleInfo, MSG_PS_NAME),
                                           WA_NewLookMenus, TRUE,
                                           WA_Gadgets,      ed->ed_Gadgets,
                                           TAG_DONE)))
    {
	if (ed->ed_Menus)
	{
            item1 = ItemAddress(ed->ed_Menus,FULLMENUNUM(2,1,0));
            item2 = ItemAddress(ed->ed_Menus,FULLMENUNUM(2,1,1));
            item3 = ItemAddress(ed->ed_Menus,FULLMENUNUM(2,1,2));

            item1->MutualExclude  = 6;
            item2->MutualExclude  = 5;
            item3->MutualExclude  = 3;

	    item1->Flags &= ~CHECKED;
	    item2->Flags &= ~CHECKED;
	    item3->Flags &= ~CHECKED;

	    if (ed->ed_CurrentSystem == 0)
		item1->Flags |= CHECKED;
	    else if (ed->ed_CurrentSystem == 1)
		item2->Flags |= CHECKED;
	    else
		item3->Flags |= CHECKED;
	}

	ed->ed_Window->RPort->AreaInfo = &ed->ed_AreaInfo;
	ed->ed_Window->RPort->TmpRas   = &ed->ed_TmpRas;

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


UWORD far pattern[] = {0xaaaa, 0x5555};


VOID RenderBox(EdDataPtr ed,
               UWORD areaLeft, UWORD areaTop, UWORD areaWidth, UWORD areaHeight,
               UWORD boxWidth, UWORD boxHeight,
               UWORD xScale, UWORD yScale,
	       BOOL centerX, BOOL centerY,
	       BOOL sideways, BOOL blankPage, BOOL stacked)
{
BOOL inrefresh = (window->Flags & WINDOWREFRESH) ? TRUE : FALSE;
struct RastPort *rp;
struct DrawInfo *di;
UWORD left, top, width, height;
struct Region *newRegion;
struct Region *oldRegion;
struct Rectangle Rect;
UWORD i;

    rp = ed->ed_Window->RPort;
    di = ed->ed_DrawInfo;

    areaLeft += window->BorderLeft;
    areaTop += window->BorderTop;

    for (i = 0; i < 2; i++)
    {
        if (!blankPage)
            i = 1;

        if (i == 1)
        {
            if (stacked)
                areaTop += areaHeight + 2;
            else
                areaLeft += areaWidth + 4;
        }

        if (i == 1)
        {
            if (newRegion = NewRegion ())
            {
                left = areaLeft;
                top = areaTop;
                width = boxWidth;
                height = boxHeight;

                Rect.MinX = areaLeft;
                Rect.MinY = areaTop;
                Rect.MaxX = areaLeft + areaWidth - 1;
                Rect.MaxY = areaTop + areaHeight - 1;

                if (OrRectRegion (newRegion, &Rect))
                {
                    if (inrefresh)
                        EndRefresh (window, FALSE);
                    oldRegion = InstallClipRegion (rp->Layer, newRegion);
                    if (inrefresh)
                        BeginRefresh (window);

                    if (centerX)
                        left += (areaWidth - boxWidth) / 2;

                    if (centerY)
                        top += (areaHeight - boxHeight) / 2;

                    if ((left != areaLeft) || (top != areaTop) || (left + width != areaLeft + areaWidth) || (top + height != areaTop + areaHeight))
                    {
                        SetAPen(rp,di->dri_Pens[FILLPEN]);
                        SetAfPt(rp,pattern,1);
                        RectFill(rp,areaLeft,areaTop,areaLeft + areaWidth - 1,areaTop + areaHeight - 1);
                        SetAfPt(rp,NULL,0);
                    }

                    if (window->WScreen->BitMap.Depth == 1)
                        SetAPen(rp, di->dri_Pens[BACKGROUNDPEN]);
                    else
                        SetAPen(rp, di->dri_Pens[SHADOWPEN]);

                    RectFill(rp, left, top, left + width - 1, top + height - 1);

                    left += xScale;
                    top += yScale;
                    width -= (xScale * 2);
                    height -= (yScale * 2);

                    SetAPen(rp, di->dri_Pens[SHINEPEN]);
                    RectFill(rp, left, top, left + width - 1, top + height - 1);

                    left += xScale;
                    top += yScale;
                    width -= (xScale * 2);
                    height -= (yScale * 2);

                    if (window->WScreen->BitMap.Depth == 1)
                        SetAPen(rp, di->dri_Pens[BACKGROUNDPEN]);
                    else
                        SetAPen(rp, di->dri_Pens[SHADOWPEN]);

                    if (sideways)
                    {
                        Move (rp, left, top + height - 1);
                        Draw (rp, left + width - 1, top);
                        Draw (rp, left + width - 1, top + height - 1);
                        Draw (rp, left, top);

                        AreaMove (rp, left, top);   /* > */
                        AreaDraw (rp, left + (width >> 1) - 1, top + (height >> 1) - 1);
                        AreaDraw (rp, left, top + height - 1);
                    }
                    else
                    {
                        Move (rp, left, top);
                        Draw (rp, left + width - 1, top + height - 1);
                        Draw (rp, left, top + height - 1);
                        Draw (rp, left + width - 1, top);

                        AreaMove (rp, left, top);   /* V */
                        AreaDraw (rp, left + (width >> 1) - 1, top + (height >> 1) - 1);
                        AreaDraw (rp, left + width - 1, top);
                    }
                    AreaEnd (rp);

                    if (inrefresh)
                        EndRefresh (window, FALSE);
                    InstallClipRegion (rp->Layer, oldRegion);
                    if (inrefresh)
                        BeginRefresh (window);
                }
                DisposeRegion (newRegion);
            }
        }
        else
        {
            SetAPen(rp,di->dri_Pens[FILLPEN]);
            SetAfPt(rp,pattern,1);
            RectFill(rp,areaLeft,areaTop,areaLeft + areaWidth - 1,areaTop + areaHeight - 1);
            SetAfPt(rp,NULL,0);
        }
    }
}


/*****************************************************************************/


struct Dimensions
{
    UWORD Width;
    UWORD Height;
};

struct Dimensions samples[] =
{
 /* 14,14    42,20    20,42    42,42					 */
    {24, 24},
    {24, 24},
    {24, 24},
    {24, 24},			/* None		 */
    {14, 14},
    {42, 42},
    {20, 20},
    {42, 42},			/* Aspect : Width	 */
    {14, 14},
    {20, 20},
    {42, 42},
    {42, 42},			/* Aspect : Height	 */
    {14, 14},
    {20, 20},
    {20, 20},
    {42, 42},			/* Aspect : Both	 */
    {14, 14},
    {42, 20},
    {20, 20},
    {42, 42},			/* Fits : Width	 */
    {14, 14},
    {20, 20},
    {20, 42},
    {42, 42},			/* Fits : Height	 */
    {14, 14},
    {42, 20},
    {20, 42},
    {42, 42}			/* Fits : Both	 */
};


/*****************************************************************************/


VOID CenterLine(EdDataPtr ed, struct RastPort * rp, AppStringsID id,
		UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(rp,(w - TextLength(rp,str,len)) / 2 + window->BorderLeft + x,
	 window->BorderTop + y);
    Text(rp,str,len);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, UWORD x, UWORD y, UWORD w, UWORD h, ULONG tag,...)
{
    DrawBevelBoxA(ed->ed_Window->RPort, x + window->BorderLeft, y + window->BorderTop, w, h, (struct TagItem *) & tag);
}


/*****************************************************************************/


VOID DoRenderDisplay(EdDataPtr ed, BOOL complete)
{
BOOL               centerX, centerY, sideways;
struct Dimensions *dims;
struct RastPort   *rp;
UWORD              borLeft;
UWORD              borTop;

    if (ed->ed_CurrentPanel == 3)
    {
	dims = &samples[ed->ed_PrefsWork.ps_ScalingType * 4];

	centerX  = (ed->ed_PrefsWork.ps_Centering == CENT_HORIZ) || (ed->ed_PrefsWork.ps_Centering == CENT_BOTH);
	centerY  = (ed->ed_PrefsWork.ps_Centering == CENT_VERT) || (ed->ed_PrefsWork.ps_Centering == CENT_BOTH);
	sideways = (ed->ed_PrefsWork.ps_Aspect == ASP_VERT);
	rp       = window->RPort;
	borLeft  = window->BorderLeft;
	borTop   = window->BorderTop;

	SetAPen(rp,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);

        if (complete)
        {
            DrawBB(ed,375,127,243,50,GTBB_Recessed, TRUE,
                                     GT_VisualInfo, ed->ed_VisualInfo,
                                     TAG_DONE);

            CenterLine(ed,rp,MSG_PS_SAMPLEPICTURE_HDR,294,120,68);
            CenterLine(ed,rp,MSG_PS_SAMPLESCALING_HDR,375,120,243);

            RectFill(rp, 349+borLeft, 134+borTop, 361+borLeft, 137+borTop);
            RectFill(rp, 362+borLeft, 131+borTop, 362+borLeft, 140+borTop);
            RectFill(rp, 363+borLeft, 132+borTop, 363+borLeft, 139+borTop);
            RectFill(rp, 364+borLeft, 133+borTop, 364+borLeft, 138+borTop);
            RectFill(rp, 365+borLeft, 134+borTop, 365+borLeft, 137+borTop);
            RectFill(rp, 366+borLeft, 135+borTop, 366+borLeft, 136+borTop);

            SetAfPt(rp,pattern,1);
            RectFill(rp, 416+borLeft, 128+borTop, 416+borLeft, 175+borTop);
            RectFill(rp, 467+borLeft, 127+borTop, 467+borLeft, 174+borTop);
            RectFill(rp, 520+borLeft, 128+borTop, 520+borLeft, 175+borTop);
            SetAfPt(rp,NULL,0);

            RenderBox(ed,284,130,24,24,24,24,1,1,TRUE,TRUE,FALSE,FALSE,FALSE);
        }

	RenderBox(ed,380,145,14,14,dims[0].Width,dims[0].Height,1,1,centerX,centerY,sideways,complete,FALSE);
	RenderBox(ed,421,131,42,20,dims[1].Width,dims[1].Height,1,1,centerX,centerY,sideways,complete,TRUE);
	RenderBox(ed,472,131,20,42,dims[2].Width,dims[2].Height,1,1,centerX,centerY,sideways,complete,FALSE);
	RenderBox(ed,525,131,42,42,dims[3].Width,dims[3].Height,1,1,centerX,centerY,sideways,complete,FALSE);
    }
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
    DoRenderDisplay(ed,TRUE);
}


/*****************************************************************************/


VOID UpdateTextDisplay(EdDataPtr ed)
{
    if (ed->ed_CurrentPanel == 1)
    {
        sprintf(ed->ed_LinesPerInchBuf,"%ld",72000 / (ed->ed_PrefsWork.ps_Leading+ed->ed_PrefsWork.ps_FontPointSize));
        ed->ed_LinesPerInch = DoPrefsGadget(ed,&EG[21],ed->ed_LinesPerInch,
                                            GTTX_Text,   ed->ed_LinesPerInchBuf,
                                            GTTX_Border, TRUE,
                                            TAG_DONE);

        sprintf(ed->ed_LinesPerPageBuf,"%ld",(ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_BottomMargin - ed->ed_PrefsWork.ps_TopMargin) / (ed->ed_PrefsWork.ps_Leading+ed->ed_PrefsWork.ps_FontPointSize));
        ed->ed_LinesPerPage = DoPrefsGadget(ed,&EG[22],ed->ed_LinesPerPage,
                                            GTTX_Text,   ed->ed_LinesPerPageBuf,
                                            GTTX_Border, TRUE,
                                            TAG_DONE);
    }
}


/*****************************************************************************/


VOID CopyPrefs(EdDataPtr ed, struct PrinterPSPrefs *p1, struct PrinterPSPrefs *p2)
{
    *p1 = *p2;
    DoRenderDisplay(ed,FALSE);
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
UWORD i;

    for (i = 0; i < 3; i++)
    {
        if ((ed->ed_PrefsWork.ps_PaperWidth == sizes[i].Width) &&
            (ed->ed_PrefsWork.ps_PaperHeight == sizes[i].Height))
            break;
    }

    if (!ed->ed_LastAdded)
    {
	ed->ed_LastAdded = CreateContext (&ed->ed_Gadgets);
	DoPrefsGadget (ed, &EG[0], NULL, TAG_DONE);
	DoPrefsGadget (ed, &EG[1], NULL, TAG_DONE);
	DoPrefsGadget (ed, &EG[2], NULL, TAG_DONE);

	ed->ed_DriverMode    = NULL;
	ed->ed_Copies        = NULL;
	ed->ed_PaperFormat   = NULL;
	ed->ed_PaperWidth    = NULL;
	ed->ed_PaperHeight   = NULL;
	ed->ed_HorizontalDPI = NULL;
	ed->ed_VerticalDPI   = NULL;
	ed->ed_Panel         = NULL;
	ed->ed_PrintFont     = NULL;
	ed->ed_Pitch         = NULL;
	ed->ed_Orientation   = NULL;
	ed->ed_Tab           = NULL;
	ed->ed_LeftMargin    = NULL;
	ed->ed_RightMargin   = NULL;
	ed->ed_TopMargin     = NULL;
	ed->ed_BottomMargin  = NULL;
	ed->ed_FontPointSize = NULL;
	ed->ed_LineLeading   = NULL;
	ed->ed_LinesPerInch  = NULL;
	ed->ed_LinesPerPage  = NULL;
	ed->ed_Aspect        = NULL;
	ed->ed_ScalingType   = NULL;
	ed->ed_ScalingMath   = NULL;
	ed->ed_Centering     = NULL;
	ed->ed_LeftEdge      = NULL;
	ed->ed_TopEdge       = NULL;
	ed->ed_Width         = NULL;
	ed->ed_Height        = NULL;
	ed->ed_Image         = NULL;
	ed->ed_Shading       = NULL;
	ed->ed_Dithering     = NULL;
	ed->ed_Transparent   = NULL;
    }

    ed->ed_DriverMode = DoPrefsGadget(ed,&EG[3],ed->ed_DriverMode,
				      GTCY_Labels, ed->ed_DriverModeLabels,
				      GTCY_Active, ed->ed_PrefsWork.ps_DriverMode,
				      TAG_DONE);

    ed->ed_Copies = DoPrefsGadget(ed,&EG[4],ed->ed_Copies,
				  GTSL_Min,         1,
				  GTSL_Max,         99,
				  GTSL_Level,       ed->ed_PrefsWork.ps_Copies,
				  GTSL_MaxLevelLen, 2,
				  GTSL_LevelFormat, "%2lu",
				  GA_RelVerify,     TRUE,
				  GA_Immediate,     TRUE,
				  TAG_DONE);

    ed->ed_PaperFormat = DoPrefsGadget(ed,&EG[5],ed->ed_PaperFormat,
				       GTCY_Labels, ed->ed_PaperFormatLabels,
				       GTCY_Active, ed->ed_PrefsWork.ps_PaperFormat,
				       TAG_DONE);

    if (ed->ed_PaperWidth = DoPrefsGadget(ed,&EG[6],ed->ed_PaperWidth,
					  GA_Disabled,   ed->ed_PrefsWork.ps_PaperFormat != 3,
					  GTST_String,   FromMille(ed,ed->ed_PrefsWork.ps_PaperWidth,2),
					  GTST_EditHook, &ed->ed_FloatHook,
					  TAG_DONE))
	ed->ed_PaperWidth->UserData = (APTR) 2;

    if (ed->ed_PaperHeight = DoPrefsGadget(ed,&EG[7],ed->ed_PaperHeight,
					   GA_Disabled,   ed->ed_PrefsWork.ps_PaperFormat != 3,
					   GTST_String,   FromMille(ed,ed->ed_PrefsWork.ps_PaperHeight,2),
					   GTST_EditHook, &ed->ed_FloatHook,
					   TAG_DONE))
	ed->ed_PaperHeight->UserData = (APTR) 2;

    ed->ed_HorizontalDPI = DoPrefsGadget(ed,&EG[8],ed->ed_HorizontalDPI,
					 GTIN_Number, ed->ed_PrefsWork.ps_HorizontalDPI,
					 TAG_DONE);

    ed->ed_VerticalDPI = DoPrefsGadget(ed,&EG[9],ed->ed_VerticalDPI,
				       GTIN_Number, ed->ed_PrefsWork.ps_VerticalDPI,
				       TAG_DONE);

    ed->ed_Panel = DoPrefsGadget(ed,&EG[10],ed->ed_Panel,
				 GTCY_Labels, ed->ed_PanelLabels,
				 GTCY_Active, ed->ed_CurrentPanel,
				 TAG_DONE);

    if (ed->ed_CurrentPanel == 0)
    {
	ed->ed_PrintFont = DoPrefsGadget(ed,&EG[11],ed->ed_PrintFont,
					 GTCY_Labels, ed->ed_PrintFontLabels,
					 GTCY_Active, ed->ed_PrefsWork.ps_Font,
					 TAG_DONE);

	ed->ed_Pitch = DoPrefsGadget(ed,&EG[12],ed->ed_Pitch,
				     GTCY_Labels, ed->ed_PitchLabels,
				     GTCY_Active, ed->ed_PrefsWork.ps_Pitch,
				     TAG_DONE);

	ed->ed_Orientation = DoPrefsGadget(ed,&EG[13],ed->ed_Orientation,
					   GTCY_Labels, ed->ed_OrientationLabels,
					   GTCY_Active, ed->ed_PrefsWork.ps_Orientation,
					   TAG_DONE);

	ed->ed_Tab = DoPrefsGadget(ed,&EG[14],ed->ed_Tab,
				   GTCY_Labels, ed->ed_TabLabels,
				   GTCY_Active, ed->ed_PrefsWork.ps_Tab,
				   TAG_DONE);
    }
    else if (ed->ed_CurrentPanel == 1)
    {
	if (ed->ed_LeftMargin = DoPrefsGadget(ed,&EG[15],ed->ed_LeftMargin,
					      GTST_String,   FromMille(ed,ed->ed_PrefsWork.ps_LeftMargin,2),
					      GTST_EditHook, &ed->ed_FloatHook,
					      TAG_DONE))
	    ed->ed_LeftMargin->UserData = (APTR) 2;

	if (ed->ed_RightMargin = DoPrefsGadget(ed,&EG[16],ed->ed_RightMargin,
					       GTST_String,   FromMille(ed,ed->ed_PrefsWork.ps_RightMargin,2),
					       GTST_EditHook, &ed->ed_FloatHook,
					       TAG_DONE))
	    ed->ed_RightMargin->UserData = (APTR) 2;


	if (ed->ed_TopMargin = DoPrefsGadget(ed,&EG[17],ed->ed_TopMargin,
					     GTST_String,   FromMille (ed, ed->ed_PrefsWork.ps_TopMargin, 2),
					     GTST_EditHook, &ed->ed_FloatHook,
					     TAG_DONE))
	    ed->ed_TopMargin->UserData = (APTR) 2;

	if (ed->ed_BottomMargin = DoPrefsGadget(ed,&EG[18],ed->ed_BottomMargin,
						GTST_String,   FromMille (ed, ed->ed_PrefsWork.ps_BottomMargin, 2),
						GTST_EditHook, &ed->ed_FloatHook,
						TAG_DONE))
	    ed->ed_BottomMargin->UserData = (APTR) 2;

	if (ed->ed_FontPointSize = DoPrefsGadget(ed,&EG[19],ed->ed_FontPointSize,
						 GTST_String,   FromMilleNC (ed, ed->ed_PrefsWork.ps_FontPointSize, 1),
						 GTST_EditHook, &ed->ed_FloatHook,
						 TAG_DONE))
	    ed->ed_FontPointSize->UserData = (APTR) 1;

	if (ed->ed_LineLeading = DoPrefsGadget(ed,&EG[20],ed->ed_LineLeading,
					       GTST_String,   FromMilleNC (ed, ed->ed_PrefsWork.ps_Leading, 1),
					       GTST_EditHook, &ed->ed_FloatHook,
					       TAG_DONE))
	    ed->ed_LineLeading->UserData = (APTR) 1;

	UpdateTextDisplay(ed);
    }
    else if (ed->ed_CurrentPanel == 2)
    {
	if (ed->ed_LeftEdge = DoPrefsGadget(ed,&EG[23],ed->ed_LeftEdge,
					    GTST_String,   FromMille (ed, ed->ed_PrefsWork.ps_LeftEdge, 2),
					    GTST_EditHook, &ed->ed_FloatHook,
					    TAG_DONE))
	    ed->ed_LeftEdge->UserData = (APTR) 2;


	if (ed->ed_TopEdge = DoPrefsGadget(ed, &EG[24],ed->ed_TopEdge,
					   GTST_String,   FromMille (ed, ed->ed_PrefsWork.ps_TopEdge, 2),
					   GTST_EditHook, &ed->ed_FloatHook,
					   TAG_DONE))
	    ed->ed_TopEdge->UserData = (APTR) 2;

	if (ed->ed_Width = DoPrefsGadget(ed,&EG[25],ed->ed_Width,
				         GTST_String,   FromMille (ed, ed->ed_PrefsWork.ps_Width, 2),
					 GTST_EditHook, &ed->ed_FloatHook,
					 TAG_DONE))
	    ed->ed_Width->UserData = (APTR) 2;

	if (ed->ed_Height = DoPrefsGadget(ed,&EG[26],ed->ed_Height,
				          GTST_String,   FromMille (ed, ed->ed_PrefsWork.ps_Height, 2),
					  GTST_EditHook, &ed->ed_FloatHook,
					  TAG_DONE))
	    ed->ed_Height->UserData = (APTR) 2;

	ed->ed_Image = DoPrefsGadget(ed,&EG[27],ed->ed_Image,
				     GTCY_Labels, ed->ed_ImageLabels,
				     GTCY_Active, ed->ed_PrefsWork.ps_Image,
				     TAG_DONE);

	ed->ed_Shading = DoPrefsGadget(ed,&EG[28],ed->ed_Shading,
				       GTCY_Labels, ed->ed_ShadingLabels,
				       GTCY_Active, ed->ed_PrefsWork.ps_Shading,
				       TAG_DONE);

	ed->ed_Dithering = DoPrefsGadget(ed,&EG[29],ed->ed_Dithering,
					 GA_Disabled, ed->ed_PrefsWork.ps_Shading != SHAD_GREYSCALE,
					 GTCY_Labels, ed->ed_DitheringLabels,
					 GTCY_Active, ed->ed_PrefsWork.ps_Dithering,
					 TAG_DONE);

	ed->ed_Transparent = DoPrefsGadget(ed,&EG[30],ed->ed_Transparent,
					 GA_Disabled, ed->ed_PrefsWork.ps_Shading == SHAD_BW,
					 GTCY_Labels, ed->ed_TransparentLabels,
					 GTCY_Active, ed->ed_PrefsWork.ps_Transparency,
					 TAG_DONE);
    }
    else if (ed->ed_CurrentPanel == 3)
    {
	ed->ed_Aspect = DoPrefsGadget(ed,&EG[31],ed->ed_Aspect,
				      GTCY_Labels, ed->ed_AspectLabels,
				      GTCY_Active, ed->ed_PrefsWork.ps_Aspect,
				      TAG_DONE);

	ed->ed_ScalingType = DoPrefsGadget(ed,&EG[32],ed->ed_ScalingType,
					   GTCY_Labels, ed->ed_ScalingTypeLabels,
					   GTCY_Active, ed->ed_PrefsWork.ps_ScalingType,
					   TAG_DONE);

	ed->ed_ScalingMath = DoPrefsGadget(ed,&EG[33],ed->ed_ScalingMath,
					   GA_Disabled, ed->ed_PrefsWork.ps_ScalingType == ST_ASPECT_ASIS,
					   GTCY_Labels, ed->ed_ScalingMathLabels,
					   GTCY_Active, ed->ed_PrefsWork.ps_ScalingMath,
					   TAG_DONE);

	ed->ed_Centering = DoPrefsGadget(ed,&EG[34],ed->ed_Centering,
					 GTCY_Labels, ed->ed_CenteringLabels,
					 GTCY_Active, ed->ed_PrefsWork.ps_Centering,
					 TAG_DONE);
    }
}


/*****************************************************************************/


VOID ProcessTextGadget(EdDataPtr ed, EdCommand ec, BOOL screenStuff)
{
BOOL           shift;
LONG           num;
struct Gadget *act;
BOOL           beep;

    shift = (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT) & ed->ed_CurrentMsg.Qualifier;
    beep = FALSE;

    switch (ec)
    {
	case EC_PAPERWIDTH: num = ToMille (ed, ((struct StringInfo *) ed->ed_PaperWidth->SpecialInfo)->Buffer);
                            if (num < 0)
                            {
                                num = 0;
                                act = ed->ed_PaperWidth;
                                beep = TRUE;
                            }
                            else
                            {
                                if (shift)
                                    act = ed->ed_VerticalDPI;
                                else
                                    act = ed->ed_PaperHeight;
                            }
                            ed->ed_PrefsWork.ps_PaperWidth = num;

                            if (ed->ed_PrefsWork.ps_PaperFormat == 3)
                                ed->ed_CurPaperWidth = num;

                            if (screenStuff)
                                SetGadgetAttr (ed, ed->ed_PaperWidth,
                                               GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_PaperWidth, 2),
                                               TAG_DONE);
                            break;

	case EC_PAPERHEIGHT: num = ToMille (ed, ((struct StringInfo *) ed->ed_PaperHeight->SpecialInfo)->Buffer);
                             if (num < 0)
                             {
                                 num = 0;
                                 act = ed->ed_PaperHeight;
                                 beep = TRUE;
                             }
                             else
                             {
                                 if (shift)
                                     act = ed->ed_PaperWidth;
                                 else
                                     act = ed->ed_HorizontalDPI;
                             }
                             ed->ed_PrefsWork.ps_PaperHeight = num;

                             if (ed->ed_PrefsWork.ps_PaperFormat == 3)
                                 ed->ed_CurPaperHeight = num;

                             if (screenStuff)
                                 SetGadgetAttr (ed, ed->ed_PaperHeight,
                                                GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_PaperHeight, 2),
                                                TAG_DONE);
                             break;

	case EC_HORIZONTALDPI: num = ((struct StringInfo *) ed->ed_HorizontalDPI->SpecialInfo)->LongInt;
                               if (num < 0)
                               {
                                   num = 0;
                                   act = ed->ed_HorizontalDPI;
                                   beep = TRUE;
                               }
                               else
                               {
                                   if (shift)
                                       act = (ed->ed_PaperHeight->Flags & GFLG_DISABLED) ? ed->ed_VerticalDPI : ed->ed_PaperHeight;
                                   else
                                       act = ed->ed_VerticalDPI;
                               }
                               ed->ed_PrefsWork.ps_HorizontalDPI = num;
                               if (screenStuff)
                                   SetGadgetAttr (ed, ed->ed_HorizontalDPI,
                                                  GTIN_Number, num,
                                                  TAG_DONE);
                               break;

	case EC_VERTICALDPI: num = ((struct StringInfo *) ed->ed_VerticalDPI->SpecialInfo)->LongInt;
                             if (num < 0)
                             {
                                 num = 0;
                                 act = ed->ed_VerticalDPI;
                                 beep = TRUE;
                             }
                             else
                             {
                                 if (shift)
                                     act = ed->ed_HorizontalDPI;
                                 else
                                     act = (ed->ed_PaperWidth->Flags & GFLG_DISABLED) ? ed->ed_HorizontalDPI : ed->ed_PaperWidth;
                             }
                             ed->ed_PrefsWork.ps_VerticalDPI = num;
                             if (screenStuff)
                                 SetGadgetAttr (ed, ed->ed_VerticalDPI,
                                                GTIN_Number, num,
                                                TAG_DONE);
                             break;

        /****************/

	case EC_LEFTMARGIN: num = ToMille (ed, ((struct StringInfo *) ed->ed_LeftMargin->SpecialInfo)->Buffer);
                            act = ed->ed_LeftMargin;
                            if (num < 0)
                            {
                                num = 0;
                                beep = TRUE;
                            }
                            else if (num > ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_RightMargin)
                            {
                                num = ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_RightMargin;
                                beep = TRUE;
                            }
                            else
                            {
                                if (shift)
                                    act = ed->ed_LineLeading;
                                else
                                    act = ed->ed_RightMargin;
                            }
                            ed->ed_PrefsWork.ps_LeftMargin = num;

                            if (screenStuff)
                                SetGadgetAttr (ed, ed->ed_LeftMargin,
                                               GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_LeftMargin, 2),
                                               TAG_DONE);
                            break;

	case EC_RIGHTMARGIN: num = ToMille (ed, ((struct StringInfo *) ed->ed_RightMargin->SpecialInfo)->Buffer);
                             act = ed->ed_RightMargin;
                             if (num < 0)
                             {
                                 num = 0;
                                 beep = TRUE;
                             }
                             else if (num > ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_LeftMargin)
                             {
                                 num = ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_LeftMargin;
                                 beep = TRUE;
                             }
                             else
                             {
                                 if (shift)
                                     act = ed->ed_LeftMargin;
                                 else
                                     act = ed->ed_TopMargin;
                             }
                             ed->ed_PrefsWork.ps_RightMargin = num;

                             if (screenStuff)
                                 SetGadgetAttr (ed, ed->ed_RightMargin,
                                                GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_RightMargin, 2),
                                                TAG_DONE);
                             break;

	case EC_TOPMARGIN: num = ToMille (ed, ((struct StringInfo *) ed->ed_TopMargin->SpecialInfo)->Buffer);
                           act = ed->ed_TopMargin;
                           if (num < 0)
                           {
                               num = 0;
                               beep = TRUE;
                           }
                           else if (num > ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_BottomMargin)
                           {
                               num = ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_BottomMargin;
                               beep = TRUE;
                           }
                           else
                           {
                               if (shift)
                                   act = ed->ed_RightMargin;
                               else
                                   act = ed->ed_BottomMargin;
                           }
                           ed->ed_PrefsWork.ps_TopMargin = num;

                           if (screenStuff)
                               SetGadgetAttr (ed, ed->ed_TopMargin,
                                              GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_TopMargin, 2),
                                              TAG_DONE);
                           break;

	case EC_BOTTOMMARGIN: num = ToMille (ed, ((struct StringInfo *) ed->ed_BottomMargin->SpecialInfo)->Buffer);
                              act = ed->ed_BottomMargin;
                              if (num < 0)
                              {
                                  num = 0;
                                  beep = TRUE;
                              }
                              else if (num > ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_TopMargin)
                              {
                                  num = ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_TopMargin;
                                  beep = TRUE;
                              }
                              else
                              {
                                  if (shift)
                                      act = ed->ed_TopMargin;
                                  else
                                      act = ed->ed_FontPointSize;
                              }
                              ed->ed_PrefsWork.ps_BottomMargin = num;

                              if (screenStuff)
                                  SetGadgetAttr (ed, ed->ed_BottomMargin,
                                                 GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_BottomMargin, 2),
                                                 TAG_DONE);
                              break;

	case EC_FONTPOINTSIZE: num = ToMilleNC (ed, ((struct StringInfo *) ed->ed_FontPointSize->SpecialInfo)->Buffer);
                               if (num < 0)
                               {
                                   num = 0;
                                   act = ed->ed_FontPointSize;
                                   beep = TRUE;
                               }
                               else
                               {
                                   if (shift)
                                       act = ed->ed_BottomMargin;
                                   else
                                       act = ed->ed_LineLeading;
                               }
                               ed->ed_PrefsWork.ps_FontPointSize = num;

                               if (screenStuff)
                                   SetGadgetAttr (ed, ed->ed_FontPointSize,
                                                  GTST_String, FromMilleNC (ed, ed->ed_PrefsWork.ps_FontPointSize, 1),
                                                  TAG_DONE);
                               break;

	case EC_LINELEADING: num = ToMilleNC (ed, ((struct StringInfo *) ed->ed_LineLeading->SpecialInfo)->Buffer);
                             if (num < 0)
                             {
                                 num = 0;
                                 act = ed->ed_LineLeading;
                                 beep = TRUE;
                             }
                             else
                             {
                                 if (shift)
                                     act = ed->ed_FontPointSize;
                                 else
                                     act = ed->ed_LeftMargin;
                             }
                             ed->ed_PrefsWork.ps_Leading = num;

                             if (screenStuff)
                                 SetGadgetAttr (ed, ed->ed_LineLeading,
                                                GTST_String, FromMilleNC (ed, ed->ed_PrefsWork.ps_Leading, 1),
                                                TAG_DONE);
                             break;

        /****************/

	case EC_LEFTEDGE: num = ToMille (ed, ((struct StringInfo *) ed->ed_LeftEdge->SpecialInfo)->Buffer);
                          act = ed->ed_LeftEdge;
                          if (num < 0)
                          {
                              num = 0;
                              beep = TRUE;
                          }
                          else if (num > ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_Width)
                          {
                              num = ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_Width;
                              beep = TRUE;
                          }
                          else
                          {
                              if (shift)
                                  act = ed->ed_Height;
                              else
                                  act = ed->ed_TopEdge;
                          }
                          ed->ed_PrefsWork.ps_LeftEdge = num;

                          if (screenStuff)
                              SetGadgetAttr (ed, ed->ed_LeftEdge,
                                             GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_LeftEdge, 2),
                                             TAG_DONE);
                          break;

	case EC_TOPEDGE: num = ToMille (ed, ((struct StringInfo *) ed->ed_TopEdge->SpecialInfo)->Buffer);
                         act = ed->ed_TopEdge;
                         if (num < 0)
                         {
                             num = 0;
                             beep = TRUE;
                         }
                         else if (num > ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_Height)
                         {
                             num = ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_Height;
                             beep = TRUE;
                         }
                         else
                         {
                             if (shift)
                                 act = ed->ed_LeftEdge;
                             else
                                 act = ed->ed_Width;
                         }
                         ed->ed_PrefsWork.ps_TopEdge = num;

                         if (screenStuff)
                             SetGadgetAttr (ed, ed->ed_TopEdge,
                                            GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_TopEdge, 2),
                                            TAG_DONE);
                         break;

	case EC_WIDTH: num = ToMille (ed, ((struct StringInfo *) ed->ed_Width->SpecialInfo)->Buffer);
                       act = ed->ed_Width;
                       if (num < 0)
                       {
                           num = 0;
                           beep = TRUE;
                       }
                       else if (num > ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_LeftEdge)
                       {
                           num = ed->ed_PrefsWork.ps_PaperWidth - ed->ed_PrefsWork.ps_LeftEdge;
                           beep = TRUE;
                       }
                       else
                       {
                           if (shift)
                               act = ed->ed_TopEdge;
                           else
                               act = ed->ed_Height;
                       }
                       ed->ed_PrefsWork.ps_Width = num;

                       if (screenStuff)
                           SetGadgetAttr (ed, ed->ed_Width,
                                          GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_Width, 2),
                                          TAG_DONE);
                       break;

	case EC_HEIGHT: num = ToMille (ed, ((struct StringInfo *) ed->ed_Height->SpecialInfo)->Buffer);
                        act = ed->ed_Height;
                        if (num < 0)
                        {
                            num = 0;
                            beep = TRUE;
                        }
                        else if (num > ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_TopEdge)
                        {
                            num = ed->ed_PrefsWork.ps_PaperHeight - ed->ed_PrefsWork.ps_TopEdge;
                            beep = TRUE;
                        }
                        else
                        {
                            if (shift)
                                act = ed->ed_Width;
                            else
                                act = ed->ed_LeftEdge;
                        }
                        ed->ed_PrefsWork.ps_Height = num;

                        if (screenStuff)
                            SetGadgetAttr (ed, ed->ed_Height,
                                           GTST_String, FromMille (ed, ed->ed_PrefsWork.ps_Height, 2),
                                           TAG_DONE);
                        break;

    }

    if (screenStuff)
    {
	if (!(act->Flags & GFLG_DISABLED))
	    ActivateGadget(act,window,NULL);

	if (beep)
	    DisplayBeep (window->WScreen);

	UpdateTextDisplay (ed);
    }
}


/*****************************************************************************/


VOID SyncTextGadgets(EdDataPtr ed)
{
    ProcessTextGadget(ed, EC_PAPERWIDTH,    FALSE);
    ProcessTextGadget(ed, EC_PAPERHEIGHT,   FALSE);
    ProcessTextGadget(ed, EC_VERTICALDPI,   FALSE);
    ProcessTextGadget(ed, EC_HORIZONTALDPI, FALSE);

    if (ed->ed_CurrentPanel == 1)
    {
	ProcessTextGadget(ed, EC_LEFTMARGIN,    FALSE);
	ProcessTextGadget(ed, EC_RIGHTMARGIN,   FALSE);
	ProcessTextGadget(ed, EC_TOPMARGIN,     FALSE);
	ProcessTextGadget(ed, EC_BOTTOMMARGIN,  FALSE);
	ProcessTextGadget(ed, EC_FONTPOINTSIZE, FALSE);
	ProcessTextGadget(ed, EC_LINELEADING,   FALSE);
    }
    else if (ed->ed_CurrentPanel == 2)
    {
	ProcessTextGadget(ed, EC_LEFTEDGE,  FALSE);
	ProcessTextGadget(ed, EC_TOPEDGE,   FALSE);
	ProcessTextGadget(ed, EC_WIDTH,     FALSE);
	ProcessTextGadget(ed, EC_HEIGHT,    FALSE);
    }
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD icode;

    icode = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
	case EC_PANEL      : RemoveGList (window, ed->ed_Gadgets, -1);

                             SetAPen (window->RPort, ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
                             RectFill (window->RPort, window->BorderLeft +
280, window->BorderTop + 28, window->Width - window->BorderRight - 1, window->Height - window->BorderBottom - 20);

                             FreeGadgets (ed->ed_Gadgets);
                             ed->ed_Gadgets = NULL;
                             ed->ed_LastAdded = NULL;

                             ed->ed_CurrentPanel = icode;
                             RenderGadgets (ed);
                             if (ed->ed_LastAdded)
                             {
                                 AddGList (window, ed->ed_Gadgets, -1, -1, NULL);
                                 RefreshGList (ed->ed_Gadgets, window, NULL, -1);
                                 GT_RefreshWindow (window, NULL);
                             }
                             RenderDisplay (ed);
                             break;

	case EC_CM         :
        case EC_INCHES     :
	case EC_POINTS     : icode = ec - EC_CM;
                             if (ed->ed_CurrentSystem != icode)
                             {
                                 ed->ed_CurrentSystem = icode;
                                 RenderGadgets(ed);
                             }
                             break;

        /****************/

	case EC_DRIVERMODE : ed->ed_PrefsWork.ps_DriverMode = icode;
                             break;

	case EC_COPIES     : ed->ed_PrefsWork.ps_Copies = icode;
                             break;

	case EC_PAPERFORMAT: ed->ed_PrefsWork.ps_PaperFormat = icode;
                             SetGadgetAttr(ed,ed->ed_PaperWidth,GA_Disabled, icode != 3,
                                                                TAG_DONE);
                             SetGadgetAttr(ed,ed->ed_PaperHeight,GA_Disabled, icode != 3,
                                                                 TAG_DONE);
                             if (icode < 3)
                             {
                                 ed->ed_PrefsWork.ps_PaperWidth = sizes[icode].Width;
                                 ed->ed_PrefsWork.ps_PaperHeight = sizes[icode].Height;
                             }
                             else
                             {
                                 ed->ed_PrefsWork.ps_PaperWidth = ed->ed_CurPaperWidth;
                                 ed->ed_PrefsWork.ps_PaperHeight = ed->ed_CurPaperHeight;
                             }

                             SetGadgetAttr(ed,ed->ed_PaperWidth,
					   GA_Disabled, icode != 3,
					   GTST_String, FromMille(ed,ed->ed_PrefsWork.ps_PaperWidth,2),
					   TAG_DONE);

                             SetGadgetAttr(ed,ed->ed_PaperHeight,
					   GA_Disabled,   icode != 3,
					   GTST_String,   FromMille(ed,ed->ed_PrefsWork.ps_PaperHeight,2),
					   TAG_DONE);

                             UpdateTextDisplay(ed);
                             break;

	case EC_PAPERWIDTH     :
	case EC_PAPERHEIGHT    :
	case EC_HORIZONTALDPI  :
	case EC_VERTICALDPI    : ProcessTextGadget(ed,ec,TRUE);
                                 break;

        /****************/

	case EC_PRINTFONT  : ed->ed_PrefsWork.ps_Font = icode;
                             break;

	case EC_PITCH      : ed->ed_PrefsWork.ps_Pitch = icode;
                             break;

	case EC_ORIENTATION: ed->ed_PrefsWork.ps_Orientation = icode;
                             break;

	case EC_TAB        : ed->ed_PrefsWork.ps_Tab = icode;
                             break;

        /****************/

	case EC_LEFTMARGIN   :
	case EC_RIGHTMARGIN  :
	case EC_TOPMARGIN    :
	case EC_BOTTOMMARGIN :
	case EC_FONTPOINTSIZE:
	case EC_LINELEADING  : ProcessTextGadget(ed,ec,TRUE);
                               UpdateTextDisplay(ed);
                               break;

        /****************/

	case EC_LEFTEDGE    :
	case EC_TOPEDGE     :
	case EC_WIDTH       :
	case EC_HEIGHT      : ProcessTextGadget(ed,ec,TRUE);
                              break;

	case EC_IMAGE       : ed->ed_PrefsWork.ps_Image = icode;
                              break;

	case EC_SHADING     : ed->ed_PrefsWork.ps_Shading = icode;
                              SetGadgetAttr(ed,ed->ed_Dithering,GA_Disabled, icode != SHAD_GREYSCALE,
                                                                TAG_DONE);
                              SetGadgetAttr(ed,ed->ed_Transparent,GA_Disabled, icode == SHAD_BW,
                                                                TAG_DONE);
	                      break;

	case EC_DITHERING   : ed->ed_PrefsWork.ps_Dithering = icode;
                              break;

	case EC_TRANSPARENT : ed->ed_PrefsWork.ps_Transparency = icode;
                              break;

        /****************/

	case EC_ASPECT     : ed->ed_PrefsWork.ps_Aspect = icode;
                             DoRenderDisplay(ed,FALSE);
                             break;

	case EC_SCALINGTYPE: ed->ed_PrefsWork.ps_ScalingType = icode;
                             SetGadgetAttr(ed,ed->ed_ScalingMath,GA_Disabled, icode == ST_ASPECT_ASIS,
                                                                 TAG_DONE);
                             DoRenderDisplay(ed,FALSE);
                             break;

	case EC_SCALINGMATH: ed->ed_PrefsWork.ps_ScalingMath = icode;
                             break;

	case EC_CENTERING  : ed->ed_PrefsWork.ps_Centering = icode;
                             DoRenderDisplay(ed,FALSE);
                             break;

	default            : break;
    }
}


/*****************************************************************************/


VOID GetSpecialCmdState (EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{

    state->cs_Available = TRUE;
    state->cs_Selected = (((ec == EC_CM) && (ed->ed_CurrentSystem == 0)) ||
			  ((ec == EC_INCHES) && (ed->ed_CurrentSystem == 1)) ||
			  ((ec == EC_POINTS) && (ed->ed_CurrentSystem == 2)));
}
