
/* include */
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>
#include <clib/wb_protos.h>
#include <clib/utility_protos.h>

/* direct ROM interface */
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/wb_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>

/* application includes */
#include "iemain.h"
#include "ieio.h"
#include "iemisc.h"
#include "iemenus.h"
#include "iemessages.h"
#include "iegads.h"
#include "texttable.h"
#include "ieutils.h"
#include "sp_tools.h"

/*****************************************************************************/

#include "iconedit_rev.h"
char *version = VERSTAG;

/*****************************************************************************/

#define	HeadNode(list) ((list)->lh_Head)
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))

void exit (int);

/*****************************************************************************/

extern struct Library *DOSBase;
extern struct Library *SysBase;

/*****************************************************************************/

struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *IconBase;
struct Library *LayersBase;
struct Library *GadToolsBase;
struct Library *WorkbenchBase;
struct Library *AslBase;
struct Library *IFFParseBase;
struct Library *UtilityBase;
struct Library *DataTypesBase;

/*****************************************************************************/

struct NewWindow NW =
{
    0, 11,			/* LeftEdge, TopEdge */
    524, 166,			/* Width, Height */
    -1, -1,			/* DetailPen, BlockPen */
    IDCMPS,
    ACTIVATE | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE |
    SMART_REFRESH,		/* Flags */
    NULL,			/* FirstGadget */
    NULL,			/* CheckMark */
    NULL,			/* Title */
    NULL,			/* Screen */
    NULL,			/* BitMap */
    225, 11,			/* MinWidth, MinHeight */
    552, 170,			/* MaxWidth, MaxHeight */
    WBENCHSCREEN,		/* Type */
};

/*****************************************************************************/

struct Node *MakeTT (WindowInfoPtr wi, struct List *l, VOID * fmt, ULONG data)
{
    struct Node *n;
    LONG msize;

    /* Build the tool type */
    sprintf (wi->w_Tmp, fmt, data);

    /* How much room do we need */
    msize = sizeof (struct Node) + strlen (wi->w_Tmp) + 1L;

    /* Allocate a node to hold the tool type */
    if (n = AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* Copy the tool type into the node */
	n->ln_Name = MEMORY_FOLLOWING (n);
	strcpy (n->ln_Name, wi->w_Tmp);

	/* Add the tool type to the end of the list */
	AddTail (l, n);
    }

    return (n);
}

/*****************************************************************************/

VOID SetIconEditPrefs (WindowInfoPtr wi)
{
    BPTR lock;
    struct Node *node;

    /* Do we have a home directory? */
    if (GetProgramDir () && wi->wi_ToolDOB)
    {
	char **otooltypes, **tooltypes;
	LONG newcount, msize;
	struct List list;

	/* Initialize the tool type list */
	NewList (&list);

	/* Lock the directory */
	lock = CurrentDir (GetProgramDir ());

	/* Make the new tool types */
	MakeTT (wi, &list, "CLIPUNIT=%ld", wi->w_ClipUnit);
	MakeTT (wi, &list, "XMAG=%ld", wi->w_MagX);
	MakeTT (wi, &list, "YMAG=%ld", wi->w_MagY);
	MakeTT (wi, &list, "LEFTEDGE=%ld", wi->win->LeftEdge);
	MakeTT (wi, &list, "TOPEDGE=%ld", wi->win->TopEdge);

	if (wi->Width != (-1))
	{
	    MakeTT (wi, &list, "FRLEFTEDGE=%ld", wi->LeftEdge);
	    MakeTT (wi, &list, "FRTOPEDGE=%ld", wi->TopEdge);
	    MakeTT (wi, &list, "FRWIDTH=%ld", wi->Width);
	    MakeTT (wi, &list, "FRHEIGHT=%ld", wi->Height);
	}

	MakeTT (wi, &list, "PALETTE=%s", (ULONG) wi->w_Palette);
	MakeTT (wi, &list, "SHOWCLIP=%s", (ULONG) wi->w_ShowClip);

	if (strlen (wi->w_FR[FR_ICON]->rf_Dir) > 0)
	{
	    MakeTT (wi, &list, "ICONDRAWER=%s", (ULONG) wi->w_FR[FR_ICON]->rf_Dir);
	}

	if (strlen (wi->w_FR[FR_ILBM]->rf_Dir) > 0)
	{
	    MakeTT (wi, &list, "ILBMDRAWER=%s", (ULONG) wi->w_FR[FR_ILBM]->rf_Dir);
	}

	if (strlen (wi->w_FR[FR_CLIP]->rf_Dir) > 0)
	{
	    MakeTT (wi, &list, "CLIPDRAWER=%s", (ULONG) wi->w_FR[FR_CLIP]->rf_Dir);
	}

	if (strlen (wi->w_FR[FR_ALT]->rf_Dir) > 0)
	{
	    MakeTT (wi, &list, "ALTDRAWER=%s", (ULONG) wi->w_FR[FR_ALT]->rf_Dir);
	}

	if (strlen (wi->w_FR[FR_C]->rf_Dir) > 0)
	{
	    MakeTT (wi, &list, "SRCDRAWER=%s", (ULONG) wi->w_FR[FR_C]->rf_Dir);
	}

	if (!wi->w_SaveIcons)
	{
	    MakeTT (wi, &list, "NOICONS", NULL);
	}

	if (!wi->w_UseGrid)
	{
	    MakeTT (wi, &list, "NOGRID", NULL);
	}

	if (wi->w_SaveSrc)
	{
	    MakeTT (wi, &list, "SRC", NULL);
	}

	if (!wi->w_RemapImage)
	    MakeTT (wi, &list, "NOREMAP", NULL);

	/* Count the number of new ToolTypes: */
	newcount = 0;
	for (node = HeadNode (&list); node->ln_Succ; node = node->ln_Succ)
	{
	    newcount++;
	}

	/* How much memory do we need */
	msize = (newcount + 1) * sizeof (char *);

	/* Allocate the new tool types */
	if (tooltypes = AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
	{
	    LONG new;

	    /* Initialize the tooltypes */
	    for (node = HeadNode (&list), new = 0; node->ln_Succ; node = node->ln_Succ, new++)
		tooltypes[new] = node->ln_Name;

	    /* Remember the old tool types */
	    otooltypes = wi->wi_ToolDOB->do_ToolTypes;

	    /* Set the new tool types */
	    wi->wi_ToolDOB->do_ToolTypes = tooltypes;

	    /* Save the icon */
	    PutDiskObject (VNAM, wi->wi_ToolDOB);

	    /* Restore the old tool types */
	    wi->wi_ToolDOB->do_ToolTypes = otooltypes;

	    /* Free the tool type buffer */
	    FreeVec (tooltypes);
	}

	while (node = RemHead (&list))
	    FreeVec (node);

	CurrentDir (lock);
    }
}

/*****************************************************************************/

VOID GetNum (APTR tooltypes, STRPTR name, UWORD * value)
{
    STRPTR tmp;
    LONG num;

    if (tmp = FindToolType (tooltypes, name))
    {
	if (StrToLong (tmp, &num) > 0)
	{
	    *value = (UWORD) num;
	}
    }
}

/*****************************************************************************/

VOID GetStr (APTR tooltypes, STRPTR name, STRPTR result)
{
    STRPTR tmp;

    if (tmp = FindToolType (tooltypes, name))
    {
	strcpy (result, tmp);
    }
}

/*****************************************************************************/

VOID GetIconEditPrefs (WindowInfoPtr wi)
{
    BPTR lock;

    wi->w_MagX = 4;
    wi->w_MagY = 4;
    wi->w_SaveIcons = TRUE;
    wi->w_UseGrid = TRUE;
    wi->w_RemapImage = TRUE;
    wi->Width = (-1);

    strcpy (wi->w_Palette, "SYS:Prefs/Palette");
    strcpy (wi->w_ShowClip, "SYS:Utilities/MultiView CLIPBOARD");

    /* Do we have a home directory? */
    if (GetProgramDir ())
    {
	lock = CurrentDir (GetProgramDir ());

	/* Get the icon for the editor */
	if (wi->wi_ToolDOB = GetDiskObject (VNAM))
	{
	    APTR ttypes;

	    ttypes = wi->wi_ToolDOB->do_ToolTypes;

	    GetNum (ttypes, "CLIPUNIT", &wi->w_ClipUnit);
	    GetNum (ttypes, "XMAG", &wi->w_MagX);
	    GetNum (ttypes, "YMAG", &wi->w_MagY);
	    GetNum (ttypes, "LEFTEDGE", &NW.LeftEdge);
	    GetNum (ttypes, "TOPEDGE", &NW.TopEdge);
	    GetNum (ttypes, "FRLEFTEDGE", &wi->LeftEdge);
	    GetNum (ttypes, "FRTOPEDGE", &wi->TopEdge);
	    GetNum (ttypes, "FRWIDTH", &wi->Width);
	    GetNum (ttypes, "FRHEIGHT", &wi->Height);
	    GetStr (ttypes, "PALETTE", wi->w_Palette);
	    GetStr (ttypes, "SHOWCLIP", wi->w_ShowClip);
	    GetStr (ttypes, "ICONDRAWER", wi->w_FR[FR_ICON]->rf_Dir);
	    GetStr (ttypes, "ILBMDRAWER", wi->w_FR[FR_ILBM]->rf_Dir);
	    GetStr (ttypes, "SRCDRAWER", wi->w_FR[FR_C]->rf_Dir);

	    strcpy (wi->w_FR[FR_CLIP]->rf_Dir, wi->w_FR[FR_ILBM]->rf_Dir);
	    strcpy (wi->w_FR[FR_ALT]->rf_Dir, wi->w_FR[FR_ICON]->rf_Dir);

	    GetStr (ttypes, "CLIPDRAWER", wi->w_FR[FR_CLIP]->rf_Dir);
	    GetStr (ttypes, "ALTDRAWER", wi->w_FR[FR_ALT]->rf_Dir);

	    if (FindToolType (ttypes, "NOICONS"))
		wi->w_SaveIcons = FALSE;

	    if (FindToolType (ttypes, "NOGRID"))
		wi->w_UseGrid = FALSE;

	    if (FindToolType (ttypes, "SRC"))
		wi->w_SaveSrc = TRUE;

	    if (FindToolType (ttypes, "NOREMAP"))
		wi->w_RemapImage = FALSE;
	}

	CurrentDir (lock);
    }

    if (wi->w_ClipUnit > 255)
	wi->w_ClipUnit = 255;

    if (wi->w_MagY < 4)
	wi->w_MagY = 4;

    if (wi->w_MagY > 16)
	wi->w_MagY = 16;

    if (wi->w_MagX < 4)
	wi->w_MagX = 4;

    if (wi->w_MagX > 16)
	wi->w_MagX = 16;
}

/*****************************************************************************/

VOID IECloseLibrary (APTR lib)
{
    if (lib)
	CloseLibrary (lib);
}

/*****************************************************************************/

VOID CloseAll (struct WindowInfo *wi, AppStringsID code)
{
    WORD i;

    if (code != MSG_IE_ERROR_WB2_0)
	NotifyUser (code, NULL);

    if (wi)
    {
	if (wi->win)
	{
	    if (wi->aw)
		RemoveAppWindow (wi->aw);

	    ClearMenuStrip (wi->win);
	    CloseWindow (wi->win);
	}

	CurrentDir (wi->wi_OriginalCD);
	UnLock (wi->wi_CurrentCD);

	for (i = 0; i < FR_MAX; i++)
	    if (wi->w_FR[i])
		FreeFileRequest (wi->w_FR[i]);

	DeleteMsgPort (wi->msgport);
	FreeMenus (wi->menu);
	FreeGadgets (wi->firstgad);

	if (wi->diskobj)
	    FreeDiskObject (wi->diskobj);

	if (wi->wi_ToolDOB)
	    FreeDiskObject (wi->wi_ToolDOB);

	if (wi->font)
	    CloseFont (wi->font);

	CloseSketchPad (wi->wi_sketch);
	FreeDynamicImage (&wi->images[0]);
	FreeDynamicImage (&wi->images[1]);
	FreeVisualInfo (wi->vi);
	FreeScreenDrawInfo (wi->mysc, wi->w_DrInfo);
	UnlockPubScreen (NULL, wi->mysc);
	FreeVec (wi);
    }

    IECloseLibrary (DataTypesBase);
    IECloseLibrary (IFFParseBase);
    IECloseLibrary (AslBase);
    IECloseLibrary (WorkbenchBase);
    IECloseLibrary (GadToolsBase);
    IECloseLibrary (LayersBase);
    IECloseLibrary (IconBase);
    IECloseLibrary (GfxBase);
    IECloseLibrary (IntuitionBase);
    IECloseLibrary (UtilityBase);

    CloseCat ();

    exit (0);
}

/*****************************************************************************/

void main (int argc, char *argv[])
{
    struct Window *wp;
    WindowInfoPtr wi;
    WORD i, j, k;
    struct ZoomBox ZB;

    OpenCat ();

    if (!(IntuitionBase = OpenLibrary ("intuition.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_WB2_0);

    if (!(GfxBase = OpenLibrary ("graphics.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_WB2_0);

    if (!(LayersBase = OpenLibrary ("layers.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_WB2_0);

    if (!(GadToolsBase = OpenLibrary ("gadtools.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_WB2_0);

    if (!(UtilityBase = OpenLibrary ("utility.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_WB2_0);

    if (!(IconBase = OpenLibrary ("icon.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_ICON_LIBRARY);

    if (!(WorkbenchBase = OpenLibrary ("workbench.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_WORKBENCH_LIBRARY);

    if (!(AslBase = OpenLibrary ("asl.library", 38)))
	CloseAll (NULL, MSG_IE_ERROR_ASL_LIBRARY);

    if (!(IFFParseBase = OpenLibrary ("iffparse.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_IFFPARSE_LIBRARY);

    if (!(DataTypesBase = OpenLibrary ("datatypes.library", 39)))
	CloseAll (NULL, MSG_IE_ERROR_DATATYPES_LIBRARY);

    if (!(wi = AllocVec (sizeof (struct WindowInfo), MEMF_CLEAR | MEMF_PUBLIC)))
	 CloseAll (NULL, MSG_IE_ERROR_NO_FREE_STORE);

    wi->clippable = FALSE;
    wi->wi_OriginalCD = ((struct Process *) FindTask (NULL))->pr_CurrentDir;
    wi->mydesiredfont.ta_Name = "topaz.font";
    wi->mydesiredfont.ta_YSize = 8;
    wi->mydesiredfont.ta_Style = 0;
    wi->mydesiredfont.ta_Flags = 0;

    wi->images[0].di_Flags = DI_FILL | DI_LAYER;
    wi->images[1].di_Flags = DI_FILL | DI_LAYER;
    InitDynamicImage (&wi->images[0], MAXDEPTH, ICON_WIDTH, ICON_HEIGHT);
    InitDynamicImage (&wi->images[1], MAXDEPTH, ICON_WIDTH, ICON_HEIGHT);

    for (i = 0; i < FR_MAX; i++)
    {
	if (!(wi->w_FR[i] = AllocAslRequest (ASL_FileRequest, NULL)))
	{
	    CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);
	}
    }

    /* What screen are we going to pop up on? */
    if (!(wi->mysc = LockPubScreen (NULL)))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    j = 1 << wi->mysc->BitMap.Depth;
    for (i = 0; i < 4; i++)
	wi->w_ColorTable[i] = i;
    for (i = 1, k = 7; i < 5; i++, k--)
	wi->w_ColorTable[k] = j - i;

    /* Get the visual information */
    if (!(wi->w_DrInfo = GetScreenDrawInfo (wi->mysc)))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    /* Get the visual information */
    if (!(wi->vi = GetVisualInfoA (wi->mysc, NULL)))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    if (!(wi->font = OpenFont (&wi->mydesiredfont)))
	CloseAll (wi, MSG_IE_ERROR_TOPAZ_FONT);

    if (!AllocDynamicImage (&wi->images[0]))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    if (!AllocDynamicImage (&wi->images[1]))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    /* Get the topborder & set the zoom */
    wi->topborder = wi->mysc->WBorTop + (wi->mysc->Font->ta_YSize + 1);
    NW.TopEdge = wi->topborder;

    /* Get the preferences */
    GetIconEditPrefs (wi);

    /* Adjust for virtual screens */
    if (wi->mysc)
    {
	/* Set the window coordinates */
	NW.LeftEdge += -(wi->mysc->ViewPort.DxOffset);
	NW.TopEdge += -(wi->mysc->ViewPort.DyOffset);
    }

    /* Set the zoom */
    ZB.LeftEdge = NW.LeftEdge;
    ZB.TopEdge = NW.TopEdge;
    ZB.Width = 225;
    ZB.Height = wi->topborder;

    /* Calculate the window size */
    NW.Width += ((ICON_WIDTH * wi->w_MagX) - (ICON_WIDTH * 4));
    NW.Height += ((ICON_HEIGHT * wi->w_MagY) - (ICON_HEIGHT * 4));

    if (!(wp = OpenWindowTags (&NW,
			       WA_InnerHeight, NW.Height,
			       WA_InnerWidth, NW.Width,
			       WA_PubScreen, wi->mysc,
			       WA_AutoAdjust, TRUE,
			       WA_Zoom, &ZB,
			       WA_NewLookMenus, TRUE,
			       TAG_DONE)))
    {
	CloseAll (wi, MSG_IE_ERROR_WINDOW);
    }

    SetBusyPointer (wp);
    wi->win = wp;
    wp->UserData = (BYTE *) wi;
    SetFont (wp->RPort, wi->font);

    /* Figure out the top */
    wi->wintopx[0] = wi->topborder + IMG1TOP;
    wi->wintopx[1] = wi->topborder + IMG2TOP;

    /* Get the depth */
    wi->w_Depth = MIN (wp->WScreen->RastPort.BitMap->Depth, MAXDEPTH);

    /* Set the window title */
    strcpy (wi->wintitle, GetString (MSG_IE_DEFAULT_TITLE));
    SetWindowTitles (wp, wi->wintitle, (STRPTR) - 1);

    /* Prepare the menus */
    if (!CreateIconMenus (wi))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    /* Prepare the gadgets */
    if (!CreateIconGadgets (wi))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    /* Create message port for AppWindow */
    if (!(wi->msgport = CreateMsgPort ()))
	CloseAll (wi, MSG_IE_ERROR_NO_FREE_STORE);

    /* Start the AppWindow */
    wi->aw = AddAppWindow (0, 0, wp, wi->msgport, NULL);

    /* Refresh all the custom imagery: */
    RefreshCustomImagery (wi);
    RefreshGList (wi->firstgad, wp, NULL, ((USHORT) - 1));
    GT_RefreshWindow (wp, NULL);
    SPSelectToolGad (wi->wi_sketch, 0);

/*
    /* Check the arguments */
    if (argc == 0)
    {
	struct WBStartup *wb = (struct WBStartup *) argv;
	struct WBArg *wa;

	if (wb->sm_NumArgs > 1)
	{
	    wa = wb->sm_ArgList;

	    /* skip the IconEdit icon */
	    wa++;

	    /* lock the dir, get the icon, etc... */
	}
    }
    */

    /* Do we have an icon to load? */
	if (!wi->diskobj)
    {
	NewFunc (wi);
    }
    ClearPointer (wp);

    SetIoErr (0);		/* no leftovers please */

    ProcessIMessages (wi);
    CloseAll (wi, MSG_IE_NOTHING);
}
