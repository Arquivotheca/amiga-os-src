
/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/


/****************************************************************************/


#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/displayinfo.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <utility/hooks.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <graphics/gfxmacros.h>
#include <string.h>
#include <stdio.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "asltester_rev.h"
#include "edithook.h"
#include "gadgets.h"
#include "reqtest.h"


/****************************************************************************/


static struct TextAttr topazAttr =
{
    "topaz.font",
     8,
     FS_NORMAL,
     FPF_ROMFONT
};

static const struct NewGadget NG[]=
{
    {12,21,148,14,"Reference Object",  NULL,ID_REFOBJECT,  PLACETEXT_ABOVE,NULL,NULL},

    {12,45,148,14, "InitialLeftEdge",  NULL,ID_INITIALLEFTEDGE,PLACETEXT_RIGHT,NULL,NULL},
    {12,61,148,14, "InitialTopEdge",   NULL,ID_INITIALTOPEDGE,PLACETEXT_RIGHT,NULL,NULL},
    {12,77,148,14, "InitialWidth",     NULL,ID_INITIALWIDTH,PLACETEXT_RIGHT,NULL,NULL},
    {12,93,148,14, "InitialHeight",    NULL,ID_INITIALHEIGHT,PLACETEXT_RIGHT,NULL,NULL},
    {12,113,148,14,"TitleText",        NULL,ID_TITLETEXT,PLACETEXT_RIGHT,NULL,NULL},
    {12,129,148,14,"PositiveText",     NULL,ID_POSITIVETEXT,PLACETEXT_RIGHT,NULL,NULL},
    {12,145,148,14,"NegativeText",     NULL,ID_NEGATIVETEXT,PLACETEXT_RIGHT,NULL,NULL},
    {12,165,148,14,"TextAttr.ta_Name", NULL,ID_FONTNAME,PLACETEXT_RIGHT,NULL,NULL},
    {12,181,148,14,"TextAttr.ta_YSize",NULL,ID_FONTSIZE,PLACETEXT_RIGHT,NULL,NULL},
    {12,201,148,14,"Locale",           NULL,ID_LOCALE,PLACETEXT_RIGHT,NULL,NULL},
    {12,221,26,11, "PrivateIDCMP",     NULL,ID_PRIVATEIDCMP,PLACETEXT_RIGHT,NULL,NULL},
    {12,233,26,11, "SleepWindow",      NULL,ID_SLEEPWINDOW,PLACETEXT_RIGHT,NULL,NULL},
    {12,245,26,11, "IntuiMsgFunc",     NULL,ID_INTUIMSGFUNC,PLACETEXT_RIGHT,NULL,NULL},
    {12,257,26,11, "FilterFunc",       NULL,ID_FILTERFUNC,PLACETEXT_RIGHT,NULL,NULL},
    {408,21,148,14,"Requester Type",   NULL,ID_REQTYPE,PLACETEXT_ABOVE,NULL,NULL},
    {111,341,87,20,"Test",             NULL,ID_TEST,0,NULL,NULL},

    {472,57,148,14, "InitialFile",     NULL,ID_INITIALFILE,0,NULL,NULL},
    {472,73,148,14, "InitialDrawer",   NULL,ID_INITIALDRAWER,0,NULL,NULL},
    {472,89,148,14, "InitialPattern",  NULL,ID_INITIALPATTERN,0,NULL,NULL},
    {472,109,148,14,"AcceptPattern",   NULL,ID_ACCEPTPATTERN,0,NULL,NULL},
    {472,125,148,14,"RejectPattern",   NULL,ID_REJECTPATTERN,0,NULL,NULL},
    {594,145,26,11, "DoSaveMode",      NULL,ID_DOSAVEMODE,0,NULL,NULL},
    {594,157,26,11, "DoMultiSelect",   NULL,ID_DOMULTISELECT,0,NULL,NULL},
    {594,169,26,11, "DoPatterns",      NULL,ID_DOPATTERNS,0,NULL,NULL},
    {594,181,26,11, "DrawersOnly",     NULL,ID_DRAWERSONLY,0,NULL,NULL},
    {594,193,26,11, "RejectIcons",     NULL,ID_REJECTICONS,0,NULL,NULL},
    {594,205,26,11, "FilterDrawers",   NULL,ID_FILTERDRAWERS,0,NULL,NULL},

    {504,57,116,14, "InitialName    ",     NULL,ID_INITIALNAME,0,NULL,NULL},
    {504,73,116,11, "InitialSize    ",     NULL,ID_INITIALSIZE,0,NULL,NULL},
    {504,86,116,11, "InitialFrontPen    ", NULL,ID_INITIALFRONTPEN,0,NULL,NULL},
    {504,99,116,11, "InitialBackPen    ",  NULL,ID_INITIALBACKPEN,0,NULL,NULL},
    {504,112,116,14,"InitialDrawMode    ", NULL,ID_INITIALDRAWMODE,0,NULL,NULL},
    {472,148,116,11,"InitialStyle",        NULL,ID_INITIALSTYLE,0,NULL,NULL},
    {594,131,26,11, "Plain",               NULL,ID_INITIALSTYLE_PLAIN,0,NULL,NULL},
    {594,143,26,11, "Bold",                NULL,ID_INITIALSTYLE_BOLD,0,NULL,NULL},
    {594,155,26,11, "Italic",              NULL,ID_INITIALSTYLE_ITALIC,0,NULL,NULL},
    {594,167,26,11, "Underline",           NULL,ID_INITIALSTYLE_UNDERLINE,0,NULL,NULL},
    {594,183,26,11, "DoFrontPen",          NULL,ID_DOFRONTPEN,0,NULL,NULL},
    {594,195,26,11, "DoBackPen",           NULL,ID_DOBACKPEN,0,NULL,NULL},
    {594,207,26,11, "DoStyle",             NULL,ID_DOSTYLE,0,NULL,NULL},
    {594,219,26,11, "DoDrawMode",          NULL,ID_DODRAWMODE,0,NULL,NULL},
    {594,231,26,11, "FixedWidthOnly",      NULL,ID_FIXEDWIDTHONLY,0,NULL,NULL},
    {504,246,116,11,"MinHeight    ",       NULL,ID_MINHEIGHT,0,NULL,NULL},
    {504,259,116,11,"MaxHeight    ",       NULL,ID_MAXHEIGHT,0,NULL,NULL},
    {504,272,116,11,"MaxFrontPen    ",     NULL,ID_MAXFRONTPEN,0,NULL,NULL},
    {504,285,116,11,"MaxBackPen    ",      NULL,ID_MAXBACKPEN,0,NULL,NULL},
    {594,300,26,11, "FrontPens",           NULL,ID_FRONTPENS,0,NULL,NULL},
    {594,312,26,11, "BackPens",            NULL,ID_BACKPENS,0,NULL,NULL},
    {594,324,26,11, "ModeList",            NULL,ID_MODELIST,0,NULL,NULL},

    {504,57,116,14, "InitialDispID   ",       NULL,ID_INITIALDISPLAYID,0,NULL,NULL},
    {504,72,116,14, "InitialDispWidth   ",    NULL,ID_INITIALDISPLAYWIDTH,0,NULL,NULL},
    {504,87,116,14, "InitialDispHeight   ",   NULL,ID_INITIALDISPLAYHEIGHT,0,NULL,NULL},
    {504,102,116,11,"InitialDispDepth   ",    NULL,ID_INITIALDISPLAYDEPTH,0,NULL,NULL},
    {504,114,116,14,"InitialOScanType   ",    NULL,ID_INITIALOVERSCANTYPE,0,NULL,NULL},
    {504,129,116,14,"InitialInfoLeftEdge   ", NULL,ID_INITIALINFOLEFTEDGE,0,NULL,NULL},
    {504,144,116,14,"InitialInfoTopEdge   ",  NULL,ID_INITIALINFOTOPEDGE,0,NULL,NULL},
    {594,159,26,11, "InitialInfoOpened",      NULL,ID_INITIALINFOOPENED,0,NULL,NULL},
    {594,171,26,11, "InitialAutoScroll",      NULL,ID_INITIALAUTOSCROLL,0,NULL,NULL},
    {594,183,26,11, "DoWidth",                NULL,ID_DOWIDTH,0,NULL,NULL},
    {594,195,26,11, "DoHeight",               NULL,ID_DOHEIGHT,0,NULL,NULL},
    {594,207,26,11, "DoDepth",                NULL,ID_DODEPTH,0,NULL,NULL},
    {594,219,26,11, "DoOverscanType",         NULL,ID_DOOVERSCANTYPE,0,NULL,NULL},
    {594,231,26,11, "DoAutoScroll",           NULL,ID_DOAUTOSCROLL,0,NULL,NULL},
    {594,243,26,11, "CustomSMList",           NULL,ID_CUSTOMSMLIST,0,NULL,NULL},
    {504,255,116,14,"PropertyFlags   ",       NULL,ID_PROPERTYFLAGS,0,NULL,NULL},
    {504,270,116,14,"PropertyMask   ",        NULL,ID_PROPERTYMASK,0,NULL,NULL},
    {504,285,116,14,"MinWidth   ",            NULL,ID_MINWIDTH_SM,0,NULL,NULL},
    {504,300,116,14,"MaxWidth   ",            NULL,ID_MAXWIDTH_SM,0,NULL,NULL},
    {504,315,116,14,"MinHeight   ",           NULL,ID_MINHEIGHT_SM,0,NULL,NULL},
    {504,330,116,14,"MaxHeight   ",           NULL,ID_MAXHEIGHT_SM,0,NULL,NULL},
    {504,345,116,11,"MinDepth   ",            NULL,ID_MINDEPTH,0,NULL,NULL},
    {504,357,116,11,"MaxDepth   ",            NULL,ID_MAXDEPTH,0,NULL,NULL},
};

static const STRPTR ReferenceLabels[] =
{
    "None",
    "Window",
    "Screen",
    "PubScreenName",
    NULL
};

static const STRPTR ReqTypeLabels[] =
{
    "File",
    "Font",
    "ScreenMode",
    NULL
};

static const STRPTR DrawModeLabels[] =
{
    "Jam1",
    "Jam2",
    "Complement",
    NULL
};

static const STRPTR OverscanTypeLabels[] =
{
    "Text",
    "Graphics",
    "Extreme",
    "Maximum",
    NULL
};

struct CommonData cd =
{
    0,     // RefObject
    30,    // InitialLeftEdge
    20,    // InitialTopEdge
    320,   // InitialWidth
    200,   // InitialHeight
    "",    // TitleText
    "",    // PositiveText
    "",    // NegativeText
    "",    // FontName
    8,     // FontSize
    "",    // Locale
    TRUE,  // PrivateIDCMP
    TRUE,  // SleepWindow
    FALSE, // IntuiMsgFunc
    FALSE  // FilterFunc
};

struct FileReqData frd =
{
    "",    // InitialFile
    "",    // InitialDrawer
    "",    // InitialPattern
    "",    // AcceptPattern
    "",    // RejectPattern
    FALSE, // DoSaveMode
    FALSE, // DoMultiSelect
    FALSE, // DoPatterns
    FALSE, // DrawersOnly
    TRUE,  // RejectIcons
    FALSE  // FilterDrawers
};

struct FontReqData fod =
{
    "",    // InitialName
    8,     // InitialSize
    1,     // InitialFrontPen
    0,     // InitialBackPen
    JAM1,  // InitialDrawMode
    0,     // InitialStyle
    TRUE,  // DoFrontPen
    TRUE,  // DoBackPen
    TRUE,  // DoDrawMode
    TRUE,  // DoStyle
    FALSE, // FixedWidthOnly
    4,     // MinHeight
    32,    // MaxHeight
    255,   // MaxFrontPen
    255,   // MaxBackPen
    FALSE, // FrontPens
    FALSE, // BackPens
    FALSE  // ModeList
};

struct ScreenModeReqData smrd =
{
    0,          // InitialDisplayID
    640,        // InitialDisplayWidth
    200,        // InitialDisplayHeight
    2,          // InitialDisplayDepth
    OSCAN_TEXT, // InitialOverscanType
    30,         // InitialInfoLeftEdge
    20,         // InitialInfoTopEdge
    FALSE,      // InitialInfoOpened
    TRUE,       // InitialAutoScroll
    TRUE,       // DoWidth
    TRUE,       // DoHeight
    TRUE,       // DoDepth
    TRUE,       // DoOverscanType
    TRUE,       // DoAutoScroll
    FALSE,      // CustomSMList
    DIPF_IS_WB, // PropertyFlags
    DIPF_IS_WB, // PropertyMask
    320,        // MinWidth
    10000,      // MaxWidth
    200,        // MinHeight
    10000,      // MaxHeight
    1,          // MinDepth
    8,          // MaxDepth
};


/*****************************************************************************/


extern struct Library *SysBase;
extern struct Library *DOSBase;
struct Library        *GfxBase;
struct Library        *IntuitionBase;
struct Library        *GadToolsBase;
struct Library        *UtilityBase;
struct Library        *AslBase;
struct Library        *LocaleBase;

struct Screen         *screen;
struct Window         *window;
struct DrawInfo       *drawInfo;
APTR                   visualInfo;
struct TextFont       *textFont;
struct Gadget         *gadgetList;
struct Gadget         *gadgets[ID_MAX];
struct Gadget         *lastAdded;
struct Hook            hexHook;

ULONG                  requesterType;


/*****************************************************************************/


/* This function render a few borders and lines within the main window.
 * This gets called whenever the window is initially opened, and whenever
 * there is damage.
 */
static VOID RenderWindow(VOID)
{
static UWORD pattern[] = {0xaaaa, 0x5555};

    SetAPen(window->RPort,drawInfo->dri_Pens[SHADOWPEN]);
    RectFill(window->RPort,window->BorderLeft + 316,window->BorderTop,
                           window->BorderLeft + 316,window->Height - window->BorderBottom - 1);

    RectFill(window->RPort,window->BorderLeft,window->BorderTop + 325,
                           window->BorderLeft + 315,window->BorderTop + 325);

    SetAPen(window->RPort,drawInfo->dri_Pens[SHINEPEN]);
    RectFill(window->RPort,window->BorderLeft + 317,window->BorderTop,
                           window->BorderLeft + 317,window->Height - window->BorderBottom - 1);

    RectFill(window->RPort,window->BorderLeft,window->BorderTop + 326,
                           window->BorderLeft + 315,window->BorderTop + 326);

    SetAPen(window->RPort,drawInfo->dri_Pens[TEXTPEN]);
    SetAfPt(window->RPort,pattern,1);
    RectFill(window->RPort,window->BorderLeft + 328,window->BorderTop + 47,
                           window->Width - window->BorderRight - 11,window->BorderTop + 47);
    SetAfPt(window->RPort,NULL,0);

    if (requesterType == REQ_FONT)
    {
        Move(window->RPort,window->BorderLeft + 504,window->BorderTop + 134);
        Draw(window->RPort,window->BorderLeft + 498,window->BorderTop + 134);
        Draw(window->RPort,window->BorderLeft + 468,window->BorderTop + 153);
        Draw(window->RPort,window->BorderLeft + 498,window->BorderTop + 172);
        Draw(window->RPort,window->BorderLeft + 504,window->BorderTop + 172);
    }
}


/*****************************************************************************/


/* Short stub to create a gadget. Copies one of the static definitions,
 * sets its visual info, and text attr pointer, offsets its position by the
 * window's left/top border, then creates the gadget. The resulting pointer
 * is stashes in the gadget pointer array that's maintained
 */
static VOID CreateGad(ULONG kind, ULONG id, ULONG tag1, ...)
{
struct NewGadget ng;

    ng = NG[id];
    ng.ng_TextAttr    = &topazAttr;
    ng.ng_VisualInfo  = visualInfo;
    ng.ng_LeftEdge   += window->BorderLeft;
    ng.ng_TopEdge    += window->BorderTop;

    lastAdded = gadgets[id] = CreateGadgetA(kind,lastAdded,&ng,(struct TagItem *)&tag1);
}


/*****************************************************************************/


/* Special version of CreateGad() that creates a hex-edit gadget, as
 * used to enter a display mode id in the screen mode requester section
 * of the program.
 */
static VOID CreateHexGad(ULONG id, ULONG value)
{
char buffer[12];

    sprintf(buffer,"0x%08lX",value);
    CreateGad(STRING_KIND,id,GTST_MaxChars, 10,
                             GTST_String,   buffer,
                             GTST_EditHook, &hexHook,
                             TAG_DONE);

    if (gadgets[id])
        gadgets[id]->UserData = (APTR)value;
}


/*****************************************************************************/


/* Get a single attribute from one of the gadgets in the gadget array */
static ULONG GetGadAttr(ULONG id, ULONG tag)
{
ULONG result;

    GT_GetGadgetAttrs(gadgets[id],window,NULL,tag,&result,TAG_DONE);

    return(result);
}


/*****************************************************************************/


/* set attributes of one of the gadgets in the gadget array */
static VOID SetGadAttrs(ULONG id, ULONG tag1, ...)
{
    GT_SetGadgetAttrsA(gadgets[id],window,NULL,(struct TagItem *)&tag1);
}


/*****************************************************************************/


/* Range check an integer gadget. If the value falls outside of its allowed
 * range, fence it.
 */
static VOID RangeCheckGad(ULONG id, LONG min, LONG max)
{
LONG num;

    if (gadgets[id])
    {
        num = GetGadAttr(id,GTIN_Number);

        if (num < min)
            SetGadAttrs(id,GTIN_Number,min,TAG_DONE);

        if (num > max)
            SetGadAttrs(id,GTIN_Number,max,TAG_DONE);
    }
}


/*****************************************************************************/


/* Creates all the gadgets needed for the 3 possible displays presented
 * by the program... The values to set the gadgets to are taken from the
 * global requester data variables (cd, frd, fod, and smrd)
 */
static BOOL InstallGadgets(VOID)
{
    gadgetList = NULL;
    lastAdded  = CreateContext(&gadgetList);
    memset(gadgets,0,sizeof(gadgets));

    CreateGad(CYCLE_KIND,ID_REFOBJECT,GTCY_Labels, &ReferenceLabels,
                                      GTCY_Active, cd.RefObject,
                                      TAG_DONE);

    CreateGad(INTEGER_KIND,ID_INITIALLEFTEDGE,GTIN_MaxChars, 4,
                                              GTIN_Number,   cd.InitialLeftEdge,
                                              TAG_DONE);

    CreateGad(INTEGER_KIND,ID_INITIALTOPEDGE,GTIN_MaxChars, 4,
                                             GTIN_Number,   cd.InitialTopEdge,
                                             TAG_DONE);

    CreateGad(INTEGER_KIND,ID_INITIALWIDTH,GTIN_MaxChars, 4,
                                           GTIN_Number,   cd.InitialWidth,
                                           TAG_DONE);

    CreateGad(INTEGER_KIND,ID_INITIALHEIGHT,GTIN_MaxChars, 4,
                                            GTIN_Number,   cd.InitialHeight,
                                            TAG_DONE);

    CreateGad(STRING_KIND,ID_TITLETEXT,GTST_MaxChars, sizeof(cd.TitleText) - 1,
                                       GTST_String,   cd.TitleText,
                                       TAG_DONE);

    CreateGad(STRING_KIND,ID_POSITIVETEXT,GTST_MaxChars, sizeof(cd.PositiveText) - 1,
                                          GTST_String,   cd.PositiveText,
                                          TAG_DONE);

    CreateGad(STRING_KIND,ID_NEGATIVETEXT,GTST_MaxChars, sizeof(cd.NegativeText) - 1,
                                          GTST_String,   cd.NegativeText,
                                          TAG_DONE);

    CreateGad(STRING_KIND,ID_FONTNAME,GTST_MaxChars, sizeof(cd.FontName) - 1,
                                      GTST_String,   cd.FontName,
                                      TAG_DONE);

    CreateGad(‰INTEGER_KIND,ID_FONTSIZE,GTIN_MaxChars, 4,
                                        GTIN_Number,   cd.FontSize,
                                        TAG_DONE);

    CreateGad(STRING_KIND,ID_LOCALE,GTST_MaxChars, sizeof(cd.Locale) - 1,
                                    GTST_String,   cd.Locale,
                                    TAG_DONE);

    CreateGad(CHECKBOX_KIND,ID_PRIVATEIDCMP,GTCB_Checked, cd.PrivateIDCMP,
                                            TAG_DONE);

    CreateGad(CHECKBOX_KIND,ID_SLEEPWINDOW,GTCB_Checked, cd.SleepWindow,
                                           TAG_DONE);

    CreateGad(CHECKBOX_KIND,ID_INTUIMSGFUNC,GTCB_Checked, cd.IntuiMsgFunc,
                                            TAG_DONE);

    CreateGad(CHECKBOX_KIND,ID_FILTERFUNC,GTCB_Checked, cd.FilterFunc,
                                          TAG_DONE);

    CreateGad(CYCLE_KIND,ID_REQTYPE,GTCY_Labels, &ReqTypeLabels,
                                    GTCY_Active, requesterType,
                                    TAG_DONE);

    CreateGad(BUTTON_KIND,ID_TEST,TAG_DONE);

    if (requesterType == REQ_FILE)
    {
        CreateGad(STRING_KIND,ID_INITIALFILE,GTST_MaxChars, sizeof(frd.InitialFile) - 1,
                                             GTST_String,   frd.InitialFile,
                                             TAG_DONE);

        CreateGad(STRING_KIND,ID_INITIALDRAWER,GTST_MaxChars, sizeof(frd.InitialDrawer) - 1,
                                               GTST_String,   frd.InitialDrawer,
                                               TAG_DONE);

        CreateGad(STRING_KIND,ID_INITIALPATTERN,GTST_MaxChars, sizeof(frd.InitialPattern) - 1,
                                                GTST_String,   frd.InitialPattern,
                                                TAG_DONE);

        CreateGad(STRING_KIND,ID_ACCEPTPATTERN,GTST_MaxChars, sizeof(frd.AcceptPattern) - 1,
                                               GTST_String,   frd.AcceptPattern,
                                               TAG_DONE);

        CreateGad(STRING_KIND,ID_REJECTPATTERN,GTST_MaxChars, sizeof(frd.RejectPattern) - 1,
                                               GTST_String,   frd.RejectPattern,
                                               TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOSAVEMODE,GTCB_Checked, frd.DoSaveMode,
                                              TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOMULTISELECT,GTCB_Checked, frd.DoMultiSelect,
                                                 TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOPATTERNS,GTCB_Checked, frd.DoPatterns,
                                              TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DRAWERSONLY,GTCB_Checked, frd.DrawersOnly,
                                               TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_REJECTICONS,GTCB_Checked, frd.RejectIcons,
                                               TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_FILTERDRAWERS,GTCB_Checked, frd.FilterDrawers,
                                                 TAG_DONE);
    }
    else if (requesterType == REQ_FONT)
    {
        CreateGad(STRING_KIND,ID_INITIALNAME,GTST_MaxChars, sizeof(fod.InitialName) - 1,
                                             GTST_String,   fod.InitialName,
                                             TAG_DONE);

        CreateGad(SLIDER_KIND,ID_INITIALSIZE,GTSL_MaxLevelLen, 3,
                                             GTSL_LevelFormat, "%3ld",
                                             GTSL_Level,       fod.InitialSize,
                                             GTSL_Min,         4,
                                             GTSL_Max,         127,
                                             TAG_DONE);

        CreateGad(SLIDER_KIND,ID_INITIALFRONTPEN,GTSL_MaxLevelLen, 3,
                                                 GTSL_LevelFormat, "%3ld",
                                                 GTSL_Level,       fod.InitialFrontPen,
                                                 GTSL_Min,         0,
                                                 GTSL_Max,         255,
                                                 TAG_DONE);

        CreateGad(SLIDER_KIND,ID_INITIALBACKPEN,GTSL_MaxLevelLen, 3,
                                                GTSL_LevelFormat, "%3ld",
                                                GTSL_Level,       fod.InitialBackPen,
                                                GTSL_Min,         0,
                                                GTSL_Max,         255,
                                                TAG_DONE);

        CreateGad(CYCLE_KIND,ID_INITIALDRAWMODE,GTCY_Labels, &DrawModeLabels,
                                                GTCY_Active, fod.InitialDrawMode,
                                                TAG_DONE);

        CreateGad(TEXT_KIND,ID_INITIALSTYLE,TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_INITIALSTYLE_PLAIN,GTCB_Checked, fod.InitialStyle == 0,
                                                      TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_INITIALSTYLE_BOLD,GTCB_Checked, fod.InitialStyle & FSF_BOLD,
                                                     TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_INITIALSTYLE_ITALIC,GTCB_Checked, fod.InitialStyle & FSF_ITALIC,
                                                       TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_INITIALSTYLE_UNDERLINE,GTCB_Checked, fod.InitialStyle & FSF_UNDERLINED,
                                                          TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOFRONTPEN,GTCB_Checked, fod.DoFrontPen,
                                              TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOBACKPEN,GTCB_Checked, fod.DoBackPen,
                                             TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOSTYLE,GTCB_Checked, fod.DoStyle,
                                           TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DODRAWMODE,GTCB_Checked, fod.DoDrawMode,
                                              TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_FIXEDWIDTHONLY,GTCB_Checked, fod.FixedWidthOnly,
                                                  TAG_DONE);

        CreateGad(SLIDER_KIND,ID_MINHEIGHT,GTSL_MaxLevelLen, 3,
                                           GTSL_LevelFormat, "%3ld",
                                           GTSL_Level,       fod.MinHeight,
                                           GTSL_Min,         4,
                                           GTSL_Max,         127,
                                           TAG_DONE);

        CreateGad(SLIDER_KIND,ID_MAXHEIGHT,GTSL_MaxLevelLen, 3,
                                           GTSL_LevelFormat, "%3ld",
                                           GTSL_Level,       fod.MaxHeight,
                                           GTSL_Min,         4,
                                           GTSL_Max,         127,
                                           TAG_DONE);

        CreateGad(SLIDER_KIND,ID_MAXFRONTPEN,GTSL_MaxLevelLen, 3,
                                             GTSL_LevelFormat, "%3ld",
                                             GTSL_Level,       fod.MaxFrontPen,
                                             GTSL_Min,         1,
                                             GTSL_Max,         255,
                                             TAG_DONE);

        CreateGad(SLIDER_KIND,ID_MAXBACKPEN,GTSL_MaxLevelLen, 3,
                                            GTSL_LevelFormat, "%3ld",
                                            GTSL_Level,       fod.MaxBackPen,
                                            GTSL_Min,         1,
                                            GTSL_Max,         255,
                                            TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_FRONTPENS,GTCB_Checked, fod.FrontPens,
                                             GA_Disabled,  TRUE,  // Not implemented in demo
                                             TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_BACKPENS,GTCB_Checked, fod.BackPens,
                                            GA_Disabled,  TRUE,   // Not implemented in demo
                                            TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_MODELIST,GTCB_Checked, fod.ModeList,
                                            TAG_DONE);

        RenderWindow(); /* to get the little lines drawn */
    }
    else /* if (requesterType == REQ_SCREENMODE) */
    {
        CreateHexGad(ID_INITIALDISPLAYID,smrd.InitialDisplayID);

        CreateGad(INTEGER_KIND,ID_INITIALDISPLAYWIDTH,GTIN_MaxChars, 5,
                                                      GTIN_Number,   smrd.InitialDisplayWidth,
                                                      TAG_DONE);

        CreateGad(INTEGER_KIND,ID_INITIALDISPLAYHEIGHT,GTIN_MaxChars, 5,
                                                       GTIN_Number,   smrd.InitialDisplayHeight,
                                                       TAG_DONE);

        CreateGad(SLIDER_KIND,ID_INITIALDISPLAYDEPTH,GTSL_MaxLevelLen, 2,
                                                     GTSL_LevelFormat, "%2ld",
                                                     GTSL_Level,       smrd.InitialDisplayDepth,
                                                     GTSL_Min,         1,
                                                     GTSL_Max,         32,
                                                     TAG_DONE);

        CreateGad(CYCLE_KIND,ID_INITIALOVERSCANTYPE,GTCY_Labels, &OverscanTypeLabels,
                                                    GTCY_Active, smrd.InitialOverscanType - 1,
                                                    TAG_DONE);

        CreateGad(INTEGER_KIND,ID_INITIALINFOLEFTEDGE,GTIN_MaxChars, 5,
                                                      GTIN_Number,   smrd.InitialInfoLeftEdge,
                                                      TAG_DONE);

        CreateGad(INTEGER_KIND,ID_INITIALINFOTOPEDGE,GTIN_MaxChars, 5,
                                                     GTIN_Number,   smrd.InitialInfoTopEdge,
                                                     TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_INITIALINFOOPENED,GTCB_Checked, smrd.InitialInfoOpened,
                                                     TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_INITIALAUTOSCROLL,GTCB_Checked, smrd.InitialAutoScroll,
                                                     TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOWIDTH,GTCB_Checked, smrd.DoWidth,
                                           TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOHEIGHT,GTCB_Checked, smrd.DoHeight,
                                            TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DODEPTH,GTCB_Checked, smrd.DoDepth,
                                           TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOOVERSCANTYPE,GTCB_Checked, smrd.DoOverscanType,
                                                  TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_DOAUTOSCROLL,GTCB_Checked, smrd.DoAutoScroll,
                                                TAG_DONE);

        CreateGad(CHECKBOX_KIND,ID_CUSTOMSMLIST,GTCB_Checked, smrd.CustomSMList,
                                                TAG_DONE);

        CreateHexGad(ID_PROPERTYFLAGS,smrd.PropertyFlags);
        CreateHexGad(ID_PROPERTYMASK,smrd.PropertyMask);

        CreateGad(INTEGER_KIND,ID_MINWIDTH_SM,GTIN_MaxChars, 5,
                                              GTIN_Number,   smrd.MinWidth,
                                              TAG_DONE);

        CreateGad(INTEGER_KIND,ID_MAXWIDTH_SM,GTIN_MaxChars, 5,
                                              GTIN_Number,   smrd.MaxWidth,
                                              TAG_DONE);

        CreateGad(INTEGER_KIND,ID_MINHEIGHT_SM,GTIN_MaxChars, 5,
                                               GTIN_Number,   smrd.MinHeight,
                                               TAG_DONE);

        CreateGad(INTEGER_KIND,ID_MAXHEIGHT_SM,GTIN_MaxChars, 5,
                                               GTIN_Number,   smrd.MaxHeight,
                                               TAG_DONE);

        CreateGad(SLIDER_KIND,ID_MINDEPTH,GTSL_MaxLevelLen, 2,
                                          GTSL_LevelFormat, "%2ld",
                                          GTSL_Level,       smrd.MinDepth,
                                          GTSL_Min,         1,
                                          GTSL_Max,         32,
                                          TAG_DONE);

        CreateGad(SLIDER_KIND,ID_MAXDEPTH,GTSL_MaxLevelLen, 2,
                                          GTSL_LevelFormat, "%2ld",
                                          GTSL_Level,       smrd.MaxDepth,
                                          GTSL_Min,         1,
                                          GTSL_Max,         32,
                                          TAG_DONE);

    }

    if (!lastAdded)
    {
        FreeGadgets(gadgetList);
        return(FALSE);
    }

    AddGList(window,gadgetList,-1,-1,NULL);
    RefreshGList(gadgetList,window,NULL,-1);
    GT_RefreshWindow(window,NULL);

    return(TRUE);
}


/*****************************************************************************/


/* Standard range check of all gadgets that need it */
static VOID RangeCheckGadgets(VOID)
{
    RangeCheckGad(ID_INITIALLEFTEDGE,0,10000);
    RangeCheckGad(ID_INITIALTOPEDGE,0,10000);
    RangeCheckGad(ID_INITIALWIDTH,10,10000);
    RangeCheckGad(ID_INITIALHEIGHT,10,10000);
    RangeCheckGad(ID_INITIALDISPLAYWIDTH,16,32000);
    RangeCheckGad(ID_INITIALDISPLAYHEIGHT,16,32000);
    RangeCheckGad(ID_FONTSIZE,4,127);
}


/*****************************************************************************/


/* This function reads the current values of the gadgets in the window,
 * and updates the contents of the global requester data variables (cd,
 * frd, fod, and smrd) to match what the gadgets indicate.
 */
static VOID ReadGadgets(VOID)
{
    RangeCheckGadgets();

    cd.RefObject       = GetGadAttr(ID_REFOBJECT,GTCY_Active);
    cd.InitialLeftEdge = GetGadAttr(ID_INITIALLEFTEDGE,GTIN_Number);
    cd.InitialTopEdge  = GetGadAttr(ID_INITIALTOPEDGE,GTIN_Number);
    cd.InitialWidth    = GetGadAttr(ID_INITIALWIDTH,GTIN_Number);
    cd.InitialHeight   = GetGadAttr(ID_INITIALHEIGHT,GTIN_Number);
    strcpy(cd.TitleText,(STRPTR)GetGadAttr(ID_TITLETEXT,GTST_String));
    strcpy(cd.PositiveText,(STRPTR)GetGadAttr(ID_POSITIVETEXT,GTST_String));
    strcpy(cd.NegativeText,(STRPTR)GetGadAttr(ID_NEGATIVETEXT,GTST_String));
    strcpy(cd.FontName,(STRPTR)GetGadAttr(ID_FONTNAME,GTST_String));
    cd.FontSize       = GetGadAttr(ID_FONTSIZE,GTIN_Number);
    strcpy(cd.Locale,(STRPTR)GetGadAttr(ID_LOCALE,GTST_String));
    cd.PrivateIDCMP   = GetGadAttr(ID_PRIVATEIDCMP,GTCB_Checked);
    cd.SleepWindow    = GetGadAttr(ID_SLEEPWINDOW,GTCB_Checked);
    cd.IntuiMsgFunc   = GetGadAttr(ID_INTUIMSGFUNC,GTCB_Checked);
    cd.FilterFunc     = GetGadAttr(ID_FILTERFUNC,GTCB_Checked);

    if (requesterType == REQ_FILE)
    {
        strcpy(frd.InitialFile,(STRPTR)GetGadAttr(ID_INITIALFILE,GTST_String));
        strcpy(frd.InitialDrawer,(STRPTR)GetGadAttr(ID_INITIALDRAWER,GTST_String));
        strcpy(frd.InitialPattern,(STRPTR)GetGadAttr(ID_INITIALPATTERN,GTST_String));
        strcpy(frd.AcceptPattern,(STRPTR)GetGadAttr(ID_ACCEPTPATTERN,GTST_String));
        strcpy(frd.RejectPattern,(STRPTR)GetGadAttr(ID_REJECTPATTERN,GTST_String));
        frd.DoSaveMode    = GetGadAttr(ID_DOSAVEMODE,GTCB_Checked);
        frd.DoMultiSelect = GetGadAttr(ID_DOMULTISELECT,GTCB_Checked);
        frd.DoPatterns    = GetGadAttr(ID_DOPATTERNS,GTCB_Checked);
        frd.DrawersOnly   = GetGadAttr(ID_DRAWERSONLY,GTCB_Checked);
        frd.RejectIcons   = GetGadAttr(ID_REJECTICONS,GTCB_Checked);
        frd.FilterDrawers = GetGadAttr(ID_FILTERDRAWERS,GTCB_Checked);
    }
    else if (requesterType == REQ_FONT)
    {
        strcpy(fod.InitialName,(STRPTR)GetGadAttr(ID_INITIALNAME,GTST_String));
        fod.InitialSize     = GetGadAttr(ID_INITIALSIZE,GTSL_Level);
        fod.InitialFrontPen = GetGadAttr(ID_INITIALFRONTPEN,GTSL_Level);
        fod.InitialBackPen  = GetGadAttr(ID_INITIALBACKPEN,GTSL_Level);
        fod.InitialDrawMode = GetGadAttr(ID_INITIALDRAWMODE,GTCY_Active);
        fod.DoFrontPen      = GetGadAttr(ID_DOFRONTPEN,GTCB_Checked);
        fod.DoBackPen       = GetGadAttr(ID_DOBACKPEN,GTCB_Checked);
        fod.DoStyle         = GetGadAttr(ID_DOSTYLE,GTCB_Checked);
        fod.DoDrawMode      = GetGadAttr(ID_DODRAWMODE,GTCB_Checked);
        fod.FixedWidthOnly  = GetGadAttr(ID_FIXEDWIDTHONLY,GTCB_Checked);
        fod.MinHeight       = GetGadAttr(ID_MINHEIGHT,GTSL_Level);
        fod.MaxHeight       = GetGadAttr(ID_MAXHEIGHT,GTSL_Level);
        fod.MaxFrontPen     = GetGadAttr(ID_MAXFRONTPEN,GTSL_Level);
        fod.MaxBackPen      = GetGadAttr(ID_MAXBACKPEN,GTSL_Level);
        fod.FrontPens       = GetGadAttr(ID_FRONTPENS,GTCB_Checked);
        fod.BackPens        = GetGadAttr(ID_BACKPENS,GTCB_Checked);
        fod.ModeList        = GetGadAttr(ID_MODELIST,GTCB_Checked);

        fod.InitialStyle    = (GetGadAttr(ID_INITIALSTYLE_BOLD,GTCB_Checked) << FSB_BOLD)
                              | (GetGadAttr(ID_INITIALSTYLE_ITALIC,GTCB_Checked) << FSB_ITALIC)
                              | (GetGadAttr(ID_INITIALSTYLE_UNDERLINE,GTCB_Checked) << FSB_UNDERLINED);
    }
    else /* if (requesterType == REQ_SCREENMODE) */
    {
        smrd.InitialDisplayID     = (ULONG)gadgets[ID_INITIALDISPLAYID]->UserData;
        smrd.InitialDisplayWidth  = GetGadAttr(ID_INITIALDISPLAYWIDTH,GTIN_Number);
        smrd.InitialDisplayHeight = GetGadAttr(ID_INITIALDISPLAYHEIGHT,GTIN_Number);
        smrd.InitialDisplayDepth  = GetGadAttr(ID_INITIALDISPLAYDEPTH,GTSL_Level);
        smrd.InitialOverscanType  = GetGadAttr(ID_INITIALOVERSCANTYPE,GTCY_Active) + 1;
        smrd.InitialInfoLeftEdge  = GetGadAttr(ID_INITIALINFOLEFTEDGE,GTIN_Number);
        smrd.InitialInfoTopEdge   = GetGadAttr(ID_INITIALINFOTOPEDGE,GTIN_Number);
        smrd.InitialInfoOpened    = GetGadAttr(ID_INITIALINFOOPENED,GTCB_Checked);
        smrd.InitialAutoScroll    = GetGadAttr(ID_INITIALAUTOSCROLL,GTCB_Checked);
        smrd.DoWidth              = GetGadAttr(ID_DOWIDTH,GTCB_Checked);
        smrd.DoHeight             = GetGadAttr(ID_DOHEIGHT,GTCB_Checked);
        smrd.DoDepth              = GetGadAttr(ID_DODEPTH,GTCB_Checked);
        smrd.DoOverscanType       = GetGadAttr(ID_DOOVERSCANTYPE,GTCB_Checked);
        smrd.DoAutoScroll         = GetGadAttr(ID_DOAUTOSCROLL,GTCB_Checked);
        smrd.CustomSMList         = GetGadAttr(ID_CUSTOMSMLIST,GTCB_Checked);
        smrd.PropertyFlags        = (ULONG)gadgets[ID_PROPERTYFLAGS]->UserData;
        smrd.PropertyMask         = (ULONG)gadgets[ID_PROPERTYMASK]->UserData;
        smrd.MinWidth             = GetGadAttr(ID_MINWIDTH_SM,GTIN_Number);
        smrd.MaxWidth             = GetGadAttr(ID_MAXWIDTH_SM,GTIN_Number);
        smrd.MinHeight            = GetGadAttr(ID_MINHEIGHT_SM,GTIN_Number);
        smrd.MaxHeight            = GetGadAttr(ID_MAXHEIGHT_SM,GTIN_Number);
        smrd.MinDepth             = GetGadAttr(ID_MINDEPTH,GTSL_Level);
        smrd.MaxDepth             = GetGadAttr(ID_MAXDEPTH,GTSL_Level);
    }
}


/*****************************************************************************/


/* Main event loop.
 * Collect all events, and act on them...
 */
static VOID EventLoop(VOID)
{
ULONG                sigs;
struct IntuiMessage *intuiMsg;
BOOL                 quit;
UWORD                icode;

    quit = FALSE;
    while (!quit)
    {
        sigs = Wait((1 << window->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

        /* if ctrl-c, quit */
        if (sigs & SIGBREAKF_CTRL_C)
            break;

        /* if ctrl-f, activate and bring to front */
        if (sigs & SIGBREAKF_CTRL_F)
        {
            WindowToFront(window);
            ActivateWindow(window);
        }

        /* drain IDCMP */
        while (intuiMsg = GT_GetIMsg(window->UserPort))
        {
            icode = intuiMsg->Code;

            switch (intuiMsg->Class)
            {
                case IDCMP_VANILLAKEY   : if (icode != 0x1b) /* ESC key */
                                              break;

                case IDCMP_CLOSEWINDOW  : quit = TRUE;
                                          break;

                case IDCMP_REFRESHWINDOW: GT_BeginRefresh(window);
                                          RenderWindow();
                                          GT_EndRefresh(window,TRUE);
                                          break;

                case IDCMP_GADGETDOWN   :
                case IDCMP_MOUSEMOVE    :
                case IDCMP_GADGETUP     : switch (((struct Gadget *)intuiMsg->IAddress)->GadgetID)
                                          {
                                              case ID_REQTYPE: ReadGadgets();
                                                               RemoveGList(window,gadgetList,-1);
                                                               EraseRect(window->RPort,window->BorderLeft + 319,window->BorderTop + 48,
                                                                                       window->Width - window->BorderRight - 1,window->Height - window->BorderBottom - 1);
                                                               FreeGadgets(gadgetList);

                                                               requesterType = intuiMsg->Code;
                                                               if (!InstallGadgets())
                                                                   quit = TRUE;

                                                               break;

                                              case ID_INITIALSTYLE_PLAIN:
                                                               if (!icode)
                                                                   SetGadAttrs(ID_INITIALSTYLE_PLAIN,GTCB_Checked,TRUE,TAG_DONE);

                                                               SetGadAttrs(ID_INITIALSTYLE_BOLD,GTCB_Checked,FALSE,TAG_DONE);
                                                               SetGadAttrs(ID_INITIALSTYLE_ITALIC,GTCB_Checked,FALSE,TAG_DONE);
                                                               SetGadAttrs(ID_INITIALSTYLE_UNDERLINE,GTCB_Checked,FALSE,TAG_DONE);
                                                               break;

                                              case ID_INITIALSTYLE_BOLD:
                                                               if (icode)
                                                                   SetGadAttrs(ID_INITIALSTYLE_PLAIN,GTCB_Checked,FALSE,TAG_DONE);
                                                               else if (!GetGadAttr(ID_INITIALSTYLE_ITALIC,GTCB_Checked) && !GetGadAttr(ID_INITIALSTYLE_UNDERLINE,GTCB_Checked))
                                                                   SetGadAttrs(ID_INITIALSTYLE_PLAIN,GTCB_Checked,TRUE,TAG_DONE);
                                                               break;

                                              case ID_INITIALSTYLE_ITALIC:
                                                               if (icode)
                                                                   SetGadAttrs(ID_INITIALSTYLE_PLAIN,GTCB_Checked,FALSE,TAG_DONE);
                                                               else if (!GetGadAttr(ID_INITIALSTYLE_BOLD,GTCB_Checked) && !GetGadAttr(ID_INITIALSTYLE_UNDERLINE,GTCB_Checked))
                                                                   SetGadAttrs(ID_INITIALSTYLE_PLAIN,GTCB_Checked,TRUE,TAG_DONE);
                                                               break;

                                              case ID_INITIALSTYLE_UNDERLINE:
                                                               if (icode)
                                                                   SetGadAttrs(ID_INITIALSTYLE_PLAIN,GTCB_Checked,FALSE,TAG_DONE);
                                                               else if (!GetGadAttr(ID_INITIALSTYLE_BOLD,GTCB_Checked) && !GetGadAttr(ID_INITIALSTYLE_ITALIC,GTCB_Checked))
                                                                   SetGadAttrs(ID_INITIALSTYLE_PLAIN,GTCB_Checked,TRUE,TAG_DONE);
                                                               break;

                                              case ID_MINHEIGHT: if (GetGadAttr(ID_MAXHEIGHT,GTSL_Level) < icode)
                                                                     SetGadAttrs(ID_MAXHEIGHT,GTSL_Level,icode,TAG_DONE);
                                                                 break;

                                              case ID_MAXHEIGHT: if (GetGadAttr(ID_MINHEIGHT,GTSL_Level) > icode)
                                                                     SetGadAttrs(ID_MINHEIGHT,GTSL_Level,icode,TAG_DONE);
                                                                 break;

                                              case ID_MINDEPTH: if (GetGadAttr(ID_MAXDEPTH,GTSL_Level) < icode)
                                                                    SetGadAttrs(ID_MAXDEPTH,GTSL_Level,icode,TAG_DONE);
                                                                break;

                                              case ID_MAXDEPTH: if (GetGadAttr(ID_MINDEPTH,GTSL_Level) > icode)
                                                                    SetGadAttrs(ID_MINDEPTH,GTSL_Level,icode,TAG_DONE);
                                                                break;

                                              case ID_INITIALLEFTEDGE     :
                                              case ID_INITIALTOPEDGE      :
                                              case ID_INITIALWIDTH        :
                                              case ID_INITIALHEIGHT       :
                                              case ID_INITIALDISPLAYWIDTH :
                                              case ID_INITIALDISPLAYHEIGHT:
                                              case ID_FONTSIZE            : RangeCheckGadgets();
                                                                            break;


                                              case ID_TEST   : ReadGadgets();
                                                               TestRequester(&cd,&frd,&fod,&smrd,requesterType);
                                                               break;
                                          }
            }

            GT_ReplyIMsg(intuiMsg);

            if (quit)
                break;
        }
    }
}


/*****************************************************************************/


/* Main entry point. Init the universe, and enter the event loop */
LONG main(VOID)
{
    requesterType   = REQ_FILE;
    hexHook.h_Entry = (HOOKFUNC)HexHook;

    /* open all basic libraries in case we need 'em later */
    if (IntuitionBase = OpenLibrary("intuition.library" VERSTAG, 39))
    {
        GfxBase       = OpenLibrary("graphics.library", 39);
        GadToolsBase  = OpenLibrary("gadtools.library", 39);
        UtilityBase   = OpenLibrary("utility.library", 39);
        LocaleBase    = OpenLibrary("locale.library", 38);
    }

    /* check if we got everything we need */
    if (IntuitionBase && GfxBase && GadToolsBase && UtilityBase && LocaleBase)
    {
        if (screen = LockPubScreen(NULL))
        {
            if (drawInfo = GetScreenDrawInfo(screen))
            {
                if (visualInfo = GetVisualInfoA(screen,NULL))
                {
                    if (textFont = OpenFont(&topazAttr))
                    {
                        if (window = OpenWindowTags(NULL,
                                          WA_InnerWidth,   630,
                                          WA_InnerHeight,  375,
                                          WA_IDCMP,        IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MOUSEMOVE | IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_VANILLAKEY | BUTTONIDCMP | CYCLEIDCMP | CHECKBOXIDCMP | STRINGIDCMP | INTEGERIDCMP | SLIDERIDCMP,
                                          WA_DepthGadget,  TRUE,
                                          WA_DragBar,      TRUE,
                                          WA_CloseGadget,  TRUE,
                                          WA_SimpleRefresh,TRUE,
                                          WA_Activate,     TRUE,
                                          WA_Title,        "ASL Tester",
                                          WA_PubScreen,    screen,
                                          WA_AutoAdjust,   TRUE,
                                          TAG_DONE))
                        {
                            SetFont(window->RPort,textFont);
                            RenderWindow();

                            if (InstallGadgets())
                                EventLoop();

                            CloseWindow(window);

                            FreeGadgets(gadgetList);
                        }
                        CloseFont(textFont);
                    }
                    FreeVisualInfo(visualInfo);
                }
                FreeScreenDrawInfo(screen,drawInfo);
            }
            UnlockPubScreen(NULL,screen);
        }
    }

    if (IntuitionBase)
    {
        CloseLibrary(LocaleBase);
        CloseLibrary(UtilityBase);
        CloseLibrary(GadToolsBase);
        CloseLibrary(GfxBase);
        CloseLibrary(IntuitionBase);
    }

    return(0);
}
