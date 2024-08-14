/* pe_custom.c
 * Pointer Preference Editor
 */

#include "pointer.h"
#include <intuition/pointerclass.h>

#define	DB(x)	;
#define	DV(x)	;
#define	DR(x)	;

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
    {NM_ITEM, MSG_PTR_EDIT_LOAD_IMAGE, EC_LOAD_IMAGE, 0},
    {NM_ITEM, MSG_NOTHING, EC_NOP, 0},
    {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_MYRESETALL, 0},
    {NM_ITEM, MSG_EDIT_LAST_SAVED, EC_MYLASTSAVED, 0},
    {NM_ITEM, MSG_EDIT_RESTORE, EC_MYRESTORE, 0},

    {NM_TITLE, MSG_OPTIONS_MENU, EC_NOP, 0},
    {NM_ITEM, MSG_OPTIONS_SAVE_ICONS, EC_SAVEICONS, CHECKIT | MENUTOGGLE},

    {NM_END, MSG_NOTHING, EC_NOP, 0}
};

/*****************************************************************************/

#define NW_IDCMP	(IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | BUTTONIDCMP | PALETTEIDCMP | SLIDERIDCMP)
#define NW_FLAGS	(WFLG_ACTIVATE | WFLG_SIMPLE_REFRESH)
#define NWW_FLAGS	(WFLG_ACTIVATE | WFLG_SIMPLE_REFRESH | WFLG_DRAGBAR | WFLG_DEPTHGADGET)

/* main display gadgets */
struct EdGadget far EG[] =
{
    {BUTTON_KIND, 0, 0, 87, 14, MSG_SAVE_GAD, EC_SAVE, 0},	/* 0 : Save */
    {BUTTON_KIND, 0, 0, 87, 14, MSG_USE_GAD, EC_USE, 0},	/* 1 : Use */
    {BUTTON_KIND, 0, 0, 87, 14, MSG_CANCEL_GAD, EC_CANCEL, 0},	/* 2 : Cancel */

    {BUTTON_KIND, 0, 20, 148, 14, MSG_PTR_TEST_GAD, EC_TEST, 0},	/* 3 : Test */
    {BUTTON_KIND, 0, 36, 148, 14, MSG_PTR_CLEAR_GAD, EC_CLEAR, 0},	/* 4 : Clear */
    {BUTTON_KIND, 0, 52, 148, 14, MSG_PTR_SET_GAD, EC_SET, 0},	/* 5 : Set Grab Point */
    {BUTTON_KIND, 0, 68, 148, 14, MSG_PTR_RESET_GAD, EC_RESET, 0},	/* 6 : Reset */

    {PALETTE_KIND, 0, 86, 148, 40, NULL, EC_PALETTE, 0},	/* 7 : Palette */

    {SLIDER_KIND, 0, 130, 104, 11, MSG_PTR_RED_GAD, EC_RED, 0},	/* 8 : Red */
    {SLIDER_KIND, 0, 143, 104, 11, MSG_PTR_GREEN_GAD, EC_GREEN, 0},	/* 9 : Green */
    {SLIDER_KIND, 0, 156, 104, 11, MSG_PTR_BLUE_GAD, EC_BLUE, 0},	/* 10 : Blue  */

    {CYCLE_KIND, 0, 2, 100, 14, NULL, EC_TYPE, 0},	/* 11 : Type */
    {CYCLE_KIND, 0, 2, 148, 14, NULL, EC_SIZE, 0},	/* 12 : Size */
};

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

ULONG CheckBitRange (EdDataPtr ed, UBYTE bit)
{
    if (bit > HIGHBITSPERGUN)
	bit = HIGHBITSPERGUN;
    if (bit < LOWBITSPERGUN)
	bit = LOWBITSPERGUN;
    return ((ULONG) bit);
}

/*****************************************************************************/

/* value is in the current bit size */
ULONG Make8 (EdDataPtr ed, ULONG value, ULONG bits)
{
    ULONG result;
    ULONG mask;

    if (bits == 8)
	return (value);

    mask = value << (8 - bits);
    result = 0;
    while (mask)
    {
	result |= mask;
	mask = mask >> bits;
    }

    return (result);
}


/*****************************************************************************/

ULONG Make32 (EdDataPtr ed, ULONG eight)
{
    return ((eight << 24) + (eight << 16) + (eight << 8) + eight);
}

/*****************************************************************************/

VOID SetRGB8 (EdDataPtr ed, UWORD color, ULONG r, ULONG g, ULONG b)
{
    SetRGB32 (&ed->ed_Screen->ViewPort, color, Make32 (ed, r), Make32 (ed, g), Make32 (ed, b));
}

/*****************************************************************************/

UWORD GetIndex (EdDataPtr ed, UWORD pen)
{
    UWORD i;

    for (i = 0; i < MAXPENS; i++)
    {
	if (pen == ed->ed_ColorTable[i])
	    return (i);
    }
    return (0);
}

/*****************************************************************************/

void vprintf (EdDataPtr ed, void *fmt, LONG data,...)
{
    VPrintf (fmt, (LONG *) & data);
}

/*****************************************************************************/

VOID ProcessArgs (EdDataPtr ed, struct DiskObject * diskObj)
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
	/* This check is need to keep CPR from crashing */
	if (ed->ed_Args[OPT_CLIPUNIT])
	    ed->ed_ClipUnit = *((ULONG *) ed->ed_Args[OPT_CLIPUNIT]);

	ed->ed_RemapImage = (ed->ed_Args[OPT_NOREMAP]) ? FALSE : TRUE;
    }
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

EdStatus InitEdData (EdDataPtr ed)
{
    struct MonitorSpec *ms;
    struct Screen *scr;
    register WORD i, j;
    ULONG modeid;

    ed->ed_FilterHook.h_Entry = Filter;
    ed->ed_FilterHook.h_Data = ed;

    /* Clear the pens */
    for (i = 0; i < MAXPENS; i++)
	ed->ed_Pens[i] = (-1);

    /* Default to the first pen in our palette */
    ed->ed_APen = 5;
    ed->ed_Depth = 3;

    /* Set the default screen values */
    ed->ed_ModeId = LORES_KEY;
    ed->ed_SWidth = ed->ed_WWidth = 340;
    ed->ed_SHeight = ed->ed_WHeight = 200;

    /* Set the size for LORES */
    ed->ed_Width = 16;
    ed->ed_Height = 24;
    ed->ed_XMag = ed->ed_YMag = 6;

    /*******************************/
    /* Get the Display Information */
    /*******************************/

    if (scr = LockPubScreen ("Workbench"))
    {
	modeid = GetVPModeID (&scr->ViewPort);
	if (GetDisplayInfoData (NULL, (UBYTE *) & ed->ed_DispInfo, sizeof (struct DisplayInfo), DTAG_DISP, modeid))
	{
	    WORD sw, sh;

	    sw = (4 * ed->ed_DispInfo.SpriteResolution.x) / ed->ed_DispInfo.Resolution.x;
	    sh = (4 * ed->ed_DispInfo.SpriteResolution.y) / ed->ed_DispInfo.Resolution.y;
#if 0
	    ed->ed_XMag = sw;
	    ed->ed_YMag = sh;
#endif

	    DV (vprintf (ed, "screen res: %ld,%ld\n", (LONG) ed->ed_DispInfo.Resolution.x, (LONG) ed->ed_DispInfo.Resolution.y));
	    DV (vprintf (ed, "sprite res: %ld,%ld\n", (LONG) ed->ed_DispInfo.SpriteResolution.x, (LONG) ed->ed_DispInfo.SpriteResolution.y));
	    DV (vprintf (ed, "    aspect: %ld,%ld\n", (LONG) sw, (LONG) sh));
	    DV (vprintf (ed, "      bits: r%ld g%ld b%ld\n", (ULONG) ed->ed_DispInfo.RedBits, (ULONG) ed->ed_DispInfo.GreenBits, (ULONG) ed->ed_DispInfo.BlueBits));

#if 0
	    /* Get the bits */
	    ed->ed_RedBits   = CheckBitRange (ed, ed->ed_DispInfo.RedBits);
	    ed->ed_GreenBits = CheckBitRange (ed, ed->ed_DispInfo.GreenBits);
	    ed->ed_BlueBits  = CheckBitRange (ed, ed->ed_DispInfo.BlueBits);
#endif

	    if (ed->ed_DispInfo.PropertyFlags & DIPF_IS_SPRITES_ATT)
	    {
		DV (vprintf (ed, "supports attached sprites\n", NULL));
	    }
	    if (ed->ed_DispInfo.PropertyFlags & DIPF_IS_SPRITES_CHNG_RES)
	    {
		DV (vprintf (ed, "supports variable sprite resolutions\n", NULL));
	    }
	    if (ed->ed_DispInfo.PropertyFlags & DIPF_IS_SPRITES_CHNG_BASE)
	    {
		DV (vprintf (ed, "can change sprite base color\n", NULL));
	    }
	}

	if (ms = OpenMonitor (NULL, modeid))
	{
	    if (ms->ms_Flags & MSF_DOUBLE_SPRITES)
	    {
		DV (vprintf (ed, "scan double sprites\n", NULL));
	    }

	    if (ms->ms_Flags & MSF_REQUEST_A2024)
	    {
		DV (vprintf (ed, "request 2024\n", NULL));
		ed->ed_Flags |= EDF_2024;
	    }
	    CloseMonitor (ms);
	}

	{
	    if ((ed->ed_Pens[0] = ObtainPen (scr->ViewPort.ColorMap, -1, 0, 0, 0, PEN_EXCLUSIVE)) >= 0)
	    {
		if ((ed->ed_Pens[1] = ObtainPen (scr->ViewPort.ColorMap, -1, 0, 0, 0, PEN_EXCLUSIVE)) >= 0)
		{
		    if ((ed->ed_Pens[2] = ObtainPen (scr->ViewPort.ColorMap, -1, 0, 0, 0, PEN_EXCLUSIVE)) >= 0)
		    {
			ed->ed_Flags |= EDF_WINDOW | EDF_EXCLUSIVE;
			DV (vprintf (ed, "got exclusive pens\n", NULL));
		    }
		}
	    }

	    if (!(ed->ed_Flags & EDF_EXCLUSIVE))
	    {
		DV (vprintf (ed, "couldn't get exclusive pens\n", NULL));
		for (i = 0; i < MAXPENS; i++)
		{
		    ReleasePen (scr->ViewPort.ColorMap, ed->ed_Pens[i]);
		    ed->ed_Pens[i] = (-1);
		}
		for (i = 5, j = 0; j < 3; i++, j++)
		    ed->ed_Pens[j] = i;
	    }
	}

	ed->ed_ColorTable[0] = 0;
	for (i = 0; i < 3; i++)
	    ed->ed_ColorTable[(i + 1)] = ed->ed_Pens[i];

	if (ed->ed_Flags & EDF_WINDOW)
	{
	    ed->ed_Depth = scr->BitMap.Depth;
	    ed->ed_APen = 0;
	    ed->ed_WHeight -= 13;
	}

	UnlockPubScreen (NULL, scr);
    }

    /***********************/
    /* Initialize the Data */
    /***********************/

    /* Initialize the sketchpad gadget */
    if (ed->ed_SketchClass = initsketchGClass (ed))
    {
	/* Create the bitplanes */
	if ((allocpp (ed, &ed->ed_PrefsDefaults)) == ES_NORMAL)
	{
	    if ((allocpp (ed, &ed->ed_PrefsWork)) == ES_NORMAL)
	    {
		if ((allocpp (ed, &ed->ed_PrefsInitial)) == ES_NORMAL)
		{
		    if (ed->ed_UndoBM = AllocBitMap ((MAXWIDTH * 2), MAXHEIGHT, ed->ed_Depth, BMF_DISPLAYABLE | BMF_STANDARD | BMF_CLEAR, NULL))
		    {
			if (ed->ed_TempBM = AllocBitMap (MAXWIDTH, MAXHEIGHT, ed->ed_Depth, BMF_DISPLAYABLE | BMF_STANDARD | BMF_CLEAR, NULL))
			{
			    ULONG msize;

			    InitBitMap (&ed->ed_WorkBM, ed->ed_Depth, MAXWIDTH, MAXHEIGHT);
			    msize = RASSIZE (MAXWIDTH, MAXHEIGHT);
			    if (ed->ed_WorkBM.Planes[0] = (PLANEPTR) AllocVec (msize, MEMF_CHIP | MEMF_CLEAR))
			    {
				for (i = 1; i < ed->ed_Depth; i++)
				    ed->ed_WorkBM.Planes[i] = ed->ed_WorkBM.Planes[0];

				InitRastPort (&ed->ed_WorkRP);
				ed->ed_WorkRP.BitMap = &ed->ed_WorkBM;

				/* Make the default image */
				CopyFromDefault (ed);

				return (ES_NORMAL);
			    }
			    FreeBitMap (ed->ed_UndoBM);
			}
			FreeBitMap (ed->ed_TempBM);
		    }
		    FreeBitMap (ed->ed_PrefsInitial.ep_BMap);
		}
		FreeBitMap (ed->ed_PrefsWork.ep_BMap);
	    }
	    FreeBitMap (ed->ed_PrefsDefaults.ep_BMap);
	}
    }
    return (ES_NO_MEMORY);
}

/*****************************************************************************/

BOOL InitDisp (EdDataPtr ed)
{
    struct Rectangle mrect;
    struct Rectangle rect;
    BOOL result = FALSE;
    LONG left = 0;
    WORD rofs = 4;
    WORD lofs = 4;
    WORD ofs = 2;

    if (ed->ed_Flags & EDF_WINDOW)
    {
	if (ed->ed_Screen = LockPubScreen ("Workbench"))
	{
	    /* No problem */
	    result = TRUE;
	}
    }
    else
    {
	ofs = 13;
	lofs = -2;
	rofs = 6;

	/* Set the screen name */
	strcpy (ed->ed_ScreenNameBuf, "POINTER");
	ed->ed_PubScreenName = ed->ed_ScreenNameBuf;

	QueryOverscan (ed->ed_ModeId, &mrect, OSCAN_MAX);
	QueryOverscan (ed->ed_ModeId, &rect, OSCAN_TEXT);

	if (ed->ed_SWidth >= (rect.MaxX - rect.MinX + 1))
	{
	    if (ed->ed_SWidth <= (mrect.MaxX - rect.MinX + 1))
		rect.MaxX = ed->ed_SWidth + 1;
	}
	else
	{
	    left = ((rect.MaxX - rect.MinX + 1) - ed->ed_SWidth) / 2;
	}

	/* Open the screen */
	if (ed->ed_Screen = OpenPrefsScreen (ed,
					     SA_DisplayID, ed->ed_ModeId,
					     SA_Title, GetString (&ed->ed_LocaleInfo, MSG_PREFS_NAME),
					     SA_Depth, 4,
					     SA_Left, left,
					     SA_Width, ed->ed_SWidth,
					     SA_Height, ed->ed_SHeight,
					     SA_Font, &(ed->ed_TextAttr),
					     SA_PubName, ed->ed_ScreenNameBuf,
					     SA_DClip, &rect,
					     SA_LikeWorkbench, TRUE,
					     SA_AutoScroll, TRUE,
					     TAG_DONE))
	{
	    /* We opened it */
	    ed->ed_Flags |= EDF_OUR_PUBLIC;

	    /* Make the screen public */
	    PubScreenStatus (ed->ed_Screen, 0L);

	    /* No problem */
	    result = TRUE;
	}
    }

    if (ed->ed_Screen)
    {
#if 1
	ed->ed_RedBits   = CheckBitRange (ed, ed->ed_DispInfo.RedBits);
	ed->ed_GreenBits = CheckBitRange (ed, ed->ed_DispInfo.GreenBits);
	ed->ed_BlueBits  = CheckBitRange (ed, ed->ed_DispInfo.BlueBits);
#else
	/* Get the ModeId */
	ed->ed_ModeId = GetVPModeID (&ed->ed_Screen->ViewPort);
	if (GetDisplayInfoData (NULL, (UBYTE *) & ed->ed_DispInfo, sizeof (struct DisplayInfo), DTAG_DISP, ed->ed_ModeId))
	{
	    ed->ed_RedBits   = CheckBitRange (ed, ed->ed_DispInfo.RedBits);
	    ed->ed_GreenBits = CheckBitRange (ed, ed->ed_DispInfo.GreenBits);
	    ed->ed_BlueBits  = CheckBitRange (ed, ed->ed_DispInfo.BlueBits);
	}
#endif

	/* Save */
	EG[0].eg_LeftEdge = lofs;
	EG[0].eg_TopEdge = ed->ed_WHeight - EG[0].eg_Height - ofs;
	/* Use */
	EG[1].eg_LeftEdge = (ed->ed_WWidth - EG[1].eg_Width - rofs) / 2;
	EG[1].eg_TopEdge = ed->ed_WHeight - EG[1].eg_Height - ofs;
	/* Cancel */
	EG[2].eg_LeftEdge = ed->ed_WWidth - EG[2].eg_Width - rofs;
	EG[2].eg_TopEdge = ed->ed_WHeight - EG[2].eg_Height - ofs;

	/* Test */
	EG[3].eg_LeftEdge = ed->ed_WWidth - EG[3].eg_Width - rofs;
	/* Clear */
	EG[4].eg_LeftEdge = ed->ed_WWidth - EG[4].eg_Width - rofs;
	/* Set Point */
	EG[5].eg_LeftEdge = ed->ed_WWidth - EG[5].eg_Width - rofs;
	/* Reset Colors */
	EG[6].eg_LeftEdge = ed->ed_WWidth - EG[6].eg_Width - rofs;

	/* Palette */
	EG[7].eg_LeftEdge = ed->ed_WWidth - EG[7].eg_Width - rofs;

	/* R G B Sliders */
	EG[8].eg_LeftEdge = ed->ed_WWidth - EG[8].eg_Width - rofs;
	EG[9].eg_LeftEdge = ed->ed_WWidth - EG[9].eg_Width - rofs;
	EG[10].eg_LeftEdge = ed->ed_WWidth - EG[10].eg_Width - rofs;

	EG[11].eg_Width = 2 + (16 * 6) + 2 + 1;
	EG[11].eg_LeftEdge = lofs;
	EG[12].eg_LeftEdge = ed->ed_WWidth - EG[12].eg_Width - rofs;
    }

    return (result);
}

/*****************************************************************************/

VOID CopyPrefs (EdDataPtr ed, struct ExtPrefs * dp, struct ExtPrefs * sp)
{
    register WORD c;

    /* Make a backup */
    if (dp == &(ed->ed_PrefsWork))
    {
	Store (ed);
    }

    /* 32 is the size of the copiable portion of the structure */
    CopyMem (&sp->ep_PP[0], &dp->ep_PP[0], 32);
    CopyMem (&sp->ep_PP[1], &dp->ep_PP[1], 32);

    /* Copy from the sketchpad into our buffer */
    bltbm (sp->ep_BMap, 0, 0,
	   dp->ep_BMap, 0, 0, (MAXWIDTH * 2), MAXHEIGHT,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);

    /* Copy the colors */
    for (c = 0; c < MAXCOLORS; c++)
	dp->ep_CMap[c] = *(&sp->ep_CMap[c]);

    ed->ed_Size = (sp->ep_PP[0].pp_Size) ? sp->ep_PP[0].pp_Size - POINTERXRESN_LORES : 0;
}

/*****************************************************************************/

VOID CleanUpEdData (EdDataPtr ed)
{
    UWORD i;

    if (ed->ed_Cancelled && (ed->ed_Flags & EDF_TESTED))
    {
	/* Restore the initial pointer */
	CopyPrefs (ed, &ed->ed_PrefsWork, &ed->ed_PrefsInitial);

	/* Set the pointer */
	SavePrefs (ed, ENV_NAME);
    }

    /* Free the bitmaps */
    WaitBlit ();
    FreeBitMap (ed->ed_PrefsInitial.ep_BMap);
    FreeBitMap (ed->ed_PrefsWork.ep_BMap);
    FreeBitMap (ed->ed_PrefsDefaults.ep_BMap);
    FreeBitMap (ed->ed_UndoBM);
    FreeBitMap (ed->ed_TempBM);
    FreeVec (ed->ed_WorkBM.Planes[0]);

    /* Get rid of the sketch class */
    if (ed->ed_SketchClass)
    {
	freesketchGClass (ed, ed->ed_SketchClass);
    }
    /* Is our public screen open? */
    if (ed->ed_Screen)
    {
	if (ed->ed_Flags & EDF_OUR_PUBLIC)
	{
	    /* Keep trying to close it */
	    while (!CloseScreen (ed->ed_Screen))
		Delay (50);
	}
	else
	{
	    if (ed->ed_Flags & EDF_EXCLUSIVE)
	    {
		for (i = 0; i < MAXPENS; i++)
		{
		    ReleasePen (ed->ed_Screen->ViewPort.ColorMap, ed->ed_Pens[i]);
		    ed->ed_Pens[i] = (-1);
		}
	    }

	    UnlockPubScreen (NULL, ed->ed_Screen);
	}
    }
    FreeAslRequest (ed->ed_ImageReq);
}

/*****************************************************************************/

BOOL CreateDisplay (EdDataPtr ed)
{
    struct LocaleInfo *li;

    li = &(ed->ed_LocaleInfo);
    ed->ed_WhichLabels[0] = GetString (li, MSG_PTR_NORMAL_GAD);
    ed->ed_WhichLabels[1] = GetString (li, MSG_PTR_BUSY_GAD);

    ed->ed_SizeLabels[0] = GetString (li, MSG_PTR_LORES_GAD);
    ed->ed_SizeLabels[1] = GetString (li, MSG_PTR_HIRES_GAD);

    ed->ed_LastAdded = CreateContext (&ed->ed_Gadgets);

    DoPrefsGadget (ed, &EG[0], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[1], NULL, TAG_DONE);
    DoPrefsGadget (ed, &EG[2], NULL, TAG_DONE);

    /* Build the editor specific gadgets */
    RenderGadgets (ed);

    /* Create the menus and open the window */
    if ((ed->ed_LastAdded)
	&& (ed->ed_Menus = CreatePrefsMenus (ed, EM))
	&& (ed->ed_Window = OpenPrefsWindow (ed,
					     ((ed->ed_Flags & EDF_WINDOW) ? WA_Title : TAG_IGNORE), GetString (&ed->ed_LocaleInfo, MSG_PREFS_NAME),
					     WA_IDCMP, NW_IDCMP,
					     WA_Flags, ((ed->ed_Flags & EDF_WINDOW) ? NWW_FLAGS : NW_FLAGS),
					     WA_InnerWidth, ed->ed_WWidth,
					     WA_InnerHeight, ed->ed_WHeight,
					     WA_Borderless, !(ed->ed_Flags & EDF_WINDOW),
					     WA_Backdrop, !(ed->ed_Flags & EDF_WINDOW),
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

VOID DisposeDisplay (EdDataPtr ed)
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

VOID RefreshRepeats (EdDataPtr ed)
{
    WORD echo_w = MAXWIDTH + 4;
    WORD echo_h = MAXHEIGHT + 4;
    register WORD tx, ty;
    register WORD bx, by;
    register WORD i;

    /* Get a pointer to the sketchpad rastport */
    if (ed->ed_Window)
    {
	tx = ed->ed_GSketch->LeftEdge + ed->ed_GSketch->Width + 7;
	ty = ed->ed_GSketch->TopEdge + 2;

	/* Create the mask */
	SetRast (&ed->ed_WorkRP, 0);
	clipblit (&(ed->ed_PrefsWork.ep_RPort), (ed->ed_Which * MAXWIDTH), 0,
		  &ed->ed_WorkRP, 0, 0, ed->ed_Width, ed->ed_Height,
		  0xE0, ed->ed_GfxBase);

	for (i = 0; i < 4; i++, tx += echo_w)
	{
	    if (i == 2)
	    {
		ty += echo_h;
		tx = ed->ed_GSketch->LeftEdge + ed->ed_GSketch->Width + 7;
	    }

	    bx = tx + echo_w - 1;
	    by = ty + echo_h - 1;

	    SetAPen (ed->ed_Window->RPort, i);
	    RectFill (ed->ed_Window->RPort, tx, ty, bx, by);

	    clipblit (&ed->ed_WorkRP, 0, 0,
		      ed->ed_Window->RPort, tx + 2, ty + 2,
		      ed->ed_Width, ed->ed_Height,
		      0x20, ed->ed_GfxBase);

	    clipblit (&(ed->ed_PrefsWork.ep_RPort), (ed->ed_Which * MAXWIDTH), 0,
		      ed->ed_Window->RPort, tx + 2, ty + 2,
		      ed->ed_Width, ed->ed_Height,
		      0xE0, ed->ed_GfxBase);
	}
    }
}

/*****************************************************************************/

VOID RenderDisplay (EdDataPtr ed)
{
    WORD echo_w = MAXWIDTH + 4;
    WORD echo_h = MAXHEIGHT + 4;
    WORD echo_tw = 4 + (echo_w * 2) + 4;
    WORD echo_th = 2 + (echo_h * 2) + 2;
    register WORD tx, ty;

    DrawBB (ed,
	    ed->ed_GSketch->LeftEdge - 1,
	    ed->ed_GSketch->TopEdge,
	    ed->ed_GSketch->Width + 2,
	    ed->ed_GSketch->Height,
	    GT_VisualInfo, ed->ed_VisualInfo,
	    TAG_DONE);

    tx = ed->ed_GSketch->LeftEdge + ed->ed_GSketch->Width + 3;
    ty = ed->ed_GSketch->TopEdge;

    DrawBB (ed, tx, ty, echo_tw, echo_th,
	    GT_VisualInfo, ed->ed_VisualInfo,
	    GTBB_Recessed, TRUE,
	    TAG_DONE);

    /* Draw the repeat rectangles */
    RefreshRepeats (ed);
}

/*****************************************************************************/

VOID SetSliders (EdDataPtr ed)
{
    struct ExtPrefs *pp = &(ed->ed_PrefsWork);
    BOOL disable = FALSE;

    if (ed->ed_APen == 0)
	disable = TRUE;

    ed->ed_GRed = DoPrefsGadget (ed, &EG[8], ed->ed_GRed,
				 GTSL_Level, pp->ep_CMap[GetIndex (ed, ed->ed_APen)].t_Red >> (HIGHBITSPERGUN - ed->ed_RedBits),
				 GTSL_Min, 0,
				 GTSL_Max, (1 << ed->ed_RedBits) - 1,
				 GTSL_MaxLevelLen, 3,
				 GTSL_LevelFormat, "%3ld",
				 GA_Immediate, TRUE,
				 GA_Disabled, disable,
				 TAG_DONE);


    ed->ed_GGreen = DoPrefsGadget (ed, &EG[9], ed->ed_GGreen,
				   GTSL_Level, pp->ep_CMap[GetIndex (ed, ed->ed_APen)].t_Green >> (HIGHBITSPERGUN - ed->ed_GreenBits),
				   GTSL_Min, 0,
				   GTSL_Max, (1 << ed->ed_GreenBits) - 1,
				   GTSL_MaxLevelLen, 3,
				   GTSL_LevelFormat, "%3ld",
				   GA_Immediate, TRUE,
				   GA_Disabled, disable,
				   TAG_DONE);

    ed->ed_GBlue = DoPrefsGadget (ed, &EG[10], ed->ed_GBlue,
				  GTSL_Level, pp->ep_CMap[GetIndex (ed, ed->ed_APen)].t_Blue >> (HIGHBITSPERGUN - ed->ed_BlueBits),
				  GTSL_Min, 0,
				  GTSL_Max, (1 << ed->ed_BlueBits) - 1,
				  GTSL_MaxLevelLen, 3,
				  GTSL_LevelFormat, "%3ld",
				  GA_Immediate, TRUE,
				  GA_Disabled, disable,
				  TAG_DONE);
}

/*****************************************************************************/

BOOL upgadget (EdDataPtr ed, struct Gadget * g)
{

    if (g->Flags & GFLG_SELECTED)
    {
	struct Window *w = ed->ed_Window;
	USHORT pos;

	/* Remove the gadget from the list */
	pos = RemoveGList (w, g, 1L);

	/* Turn it back on */
	g->Flags &= ~GFLG_SELECTED;

	/* Add it back to the list */
	AddGList (w, g, (LONG) pos, 1L, NULL);

	/* Refresh the entire gadget */
	RefreshGList (g, w, NULL, 1);

	return (TRUE);
    }
    return (FALSE);
}

/*****************************************************************************/

VOID RenderGadgets (EdDataPtr ed)
{
    register WORD i, j;

    switch (ed->ed_Size)
    {
	case 0:
	    ed->ed_Width = 16;
	    ed->ed_Height = 24;
	    ed->ed_XMag = ed->ed_YMag = 6;
	    break;

	case 1:
	    ed->ed_Width = 32;
	    ed->ed_Height = 48;
	    ed->ed_XMag = ed->ed_YMag = 3;
	    break;
    }

    /* Use the colors from the new brush */
    for (i = 0, j = 1; i < 3; i++, j++)
    {
	DR (vprintf (ed, "%ld = %ld\n", (ULONG) i, ed->ed_Pens[i]));
	SetRGB8 (ed, ed->ed_Pens[i],
		 ed->ed_PrefsWork.ep_CMap[j].t_Red,
		 ed->ed_PrefsWork.ep_CMap[j].t_Green,
		 ed->ed_PrefsWork.ep_CMap[j].t_Blue);
    }

    ed->ed_GTest = DoPrefsGadget (ed, &EG[3], ed->ed_GTest, TAG_DONE);

    ed->ed_GClear = DoPrefsGadget (ed, &EG[4], ed->ed_GClear, TAG_DONE);

    /* Force the gadget to be toggle select */
    if (ed->ed_GSetPoint = DoPrefsGadget (ed, &EG[5], ed->ed_GSetPoint, TAG_DONE))
    {
	ed->ed_GSetPoint->Activation |= GACT_TOGGLESELECT;
    }
    ed->ed_GResetColor = DoPrefsGadget (ed, &EG[6], ed->ed_GResetColor, TAG_DONE);

    ed->ed_GPalette = DoPrefsGadget (ed, &EG[7], ed->ed_GPalette,
				     GTPA_Color, ed->ed_APen,
				     GTPA_NumColors, 4,
				     GTPA_ColorTable, ed->ed_ColorTable,
				     GTPA_IndicatorWidth, 18,
				     TAG_DONE);

    /* Set/Create the sliders */
    SetSliders (ed);

    /* Set the radio button for which image to edit */
    ed->ed_GType = DoPrefsGadget (ed, &EG[11], ed->ed_GType,
				  GTCY_Labels, ed->ed_WhichLabels,
				  GTCY_Active, ed->ed_Which,
				  TAG_DONE);

    if (ed->ed_DispInfo.PropertyFlags & DIPF_IS_SPRITES_CHNG_RES)
    {
	/* Set the radio button for which size to edit */
	ed->ed_GSize = DoPrefsGadget (ed, &EG[12], ed->ed_GSize,
				      GTCY_Labels, ed->ed_SizeLabels,
				      GTCY_Active, ed->ed_Size,
				      TAG_DONE);
    }

    /* Create the sketch gadget */
    if ((ed->ed_GSketch == NULL) && (ed->ed_LastAdded))
    {
	/* Create the sketchpad gadget */
	ed->ed_LastAdded = ed->ed_GSketch = (struct Gadget *)
	  NewPrefsObject (ed, ed->ed_SketchClass, NULL,
			  GA_Left, ed->ed_Screen->WBorLeft + ((ed->ed_Flags & EDF_WINDOW) ? 4 : -1),
			  GA_Top, 20 + ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1,
			  GA_Width, 2 + (16 * 6) + 1,
			  GA_Height, 2 + (24 * 6) + 1,
			  GA_Immediate, TRUE,
			  GA_FollowMouse, TRUE,
			  GA_RelVerify, TRUE,
			  GA_ID, EC_SKETCH,
			  GA_UserData, EC_SKETCH,
			  GA_Previous, ed->ed_LastAdded,
			  TAG_DONE);
    }

    if (ed->ed_GSketch)
    {
	UpdateSketch (ed);
    }
}

/*****************************************************************************/

VOID SyncTextGadgets (EdDataPtr ed)
{
}

/*****************************************************************************/

EdStatus TestFunc (EdDataPtr ed)
{
    struct SignalSemaphore *sem;
    register WORD i, j;

    DoPrefsGadget (ed, NULL, ed->ed_GTest, GA_Disabled, TRUE, TAG_DONE);
    Forbid ();
    if (sem = FindSemaphore ("« IPrefs »"))
	ObtainSemaphore (sem);
    Permit ();

    if ((ed->ed_Screen->Flags & SCREENTYPE) != WBENCHSCREEN)
    {
	DR (vprintf (ed, "test func\n", NULL));
	for (i = 17, j = 1; j < 8; i++, j++)
	{
	    SetRGB8 (ed, i,
		     ed->ed_PrefsWork.ep_CMap[j].t_Red,
		     ed->ed_PrefsWork.ep_CMap[j].t_Green,
		     ed->ed_PrefsWork.ep_CMap[j].t_Blue);
	}
    }

    ed->ed_Flags |= EDF_TESTED;

    if (sem)
	ReleaseSemaphore (sem);
    DoPrefsGadget (ed, NULL, ed->ed_GTest, GA_Disabled, FALSE, TAG_DONE);
    return (SavePrefs (ed, ENV_NAME));
}

/*****************************************************************************/

VOID UpdateColor (EdDataPtr ed)
{
    SetRGB8 (ed, ed->ed_APen,
	     ed->ed_PrefsWork.ep_CMap[GetIndex (ed, ed->ed_APen)].t_Red,
	     ed->ed_PrefsWork.ep_CMap[GetIndex (ed, ed->ed_APen)].t_Green,
	     ed->ed_PrefsWork.ep_CMap[GetIndex (ed, ed->ed_APen)].t_Blue);
}

/*****************************************************************************/

VOID ResetColors (EdDataPtr ed)
{
    register UWORD c;

    for (c = 0; c < MAXCOLORS; c++)
	ed->ed_PrefsWork.ep_CMap[c] = ed->ed_PrefsInitial.ep_CMap[c];

    RenderGadgets (ed);
}

/*****************************************************************************/

VOID ProcessSpecialCommand (EdDataPtr ed, EdCommand ec)
{
    EdStatus result = ES_NORMAL;
    BOOL untest = FALSE;
    UWORD icode;
    LONG hx, hy;

    icode = ed->ed_CurrentMsg.Code;
    switch (ec)
    {
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
	    setgadgetattrs (ed, ed->ed_GSketch, SPA_Restore, TRUE, TAG_DONE);
	    break;

	case EC_TEST:
	    result = TestFunc (ed);
	    break;

	case EC_CLEAR:
	    setgadgetattrs (ed, ed->ed_GSketch,
			    SPA_Store, TRUE,
			    SPA_Clear, TRUE,
			    TAG_DONE);
	    break;

	case EC_RESET:
	    ResetColors (ed);
	    break;

	case EC_PALETTE:
	    /* Set the currently select color */
	    ed->ed_APen = icode;

	    /* Set the sliders */
	    SetSliders (ed);
	    break;

	case EC_RED:
	    ed->ed_PrefsWork.ep_CMap[GetIndex (ed, ed->ed_APen)].t_Red = Make8 (ed, icode, ed->ed_RedBits);
	    UpdateColor (ed);
	    break;

	case EC_GREEN:
	    ed->ed_PrefsWork.ep_CMap[GetIndex (ed, ed->ed_APen)].t_Green = Make8 (ed, icode, ed->ed_GreenBits);
	    UpdateColor (ed);
	    break;

	case EC_BLUE:
	    ed->ed_PrefsWork.ep_CMap[GetIndex (ed, ed->ed_APen)].t_Blue = Make8 (ed, icode, ed->ed_GreenBits);
	    UpdateColor (ed);
	    break;

	case EC_SKETCH:
	    upgadget (ed, ed->ed_GSetPoint);
	    break;

	case EC_MYRESETALL:
	    CopyPrefs (ed, &ed->ed_PrefsWork, &ed->ed_PrefsDefaults);
	    untest = TRUE;
	    break;

	case EC_MYLASTSAVED:
	    result = OpenPrefs (ed, ARC_NAME);
	    untest = TRUE;
	    break;

	case EC_MYRESTORE:
	    CopyPrefs (ed, &ed->ed_PrefsWork, &ed->ed_PrefsInitial);
	    untest = TRUE;
	    break;

	case EC_TYPE:
	    ed->ed_Which = (UWORD) icode;

	    /* Update the display */
	    RenderGadgets (ed);
	    RenderDisplay (ed);
	    break;

	case EC_SIZE:
	    ed->ed_Size = (UWORD) icode;

	    /* Both pointers must be the same resolution */
	    ed->ed_PrefsWork.ep_PP[0].pp_Size = icode + POINTERXRESN_LORES;
	    ed->ed_PrefsWork.ep_PP[1].pp_Size = icode + POINTERXRESN_LORES;

	    /* Both pointers must be the same resolution */
	    ed->ed_PrefsWork.ep_PP[0].pp_YSize = (icode) ? POINTERYRESN_HIGH : POINTERYRESN_DEFAULT;
	    ed->ed_PrefsWork.ep_PP[1].pp_YSize = (icode) ? POINTERYRESN_HIGH : POINTERYRESN_DEFAULT;

	    /* Range check the hot spot */
	    hx = (ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_X) ? 65536L - (LONG)ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_X : 0;
	    hy = (ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_Y) ? 65536L - (LONG)ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_Y : 0;
	    if ((icode==0) && (hx >= 16L))
		ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_X = -15L;
	    if ((icode==0) && (hy >= 24L))
		ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_Y = -23L;

	    /* Update the display */
	    RenderGadgets (ed);
	    RenderDisplay (ed);
	    break;

	default:
	    break;
    }

    if (untest)
    {
	if (ed->ed_Flags & EDF_TESTED)
	{
	    TestFunc (ed);
	    ed->ed_Flags &= ~EDF_TESTED;
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
