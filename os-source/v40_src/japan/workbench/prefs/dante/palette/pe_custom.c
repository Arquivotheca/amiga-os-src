
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

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "sample.h"
#include "extent.h"
#include "pe_iff.h"


#define SysBase ed->ed_SysBase


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

VOID SetColors(EdDataPtr ed);


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
    SetGadgetAttrsA(gad,ed->ed_Window,NULL,(struct TagItem *)&tags);
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
        {NM_SUB,MSG_PAL_OPTIONS_RGB,	    EC_RGBMODEL,  CHECKIT|MENUTOGGLE},
        {NM_SUB,MSG_PAL_OPTIONS_HSB,	    EC_HSBMODEL,  CHECKIT|MENUTOGGLE},

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

    {LISTVIEW_KIND, 315,  21,  293, 116,  MSG_NOTHING,          EC_PENS,         0},
    {PALETTE_KIND,  315,  133, 293,  20,  MSG_NOTHING,          EC_PENPALETTE,   0},

    {CYCLE_KIND,    315,  3,   293,  14, MSG_NOTHING,          EC_COLORMODE,    0},
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
ULONG                wbmode;
BOOL                 usewb;
UWORD                scrDepth;
UWORD                numAllocated;
UWORD                maxColors;

    ed->ed_Screen = ed->ed_WBScreen;

    if (editDepth == 1)
    {
        ed->ed_ColorTable[0] = 0;
        ed->ed_ColorTable[1] = 1;
    }
    else
    {
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

        /* see what we could get out of a custom screen in the same mode */
        wbmode = GetVPModeID(&ed->ed_WBScreen->ViewPort);
        GetDisplayInfoData(NULL,(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,wbmode);
        scrDepth = dimInfo.MaxDepth;

        if (!ok || (ed->ed_WBScreen->BitMap.Depth < 5))
        {
            /* There are not enough pens on the WB to allow color editing,
             * or the WB screen has less than 32 colors, which means the
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
                 * colors we might have gotten, so that the color wheel,
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

                if (ed->ed_Screen = OpenPrefsScreen(ed,
                                              SA_LikeWorkbench, TRUE,
                                              SA_Title,         GetString(&ed->ed_LocaleInfo,MSG_PREFS_NAME),
                                              SA_Width,         scrWidth,
                                              SA_Height,        scrHeight,
                                              SA_Depth,         scrDepth,
                                              SA_FullPalette,   TRUE,
                                              SA_Interleaved,   TRUE,
                                              SA_DisplayID,     wbmode,
                                              SA_PubName,       "PALETTE",
                                              SA_PubTask,       FindTask(NULL),
                                              SA_PubSig,        SIGBREAKB_CTRL_E,
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
    }

    return(TRUE);
}


/*****************************************************************************/


BOOL InitDisp(EdDataPtr ed)
{
UWORD height, depth;

    depth = ed->ed_WBScreen->BitMap.Depth;
    if (depth > MAXDEPTH)
        depth = MAXDEPTH;

    ed->ed_Depth = depth;
    ed->ed_4ColorMode = (depth == 2);

    if (depth == 1)
    {
        height = 92;
    }
    else
    {
        height = NW_HEIGHT;
        if (ed->ed_WBScreen->Height - ed->ed_WBScreen->WBorTop - ed->ed_WBScreen->Font->ta_YSize - 1 >= NW_HEIGHT + 95)
        {
            ed->ed_EmbeddedSample = TRUE;
            height += 95;
        }
    }

    if (GetScreen(ed,depth,NW_WIDTH + ed->ed_WBScreen->WBorLeft + ed->ed_WBScreen->WBorRight,
                           height + ed->ed_WBScreen->WBorTop + ed->ed_WBScreen->Font->ta_YSize + 1
                           + ed->ed_WBScreen->BarHeight + 3))
    {
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD              zoomSize[4];
ULONG              id;
UWORD		   depth;
struct DisplayInfo di;
struct EdMenu      stash[sizeof(EM)/sizeof(struct EdMenu)];
UWORD              i;
UWORD              height;
UWORD              width;
UWORD              wsize;
UWORD              left;
struct MenuItem   *item1;
struct MenuItem   *item2;

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_ColorModeLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_PAL_4COLOR_MODE);
    ed->ed_ColorModeLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_PAL_MULTICOLOR_MODE);

    CopyMem(EM,stash,sizeof(stash));

    depth = ed->ed_Depth;

    if (depth == 1)
    {
        height = 92;
        width  = 309;

        /* We don't allow HSB sliders when in monochrome, because we
         * don't have a color wheel to handle conversions for us
         */
        stash[COLORMODEL_ITEM].em_Type = NM_IGNORE;
        stash[COLORMODEL_ITEM+1].em_Type = NM_IGNORE;
        stash[COLORMODEL_ITEM+2].em_Type = NM_IGNORE;
    }
    else
    {
        width  = NW_WIDTH;
        height = NW_HEIGHT;
        if (ed->ed_Screen->Height - ed->ed_Screen->WBorTop - ed->ed_Screen->Font->ta_YSize - 1 >= NW_HEIGHT + 95)
        {
            ed->ed_EmbeddedSample = TRUE;
            height += 95;
        }
    }

    if ((!GetDisplayInfoData(NULL,(APTR)&di,sizeof(struct DisplayInfo),DTAG_DISP,A2024_MONITOR_ID))
    || (di.NotAvailable))
    {
        /* if there's no A2024 installed in this machine, zap the menu item */
        stash[A2024_ITEM-1].em_Type = NM_IGNORE;
        stash[A2024_ITEM].em_Type = NM_IGNORE;
    }

    id = GetVPModeID(&ed->ed_WBScreen->ViewPort);
    if (!GetDisplayInfoData(NULL,(APTR)&di,sizeof(struct DisplayInfo),DTAG_DISP,id))
        return(FALSE);

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

    if (depth > 1)
    {
        /* figure out the width of the color wheel in order to make it as
         * square as possible (square wheel, yeah I know :-)
         */

        wsize = 91 * ed->ed_DrawInfo->dri_Resolution.Y / ed->ed_DrawInfo->dri_Resolution.X;
        if (wsize < 30)
            wsize = 30;
        else if (wsize > 293 - 21)
            wsize = 293 - 21;

        left = (293 - (wsize + 21)) / 2 + 10 + ed->ed_Screen->WBorLeft;

        ed->ed_GradPens[31]         = ~0;
        ed->ed_GradPens[30]         = ObtainBestPenA(ed->ed_Screen->ViewPort.ColorMap,0,0,0,NULL);
        ed->ed_NumGradPensAllocated = 1;
        ed->ed_FirstGradPen         = 30;

        ed->ed_GradientSlider = (struct Gadget *)NewPrefsObject(ed,NULL,"gradientslider.gadget",
                                               GA_Left,        left + wsize + 3,
                                               GA_Top,         28 + ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1,
                                               GA_Width,       18,
                                               GA_Height,      91,
                                               GA_ID,          EC_GRADIENTSLIDER,
                                               GA_RelVerify,   TRUE,
                                               GA_Immediate,   TRUE,
                                               PGA_Freedom,    LORIENT_VERT,
                                               ICA_TARGET,     ICTARGET_IDCMP,
                                               TAG_END);

        ed->ed_ColorWheel = (struct Gadget *)NewPrefsObject(ed,NULL,"colorwheel.gadget",
                                                   GA_Left,        left,
                                                   GA_Top,         28 + ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1,
                                                   GA_Width,       wsize,
                                                   GA_Height,      91,
                                                   GA_ID,          EC_COLORWHEEL,
                                                   GA_Previous,    ed->ed_GradientSlider,
                                                   GA_RelVerify,   TRUE,
                                                   GA_Immediate,   TRUE,
                                                   WHEEL_Screen,   ed->ed_Screen,
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
                ed->ed_GradPens[i] = ObtainPen(ed->ed_Screen->ViewPort.ColorMap,-1,0,0,0,PEN_EXCLUSIVE | PEN_NO_SETCOLOR);
                if (ed->ed_GradPens[i] == -1)
                    break;

                ed->ed_NumGradPensAllocated++;
                ed->ed_FirstGradPen = i;
            }

            SetGadgetAttrsP(ed,ed->ed_GradientSlider,GRAD_PenArray,&ed->ed_GradPens[ed->ed_FirstGradPen],
                                                     TAG_DONE);
        }
    }

    RenderGadgets(ed);

    if ((ed->ed_LastAdded)
    &&  (ed->ed_Menus = CreatePrefsMenus(ed,stash))
    &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,    width,
                                            WA_InnerHeight,   height,
                                            WA_MinWidth,      NW_MINWIDTH,
                                            WA_MinHeight,     height,
                                            WA_MaxWidth,      NW_MAXWIDTH,
                                            WA_MaxHeight,     NW_MAXHEIGHT,
                                            WA_IDCMP,         NW_IDCMP,
                                            WA_Flags,         NW_FLAGS,
                                            WA_Zoom,          zoomSize,
                                            WA_AutoAdjust,    TRUE,
                                            WA_CustomScreen,  ed->ed_Screen,
                                            WA_Title,         GetString(&ed->ed_LocaleInfo,MSG_PAL_NAME),
                                            WA_NewLookMenus,  TRUE,
                                            WA_Gadgets,       ed->ed_Gadgets,
                                            TAG_DONE)))
    {
        if (ed->ed_GradientSlider)
        {
            AddGList(window, ed->ed_GradientSlider, 0, 2, NULL);
            RefreshGList(ed->ed_GradientSlider, window, NULL, 2);
        }

        if (depth > 1)
        {
            item1 = ItemAddress(ed->ed_Menus,FULLMENUNUM(2,1,0));
            item2 = item1->NextItem;

            item1->MutualExclude = 2;
            item2->MutualExclude = 1;

            item1->Flags &= ~CHECKED;
            item2->Flags &= ~CHECKED;
        }

        if (ed->ed_Screen != ed->ed_WBScreen)
        {
            UnlockPubScreen(NULL,ed->ed_WBScreen);
            ed->ed_WBScreen = NULL;
        }

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

    ClosePrefsWindow(ed,ed->ed_SampleWindow,TRUE);
    ClosePrefsWindow(ed,ed->ed_Window,FALSE);

    FreeMenus(ed->ed_Menus);
    FreeGadgets(ed->ed_Gadgets);

    for (i = 0; i < ed->ed_NumColorsAllocated; i++)
        ReleasePen(ed->ed_Screen->ViewPort.ColorMap,ed->ed_ColorTable[i]);

    for (i = ed->ed_FirstGradPen; i < ed->ed_FirstGradPen + ed->ed_NumGradPensAllocated; i++)
        ReleasePen(ed->ed_Screen->ViewPort.ColorMap,ed->ed_GradPens[i]);

    if (ed->ed_ColorWheel)
        DisposeObject(ed->ed_ColorWheel);

    if (ed->ed_GradientSlider)
        DisposeObject(ed->ed_GradientSlider);

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


VOID SetRGB16(EdDataPtr ed, UWORD color, ULONG r, ULONG g, ULONG b)
{
    SetRGB32(&ed->ed_Screen->ViewPort,color,Make32(r),Make32(g),Make32(b));
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
/*
kprintf("        0x00%02lx%02lx%02lx,\n",lrt[color].Red / (256*256*256),
                                 lrt[color].Green / (256*256*256),
                                 lrt[color].Blue / (256*256*256));
*/
        }
        lrt[maxColor].NumColors = 0;

        LoadRGB32(&ed->ed_Screen->ViewPort,(ULONG *)lrt);
    }
}


/*****************************************************************************/


#define HUE_SIDE (0xffff / 6 + 1)

static VOID ConvertHSB2RGB(struct ColorWheelHSB *hsb, struct ColorWheelRGB *rgb)
{
ULONG i, f, p, q, t, v;

    if (hsb->cw_Saturation == 0)
    {
        rgb->cw_Red = rgb->cw_Green = rgb->cw_Blue = hsb->cw_Brightness;
    }
    else
    {
        v = hsb->cw_Brightness;
        i = hsb->cw_Hue / HUE_SIDE;
        f = (hsb->cw_Hue % HUE_SIDE) * 6;
        f += f >> 14;

        p = hsb->cw_Brightness * (0xffff - hsb->cw_Saturation) >> 16;
        q = hsb->cw_Brightness * (0xffff - (hsb->cw_Saturation * f >> 16)) >> 16;
        t = hsb->cw_Brightness * (0xffff - (hsb->cw_Saturation * (0xffff - f) >> 16)) >> 16;

        switch (i)
        {
            case 0: rgb->cw_Red = v; rgb->cw_Green = t; rgb->cw_Blue = p; break;
            case 1: rgb->cw_Red = q; rgb->cw_Green = v; rgb->cw_Blue = p; break;
            case 2: rgb->cw_Red = p; rgb->cw_Green = v; rgb->cw_Blue = t; break;
            case 3: rgb->cw_Red = p; rgb->cw_Green = q; rgb->cw_Blue = v; break;
            case 4: rgb->cw_Red = t; rgb->cw_Green = p; rgb->cw_Blue = v; break;
            case 5: rgb->cw_Red = v; rgb->cw_Green = p; rgb->cw_Blue = q; break;
        }
    }
}


/*****************************************************************************/


VOID DoWheel(EdDataPtr ed)
{
struct ColorSpec *cspec;

    cspec = &ed->ed_PrefsWork.pap_Colors[ed->ed_CurrentColor];

    SetGadgetAttrsP(ed,ed->ed_ColorWheel,WHEEL_Red,   Make32(cspec->Red),
                                         WHEEL_Green, Make32(cspec->Green),
                                         WHEEL_Blue,  Make32(cspec->Blue),
                                         TAG_DONE);
}


/*****************************************************************************/


VOID DoBrightness(EdDataPtr ed)
{
struct ColorWheelHSB  hsb;
struct ColorWheelRGB  rgb;
UWORD                 i,j;
struct LoadRGBTable   lrt[32];

    GetAttr(WHEEL_HSB,ed->ed_ColorWheel,(ULONG *)&hsb);

    hsb.cw_Hue        = (hsb.cw_Hue        >> 16);
    hsb.cw_Saturation = (hsb.cw_Saturation >> 16);

    /* set all colors except the last, which is always black */
    i = ed->ed_FirstGradPen;
    j = 0;
    while (i < ed->ed_NumGradPensAllocated + ed->ed_FirstGradPen - 1)
    {
        hsb.cw_Brightness = 0xffff * (ed->ed_NumGradPensAllocated - (i - ed->ed_FirstGradPen)) / ed->ed_NumGradPensAllocated;
        ConvertHSB2RGB(&hsb,&rgb);

        lrt[j].NumColors  = 1;
        lrt[j].FirstColor = ed->ed_GradPens[i];
        lrt[j].Red        = Make32(rgb.cw_Red);
        lrt[j].Green      = Make32(rgb.cw_Green);
        lrt[j].Blue       = Make32(rgb.cw_Blue);
        i++;
        j++;
    }

    lrt[j].NumColors = 0;

    LoadRGB32(&ed->ed_Screen->ViewPort,(ULONG *)lrt);
}


/*****************************************************************************/


VOID DoSliders(EdDataPtr ed)
{
struct ColorSpec     *cspec;
struct EdGadget       eg;
struct ColorWheelHSB  hsb;
ULONG                 level1, level2, level3;
UWORD                 max1, max2, max3;

    if (ed->ed_SlidersRGB)
    {
        cspec  = &ed->ed_PrefsWork.pap_Colors[ed->ed_CurrentColor];
        level1 = cspec->Red >> (HIGHBITSPERGUN-ed->ed_RedBits);
        max1   = (1<<ed->ed_RedBits)-1;
        level2 = cspec->Green >> (HIGHBITSPERGUN-ed->ed_GreenBits);
        max2   = (1<<ed->ed_GreenBits)-1;
        level3 = cspec->Blue >> (HIGHBITSPERGUN-ed->ed_BlueBits);
        max3   = (1<<ed->ed_BlueBits)-1;
    }
    else
    {
        GetAttr(WHEEL_HSB,ed->ed_ColorWheel,(ULONG *)&hsb);
        level1 = (hsb.cw_Hue >> 24);
        max1   = 255;
        level2 = (hsb.cw_Saturation >> 24);
        max2   = 255;
        level3 = (hsb.cw_Brightness >> 24);
        max3   = 255;
    }

    eg = EG[5];
    if (ed->ed_Depth == 1)
        eg.eg_TopEdge = 30;

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
    if (ed->ed_Depth == 1)
        eg.eg_TopEdge = 42;

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
    if (ed->ed_Depth == 1)
        eg.eg_TopEdge = 55;

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

    if (ed->ed_Depth == 1)
        y = 38;

    if (!ed->ed_SlidersRGB)
        offset = MSG_PAL_HUE_GAD;

    i = 0;
    while (i < 3)
    {
       str  = GetString(&ed->ed_LocaleInfo,i+offset),
       len  = strlen(str);
       plen = TextLength(rp,str,len);
       Move(rp,110+window->BorderLeft-plen,y+window->BorderTop + 12*i);
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
            RectFill(window->RPort,window->BorderLeft,
                                   window->BorderTop + 186,
                                   window->Width - window->BorderRight - 1,
                                   window->BorderTop + 186);

            SetAPen(window->RPort,ed->ed_DrawInfo->dri_Pens[SHINEPEN]);
            RectFill(window->RPort,window->BorderLeft,
                                   window->BorderTop + 187,
                                   window->Width - window->BorderRight - 1,
                                   window->BorderTop + 187);

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
        if (ed->ed_Depth > 1)
        {
            DrawBB(ed,8,3,299,156,GT_VisualInfo,ed->ed_VisualInfo,
                                  TAG_DONE);
        }

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
        if (depth == 1)
            eg.eg_TopEdge = 75;
        DoPrefsGadget(ed, &eg, NULL, TAG_DONE);

        eg = EG[1];
        if (depth == 1)
        {
            eg.eg_LeftEdge = 111;
            eg.eg_TopEdge = 75;
        }
        DoPrefsGadget(ed, &eg, NULL, TAG_DONE);

        eg = EG[2];
        if (depth == 1)
        {
            eg.eg_LeftEdge = 214;
            eg.eg_TopEdge = 75;
        }
        DoPrefsGadget(ed, &eg, NULL, TAG_DONE);

        if (!ed->ed_EmbeddedSample)
            DoPrefsGadget(ed, &EG[3], NULL, TAG_DONE);

        DoPrefsGadget(ed, &EG[10], NULL, GTCY_Labels, &ed->ed_ColorModeLabels,
                                         GTCY_Active, !ed->ed_4ColorMode,
                                         TAG_DONE);

        ed->ed_Slider1      = NULL;
        ed->ed_Slider2      = NULL;
        ed->ed_Slider3      = NULL;
        ed->ed_PenPalette   = NULL;
        ed->ed_ColorPalette = NULL;
        ed->ed_Pens         = NULL;

        ed->ed_ColorPalette = DoPrefsGadget(ed, &EG[4], ed->ed_ColorPalette,
                                            GTPA_Depth,          depth,
                                            GTPA_IndicatorWidth, 40,
                                            GTPA_ColorTable,     ed->ed_ColorTable,
                                            TAG_DONE);
    }

    if (ed->ed_Depth <= 2)
        pens = ed->ed_PrefsWork.pap_4ColorPens;
    else
        pens = ed->ed_PrefsWork.pap_8ColorPens;

    SetColors(ed);

    eg = EG[8];
    if (!ed->ed_EmbeddedSample)
        eg.eg_Height -= 12;

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
BOOL                  dohsb;

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
	case EC_COLORMODE    : RemoveGList(window, ed->ed_Gadgets, -1);
                               FreeGadgets(ed->ed_Gadgets);
                               ed->ed_Gadgets   = NULL;
                               ed->ed_LastAdded = NULL;

                               ed->ed_4ColorMode = (icode == 0);
                               ed->ed_Depth      = icode+2;
                               RenderGadgets(ed);

                               if (ed->ed_LastAdded)
                               {
                                   AddGList(window, ed->ed_Gadgets, -1, -1, NULL);
                                   RefreshGList(ed->ed_Gadgets, window, NULL, -1);
                                   GT_RefreshWindow(window, NULL);
                               }
                               break;

        case EC_COLORWHEEL   : GetAttr(WHEEL_RGB,ed->ed_ColorWheel,(ULONG *)&rgb);
                               cspec[color].Red   = (rgb.cw_Red   >> 16);
                               cspec[color].Green = (rgb.cw_Green >> 16);
                               cspec[color].Blue  = (rgb.cw_Blue  >> 16);
                               SetColors(ed);
                               DoSliders(ed);
                               DoBrightness(ed);
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
                                                 WA_InnerHeight,   93,
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
                                       ModifyIDCMP(ed->ed_SampleWindow,IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW);

                                       SetFont(ed->ed_SampleWindow->RPort,ed->ed_Font);
                                       RenderSample(ed,pens,TRUE);
                                   }
                                   else
                                   {
                                       DisplayBeep(NULL);
                                   }
                               }
                               break;

        case EC_CLOSEGADGET  : ClosePrefsWindow(ed,ed->ed_SampleWindow,TRUE);
                               ed->ed_SampleWindow = NULL;
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
                                   SetGadgetAttrsP(ed,ed->ed_ColorWheel,
                                                   WHEEL_Hue, Make32(Make16(icode,8)),
                                                   TAG_DONE);
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
                                   SetGadgetAttrsP(ed,ed->ed_ColorWheel,
                                                   WHEEL_Saturation, Make32(Make16(icode,8)),
                                                   TAG_DONE);
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
                                   SetGadgetAttrsP(ed,ed->ed_ColorWheel,
                                                   WHEEL_Brightness, Make32(Make16(icode,8)),
                                                   TAG_DONE);
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
        SetRGB16(ed,ed->ed_ColorTable[color],cspec[color].Red,cspec[color].Green,cspec[color].Blue);
        DoWheel(ed);
        DoBrightness(ed);
    }

    if (dohsb)
    {
        GetAttr(WHEEL_RGB,ed->ed_ColorWheel,(ULONG *)&rgb);
        cspec[color].Red   = (rgb.cw_Red   >> 16);
        cspec[color].Green = (rgb.cw_Green >> 16);
        cspec[color].Blue  = (rgb.cw_Blue  >> 16);
        SetRGB16(ed,ed->ed_ColorTable[color],cspec[color].Red,cspec[color].Green,cspec[color].Blue);
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

    return(EC_NOP);
}
