
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
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

/* prototypes */
#include <clib/macros.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "worldmap.h"
#include "pe_iff.h"


#define SysBase ed->ed_SysBase;

BOOL InitMapData (EdDataPtr ed);
VOID FreeMapData (EdDataPtr ed);

/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 3
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_LCLE,
    ID_PREF, ID_CTRY
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


EdStatus BuildList (EdDataPtr ed, struct List * list, STRPTR directory,
		     STRPTR pattern, WORD strip, STRPTR extra)
{
    UBYTE exAllBuffer[512];
    struct ExAllControl *eac;
    struct ExAllData *ead;
    BPTR lock;
    BOOL more;
    char pat[20];
    BOOL ok;
    BOOL nomem = FALSE;
    UWORD len;
    STRPTR name;
    struct Node *node;
    struct Node *new;

    NewList (list);

    if (extra)
    {
	len = strlen (extra);
	if (new = AllocRemember (&ed->ed_Tracker, sizeof (struct Node) + len + 1, MEMF_PUBLIC | MEMF_CLEAR))
	{
	    new->ln_Name = (STRPTR) ((ULONG) new + sizeof (struct Node));
	    CopyMem (extra, new->ln_Name, len);
	    AddHead (list, (struct Node *) new);
	    new->ln_Type = new->ln_Name[0];
	    new->ln_Name[0] = ToUpper (new->ln_Name[0]);
	}
    }

    ok = FALSE;
    if (eac = (struct ExAllControl *) AllocDosObject (DOS_EXALLCONTROL, 0))
    {
	ParsePatternNoCase (pattern, pat, 20);
	eac->eac_LastKey = 0;
	eac->eac_MatchString = pat;

	if (lock = Lock (directory, ACCESS_READ))
	{
	    ok = TRUE;
	    do
	    {
		more = ExAll (lock, (struct ExAllData *) exAllBuffer, sizeof (exAllBuffer), ED_TYPE, eac);
		if ((!more) && (IoErr () != ERROR_NO_MORE_ENTRIES))
		{
		    ok = FALSE;
		    break;
		}

		if (eac->eac_Entries > 0)
		{
		    ead = (struct ExAllData *) exAllBuffer;
		    do
		    {
			if (ead->ed_Type < 0)
			{
			    name = (STRPTR) ead->ed_Name;
			    len = strlen (name) - strip;

			    if (new = AllocRemember (&ed->ed_Tracker, sizeof (struct Node) + len + 1, MEMF_PUBLIC | MEMF_CLEAR))
			    {
				new->ln_Name = (STRPTR) ((ULONG) new + sizeof (struct Node));
				CopyMem (name, new->ln_Name, len);
				new->ln_Type = new->ln_Name[0];
				new->ln_Name[0] = ToUpper (new->ln_Name[0]);

				node = list->lh_Head;
				while (node->ln_Succ)
				{
				    if (Stricmp (node->ln_Name, new->ln_Name) >= 0)
					break;
				    node = node->ln_Succ;
				}
				Insert (list, new, node->ln_Pred);
			    }
			    else
			    {
				nomem = TRUE;
				break;
			    }
			}
			ead = ead->ed_Next;
		    }
		    while (ead);
		}
	    }
	    while (more);

	    UnLock (lock);
	}
	FreeDosObject (DOS_EXALLCONTROL, eac);
    }

    if (nomem)
    {
	SetIoErr (ERROR_NO_FREE_STORE);
	ok = FALSE;
    }

    if (ok)
	return (ES_NORMAL);

    return (ES_DOSERROR);
}


/*****************************************************************************/

EdStatus InitEdData (EdDataPtr ed)
{
    UWORD i;

    if (InitMapData (ed))
    {
	for (i = 0; i < 10; i++)
	{
	    ed->ed_PrefsDefaults.lp_PreferredLanguages[i][0] = NULL;
	    ed->ed_LangNodes[i].ln_Name = ed->ed_PrefsWork.lp_PreferredLanguages[i];
	}

	ed->ed_PreviousYOffset = -1;
	ed->ed_PreviousZone = -1;

#if 1
        strcpy(ed->ed_PrefsDefaults.lp_CountryName,"united_states"); /* .country"); */
        ed->ed_PrefsDefaults.lp_GMTOffset = 300;
        ed->ed_PrefsDefaults.lp_Flags = 0;

        ed->ed_PrefsDefaults.lp_CountryData.cp_CountryCode = MAKE_ID ('U', 'S', 'A', '\0');
        ed->ed_PrefsDefaults.lp_CountryData.cp_TelephoneCode = 1;
        ed->ed_PrefsDefaults.lp_CountryData.cp_MeasuringSystem = MS_AMERICAN;
        ed->ed_PrefsDefaults.lp_CountryData.cp_CalendarType    = CT_7SUN;

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_DateTimeFormat, "%A %B %e %Y %Q:%M %p");

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_DateFormat, "%A %B %e %Y");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_TimeFormat, "%Q:%M %p");

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_ShortDateTimeFormat, "%m/%d/%y %Q:%M %p");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_ShortDateFormat, "%m/%d/%y");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_ShortTimeFormat, "%Q:%M %p");

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_DecimalPoint, ".");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_GroupSeparator, ",");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_FracGroupSeparator, ",");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_Grouping, "\3\0");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_FracGrouping, "\3\0");

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonDecimalPoint, ".");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonGroupSeparator, ",");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonFracGroupSeparator, ",");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonGrouping, "\3\0");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonFracGrouping, "\3\0");
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonFracDigits = 2;
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonIntFracDigits = 2;

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonCS, "$");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonSmallCS, "");
        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonIntCS, "USD");

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveSign, "");
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveSpaceSep = SS_NOSPACE;
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveSignPos = SP_PREC_ALL;
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveCSPos = CSP_PRECEDES;

        strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeSign, "-");
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeSpaceSep = SS_NOSPACE;
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeSignPos = SP_PREC_ALL;
        ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeCSPos = CSP_PRECEDES;

#else
	ed->ed_PrefsDefaults.lp_CountryName[0] = NULL;
	ed->ed_PrefsDefaults.lp_GMTOffset = 300;
	ed->ed_PrefsDefaults.lp_Flags = 0;

	ed->ed_PrefsDefaults.lp_CountryData.cp_CountryCode = MAKE_ID ('U', 'S', 'A', '\0');
	ed->ed_PrefsDefaults.lp_CountryData.cp_TelephoneCode = 1;
	ed->ed_PrefsDefaults.lp_CountryData.cp_MeasuringSystem = MS_AMERICAN;
	ed->ed_PrefsDefaults.lp_CountryData.cp_CalendarType    = CT_7SUN;

	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_DateTimeFormat, "%A %B %e %Y %I:%M %p");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_DateFormat, "%A %B %e %Y");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_TimeFormat, "%I:%M %p");

	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_ShortDateTimeFormat, "%m/%d/%y %I:%M %p");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_ShortDateFormat, "%m/%d/%y");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_ShortTimeFormat, "%I:%M");

	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_DecimalPoint, ".");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_GroupSeparator, ",");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_FracGroupSeparator, ",");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_Grouping, "\3\0");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_FracGrouping, "\3\0");

	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonDecimalPoint, ".");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonGroupSeparator, ",");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonFracGroupSeparator, ",");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonGrouping, "\3\0");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonFracGrouping, "\3\0");
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonFracDigits = 2;
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonIntFracDigits = 2;

	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonCS, "$");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonSmallCS, "¢");
	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonIntCS, "USD");

	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveSign, "");
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveSpaceSep = SS_NOSPACE;
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveSignPos = SP_PREC_ALL;
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonPositiveCSPos = CSP_PRECEDES;

	strcpy (ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeSign, "-");
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeSpaceSep = SS_NOSPACE;
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeSignPos = SP_PREC_ALL;
	ed->ed_PrefsDefaults.lp_CountryData.cp_MonNegativeCSPos = CSP_PRECEDES;
#endif

	ed->ed_PrefsWork = ed->ed_PrefsDefaults;
	ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

	NewList (&ed->ed_PrefLanguages);

	ed->ed_ZoneYOffset = 35;

	return (ES_NORMAL);
    }

    return (ES_NO_MEMORY);
}


/*****************************************************************************/


VOID CleanUpEdData (EdDataPtr ed)
{
    FreeRemember (&ed->ed_Tracker, TRUE);
    FreeMapData (ed);
}


/*****************************************************************************/


VOID UpdateEdData (EdDataPtr ed)
{
    struct Node *node;
    UBYTE name[32];

    NewList (&ed->ed_PrefLanguages);
    ed->ed_UsedNodes = 0;

    while ((ed->ed_UsedNodes < 10) && ed->ed_PrefsWork.lp_PreferredLanguages[ed->ed_UsedNodes][0])
    {
	ed->ed_LangNodes[ed->ed_UsedNodes].ln_Type = ed->ed_PrefsWork.lp_PreferredLanguages[ed->ed_UsedNodes][0];
	ed->ed_LangNodes[ed->ed_UsedNodes].ln_Name[0] = ToUpper (ed->ed_LangNodes[ed->ed_UsedNodes].ln_Name[0]);
	AddTail (&ed->ed_PrefLanguages, &ed->ed_LangNodes[ed->ed_UsedNodes++]);
    }

    strcpy (name, ed->ed_PrefsWork.lp_CountryName);
    name[0] = ToUpper (name[0]);

    if ((!ed->ed_PrefsWork.lp_CountryName[0]) || (!FindName (&ed->ed_AvailCountries, name)))
    {
	if (node = RemHead (&ed->ed_AvailCountries))
	{
	    strcpy (ed->ed_PrefsWork.lp_CountryName, node->ln_Name);
	    AddHead (&ed->ed_AvailCountries, node);
	}
    }
}


/*****************************************************************************/

VOID SetZone (EdDataPtr ed)
{
    for (ed->ed_Zone = 0; ed->ed_Zone < MAX_ZONES; ed->ed_Zone++)
    {
	if (ed->ed_PrefsWork.lp_GMTOffset == Masks_images[ed->ed_Zone].i_Data)
	{
	    return;
	}
    }
    ed->ed_PrefsWork.lp_GMTOffset = 0;
    ed->ed_Zone = 12;
}

/*****************************************************************************/

EdStatus ReadPrefs (EdDataPtr ed, struct IFFHandle * iff, struct ContextNode * cn)
{
    if (cn->cn_ID != ID_LCLE || cn->cn_Type != ID_PREF)
	return (ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes (iff, ed->ed_PrefsIO, sizeof (struct LocalePrefs)) == sizeof (struct LocalePrefs))
    {
	SetZone (ed);

	return (ES_NORMAL);
    }

    return (ES_IFFERROR);
}


EdStatus OpenPrefs (EdDataPtr ed, STRPTR name)
{

    ed->ed_PrefsIO = &ed->ed_PrefsWork;
    return (ReadIFF (ed, name, IFFPrefChunks, IFFPrefChunkCnt, ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs (EdDataPtr ed, struct IFFHandle * iff, struct ContextNode * cn)
{
    EdStatus retval = ES_IFFERROR;
    WORD i;

    for (i = 0; i < ed->ed_UsedNodes; i++)
	ed->ed_LangNodes[i].ln_Name[0] = ed->ed_LangNodes[i].ln_Type;

    if (!PushChunk (iff, 0, ID_LCLE, sizeof (struct LocalePrefs)))
	if (WriteChunkBytes (iff, &ed->ed_PrefsWork, sizeof (struct LocalePrefs)) == sizeof (struct LocalePrefs))
	    if (!PopChunk (iff))
		retval = ES_NORMAL;

    for (i = 0; i < ed->ed_UsedNodes; i++)
	ed->ed_LangNodes[i].ln_Name[0] = ToUpper (ed->ed_LangNodes[i].ln_Name[0]);

    return (retval);
}


EdStatus SavePrefs (EdDataPtr ed, STRPTR name)
{

    return (WriteIFF (ed, name, &IFFPrefHeader, WritePrefs));
}


/*****************************************************************************/


EdStatus ReadCountry (EdDataPtr ed, struct IFFHandle * iff, struct ContextNode * cn)
{

    if (cn->cn_ID != ID_CTRY || cn->cn_Type != ID_PREF)
	return (ES_IFF_UNKNOWNCHUNK);

    if (ReadChunkBytes (iff, ed->ed_PrefsIO, sizeof (struct CountryPrefs)) == sizeof (struct CountryPrefs))
	return (ES_NORMAL);

    return (ES_IFFERROR);
}


/*****************************************************************************/


#define NW_LEFT      0
#define NW_TOP       0
#define NW_WIDTH     544
#define NW_HEIGHT    176
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | BUTTONIDCMP | LISTVIEWIDCMP | SCROLLERIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] =
{
    {NM_TITLE, MSG_PROJECT_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_PROJECT_OPEN, EC_OPEN, 0},
    {NM_ITEM, MSG_PROJECT_SAVE_AS, EC_SAVEAS, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_PROJECT_QUIT, EC_CANCEL, 0},

    {NM_TITLE, MSG_EDIT_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
    {NM_ITEM, MSG_EDIT_LAST_SAVED, EC_LASTSAVED, 0},
    {NM_ITEM, MSG_EDIT_RESTORE, EC_RESTORE, 0},

    {NM_TITLE, MSG_OPTIONS_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_OPTIONS_SAVE_ICONS, EC_SAVEICONS, CHECKIT | MENUTOGGLE},

    {NM_END, MSG_NOTHING, EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] =
{
    {BUTTON_KIND, 8, 159, 87, 14, MSG_SAVE_GAD, EC_SAVE, 0},
    {BUTTON_KIND, 228, 159, 87, 14, MSG_USE_GAD, EC_USE, 0},
    {BUTTON_KIND, 449, 159, 87, 14, MSG_CANCEL_GAD, EC_CANCEL, 0},

    {LISTVIEW_KIND, 9, 18, 155, 42, MSG_LOC_AVAILLANG_GAD, EC_ADDLANG, 0},
    {LISTVIEW_KIND, 193, 18, 155, 42, MSG_LOC_PREFLANG_GAD, EC_NOP, 0},
    {LISTVIEW_KIND, 9, 73, 155, WM_HEIGHT, MSG_LOC_COUNTRY_GAD, EC_SELECTCTRY, 0},
    {BUTTON_KIND, 364, 26, 172, 14, MSG_LOC_CLEARLANG_GAD, EC_CLEARLANG, 0},

    {SCROLLER_KIND,
     WM_LEFT + WM_WIDTH - WM_SCROLLERWIDTH,
     WM_TOP,
     WM_SCROLLERWIDTH,
     WM_HEIGHT,
     MSG_NOTHING, EC_SYNCBITMAP, 0}
};

BOOL CreateDisplay (EdDataPtr ed)
{
    struct Rectangle rect;
    BOOL scroll = TRUE;
    UWORD zoomSize[4];
    UWORD topborder;
    UWORD dif = 0;
    UWORD height;
    ULONG mode;

    mode = GetVPModeID (&ed->ed_Screen->ViewPort);
    QueryOverscan (mode, &rect, OSCAN_TEXT);
    height = rect.MaxY - rect.MinY + 1;
    if (height > ed->ed_Screen->Height)
	height = ed->ed_Screen->Height;

    topborder = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    BuildList (ed, &ed->ed_AvailCountries, "LOCALE:Countries", "#?.country", 8, NULL);
    BuildList (ed, &ed->ed_AvailLanguages, "LOCALE:Languages", "#?.language", 9, "english");

    zoomSize[0] = zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = topborder;

    ed->ed_ZoneHeight = WM_HEIGHT;
    ed->ed_WM_LEFT = WM_LEFT;
    ed->ed_WM_WIDTH = WM_WIDTH;

    if ((topborder + EG[7].eg_TopEdge + WM_IMAGEHEIGHT + 26) < height)
    {
	dif = WM_IMAGEHEIGHT - EG[7].eg_Height;

	/* Save, Use & Cancel */
	EG[0].eg_TopEdge += dif;
	EG[1].eg_TopEdge += dif;
	EG[2].eg_TopEdge += dif;

	EG[5].eg_Height += dif;
	EG[7].eg_Height += dif + 4;

	ed->ed_ZoneHeight += dif + 4;
	ed->ed_ZoneYOffset = 0;

	scroll = FALSE;
	ed->ed_WM_LEFT += WM_SCROLLERWIDTH + 1;
	ed->ed_WM_WIDTH -= WM_SCROLLERWIDTH + 1;
    }

    /* How big are we */
    ed->ed_WinHeight = NW_HEIGHT + dif;

    ed->ed_LastAdded = CreateContext (&ed->ed_Gadgets);

    DoPrefsGadget (ed, &EG[0], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[1], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[2], NULL, TAG_DONE);

    DoPrefsGadget (ed, &EG[3], NULL, GTLV_Labels, &ed->ed_AvailLanguages,
		   LAYOUTA_SPACING, 1,
		   GTLV_ScrollWidth, 18,
		   TAG_DONE);

    if (scroll)
    {
        EG[7].eg_LeftEdge = ed->ed_WM_LEFT + WM_WIDTH - WM_SCROLLERWIDTH;
        ed->ed_VZoneScroller = DoPrefsGadget (ed, &EG[7], NULL,
                                              PGA_Freedom, LORIENT_VERT,
                                              GA_Immediate, TRUE,
                                              GTSC_Top, ed->ed_ZoneYOffset,
                                              GTSC_Visible, ed->ed_ZoneHeight - 4,
                                              GTSC_Total, WM_IMAGEHEIGHT,
                                              GTSC_Arrows, 9,
                                              TAG_DONE);
    }

    RenderGadgets (ed);

    if ((ed->ed_LastAdded)
	&& (ed->ed_Menus = CreatePrefsMenus (ed, EM))
	&& (ed->ed_Window = OpenPrefsWindow (ed,
#if 0
					     WA_Left, NW_LEFT,
					     WA_Top, NW_TOP,
#endif
					     WA_InnerWidth, NW_WIDTH,
					     WA_InnerHeight, ed->ed_WinHeight,
					     WA_MinWidth, NW_MINWIDTH,
					     WA_MinHeight, NW_MINHEIGHT,
					     WA_MaxWidth, NW_MAXWIDTH,
					     WA_MaxHeight, NW_MAXHEIGHT,
					     WA_IDCMP, NW_IDCMP,
					     WA_Flags, NW_FLAGS,
					     WA_Zoom, zoomSize,
					     WA_AutoAdjust, TRUE,
					     WA_PubScreen, ed->ed_Screen,
					     WA_Title, GetString (&ed->ed_LocaleInfo, MSG_LOC_NAME),
					     WA_NewLookMenus, TRUE,
					     WA_Gadgets, ed->ed_Gadgets,
					     TAG_DONE)))
    {
	return (TRUE);
    }

    DisposeDisplay (ed);
    return (FALSE);
}


/*****************************************************************************/


VOID DisposeDisplay (EdDataPtr ed)
{

    if (ed->ed_Window)
    {
	ClearMenuStrip (ed->ed_Window);
	CloseWindow (ed->ed_Window);
    }
    FreeMenus (ed->ed_Menus);
    FreeGadgets (ed->ed_Gadgets);
}


/*****************************************************************************/


VOID PrintTimeZone (EdDataPtr ed)
{
    struct RastPort *rp;
    UWORD len, len2;
    UBYTE str2[60];
    STRPTR str3;
    STRPTR str;
    LONG hour;
    LONG min;

    rp = ed->ed_Window->RPort;
    SetAPen (rp, ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    RectFill(rp, ed->ed_WM_LEFT + ed->ed_Window->BorderLeft - 8,
		 WM_TOP + ed->ed_Window->BorderTop - 12,
		 ed->ed_WM_LEFT + ed->ed_Window->BorderLeft + ed->ed_WM_WIDTH + 8 - 1,
		 WM_TOP + ed->ed_Window->BorderTop - 2);

    SetAPen (rp, ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
    SetBPen (rp, ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    str = GetString (&ed->ed_LocaleInfo, MSG_LOC_TIMEZONE_BOX);
    len = strlen (str);

    hour = (LONG)ed->ed_PrefsWork.lp_GMTOffset / 60L;
    min  = (LONG) ABS (ed->ed_PrefsWork.lp_GMTOffset - (hour * 60L));
    hour = ABS (hour);
    if (min)
    {
	str3 = GetString (&ed->ed_LocaleInfo, MSG_LOC_HR_MIN);
	sprintf (str2, str3, hour, min);
    }
    else if (hour)
    {
	str3 = GetString (&ed->ed_LocaleInfo, MSG_LOC_HR);
	sprintf (str2, str3, hour);
    }
    else
    {
	strcpy (str2, GetString (&ed->ed_LocaleInfo, MSG_LOC_GMT));
    }
    len2 = strlen (str2);

    Move (rp, (ed->ed_WM_WIDTH - TextLength (rp, str, len) - TextLength (rp, str2, len2)) / 2 + ed->ed_WM_LEFT + ed->ed_Window->BorderLeft,
	  WM_TOP + ed->ed_Window->BorderTop - 6);
    Text (rp, str, len);

    SetAPen (rp, ed->ed_DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]);
    Text (rp, str2, len2);
}


/*****************************************************************************/


VOID DrawBB (EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags,...)
{
    DrawBevelBoxA (ed->ed_Window->RPort, x + ed->ed_Window->BorderLeft,
		   y + ed->ed_Window->BorderTop,
		   w, h, (struct TagItem *) & tags);
}


/*****************************************************************************/

VOID CopyPrefs (EdDataPtr ed, struct LocalePrefs *p1, struct LocalePrefs *p2)
{
    *p1 = *p2;
    SetZone (ed);
}

/*****************************************************************************/


VOID RenderDisplay (EdDataPtr ed)
{
    ed->ed_PreviousZone = -1;

    PrintTimeZone (ed);
    DrawBB (ed, ed->ed_WM_LEFT, WM_TOP,
	    WM_WIDTH - WM_SCROLLERWIDTH, ed->ed_ZoneHeight,
	    GT_VisualInfo, ed->ed_VisualInfo,
	    TAG_DONE);

    PutMap (ed);
}


/*****************************************************************************/


VOID RenderGadgets (EdDataPtr ed)
{
    struct Node *node;
    UWORD i;

    if (ed->ed_LangList)
	ed->ed_LangList = DoPrefsGadget (ed, &EG[4], ed->ed_LangList,
					 GTLV_Labels, ~0,
					 TAG_DONE);

    UpdateEdData (ed);

    ed->ed_LangList = DoPrefsGadget (ed, &EG[4], ed->ed_LangList,
				     GTLV_Labels, &ed->ed_PrefLanguages,
				     GTLV_Top, ed->ed_UsedNodes,
				     GTLV_ReadOnly, TRUE,
				     LAYOUTA_SPACING, 1,
				     GTLV_ScrollWidth, 18,
				     TAG_DONE);

    ed->ed_ClearLang = DoPrefsGadget (ed, &EG[6], ed->ed_ClearLang,
				      GA_Disabled, ed->ed_UsedNodes == 0,
				      TAG_DONE);

    i = 0;
    node = ed->ed_AvailCountries.lh_Head;
    while (node->ln_Succ)
    {
	if (Stricmp (ed->ed_PrefsWork.lp_CountryName, node->ln_Name) == 0)
	    break;

	i++;
	node = node->ln_Succ;
    }

    if (!node->ln_Succ)
	i = 0;

    ed->ed_CountryList = DoPrefsGadget (ed, &EG[5], ed->ed_CountryList,
					GTLV_Labels, &ed->ed_AvailCountries,
					GTLV_Selected, i,
					GTLV_MakeVisible, i,
					GTLV_ShowSelected, NULL,
					LAYOUTA_SPACING, 1,
					GTLV_ScrollWidth, 18,
					TAG_DONE);

    if (ed->ed_Window)
    {
	PrintTimeZone (ed);
	PutMap (ed);
    }
}


/*****************************************************************************/


/*
#undef DOSBase
#define DOSBase DOSBase
struct Library * far DOSBase;


VOID MyPrintf(STRPTR format, STRPTR args, ...)
{
    VPrintf(format,&args);
}


VOID __asm PutCh(register __d0 char ch)
{
    if (ch)
        MyPrintf("%lc",(STRPTR)ch);
}


VOID MakeNumber(STRPTR result, STRPTR decPoint, STRPTR thouSep, STRPTR fracSep,
                UBYTE *thouGroup, UBYTE *fracGroup, UBYTE maxFrac)
{
char string[60],string2[60];
WORD i,j,k,cnt,split,oldSplit;

    strcpy(string,"1234567890");

    i        = 0;
    j        = strlen(string)-1;
    oldSplit = 255;
    cnt      = 58;

    while (j >= 0)
    {
        split = thouGroup[i++];
        if (split == 0)
            split = oldSplit;
        oldSplit = split;

        while ((split > 0) && (j >= 0))
        {
            string2[cnt--] = string[j--];
            split--;
        }

        if (j >= 0)
        {
            k = strlen(thouSep);
            while (k > 0)
                string2[cnt--] = thouSep[--k];
        }
    }

    string2[59] = 0;
    strcpy(result,&string2[cnt+1]);

    if (maxFrac)
      strcat(result,decPoint);

    strcpy(string,"987654321");

    i        = 0;
    j        = 0;
    oldSplit = 255;
    cnt      = 0;

    if (maxFrac > strlen(string))
        maxFrac = strlen(string);

    while (j < maxFrac)
    {
        split = fracGroup[i++];
        if (split == 0)
            split = oldSplit;
        oldSplit = split;

        while ((split > 0) && (j < maxFrac))
        {
            string2[cnt++] = string[j++];
            split--;
        }

        if (j < maxFrac)
        {
            k = strlen(fracSep);
            while (k > 0)
                string2[cnt++] = fracSep[--k];
        }
    }

    string2[cnt] = 0;
    strcat(result,string2);
}


VOID DumpCountries(EdDataPtr ed)
{
STRPTR           ms;
char             str[4],string[100];
struct DateStamp date;
struct Locale   *locale;
struct Hook      hook;
struct Node     *node;
char             name[100];

    DateStamp(&date);
    locale = OpenLocale(NULL);
    hook.h_Entry = PutCh;

    MyPrintf("******************\n****************\nLANGUAGE = %s\n\n",locale->loc_LanguageName);

    node = ed->ed_AvailCountries.lh_Head;
    while (node->ln_Succ)
    {
        strcpy(name,"LOCALE:Countries/");
        strcat(name,node->ln_Name);
        strcat(name,".country");

        ed->ed_PrefsIO = &ed->ed_PrefsWork.lp_CountryData;
        ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadCountry);
        strcpy(ed->ed_PrefsWork.lp_CountryName,node->ln_Name);

        switch (ed->ed_PrefsWork.lp_CountryData.cp_MeasuringSystem)
        {
            case MS_ISO      : ms = "Metric";
                               break;
            case MS_AMERICAN : ms = "American";
                               break;
            case MS_IMPERIAL : ms = "Imperial";
                               break;
            case MS_BRITISH  : ms = "British";
                               break;
        }

        str[3] = ed->ed_PrefsWork.lp_CountryData.cp_CountryCode & 0xff;
        str[2] = (ed->ed_PrefsWork.lp_CountryData.cp_CountryCode >> 8) & 0xff;
        str[1] = (ed->ed_PrefsWork.lp_CountryData.cp_CountryCode >> 16) & 0xff;
        str[0] = (ed->ed_PrefsWork.lp_CountryData.cp_CountryCode >> 24) & 0xff;

        MyPrintf("\n\nCountry Name                 : %s \n", ed->ed_PrefsWork.lp_CountryName,"");
        MyPrintf("Country Code                 : %.4s\n", str,"");
        MyPrintf("Telephone Code               : %lu\n", (STRPTR)ed->ed_PrefsWork.lp_CountryData.cp_TelephoneCode,"");
        MyPrintf("Measuring System             : %s \n\n", ms,"");

        MyPrintf("Combined Date and Time       : ","","");
        FormatDate(locale,ed->ed_PrefsWork.lp_CountryData.cp_DateTimeFormat,&date,&hook);
        MyPrintf("\n","","");

        MyPrintf("Date                         : ","","");
        FormatDate(locale,ed->ed_PrefsWork.lp_CountryData.cp_DateFormat,&date,&hook);
        MyPrintf("\n","","");

        MyPrintf("Time                         : ","","");
        FormatDate(locale,ed->ed_PrefsWork.lp_CountryData.cp_TimeFormat,&date,&hook);
        MyPrintf("\n\n","","");

        MyPrintf("Short Combined Date and Time : ","","");
        FormatDate(locale,ed->ed_PrefsWork.lp_CountryData.cp_ShortDateTimeFormat,&date,&hook);
        MyPrintf("\n","","");

        MyPrintf("Short Date                   : ","","");
        FormatDate(locale,ed->ed_PrefsWork.lp_CountryData.cp_ShortDateFormat,&date,&hook);
        MyPrintf("\n","","");

        MyPrintf("Short Time                   : ","","");
        FormatDate(locale,ed->ed_PrefsWork.lp_CountryData.cp_ShortTimeFormat,&date,&hook);
        MyPrintf("\n\n","","");

        MyPrintf("Number format                : ","","");
        MakeNumber(string,ed->ed_PrefsWork.lp_CountryData.cp_DecimalPoint,
                          ed->ed_PrefsWork.lp_CountryData.cp_GroupSeparator,
                          ed->ed_PrefsWork.lp_CountryData.cp_FracGroupSeparator,
                          ed->ed_PrefsWork.lp_CountryData.cp_Grouping,
                          ed->ed_PrefsWork.lp_CountryData.cp_FracGrouping,
                          255);
        MyPrintf("%s\n",string,"");

        MyPrintf("Positive Monetary format     : ","","");
        MakeNumber(string,ed->ed_PrefsWork.lp_CountryData.cp_MonDecimalPoint,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonGroupSeparator,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonFracGroupSeparator,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonGrouping,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonFracGrouping,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonFracDigits);


        if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveCSPos == CSP_PRECEDES)
        {
            switch (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSignPos)
            {
                case SP_PARENS   : MyPrintf("(","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string,"");
                                   MyPrintf(")","");
                                   break;

                case SP_PREC_ALL : MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string,"");
                                   break;

                case SP_SUCC_ALL : MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   break;

                case SP_PREC_CURR: MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string,"");
                                   break;

                case SP_SUCC_CURR: MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string,"");
                                   break;

            }
        }
        else
        {
            switch (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSignPos)
            {
                case SP_PARENS   : MyPrintf("(","");
                                   MyPrintf("%s",string,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(")","");
                                   break;

                case SP_PREC_ALL : MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   MyPrintf("%s",string,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   break;

                case SP_SUCC_ALL : MyPrintf("%s",string,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   break;

                case SP_PREC_CURR: MyPrintf("%s",string,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   break;

                case SP_SUCC_CURR: MyPrintf("%s",string,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonPositiveSign,"");
                                   break;

            }
        }
        MyPrintf("\n","");


        MyPrintf("Negative Monetary format     : ","");
        MakeNumber(string,ed->ed_PrefsWork.lp_CountryData.cp_MonDecimalPoint,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonGroupSeparator,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonFracGroupSeparator,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonGrouping,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonFracGrouping,
                          ed->ed_PrefsWork.lp_CountryData.cp_MonFracDigits);


        if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeCSPos == CSP_PRECEDES)
        {
            switch (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSignPos)
            {
                case SP_PARENS   : MyPrintf("(","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string);
                                   MyPrintf(")","");
                                   break;

                case SP_PREC_ALL : MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string);
                                   break;

                case SP_SUCC_ALL : MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string);
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   break;

                case SP_PREC_CURR: MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string);
                                   break;

                case SP_SUCC_CURR: MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf("%s",string);
                                   break;

            }
        }
        else
        {
            switch (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSignPos)
            {
                case SP_PARENS   : MyPrintf("(","");
                                   MyPrintf("%s",string);
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(")","");
                                   break;

                case SP_PREC_ALL : MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   MyPrintf("%s",string);
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   break;

                case SP_SUCC_ALL : MyPrintf("%s",string);
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   break;

                case SP_PREC_CURR: MyPrintf("%s",string);
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   break;

                case SP_SUCC_CURR: MyPrintf("%s",string);
                                   if (ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSpaceSep == SS_SPACE)
                                       MyPrintf(" ","");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonCS,"");
                                   MyPrintf(ed->ed_PrefsWork.lp_CountryData.cp_MonNegativeSign,"");
                                   break;

            }
        }

        MyPrintf("\n\n","");
        MyPrintf("Small Currency Symbol        : %s\n",ed->ed_PrefsWork.lp_CountryData.cp_MonSmallCS);
        MyPrintf("International Monetary Symbol: %s\n",ed->ed_PrefsWork.lp_CountryData.cp_MonIntCS);

        node = node->ln_Succ;
    }
    CloseLocale(locale);
}
*/


/*****************************************************************************/


VOID ProcessSpecialCommand (EdDataPtr ed, EdCommand ec)
{
    struct Node *node;
    struct Node *langNode;
    UWORD i;
    EdStatus error;
    char name[60];

/*
    DOSBase = ed->ed_DOSBase;
*/

    switch (ec)
    {
	case EC_SYNCBITMAP:
	    PutMap (ed);
	    break;

	case EC_CLEARLANG:
	    SetGadgetAttr (ed, ed->ed_LangList, GTLV_Labels, NULL,
			   TAG_DONE);
	    NewList (&ed->ed_PrefLanguages);
	    ed->ed_UsedNodes = 0;
	    for (i = 0; i < 10; i++)
		ed->ed_PrefsWork.lp_PreferredLanguages[i][0] = NULL;

	    SetGadgetAttr (ed, ed->ed_ClearLang, GA_Disabled, TRUE,
			   TAG_DONE);
	    break;

	case EC_ADDLANG:
	    i = ed->ed_CurrentMsg.Code;
	    langNode = ed->ed_AvailLanguages.lh_Head;
	    while (i--)
	    {
		langNode = langNode->ln_Succ;
	    }

	    node = ed->ed_PrefLanguages.lh_Head;
	    while (node->ln_Succ)
	    {
		if (Stricmp (node->ln_Name, langNode->ln_Name) == 0)
		{
		    break;
		}
		node = node->ln_Succ;
	    }

	    if ((!node->ln_Succ) && (ed->ed_UsedNodes < 10))
	    {
		SetGadgetAttr (ed, ed->ed_LangList, GTLV_Labels, ~0,
			       TAG_DONE);

		strcpy (ed->ed_PrefsWork.lp_PreferredLanguages[ed->ed_UsedNodes], langNode->ln_Name);
		node = &ed->ed_LangNodes[ed->ed_UsedNodes];
		node->ln_Type = langNode->ln_Type;
		AddTail (&ed->ed_PrefLanguages, node);

		SetGadgetAttr (ed, ed->ed_LangList, GTLV_Labels, &ed->ed_PrefLanguages,
			       GTLV_Top, ed->ed_UsedNodes,
			       TAG_DONE);
		SetGadgetAttr (ed, ed->ed_ClearLang, GA_Disabled, FALSE,
			       TAG_DONE);
		ed->ed_UsedNodes++;
	    }
	    else
	    {
		DisplayBeep (NULL);
	    }
	    break;

	case EC_SELECTCTRY:	/* DumpCountries(ed); */
	    i = ed->ed_CurrentMsg.Code;
	    node = ed->ed_AvailCountries.lh_Head;
	    while (i--)
	    {
		node = node->ln_Succ;
	    }
	    strcpy (name, "LOCALE:Countries/");
	    strcat (name, node->ln_Name);
	    name[17] = node->ln_Type;
	    strcat (name, ".country");

	    ed->ed_PrefsIO = &ed->ed_PrefsWork.lp_CountryData;
	    if ((error = ReadIFF (ed, name, IFFPrefChunks, IFFPrefChunkCnt, ReadCountry)) != ES_NORMAL)
	    {
		ShowError2 (ed, error);
	    }

	    strcpy (ed->ed_PrefsWork.lp_CountryName, node->ln_Name);
	    ed->ed_PrefsWork.lp_CountryName[0] = node->ln_Type;
	    break;

	default:
	    break;
    }
}


/*****************************************************************************/


EdCommand GetCommand (EdDataPtr ed)
{
    struct IntuiMessage *intuiMsg;
    struct Window *intuiWindow;
    UWORD oldzone = ed->ed_Zone;

    intuiMsg = &ed->ed_CurrentMsg;
    intuiWindow = intuiMsg->IDCMPWindow;

    switch (intuiMsg->Class)
    {
	case MOUSEBUTTONS:
	    if (intuiMsg->Code == SELECTDOWN)
	    {
		if ((intuiMsg->MouseX - intuiWindow->BorderLeft >= ed->ed_WM_LEFT)
		    && (intuiMsg->MouseX - intuiWindow->BorderLeft <= ed->ed_WM_LEFT + WM_WIDTH - WM_SCROLLERWIDTH - 4)
		    && (intuiMsg->MouseY - intuiWindow->BorderTop >= WM_TOP)
		    && (intuiMsg->MouseY - intuiWindow->BorderTop <= WM_TOP + ed->ed_ZoneHeight - 1))
		{
		    PickTimeZone (ed, intuiMsg->MouseX, intuiMsg->MouseY);

		    if (ed->ed_Zone != oldzone)
		    {
			PrintTimeZone (ed);
			PutMap (ed);
		    }
		}
	    }
	    break;

	default:
	    break;
    }

    return (EC_NOP);
}


/*****************************************************************************/


VOID GetSpecialCmdState (EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{

    state->cs_Available = TRUE;
    state->cs_Selected = FALSE;
}
