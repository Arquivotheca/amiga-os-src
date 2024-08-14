/* events.c
 *
 */

/*****************************************************************************/

#include "multiview.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#define	MMC_NOP			0

#define	MMC_OPEN		500
#define	MMC_OPENNEW		501
#define	MMC_SAVEAS		502
#define	MMC_PRINT		503
#define	MMC_ABOUT		504
#define	MMC_QUIT		505
#define	MMC_RELOAD		506

#define	MMC_MINIMIZE		510
#define	MMC_NORMAL		511
#define	MMC_MAXIMIZE		512

#define	MMC_MARK		520
#define	MMC_COPY		521
#define	MMC_SELECTALL		522
#define	MMC_CLEARSELECTED	523
#define	MMC_SHOWCLIP		524
#define	MMC_PRINTCLIP		525

#define	MMC_FIND		530
#define	MMC_NEXT		531
#define	MMC_GOTO		532
#define	MMC_SETBOOKMARK		533
#define	MMC_GOTOBOOKMARK	534

#define	MMC_EXECUTE		540

#define	MMC_SAVE_AS_DEFAULTS	550

#define	MMC_GETTRIGGERINFO	1000
#define	MMC_TRIGGER		1001
#define	MMC_USER		1002
#define	MMC_SCREEN		1003
#define	MMC_PUBSCREEN		1004

#define	MMC_WINDOW2FRONT	2000
#define	MMC_WINDOW2BACK		2001
#define	MMC_ACTIVATEWINDOW	2002
#define	MMC_SCREEN2FRONT	2003
#define	MMC_SCREEN2BACK		2004
#define	MMC_BEEPSCREEN		2005

#define	MMC_GCURRENTDIR		3000
#define	MMC_GFILEINFO		3001
#define	MMC_GOBJINFO		3002

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

struct Cmd cmdArray[] =
{
    {"OPEN",		openFunc,	MMC_OPEN,		"NAME/K,CLIPBOARD/S,CLIPUNIT/K/N", 3,},
    {"RELOAD",		reloadFunc,	MMC_RELOAD,		",",				0,},
    {"SAVEAS",		saveasFunc,	MMC_SAVEAS,		"NAME/K,IFF/S",			3,},
    {"PRINT",		printFunc,	MMC_PRINT,		",",				0,},
    {"ABOUT",		aboutFunc,	MMC_ABOUT,		"VAR/S,STEM/K",			2,},
    {"QUIT",		quitFunc,	MMC_QUIT,		",",				0,},

    {"MARK",		markFunc,	MMC_MARK,		",",				0,},
    {"COPY",		copyFunc,	MMC_COPY,		",",				0,},
    {"CLEARSELECTED",	clselectFunc,	MMC_CLEARSELECTED,	",",				0,},

    {"GETTRIGGERINFO",	gettiFunc,	MMC_GETTRIGGERINFO,	"VAR/S,STEM/K",			2,},
    {"DOTRIGGERMETHOD",	triggerFunc,	MMC_TRIGGER,		"METHOD/A",			1,},

    {"SCREEN",		screenFunc,	MMC_SCREEN,		"TRUE/S,FALSE/S",		2,},
    {"PUBSCREEN",	pscreenFunc,	MMC_PUBSCREEN,		"NAME/A",			1,},

    {"SNAPSHOT",	snapshotFunc,	MMC_SAVE_AS_DEFAULTS,	",",				0,},

    {"GETCURRENTDIR",	gcurdirFunc,	MMC_GCURRENTDIR,	",",				0,},
    {"GETFILEINFO",	gfileinfoFunc,	MMC_GFILEINFO,		",",				0,},
    {"GETOBJECTINFO",	gobjinfoFunc,	MMC_GOBJINFO,		"VAR/S,STEM/K",			2,},

    {"MINIMUMSIZE",	minFunc,	MMC_MINIMIZE,		",",				0,},
    {"NORMALSIZE",	nomFunc,	MMC_NORMAL,		",",				0,},
    {"MAXIMUMSIZE",	maxFunc,	MMC_MAXIMIZE,		",",				0,},
    {"WINDOWTOFRONT",	wfrontFunc,	MMC_WINDOW2FRONT,	",",				0,},
    {"WINDOWTOBACK",	wbackFunc,	MMC_WINDOW2BACK,	",",				0,},
    {"ACTIVATEWINDOW",	wactFunc,	MMC_ACTIVATEWINDOW,	",",				0,},
    {"SCREENTOFRONT",	sfrontFunc,	MMC_SCREEN2FRONT,	",",				0,},
    {"SCREENTOBACK",	sbackFunc,	MMC_SCREEN2BACK,	",",				0,},
    {"BEEPSCREEN",	sbeepFunc,	MMC_BEEPSCREEN,		",",				0,},

    {NULL,},
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

struct EdMenu newmenu[] =
{
    {NM_TITLE,	MV_MENU_PROJECT,		MMC_NOP,},
    { NM_ITEM,	MV_ITEM_OPEN,			MMC_OPEN,},
    { NM_ITEM,	MSG_NOTHING,			MMC_NOP,},
    { NM_ITEM,	MV_ITEM_SAVE_AS,		MMC_SAVEAS,},
    { NM_ITEM,	MSG_NOTHING,			MMC_NOP,},
    { NM_ITEM,	MV_ITEM_PRINT,			MMC_PRINT,},
    { NM_ITEM,	MV_ITEM_ABOUT,			MMC_ABOUT,},
    { NM_ITEM,	MSG_NOTHING,			MMC_NOP,},
    { NM_ITEM,	MV_ITEM_QUIT,			MMC_QUIT,},

    {NM_TITLE,	MV_MENU_EDIT,			MMC_NOP,},
    { NM_ITEM,	MV_ITEM_MARK,			MMC_MARK,},
    { NM_ITEM,	MV_ITEM_COPY,			MMC_COPY,},
    { NM_ITEM,	MSG_NOTHING,			MMC_NOP,},
    { NM_ITEM,	MV_ITEM_SELECT_ALL,		MMC_SELECTALL,},
    { NM_ITEM,	MV_ITEM_CLEAR_SELECTED,		MMC_CLEARSELECTED,},

    {NM_TITLE,	MV_MENU_WINDOW,			MMC_NOP,},
    { NM_ITEM,	MV_ITEM_DISPLAY,		MMC_SCREEN,},
    { NM_ITEM,	MSG_NOTHING,			MMC_NOP,},
    { NM_ITEM,	MV_ITEM_MINIMIZE,		MMC_MINIMIZE,},
    { NM_ITEM,	MV_ITEM_NORMAL,			MMC_NORMAL,},
    { NM_ITEM,	MV_ITEM_MAXIMIZE,		MMC_MAXIMIZE,},

    {NM_TITLE,	MV_MENU_SETTINGS,		MMC_NOP,},
    { NM_ITEM,	MV_ITEM_SAVE_AS_DEFAULTS,	MMC_SAVE_AS_DEFAULTS,},

    {  NM_END,	MSG_NOTHING, MMC_NOP,},
};

/*****************************************************************************/

ULONG GetUserMenus (struct GlobalData *gd, struct MinList *list)
{
    struct EdMenuNode *em;
    struct AnchorPath *ap;
    STRPTR sptr = "#?";
    struct Node *node;
    LONG len, retval;
    BPTR lock, old;
    ULONG num = 0;
    ULONG msize;

    /* Make sure the list is empty */
    while (node = RemHead ((struct List *) list))
	FreeVec (node);

    /* Scan the REXX:MultiView directory for directories */
    if (lock = Lock ("REXX:MultiView", ACCESS_READ))
    {
	/* Make it the current directory */
	old = CurrentDir (lock);

	/* Allocate an AnchorPath for work */
	msize = sizeof (struct AnchorPath) + MAXNAME + 2;
	if (ap = (struct AnchorPath *) AllocVec (msize, MEMF_CLEAR))
	{
	    /* Set the maximum string length */
	    ap->ap_Strlen = MAXNAME;

	    /* Do a wildcard search on the directory */
	    for (retval = MatchFirst (sptr, ap), len = 0; retval == 0L; retval = MatchNext (ap))
	    {
		/* Is it a directory? */
		if (ap->ap_Info.fib_DirEntryType > 0)
		{
		    /* It's a directory */
		    if (ap->ap_Flags & APF_DIDDIR)
		    {
			len--;
		    }
		    else
		    {
			len++;
			if (em = AllocVec (sizeof (struct EdMenuNode), MEMF_CLEAR))
			{
			    em->em_Node.ln_Type = NM_TITLE;
			    em->em_Node.ln_Name = em->em_Name;
			    strcpy (em->em_Name, ap->ap_Buf);
			    AddTail ((struct List *) list, &em->em_Node);
			    num++;
			}

			ap->ap_Flags |= APF_DODIR;
		    }

		    /* clear the completed directory flag */
		    ap->ap_Flags &= ~APF_DIDDIR;
		}
		/* Make sure we are in a directory */
		else if (len)
		{
		    /* Add to the list */
		    if (em = AllocVec (sizeof (struct EdMenuNode), MEMF_CLEAR))
		    {
			em->em_Node.ln_Type = NM_ITEM;
			em->em_Node.ln_Name = em->em_Name;
			strcpy (em->em_Name, FilePart (ap->ap_Buf));
			em->em_Command[0] = ap->ap_Info.fib_Comment[0];
			AddTail ((struct List *) list, &em->em_Node);
			num++;
		    }
		}
	    }

	    /* End of pattern matching session */
	    MatchEnd (ap);

	    /* Free the AnchorPath */
	    FreeVec (ap);
	}

	/* Restore the previous directory */
	CurrentDir (old);

	/* Release the directory lock */
	UnLock (lock);
    }

    return (num);
}

/*****************************************************************************/

struct Menu *localizemenus (struct GlobalData * gd, struct EdMenu * em)
{
    struct EdMenuNode *emn;
    struct NewMenu *nm;
    struct Menu *menus;
    UWORD i, j, k;

    struct Node *node;

    i = 0;
    while (em[i++].em_Type != NM_END)
    {
    }

    /* Scan for user menus */
    if (gd->gd_NumUserMenus == 0)
	gd->gd_NumUserMenus = GetUserMenus (gd, &gd->gd_UserMenuList);
    j = (UWORD) gd->gd_NumUserMenus;

    /* Allocate the menu strip */
    k = i + j;
    if (!(nm = AllocVec (sizeof (struct NewMenu) * k, MEMF_CLEAR | MEMF_PUBLIC)))
	return (NULL);

    /* Step through the menu */
    for (i = k = 0; em[i].em_Type != NM_END; i++, k++)
    {
	/* Is this the Settings menu? */
	if ((em[i].em_Type == NM_TITLE) && (em[i].em_Label == MV_MENU_SETTINGS))
	{
	    /* We found the settings menu, so insert the user menus before that */
	    for (node = (struct Node *) gd->gd_UserMenuList.mlh_Head; node->ln_Succ; node = node->ln_Succ, k++)
	    {
		emn = (struct EdMenuNode *) node;
		nm[k].nm_Type     = emn->em_Node.ln_Type;
		nm[k].nm_Label    = emn->em_Node.ln_Name;
		nm[k].nm_Flags    = NULL;
		nm[k].nm_CommKey  = NULL;
		if ((emn->em_Command[0] != ' ') && (emn->em_Command[0] != 0))
		    nm[k].nm_CommKey = emn->em_Command;
		nm[k].nm_UserData = (APTR) MMC_USER;
	    }
	}

	nm[k].nm_Type     = em[i].em_Type;
	nm[k].nm_Flags    = em[i].em_ItemFlags;
	nm[k].nm_UserData = (APTR) em[i].em_Command;

	/* Check the USE SCREEN menu item */
	if (em[i].em_Command == MMC_SCREEN)
	    nm[k].nm_Flags |= ((gd->gd_Options[OPT_SCREEN]) ? CHECKED : NULL) | CHECKIT | MENUTOGGLE;

	if (em[i].em_Type == NM_TITLE)
	{
	    nm[k].nm_Label = GetString (gd, em[i].em_Label);
	}
	else if (em[i].em_Command == 0)
	{
	    nm[k].nm_Label = NM_BARLABEL;
	}
	else if (em[i].em_Type != NM_END)
	{
	    nm[k].nm_CommKey = GetString (gd, em[i].em_Label);
	    nm[k].nm_Label   = &nm[k].nm_CommKey[2];
	    if (nm[k].nm_CommKey[0] == ' ')
	    {
		nm[k].nm_CommKey = NULL;
	    }
	}
    }

    if (menus = CreateMenusA (nm, NULL))
    {
	if (!(layoutmenus (gd, menus, GTMN_NewLookMenus, TRUE,
			   TAG_DONE)))
	{
	    FreeMenus (menus);
	    menus = NULL;
	}
    }

    FreeVec (nm);

    return (menus);
}

/*****************************************************************************/

struct MenuItem *SetMenuItemState (struct GlobalData * gd, ULONG command, BOOL state)
{
    struct MenuItem *mi;
    struct Menu *m;

    if (m = gd->gd_Menu)
    {
	while (m)
	{
	    mi = m->FirstItem;
	    while (mi)
	    {
		if (((ULONG) GTMENUITEM_USERDATA (mi)) == command)
		{
		    if (state)
			mi->Flags |= ITEMENABLED;
		    else
			mi->Flags &= ~ITEMENABLED;
		    return (mi);
		}
		mi = mi->NextItem;
	    }
	    m = m->NextMenu;
	}
    }
    return (NULL);
}

/*****************************************************************************/

void BackdropMenus (struct GlobalData *gd)
{
    SetMenuItemState (gd, MMC_MINIMIZE, FALSE);
    SetMenuItemState (gd, MMC_NORMAL,   FALSE);
    SetMenuItemState (gd, MMC_MAXIMIZE, FALSE);
}

/*****************************************************************************/

void NoObjectLoaded (struct GlobalData * gd)
{

    SetMenuItemState (gd, MMC_SAVEAS, FALSE);
    SetMenuItemState (gd, MMC_PRINT, FALSE);
    SetMenuItemState (gd, MMC_MARK, FALSE);
    SetMenuItemState (gd, MMC_COPY, FALSE);
    SetMenuItemState (gd, MMC_SELECTALL, FALSE);
    SetMenuItemState (gd, MMC_CLEARSELECTED, FALSE);
    SetMenuItemState (gd, MMC_FIND, FALSE);
    SetMenuItemState (gd, MMC_NEXT, FALSE);
    SetMenuItemState (gd, MMC_GOTO, FALSE);
    SetMenuItemState (gd, MMC_SETBOOKMARK, FALSE);
    SetMenuItemState (gd, MMC_GOTOBOOKMARK, FALSE);
    SetMenuItemState (gd, MMC_EXECUTE, FALSE);
}

/*****************************************************************************/

void PrintMenus (struct GlobalData *gd, BOOL state)
{
    SetMenuItemState (gd, MMC_OPEN,  state);
    SetMenuItemState (gd, MMC_PRINT, state);
    SetMenuItemState (gd, MMC_QUIT,  state);
}

/*****************************************************************************/

VOID syncMenuItems (struct GlobalData * gd, Class * cl, Object * o)
{
    UWORD i;

    if (gd->gd_Flags & GDF_REQUIRES_SCREEN)
	SetMenuItemState (gd, MMC_SCREEN, FALSE);
    else
	SetMenuItemState (gd, MMC_SCREEN, TRUE);

    if (gd->gd_Methods = (ULONG *) GetDTMethods (gd->gd_DataObject))
    {
	for (i = 0; gd->gd_Methods[i] != ~0; i++)
	{
	    switch (gd->gd_Methods[i])
	    {
		case DTM_SELECT:
		    if (!(gd->gd_Flags & GDF_BITMAP))
			SetMenuItemState (gd, MMC_MARK, TRUE);
		    break;

		case DTM_CLEARSELECTED:
		    SetMenuItemState (gd, MMC_CLEARSELECTED, TRUE);
		    break;

		case DTM_COPY:
		    SetMenuItemState (gd, MMC_COPY, TRUE);
		    break;

		case DTM_PRINT:
		    SetMenuItemState (gd, MMC_PRINT, TRUE);
		    break;

		case DTM_WRITE:
		    SetMenuItemState (gd, MMC_SAVEAS, TRUE);
		    break;
	    }
	}
    }
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

ULONG ASM openFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    BOOL open = FALSE;
    ULONG retval = 0;
    ULONG unit = 0;

    gd->gd_SourceType = DTST_FILE;
    if (c->c_Options[0])
    {
	strcpy (gd->gd_NameBuffer, (STRPTR) c->c_Options[0]);
	gd->gd_SourceName = gd->gd_NameBuffer;
	open = TRUE;
    }
    else if (c->c_Options[1])
    {
	unit = *((ULONG *) c->c_Options[2]);
	gd->gd_NameBuffer[0] = 0;
	gd->gd_SourceType = DTST_CLIPBOARD;
	gd->gd_SourceName = (APTR) unit;
	open = TRUE;
    }
    else
    {
	if (FileRequest (gd, 0, GetString (gd, MV_TITLE_SELECT_FILE_TO_OPEN), GetString (gd, MV_LABEL_OPEN), gd->gd_NameBuffer))
	    open = TRUE;
    }

    if (open)
    {
	/* Open the data */
	OpenNewData (gd, gd->gd_SourceType, unit);
	retval = TRUE;
    }

    return (retval);
}

/*****************************************************************************/

ULONG ASM reloadFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    OpenNewData ((struct GlobalData *) h->h_Data, gd->gd_SourceType, gd->gd_Unit);

    return (TRUE);
}

/*****************************************************************************/

ULONG ASM aboutFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    AboutObject (gd);

    return (TRUE);
}

/*****************************************************************************/

ULONG ASM snapshotFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    SaveSnapShot (gd);

    return (TRUE);
}

/*****************************************************************************/

ULONG ASM markFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct DTSpecialInfo *si;

    if (gd->gd_DataObject)
    {
	si = (struct DTSpecialInfo *) G (gd->gd_DataObject)->SpecialInfo;
	si->si_Flags |= DTSIF_DRAGSELECT;
	SetBlockPointer (gd, gd->gd_Window);
	gd->gd_Flags &= ~GDF_CLEARPOINTER;
    }

    return (TRUE);
}

/*****************************************************************************/

ULONG ASM copyFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct dtGeneral dtg;
    BOOL clears = FALSE;

    if (gd->gd_DataObject)
    {
	dtg.MethodID = DTM_COPY;
	if (!(clears = DoDTMethodA (gd->gd_DataObject, gd->gd_Window, NULL, &dtg)))
	    DisplayBeep (NULL);
    }

    if (clears)
    {
	dtg.MethodID = DTM_CLEARSELECTED;
	DoDTMethodA (gd->gd_DataObject, gd->gd_Window, NULL, &dtg);
    }

    return (TRUE);
}

/*****************************************************************************/

ULONG ASM printFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    if (gd->gd_DataObject)
	PrintObject (gd, 0);

    return (TRUE);
}

/*****************************************************************************/

ULONG ASM clselectFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct dtGeneral dtg;

    if (gd->gd_DataObject)
    {
	dtg.MethodID = DTM_CLEARSELECTED;
	DoDTMethodA (gd->gd_DataObject, gd->gd_Window, NULL, &dtg);
    }
    return (TRUE);
}

/*****************************************************************************/

ULONG ASM quitFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    gd->gd_Going = FALSE;
    return (1);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

ULONG ASM minFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct Window *win = gd->gd_Window;

    ChangeWindowBox (win, win->LeftEdge, win->TopEdge, win->MinWidth, win->MinHeight);

    return (1);
}

/*****************************************************************************/

ULONG ASM nomFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct Window *win = gd->gd_Window;
    ULONG left, top, width, height;

    left = win->LeftEdge;
    top = win->TopEdge;
    width = 0;
    height = 0;
    if (gd->gd_DataObject)
    {
	getdtattrs (gd, gd->gd_DataObject,
		    DTA_NominalHoriz, &width,
		    DTA_NominalVert, &height,
		    TAG_DONE);
    }

    if (width == 0)
	width = gd->gd_TextFont->tf_XSize * 80;
    if (height == 0)
	height = gd->gd_TextFont->tf_YSize * 24;
    width  += 4 + win->BorderLeft + win->BorderRight;
    height += 2 + win->BorderTop + win->BorderBottom;

    ChangeWindowBox (win, left, top, width, height);

    return (1);
}

/*****************************************************************************/

ULONG ASM maxFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct Window *win = gd->gd_Window;

    ChangeWindowBox (win, 0, 0, win->WScreen->Width, win->WScreen->Height);

    return (1);
}

/*****************************************************************************/

ULONG ASM wfrontFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    WindowToFront (gd->gd_Window);

    return (1);
}

/*****************************************************************************/

ULONG ASM wactFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    ActivateWindow (gd->gd_Window);

    return (1);
}

/*****************************************************************************/

ULONG ASM wbackFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    WindowToBack (gd->gd_Window);

    return (1);
}

/*****************************************************************************/

ULONG ASM sfrontFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    ScreenToFront (gd->gd_Window->WScreen);

    return (1);
}

/*****************************************************************************/

ULONG ASM sbackFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    ScreenToBack (gd->gd_Window->WScreen);

    return (1);
}

/*****************************************************************************/

ULONG ASM sbeepFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    DisplayBeep (gd->gd_Window->WScreen);

    return (1);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

ULONG ASM gettiFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct DTMethod *dtm, *tdtm;
    UBYTE name[128];
    UBYTE valu[128];
    ULONG i, len;

    if (dtm = GetDTTriggerMethods (gd->gd_DataObject))
    {
	/* Count the number of elements */
	tdtm = dtm;
	i = len = 0;
	while (tdtm->dtm_Label)
	{
	    len = strlen (tdtm->dtm_Command) + 1;
	    tdtm++;
	    i++;
	}

	/* Doing stems? */
	if (c->c_Options[1])
	{
	    sprintf (name, "%sCOUNT", (STRPTR)c->c_Options[1]);
	    sprintf (valu, "%ld", i);
	    SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, valu);
	}
	else
	{
	    gd->gd_ARexxReturn = AllocVec (len, MEMF_CLEAR);
	}

	/* Step through the items */
	tdtm = dtm;
	i = 0;
	while (tdtm->dtm_Label)
	{
	    if (c->c_Options[1])
	    {
		/* Set the label */
		sprintf (name, "%s%ld.LABEL", (STRPTR)c->c_Options[1], i);
		sprintf (valu, "%s", tdtm->dtm_Label);
		SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, valu);

		/* Set the command */
		sprintf (name, "%s%ld.COMMAND", (STRPTR)c->c_Options[1], i);
		sprintf (valu, "%s", tdtm->dtm_Command);
		SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, valu);

		/* Set the method */
		sprintf (name, "%s%ld.METHOD", (STRPTR)c->c_Options[1], i);
		sprintf (valu, "%ld", tdtm->dtm_Method);
		SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, valu);
	    }
	    else if (gd->gd_ARexxReturn)
	    {
		strcat (gd->gd_ARexxReturn, tdtm->dtm_Command);
		strcat (gd->gd_ARexxReturn, " ");
	    }

	    tdtm++;
	    i++;
	}
    }

    return (1);
}

/*****************************************************************************/

ULONG ASM triggerFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct DTMethod *dtm;
    ULONG retval = FALSE;

    /* Make sure we have a trigger method */
    if (c->c_Options[0])
    {
	/* Get the trigger methods */
	if (dtm = GetDTTriggerMethods (gd->gd_DataObject))
	{
	    /* Step through the trigger methods */
	    while (dtm->dtm_Label)
	    {
		/* Is this the trigger method */
		if (Stricmp (dtm->dtm_Command, (STRPTR) c->c_Options[0]) == 0)
		{
		    /* Perform the trigger method */
		    gd->gd_Trigger.dtt_Function = dtm->dtm_Method;
		    DoDTMethodA (gd->gd_DataObject, gd->gd_Window, NULL, &gd->gd_Trigger);

		    /* Indicate success */
		    retval = TRUE;
		    break;
		}
		dtm++;
	    }
	}
    }

    return (retval);
}

/*****************************************************************************/

ULONG ASM screenFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct MenuItem *mi;

    if (c->c_Options[1])
    {
	if (gd->gd_Options[OPT_SCREEN] == FALSE)
	    return (1);
    }
    else if (c->c_Options[0])
    {
	if (gd->gd_Options[OPT_SCREEN] == TRUE)
	    return (1);
    }
    else if (mi = SetMenuItemState (gd, MMC_SCREEN, TRUE))
    {
	if (mi->Flags & CHECKED)
	    c->c_Options[0] = TRUE;
	else
	    c->c_Options[1] = TRUE;
    }
    else
    {
	return (0);
    }

    CloseEnvironment (gd, 2);

    if (c->c_Options[1])
    {
	strcpy (gd->gd_ScreenNameBuffer, "Workbench");
	gd->gd_Options[OPT_SCREEN] = FALSE;
	gd->gd_Flags &= ~GDF_SCREEN;
	gd->gd_ScreenName = NULL;
    }
    else if (c->c_Options[0])
    {
	strcpy (gd->gd_ScreenNameBuffer, "MultiView");
	gd->gd_Options[OPT_SCREEN] = TRUE;
	gd->gd_Flags |= GDF_SCREEN;
	gd->gd_ScreenName = NULL;
    }

#if 1
    OpenNewData (gd, gd->gd_SourceType, gd->gd_Unit);
#else
    /* Get the new object */
    if (gd->gd_DataObject = NewDTObject (gd->gd_NameBuffer,
					DTA_ARexxPortName,	(ULONG)gd->gd_ARexxPortName,
					DTA_SourceType,		gd->gd_SourceType,
					DTA_TextAttr,		(ULONG) & gd->gd_TextAttr,
					GA_Immediate,		TRUE,
					GA_RelVerify,		TRUE,
					TAG_DONE))
    {
	/* Try opening the environment */
	if (!OpenEnvironment (gd))
	{
	    /* We have problems!!!! */
	    PrintF (gd, 1, gd->gd_SecondaryResult, NULL);
	    gd->gd_Going = FALSE;
	}
    }
#endif

    return (1);
}

/*****************************************************************************/

ULONG ASM pscreenFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;

    CloseEnvironment (gd, 2);

    gd->gd_Options[OPT_SCREEN] = FALSE;
    gd->gd_Flags &= ~GDF_SCREEN;

    if (c->c_Options[0])
	strcpy (gd->gd_ScreenNameBuffer, (STRPTR) c->c_Options[0]);
    else
	strcpy (gd->gd_ScreenNameBuffer, "Workbench");
    gd->gd_ScreenName = gd->gd_ScreenNameBuffer;

#if 1
    OpenNewData (gd, gd->gd_SourceType, gd->gd_Unit);
#else
    /* Get the new object */
    if (gd->gd_DataObject = NewDTObject (gd->gd_NameBuffer,
					DTA_ARexxPortName,	(ULONG)gd->gd_ARexxPortName,
					DTA_SourceType, gd->gd_SourceType,
					DTA_TextAttr, (ULONG) & gd->gd_TextAttr,
					GA_Immediate, TRUE,
					GA_RelVerify, TRUE,
					TAG_DONE))
    {
	/* Try opening the environment */
	if (!OpenEnvironment (gd))
	{
	    /* We have problems!!!! */
	    PrintF (gd, 1, gd->gd_SecondaryResult, NULL);
	    gd->gd_Going = FALSE;
	}
    }
#endif

    return (1);
}

/*****************************************************************************/

ULONG ASM gcurdirFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    STRPTR buff;
    ULONG len;
    STRPTR pp;
    BPTR lock;

    if (gd->gd_SourceType == DTST_CLIPBOARD)
    {
	if (gd->gd_ARexxReturn = AllocVec (24, MEMF_CLEAR))
	{
	    sprintf (gd->gd_ARexxReturn, "CLIPBOARD,%ld", gd->gd_Unit);
	}
    }
    else if (gd->gd_SourceType == DTST_FILE)
    {
	if (buff = AllocMem (512, NULL))
	{
	    if (lock = Lock (gd->gd_NameBuffer, ACCESS_READ))
	    {
		if (NameFromLock (lock, buff, 512))
		{
		    /* Make sure we have a directory part */
		    if ((pp = PathPart (buff)) != buff)
			*pp = 0;

		    len = strlen (buff) + 1;
		    if (gd->gd_ARexxReturn = AllocVec (len, MEMF_CLEAR))
		    {
			strcpy (gd->gd_ARexxReturn, buff);
		    }
		}
		UnLock (lock);
	    }
	    FreeMem (buff, 512);
	}
    }

    return (1);
}

/*****************************************************************************/

ULONG ASM gfileinfoFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    STRPTR buff;
    BPTR lock;
    ULONG len;

    if (gd->gd_SourceType == DTST_CLIPBOARD)
    {
	if (gd->gd_ARexxReturn = AllocVec (24, MEMF_CLEAR))
	{
	    sprintf (gd->gd_ARexxReturn, "CLIPBOARD,%ld", gd->gd_Unit);
	}
    }
    else if (gd->gd_SourceType == DTST_FILE)
    {
	if (buff = AllocMem (512, NULL))
	{
	    if (lock = Lock (gd->gd_NameBuffer, ACCESS_READ))
	    {
		if (NameFromLock (lock, buff, 512))
		{
		    len = strlen (buff) + 1;
		    if (gd->gd_ARexxReturn = AllocVec (len, MEMF_CLEAR))
		    {
			strcpy (gd->gd_ARexxReturn, buff);
		    }
		}
		UnLock (lock);
	    }
	    FreeMem (buff, 512);
	}
    }

    return (1);
}

/*****************************************************************************/

#include <libraries/iffparse.h>
#include <clib/iffparse_protos.h>
#include <pragmas/iffparse_pragmas.h>

/*****************************************************************************/

ULONG ASM gobjinfoFunc (REG (a0) struct Hook *h, REG (a2) struct Cmd *c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct DataType *dtn = NULL;
    struct DataTypeHeader *dth;
    BOOL success = FALSE;
    STRPTR buff = NULL;
    UBYTE name[128];
    UBYTE valu[16];
    BPTR lock;

    struct Library *IFFParseBase;

    /* Get a pointer to the DataTypes descriptor */
    getdtattrs (gd, gd->gd_DataObject, DTA_DataType, (ULONG)&dtn, TAG_DONE);
    if (dtn == NULL)
	return (0);
    dth = dtn->dtn_Header;

    if ((IFFParseBase = OpenLibrary ("iffparse.library", 0)) == NULL)
	return (0);

    /* Get the file name */
    if (gd->gd_SourceType == DTST_CLIPBOARD)
    {
    }
    else if (gd->gd_SourceType == DTST_FILE)
    {
	if (buff = AllocMem (512, NULL))
	{
	    if (lock = Lock (gd->gd_NameBuffer, ACCESS_READ))
	    {
		if (NameFromLock (lock, buff, 512))
		    success = TRUE;
		UnLock (lock);
	    }

	    if (!success)
	    {
		FreeMem (buff, 512);
		buff = NULL;
	    }
	}
    }

    /* STEM */
    if (c->c_Options[1])
    {
	if (buff)
	{
	    sprintf (name, "%sFILENAME", (STRPTR)c->c_Options[1]);
	    SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, buff);
	}

	sprintf (name, "%sNAME", (STRPTR) c->c_Options[1]);
	SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, dth->dth_Name);

	sprintf (name, "%sBASENAME", (STRPTR) c->c_Options[1]);
	SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, dth->dth_BaseName);

	sprintf (name, "%sGROUP", (STRPTR) c->c_Options[1]);
	SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, GetDTString (dth->dth_GroupID));

	sprintf (name, "%sID", (STRPTR) c->c_Options[1]);
	SetARexxStem (gd->gd_ARexxHandle, (struct RexxMsg *) gd->gd_Msg, name, IDtoStr (dth->dth_ID, valu));
    }
    /* VAR */
    else if (gd->gd_ARexxReturn = AllocVec (512, MEMF_CLEAR))
    {
	sprintf (gd->gd_ARexxReturn, "\"%s\",\"%s\",\"%s\",\"%s\"",
		 dth->dth_Name, dth->dth_BaseName,
		 GetDTString (dth->dth_GroupID), IDtoStr (dth->dth_ID, valu));
    }

    if (buff)
	FreeMem (buff, 512);

    CloseLibrary (IFFParseBase);

    return (1);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

BOOL ProcessCommand (struct GlobalData * gd, ULONG id, struct IntuiMessage * imsg)
{
    if (!id)
	return FALSE;

    /* Set busy pointer */
    gd->gd_Flags |= GDF_CLEARPOINTER;
    setwindowpointer (gd, window, WA_BusyPointer, TRUE, WA_PointerDelay, TRUE, TAG_DONE);

    /* Perform the function */
    DispatchCmd (gd->gd_CmdHeader, id, NULL);

    /* Clear busy pointer */
    if (gd->gd_Flags & GDF_CLEARPOINTER)
    {
	setwindowpointer (gd, window, WA_Pointer, NULL, TAG_DONE);
	gd->gd_Flags &= ~GDF_CLEARPOINTER;
    }
    return (TRUE);
}

/*****************************************************************************/

VOID Navigate (struct GlobalData * gd, LONG cmd, BOOL needvisual)
{
    LONG otoph, toph, vish, toth;
    LONG otopv, topv, visv, totv;

    if (!gd->gd_DataObject)
	return;

    if (needvisual)
    {
	/* Get the current values from the data object */
	getdtattrs (gd, gd->gd_DataObject,
		    DTA_TopVert,	&topv,
		    DTA_VisibleVert,	&visv,
		    DTA_TotalVert,	&totv,
		    DTA_TopHoriz,	&toph,
		    DTA_VisibleHoriz,	&vish,
		    DTA_TotalHoriz,	&toth,
		    TAG_DONE);

	/* Check for bogus values */
	if ((topv < 0) || (toph < 0) || (totv == 0) || (toth == 0))
	{
	    topv = gd->gd_TopVert;
	    visv = gd->gd_VisVert;
	    totv = gd->gd_TotVert;

	    toph = gd->gd_TopHoriz;
	    vish = gd->gd_VisHoriz;
	    toth = gd->gd_TotHoriz;
	}

	otoph = toph;
	otopv = topv;
    }

    switch (cmd)
    {
	case AC_TOP:
	    topv = 0;
	    break;

	case AC_BOTTOM:
	    topv = totv - (visv - 1);
	    break;

	case AC_FAR_LEFT:
	    toph = 0;
	    break;

	case AC_FAR_RIGHT:
	    toph = toth - (vish - 1);
	    break;

	case AC_UNIT_UP:
	    topv--;
	    break;

	case AC_UNIT_DOWN:
	    topv++;
	    break;

	case AC_UNIT_LEFT:
	    toph--;
	    break;

	case AC_UNIT_RIGHT:
	    toph++;
	    break;

	case AC_PAGE_UP:
	    topv -= (visv - 1);
	    break;

	case AC_PAGE_DOWN:
	    topv += (visv - 1);
	    break;

	case AC_PAGE_LEFT:
	    toph -= (vish - 1);
	    break;

	case AC_PAGE_RIGHT:
	    toph += (vish - 1);
	    break;

#ifndef	STM_HELP
#define	STM_HELP		17
#endif

	case AC_HELP:
	    gd->gd_Trigger.dtt_Function = STM_HELP;
	    DoDTMethodA (gd->gd_DataObject, window, NULL, &gd->gd_Trigger);
	    break;

	case AC_FIELD_NEXT:
	    gd->gd_Trigger.dtt_Function = STM_NEXT_FIELD;
	    DoDTMethodA (gd->gd_DataObject, window, NULL, &gd->gd_Trigger);
	    break;

	case AC_FIELD_PREV:
	    gd->gd_Trigger.dtt_Function = STM_PREV_FIELD;
	    DoDTMethodA (gd->gd_DataObject, window, NULL, &gd->gd_Trigger);
	    break;

	case AC_ACTIVATE_FIELD:
	    gd->gd_Trigger.dtt_Function = STM_ACTIVATE_FIELD;
	    DoDTMethodA (gd->gd_DataObject, window, NULL, &gd->gd_Trigger);
	    break;

	case AC_NODE_NEXT:
	    gd->gd_Trigger.dtt_Function = STM_BROWSE_NEXT;
	    DoDTMethodA (gd->gd_DataObject, window, NULL, &gd->gd_Trigger);
	    break;

	case AC_NODE_PREV:
	    gd->gd_Trigger.dtt_Function = STM_BROWSE_PREV;
	    DoDTMethodA (gd->gd_DataObject, window, NULL, &gd->gd_Trigger);
	    break;

	case AC_RETRACE:
	    gd->gd_Trigger.dtt_Function = STM_RETRACE;
	    DoDTMethodA (gd->gd_DataObject, window, NULL, &gd->gd_Trigger);
	    break;
    }

    if (needvisual)
    {
	if (topv < 0)
	    topv = 0;
	if (toph < 0)
	    toph = 0;
	if (topv > totv - visv)
	    topv = totv - visv;
	if (toph > toth - vish)
	    toph = toth - vish;

	if ((topv != otopv) || (toph != otoph))
	{
	    /* Do the scrollers first, because they are faster */
	    setgadgetattrs (gd, gd->gd_VGad, gd->gd_Window, PGA_Top, topv, TAG_DONE);
	    setgadgetattrs (gd,	gd->gd_HGad, gd->gd_Window, PGA_Top, toph, TAG_DONE);

	    /* Now tell the data type to scroll */
	    setgadgetattrs (gd, (struct Gadget *)gd->gd_DataObject, gd->gd_Window,
			    DTA_TopVert,	topv,
			    DTA_TopHoriz,	toph,
			    TAG_DONE);

	    /* Remember these values */
	    gd->gd_TopVert  = topv;
	    gd->gd_TopHoriz = toph;
	}
    }
}

/*****************************************************************************/

void HandleEvents (struct GlobalData * gd)
{
    struct Gadget *agad = NULL;
    struct MenuItem *menuitem;
    struct IntuiMessage *imsg;
    struct AppMessage *amsg;
    struct RexxMsg *rmsg;
    struct WBArg *wbArg;
    LONG topv, toph;
    BOOL reply;
    ULONG siga;
    ULONG sigw;
    ULONG sigr;
    ULONG sigp;
    ULONG sigx;

    BOOL quit = FALSE;

    struct TagItem *tstate;
    struct TagItem *tags;
    struct TagItem *tag;
    LONG tidata;

    char *text = NULL;
    BOOL errMsg;
    LONG level;

    sigw = 1L << gd->gd_IDCMPPort->mp_SigBit;
    siga = 1L << gd->gd_AWPort->mp_SigBit;
    sigx = ARexxSignal (gd->gd_ARexxHandle);

    while (gd->gd_Going)
    {
	/* Wait for something to happen */
	sigp = ((gd->gd_PrintWin) ? (1L << gd->gd_PrintWin->UserPort->mp_SigBit) : NULL);

	sigr = Wait (sigx | siga | sigw | sigp | SIGBREAKF_CTRL_C);

	/* Handle the printer messages */
	if ((sigr & sigp) && gd->gd_PrintWin)
	{
	    if (SysReqHandler (gd->gd_PrintWin, NULL, FALSE) == 0)
		DoMethod (gd->gd_DataObject, DTM_ABORTPRINT, NULL);
	}

	/* User break */
	if (sigr & SIGBREAKF_CTRL_C)
	    gd->gd_Going = FALSE;

	/* AppWindow messages */
	if (sigr & siga)
	{
	    ActivateWindow (window);
	    setwindowpointer (gd, window, WA_BusyPointer, TRUE, WA_PointerDelay, TRUE, TAG_DONE);

	    /* Pull all the messages from the AppWindow port */
	    while (amsg = (struct AppMessage *) GetMsg (gd->gd_AWPort))
	    {
		/* Any arguments? */
		if (amsg->am_NumArgs)
		{
		    wbArg = amsg->am_ArgList;
		    NameFromLock (wbArg->wa_Lock, gd->gd_NameBuffer, NAMEBUFSIZE);
		    AddPart (gd->gd_NameBuffer, wbArg->wa_Name, NAMEBUFSIZE);

		    /* Try opening the data.  If it is a directory, and there isn't a class to handle it,
		     * then popup in it. */
		    if (OpenNewData (gd, DTST_FILE, 0) == 2)
			if (FileRequest (gd, 0, GetString (gd, MV_TITLE_SELECT_FILE_TO_OPEN), GetString (gd, MV_LABEL_OPEN), gd->gd_NameBuffer))
			    OpenNewData (gd, DTST_FILE, 0);
		}
		ReplyMsg ((struct Message *) amsg);
	    }
	}

	/* Handle Intuition messages */
	while (imsg = (struct IntuiMessage *) GetMsg (gd->gd_IDCMPPort))
	{
	    /* Set the current message */
	    gd->gd_Msg = (struct Message *) imsg;
	    gd->gd_MsgType = 0;
	    reply = TRUE;

	    if (imsg->IDCMPWindow == window)
	    {
                switch (imsg->Class)
                {
                    case IDCMP_GADGETDOWN:
                        /* Remember the active gadget */
                        agad = (struct Gadget *) imsg->IAddress;

                        /* Get the current top values */
                        getdtattrs (gd, gd->gd_DataObject, DTA_TopVert, &topv, DTA_TopHoriz, &toph, TAG_DONE);
                        break;

                    case IDCMP_INTUITICKS:
                    case IDCMP_MOUSEMOVE:
                        if (agad && (agad->Flags & GFLG_SELECTED))
                        {
                            switch (agad->GadgetID)
                            {
                                case LEFT_GID:
                                    Navigate (gd, AC_UNIT_LEFT, TRUE);
                                    break;

                                case RIGHT_GID:
                                    Navigate (gd, AC_UNIT_RIGHT, TRUE);
                                    break;

                                case UP_GID:
                                    Navigate (gd, AC_UNIT_UP, TRUE);
                                    break;

                                case DOWN_GID:
                                    Navigate (gd, AC_UNIT_DOWN, TRUE);
                                    break;

                                case HORIZ_GID:
                                case VERT_GID:
                                    break;
                            }
                        }
                        break;

                    case IDCMP_GADGETUP:
                        if (agad)
                        {
                            switch (agad->GadgetID)
                            {
                                case LEFT_GID:
                                case RIGHT_GID:
                                case UP_GID:
                                case DOWN_GID:
                                case HORIZ_GID:
                                case VERT_GID:
                                    break;
                            }
                        }
                        agad = NULL;
                        break;

                    case IDCMP_IDCMPUPDATE:
                        /* Get the latest values from the scrollers */
			if (gd->gd_HGad && gd->gd_VGad)
			{
			    GetAttr (PGA_Top, gd->gd_HGad, (ULONG *) &toph);
			    GetAttr (PGA_Top, gd->gd_VGad, (ULONG *) &topv);
			}

			errMsg = FALSE;
                        tstate = tags = (struct TagItem *) imsg->IAddress;
                        while (tag = NextTagItem (&tstate))
                        {
                            tidata = tag->ti_Data;
                            switch (tag->ti_Tag)
                            {
                                case DTA_ErrorLevel:
				    errMsg = TRUE;
                                    break;

                                case DTA_PrinterStatus:
                                    PrintComplete (gd);
                                    if (tidata)
                                        PrintF (gd, 1, 990 + tidata, NULL);
                                    if (quit)
                                        gd->gd_Going = FALSE;
                                    break;

                                case DTA_Busy:
                                    if (tidata)
                                        setwindowpointer (gd, window, WA_BusyPointer, TRUE, WA_PointerDelay, TRUE, TAG_DONE);
                                    else
                                        setwindowpointer (gd, window, WA_Pointer, NULL, TAG_DONE);
                                    break;

                                case DTA_Title:
				    if (tidata)
					strcpy (gd->gd_Title, (STRPTR) tidata);
				    else
					gd->gd_Title[0] = 0;
				    if (gd->gd_Window)
					SetWindowTitles (gd->gd_Window, gd->gd_Title, "MultiView");
                                    break;
                            }
                        }

			/* See if we had an error message */
			if (errMsg)
			{
			    gd->gd_SecondaryResult = GetTagData (DTA_ErrorNumber, NULL, tags);
			    PrintF (gd, 2, gd->gd_SecondaryResult, (STRPTR) GetTagData (DTA_ErrorString, NULL, tags));
			}
                        break;

                    case IDCMP_MENUPICK:
                        if (imsg->Code != MENUNULL)
                        {
                            /* Get the menu item address */
                            menuitem = ItemAddress (imsg->IDCMPWindow->MenuStrip, imsg->Code);

                            /* Reply to the IntuiMessage */
                            ReplyMsg ((struct Message *) imsg);
                            reply = FALSE;

                            /* Was it a User supplied menu item? */
                            if ((ULONG)MENU_USERDATA (menuitem) == MMC_USER)
                            {
                                /* Make sure it is a text item */
                                if (menuitem->Flags & ITEMTEXT)
                                {
                                    struct Menu *pm = NULL;
                                    struct MenuItem *mi;
                                    struct Menu *m;

                                    /* Locate the menu that the item resides in */
                                    for (m = gd->gd_Window->MenuStrip; m && (pm == NULL); m = m->NextMenu)
                                    {
                                        for (mi = m->FirstItem; mi && (pm == NULL); mi = mi->NextItem)
                                        {
                                            if (mi == menuitem)
                                            {
                                                pm = m;
                                                break;
                                            }
                                        }
                                    }

                                    /* Make sure we found the menu that the item resides in */
                                    if (pm)
                                    {
                                        /* Build the complete script name */
                                        sprintf (gd->gd_TempBuffer, "REXX:MultiView/%s/%s", pm->MenuName, ((struct IntuiText *)(menuitem->ItemFill))->IText);

                                        /* Execute the ARexx script */
                                        SendARexxMsg (gd->gd_ARexxHandle, gd->gd_TempBuffer, FALSE);
                                    }
                                }
                            }
                            else
                            {
                                ProcessCommand (gd, (ULONG) MENU_USERDATA (menuitem), imsg);
                            }
                        }
                        break;

                    case IDCMP_CHANGEWINDOW:
                        if (gd->gd_Window->Flags & WFLG_ZOOMED)
                        {
                            gd->gd_Prefs.p_Zoom = *((struct IBox *) (&gd->gd_Window->LeftEdge));
                        }
                        else if (gd->gd_Flags & GDF_SNAPSHOT)
                        {
                            gd->gd_Prefs.p_Window = *((struct IBox *) (&gd->gd_Window->LeftEdge));
                            gd->gd_Prefs.p_Window.Width -= (gd->gd_Window->BorderLeft + gd->gd_Window->BorderRight + 4);
                            gd->gd_Prefs.p_Window.Height -= (gd->gd_Window->BorderTop + gd->gd_Window->BorderBottom + 2);
                        }
                        else
                        {
                            gd->gd_Prefs.p_Window.Left = gd->gd_Window->LeftEdge;
                            gd->gd_Prefs.p_Window.Top = gd->gd_Window->TopEdge;
                        }
                        break;

                    case IDCMP_CLOSEWINDOW:
                        gd->gd_Going = FALSE;
                        break;

                    case IDCMP_RAWKEY:
                        switch (imsg->Code)
                        {
			    /* Help key */
			    case 95:
                                Navigate (gd, AC_HELP, FALSE);
				break;

                                /* Shift Tab */
                            case 66:
                                Navigate (gd, AC_FIELD_PREV, FALSE);
                                break;

                                /* Up Arrow */
                            case 76:
                                if (imsg->Qualifier & ALTED)
                                    Navigate (gd, AC_TOP, TRUE);
                                else if (imsg->Qualifier & SHIFTED)
                                    Navigate (gd, AC_PAGE_UP, TRUE);
                                else
                                    Navigate (gd, AC_UNIT_UP, TRUE);
                                break;

                                /* Down Arrow */
                            case 77:
                                if (imsg->Qualifier & ALTED)
                                    Navigate (gd, AC_BOTTOM, TRUE);
                                else if (imsg->Qualifier & SHIFTED)
                                    Navigate (gd, AC_PAGE_DOWN, TRUE);
                                else
                                    Navigate (gd, AC_UNIT_DOWN, TRUE);
                                break;

                                /* Right Arrow */
                            case 78:
                                if (imsg->Qualifier & ALTED)
                                    Navigate (gd, AC_FAR_RIGHT, TRUE);
                                else if (imsg->Qualifier & SHIFTED)
                                    Navigate (gd, AC_PAGE_RIGHT, TRUE);
                                else
                                    Navigate (gd, AC_UNIT_RIGHT, TRUE);
                                break;

                                /* Left Arrow */
                            case 79:
                                if (imsg->Qualifier & ALTED)
                                    Navigate (gd, AC_FAR_LEFT, TRUE);
                                else if (imsg->Qualifier & SHIFTED)
                                    Navigate (gd, AC_PAGE_LEFT, TRUE);
                                else
                                    Navigate (gd, AC_UNIT_LEFT, TRUE);
                                break;
                        }
                        break;

                    case IDCMP_VANILLAKEY:
                        switch (imsg->Code)
                        {
                                /* Backspace */
                            case 8:
                                Navigate (gd, AC_PAGE_UP, TRUE);
                                break;

                                /* Tab */
                            case 9:
                                if (imsg->Qualifier & SHIFTED)
                                    Navigate (gd, AC_FIELD_PREV, FALSE);
                                else
                                    Navigate (gd, AC_FIELD_NEXT, FALSE);
                                break;

                                /* Return */
                            case 13:
                                Navigate (gd, AC_ACTIVATE_FIELD, FALSE);
                                break;

                                /* Space */
                            case 32:
                                Navigate (gd, AC_PAGE_DOWN, TRUE);
                                break;

                            case 'r':
                            case 'R':
                                break;

                            case '/':
                                Navigate (gd, AC_RETRACE, FALSE);
                                break;

                            case '<':
                                Navigate (gd, AC_NODE_PREV, FALSE);
                                break;

                            case '>':
                                Navigate (gd, AC_NODE_NEXT, FALSE);
                                break;

                            case 3:
                            case 27:
                            case 'q':
                            case 'Q':
                                gd->gd_Going = FALSE;
                                break;
                        }
                        break;

                }
	    }

	    /* Are we supposed to reply to the message? */
	    if (reply)
	    {
		ReplyMsg ((struct Message *) imsg);
	    }
	}

	/* Handle ARexx messages */
	if (sigr & sigx)
	{
	    /* Pull all the ARexx messages and handle them */
	    while (rmsg = GetARexxMsg (gd->gd_ARexxHandle))
	    {
		/* Set the current message */
		gd->gd_Msg = (struct Message *) rmsg;
		gd->gd_MsgType = 1;

		/* A message returned with an error */
		if (rmsg == REXX_RETURN_ERROR)
		{
		    DisplayBeep (gd->gd_Screen);
		}
		else
		{
		    /* Dispatch the command */
		    DispatchCmd (gd->gd_CmdHeader, 0, ARG0 (rmsg));
		    if ((level = gd->gd_Result) == RETURN_OK)
		    {
			text = gd->gd_ARexxReturn;
		    }
		    else
		    {
			sprintf (gd->gd_Buffer, "%ld", gd->gd_Result2);
			SetARexxLastError (gd->gd_ARexxHandle, rmsg, gd->gd_Buffer);
		    }

		    /* Reply to the message */
		    ReplyARexxMsg (gd->gd_ARexxHandle, rmsg, text, level);

		    FreeVec (gd->gd_ARexxReturn);
		    text = gd->gd_ARexxReturn = NULL;
		}
	    }
	}

	if (!gd->gd_Going)
	{
	    if (gd->gd_PIO)
	    {
		DoMethod (gd->gd_DataObject, DTM_ABORTPRINT, NULL);
		gd->gd_Going = quit = TRUE;
	    }
	}
    }
}
