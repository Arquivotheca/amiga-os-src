
/* NOTE: This is the ugliest most disgusting UI code I've ever done. I
 *       apologize to anyone trying to make changes in here. Watch out, there
 *       are bazzilions of wierd interactions and side effects!
 */

/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <graphics/text.h>
#include <graphics/displayinfo.h>
#include <graphics/view.h>
#include <graphics/videocontrol.h>
#include <graphics/layers.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <dos/dos.h>
#include <gadgets/colorwheel.h>
#include <gadgets/gradientslider.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/colorwheel_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/colorwheel_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "sample.h"
#include "extent.h"
#include "pe_iff.h"


#define SysBase ed->ed_SysBase
#define MONITOR_PART(id) ((id) & MONITOR_ID_MASK)


/*****************************************************************************/


struct LoadRGBTable
{
    UWORD NumColors;
    UWORD FirstColor;
    ULONG Red;
    ULONG Green;
    ULONG Blue;
};


/*****************************************************************************/


#define Make32(sixteen) ((((ULONG)sixteen)<<16) | (ULONG)sixteen)
ULONG Make16(ULONG value, ULONG bits);
static VOID __asm PenCallBack(register __a0 struct Hook *hook, register __a1 struct LVDrawMsg *msg, register __a2 struct Node *node);
static VOID DoBrightness(EdDataPtr ed);
VOID DoWheel(EdDataPtr ed);
VOID ClosePrefsWindow(EdDataPtr ed, struct Window *wp, BOOL others);
VOID SetColors(EdDataPtr ed);


/*****************************************************************************/


/* ILBM Color register structure */
struct ColorRegister
{
    UBYTE red, green, blue;
};

#define	ID_ILBM	MAKE_ID('I','L','B','M')
#define	ID_CMAP	MAKE_ID('C','M','A','P')


/*****************************************************************************/


UBYTE far NodeMap[] =
{
    BACKGROUNDPEN,
    TEXTPEN,
    HIGHLIGHTTEXTPEN,
    SHINEPEN,
    SHADOWPEN,
    FILLPEN,
    FILLTEXTPEN,
    BARBLOCKPEN,
    BARDETAILPEN
};

UBYTE far RevNodeMap[] =
{
    0, /* should not happen */
    0, /* should not happen */
    1,
    3,
    4,
    5,
    6,
    0,
    2,
    8,
    7,
    0  /* should not happen */
};

UBYTE far DefaultPens[] =
{
    0,	/* DETAILPEN		 */
    1,	/* BLOCKPEN		 */
    1,	/* TEXTPEN		 */
    2,	/* SHINEPEN		 */
    1,	/* SHADOWPEN		 */
    3,	/* FILLPEN		 */
    1,	/* FILLTEXTPEN		 */
    0,	/* BACKGROUNDPEN	 */
    2,	/* HIGHLIGHTTEXTPEN 	 */
    1,	/* BARDETAILPEN	 	 */
    2,	/* BARBLOCKPEN	 	 */
    1	/* BARTRIMPEN	 	 */
};

UWORD far DefaultColors[] =
{
    0x0AAA,	/*  Color 0 (gray) */
    0x0000,	/*  Color 1 (black) */
    0x0FFF,	/*  Color 2 (white) */
    0x068B,	/*  Color 3 (baby-blue) */
    0x0E44,
    0x05D5,
    0x004D,
    0x0E90
};


/*****************************************************************************/


struct Preset
{
    ULONG Colors[8];  /* 8 bits for red/green/blue each per longword */
};


struct Preset far Presets[] =
{
    /* Tint */
    {
        0x00BBAA99,  /* beige */
        0x00000022,
        0x00FFFFFF,
        0x006688BB,  /* blue */
        0x00EE4444,
        0x0055DD55,
        0x000044DD,
        0x00EE9900
    },

    /* Pharaoh */
    {
        0x0088AACC,     /* blue-gray */
        0x00000022,
        0x00FFFFFF,
        0x00FFCC99,     /* gold */
        0x000000F0,
        0x00F92121,
        0x0052F276,
        0x00DFA526
    },

    /* Sunset */
    {
        0x0088AACC, /* blue-gray */
        0x00000022,
        0x00FFFFFF,
        0x00EE9977, /* orange */
        0x00D9FF09,
        0x00F02D31,
        0x0038F238,
        0x004449F0
    },

    /* Ocean */
    {
        0x0055BBAA, /* ocean green */
        0x00000022,
        0x00EEEEFF,
        0x005577AA, /* ocean blue */
        0x00F6F600,
        0x006251F0,
        0x0000F000,
        0x00F03010
    },

    /* Steel */
    {
        0x0099BBDD, /* blue-gray */
        0x00000022,
        0x00FFFFFF,
        0x006688BB,
        0x00B2DEFF,
        0x00FFA11C,
        0x00F04487,
        0x00BFFF90
    },

    /* Chocolate */
    {
        0x00AA9988, /* med-brown */
        0x00332211,
        0x00FFEEEE,
        0x00FFDDBB, /* tan-gold */
        0x00EE4444,
        0x0055DD55,
        0x00CFDBFF,
        0x000044DD
    },

    /* Pewter */
    {
        0x00CCCCBB,
        0x00000033,
        0x00FFFFFF,
        0x0099AABB, /* gray */
        0x00EE4444,
        0x0055DD55,
        0x00E0F088,
        0x000044DD
    },

    /* Wine */
    {
        0x00CC9999,
        0x00000022,
        0x00FFEEEE,
        0x00BB6677,
        0x00E0F088,
        0x007946E8,
        0x0060C752,
        0x0089EAC8
    },

    /* A2024 */
    {
        0x0A09060, /* Hedley light-gray (brown) */
        0x0000020, /* Hedley black */
        0x0F0F0F0, /* Hedley white */
        0x0707090, /* Hedley dark-gray (blue) */
	0x0F0C0C0,
	0x0B08010,
	0x030D030,
	0x0F03010
    }
};


/*****************************************************************************/


#define IFFPrefChunkCnt 3
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_PALT,
    ID_ILBM, ID_CMAP
};


/*****************************************************************************/


static struct PrefHeader far IFFPrefHeader =
{
    0,   /* version */
    0,   /* type    */
    0    /* flags   */
};


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
UWORD    i;
EdStatus result;

    result = ES_NO_COLORWHEEL;
    if (ed->ed_ColorWheelBase = OpenLibrary("gadgets/colorwheel.gadget",39))
    {
        result = ES_NO_GRADIENT;
        if (ed->ed_GradientSliderBase = OpenLibrary("gadgets/gradientslider.gadget",39))
        {
            result = ES_NO_MEMORY;
            if (ed->ed_WBScreen = LockPubScreen("Workbench"))
            {
                for (i = 0; i < MAXCOLORS; i++)
                {
                    ed->ed_PrefsDefaults.pap_Colors[i].ColorIndex = i;
                    ed->ed_PrefsDefaults.pap_Colors[i].Red        = Make16((DefaultColors[i] / 256) % 16,4);
                    ed->ed_PrefsDefaults.pap_Colors[i].Green      = Make16((DefaultColors[i] / 16) % 16,4);
                    ed->ed_PrefsDefaults.pap_Colors[i].Blue       = Make16(DefaultColors[i] % 16,4);
                }
                ed->ed_PrefsDefaults.pap_Colors[MAXCOLORS].ColorIndex = -1;

                for (i = 0; i < NUM_KNOWNPENS; i++)
                {
                    ed->ed_PrefsDefaults.pap_4ColorPens[i] = DefaultPens[i];
                    ed->ed_PrefsDefaults.pap_8ColorPens[i] = DefaultPens[i];
                }
                ed->ed_PrefsDefaults.pap_4ColorPens[NUM_KNOWNPENS] = ~0;
                ed->ed_PrefsDefaults.pap_8ColorPens[NUM_KNOWNPENS] = ~0;

                ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
                ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

                ed->ed_PenCallBack.h_Entry = (APTR)PenCallBack;
                ed->ed_PenCallBack.h_Data  = ed;

                ed->ed_SlidersRGB = TRUE;

                return(ES_NORMAL);
            }
            CloseLibrary(ed->ed_GradientSliderBase);
        }
        CloseLibrary(ed->ed_ColorWheelBase);
    }

    return(result);
}


/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    if (ed->ed_Cancelled)
    {
        ed->ed_PrefsWork = ed->ed_PrefsInitial;
        SetColors(ed);
    }

    UnlockPubScreen(NULL,ed->ed_WBScreen);
    CloseLibrary(ed->ed_ColorWheelBase);
    CloseLibrary(ed->ed_GradientSliderBase);
}


/*****************************************************************************/


/* any reference to colors 4-7 must be changed to PEN_C3 to PEN_C0 */
VOID MapUp(EdDataPtr ed)
{
UWORD i;

    for (i = 0; i < NUM_KNOWNPENS; i++)
    {
        if (ed->ed_PrefsWork.pap_8ColorPens[i] >= 4)
            ed->ed_PrefsWork.pap_8ColorPens[i] += PEN_C3 - 4;
    }
}


/* any reference to PEN_C3 to PEN_C0 must be changed to colors 4-7 */
VOID MapDown(EdDataPtr ed)
{
UWORD i;

    for (i = 0; i < NUM_KNOWNPENS; i++)
    {
        if (ed->ed_PrefsWork.pap_8ColorPens[i] >= PEN_C3)
            ed->ed_PrefsWork.pap_8ColorPens[i] -= PEN_C3 - 4;
    }
}


/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
struct ColorRegister creg;
LONG                 error;
UWORD                i;

    if (cn->cn_ID == ID_PALT && cn->cn_Type == ID_PREF)
    {
        if (ReadChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PalettePrefs)) == sizeof(struct PalettePrefs))
        {
            MapDown(ed);
            return(ES_NORMAL);
        }
        return(ES_IFFERROR);
    }
    else if (cn->cn_ID == ID_CMAP || cn->cn_Type == ID_ILBM)
    {
        i = 0;
        while ((i < MAXCOLORS) && ((error = ReadChunkBytes(iff,&creg,sizeof(struct ColorRegister))) == sizeof(struct ColorRegister)))
        {
            ed->ed_PrefsWork.pap_Colors[i].Red   = Make32(Make16(creg.red,8));
            ed->ed_PrefsWork.pap_Colors[i].Green = Make32(Make16(creg.green,8));
            ed->ed_PrefsWork.pap_Colors[i].Blue  = Make32(Make16(creg.blue,8));
            i++;
        }

        if (error < 0)
            return(ES_IFFERROR);

        return(ES_NORMAL);
    }

    return(ES_IFF_UNKNOWNCHUNK);
}


EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
    return(ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    MapUp(ed);

    if (!PushChunk(iff,0,ID_PALT,sizeof(struct PalettePrefs)))
    {
        if (WriteChunkBytes(iff,&ed->ed_PrefsWork,sizeof(struct PalettePrefs)) == sizeof(struct PalettePrefs))
        {
            if (!PopChunk(iff))
            {
                MapDown(ed);
                return(ES_NORMAL);
            }
        }
    }

    MapDown(ed);

    return(ES_IFFERROR);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    ed->ed_PrefsWork.pap_4ColorPens[BARTRIMPEN] = ed->ed_PrefsWork.pap_4ColorPens[BARDETAILPEN];
    ed->ed_PrefsWork.pap_8ColorPens[BARTRIMPEN] = ed->ed_PrefsWork.pap_8ColorPens[BARDETAILPEN];

    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


VOID SetGadgetAttrsP(EdDataPtr ed, struct Gadget * gad, ULONG tags,...)
{
    if (ed->ed_WheelWindow)
        SetGadgetAttrsA(gad,ed->ed_WheelWindow,NULL,(struct TagItem *)&tags);
    else
        SetGadgetAttrsA(gad,ed->ed_Window,NULL,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID GetGadgetAttrs(EdDataPtr ed, struct Gadget *gad, ULONG tags, ...)
{
    GT_GetGadgetAttrsA(gad,ed->ed_Window,NULL,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, WORD left, WORD top, WORD width, WORD height, ULONG tags, ...)
{
    DrawBevelBoxA(window->RPort,left + window->BorderRight, top + window->BorderTop,
                                width,height,(struct TagItem *)&tags);
}


/*****************************************************************************/


struct Screen *OpenPrefsScreen(EdDataPtr ed, ULONG tag1,...)
{
    return (OpenScreenTagList(NULL, (struct TagItem *)&tag1));
}


/*****************************************************************************/


#define NW_WIDTH     616
#define NW_HEIGHT    184
#define	NW_IDCMP     (IDCMP_IDCMPUPDATE | IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | SLIDERIDCMP | LISTVIEWIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP,       0},
      {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN,      0},
      {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS,    0},
      {NM_ITEM, MSG_NOTHING,                EC_NOP,       0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL,    0},

    {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP,       0},
      {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL,  0},
      {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED, 0},
      {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE,   0},
      {NM_ITEM, MSG_PAL_EDIT_PRESETS,	    EC_PRESET1,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET1,	    EC_PRESET1,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET2,	    EC_PRESET2,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET3,	    EC_PRESET3,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET4,	    EC_PRESET4,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET5,	    EC_PRESET5,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET6,	    EC_PRESET6,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET7,	    EC_PRESET7,   0},
        {NM_SUB,MSG_PAL_EDIT_PRESET8,	    EC_PRESET8,   0},
        {NM_SUB,MSG_NOTHING,                EC_NOP,       0},
        {NM_SUB,MSG_PAL_EDIT_PRESET9,	    EC_PRESET9,   0},   /* A2024 preset */

    {NM_TITLE,  MSG_OPTIONS_MENU,           EC_NOP,       0},
      {NM_ITEM, MSG_OPTIONS_SAVE_ICONS,     EC_SAVEICONS, CHECKIT|MENUTOGGLE},
      {NM_ITEM, MSG_PAL_OPTIONS_COLORMODEL, EC_RGBMODEL,  0},
        {NM_SUB,MSG_PAL_OPTIONS_RGB,	    EC_RGBMODEL,  CHECKIT},
        {NM_SUB,MSG_PAL_OPTIONS_HSB,	    EC_HSBMODEL,  CHECKIT},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

#define A2024_ITEM      19
#define COLORMODEL_ITEM 22

/* main display gadgets */
struct EdGadget far EG[] = {
    {BUTTON_KIND,   8,    168,  87,  14, MSG_SAVE_GAD,         EC_SAVE,         0},
    {BUTTON_KIND,   264,  168,  87,  14, MSG_USE_GAD,          EC_USE,          0},
    {BUTTON_KIND,   521,  168,  87,  14, MSG_CANCEL_GAD,       EC_CANCEL,       0},

    {BUTTON_KIND,   315,  145,  293, 14, MSG_PAL_SHOW_SAMPLE,  EC_SHOWSAMPLE,   0},

    {PALETTE_KIND,  15,   6,    288, 20, MSG_NOTHING,          EC_COLORPALETTE, 0},
    {SLIDER_KIND,   145,  121,  157, 11, MSG_NOTHING,          EC_SLIDER1,      0},
    {SLIDER_KIND,   145,  133,  157, 11, MSG_NOTHING,          EC_SLIDER2,      0},
    {SLIDER_KIND,   145,  145,  157, 11, MSG_NOTHING,          EC_SLIDER3,      0},

    {LISTVIEW_KIND, 315,  21,  293, 116,  MSG_NOTHING,         EC_PENS,         0},
    {PALETTE_KIND,  315,  133, 293,  20,  MSG_NOTHING,         EC_PENPALETTE,   0},

    {CYCLE_KIND,    315,  3,   293,  14, MSG_NOTHING,          EC_COLORMODE,    0},

    {BUTTON_KIND,   50,   65,  218,  14, MSG_PAL_SHOWWHEEL_GAD,EC_SHOWWHEEL,    0},

    {BUTTON_KIND,   0,    0,   87,  14, MSG_OK_GAD,            EC_WHEELOK,      0},
    {BUTTON_KIND,   0,    0,   87,  14, MSG_CANCEL_GAD,        EC_WHEELCANCEL,  0},
};


/*****************************************************************************/


ULONG CheckBitRange(EdDataPtr ed, UBYTE bit)
{
    if (bit > HIGHBITSPERGUN)
        bit = HIGHBITSPERGUN;

    if (bit < LOWBITSPERGUN)
        bit = LOWBITSPERGUN;
/*
    if (bit < HIGHBITSPERGUN)
        bit = LOWBITSPERGUN;
*/
    return((ULONG)bit);
}


/*****************************************************************************/


BOOL GetScreen(EdDataPtr ed, UWORD editDepth, UWORD scrWidth, UWORD scrHeight)
{
UWORD                i;
BOOL                 ok;
struct DimensionInfo dimInfo;
ULONG                wbMode;
ULONG                scrMode;
BOOL                 usewb;
UWORD                scrDepth;
UWORD                numAllocated;
UWORD                maxColors;
struct TagItem       vtags[2];

    ok    = TRUE;
    usewb = TRUE;

    /* allocate the exclusive colors we need for editing on the WB */
    for (i = 0; i < 8; i++)
    {
        ed->ed_ColorTable[i] = ObtainPen(ed->ed_WBScreen->ViewPort.ColorMap,-1,
                                         0,0,0,PEN_NO_SETCOLOR|PEN_EXCLUSIVE);
        if (ed->ed_ColorTable[i] == -1)
        {
            /* not enough pens on the WB screen for our needs */
            ok = FALSE;
            break;
        }
    }
    numAllocated = i;

    /* see what we could get out of a custom screen in roughly the same mode */
    wbMode = GetVPModeID(&ed->ed_WBScreen->ViewPort);

    scrMode = BestModeIDP(ed,BIDTAG_MonitorID, MONITOR_PART(wbMode),
                             BIDTAG_SourceID,  wbMode,
                             BIDTAG_Depth,     4,
                             TAG_DONE);

    if (scrMode == INVALID_ID)
       scrMode = wbMode;

    GetDisplayInfoData(NULL,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,scrMode);
    scrDepth = dimInfo.MaxDepth;

    ed->ed_Screen = ed->ed_WBScreen;  /* for now... */

    if (!ok || (ed->ed_WBScreen->BitMap.Depth < 6))
    {
        /* There are not enough pens on the WB to allow color editing,
         * or the WB screen has less than 64 colors, which means the
         * wheel would not look so hot.
         *
         * Let's figure out if we can do better by opening a custom
         * screen.
         */

        maxColors = (1 << scrDepth);

        if (maxColors >= numAllocated + 8)     /* 8 being the maximum number of pens used by the UI */
            usewb = FALSE;    /* more pens available on a separate screen than on WB, so use a separate screen */

        if (usewb && !ok)
        {
            /* So here we know we don't have enough WB pens, but we
             * also know we can't get more pens on a custom screen...
             * Sheshh, life is tough.
             *
             * The solution to this problem is to simply fall back
             * and use the UI pens for color editing. Note that if
             * we did get some pens allocated, use those for the lower
             * colors (background and text) since these are the most
             * likely colors to be edited by the user. Free all other
             * colors we might have gotten, so that the color wheel
             * and the gradient slider have something to play with.
             */

             while (numAllocated > 2)
             {
                 numAllocated--;
                 ReleasePen(ed->ed_WBScreen->ViewPort.ColorMap,ed->ed_ColorTable[numAllocated]);
             }

             i = numAllocated;
             while (i < 8)
             {
                 if (i < 4)
                     ed->ed_ColorTable[i] = i;
                 else
                     ed->ed_ColorTable[i] = i + (1 << ed->ed_Screen->BitMap.Depth) - 8;
                 i++;
             }
        }
        else if (!usewb)
        {
            /* Now we've decided to open on a custom screen */

            /* release all pens on the WB since we ain't using 'em */
            while (numAllocated > 0)
            {
                numAllocated--;
                ReleasePen(ed->ed_WBScreen->ViewPort.ColorMap,ed->ed_ColorTable[numAllocated]);
            }

            while ((1<<scrDepth) > 8 + 8 + MAXWHEELPENS + 16)
                scrDepth--;

            if (scrDepth < dimInfo.MaxDepth)
                scrDepth++;

            vtags[0].ti_Tag  = VC_IntermediateCLUpdate;
            vtags[0].ti_Data = FALSE;
            vtags[1].ti_Tag  = TAG_DONE;

            if (ed->ed_Screen = OpenPrefsScreen(ed,
                                          SA_LikeWorkbench, TRUE,
                                          SA_MinimizeISG,   TRUE,
                                          SA_Title,         GetString(&ed->ed_LocaleInfo,MSG_PREFS_NAME),
                                          SA_Width,         scrWidth,
                                          SA_Height,        scrHeight,
                                          SA_Depth,         scrDepth,
                                          SA_FullPalette,   TRUE,
                                          SA_Interleaved,   TRUE,
                                          SA_DisplayID,     scrMode,
                                          SA_PubName,       "PALETTE",
                                          SA_PubTask,       FindTask(NULL),
                                          SA_PubSig,        SIGBREAKB_CTRL_E,
                                          SA_AutoScroll,    TRUE,
                                          SA_VideoControl,  vtags,
                                          SA_Behind,        TRUE,
                                          TAG_DONE))
            {
                PubScreenStatus(ed->ed_Screen,0);

                /* allocate the editing pens on the custom screen */
                for (i = 0; i < 8; i++)
                {
                    ed->ed_ColorTable[i] = ObtainPen(ed->ed_Screen->ViewPort.ColorMap,-1,0,0,0,PEN_EXCLUSIVE | PEN_NO_SETCOLOR);
                    numAllocated++;
                }
            }
            else
            {
                return(FALSE);
            }
        }
    }
    ed->ed_NumColorsAllocated = numAllocated;

    ed->ed_DoWheelScreen = FALSE;
    if (scrDepth < 5)
        ed->ed_DoWheelScreen = TRUE;

    return(TRUE);
}


/*****************************************************************************/


BOOL InitDisp(EdDataPtr ed)
{
UWORD height, depth;

    depth = ed->ed_WBScreen->BitMap.Depth;
    if (depth > MAXDEPTH)
        depth = MAXDEPTH;
    else if (depth < 2)
        depth = 2;

    ed->ed_Depth = depth;
    ed->ed_4ColorMode = (depth == 2);

    height = NW_HEIGHT - 2;
    if (ed->ed_WBScreen->Height - ed->ed_TopBorder >= NW_HEIGHT + SAMPLE_HEIGHT + 5)
    {
        ed->ed_EmbeddedSample = TRUE;
        height += SAMPLE_HEIGHT + 5;
    }

    if (GetScreen(ed,depth,NW_WIDTH,
                           height + ed->ed_WBScreen->BarHeight))
    {
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID MakeColorWheel(EdDataPtr ed)
{
UWORD left;
UWORD wsize;
UWORD i;
UWORD gleft, gtop, gwidth, gheight;
UWORD wleft, wtop, wwidth, wheight;

    if (ed->ed_WheelScreen)
    {
        ed->ed_GradPens[31]         = ~0;
        ed->ed_GradPens[30]         = ObtainBestPenA(ed->ed_WheelScreen->ViewPort.ColorMap,0,0,0,NULL);
        ed->ed_NumGradPensAllocated = 1;
        ed->ed_FirstGradPen         = 30;

        /* figure out the width of the color wheel in order to make it as
         * square as possible (square wheel, yeah I know :-)
         */

        if (!ed->ed_DoWheelScreen)
        {
            wsize = 91 * ed->ed_DrawInfo->dri_Resolution.Y / ed->ed_DrawInfo->dri_Resolution.X;
            if (wsize < 30)
                wsize = 30;
            else if (wsize > 293 - 21)
                wsize = 293 - 21;

            left = (293 - (wsize + 21)) / 2 + 10 + ed->ed_LeftBorder;

            gleft   = left + wsize + 3;
            gtop    = 28 + ed->ed_TopBorder;
            gwidth  = 18;
            gheight = 91;

            wleft   = left;
            wtop    = 28 + ed->ed_TopBorder;
            wwidth  = wsize;
            wheight = 91;
        }
        else
        {
            wheight = ed->ed_WheelScreen->Height - 1;
            wwidth  = wheight * ed->ed_WheelDrawInfo->dri_Resolution.Y / ed->ed_WheelDrawInfo->dri_Resolution.X;
            if (wwidth > ed->ed_WheelScreen->Width - 96 - 21)
            {
                wwidth  = ed->ed_WheelScreen->Width - 96 - 21;
                wheight = wwidth * ed->ed_WheelDrawInfo->dri_Resolution.X / ed->ed_WheelDrawInfo->dri_Resolution.Y;
            }

            wleft   = (ed->ed_WheelScreen->Width - wwidth - 96 - 21) / 2;
            wtop    = (ed->ed_WheelScreen->Height - wheight) / 2;

            gleft   = wleft + wwidth + 3;
            gtop    = wtop;
            gwidth  = 18;
            gheight = wheight;
        }

        ed->ed_GradientSlider = (struct Gadget *)NewPrefsObject(ed,NULL,"gradientslider.gadget",
                                               GA_Left,        gleft,
                                               GA_Top,         gtop,
                                               GA_Width,       gwidth,
                                               GA_Height,      gheight,
                                               GA_ID,          EC_GRADIENTSLIDER,
                                               GA_RelVerify,   TRUE,
                                               GA_Immediate,   TRUE,
                                               GA_FollowMouse, TRUE,
                                               PGA_Freedom,    LORIENT_VERT,
                                               ICA_TARGET,     ICTARGET_IDCMP,
                                               TAG_END);

        ed->ed_ColorWheel = (struct Gadget *)NewPrefsObject(ed,NULL,"colorwheel.gadget",
                                                   GA_Left,        wleft,
                                                   GA_Top,         wtop,
                                                   GA_Width,       wwidth,
                                                   GA_Height,      wheight,
                                                   GA_ID,          EC_COLORWHEEL,
                                                   GA_Previous,    ed->ed_GradientSlider,
                                                   GA_RelVerify,   TRUE,
                                                   GA_Immediate,   TRUE,
                                                   WHEEL_Screen,   ed->ed_WheelScreen,
                                                   WHEEL_GradientSlider, ed->ed_GradientSlider,
                                                   WHEEL_Abbrv,    GetString(&ed->ed_LocaleInfo,MSG_PAL_WHEEL_ABBRV),
                                                   WHEEL_MaxPens,  MAXWHEELPENS,
                                                   ICA_TARGET,     ICTARGET_IDCMP,
                                                   TAG_END);

        if (ed->ed_GradientSlider)
        {
            /* allocate pens for the gradient slider */
            i = 30;
            while (i > 0)
            {
                i--;
                ed->ed_GradPens[i] = ObtainPen(ed->ed_WheelScreen->ViewPort.ColorMap,-1,0,0,0,PEN_EXCLUSIVE | PEN_NO_SETCOLOR);
                if (ed->ed_GradPens[i] == -1)
                    break;

                ed->ed_NumGradPensAllocated++;
                ed->ed_FirstGradPen = i;
            }

            SetGadgetAttrsP(ed,ed->ed_GradientSlider,GRAD_PenArray,&ed->ed_GradPens[ed->ed_FirstGradPen],
                                                     TAG_DONE);
        }
        else
        {
            ReleasePen(ed->ed_WheelScreen->ViewPort.ColorMap,ed->ed_GradPens[30]);
            ed->ed_NumGradPensAllocated = 0;
        }
    }
}


/*****************************************************************************/


struct Gadget *DoWheelGadget(EdDataPtr ed, struct EdGadget *eg,
                             struct Gadget *gad, ULONG tags, ...)
{
struct NewGadget ng;

    ng.ng_LeftEdge   = eg->eg_LeftEdge;
    ng.ng_TopEdge    = eg->eg_TopEdge;
    ng.ng_Width      = eg->eg_Width;
    ng.ng_Height     = eg->eg_Height;
    ng.ng_GadgetText = GetString(&ed->ed_LocaleInfo,eg->eg_Label);
    ng.ng_TextAttr   = &ed->ed_TextAttr;
    ng.ng_GadgetID   = eg->eg_Command;
    ng.ng_Flags      = eg->eg_GadgetFlags;
    ng.ng_VisualInfo = ed->ed_VisualInfo;
    ng.ng_UserData   = NULL;

    return(ed->ed_WheelLastAdded = CreateGadgetA(eg->eg_GadgetKind,ed->ed_WheelLastAdded,
                                                 &ng,(struct TagItem *)&tags));
}


/*****************************************************************************/


VOID DrawWheelBB(EdDataPtr ed, WORD left, WORD top, WORD width, WORD height, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_WheelWindow->RPort,left, top, width, height, (struct TagItem *)&tags);
}


/*****************************************************************************/


VOID SleepControl(EdDataPtr ed, BOOL sleeping)
{
struct TagItem tags[3];

    tags[0].ti_Tag  = WA_BusyPointer;
    tags[0].ti_Data = sleeping;
    tags[1].ti_Tag  = TAG_DONE;
    SetWindowPointerA(window,tags);

    if (sleeping)
    {
        InitRequester(&ed->ed_SleepReq);
        ed->ed_ASleep = Request(&ed->ed_SleepReq,window);
    }
    else if (ed->ed_ASleep)
    {
        EndRequest(&ed->ed_SleepReq,window);
    }
}


/*****************************************************************************/


VOID OpenWheelScreen(EdDataPtr ed)
{
ULONG                modeID;
struct DimensionInfo dimInfo;
WORD                 step;
WORD                 old;
struct EdGadget      eg;
struct ColorWheelRGB rgb;
struct TagItem       vtags[2];

    if (ed->ed_WheelScreen)
    {
        ScreenToFront(ed->ed_WheelScreen);
    }
    else
    {
        SleepControl(ed,TRUE);

        modeID = GetVPModeID(&ed->ed_Screen->ViewPort);
        GetDisplayInfoData(NULL,(UBYTE *)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,modeID);

        vtags[0].ti_Tag  = VC_IntermediateCLUpdate;
        vtags[0].ti_Data = FALSE;
        vtags[1].ti_Tag  = TAG_DONE;

        if (ed->ed_WheelScreen = OpenPrefsScreen(ed,
               SA_Parent,        ed->ed_Screen,
               SA_Quiet,         TRUE,
               SA_Top,           dimInfo.TxtOScan.MaxY,
               SA_Width,         dimInfo.TxtOScan.MaxX - dimInfo.TxtOScan.MinX + 1,
               SA_Height,        dimInfo.TxtOScan.MaxY - dimInfo.TxtOScan.MinY + 1,
               SA_Depth,         dimInfo.MaxDepth,
               SA_FullPalette,   TRUE,
               SA_Interleaved,   TRUE,
               SA_DisplayID,     modeID,
               SA_Draggable,     FALSE,
               SA_SharePens,     TRUE,
               SA_ShowTitle,     FALSE,
               SA_LikeWorkbench, TRUE,
               SA_MinimizeISG,   TRUE,
               SA_BackFill,      LAYERS_NOBACKFILL,
               SA_AutoScroll,    FALSE,
               SA_Behind,        TRUE,
               SA_VideoControl,  vtags,
               TAG_DONE))
        {
            if (ed->ed_WheelDrawInfo = GetScreenDrawInfo(ed->ed_WheelScreen))
            {
                if (ed->ed_WheelVisualInfo = GetVisualInfoA(ed->ed_WheelScreen,NULL))
                {
                    ed->ed_WheelSamplePen = ObtainPen(ed->ed_WheelScreen->ViewPort.ColorMap,-1,
                                                      0,0,0,PEN_NO_SETCOLOR|PEN_EXCLUSIVE);
                    ed->ed_WheelGadgets   = NULL;
                    ed->ed_WheelLastAdded = CreateContext(&ed->ed_WheelGadgets);

                    eg = EG[12];
                    eg.eg_LeftEdge = ed->ed_WheelScreen->Width - 90;
                    eg.eg_TopEdge  = 60;

                    DoWheelGadget(ed, &eg, NULL, TAG_DONE);

                    eg = EG[13];
                    eg.eg_LeftEdge = ed->ed_WheelScreen->Width - 90;
                    eg.eg_TopEdge  = 80;

                    DoWheelGadget(ed, &eg, NULL, TAG_DONE);

                    if (ed->ed_WheelLastAdded)
                    {
                        if (ed->ed_WheelWindow = OpenPrefsWindow(ed,
                                      WA_Width,         ed->ed_WheelScreen->Width,
                                      WA_Height,        ed->ed_WheelScreen->Height,
                                      WA_Backdrop,      TRUE,
                                      WA_Borderless,    TRUE,
                                      WA_SimpleRefresh, TRUE,
                                      WA_CustomScreen,  ed->ed_WheelScreen,
                                      WA_Gadgets,       ed->ed_WheelGadgets,
                                      WA_RMBTrap,       TRUE,
                                      TAG_DONE))
                        {
                            ed->ed_WheelWindow->UserPort = ed->ed_Window->UserPort;
                            ModifyIDCMP(ed->ed_WheelWindow,IDCMP_IDCMPUPDATE | IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_GADGETUP);

                            MakeColorWheel(ed);

                            AddGList(ed->ed_WheelWindow, ed->ed_GradientSlider, 0, 2, NULL);
                            RefreshGList(ed->ed_GradientSlider, ed->ed_WheelWindow, NULL, 2);
                            GT_RefreshWindow(ed->ed_WheelWindow,NULL);

                            DoWheel(ed);
                            DoBrightness(ed);

                            GetAttr(WHEEL_RGB,ed->ed_ColorWheel,(ULONG *)&rgb);
                            SetRGB32(&ed->ed_WheelScreen->ViewPort,ed->ed_WheelSamplePen,rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue);

                            DrawWheelBB(ed,ed->ed_WheelScreen->Width - 90, 40,
                                           87,14,GTBB_Recessed, TRUE,
                                                 GT_VisualInfo, ed->ed_WheelVisualInfo,
                                                 TAG_DONE);

                            SetAPen(ed->ed_WheelWindow->RPort,ed->ed_WheelSamplePen);
                            RectFill(ed->ed_WheelWindow->RPort,ed->ed_WheelScreen->Width - 86,42,
                                                               ed->ed_WheelScreen->Width - 8,51);

                            ScreenDepth(ed->ed_WheelScreen,SDEPTH_TOFRONT|SDEPTH_INFAMILY,NULL);

                            step = ed->ed_WheelScreen->TopEdge / 15;

                            while (ed->ed_WheelScreen->TopEdge)
                            {
                                old = ed->ed_WheelScreen->TopEdge;
                                ScreenPosition(ed->ed_WheelScreen,SPOS_RELATIVE|SPOS_FORCEDRAG,0,-step,0,0);

                                if (old == ed->ed_WheelScreen->TopEdge)
                                    break;
                            }

                            ScreenPosition(ed->ed_WheelScreen,SPOS_RELATIVE|SPOS_FORCEDRAG,- (ed->ed_WheelScreen->TopEdge - ed->ed_Screen->TopEdge),0,0,0);

                            ActivateWindow(ed->ed_WheelWindow);

                            return;
                        }
                    }
                    FreeGadgets(ed->ed_WheelGadgets);
                    FreeVisualInfo(ed->ed_VisualInfo);
                }
                FreeScreenDrawInfo(ed->ed_WheelScreen,ed->ed_WheelDrawInfo);
            }
            CloseScreen(ed->ed_WheelScreen);
        }

        ed->ed_WheelWindow     = NULL;
        ed->ed_WheelScreen     = NULL;
        ed->ed_WheelDrawInfo   = NULL;
        ed->ed_WheelVisualInfo = NULL;
        ed->ed_WheelGadgets    = NULL;
        DisplayBeep(NULL);

        SleepControl(ed,FALSE);
    }
}


/*****************************************************************************/


VOID CloseWheelScreen(EdDataPtr ed)
{
WORD                 i, step;
struct DimensionInfo dimInfo;
struct DisplayInfo   dispInfo;

    if (ed->ed_WheelScreen)
    {
        if (ed->ed_DoWheelScreen)
        {
            ActivateWindow(window);

            GetDisplayInfoData(NULL,(UBYTE *)&dispInfo,sizeof(struct DisplayInfo),DTAG_DIMS,GetVPModeID(&ed->ed_WheelScreen->ViewPort));

            if (!(dispInfo.PropertyFlags & DIPF_IS_DRAGGABLE))
            {
                GetDisplayInfoData(NULL,(UBYTE *)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,GetVPModeID(&ed->ed_WheelScreen->ViewPort));
                step = dimInfo.TxtOScan.MaxY / 15;

                while (ed->ed_WheelScreen->TopEdge < dimInfo.TxtOScan.MaxY)
                    ScreenPosition(ed->ed_WheelScreen,SPOS_RELATIVE|SPOS_FORCEDRAG,0,step,0,0);

                ScreenDepth(ed->ed_WheelScreen,SDEPTH_TOBACK|SDEPTH_INFAMILY,NULL);
            }
        }

        ClosePrefsWindow(ed,ed->ed_WheelWindow,TRUE);

        FreeGadgets(ed->ed_WheelGadgets);

        for (i = ed->ed_FirstGradPen; i < ed->ed_FirstGradPen + ed->ed_NumGradPensAllocated; i++)
            ReleasePen(ed->ed_WheelScreen->ViewPort.ColorMap,ed->ed_GradPens[i]);

        if (ed->ed_DoWheelScreen)
        {
            DisposeObject(ed->ed_ColorWheel);
            DisposeObject(ed->ed_GradientSlider);
            FreeScreenDrawInfo(ed->ed_WheelScreen,ed->ed_WheelDrawInfo);
            FreeVisualInfo(ed->ed_WheelVisualInfo);
            ReleasePen(ed->ed_WheelScreen->ViewPort.ColorMap,ed->ed_WheelSamplePen);
            CloseScreen(ed->ed_WheelScreen);

            ed->ed_WheelScreen     = NULL;
            ed->ed_WheelWindow     = NULL;
            ed->ed_WheelDrawInfo   = NULL;
            ed->ed_WheelVisualInfo = NULL;
            ed->ed_WheelGadgets    = NULL;
            ed->ed_ColorWheel     = NULL;
            ed->ed_GradientSlider = NULL;

            SleepControl(ed,FALSE);
        }
    }
}


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD              zoomSize[4];
ULONG              id;
struct DisplayInfo di;
struct EdMenu      stash[sizeof(EM)/sizeof(struct EdMenu)];
UWORD              i;
UWORD              height;
UWORD              width;
ULONG              windowFlags;
ULONG              zoomTag;
ULONG              titleTag;
struct MenuItem   *item1;
struct MenuItem   *item2;

    if (!ed->ed_DoWheelScreen)
    {
        ed->ed_WheelScreen   = ed->ed_Screen;
        ed->ed_WheelDrawInfo = ed->ed_DrawInfo;
    }

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_ColorModeLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_PAL_4COLOR_MODE);
    ed->ed_ColorModeLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_PAL_MULTICOLOR_MODE);

    CopyMem(EM,stash,sizeof(stash));

    windowFlags       = NW_FLAGS;
    zoomTag           = WA_Zoom;
    titleTag          = WA_Title;
    ed->ed_LeftBorder = ed->ed_Screen->WBorLeft;
    ed->ed_TopBorder  = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;
    if (ed->ed_Screen != ed->ed_WBScreen)
    {
        windowFlags       = (WFLG_ACTIVATE | WFLG_SIMPLE_REFRESH | WFLG_BORDERLESS | WFLG_BACKDROP);
        zoomTag           = TAG_IGNORE;
        titleTag          = TAG_IGNORE;
        ed->ed_LeftBorder = 0;
        ed->ed_TopBorder  = 0;
    }

    width  = NW_WIDTH;
    height = NW_HEIGHT;
    if (ed->ed_Screen->Height - ed->ed_TopBorder >= NW_HEIGHT + SAMPLE_HEIGHT)
    {
        ed->ed_EmbeddedSample = TRUE;
        height += SAMPLE_HEIGHT;
    }

    if ((!GetDisplayInfoData(NULL,(APTR)&di,sizeof(struct DisplayInfo),DTAG_DISP,A2024_MONITOR_ID))
    || (di.NotAvailable))
    {
        /* if there's no A2024 installed in this machine, zap the menu item */
        stash[A2024_ITEM-1].em_Type = NM_IGNORE;
        stash[A2024_ITEM].em_Type = NM_IGNORE;
    }

    id = GetVPModeID(&ed->ed_WBScreen->ViewPort);
    GetDisplayInfoData(NULL,(APTR)&di,sizeof(struct DisplayInfo),DTAG_DISP,id);

    ed->ed_RedBits   = CheckBitRange(ed,di.RedBits);
    ed->ed_GreenBits = CheckBitRange(ed,di.GreenBits);
    ed->ed_BlueBits  = CheckBitRange(ed,di.BlueBits);

    NewList(&ed->ed_PenList);
    for (i=0; i<9; i++)
    {
        ed->ed_PenNodes[i].ln_Name = GetString(&ed->ed_LocaleInfo,i+MSG_PAL_BACKGROUND_PEN);
        ed->ed_PenNodes[i].ln_Pri  = i;
        AddTail(&ed->ed_PenList,&ed->ed_PenNodes[i]);
    }

    MakeColorWheel(ed);
    RenderGadgets(ed);

    if (ed->ed_Screen != ed->ed_WBScreen)
        ScreenToFront(ed->ed_Screen);

    if ((ed->ed_LastAdded)
    &&  (ed->ed_Menus = CreatePrefsMenus(ed,stash))
    &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,    width,
                                            WA_InnerHeight,   height,
                                            WA_MinWidth,      NW_MINWIDTH,
                                            WA_MinHeight,     height,
                                            WA_MaxWidth,      NW_MAXWIDTH,
                                            WA_MaxHeight,     NW_MAXHEIGHT,
                                            WA_IDCMP,         NW_IDCMP,
                                            WA_Flags,         windowFlags,
                                            zoomTag,          zoomSize,
                                            WA_AutoAdjust,    TRUE,
                                            WA_CustomScreen,  ed->ed_Screen,
                                            titleTag,         GetString(&ed->ed_LocaleInfo,MSG_PAL_NAME),
                                            WA_NewLookMenus,  TRUE,
                                            WA_Gadgets,       ed->ed_Gadgets,
                                            TAG_DONE)))
    {
        if (!ed->ed_DoWheelScreen)
        {
            AddGList(window, ed->ed_GradientSlider, 0, 2, NULL);
            RefreshGList(ed->ed_GradientSlider, window, NULL, 2);
        }

        item1 = ItemAddress(ed->ed_Menus,FULLMENUNUM(2,1,0));
        item2 = item1->NextItem;

        item1->MutualExclude = 2;
        item2->MutualExclude = 1;

        item1->Flags &= ~CHECKED;
        item2->Flags &= ~CHECKED;

        if (ed->ed_Screen != ed->ed_WBScreen)
        {
            UnlockPubScreen(NULL,ed->ed_WBScreen);
            ed->ed_WBScreen = NULL;
        }

        CreateClickMap(ed);

        return(TRUE);
    }

    DisposeDisplay(ed);
    return(FALSE);
}


/*****************************************************************************/


VOID ClosePrefsWindow(EdDataPtr ed, struct Window *wp, BOOL others)
{
struct IntuiMessage *msg;
struct Node         *succ;

    if (wp)
    {
        ClearMenuStrip(wp);

        if (wp->UserPort)
        {
            Forbid();

            msg = (struct IntuiMessage *) wp->UserPort->mp_MsgList.lh_Head;
            while (succ = msg->ExecMessage.mn_Node.ln_Succ)
            {
                if (msg->IDCMPWindow == wp)
                {
                    Remove(msg);
                    ReplyMsg(msg);
                }
                msg = (struct IntuiMessage *) succ;
            }

            if (others)
                wp->UserPort = NULL;

            ModifyIDCMP(wp,NULL);
            Permit();
        }

        CloseWindow(wp);
    }
}


/*****************************************************************************/


VOID DisposeDisplay(EdDataPtr ed)
{
UWORD i;

    DisposeClickMap(ed);

    CloseWheelScreen(ed);

    ClosePrefsWindow(ed,ed->ed_SampleWindow,TRUE);
    ClosePrefsWindow(ed,ed->ed_Window,FALSE);

    FreeMenus(ed->ed_Menus);
    FreeGadgets(ed->ed_Gadgets);

    if (ed->ed_ColorWheel)
        DisposeObject(ed->ed_ColorWheel);

    if (ed->ed_GradientSlider)
        DisposeObject(ed->ed_GradientSlider);

    for (i = 0; i < ed->ed_NumColorsAllocated; i++)
        ReleasePen(ed->ed_Screen->ViewPort.ColorMap,ed->ed_ColorTable[i]);

    if (ed->ed_Screen != ed->ed_WBScreen)
    {
        while (!CloseScreen(ed->ed_Screen))
            Wait(SIGBREAKF_CTRL_E);

        ed->ed_Screen = NULL;
    }
}


/*****************************************************************************/


static VOID __asm PenCallBack(register __a0 struct Hook *hook,
                              register __a1 struct LVDrawMsg *msg,
                              register __a2 struct Node *node)
{
EdDataPtr          ed    = hook->h_Data;
struct RastPort   *rp    = msg->lvdm_RastPort;
UBYTE              state = msg->lvdm_State;
struct TextExtent  extent;
ULONG              fit;
WORD               x,y;
WORD               slack;
ULONG              apen;
ULONG              bpen;
UWORD             *uipens = msg->lvdm_DrawInfo->dri_Pens;
STRPTR             name;
UWORD             *pens;

    if (ed->ed_Depth <= 2)
        pens = ed->ed_PrefsWork.pap_4ColorPens;
    else
        pens = ed->ed_PrefsWork.pap_8ColorPens;

    apen = uipens[FILLTEXTPEN];
    bpen = uipens[FILLPEN];
    if (state == LVR_NORMAL)
    {
        apen = uipens[TEXTPEN];
        bpen = uipens[BACKGROUNDPEN];
    }
    SetABPenDrMd(rp,apen,bpen,JAM2);

    name = node->ln_Name;

    fit = TextFit(rp,name,strlen(name),&extent,NULL,1,
                  msg->lvdm_Bounds.MaxX-msg->lvdm_Bounds.MinX-3-20,
                  msg->lvdm_Bounds.MaxY-msg->lvdm_Bounds.MinY+1);

    slack = (msg->lvdm_Bounds.MaxY - msg->lvdm_Bounds.MinY) - (extent.te_Extent.MaxY - extent.te_Extent.MinY);

    x = msg->lvdm_Bounds.MinX - extent.te_Extent.MinX + 2 + 20;
    y = msg->lvdm_Bounds.MinY - extent.te_Extent.MinY + ((slack+1) / 2);

    extent.te_Extent.MinX += x - 20;
    extent.te_Extent.MaxX += x;
    extent.te_Extent.MinY += y;
    extent.te_Extent.MaxY += y;

    Move(rp,x,y);
    Text(rp,name,fit);

    x -= 20;

    SetAPen(rp,bpen);
    FillOldExtent(rp,&msg->lvdm_Bounds,&extent.te_Extent,GfxBase);
    RectFill(rp,x+17,y-6,x+19,y+1);

    SetAPen(rp,ed->ed_ColorTable[pens[NodeMap[node->ln_Pri]]]);
    RectFill(rp,x+2,y-5,x+14,y);

    if (state == LVR_SELECTED)
        SetAPen(rp,uipens[FILLTEXTPEN]);
    else
        SetAPen(rp,uipens[TEXTPEN]);

    RectFill(rp,x,y-6,x+16,y-6);
    RectFill(rp,x,y-5,x+1,y);
    RectFill(rp,x+15,y-5,x+16,y);
    RectFill(rp,x,y+1,x+16,y+1);
}


/*****************************************************************************/


/* "value" is in the current bit size */
ULONG Make16(ULONG value, ULONG bits)
{
ULONG result;
ULONG mask;

    if (bits == 16)
        return(value);

    mask   = value << (16 - bits);
    result = 0;
    while (mask)
    {
        result |= mask;
        mask    = mask >> bits;
    }

    return(result);
}


/*****************************************************************************/


VOID SetColors(EdDataPtr ed)
{
UWORD                color;
UWORD		     maxColor;
struct ColorSpec    *cspec;
struct LoadRGBTable  lrt[MAXCOLORS+1];

    if (ed->ed_Screen)
    {
        cspec    = ed->ed_PrefsWork.pap_Colors;
        maxColor = (1<<ed->ed_Depth);

        for (color = 0; color < maxColor; color++)
        {
            lrt[color].NumColors  = 1;
            lrt[color].FirstColor = ed->ed_ColorTable[color];
            lrt[color].Red        = Make32(cspec[color].Red);
            lrt[color].Green      = Make32(cspec[color].Green);
            lrt[color].Blue       = Make32(cspec[color].Blue);
        }
        lrt[maxColor].NumColors = 0;

        LoadRGB32(&ed->ed_Screen->ViewPort,(ULONG *)lrt);
    }
}


/*****************************************************************************/


VOID DoWheel(EdDataPtr ed)
{
struct ColorSpec *cspec;

    if (ed->ed_ColorWheel)
    {
        cspec = &ed->ed_PrefsWork.pap_Colors[ed->ed_CurrentColor];

        SetGadgetAttrsP(ed,ed->ed_ColorWheel,WHEEL_Red,   Make32(cspec->Red),
                                             WHEEL_Green, Make32(cspec->Green),
                                             WHEEL_Blue,  Make32(cspec->Blue),
                                             TAG_DONE);
    }
}


/*****************************************************************************/


static VOID DoBrightness(EdDataPtr ed)
{
struct ColorWheelHSB hsb;
struct ColorWheelRGB rgb;
UWORD                i,j;
struct LoadRGBTable  lrt[32];

    if (ed->ed_ColorWheel)
    {
        GetAttr(WHEEL_HSB,ed->ed_ColorWheel,(ULONG *)&hsb);

        hsb.cw_Hue        = hsb.cw_Hue;
        hsb.cw_Saturation = hsb.cw_Saturation;

        /* set all colors except the last, which is always black */
        i = ed->ed_FirstGradPen;
        j = 0;
        while (i < ed->ed_NumGradPensAllocated + ed->ed_FirstGradPen - 1)
        {
            hsb.cw_Brightness = 0xffff * (ed->ed_NumGradPensAllocated - (i - ed->ed_FirstGradPen)) / ed->ed_NumGradPensAllocated;
            hsb.cw_Brightness = (hsb.cw_Brightness << 16) + hsb.cw_Brightness;
            ConvertHSBToRGB(&hsb,&rgb);

            lrt[j].NumColors  = 1;
            lrt[j].FirstColor = ed->ed_GradPens[i];
            lrt[j].Red        = rgb.cw_Red;
	    lrt[j].Green      = rgb.cw_Green;
            lrt[j].Blue       = rgb.cw_Blue;
            i++;
            j++;
        }

        lrt[j].NumColors = 0;

        LoadRGB32(&ed->ed_WheelScreen->ViewPort,(ULONG *)lrt);
    }
}


/*****************************************************************************/


VOID DoSliders(EdDataPtr ed)
{
struct ColorSpec     *cspec;
struct EdGadget       eg;
struct ColorWheelHSB  hsb;
struct ColorWheelRGB  rgb;
ULONG                 level1, level2, level3;
UWORD                 max1, max2, max3;

    cspec = &ed->ed_PrefsWork.pap_Colors[ed->ed_CurrentColor];
    if (ed->ed_SlidersRGB)
    {
        level1 = cspec->Red >> (HIGHBITSPERGUN-ed->ed_RedBits);
        max1   = (1<<ed->ed_RedBits)-1;
        level2 = cspec->Green >> (HIGHBITSPERGUN-ed->ed_GreenBits);
        max2   = (1<<ed->ed_GreenBits)-1;
        level3 = cspec->Blue >> (HIGHBITSPERGUN-ed->ed_BlueBits);
        max3   = (1<<ed->ed_BlueBits)-1;
    }
    else
    {
        if (ed->ed_ColorWheel)
        {
            GetAttr(WHEEL_HSB,ed->ed_ColorWheel,(ULONG *)&hsb);
        }
        else
        {
            rgb.cw_Red   = Make32(cspec->Red);
            rgb.cw_Green = Make32(cspec->Green);
            rgb.cw_Blue  = Make32(cspec->Blue);
            ConvertRGBToHSB(&rgb,&hsb);
        }

        level1 = (hsb.cw_Hue >> 24);
        max1   = 255;
        level2 = (hsb.cw_Saturation >> 24);
        max2   = 255;
        level3 = (hsb.cw_Brightness >> 24);
        max3   = 255;
    }

    eg = EG[5];
    eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
    eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

    if (MONITOR_PART(GetVPModeID(&ed->ed_Screen->ViewPort)) == A2024_MONITOR_ID)
        eg.eg_TopEdge -= 40;

    ed->ed_Slider1 = DoPrefsGadget(ed,&eg,ed->ed_Slider1,
                                      GTSL_Level,       level1,
                                      GTSL_Min,         0,
                                      GTSL_Max,         max1,
                                      GTSL_MaxLevelLen, 3,
                                      GTSL_LevelFormat, "%3ld",
                                      GA_RelVerify,     TRUE,
                                      GA_Immediate,     TRUE,
                                      TAG_DONE);

    eg = EG[6];
    eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
    eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

    if (MONITOR_PART(GetVPModeID(&ed->ed_Screen->ViewPort)) == A2024_MONITOR_ID)
        eg.eg_TopEdge -= 40;

    ed->ed_Slider2 = DoPrefsGadget(ed,&eg,ed->ed_Slider2,
                                        GTSL_Level,       level2,
                                        GTSL_Min,         0,
                                        GTSL_Max,         max2,
                                        GTSL_MaxLevelLen, 3,
                                        GTSL_LevelFormat, "%3ld",
                                        GA_RelVerify,     TRUE,
                                        GA_Immediate,     TRUE,
                                        TAG_DONE);

    eg = EG[7];
    eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
    eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

    if (MONITOR_PART(GetVPModeID(&ed->ed_Screen->ViewPort)) == A2024_MONITOR_ID)
        eg.eg_TopEdge -= 40;

    ed->ed_Slider3 = DoPrefsGadget(ed,&eg,ed->ed_Slider3,
                                       GTSL_Level,       level3,
                                       GTSL_Min,         0,
                                       GTSL_Max,         max3,
                                       GTSL_MaxLevelLen, 3,
                                       GTSL_LevelFormat, "%3ld",
                                       GA_RelVerify,     TRUE,
                                       GA_Immediate,     TRUE,
                                       TAG_DONE);
}


/*****************************************************************************/


VOID RenderSliderNames(EdDataPtr ed)
{
struct RastPort *rp = window->RPort;
STRPTR           str;
UWORD            len, plen;
UWORD            i;
ULONG            offset;
ULONG            y;

    SetABPenDrMd(rp,ed->ed_DrawInfo->dri_Pens[TEXTPEN],
                    ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN],
                    JAM2);

    offset = MSG_PAL_RED_GAD;
    y      = 129;

    if (MONITOR_PART(GetVPModeID(&ed->ed_Screen->ViewPort)) == A2024_MONITOR_ID)
        y -= 40;

    if (!ed->ed_SlidersRGB)
        offset = MSG_PAL_HUE_GAD;

    i = 0;
    while (i < 3)
    {
       str  = GetString(&ed->ed_LocaleInfo,i+offset),
       len  = strlen(str);
       plen = TextLength(rp,str,len);
       Move(rp,110+ed->ed_LeftBorder-plen,y+ed->ed_TopBorder + 12*i);
       Text(rp,str,len);
       i++;
    }
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
BOOL refresh;

    /* if either window is in refresh state, only redraw that one, and
     * not the other window
     */

    refresh = FALSE;
    if ((WFLG_WINDOWREFRESH & window->Flags)
    ||  (ed->ed_SampleWindow && (WFLG_WINDOWREFRESH & ed->ed_SampleWindow->Flags)))
        refresh = TRUE;

    if (ed->ed_EmbeddedSample)
    {
        if (!refresh || (window->Flags & WFLG_WINDOWREFRESH))
        {
            SetAPen(window->RPort,ed->ed_DrawInfo->dri_Pens[SHADOWPEN]);
            RectFill(window->RPort,ed->ed_LeftBorder,
                                   ed->ed_TopBorder + 186,
                                   window->Width - window->BorderRight - 1,
                                   ed->ed_TopBorder + 186);

            SetAPen(window->RPort,ed->ed_DrawInfo->dri_Pens[SHINEPEN]);
            RectFill(window->RPort,ed->ed_LeftBorder,
                                   ed->ed_TopBorder + 187,
                                   window->Width - window->BorderRight - 1,
                                   ed->ed_TopBorder + 187);

            if (ed->ed_Depth <= 2)
                RenderSample(ed,ed->ed_PrefsWork.pap_4ColorPens,TRUE);
            else
                RenderSample(ed,ed->ed_PrefsWork.pap_8ColorPens,TRUE);
        }
    }

    if (ed->ed_SampleWindow)
    {
        if (!refresh || (ed->ed_SampleWindow->Flags & WFLG_WINDOWREFRESH))
        {
            if (ed->ed_Depth <= 2)
                RenderSample(ed,ed->ed_PrefsWork.pap_4ColorPens,TRUE);
            else
                RenderSample(ed,ed->ed_PrefsWork.pap_8ColorPens,TRUE);
        }
    }

    if (!refresh || (window->Flags & WFLG_WINDOWREFRESH))
    {
        DrawBB(ed,8,3,299,156,GT_VisualInfo,ed->ed_VisualInfo,
                              TAG_DONE);

        RenderSliderNames(ed);
    }
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
UWORD            depth;
UWORD           *pens;
struct EdGadget  eg;

    depth = ed->ed_Depth;

    if (!ed->ed_LastAdded)
    {
	ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);

        eg = EG[0];
        eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
        eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

        DoPrefsGadget(ed, &eg, NULL, TAG_DONE);

        eg = EG[1];
        eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
        eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

        DoPrefsGadget(ed, &eg, NULL, TAG_DONE);

        eg = EG[2];
        eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
        eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

        DoPrefsGadget(ed, &eg, NULL, TAG_DONE);

        eg = EG[3];
        eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
        eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

        if (!ed->ed_EmbeddedSample)
            DoPrefsGadget(ed, &eg, NULL, TAG_DONE);

        if (ed->ed_Screen->BitMap.Depth > 2)
        {
            eg = EG[10];
            eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
            eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

            DoPrefsGadget(ed, &eg, NULL, GTCY_Labels, &ed->ed_ColorModeLabels,
                                         GTCY_Active, !ed->ed_4ColorMode,
                                         TAG_DONE);
        }

        ed->ed_Slider1      = NULL;
        ed->ed_Slider2      = NULL;
        ed->ed_Slider3      = NULL;
        ed->ed_PenPalette   = NULL;
        ed->ed_ColorPalette = NULL;
        ed->ed_Pens         = NULL;

        eg = EG[4];
        eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
        eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

        if (MONITOR_PART(GetVPModeID(&ed->ed_Screen->ViewPort)) == A2024_MONITOR_ID)
            eg.eg_TopEdge += 40;

        ed->ed_ColorPalette = DoPrefsGadget(ed, &eg, ed->ed_ColorPalette,
                                            GTPA_Depth,          depth,
                                            GTPA_IndicatorWidth, 40,
                                            GTPA_ColorTable,     ed->ed_ColorTable,
                                            GTPA_Color,          ed->ed_ColorTable[ed->ed_CurrentColor],
                                            TAG_DONE);

        if (ed->ed_DoWheelScreen)
        {
            if (MONITOR_PART(GetVPModeID(&ed->ed_Screen->ViewPort)) != A2024_MONITOR_ID)
            {
                eg = EG[11];
                eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
                eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

                DoPrefsGadget(ed, &eg, NULL, TAG_DONE);
            }
        }
    }

    if (ed->ed_Depth <= 2)
        pens = ed->ed_PrefsWork.pap_4ColorPens;
    else
        pens = ed->ed_PrefsWork.pap_8ColorPens;

    SetColors(ed);

    eg = EG[8];
    if (!ed->ed_EmbeddedSample)
        eg.eg_Height -= 12;

    eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
    eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

    if (ed->ed_Screen->BitMap.Depth <= 2)
    {
        eg.eg_TopEdge -= 18;
        eg.eg_Height  += 22;
    }

    ed->ed_Pens = DoPrefsGadget(ed,&eg,ed->ed_Pens,
                                GTLV_Labels,       &ed->ed_PenList,
                                GTLV_ShowSelected, NULL,
                                GTLV_ScrollWidth,  19,
                                GTLV_ItemHeight,   12,
                                GTLV_CallBack,     &ed->ed_PenCallBack,
                                GTLV_Selected,     ed->ed_CurrentPen,
                                GTLV_MaxPen,       255,
                                TAG_DONE);

    eg = EG[9];
    if (!ed->ed_EmbeddedSample)
        eg.eg_TopEdge -= 12;

    eg.eg_LeftEdge = eg.eg_LeftEdge + ed->ed_LeftBorder - ed->ed_Screen->WBorLeft;
    eg.eg_TopEdge  = eg.eg_TopEdge + ed->ed_TopBorder - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1;

    if (ed->ed_Screen->BitMap.Depth <= 2)
        eg.eg_TopEdge += 6;

    ed->ed_PenPalette = DoPrefsGadget(ed, &eg, ed->ed_PenPalette,
                                      GTPA_Depth,          depth,
                                      GTPA_Color,          ed->ed_ColorTable[pens[NodeMap[ed->ed_PenNodes[ed->ed_CurrentPen].ln_Pri]]],
                                      GTPA_IndicatorWidth, 40,
                                      GTPA_ColorTable,     ed->ed_ColorTable,
                                      GA_Disabled,         ed->ed_CurrentPen < 2,
                                      TAG_DONE);

    DoSliders(ed);
    DoWheel(ed);
    DoBrightness(ed);

    if (window)
        RenderSample(ed,pens,FALSE);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD                 icode;
BOOL                  docolor;
struct Preset        *preset;
UWORD                 i,color;
struct ColorSpec     *cspec;
UWORD                *pens;
UWORD                 zoomSize[4];
struct ColorWheelRGB  rgb;
struct ColorWheelHSB  hsb;
BOOL                  dohsb;
WORD                  penNum;

    icode   = ed->ed_CurrentMsg.Code;
    docolor = FALSE;
    dohsb   = FALSE;
    color   = ed->ed_CurrentColor;
    cspec   = ed->ed_PrefsWork.pap_Colors;

    if (ed->ed_Depth <= 2)
        pens = ed->ed_PrefsWork.pap_4ColorPens;
    else
        pens = ed->ed_PrefsWork.pap_8ColorPens;

    switch (ec)
    {
        case EC_COLORWHEEL   : GetAttr(WHEEL_RGB,ed->ed_ColorWheel,(ULONG *)&rgb);
                               cspec[color].Red   = (rgb.cw_Red   >> 16);
                               cspec[color].Green = (rgb.cw_Green >> 16);
                               cspec[color].Blue  = (rgb.cw_Blue  >> 16);
                               DoBrightness(ed);
                               SetRGB32(&ed->ed_Screen->ViewPort,ed->ed_ColorTable[color],rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue);

                               if (ed->ed_DoWheelScreen)
                                   SetRGB32(&ed->ed_WheelScreen->ViewPort,ed->ed_WheelSamplePen,rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue);
                               else
                                   DoSliders(ed);

                               break;

        case EC_SHOWSAMPLE   : if (ed->ed_SampleWindow)
                               {
                                   WindowToFront(ed->ed_SampleWindow);
                               }
                               else
                               {
                                   zoomSize[0] = -1;
                                   zoomSize[1] = -1;
                                   zoomSize[2] = ZOOMWIDTH;
                                   zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

                                   if (ed->ed_SampleWindow = OpenPrefsWindow(ed,
                                                 WA_InnerWidth,    NW_WIDTH,
                                                 WA_InnerHeight,   SAMPLE_HEIGHT - 2,
                                                 WA_MinWidth,      NW_MINWIDTH,
                                                 WA_MinHeight,     NW_MINHEIGHT,
                                                 WA_MaxWidth,      NW_MAXWIDTH,
                                                 WA_MaxHeight,     NW_MAXHEIGHT,
                                                 WA_Flags,         NW_FLAGS | WFLG_CLOSEGADGET,
                                                 WA_Zoom,          zoomSize,
                                                 WA_AutoAdjust,    TRUE,
                                                 WA_CustomScreen,  ed->ed_Screen,
                                                 WA_Title,         GetString(&ed->ed_LocaleInfo,MSG_PAL_SAMPLE_TITLE),
                                                 WA_NewLookMenus,  TRUE,
                                                 TAG_DONE))
                                   {
                                       ed->ed_SampleWindow->UserPort = ed->ed_Window->UserPort;
                                       ModifyIDCMP(ed->ed_SampleWindow,IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS);

                                       SetFont(ed->ed_SampleWindow->RPort,ed->ed_Font);
                                       RenderSample(ed,pens,TRUE);
                                   }
                                   else
                                   {
                                       DisplayBeep(NULL);
                                   }
                               }
                               break;

        case EC_SHOWWHEEL    : ed->ed_WheelBackup = cspec[color];
                               OpenWheelScreen(ed);
                               break;

        case EC_WHEELOK      : DoSliders(ed);
                               CloseWheelScreen(ed);
                               break;

        case EC_WHEELCANCEL  : cspec[color] = ed->ed_WheelBackup;
                               SetColors(ed);
                               CloseWheelScreen(ed);
                               break;

        case EC_CLOSEGADGET  : ClosePrefsWindow(ed,ed->ed_SampleWindow,TRUE);
                               ed->ed_SampleWindow = NULL;
                               break;

	case EC_COLORMODE    : RemoveGList(window, ed->ed_Gadgets, -1);
                               FreeGadgets(ed->ed_Gadgets);
                               ed->ed_Gadgets   = NULL;
                               ed->ed_LastAdded = NULL;

                               ed->ed_4ColorMode = (icode == 0);
                               ed->ed_Depth      = icode+2;

                               if (ed->ed_4ColorMode)
                               {
                                   if (ed->ed_CurrentColor > 3)
                                       ed->ed_CurrentColor = 3;
                               }

                               RenderGadgets(ed);

                               if (!ed->ed_LastAdded)
                               {
                                   ed->ed_Quit      = TRUE;
                                   ed->ed_Cancelled = TRUE;
                                   return;
                               }

                               AddGList(window, ed->ed_Gadgets, -1, -1, NULL);
                               RefreshGList(ed->ed_Gadgets, window, NULL, -1);
                               GT_RefreshWindow(window, NULL);

                               DoWheel(ed);
                               DoBrightness(ed);
                               DoSliders(ed);
                               break;

        case EC_PENPALETTE   : i = 0;
                               while (icode != ed->ed_ColorTable[i])
                                   i++;
                               pens[NodeMap[ed->ed_CurrentPen]] = i;

                               RenderSample(ed,pens,FALSE);
                               SetGadgetAttr(ed,ed->ed_Pens,
                                             GTLV_Labels,   &ed->ed_PenList,
                                             GTLV_Selected, ed->ed_CurrentPen,
                                             TAG_DONE);

                               /* FALLS THROUGH! */

        case EC_SAMPLECLICK  : if (ec == EC_SAMPLECLICK)
                               {
                                   penNum = WhichPen(ed,ed->ed_CurrentMsg.MouseX,ed->ed_CurrentMsg.MouseY);
                                   if (penNum < 0)
                                       break;

                                   icode = RevNodeMap[penNum];

                                   SetGadgetAttr(ed,ed->ed_Pens,GTLV_Selected,icode,
                                                                TAG_DONE);

                                   ec = EC_PENS;
                               }

                               /* FALLS THROUGH */

        case EC_PENS         : if (ec == EC_PENS)
                               {
                                   ed->ed_CurrentPen = icode;
                                   SetGadgetAttr(ed,ed->ed_PenPalette,
                                                 GTPA_Color,  ed->ed_ColorTable[pens[NodeMap[ed->ed_PenNodes[icode].ln_Pri]]],
                                                 GA_Disabled, ed->ed_CurrentPen < 2,
                                                 TAG_DONE);

                                   icode = ed->ed_ColorTable[pens[NodeMap[ed->ed_PenNodes[icode].ln_Pri]]];
                               }
                               SetGadgetAttr(ed,ed->ed_ColorPalette,
                                             GTPA_Color,  icode,
                                             TAG_DONE);

                               /* FALLS THROUGH! */

        case EC_COLORPALETTE : i = 0;
                               while (icode != ed->ed_ColorTable[i])
                                   i++;

                               ed->ed_CurrentColor = i;
                               DoWheel(ed);
                               DoBrightness(ed);
                               DoSliders(ed);
                               break;

        case EC_SLIDER1      : if (ed->ed_SlidersRGB)
                               {
                                   cspec[color].Red = Make16(icode,ed->ed_RedBits);
                                   docolor = TRUE;
                               }
                               else
                               {
                                   dohsb = TRUE;
                               }
                               break;

        case EC_SLIDER2      : if (ed->ed_SlidersRGB)
                               {
                                   cspec[color].Green = Make16(icode,ed->ed_GreenBits);
                                   docolor = TRUE;
                               }
                               else
                               {
                                   dohsb = TRUE;
                               }
                               break;

        case EC_SLIDER3      : if (ed->ed_SlidersRGB)
                               {
                                   cspec[color].Blue = Make16(icode,ed->ed_BlueBits);
                                   docolor = TRUE;
                               }
                               else
                               {
                                   dohsb = TRUE;
                               }
                               break;

        case EC_PRESET1      :
        case EC_PRESET2      :
        case EC_PRESET3      :
        case EC_PRESET4      :
        case EC_PRESET5      :
        case EC_PRESET6      :
        case EC_PRESET7      :
        case EC_PRESET8      :
        case EC_PRESET9      : preset = &Presets[ec-EC_PRESET1];
                               for (i = 0; i < MAXCOLORS; i++)
                               {
                                   cspec[i].Red   = Make16((preset->Colors[i] / 65536) % 256,8);
                                   cspec[i].Green = Make16((preset->Colors[i] / 256) % 256,8);
                                   cspec[i].Blue  = Make16((preset->Colors[i] % 256),8);
                               }

                               for (i = 0; i < NUM_KNOWNPENS; i++)
                               {
                                   ed->ed_PrefsWork.pap_4ColorPens[i] = DefaultPens[i];
                                   ed->ed_PrefsWork.pap_8ColorPens[i] = DefaultPens[i];
                               }

                               SetColors(ed);
                               RenderSample(ed,pens,FALSE);
                               SetGadgetAttr(ed,ed->ed_PenPalette,
                                             GTPA_Color, ed->ed_ColorTable[pens[NodeMap[ed->ed_PenNodes[ed->ed_CurrentPen].ln_Pri]]],
                                             TAG_DONE);
                               SetGadgetAttr(ed,ed->ed_Pens,
                                             GTLV_Labels,   &ed->ed_PenList,
                                             GTLV_Selected, ed->ed_CurrentPen,
                                             TAG_DONE);
                               DoWheel(ed);
                               DoBrightness(ed);
                               DoSliders(ed);
                               break;

        case EC_RGBMODEL     : ed->ed_SlidersRGB = TRUE;
                               RenderSliderNames(ed);
                               DoSliders(ed);
                               break;

        case EC_HSBMODEL     : ed->ed_SlidersRGB = FALSE;
                               RenderSliderNames(ed);
                               DoSliders(ed);
                               break;
    }

    if (docolor)
    {
        SetRGB32(&ed->ed_Screen->ViewPort,ed->ed_ColorTable[color],
                 Make32(cspec[color].Red),
                 Make32(cspec[color].Green),
                 Make32(cspec[color].Blue));

        DoWheel(ed);
        DoBrightness(ed);
    }

    if (dohsb)
    {
	GetGadgetAttrs(ed,ed->ed_Slider1,GTSL_Level, &hsb.cw_Hue,
	                                 TAG_DONE);
	GetGadgetAttrs(ed,ed->ed_Slider2,GTSL_Level, &hsb.cw_Saturation,
	                                 TAG_DONE);
	GetGadgetAttrs(ed,ed->ed_Slider3,GTSL_Level, &hsb.cw_Brightness,
	                                 TAG_DONE);

        hsb.cw_Hue        = Make32(Make16(hsb.cw_Hue,8));
        hsb.cw_Saturation = Make32(Make16(hsb.cw_Saturation,8));
        hsb.cw_Brightness = Make32(Make16(hsb.cw_Brightness,8));

        SetGadgetAttrsP(ed,ed->ed_ColorWheel,WHEEL_HSB, &hsb,
                                             TAG_DONE);

        ConvertHSBToRGB(&hsb,&rgb);
        cspec[color].Red   = (rgb.cw_Red   >> 16);
        cspec[color].Green = (rgb.cw_Green >> 16);
        cspec[color].Blue  = (rgb.cw_Blue  >> 16);
        SetRGB32(&ed->ed_Screen->ViewPort,ed->ed_ColorTable[color],rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue);
        DoBrightness(ed);
    }
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;

    if (ec == EC_RGBMODEL)
        state->cs_Selected = ed->ed_SlidersRGB;

    if (ec == EC_HSBMODEL)
        state->cs_Selected = !ed->ed_SlidersRGB;
}


/*****************************************************************************/


EdCommand GetCommand(EdDataPtr ed)
{
    if (ed->ed_CurrentMsg.Class == IDCMP_IDCMPUPDATE)
        return(EC_COLORWHEEL);

    if ((ed->ed_CurrentMsg.Class == IDCMP_MOUSEBUTTONS) && (ed->ed_CurrentMsg.Code == SELECTDOWN))
        return(EC_SAMPLECLICK);

    return(EC_NOP);
}
