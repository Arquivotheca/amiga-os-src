/* pe_custom.c
 * WBPattern Preference Editor
 *
 */

#include "wbpattern.h"

extern struct BitMap Presets;

UWORD ghost_pattern[] = {0x4444, 0x1111, 0x4444, 0x1111};

/*****************************************************************************/

#pragma libcall WorkbenchBase WBConfig 54 1002
void *WBConfig (ULONG, void *);

struct PatternBitMap
{
    struct BitMap *pbm_BitMap;
    UWORD pbm_Width;
    UWORD pbm_Height;
};

/*****************************************************************************/

#define	PWIDTH		16
#define	PHEIGHT		16
#define	PDEPTH		2

#define	XSCALE		6
#define	YSCALE		6

#define NW_LEFT		0
#define NW_TOP		0
#define NW_WIDTH	634
#define NW_HEIGHT	126
#define	BUTTON_WIDTH	87

#define NW_IDCMP	(IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | BUTTONIDCMP | PALETTEIDCMP | SLIDERIDCMP)
#define BNW_FLAGS	(WFLG_ACTIVATE | SIMPLE_REFRESH)
#define NW_FLAGS	(WFLG_ACTIVATE | WINDOWDEPTH | WINDOWDRAG | SIMPLE_REFRESH)
#define NW_MINWIDTH	NW_WIDTH
#define NW_MINHEIGHT	NW_HEIGHT
#define NW_MAXWIDTH	NW_WIDTH
#define NW_MAXHEIGHT	NW_HEIGHT
#define ZOOMWIDTH	200

/* Sketchpad left edge */
#define	SKETCH_LEFT	432	/* was 327 */

/* Tool left edge */
#define	TOOL_LEFT	534	/* was 429 */

/* Preset left edge */
#define	PRESET_LEFT	340	/* was 527 */

/* Preset top edge */
#define	PRESET_TOP	67

/* Repeat top edge */
#define	REPEAT_TOP	4

#define	XREPEATS	4
#define	YREPEATS	3

#define	DX		4
#define	DY		2
#define	DW		8
#define	DH		4

/*****************************************************************************/

VOID
ProcessArgs (EdDataPtr ed, struct DiskObject * diskObj)
{
    UBYTE *tmp;
    ULONG hold;

    if (diskObj)
    {
	if (tmp = FindToolType (diskObj->do_ToolTypes, "CLIPUNIT"))
	    if (StrToLong (tmp, &hold) > 0L)
		ed->ed_ClipUnit = hold;

	ed->ed_RemapImage = TRUE;
	if (FindToolType (diskObj->do_ToolTypes, "NOREMAP"))
	    ed->ed_RemapImage = FALSE;
    }
    else
    {
	if (ed->ed_Args[OPT_CLIPUNIT])
	    ed->ed_ClipUnit = *((ULONG *) ed->ed_Args[OPT_CLIPUNIT]);

	ed->ed_RemapImage = (ed->ed_Args[OPT_NOREMAP]) ? FALSE : TRUE;
    }
    ed->ed_PubScreenName = "Workbench";
}

/*****************************************************************************/

/* Need to copy the type (pattern or picture) and the data
(pattern data or picture name). */
VOID CopyPrefs (EdDataPtr ed, struct ExtPrefs * dp, struct ExtPrefs * sp)
{
    UWORD i;

    /* Make a backup if we are changing the work buffer */
    if (dp == &ed->ed_PrefsWork)
	Store (ed);

    bltbm (&(sp->ep_BMap), 0, 0,
	   &(dp->ep_BMap), 0, 0, (ed->ed_Width * 3), ed->ed_Height,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);

    /* Copy the type (picture or pattern) */
    dp->ep_WBP[0].wbp_Flags = sp->ep_WBP[0].wbp_Flags;
    dp->ep_WBP[1].wbp_Flags = sp->ep_WBP[1].wbp_Flags;
    dp->ep_WBP[2].wbp_Flags = sp->ep_WBP[2].wbp_Flags;

    /* Copy the names also */
    for (i = 0; i < 3; i++)
	strcpy (dp->ep_Path[i], sp->ep_Path[i]);

    /* Set the current type */
    ed->ed_Type = 0x1 & (ed->ed_PrefsWork.ep_WBP[(ed->ed_Which / ed->ed_Width)].wbp_Flags);
}

/*****************************************************************************/

BOOL InitDisp (EdDataPtr ed)
{

    return (TRUE);
}

/*****************************************************************************/

EdStatus InitEdData (EdDataPtr ed)
{
    struct Screen *scr;

    ed->ed_FilterHook.h_Entry = Filter;
    ed->ed_FilterHook.h_Data = ed;

    /* Initialize the sketchpad gadget */
    if (ed->ed_SketchClass = initsketchGClass (ed))
    {
	/* Default to the first pen in our palette */
	ed->ed_APen = 1;

	/* Set the maximum width, height & depth */
	ed->ed_Width = 16;
	ed->ed_Height = 16;
	ed->ed_Depth = 3;
	if (scr = LockPubScreen (NULL))
	{
	    ed->ed_Depth = scr->BitMap.Depth;
	    UnlockPubScreen (NULL, scr);
	}

	/* Set the amount to magnify by */
	ed->ed_XMag = ed->ed_YMag = 6;

	/* Default to pattern */
	ed->ed_PrefsDefaults.ep_WBP[0].wbp_Flags = 0x1;
	ed->ed_PrefsDefaults.ep_WBP[1].wbp_Flags = 0x1;
	ed->ed_PrefsDefaults.ep_WBP[2].wbp_Flags = 0x1;

	/* Create the bitplanes */
	if ((allocpp (ed, &ed->ed_PrefsDefaults)) == ES_NORMAL)
	{
	    if ((allocpp (ed, &ed->ed_PrefsWork)) == ES_NORMAL)
	    {
		if ((allocpp (ed, &ed->ed_PrefsInitial)) == ES_NORMAL)
		{
		    if (allocbitmap (ed, &ed->ed_Undo, ed->ed_Depth, ed->ed_Width, ed->ed_Height))
		    {
			if (allocbitmap (ed, &ed->ed_Temp, ed->ed_Depth, ed->ed_Width, ed->ed_Height))
			{
			    /* Make the default image */
			    CopyFromDefault (ed);

			    return (ES_NORMAL);
			}
			freebitmap (ed, &(ed->ed_Temp), ed->ed_Width, ed->ed_Height);
		    }
		    freebitmap (ed, &(ed->ed_Undo), ed->ed_Width, ed->ed_Height);
		}
		freebitmap (ed, &(ed->ed_PrefsInitial.ep_BMap), (ed->ed_Width * 3) + 2, ed->ed_Height);
	    }
	    freebitmap (ed, &(ed->ed_PrefsWork.ep_BMap), (ed->ed_Width * 3) + 2, ed->ed_Height);
	}
	freebitmap (ed, &(ed->ed_PrefsDefaults.ep_BMap), (ed->ed_Width * 3) + 2, ed->ed_Height);
    }
    return (ES_NO_MEMORY);
}

/*****************************************************************************/

VOID CleanUpEdData (EdDataPtr ed)
{

    /* Did they hit the cancel button? */
    if (ed->ed_Cancelled && (ed->ed_Flags & (EDF_TESTBD | EDF_CHANGED)))
    {
	/* Restore the initial pointer */
	CopyPrefs (ed, &ed->ed_PrefsWork, &ed->ed_PrefsInitial);
	SavePrefs (ed, ENV_NAME);
    }

    /* Free the bitmaps */
    freebitmap (ed, &(ed->ed_PrefsInitial.ep_BMap), (ed->ed_Width * 3) + 2, ed->ed_Height);
    freebitmap (ed, &(ed->ed_PrefsWork.ep_BMap), (ed->ed_Width * 3) + 2, ed->ed_Height);
    freebitmap (ed, &(ed->ed_PrefsDefaults.ep_BMap), (ed->ed_Width * 3) + 2, ed->ed_Height);

    /* Free the bitmaps */
    freebitmap (ed, &(ed->ed_Temp), ed->ed_Width, ed->ed_Height);
    freebitmap (ed, &(ed->ed_Undo), ed->ed_Width, ed->ed_Height);

    /* Get rid of the sketch class */
    if (ed->ed_SketchClass)
    {
	freesketchGClass (ed, ed->ed_SketchClass);
    }
    FreeAslRequest (ed->ed_ImageReq);
    FreeAslRequest (ed->ed_SampleReq);
}

/*****************************************************************************/

struct EdMenu far EM[] =
{
    {NM_TITLE, MSG_PROJECT_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_PROJECT_OPEN, EC_OPEN, 0},
    {NM_ITEM, MSG_PROJECT_SAVE_AS, EC_SAVEAS, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_PROJECT_QUIT, EC_CANCEL, 0},

    {NM_TITLE, MSG_EDIT_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_EDIT_CUT, EC_CUT, 0},
    {NM_ITEM, MSG_EDIT_COPY, EC_COPY, 0},
    {NM_ITEM, MSG_EDIT_PASTE, EC_PASTE, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_EDIT_ERASE, EC_ERASE, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_EDIT_UNDO, EC_UNDO, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_WBP_EDIT_LOAD_IMAGE, EC_LOAD_IMAGE, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_MYRESETALL, 0},
    {NM_ITEM, MSG_EDIT_LAST_SAVED, EC_MYLASTSAVED, 0},
    {NM_ITEM, MSG_EDIT_RESTORE, EC_MYRESTORE, 0},

    {NM_TITLE, MSG_OPTIONS_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_OPTIONS_SAVE_ICONS, EC_SAVEICONS, CHECKIT | MENUTOGGLE},

    {NM_END, MSG_NOTHING, EC_NOP, 0}
};

/*****************************************************************************/

/* main display gadgets */
struct EdGadget far EG[] =
{
    {BUTTON_KIND, 8, 107, BUTTON_WIDTH, 14, MSG_SAVE_GAD, EC_SAVE, 0},
    {BUTTON_KIND, 273, 107, BUTTON_WIDTH, 14, MSG_USE_GAD, EC_USE, 0},
    {BUTTON_KIND, NW_WIDTH-4-BUTTON_WIDTH, 107, BUTTON_WIDTH, 14, MSG_CANCEL_GAD, EC_CANCEL, 0},

    {CYCLE_KIND, 143, 4, 178, 16, MSG_WBP_PLACEMENT_GAD, EC_WHICH, 0},		/* Placement: */

    {BUTTON_KIND, 143, 89, 178, 14, MSG_WBP_TEST_GAD, EC_TEST, 0},		/* Test */

    {BUTTON_KIND, TOOL_LEFT, 89, 96, 14, MSG_WBP_CLEAR_GAD, EC_CLEAR, 0},	/* Clear */
    {BUTTON_KIND, TOOL_LEFT, 73, 96, 14, MSG_WBP_UNDO_GAD, EC_UNDO, 0},		/* Undo */
    {PALETTE_KIND, TOOL_LEFT, 4, 96, 66, NULL, EC_PALETTE, 0},

    {CYCLE_KIND, 143, 23, 178, 16, MSG_WBP_TYPE_GAD, EC_TYPE, 0},		/* Type: */

    {BUTTON_KIND, 143, 46, 178, 14, MSG_WBP_SELECT_PICTURE_GAD, EC_SELECT, 0},	/* Select Picture... */
    {TEXT_KIND, 143, 61, 178, 14, MSG_WBP_PICTURE_NAME_GAD, EC_NOP, 0},		/* Picture Name: */
};

/*****************************************************************************/

BOOL CreateDisplay (EdDataPtr ed)
{
    struct EdGadget eg = {NULL};
    register WORD i, j, k;
    struct LocaleInfo *li;
    struct Gadget *gad;

    if (ed->ed_Screen)
    {
	j = 1 << ed->ed_Screen->BitMap.Depth;
	for (i = 0; i < 4; i++)
	    ed->ed_ColorTable[i] = i;
	for (i = 1, k = 7; i < 5; i++, k--)
	    ed->ed_ColorTable[k] = j - i;
    }

    li = &(ed->ed_LocaleInfo);
    ed->ed_WhichLabels[0] = GetString (li, MSG_WBP_WORKBENCH_GAD);
    ed->ed_WhichLabels[1] = GetString (li, MSG_WBP_WINDOWS_GAD);
    ed->ed_WhichLabels[2] = GetString (li, MSG_WBP_SCREEN_GAD);

    ed->ed_TypeLabels[0] = GetString (li, MSG_WBP_TYPE_PICTURE_GAD);
    ed->ed_TypeLabels[1] = GetString (li, MSG_WBP_TYPE_PATTERN_GAD);

    ed->ed_LastAdded = CreateContext (&ed->ed_Gadgets);

    DoPrefsGadget (ed, &EG[0], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[1], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[2], NULL, TAG_DONE);

    /* Build the editor specific gadgets */
    RenderGadgets (ed);

    /* Add the preset gadgets */
    eg.eg_GadgetKind = GENERIC_KIND;
    eg.eg_LeftEdge = PRESET_LEFT + DX;
    eg.eg_TopEdge = PRESET_TOP + DY;
    eg.eg_Width = PWIDTH;
    eg.eg_Height = PHEIGHT;
    eg.eg_Label = NULL;
    eg.eg_Command = EC_PRESETS;
    eg.eg_GadgetFlags = NULL;

    for (i = 0; i < MAXPRESETS; i++)
    {
	if (ed->ed_GPresets[i] = gad = DoPrefsGadget (ed, &eg, NULL, TAG_DONE))
	{
	    gad->UserData = (APTR) i;
	    gad->Flags = (ed->ed_Type) ? GFLG_GADGHCOMP : GFLG_GADGHCOMP | GFLG_DISABLED;
	    gad->Activation = RELVERIFY;
	    gad->GadgetType |= BOOLGADGET;
	}
	if (i == 3)
	{
	    eg.eg_LeftEdge = PRESET_LEFT + DX;
	    eg.eg_TopEdge += PHEIGHT;
	}
	else
	{
	    eg.eg_LeftEdge += PWIDTH;
	}
    }

    /* Create the menus and open the window */
    if ((ed->ed_LastAdded)
	&& (ed->ed_Menus = CreatePrefsMenus (ed, EM))
	&& (ed->ed_Window = OpenPrefsWindow (ed,
					     WA_IDCMP, NW_IDCMP,
					     WA_Flags, NW_FLAGS,
					     WA_InnerWidth, NW_WIDTH,
					     WA_InnerHeight, NW_HEIGHT,
					     WA_MinWidth, NW_MINWIDTH,
					     WA_MinHeight, NW_MINHEIGHT,
					     WA_MaxWidth, NW_MAXWIDTH,
					     WA_MaxHeight, NW_MAXHEIGHT,
					     WA_Title, GetString (&ed->ed_LocaleInfo, MSG_PREFS_NAME),
					     WA_AutoAdjust, TRUE,
					     WA_PubScreen, ed->ed_Screen,
					     WA_NewLookMenus, TRUE,
					     WA_Gadgets, ed->ed_Gadgets,
					     TAG_DONE)))
    {
	/* Bring the screen to front */
	ScreenToFront (ed->ed_Screen);

	return (TRUE);
    }
    DisposeDisplay (ed);

    return (FALSE);
}

/*****************************************************************************/

VOID
DisposeDisplay (EdDataPtr ed)
{

    /* Do we have a window? */
    if (ed->ed_Window)
    {
	ClearMenuStrip (ed->ed_Window);
	CloseWindow (ed->ed_Window);
    }
    /* Free the menus */
    FreeMenus (ed->ed_Menus);

    /* Free the GadTools gadgets */
    FreeGadgets (ed->ed_Gadgets);

    /* Do we have a sketchpad? */
    if (ed->ed_GSketch)
    {
	DisposeObject (ed->ed_GSketch);
	ed->ed_GSketch = NULL;
    }
}

/*****************************************************************************/

VOID
CenterLine (EdDataPtr ed, struct RastPort * rp, AppStringsID id,
	    UWORD x, UWORD y, UWORD w)
{
    STRPTR str;
    UWORD len;

    str = GetString (&ed->ed_LocaleInfo, id);
    len = strlen (str);

    Move (rp, (w - TextLength (rp, str, len)) / 2 + window->BorderLeft + x,
	  window->BorderTop + y);

    Text (rp, str, len);
}

/*****************************************************************************/

VOID RefreshRepeats (EdDataPtr ed)
{
    register WORD x, y;

    /* Get a pointer to the sketchpad rastport */
    if (ed->ed_Window)
    {
	for (y = REPEAT_TOP + DY + ed->ed_Window->BorderTop;
	     y < REPEAT_TOP + DY + ed->ed_Window->BorderTop + YREPEATS * ed->ed_Height;
	     y += ed->ed_Height)
	{
	    for (x = PRESET_LEFT + DX + ed->ed_Window->BorderLeft;
		 x < PRESET_LEFT + DX + ed->ed_Window->BorderLeft + XREPEATS * ed->ed_Width;
		 x += ed->ed_Width)
	    {
		bltbmrp (&(ed->ed_PrefsWork.ep_BMap), ed->ed_Which, 0,
			 ed->ed_Window->RPort, x, y, ed->ed_Width, ed->ed_Height,
			 0xC0, ed->ed_GfxBase);
	    }
	}
    }
}

/*****************************************************************************/

VOID RenderDisplay (EdDataPtr ed)
{
    struct RastPort *rp;

    rp = ed->ed_Window->RPort;
    SetAPen (rp, ed->ed_DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]);
    SetBPen (rp, ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);

    /* Draw the labels */
    CenterLine (ed, rp, MSG_WBP_PRESETS_LABEL, PRESET_LEFT - 8, PRESET_TOP - 3, 71 + 16);

    /* Draw the border around the sketchpad */
    DrawBB (ed,
	    ed->ed_GSketch->LeftEdge - 1,
	    ed->ed_GSketch->TopEdge,
	    ed->ed_GSketch->Width + 2,
	    ed->ed_GSketch->Height,
	    GT_VisualInfo, ed->ed_VisualInfo,
	    TAG_DONE);

    /* Draw the border around the repeat area */
    DrawBB (ed,
	    PRESET_LEFT + ed->ed_Window->BorderLeft,
	    REPEAT_TOP + ed->ed_Window->BorderTop,
	    XREPEATS * PWIDTH + DW,
	    YREPEATS * PHEIGHT + DH,
	    GT_VisualInfo, ed->ed_VisualInfo,
	    GTBB_Recessed, TRUE,
	    TAG_DONE);

    /* Draw the repeat area */
    RefreshRepeats (ed);

    /* Draw the border around the preset area */
    DrawBB (ed,
	    PRESET_LEFT + ed->ed_Window->BorderLeft,
	    PRESET_TOP + ed->ed_Window->BorderTop,
	    XREPEATS * PWIDTH + DW,
	    2 * PHEIGHT + DH,
	    GT_VisualInfo, ed->ed_VisualInfo,
	    TAG_DONE);

    /* Show the presets */
    WaitBlit ();
    bltbmrp (&Presets, 0, 0,
	     rp,
	     PRESET_LEFT + DX + ed->ed_Window->BorderLeft,
	     PRESET_TOP + DY + ed->ed_Window->BorderTop,
	     (XREPEATS * PWIDTH), (2 * PHEIGHT),
	     0xC0, ed->ed_GfxBase);

    if (ed->ed_Type == 0)
    {
	WaitBlit ();

	SetAfPt (rp, ghost_pattern, 2);
	SetDrMd (rp, JAM1);
	SetAPen (rp, ed->ed_DrawInfo->dri_Pens[TEXTPEN]);

	RectFill (rp,
		  PRESET_LEFT + DX + ed->ed_Window->BorderLeft,
		  REPEAT_TOP + DY + ed->ed_Window->BorderTop,
		  PRESET_LEFT + DX + ed->ed_Window->BorderLeft + (XREPEATS * ed->ed_Height) - 1,
		  REPEAT_TOP + DY + ed->ed_Window->BorderTop + (YREPEATS * ed->ed_Width) - 1);
	RectFill (rp,
		  PRESET_LEFT + DX + ed->ed_Window->BorderLeft,
		  PRESET_TOP + DY + ed->ed_Window->BorderTop,
		  PRESET_LEFT + DX + ed->ed_Window->BorderLeft + (XREPEATS * PWIDTH) - 1,
		  PRESET_TOP + DY + ed->ed_Window->BorderTop + (2 * PHEIGHT) - 1);
	SetAfPt (rp, NULL, 0);
    }
}

/*****************************************************************************/

VOID RenderGadgets (EdDataPtr ed)
{
    UWORD i;

    /* Set the radio button for which image to edit */
    ed->ed_GWhich = DoPrefsGadget (ed, &EG[3], ed->ed_GWhich,
				   GTCY_Labels, ed->ed_WhichLabels,
				   GTCY_Active, ed->ed_Which / ed->ed_Width,
				   TAG_DONE);

    /* Set all the buttons up. */
    ed->ed_GTest = DoPrefsGadget (ed, &EG[4], ed->ed_GTest, TAG_DONE);
    ed->ed_GClear = DoPrefsGadget (ed, &EG[5], ed->ed_GClear,
				   GA_Disabled, ((ed->ed_Type) ? FALSE : TRUE),
				   TAG_DONE);
    ed->ed_GUndo = DoPrefsGadget (ed, &EG[6], ed->ed_GUndo,
				  GA_Disabled, ((ed->ed_Type) ? FALSE : TRUE),
				  TAG_DONE);

    /* Set the palette */
    ed->ed_GPalette = DoPrefsGadget (ed, &EG[7], ed->ed_GPalette,
				     GTPA_ColorTable, ed->ed_ColorTable,
				     GTPA_Depth, MIN (ed->ed_Screen->BitMap.Depth, 3),
				     GTPA_Color, ed->ed_APen,
				     GTPA_IndicatorHeight, 15,
				     GA_Disabled, ((ed->ed_Type) ? FALSE : TRUE),
				     TAG_DONE);

    ed->ed_GType = DoPrefsGadget (ed, &EG[8], ed->ed_GType,
				  GTCY_Labels, ed->ed_TypeLabels,
				  GTCY_Active, ed->ed_Type,
				  TAG_DONE);

    ed->ed_GSelect = DoPrefsGadget (ed, &EG[9], ed->ed_GSelect,
				    GA_Disabled, ((ed->ed_Type) ? TRUE : FALSE),
				    TAG_DONE);

    ed->ed_GName = DoPrefsGadget (ed, &EG[10], ed->ed_GName,
				  GTTX_Border, TRUE,
				  GTTX_Text, (ed->ed_Type) ? "" : FilePart (&ed->ed_PrefsWork.ep_Path[(ed->ed_Which / ed->ed_Width)][0]),
				  GA_Disabled, ((ed->ed_Type) ? TRUE : FALSE),
				  TAG_DONE);

    /* Create the sketchpad gadget */
    if (ed->ed_LastAdded && (ed->ed_GSketch == NULL))
    {
	ed->ed_LastAdded = ed->ed_GSketch = (struct Gadget *)
	  NewPrefsObject (ed,
			  ed->ed_SketchClass, NULL,
			  GA_Left, SKETCH_LEFT + ed->ed_Screen->WBorLeft,
			  GA_Top, 4 + ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1,
			  GA_Width, 2 + (PWIDTH * XSCALE) + 1,
			  GA_Height, 2 + (PHEIGHT * YSCALE) + 1,
			  GA_Immediate, TRUE,
			  GA_FollowMouse, TRUE,
			  GA_RelVerify, TRUE,
			  GA_ID, EC_SKETCH,
			  GA_Previous, ed->ed_LastAdded,
			  TAG_DONE);
    }

    if (ed->ed_GPresets[0])
    {
	USHORT pos;

	for (i = 0; i < MAXPRESETS; i++)
	{
	    pos = RemoveGList (ed->ed_Window, ed->ed_GPresets[i], 1L);
	    if (ed->ed_Type)
		ed->ed_GPresets[i]->Flags &= ~GFLG_DISABLED;
	    else
		ed->ed_GPresets[i]->Flags |= GFLG_DISABLED;
	    AddGList (ed->ed_Window, ed->ed_GPresets[i], (LONG) pos, 1L, NULL);
	}
    }

    /* Set the sketch pad */
    if (ed->ed_GSketch)
    {
	setgadgetattrs (ed, ed->ed_GSketch, GA_Disabled, ((ed->ed_Type) ? FALSE : TRUE), TAG_DONE);

	/* Update the sketchpad */
	UpdateSketch (ed);
    }
}

/*****************************************************************************/

VOID SyncTextGadgets (EdDataPtr ed)
{
}

/*****************************************************************************/

APTR newdtobject (EdDataPtr ed, STRPTR name, Tag tag1,...)
{

    return (NewDTObjectA (name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG getdtattrs (EdDataPtr ed, Object * o, ULONG data,...)
{

    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG setdtattrs (EdDataPtr ed, Object * o, ULONG data,...)
{

    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

EdStatus UndoFunc (EdDataPtr ed)
{

    /* Restore the undo buffer */
    setgadgetattrs (ed, ed->ed_GSketch,
		    SPA_Restore, TRUE,
		    TAG_DONE);

    /* Draw the repeat area */
    RenderGadgets (ed);

    return (ES_NORMAL);
}

/*****************************************************************************/

EdStatus ClearFunc (EdDataPtr ed)
{

    /* Clear the work area */
    setgadgetattrs (ed, ed->ed_GSketch,
		    SPA_Store, TRUE,
		    SPA_Clear, TRUE,
		    TAG_DONE);

    /* Draw the repeat area */
    RenderGadgets (ed);

    return (ES_NORMAL);
}

/*****************************************************************************/

ULONG ASM Filter (REG (a0) struct Hook * h, REG (a2) struct FileRequester * fr, REG (a1) struct AnchorPath * ap)
{
    EdDataPtr ed = (EdDataPtr) h->h_Data;
    struct DataType *dtn;
    ULONG use = FALSE;
    UBYTE buffer[300];
    BPTR lock;

    strncpy (buffer, fr->fr_Drawer, sizeof (buffer));
    AddPart (buffer, ap->ap_Info.fib_FileName, sizeof (buffer));
    if (lock = Lock (buffer, ACCESS_READ))
    {
	    if (ap->ap_Info.fib_DirEntryType < 0)
	    {
		if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
		{
		    if (dtn->dtn_Header->dth_GroupID == GID_PICTURE)
			use = TRUE;
		    ReleaseDataType (dtn);
		}
	    }
	    else
		use = TRUE;
	UnLock (lock);
    }

    return (use);
}

/*****************************************************************************/

BOOL SelectSample (EdDataPtr ed, ULONG tag1,...)
{
    if (!ed->ed_SampleReq)
	ed->ed_SampleReq = AllocAslRequest (ASL_FileRequest, NULL);

    return (AslRequest (ed->ed_SampleReq, (struct TagItem *) & tag1));
}

/*****************************************************************************/

EdStatus TestFunc (EdDataPtr ed)
{
    struct SignalSemaphore *sem;
    EdStatus result;

    DoPrefsGadget (ed, NULL, ed->ed_GTest, GA_Disabled, TRUE, TAG_DONE);
    Forbid ();
    if (sem = FindSemaphore ("« IPrefs »"))
	ObtainSemaphore (sem);
    Permit ();
    ed->ed_Flags |= EDF_TESTBD | EDF_CHANGED;
    result = SavePrefs (ed, ENV_NAME);
    if (sem)
	ReleaseSemaphore (sem);
    DoPrefsGadget (ed, NULL, ed->ed_GTest, GA_Disabled, FALSE, TAG_DONE);
    return result;
}

/*****************************************************************************/

VOID ProcessSpecialCommand (EdDataPtr ed, EdCommand ec)
{
    EdStatus result = ES_NORMAL;
    char path[PATHNAMESIZE];
    struct DataType *dtn;
    BOOL untest = FALSE;
    BOOL update = FALSE;
    struct Hook hook;
    UWORD icode;
    UWORD x, y;
    BPTR lock;

    icode = ed->ed_CurrentMsg.Code;
    switch (ec)
    {
	case EC_PRESETS:
	    /* Backup the current image */
	    Store (ed);

	    /* Clear the destination */
	    SetPrefsRast (ed, 0);

	    /* Which preset was hit? */
	    icode = (UWORD) (((struct Gadget *) ed->ed_CurrentMsg.IAddress)->UserData);

	    /* Figure out the offset of the preset */
	    x = y = 0;
	    if (icode >= XREPEATS)
	    {
		icode -= XREPEATS;
		y += PHEIGHT;
	    }
	    x += (icode * PWIDTH);

	    /* Copy the selected preset into the current work area */
	    bltbm (&Presets, x, y,
		   &(ed->ed_PrefsWork.ep_BMap),
		   ed->ed_Which, 0,
		   ed->ed_Width, ed->ed_Height,
		   0xC0, 0xFF, NULL, ed->ed_GfxBase);

	    /* Update the sliders */
	    RenderGadgets (ed);
	    break;

	case EC_TYPE:
	    /* Update the type */
	    ed->ed_Type = (UWORD) icode;

	    /* Clear the old type */
	    ed->ed_PrefsWork.ep_WBP[(ed->ed_Which / ed->ed_Width)].wbp_Flags &= ~0x1;

	    /* Set the new type */
	    ed->ed_PrefsWork.ep_WBP[(ed->ed_Which / ed->ed_Width)].wbp_Flags |= ed->ed_Type;

	    /* Update the display */
	    update = TRUE;
	    break;

	case EC_WHICH:
	    /* Set the current view */
	    ed->ed_Which = (UWORD) icode *ed->ed_Width;

	    /* Set the current type */
	    ed->ed_Type = 0x1 & (ed->ed_PrefsWork.ep_WBP[(ed->ed_Which / ed->ed_Width)].wbp_Flags);

	    /* Update the display */
	    update = TRUE;
	    break;

	case EC_CUT:
	    result = CutFunc (ed);
	    break;

	case EC_COPY:
	    result = CopyFunc (ed);
	    break;

	case EC_PASTE:
	    result = PasteFunc (ed, EC_PASTE);
	    break;

	case EC_LOAD_IMAGE:
	    result = PasteFunc (ed, EC_LOAD_IMAGE);
	    break;

	case EC_ERASE:
	    result = EraseFunc (ed);
	    break;

	case EC_UNDO:
	    result = UndoFunc (ed);
	    break;

	case EC_TEST:
	    result = TestFunc (ed);
	    break;

	case EC_CLEAR:
	    result = ClearFunc (ed);
	    break;

	case EC_PALETTE:
	    /* Set the currently select color */
	    ed->ed_APen = icode;

	    /* Update the sliders */
	    RenderGadgets (ed);
	    break;

	case EC_SKETCH:
	    if ((ULONG) ed->ed_GSketch->SpecialInfo != 0xFFFFFFFF)
		RefreshRepeats (ed);
	    break;

	case EC_SELECT:
	    strcpy (path, &ed->ed_PrefsWork.ep_Path[(ed->ed_Which / ed->ed_Width)][0]);
	    *PathPart (path) = 0;
	    hook.h_Entry = IntuiHook;

	    if (SelectSample (ed,
			      ASLFR_TitleText,		GetString (&ed->ed_LocaleInfo, MSG_WBP_SELECT_PICTURE_LABEL),
			      ASLFR_Window,		ed->ed_Window,
			      ASLFR_InitialDrawer,	path,
			      ASLFR_InitialFile,	FilePart (&ed->ed_PrefsWork.ep_Path[(ed->ed_Which / ed->ed_Width)][0]),
			      ASLFR_IntuiMsgFunc,	&hook,
			      ASLFR_SleepWindow,	TRUE,
			      ASLFR_RejectIcons,	TRUE,
			      ASLFR_FilterFunc,		&ed->ed_FilterHook,
			      TAG_DONE))
	    {
		/* Build the path name */
		strcpy (path, ed->ed_SampleReq->fr_Drawer);
		AddPart (path, ed->ed_SampleReq->fr_File, sizeof (path));

		/* Construct the error message */
		result = ES_DOSERROR;
		ed->ed_ErrorFileName = path;

		/* Did they enter a name? */
		if (strlen (ed->ed_SampleReq->fr_File) == 0)
		{
		    /* Clear the name */
		    ed->ed_PrefsWork.ep_Path[(ed->ed_Which / ed->ed_Width)][0] = 0;

		    /* Update the view */
		    update = TRUE;

		    /* Show normal */
		    result = ES_NORMAL;
		}
		/* See if the file exists */
		else if (lock = Lock (path, ACCESS_READ))
		{
		    if (NameFromLock (lock, path, sizeof (path)))
		    {
			if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
			{
			    if (dtn->dtn_Header->dth_GroupID == GID_PICTURE)
			    {
				strcpy (&ed->ed_PrefsWork.ep_Path[(ed->ed_Which / ed->ed_Width)][0], path);

				/* Update the view */
				update = TRUE;

				/* Show normal */
				result = ES_NORMAL;
			    }
			    else
			    {
				result = ES_NOT_A_PICTURE;
			    }
			    ReleaseDataType (dtn);
			}
		    }
		    UnLock (lock);
		}
	    }
	    break;

	case EC_MYRESETALL:
	    CopyPrefs(ed, &ed->ed_PrefsWork,&ed->ed_PrefsDefaults);
	    update = untest = TRUE;
	    break;

	case EC_MYLASTSAVED:
	    result = OpenPrefs(ed,ARC_NAME);
	    update = untest = TRUE;
	    break;

	case EC_MYRESTORE:
	    CopyPrefs(ed,&ed->ed_PrefsWork, &ed->ed_PrefsInitial);
	    update = untest = TRUE;
	    break;

	default:
	    break;
    }

    if (update)
    {
	if (untest && (ed->ed_Flags & EDF_TESTBD))
	{
	    result = TestFunc (ed);
	    ed->ed_Flags &= ~EDF_TESTBD;
	}
	RenderGadgets (ed);
	RenderDisplay (ed);
    }

    if (result != ES_NORMAL)
	ShowError2 (ed, result);
}

/*****************************************************************************/

EdCommand
GetCommand (EdDataPtr ed)
{

    return (EC_NOP);
}

/*****************************************************************************/

VOID
GetSpecialCmdState (EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{

    state->cs_Available = TRUE;
    state->cs_Selected = FALSE;
}
