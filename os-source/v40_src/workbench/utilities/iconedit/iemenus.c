
/* includes */
#include <exec/memory.h>
#include <dos/dostags.h>
#include <graphics/scale.h>
#include <libraries/gadtools.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/asl_protos.h>
#include <clib/exec_protos.h>

/* direct ROM interface */
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/exec_pragmas.h>

/* application includes */
#include "iemenus.h"
#include "iemain.h"
#include "ieio.h"
#include "iemisc.h"
#include "ieiff.h"
#include "ieclip.h"
#include "texttable.h"
#include "iegads.h"
#include "ieutils.h"

/*****************************************************************************/

extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *IconBase;
extern struct Library *AslBase;
extern struct Library *GfxBase;
extern struct Library *GadToolsBase;
extern struct Library *SysBase;
extern struct Library *DataTypesBase;

/*****************************************************************************/

BOOL SaveAsCFunc (WindowInfoPtr wi);
BOOL TemplateFunc (WindowInfoPtr wi);
BOOL EraseFunc (WindowInfoPtr wi);
BOOL RecolorFunc (WindowInfoPtr wi);
BOOL TopLeftFunc (WindowInfoPtr wi);
BOOL PaletteFunc (WindowInfoPtr wi);

BOOL LoadIFFImage (WindowInfoPtr wi, STRPTR name, USHORT cur, BOOL errs);
BOOL SaveIFFBrush (WindowInfoPtr wi, STRPTR name, DynamicImagePtr di);

/*****************************************************************************/

/** !!!! DANGER !!!!
 ** If you change this strip, also change the stupid,
 ** yet inconvenient, absolute item # references in the init code
 ** below
 ** !!!! DANGER !!!!
 **/

struct NewMenu NM[] =
{
    {NM_TITLE, (STRPTR) MSG_IE_PROJECT_MENU,},
    {NM_ITEM, (STRPTR) MSG_IE_PROJECT_NEW, 0, 0, 0, NewFunc,},
    {NM_ITEM, (STRPTR) NM_BARLABEL,},
    {NM_ITEM, (STRPTR) MSG_IE_PROJECT_OPEN, 0, 0, 0, OpenFunc,},
    {NM_ITEM, (STRPTR) NM_BARLABEL,},
    {NM_ITEM, (STRPTR) MSG_IE_PROJECT_SAVE, 0, 0, 0, SaveFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_PROJECT_SAVEAS, 0, 0, 0, SaveAsFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_PROJECT_SAVEASDEF, 0, 0, 0, SaveDefFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_PROJECT_SAVEASC, 0, 0, 0, SaveAsCFunc,},
    {NM_ITEM, (STRPTR) NM_BARLABEL,},
    {NM_ITEM, (STRPTR) MSG_IE_PROJECT_QUIT, 0, 0, 0, QuitFunc,},

    {NM_TITLE, (STRPTR) MSG_IE_EDIT_MENU,},
    {NM_ITEM, (STRPTR) MSG_IE_EDIT_CUT, 0, 0, 0, CutClipFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_EDIT_COPY, 0, 0, 0, CopyClipFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_EDIT_PASTE, 0, 0, 0, PasteClipFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_EDIT_ERASE, 0, 0, 0, EraseFunc,},
    {NM_ITEM, (STRPTR) NM_BARLABEL,},
    {NM_ITEM, (STRPTR) MSG_IE_EDIT_OPENCLIP, 0, 0, 0, OpenClipFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_EDIT_SAVECLIP, 0, 0, 0, SaveClipFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_EDIT_SHOWCLIP, 0, 0, 0, ShowClipFunc,},

    {NM_TITLE, (STRPTR) MSG_IE_TYPE_MENU,},
    {NM_ITEM, (STRPTR) MSG_IE_TYPE_DISK, 0, CHECKIT, ~1, V (0),},
    {NM_ITEM, (STRPTR) MSG_IE_TYPE_DRAWER, 0, CHECKIT, ~2, V (1),},
    {NM_ITEM, (STRPTR) MSG_IE_TYPE_TOOL, 0, CHECKIT | CHECKED, ~4, V (2),},
    {NM_ITEM, (STRPTR) MSG_IE_TYPE_PROJECT, 0, CHECKIT, ~8, V (3),},
    {NM_ITEM, (STRPTR) MSG_IE_TYPE_GARBAGE, 0, CHECKIT, ~16, V (4),},

    {NM_TITLE, (STRPTR) MSG_IE_HIGHLIGHT_MENU,},
    {NM_ITEM, (STRPTR) MSG_IE_HIGHLIGHT_COMPLEMENT, 0, CHECKIT | CHECKED, ~1, V (0),},
    {NM_ITEM, (STRPTR) MSG_IE_HIGHLIGHT_BACKFILL, 0, CHECKIT, ~2, V (1),},
    {NM_ITEM, (STRPTR) MSG_IE_HIGHLIGHT_IMAGE, 0, CHECKIT, ~4, V (2),},

    {NM_TITLE, (STRPTR) MSG_IE_IMAGES_MENU,},
    {NM_ITEM, (STRPTR) MSG_IE_IMAGES_EXCHANGE, 0, 0, 0, ExchangeFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_IMAGES_COPY, 0, 0, 0, CopyFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_IMAGES_TEMPLATE, 0, 0, 0, TemplateFunc,},
    {NM_ITEM, (STRPTR) NM_BARLABEL,},
    {NM_ITEM, (STRPTR) MSG_IE_IMAGES_LOAD, 0, 0, 0, 0,},
    {NM_SUB, (STRPTR) MSG_IE_IMAGES_IFFBRUSH, 0, 0, 0, LIFFFunc,},
    {NM_SUB, (STRPTR) MSG_IE_IMAGES_NORMAL, 0, 0, 0, LStdFunc,},
    {NM_SUB, (STRPTR) MSG_IE_IMAGES_SELECTED, 0, 0, 0, LAltFunc,},
    {NM_SUB, (STRPTR) MSG_IE_IMAGES_BOTH, 0, 0, 0, LBthFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_IMAGES_SAVEIFF, 0, 0, 0, SIFFFunc,},
    {NM_ITEM, (STRPTR) NM_BARLABEL,},
    {NM_ITEM, (STRPTR) MSG_IE_IMAGES_RESTORE, 0, 0, 0, RestoreFunc,},

    {NM_TITLE, (STRPTR) MSG_IE_EXTRAS_MENU,},
    {NM_ITEM, (STRPTR) MSG_IE_EXTRAS_RECOLOR, 0, 0, 0, RecolorFunc,},	/* 43 */
    {NM_ITEM, (STRPTR) MSG_IE_EXTRAS_AUTO, 0, 0, 0, TopLeftFunc,},
    {NM_ITEM, (STRPTR) MSG_IE_EXTRAS_PALETTE, 0, 0, 0, PaletteFunc,},

    {NM_TITLE, (STRPTR) MSG_IE_SETTINGS_MENU,},
    {NM_ITEM, (STRPTR) MSG_IE_SETTINGS_GRID, 0, CHECKIT | MENUTOGGLE, 0, V (0),},
    {NM_ITEM, (STRPTR) MSG_IE_SETTINGS_ICONS, 0, CHECKIT | MENUTOGGLE, 0, V (1),},
    {NM_ITEM, (STRPTR) NM_BARLABEL,},
    {NM_ITEM, (STRPTR) MSG_IE_SETTINGS_SAVE, 0, 0, 0, V (2),},

    {NM_END, 0, 0, 0, 0, 0},
};

/** !!!! DANGER !!!!
 ** If you change this strip, also change the stupid,
 ** yet inconvenient, absolute item # references in the init code
 ** below
 ** !!!! DANGER !!!!
 **/

/*****************************************************************************/

struct Image blank = {0, 0, 0, 0, 0, NULL, 0, 1, NULL};

/*****************************************************************************/

void CreateWinTitle (WindowInfoPtr wi, UBYTE *name)
{
    sprintf (wi->wintitle, GetString (MSG_IE_TITLE), FilePart (name));
}

/*****************************************************************************/

VOID CheckClippable (WindowInfoPtr wi, struct DiskObject * dob)
{
    struct Image *im;

    im = dob->do_Gadget.GadgetRender;
    if ((im) && (im->Width > ICON_WIDTH) && (im->Height > ICON_HEIGHT))
    {
	wi->clippable = TRUE;
    }

    im = dob->do_Gadget.SelectRender;
    if ((im) && (im->Width > ICON_WIDTH) && (im->Height > ICON_HEIGHT))
    {
	wi->clippable = TRUE;
    }
}

/*****************************************************************************/

BOOL LayoutIEMenus (WindowInfoPtr wi, struct Menu * menus, ULONG tag1,...)
{
    return (LayoutMenusA (menus, wi->vi, (struct TagItem *) & tag1));
}

/*****************************************************************************/

struct Menu *CreateIEMenus (WindowInfoPtr wi, struct NewMenu * nm)
{
    struct Menu *menus;
    UWORD i;

    i = 0;
    while (nm[i].nm_Type != NM_END)
    {
	if (nm[i].nm_Type == NM_TITLE)
	{
	    nm[i].nm_Label = GetString ((LONG) nm[i].nm_Label);
	}
	else if ((nm[i].nm_Type != IM_ITEM) && (nm[i].nm_Label != NM_BARLABEL))
	{
	    nm[i].nm_CommKey = GetString ((LONG) nm[i].nm_Label);
	    nm[i].nm_Label = &nm[i].nm_CommKey[2];
	    if (nm[i].nm_CommKey[0] == ' ')
		nm[i].nm_CommKey = NULL;
	}
	i++;
    }

    if (menus = CreateMenusA (nm, NULL))
    {
	if (!(LayoutIEMenus (wi, menus, GTMN_NewLookMenus, TRUE,
			     TAG_DONE)))
	{
	    FreeMenus (menus);
	    menus = NULL;
	}
    }

    return (menus);
}

/*****************************************************************************/

BOOL CreateIconMenus (WindowInfoPtr wi)
{
    /*** DANGER! Absolute access to menu strip items! */
    if (!wi->w_SaveSrc)
    {
	NM[8].nm_Type = IM_ITEM;
	NM[8].nm_Label = (STRPTR) & blank;
	NM[8].nm_CommKey = NULL;
	NM[8].nm_Flags = NM_ITEMDISABLED;
	NM[8].nm_MutualExclude = NULL;
    }

    /*** DANGER! Absolute access to menu strip items! */
    NM[48].nm_Flags |= ((wi->w_UseGrid) ? CHECKED : NULL);
    NM[49].nm_Flags |= ((wi->w_SaveIcons) ? CHECKED : NULL);

    if (wi->menu = CreateIEMenus (wi, NM))
    {
	SetMenuStrip (wi->win, wi->menu);
	return (TRUE);
    }

    return (FALSE);
}


/*****************************************************************************/


VOID CheckMenuItem (WindowInfoPtr wi, USHORT menunum, UWORD item)
{
    struct Menu *m;
    struct MenuItem *mi;
    UWORD mid;
    USHORT i;

    i = 0;
    m = wi->menu;
    while (i < menunum && m)
    {
	m = m->NextMenu;
	i++;
    }
    if (m)
    {
	mi = m->FirstItem;
	while (mi)
	{
	    mi->Flags &= ~CHECKED;
	    mid = (UWORD) MENU_USERDATA (mi);
	    if (mid == item)
		mi->Flags |= CHECKED;
	    mi = mi->NextItem;
	}
    }
}


/*****************************************************************************/


VOID SetHilite (WindowInfoPtr wi, struct DiskObject * dob, UWORD mhilite)
{
    struct Window *wp;
    USHORT pos;

    wp = wi->win;
    pos = RemoveGList (wp, wi->img1, 1L);
    wi->img1->Flags = dob->do_Gadget.Flags = GADGIMAGE | (USHORT) wi->hilite;
    AddGList (wp, wi->img1, (LONG) pos, 1L, NULL);
    RefreshGList (wi->img1, wp, NULL, 1L);
    if (wi->currentwin == 1)
    {
	wi->currentwin = 0;
	GT_SetGadgetAttrs (wi->mxgad, wp, NULL,
			   GTMX_Active, wi->currentwin,
			   TAG_DONE);
	wi->Adi = &wi->images[wi->currentwin];
	SPSetRepeat (wi->wi_sketch, wi->Adi,
		     (wi->w_LO + 77) + 16, wi->wintopx[wi->currentwin]);
	SPRefresh (wi->wi_sketch);
    }

    wi->changed |= CH_MINOR;
}


/*****************************************************************************/


VOID SetType (WindowInfoPtr wi, struct DiskObject * dob, UWORD mtype)
{

    wi->type = mtype;
    wi->diskobj->do_Type = (UBYTE) mtype;
    wi->changed |= CH_MINOR;
}


/*****************************************************************************/


BOOL HandleMenuEvent (WindowInfoPtr wi, UWORD code)
{
    struct MenuItem *item;
    BOOL terminated = FALSE;
    LONG menunum, itemnum;

    while ((code != MENUNULL) && (!terminated))
    {
	item = ItemAddress (wi->menu, code);
	menunum = (int) MENUNUM (code);
	itemnum = (int) ITEMNUM (code);

	if ((menunum == 0) || (menunum == 1) || (menunum == 4) || (menunum == 5))
	{
	    BOOL (*fptr) (WindowInfoPtr) = MENU_USERDATA (item);
	    terminated = (*fptr) (wi);
	}
	else if (menunum == 2)
	{
	    wi->type = (UWORD) MENU_USERDATA (item);
	    SetType (wi, wi->diskobj, wi->type);
	}
	else if (menunum == 3)
	{
	    wi->hilite = (UWORD) MENU_USERDATA (item);
	    SetHilite (wi, wi->diskobj, wi->hilite);
	}
	else if (menunum == 6)
	{
	    switch (itemnum)
	    {
		    /* Toggle Grid */
		case 0:
		    if (item->Flags & CHECKED)
		    {
			wi->wi_sketch->Grid = TRUE;
		    }
		    else
		    {
			wi->wi_sketch->Grid = FALSE;
		    }

		    /* Remember this for settings */
		    wi->w_UseGrid = wi->wi_sketch->Grid;
		    break;

		    /* Toggle Icon */
		case 1:
		    if (item->Flags & CHECKED)
		    {
			wi->w_SaveIcons = TRUE;
		    }
		    else
		    {
			wi->w_SaveIcons = FALSE;
		    }
		    break;

		    /* Save Settings */
		case 3:
		    SetIconEditPrefs (wi);
		    break;
	    }

	    SPRefresh (wi->wi_sketch);
	}

	code = item->NextSelect;
    }

    return (terminated);
}


/*****************************************************************************/


BOOL NewFunc (WindowInfoPtr wi)
{
    struct Window *wp;
    SHORT i;
    BOOL cancel;

    wp = wi->win;
    cancel = CheckForChanges (wi, MSG_IE_NEWANDZAP, MSG_IE_NEWANDZAP_GADS);
    if (!cancel)
    {
	SPSaveToUndo (wi->wi_sketch);

	for (i = 0; i < 255; i++)
	    wi->iconname[i] = 0;
	if (wi->diskobj)
	    FreeDiskObject (wi->diskobj);
	wi->diskobj = NULL;
	if (wi->type < 0)
	    wi->type = 0;
	if (wi->diskobj = GetDefDiskObject ((UBYTE) (wi->type + 1)))
	{
	    UpdateIconInfo (wi, wi->diskobj);
	    wi->currentwin = 0;
	    wi->Adi = &wi->images[0];
	    SPSetRepeat (wi->wi_sketch, wi->Adi,
			 (wi->w_LO + 77) + 16, wi->wintopx[wi->currentwin]);
	    SPRefresh (wi->wi_sketch);
	    strcpy (wi->wintitle, GetString (MSG_IE_DEFAULT_TITLE));
	    SetWindowTitles (wp, wi->wintitle, (STRPTR) - 1);
	}

	/* Clear the changes flag */
	wi->changed = CH_NONE;
	wi->clippable = FALSE;
    }
    return (FALSE);
}

/*****************************************************************************/

VOID OpenNamedIcon (WindowInfoPtr wi, UBYTE * name, BOOL real)
{
    struct Window *wp;
    struct DiskObject *dob;

    wp = wi->win;
    if (dob = LoadIcon (name, real))
    {
	UpdateIconInfo (wi, dob);
	wi->currentwin = 0;
	wi->Adi = &wi->images[0];
	SPSetRepeat (wi->wi_sketch, wi->Adi,
		     (wi->w_LO + 77) + 16, wi->wintopx[wi->currentwin]);
	SPRefresh (wi->wi_sketch);

	if (wi->diskobj)
	{
	    FreeDiskObject (wi->diskobj);
	}

	wi->diskobj = dob;

	strcpy (wi->iconname, name);
	CreateWinTitle (wi, name);
	SetWindowTitles (wp, wi->wintitle, (UBYTE *) - 1);

	/* Clear the changes flag */
	wi->changed = CH_NONE;
	wi->clippable = FALSE;
	CheckClippable (wi, dob);
    }
    else
    {
	sprintf (wi->w_Buffer, GetString (MSG_IE_ERROR_COULDNT_LOAD), name);
	NotifyUser (0, wi->w_Buffer);
    }

    /* Set up the file requester to match */
    FixFileAndPath (wi->w_FR[FR_ICON], wi->iconname);
}


/*****************************************************************************/


BOOL OpenFunc (WindowInfoPtr wi)
{
    struct Window *wp;
    UBYTE path[255], name[50];
    BOOL cancel;

    wp = wi->win;
    cancel = CheckForChanges (wi, MSG_IE_OPENANDZAP, MSG_IE_OPENANDZAP_GADS);
    if (!cancel)
    {
	struct TagItem tg[10];

	/* Initialize the file requester position */
	if (wi->Width == (-1))
	{
	    wi->LeftEdge = wp->LeftEdge + 12;
	    wi->TopEdge  = wp->TopEdge + 12;
	    wi->Width    = 320;
	    wi->Height   = 175;
	}

	/* Set the file requester position */
	tg[4].ti_Tag = ASL_LeftEdge;
	tg[4].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
	tg[5].ti_Tag = ASL_TopEdge;
	tg[5].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
	tg[6].ti_Tag = ASL_Width;
	tg[6].ti_Data = (ULONG) wi->Width;
	tg[7].ti_Tag = ASL_Height;
	tg[7].ti_Data = (ULONG) wi->Height;

	tg[0].ti_Tag = ASL_Hail;
	tg[0].ti_Data = (ULONG) GetString (MSG_IE_OPENICON_TITLE);
	tg[1].ti_Tag = ASL_FuncFlags;
	tg[1].ti_Data = NULL;
	tg[2].ti_Tag = ASL_OKText;
	tg[2].ti_Data = (ULONG) GetString (MSG_IE_OPEN_GAD);
	tg[3].ti_Tag = ASL_Window;
	tg[3].ti_Data = (ULONG) wp;
	tg[8].ti_Tag = ASL_Pattern;
	tg[8].ti_Data = (ULONG) "#?.info";
	tg[9].ti_Tag = TAG_DONE;

	/* Do we have a file requester? */
	if (wi->w_FR[FR_ICON])
	{
	    if (AslRequest (wi->w_FR[FR_ICON], tg))
	    {
		/* Make sure everything is set up properly */
		FixFileAndPath (wi->w_FR[FR_ICON], NULL);

		/* Build the complete name */
		strcpy (name, wi->w_FR[FR_ICON]->rf_File);
		strcpy (path, wi->w_FR[FR_ICON]->rf_Dir);
		strcpy (wi->w_Tmp, path);
		AddPart (wi->w_Tmp, name, 254);

		SPSaveToUndo (wi->wi_sketch);

		/* Open the icon */
		OpenNamedIcon (wi, wi->w_Tmp, TRUE);

		/* Clear changes flag */
		wi->changed = CH_NONE;
	    }

	    /* Remember where they moved it */
	    wi->LeftEdge = wi->w_FR[FR_ICON]->rf_LeftEdge - wp->LeftEdge;
	    wi->TopEdge = wi->w_FR[FR_ICON]->rf_TopEdge - wp->TopEdge;
	    wi->Width = wi->w_FR[FR_ICON]->rf_Width;
	    wi->Height = wi->w_FR[FR_ICON]->rf_Height;
	}
    }

    return (FALSE);
}


/*****************************************************************************/


BOOL SaveFunc (WindowInfoPtr wi)
{
    if (strlen (wi->iconname) == 0)
    {
	/* Not named yet, so let's get a name */
	SaveAsFunc (wi);
    }
    else
    {
	if (SaveIcon (wi, wi->iconname, wi->diskobj, 0))
	{
	    /* Clear the changes flags */
	    wi->changed = CH_NONE;

	    /* Open the icon */
	    OpenNamedIcon (wi, wi->iconname, TRUE);
	}
	else
	    wi->changed |= CH_CANCEL;
    }

    return (FALSE);
}


/*****************************************************************************/

BOOL SaveAsFunc (WindowInfoPtr wi)
{
    struct Window *wp;
    UBYTE path[255], name[50];

    wp = wi->win;
    if (wi->w_FR[FR_ICON])
    {
	struct TagItem tg[10];

	/* Initialize the file requester position */
	if (wi->Width == (-1))
	{
	    wi->LeftEdge = wp->LeftEdge + 12;
	    wi->TopEdge  = wp->TopEdge + 12;
	    wi->Width    = 320;
	    wi->Height   = 175;
	}

	/* Set the file requester position */
	tg[4].ti_Tag = ASL_LeftEdge;
	tg[4].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
	tg[5].ti_Tag = ASL_TopEdge;
	tg[5].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
	tg[6].ti_Tag = ASL_Width;
	tg[6].ti_Data = (ULONG) wi->Width;
	tg[7].ti_Tag = ASL_Height;
	tg[7].ti_Data = (ULONG) wi->Height;

	tg[0].ti_Tag = ASL_Hail;
	tg[0].ti_Data = (ULONG) GetString (MSG_IE_SAVEICON_TITLE);
	tg[1].ti_Tag = ASL_FuncFlags;
	tg[1].ti_Data = FILF_SAVE;
	tg[2].ti_Tag = ASL_OKText;
	tg[2].ti_Data = (ULONG) GetString (MSG_IE_SAVE_GAD);
	tg[3].ti_Tag = ASL_Window;
	tg[3].ti_Data = (ULONG) wp;
	tg[8].ti_Tag = ASL_Pattern;
	tg[8].ti_Data = (ULONG) "#?";
	tg[9].ti_Tag = TAG_DONE;

	/* Request the icon name */
	if (AslRequest (wi->w_FR[FR_ICON], tg))
	{
	    WORD count;

	    /* Fix the entries */
	    FixFileAndPath (wi->w_FR[FR_ICON], NULL);

	    strcpy (name, wi->w_FR[FR_ICON]->rf_File);
	    strcpy (path, wi->w_FR[FR_ICON]->rf_Dir);
	    strcpy (wi->w_Tmp, path);
	    AddPart (wi->w_Tmp, name, 254);

	    if ((count = stcgfn (wi->w_FR[FR_ICON]->rf_File, wi->w_Tmp)) > 0)
	    {
		strcpy (wi->iconname, wi->w_Tmp);
		if (SaveIcon (wi, wi->iconname, wi->diskobj, 0))
		{
		    /* Open the icon */
		    OpenNamedIcon (wi, wi->iconname, TRUE);

		    /* Clear the changes flags */
		    wi->changed = CH_NONE;
		}
	    }
	    else
	    {
		NotifyUser (MSG_IE_ERROR_NO_FILENAME, NULL);
	    }

	    FixFileAndPath (wi->w_FR[FR_ICON], wi->iconname);
	}
	else
	    wi->changed |= CH_CANCEL;

	/* Remember where they moved it */
	wi->LeftEdge = wi->w_FR[FR_ICON]->rf_LeftEdge - wp->LeftEdge;
	wi->TopEdge = wi->w_FR[FR_ICON]->rf_TopEdge - wp->TopEdge;
	wi->Width = wi->w_FR[FR_ICON]->rf_Width;
	wi->Height = wi->w_FR[FR_ICON]->rf_Height;
    }
    return (FALSE);
}


/*****************************************************************************/


BOOL SaveDefFunc (WindowInfoPtr wi)
{

    SaveIcon (wi, wi->iconname, wi->diskobj, 1);
    return (FALSE);
}


/*****************************************************************************/


BOOL QuitFunc (WindowInfoPtr wi)
{

    return (TRUE);
}


/*****************************************************************************/


BOOL ExchangeFunc (WindowInfoPtr wi)
{
    struct DynamicImage di;

    di.di_Flags = NULL;
    InitDynamicImage (&di, MAXDEPTH, ICON_WIDTH, ICON_HEIGHT);
    if (AllocDynamicImage (&di))
    {
	DrawImage (&di.di_RPort, &wi->images[0].di_Image, 0, 0);
	DrawImage (&wi->images[0].di_RPort, &wi->images[1].di_Image, 0, 0);
	DrawImage (&wi->images[1].di_RPort, &di.di_Image, 0, 0);
	FreeDynamicImage (&di);
	RefreshImages (wi);
	SPRefresh (wi->wi_sketch);

	wi->hilite = 2;
	SetHilite (wi, wi->diskobj, wi->hilite);
	CheckMenuItem (wi, 3, 2);

	wi->changed |= CH_MAJOR;
    }

    return (FALSE);
}


/*****************************************************************************/


BOOL CopyFunc (WindowInfoPtr wi)
{
    SHORT inactivewin;

    inactivewin = (wi->currentwin == 0) ? 1 : 0;
    DrawImage (&wi->images[inactivewin].di_RPort, &wi->images[wi->currentwin].di_Image, 0, 0);
    RefreshImages (wi);

    wi->hilite = 2;
    SetHilite (wi, wi->diskobj, wi->hilite);
    CheckMenuItem (wi, 3, 2);

    /* Indicate image editing change */
    wi->changed |= CH_MAJOR;

    return (FALSE);
}


/*****************************************************************************/


VOID LoadIAI (WindowInfoPtr wi, USHORT lmode, struct DiskObject * dob, USHORT cur)
{
    DynamicImagePtr di = &wi->images[cur];
    struct Image *im;

    if (lmode == 0)
	im = (struct Image *) dob->do_Gadget.GadgetRender;
    else
	im = (struct Image *) dob->do_Gadget.SelectRender;

    if (im)
    {
	SetRast (&di->di_RPort, 0);
	DrawImage (&di->di_RPort, im, 0, 0);
	RefreshImages (wi);

	SPRefresh (wi->wi_sketch);

	if (cur == 1)
	{
	    wi->hilite = 2;
	    SetHilite (wi, wi->diskobj, wi->hilite);
	    CheckMenuItem (wi, 3, 2);
	}

	wi->changed |= CH_MAJOR;
    }
    else
    {
	NotifyUser (MSG_IE_ERROR_NO_IMAGE, NULL);
    }
}


/*****************************************************************************/


VOID LoadIconAsImage (WindowInfoPtr wi, USHORT lmode)
{
    struct Window *wp;
    struct DiskObject *dob;
    UBYTE path[255], name[50];
    struct TagItem tg[10];

    wp = wi->win;
    if (wi->w_FR[FR_ALT])
    {
	/* Initialize the file requester position */
	if (wi->Width == (-1))
	{
	    wi->LeftEdge = wp->LeftEdge + 12;
	    wi->TopEdge  = wp->TopEdge + 12;
	    wi->Width    = 320;
	    wi->Height   = 175;
	}

	/* Set the file requester position */
	tg[4].ti_Tag = ASL_LeftEdge;
	tg[4].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
	tg[5].ti_Tag = ASL_TopEdge;
	tg[5].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
	tg[6].ti_Tag = ASL_Width;
	tg[6].ti_Data = (ULONG) wi->Width;
	tg[7].ti_Tag = ASL_Height;
	tg[7].ti_Data = (ULONG) wi->Height;

	tg[0].ti_Tag = ASL_Hail;
	tg[0].ti_Data = (ULONG) GetString (MSG_IE_LOADICON_TITLE);
	tg[2].ti_Tag = ASL_OKText;
	tg[2].ti_Data = (ULONG) GetString (MSG_IE_LOAD_GAD);
	tg[3].ti_Tag = ASL_Window;
	tg[3].ti_Data = (ULONG) wp;
	tg[1].ti_Tag = ASL_FuncFlags;
	tg[1].ti_Data = NULL;
	tg[8].ti_Tag = ASL_Pattern;
	tg[8].ti_Data = (ULONG) "#?.info";
	tg[9].ti_Tag = TAG_DONE;

	if (AslRequest (wi->w_FR[FR_ALT], tg))
	{
	    FixFileAndPath (wi->w_FR[FR_ALT], NULL);
	    strcpy (name, wi->w_FR[FR_ALT]->rf_File);
	    strcpy (path, wi->w_FR[FR_ALT]->rf_Dir);
	    strcpy (wi->w_Tmp, path);
	    AddPart (wi->w_Tmp, name, 254);
	    FixFileAndPath (wi->w_FR[FR_ALT], wi->w_Tmp);

	    if (dob = LoadIcon (wi->w_Tmp, TRUE))
	    {
		LoadIAI (wi, lmode, dob, wi->currentwin);
	    }
	    else
	    {
		NotifyUser (MSG_IE_ERROR_NOT_AN_ICON, NULL);
	    }

	    if (dob)
	    {
		FreeDiskObject (dob);
	    }

	    wi->changed |= CH_MAJOR;
	}

	/* Remember where they moved it */
	wi->LeftEdge = wi->w_FR[FR_ALT]->rf_LeftEdge - wp->LeftEdge;
	wi->TopEdge = wi->w_FR[FR_ALT]->rf_TopEdge - wp->TopEdge;
	wi->Width = wi->w_FR[FR_ALT]->rf_Width;
	wi->Height = wi->w_FR[FR_ALT]->rf_Height;
    }
}


/*****************************************************************************/


BOOL LStdFunc (WindowInfoPtr wi)
{

    LoadIconAsImage (wi, 0);
    return (FALSE);
}


/*****************************************************************************/


BOOL LAltFunc (WindowInfoPtr wi)
{

    LoadIconAsImage (wi, 1);
    return (FALSE);
}


/*****************************************************************************/


BOOL LBthFunc (WindowInfoPtr wi)
{
    struct Window *wp;
    SHORT curwin = wi->currentwin;
    SHORT i;
    struct DiskObject *dob;
    UBYTE path[255], name[50];
    struct TagItem tg[10];

    wp = wi->win;
    if (wi->w_FR[FR_ALT])
    {
	/* Initialize the file requester position */
	if (wi->Width == (-1))
	{
	    wi->LeftEdge = wp->LeftEdge + 12;
	    wi->TopEdge  = wp->TopEdge + 12;
	    wi->Width    = 320;
	    wi->Height   = 175;
	}

	/* Set the file requester position */
	tg[4].ti_Tag = ASL_LeftEdge;
	tg[4].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
	tg[5].ti_Tag = ASL_TopEdge;
	tg[5].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
	tg[6].ti_Tag = ASL_Width;
	tg[6].ti_Data = (ULONG) wi->Width;
	tg[7].ti_Tag = ASL_Height;
	tg[7].ti_Data = (ULONG) wi->Height;

	tg[0].ti_Tag = ASL_Hail;
	tg[0].ti_Data = (ULONG) GetString (MSG_IE_LOADICON_TITLE);
	tg[2].ti_Tag = ASL_OKText;
	tg[2].ti_Data = (ULONG) GetString (MSG_IE_LOAD_GAD);
	tg[3].ti_Tag = ASL_Window;
	tg[3].ti_Data = (ULONG) wp;
	tg[1].ti_Tag = ASL_FuncFlags;
	tg[1].ti_Data = NULL;
	tg[8].ti_Tag = ASL_Pattern;
	tg[8].ti_Data = (ULONG) "#?.info";
	tg[9].ti_Tag = TAG_DONE;

	if (AslRequest (wi->w_FR[FR_ALT], tg))
	{
	    FixFileAndPath (wi->w_FR[FR_ALT], NULL);
	    strcpy (name, wi->w_FR[FR_ALT]->rf_File);
	    strcpy (path, wi->w_FR[FR_ALT]->rf_Dir);
	    strcpy (wi->w_Tmp, path);
	    AddPart (wi->w_Tmp, name, 254);
	    FixFileAndPath (wi->w_FR[FR_ALT], wi->w_Tmp);

	    if (dob = LoadIcon (wi->w_Tmp, TRUE))
	    {
		for (i = 0; i < 2; i++)
		{
		    wi->currentwin = i;
		    wi->Adi = &wi->images[i];
		    SPSetRepeat (wi->wi_sketch, wi->Adi, (wi->w_LO + 77) + 16, wi->wintopx[i]);
		    LoadIAI (wi, i, dob, wi->currentwin);

		    wi->clippable = FALSE;
		    CheckClippable (wi, dob);
		}
	    }
	    else
	    {
		NotifyUser (MSG_IE_ERROR_NOT_AN_ICON, NULL);
	    }

	    if (dob)
		FreeDiskObject (dob);

	    wi->changed |= CH_MAJOR;
	}

	/* Remember where they moved it */
	wi->LeftEdge = wi->w_FR[FR_ALT]->rf_LeftEdge - wp->LeftEdge;
	wi->TopEdge = wi->w_FR[FR_ALT]->rf_TopEdge - wp->TopEdge;
	wi->Width = wi->w_FR[FR_ALT]->rf_Width;
	wi->Height = wi->w_FR[FR_ALT]->rf_Height;
	wi->currentwin = curwin;
	wi->Adi = &wi->images[wi->currentwin];
	SPSetRepeat (wi->wi_sketch, wi->Adi, (wi->w_LO + 77) + 16, wi->wintopx[wi->currentwin]);
	SPRefresh (wi->wi_sketch);
    }
    return (FALSE);
}

/*****************************************************************************/

BOOL LIFFFunc (WindowInfoPtr wi)
{
    struct Window *wp;
    UBYTE path[255], name[50];
    struct TagItem tg[10];

    wp = wi->win;
    if (wi->w_FR[FR_ILBM])
    {
	/* Initialize the file requester position */
	if (wi->Width == (-1))
	{
	    wi->LeftEdge = wp->LeftEdge + 12;
	    wi->TopEdge  = wp->TopEdge + 12;
	    wi->Width    = 320;
	    wi->Height   = 175;
	}

	/* Set the file requester position */
	tg[4].ti_Tag = ASL_LeftEdge;
	tg[4].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
	tg[5].ti_Tag = ASL_TopEdge;
	tg[5].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
	tg[6].ti_Tag = ASL_Width;
	tg[6].ti_Data = (ULONG) wi->Width;
	tg[7].ti_Tag = ASL_Height;
	tg[7].ti_Data = (ULONG) wi->Height;

	tg[0].ti_Tag = ASL_Hail;
	tg[0].ti_Data = (ULONG) GetString (MSG_IE_LOADBRUSH_TITLE);
	tg[2].ti_Tag = ASL_OKText;
	tg[2].ti_Data = (ULONG) GetString (MSG_IE_LOAD_GAD);
	tg[3].ti_Tag = ASL_Window;
	tg[3].ti_Data = (ULONG) wp;
	tg[1].ti_Tag = ASL_FuncFlags;
	tg[1].ti_Data = NULL;
	tg[8].ti_Tag = ASL_Pattern;
	tg[8].ti_Data = (ULONG) "~(#?.info)";
	tg[9].ti_Tag = TAG_DONE;

	if (AslRequest (wi->w_FR[FR_ILBM], tg))
	{
	    FixFileAndPath (wi->w_FR[FR_ILBM], NULL);
	    strcpy (name, wi->w_FR[FR_ILBM]->rf_File);
	    strcpy (path, wi->w_FR[FR_ILBM]->rf_Dir);
	    strcpy (wi->w_Tmp, path);
	    AddPart (wi->w_Tmp, name, 254);
	    FixFileAndPath (wi->w_FR[FR_ILBM], wi->w_Tmp);

	    LoadIFFImage (wi, wi->w_Tmp, wi->currentwin, TRUE);

	    wi->changed |= CH_MAJOR;
	}

	/* Remember where they moved it */
	wi->LeftEdge = wi->w_FR[FR_ILBM]->rf_LeftEdge - wp->LeftEdge;
	wi->TopEdge = wi->w_FR[FR_ILBM]->rf_TopEdge - wp->TopEdge;
	wi->Width = wi->w_FR[FR_ILBM]->rf_Width;
	wi->Height = wi->w_FR[FR_ILBM]->rf_Height;
    }
    return (FALSE);
}


/*****************************************************************************/


BOOL SStdFunc (WindowInfoPtr wi)
{

    return (FALSE);
}


/*****************************************************************************/


BOOL SIFFFunc (WindowInfoPtr wi)
{
    struct Window *wp;
    DynamicImagePtr di = &wi->images[wi->currentwin];
    UBYTE path[255], name[50];
    struct TagItem tg[10];

    wp = wi->win;
    if (wi->w_FR[FR_ILBM])
    {
	/* Initialize the file requester position */
	if (wi->Width == (-1))
	{
	    wi->LeftEdge = wp->LeftEdge + 12;
	    wi->TopEdge  = wp->TopEdge + 12;
	    wi->Width    = 320;
	    wi->Height   = 175;
	}

	/* Set the file requester position */
	tg[4].ti_Tag = ASL_LeftEdge;
	tg[4].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
	tg[5].ti_Tag = ASL_TopEdge;
	tg[5].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
	tg[6].ti_Tag = ASL_Width;
	tg[6].ti_Data = (ULONG) wi->Width;
	tg[7].ti_Tag = ASL_Height;
	tg[7].ti_Data = (ULONG) wi->Height;

	tg[0].ti_Tag = ASL_Hail;
	tg[0].ti_Data = (ULONG) GetString (MSG_IE_SAVEBRUSH_TITLE);
	tg[1].ti_Tag = ASL_FuncFlags;
	tg[1].ti_Data = FILF_SAVE;
	tg[2].ti_Tag = ASL_OKText;
	tg[2].ti_Data = (ULONG) GetString (MSG_IE_SAVE_GAD);
	tg[3].ti_Tag = ASL_Window;
	tg[3].ti_Data = (ULONG) wp;
	tg[8].ti_Tag = ASL_Pattern;
	tg[8].ti_Data = (ULONG) "~(#?.info)";
	tg[9].ti_Tag = TAG_DONE;

	if (AslRequest (wi->w_FR[FR_ILBM], tg))
	{
	    FixFileAndPath (wi->w_FR[FR_ILBM], NULL);
	    strcpy (name, wi->w_FR[FR_ILBM]->rf_File);
	    strcpy (path, wi->w_FR[FR_ILBM]->rf_Dir);
	    strcpy (wi->w_Tmp, path);
	    AddPart (wi->w_Tmp, name, 254);
	    FixFileAndPath (wi->w_FR[FR_ILBM], wi->w_Tmp);

	    SaveIFFBrush (wi, wi->w_Tmp, di);
	}

	/* Remember where they moved it */
	wi->LeftEdge = wi->w_FR[FR_ILBM]->rf_LeftEdge - wp->LeftEdge;
	wi->TopEdge = wi->w_FR[FR_ILBM]->rf_TopEdge - wp->TopEdge;
	wi->Width = wi->w_FR[FR_ILBM]->rf_Width;
	wi->Height = wi->w_FR[FR_ILBM]->rf_Height;
    }
    return (FALSE);
}


/*****************************************************************************/


BOOL RestoreFunc (WindowInfoPtr wi)
{
    SHORT i;
    struct Image *im;

    for (i = 0; i < 2; i++)
    {
	SetRast (&wi->images[i].di_RPort, 0);

	if (i == 0)
	{
	    im = (struct Image *) wi->diskobj->do_Gadget.GadgetRender;
	}
	else
	{
	    im = (struct Image *) wi->diskobj->do_Gadget.SelectRender;
	}

	DrawImage (&wi->images[i].di_RPort, im, 0, 0);
    }

    RefreshImages (wi);
    SPRefresh (wi->wi_sketch);

    return (FALSE);
}


/*****************************************************************************/


VOID AutoTopLeft (DynamicImagePtr si)
{
    struct RastPort *prp = &(si->di_RPort);
    struct RastPort rp;
    SHORT w = si->di_Image.Width;
    SHORT h = si->di_Image.Height;
    SHORT x, y, mx = w, my = h, p;

    /* clone the rastport */
    rp = *prp;
    SetAPen (&rp, 0);
    SetBPen (&rp, 0);
    SetDrMd (&rp, JAM1);

    for (x = 0; x < w; x++)
    {
	for (y = 0; y < h; y++)
	{
	    p = ReadPixel (&rp, x, y);
	    if (p > 0)
	    {
		mx = MIN (x, mx);
		my = MIN (y, my);
	    }
	}
    }

    x = mx;
    y = my;
    mx = w - x;
    my = h - y;
    ClipBlit (&rp, x, y,
	      &rp, 0, 0,
	      mx, my, VANILLA_COPY);

    if (mx < ICON_WIDTH)
	RectFill (&rp, mx, 0, ICON_WIDTH, ICON_HEIGHT);

    if (my < ICON_HEIGHT)
	RectFill (&rp, 0, my, ICON_WIDTH, ICON_HEIGHT);
}

/*****************************************************************************/

BOOL CheckForChanges (WindowInfoPtr wi, AppStringsID body, AppStringsID gadgets)
{
    UBYTE pname[255];
    ULONG len;

    if ((len = strlen (wi->iconname)) < 1)
    {
	strcpy (pname, GetString (MSG_IE_UNTITLED));
    }
    else if (len < 80)
    {
	strcpy (pname, wi->iconname);
    }
    else
    {
	ULONG i, j, k;

	/* Get the volume portion */
	for (i = k = 0; i < 40; i++)
	{
	    if (wi->iconname[i] == ':')
	    {
		k = i;
		break;
	    }
	}

	/* Find a good splitting place */
	k += 34;
	while (k > 0)
	{
	    if (wi->iconname[k] == '/')
		break;
	    k--;
	}

	/* Build the first part of the name */
	for (i = j = 0; i <= k; i++)
	    pname[j++] = wi->iconname[i];

	/* Do the elipses portion */
	pname[j++] = '.';
	pname[j++] = '.';
	pname[j++] = '.';

	/* Come up with the remainder of the name */
	k = (k < 40) ? 80 - k : 40;
	for (i = len - k; i < len; i++)
	{
	    if (wi->iconname[i] == '/')
		break;
	}

	/* Copy the remainder of the name */
	for (; i < len; i++)
	    pname[j++] = wi->iconname[i];
	pname[j] = 0;
    }

    if (wi->changed)
    {
	switch (EasyReq (wi->win, VNAM, GetString (body), GetString (gadgets), pname))
	{
	    case 2:
		/* Clear the cancel flag */
		wi->changed &= ~CH_CANCEL;

		/* Save the icon */
		SaveFunc (wi);

		/* See if we were canceled */
		if (wi->changed & CH_CANCEL)
		{
		    /* Clear the cancel flag */
		    wi->changed &= ~CH_CANCEL;

		    /* Return cancel */
		    return TRUE;
		}
		break;

	    case 1:
		break;		/* do action */

	    case 0:
		return (TRUE);	/* cancel */
		break;
	}
    }
    return (FALSE);
}


/*****************************************************************************/


/* Find the upper left corner of a big picture */
VOID FindTopLeft (struct RastPort * rp, struct Rectangle * clip, SHORT mw, SHORT mh)
{
    SHORT w = clip->MaxX;
    SHORT h = clip->MaxY;
    SHORT x, y, mx = w, my = h, p;

    if ((w > mw) || (h > mh))
    {
	w--;
	h--;
	for (x = 0; (x < w); x++)
	{
	    for (y = 0; (y < h); y++)
	    {
		p = ReadPixel (rp, x, y);
		if (p > 0)
		{
		    mx = MIN (x, mx);
		    my = MIN (y, my);
		}
	    }
	}
    }
    else
    {
	mx = 0;
	my = 0;
    }
    clip->MinX = mx;
    clip->MinY = my;
    clip->MaxX = MIN (clip->MaxX, (clip->MinX + mw));
    clip->MaxY = MIN (clip->MaxY, (clip->MinY + mh));
}


/*****************************************************************************/

#if 0
#define	MAXF	8191
#define	DESTX	80
#define	DESTY	40

/* Scale brush to maximum allowable size for an icon */
BOOL ScaleImage (ILBMPtr ir, struct BitMap * bm)
{
    struct BitScaleArgs bsa =
    {NULL};

    if (ir && bm)
    {
	bsa.bsa_SrcX = 0;
	bsa.bsa_SrcY = 0;
	bsa.bsa_SrcWidth = ir->ir_Width;
	bsa.bsa_SrcHeight = ir->ir_Height;
	bsa.bsa_XSrcFactor = MAXF;
	bsa.bsa_YSrcFactor = MAXF;
	bsa.bsa_DestX = 0;
	bsa.bsa_DestY = 0;
	bsa.bsa_XDestFactor = (MAXF / ir->ir_Width) * DESTX;
	bsa.bsa_YDestFactor = (MAXF / ir->ir_Height) * DESTY;
	bsa.bsa_SrcBitMap = &(ir->ir_BMap);
	bsa.bsa_DestBitMap = bm;

	BitMapScale (&bsa);
    }
    return (TRUE);
}

#endif

/*****************************************************************************/

BOOL SaveAsCFunc (WindowInfoPtr wi)
{
    struct Window *wp;
    UBYTE path[255], name[50];
    struct TagItem tg[10];

    wp = wi->win;

    if (wi->w_FR[FR_ICON])
    {
	/* Initialize the file requester position */
	if (wi->Width == (-1))
	{
	    wi->LeftEdge = wp->LeftEdge + 12;
	    wi->TopEdge  = wp->TopEdge + 12;
	    wi->Width    = 320;
	    wi->Height   = 175;
	}

	/* Set the file requester position */
	tg[4].ti_Tag = ASL_LeftEdge;
	tg[4].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
	tg[5].ti_Tag = ASL_TopEdge;
	tg[5].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
	tg[6].ti_Tag = ASL_Width;
	tg[6].ti_Data = (ULONG) wi->Width;
	tg[7].ti_Tag = ASL_Height;
	tg[7].ti_Data = (ULONG) wi->Height;

	tg[0].ti_Tag = ASL_Hail;
	tg[0].ti_Data = (ULONG) GetString (MSG_IE_SAVEFILE_TITLE);
	tg[1].ti_Tag = ASL_FuncFlags;
	tg[1].ti_Data = FILF_SAVE;
	tg[2].ti_Tag = ASL_OKText;
	tg[2].ti_Data = (ULONG) GetString (MSG_IE_SAVE_GAD);
	tg[3].ti_Tag = ASL_Window;
	tg[3].ti_Data = (ULONG) wp;
	tg[8].ti_Tag = ASL_Pattern;
	tg[8].ti_Data = (ULONG) "~(#?.info)";
	tg[9].ti_Tag = TAG_DONE;

	if (AslRequest (wi->w_FR[FR_C], tg))
	{
	    FixFileAndPath (wi->w_FR[FR_C], NULL);
	    strcpy (name, wi->w_FR[FR_C]->rf_File);
	    strcpy (path, wi->w_FR[FR_C]->rf_Dir);
	    strcpy (wi->w_Tmp, path);
	    AddPart (wi->w_Tmp, name, 254);
	    FixFileAndPath (wi->w_FR[FR_C], wi->w_Tmp);

	    /* Save as C source */
	    SaveIcon (wi, wi->w_Tmp, wi->diskobj, 2);
	}

	/* Remember where they moved it */
	wi->LeftEdge = wi->w_FR[FR_C]->rf_LeftEdge - wp->LeftEdge;
	wi->TopEdge = wi->w_FR[FR_C]->rf_TopEdge - wp->TopEdge;
	wi->Width = wi->w_FR[FR_C]->rf_Width;
	wi->Height = wi->w_FR[FR_C]->rf_Height;
    }

    return (FALSE);
}


/*****************************************************************************/


BOOL EraseFunc (WindowInfoPtr wi)
{
    DynamicImagePtr di = &wi->images[wi->currentwin];

    if (!CheckForChanges (wi, MSG_IE_ERASEANDZAP, MSG_IE_ERASEANDZAP_GADS))
    {
	SPSaveToUndo (wi->wi_sketch);

	SetRast (&(di->di_RPort), 0);
	RefreshImages (wi);
	SPRefresh (wi->wi_sketch);
	wi->clippable = FALSE;
    }
    return (FALSE);
}


/*****************************************************************************/


BOOL TemplateFunc (WindowInfoPtr wi)
{
    DynamicImagePtr di = &wi->images[wi->currentwin];
    extern struct Image template_data;

    if (!CheckForChanges (wi, MSG_IE_TEMPLATEANDZAP, MSG_IE_TEMPLATEANDZAP_GADS))
    {
	SPSaveToUndo (wi->wi_sketch);

	/* Clear the work area */
	SetRast (&(di->di_RPort), 0);

	/* Draw the template */
	DrawImage (&(di->di_RPort), &template_data, 0, 0);

	/* Refresh the view */
	RefreshImages (wi);
	SPRefresh (wi->wi_sketch);

	wi->changed |= CH_MAJOR;
	wi->clippable = FALSE;
    }

    return (FALSE);
}


/*****************************************************************************/


void Recolor (struct Image * im)
{
    USHORT *data;
    LONG words, offset;
    LONG count;
    USHORT bits;
    USHORT plane;
    USHORT mask;
    UBYTE value;

    if (im)
	if (im->Depth > 0)
	{
	    words = ((im->Width + 15) >> 4) * im->Height;
	    data = im->ImageData;

	    for (count = words; count > 0; count--)
	    {
		bits = 16;
		while (bits--)
		{
		    mask = 1L << bits;
		    value = 0;
		    offset = 0;

		    for (plane = 0; plane < im->Depth; plane++)
		    {
			if (data[offset] & mask)
			{
			    value |= (1 << plane);
			}
			offset += words;
		    }

		    if (value == 2)
		    {
			data[0] |= mask;
			data[words] &= ~mask;
		    }
		    else if (value == 1)
		    {
			data[0] &= ~mask;
			data[words] |= mask;
		    }
		}
		data++;
	    }
	}
}


/*****************************************************************************/


BOOL RecolorFunc (WindowInfoPtr wi)
{

    SPSaveToUndo (wi->wi_sketch);

    /* Do the normal image */
    Recolor (&(wi->images[0].di_Image));
    Recolor (wi->diskobj->do_Gadget.GadgetRender);

    /* Do the alternate image */
    Recolor (&(wi->images[1].di_Image));
    Recolor (wi->diskobj->do_Gadget.SelectRender);

    /* Refresh the screen */
    RefreshImages (wi);

    /* Show that the image has been changed */
    wi->changed |= CH_REMAP;

    /* Refresh the view */
    SPRefresh (wi->wi_sketch);

    return (FALSE);
}


/*****************************************************************************/


BOOL TopLeftFunc (WindowInfoPtr wi)
{

    SPSaveToUndo (wi->wi_sketch);

    AutoTopLeft (&(wi->images[wi->currentwin]));
    RefreshImages (wi);

    /* Major image change */
    wi->changed |= CH_MAJOR;

    /* Refresh the view */
    SPRefresh (wi->wi_sketch);

    return (FALSE);
}


/*****************************************************************************/


BOOL PaletteFunc (WindowInfoPtr wi)
{

    System (wi->w_Palette, NULL);
    return (FALSE);
}
