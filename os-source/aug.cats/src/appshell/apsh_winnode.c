/* apsh_winnode.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Window management routines.
 *
 */

#include "appshell_internal.h"
#include <graphics/gfxbase.h>

void kprintf (void *,...);

#define	DB(x)	;
#define	DC(x)	;
#define	DE(x)	;
#define	DF(x)	;
#define	DO(x)	;
#define	DA(x)	;
#define	DI(x)	;
#define	DV(x)	;
#define	DQ(x)	;
#define	DW(x)	;
#define DOW(x)	;

extern STRPTR __regargs GT_Loc( struct AppInfo *, ULONG, LONG );

/* Fallback font */
struct TextAttr TOPAZ80 =
{(STRPTR) "topaz.font", TOPAZ_EIGHTY, 0, 0};

VOID SetupDisable (struct AppInfo * ai, struct WinNode * wn);
VOID SetCurrentWindowA (struct AppInfo *, struct WinNode *, struct TagItem *);
VOID SetCurrentWindow (struct AppInfo *, struct WinNode *, ULONG,...);
WORD BreakString (STRPTR text, STRPTR * array, WORD maxstrings);

struct HandleObj
{
    struct AppInfo *ho_AI;
    ULONG ho_Func;
};

VOID doObjList (struct ObjectNode * on, ULONG cnt, struct HandleObj * ho)
{

#if 0
    ULONG FuncID;

    /* It performs the function ... but what is it? */
    if (FuncID = on->on_Funcs[ho->ho_Func])
    {
	PerfFunc (ho->ho_AI, FuncID, NULL, NULL);
    }
#endif
}

struct ObjectInfo *NewObjList (ULONG tags,...)
{

    return (NewObjListA ((struct TagItem *) & tags));
}

struct TagItem *MakeNewTagList (ULONG data,...)
{
    struct TagItem *tags = (struct TagItem *) & data;

    return (CloneTagItems (tags));
}

struct WinNode *GetWinNode (struct AppInfo * ai, struct TagItem * tl)
{
    struct ObjectInfo *oi;
    struct WinNode *wn = NULL;
    struct MsgHandler *mh = NULL;
    struct TagItem *attrs;
    STRPTR name = NULL;

    /* Try getting a pointer to the Intuition message handler */
    mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE);

    if (attrs = (struct TagItem *) GetTagData (APSH_WindowEnv, (LONG) tl, tl))
    {
	struct Window *win;
	struct Funcs *f;

	if (win = (struct Window *) GetTagData (APSH_WinPointer, NULL, attrs))
	{
	    oi = (struct ObjectInfo *) win->UserData;
	    wn = (struct WinNode *) oi->oi_SystemData;

	    /* We have a window node, return with it */
	    DE (kprintf ("APSH_WinPointer %s\n", wn->wn_Header.mho_Node.ln_Name));
	    return (wn);
	}
	else if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, attrs))
	{

	    /*
	     * All the Intuition commands use the first arg as the Window name.
	     */
	    if (name = (STRPTR) f->fe_Options[0])
	    {
		DE (kprintf ("FuncEntry %s\n", name));
	    }
	}
	else if (name = (STRPTR) GetTagData (APSH_NameTag, NULL, attrs))
	{
	    DE (kprintf ("APSH_NameTag %s\n", name));
	}

	/* See if they passed a pointer to the Intuition message handler */
	mh = (struct MsgHandler *)
	  GetTagData (APSH_MsgHandler, (LONG) mh, attrs);
    }

    /* Get a pointer to the IDCMP message handler */
    if (mh)
    {
	struct MHObject *mho = &(mh->mh_Header);
	struct List *list = &(mho->mho_ObjList);

	/* Did they give us a name to search for? */
	if (name)
	{
	    if (wn = (struct WinNode *) FindNameI (list, name))
	    {
		DE (kprintf ("Found by name\n"));
	    }
	    else
	    {
		DE (kprintf ("Couldn't find name\n"));
	    }
	}
	else
	{
	    /* No name given, so use the current window */
	    if (wn = (struct WinNode *) mho->mho_CurNode)
	    {
		DE (kprintf ("Current window %s\n",
			     wn->wn_Header.mho_Node.ln_Name));
	    }
	    else
	    {
		DE (kprintf ("No current window\n"));
	    }
	}
    }
    else
    {
	DE (kprintf ("Can't find IDCMP message handler data\n"));
    }

    return (wn);
}

VOID SetCurrentWindowA (struct AppInfo * ai, struct WinNode * wn, struct TagItem * attrs)
{

    /* Make sure we have a Window node */
    if (wn)
    {
	struct ObjectInfo *oi = wn->wn_OI;
	struct Window *win = wn->wn_Win;
	struct MsgHandler *mh;
	WORD x = 0, y = 0;

	/* Is the window open? */
	if (win)
	{
	    /* Set the current mouse position */
	    x = win->MouseX;
	    y = win->MouseY;
	}

	/* Try getting a pointer to the Intuition message handler */
	mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE);

	/* See if they passed any attributes */
	if (attrs)
	{
	    /* See if they passed a pointer to the Intuition message handler */
	    mh = (struct MsgHandler *)
	      GetTagData (APSH_MsgHandler, (LONG) mh, attrs);
	}

	/* Do we have a pointer to the MsgHandler? */
	if (mh)
	{
	    struct MHObject *mho = &(mh->mh_Header);
	    struct IDCMPInfo *md = mho->mho_SysData;

	    /* Set the current object */
	    mho->mho_CurNode = (struct MHObject *) & (wn->wn_Header);

	    /* Set the IDCMP active information */
	    if (win != md->ii_Window)
	    {
		md->ii_CurObj = NULL;
	    }
	    md->ii_Window = win;
	    md->ii_CurWin = wn;
	}

	/* Set the AppInfo active information */
	if (win != ai->ai_Window)
	{
	    ai->ai_Gadget = NULL;
	    ai->ai_CurObj = NULL;
	}
	ai->ai_Screen = oi->oi_Screen;
	ai->ai_Font = oi->oi_TextFont;
	ai->ai_Window = win;
	ai->ai_DI = oi->oi_DrInfo;
	ai->ai_VI = oi->oi_VisualInfo;
	ai->ai_MouseX = x;
	ai->ai_MouseY = y;
    }
}

VOID
SetupMenuDisable (struct AppInfo * ai,struct WinNode * wn,STRPTR cmd,struct TempMenu *tm)
{
    STRPTR argv[MAXARG] = {NULL};
    USHORT mnum = tm->tm_MenuNum;
    STRPTR clone = NULL;
    ULONG argc = 0L;
    struct NewMenu *nm = &(tm->tm_NewMenu);

    if (ai && wn && cmd)
    {
	struct FuncEntry *fe;

	/* Non-destructively parse the command line */
	if (clone = BuildParseLine (cmd, &argc, argv))
	{
	    DQ(kprintf ("Parse [%s] %ld opts : ", cmd, argc));

	    if (fe = GetFuncEntry (ai, argv[0], NULL))
	    {
		DQ(kprintf ("Setup entry for [%s]\n", argv[0]));
		fe->fe_WinNode = wn;
		fe->fe_MenuNumber = mnum;

		/* See if the function is disabled */
		if (fe->fe_Flags & APSHF_DISABLED)
		{
		    nm->nm_Flags |= NM_ITEMDISABLED;
		}
	    }
	    else
	    {
		DQ(kprintf ("Can't locate entry for [%s]\n", argv[0]));
	    }
	}
	else
	{
	    DQ(kprintf ("Couldn't parse [%s]\n", cmd));
	}

	/* Deallocate the parsed line */
	FreeParseLine (clone);
    }
}

/* Setup the function disable pointers */
VOID
SetupDisable (struct AppInfo * ai, struct WinNode * wn)
{
    struct ObjectInfo *oi = wn->wn_OI;
    struct List *list = &(oi->oi_ObjList);
    struct Window *win = wn->wn_Win;
    struct ObjectNode *on;

    /* Make sure it isn't an empty list */
    DB (kprintf ("SetupDisable 0x%lx 0x%lx\n", ai, wn));
    if (win)
    {
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *next;
	    ULONG FuncID;

	    /* Start at the beginning */
	    node = list->lh_Head;
	    while (next = node->ln_Succ)
	    {
		on = (struct ObjectNode *) node;

		DB (kprintf ("object %s\n", on->on_Object.o_Name));
		FuncID = on->on_Funcs[EG_RELEASE];

		if (FuncID != NO_FUNCTION)
		{
		    struct FuncEntry *fe;

		    if (fe = GetFuncEntry (ai, NULL, FuncID))
		    {
			fe->fe_Object = on;
			fe->fe_Gadget = on->on_Gadget;
			fe->fe_WinNode = wn;

			if (fe->fe_Flags & APSHF_DISABLED)
			{
			    DB (kprintf ("Disable 0x%lx 0x%lx\n", on->on_Gadget, win));
			    if (IsGadToolObj (&(on->on_Object)))
			    {
				GT_SetGadgetAttrs (on->on_Gadget, win, NULL,
						   GA_DISABLED, TRUE,
						   TAG_DONE);
			    }
			    else
			    {
				/* set the gadget to the correct rectangle */
				SetGadgetAttrs (on->on_Gadget, win, NULL,
						GA_SELECTED, FALSE,
						GA_DISABLED, TRUE,
						TAG_END);
			    }
			}
		    }
		    else
		    {
			DB (kprintf ("no function entry\n"));
		    }
		}
		else
		{
		    DB (kprintf ("no function\n"));
		}

		/* Go to the next node */
		node = next;
	    }
	}
	else
	{
	    DB (kprintf ("list is empty\n"));
	}
    }
    else
    {
	DB (kprintf ("window not open...\n"));
    }

    DB (kprintf ("SetupDisable exit\n"));
}

VOID SetCurrentWindow (struct AppInfo * ai, struct WinNode * wn, ULONG data,...)
{

    SetCurrentWindowA (ai, wn, (struct TagItem *) & data);
}

struct TempMenu *
 CreateMenuItem (struct List * list,
		  STRPTR tname,
		  UBYTE type,
		  STRPTR label,
		  STRPTR cmdkey,
		  STRPTR cmd)
{
    struct TempMenu *tm = NULL;
    STRPTR name = tname;
    ULONG msize;

    /* Start calculating the size */
    msize = sizeof (struct TempMenu);

    /* Do we have a name? */
    if (name == NULL)
    {
	name = "*";
    }

    /* Figure in the size of the name */
    msize += (strlen (name) + 2L);

    /* Create a menu item node */
    if (tm = (struct TempMenu *) AllocVec (msize, MEMF_CLEAR))
    {
	struct NewMenu *nm = &(tm->tm_NewMenu);

	/* Do we have a name? */
	if (name)
	{
	    /* Set up the name of the node */
	    tm->tm_Node.ln_Name = MEMORY_FOLLOWING (tm);
	    strcpy (tm->tm_Node.ln_Name, name);
	}

	/* Set the menu item type */
	tm->tm_Node.ln_Type = nm->nm_Type = type;

	/* Copy the label */
	if (label)
	{
	    if (label == NM_BARLABEL)
	    {
		nm->nm_Label = NM_BARLABEL;
		DV (kprintf ("nm_barlabel\n"));
	    }
	    else if (nm->nm_Label = (STRPTR) AllocVec ((strlen (label) + 2L), MEMF_CLEAR))
	    {
		strcpy (nm->nm_Label, label);
		DV (kprintf ("nm [%s], [%s]\n", nm->nm_Label, label));
	    }
	    else
	    {
		DV (kprintf ("unable to allocate room for %s\n", label));
	    }
	}

	/* Copy the command key */
	if (cmdkey && (cmdkey[0] != 0) && (cmdkey[0] != ' '))
	{
	    if (nm->nm_CommKey = (STRPTR) AllocVec ((strlen (cmdkey) + 2L), MEMF_CLEAR))
	    {
		strcpy (nm->nm_CommKey, cmdkey);
	    }
	}

	/* Copy the command */
	if (cmd &&
	    (nm->nm_UserData = AllocVec ((strlen (cmd) + 2L), MEMF_CLEAR)))
	{
	    strcpy (nm->nm_UserData, cmd);
	}

	/* Add the menu item to the menu list */
	if (list)
	{
	    DI (kprintf (" ADDED\n"));
	    AddTail (list, (struct Node *) tm);
	}
    }
    else
    {
	DI (kprintf (" FAILED\n"));
    }

    return (tm);
}

VOID
DeleteMenuItem (struct TempMenu * tm)
{
    struct NewMenu *tnm = &(tm->tm_NewMenu);

    Remove ((struct Node *) tm);

    if (tnm->nm_Label && (tnm->nm_Label != NM_BARLABEL))
	FreeVec (tnm->nm_Label);

    if (tnm->nm_CommKey)
	FreeVec (tnm->nm_CommKey);

    if (tnm->nm_UserData)
	FreeVec (tnm->nm_UserData);

    FreeVec (tm);
}

VOID
FreeTextTableMenu (struct AppInfo * ai, STRPTR cmd, struct TagItem * attrs)
{
    struct WinNode *wn;

    DQ(kprintf ("FreeTextTableMenu     0x%lx\n", FreeTextTableMenu));

    if (wn = GetWinNode (ai, attrs))
    {
	struct List *list = &(wn->wn_MenuList);
	struct Node *node, *nxtnode;

	DQ(kprintf ("free menus for [%s]\n", wn->wn_Header.mho_Node.ln_Name));

	if (wn->wn_GTMenu)
	{
	    FreeMenus (wn->wn_Menu);
	    wn->wn_Menu = NULL;
	}

	/* Make sure we have a list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    node = list->lh_Head;
	    while (nxtnode = node->ln_Succ)
	    {
		DQ(kprintf ("delete menu [%s]\n", node->ln_Name));
		DeleteMenuItem ((struct TempMenu *) node);

		node = nxtnode;
	    }

	    /* Free the NewMenu block */
	    if (wn->wn_GTMenu)
	    {
		DQ(kprintf ("delete menu block\n"));
		FreeVec (wn->wn_GTMenu);
		wn->wn_GTMenu = NULL;
	    }
	}
    }
}

VOID
AllocTextTableMenuA (struct AppInfo * ai, STRPTR cmd, struct TagItem * attrs)
{
    struct MsgHandler *mh;
    struct WinNode *wn;

    /* Try getting a pointer to the message handler */
    if (!(mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE)))
    {
	mh = (struct MsgHandler *) GetTagData (APSH_MsgHandler, NULL, attrs);
    }

    /* Get the window node to attach the menus to */
    if (mh && (wn = GetWinNode (ai, attrs)))
    {
	struct AppLocContext	*alc;
	struct Library		*LocaleBase = NULL;
	struct AppString	*table = NULL;
	struct Catalog		*cat = NULL;
	LONG				tid = 0L,
						nid = 0L;


	struct MHObject *mho = &(mh->mh_Header);
	struct IDCMPInfo *ii = mho->mho_SysData;
	struct RDArgs *ra = &(ai->ai_CmdRDA);	/* ii->ii_MRA); */
	struct List *list = &(wn->wn_MenuList);
	struct TagItem *mtag;
	WORD cntr, start;
	UBYTE level = 0;
	WORD items = 0;
	BOOL going = TRUE;
	STRPTR cmd = NULL;
	UWORD mnum, inum, snum;

	mnum = NOMENU;
	inum = snum = 0;

	/* Get the Locale Context info */
    alc = ai->ai_ALC;
    alc->alc_nID = 0L;

    /* Get the starting point */
    start = (WORD) GetTagData (APSH_TTMenu, 0L, attrs);

	if (start > 0)
	{
	    STRPTR name, label, key, command;
	    struct TempMenu *bar = NULL;
	    struct TempMenu *tm = NULL;

	    DB (kprintf ("Build menu from text table: start %ld\n", (LONG) start));
	    for(tid=start;(going&&(tid>=0));)
	    {
		if (cmd = GT_Loc(ai,APSH_USER_ID,tid))
		{
		    struct RDArgs *pa;
		    register LONG i;

		    /* Build the command line to parse */
		    sprintf (ai->ai_TempText, "%s\n", cmd);

		    /* Prepare the ReadArgs */
		    ra->RDA_Source.CS_Buffer = ai->ai_TempText;
		    ra->RDA_Source.CS_Length = strlen (ai->ai_TempText);
		    ra->RDA_Source.CS_CurChr = 0;

		    /* Clear the option buckets */
		    for (i = 0; i < ii->ii_NumOpts; i++)
		    {
			ii->ii_Options[i] = NULL;
		    }

		    /* Parse the command line */
		    DQ(kprintf ("parse %s\n", cmd));
		    if (pa = ReadArgs (ii->ii_Template, ii->ii_Options, ra))
		    {
			label = (STRPTR) ii->ii_Options[6];
			key = (STRPTR) ii->ii_Options[7];
			command = (STRPTR) ii->ii_Options[8];

			if (name = (STRPTR) ii->ii_Options[0])	/* Window */
			{
			    DV (kprintf ("Building menus for %s\n", name));
			}
			else if (name = (STRPTR) ii->ii_Options[1])	/* Title */
			{
			    level = NM_TITLE;

			    if (mnum == NOMENU)
			    {
				mnum = 0;
			    }
			    else
			    {
				mnum++;
			    }

			    inum = NOITEM;
			    snum = NOSUB;

			    if (tm = CreateMenuItem (list, name, level, label, NULL, NULL))
			    {
				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				/* Clear the bar indicator */
				bar = NULL;

				items++;
			    }
			}
			else if (name = (STRPTR) ii->ii_Options[2])	/* Item */
			{
			    level = NM_ITEM;

			    if (inum == NOITEM)
			    {
				inum = 0;
			    }
			    else
			    {
				inum++;
			    }

			    snum = NOSUB;

			    /* Was the previous item a separator bar? */
			    if (bar && (bar->tm_Node.ln_Type == NM_SUB))
			    {
				struct NewMenu *nm = &(bar->tm_NewMenu);

				/* Change the type */
				bar->tm_Node.ln_Type = nm->nm_Type = NM_ITEM;
			    }

			    if (tm = CreateMenuItem (list, name, level, label, key, command))
			    {
				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				/* Prepare the function for disable */
				SetupMenuDisable (ai, wn, command, tm);

				/* Clear the bar indicator */
				bar = NULL;

				items++;
			    }
			}
			else if (name = (STRPTR) ii->ii_Options[3])	/* Sub */
			{
			    level = NM_SUB;

			    if (snum == NOSUB)
			    {
				snum = 0;
			    }
			    else
			    {
				snum++;
			    }

			    if (tm = CreateMenuItem (list, name, level, label, key, command))
			    {
				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				/* Prepare the function for disable */
				SetupMenuDisable (ai, wn, command, tm);

				/* Clear the bar indicator */
				bar = NULL;

				items++;
			    }
			}
			else if (ii->ii_Options[4])	/* Bar */
			{
			    if (level == NM_ITEM)
			    {
				inum++;
			    }
			    else
			    {
				snum++;
			    }

			    if (tm = CreateMenuItem (list, NULL, level, NM_BARLABEL, NULL, NULL))
			    {
				bar = tm;

				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				items++;
			    }
			}
			else if (ii->ii_Options[5])	/* End */
			{
			    level = NM_END;

			    if (CreateMenuItem (list, NULL, level, NULL, NULL, NULL))
			    {
				items++;
			    }

			    going = FALSE;
			}

			/* Free the command line */
			FreeArgs (pa);
		    }
		    else
		    {
			DI (kprintf ("failed on [%s]\n", cmd));
		    }
		}
		else
		{
		    level = NM_END;

		    if (CreateMenuItem (list, NULL, level, NULL, NULL, NULL))
		    {
			items++;
		    }

		    going = FALSE;
		}
		tid = alc->alc_nID;
	    } /* end loop */

	    /* Make sure we have something to make a menu from */
	    if (items)
	    {
		struct NewMenu *nm;
		ULONG msize;

		msize = sizeof (struct NewMenu) * items;

		if (nm = (struct NewMenu *) AllocVec (msize, MEMF_CLEAR))
		{
		    struct Node *node = list->lh_Head;
		    struct NewMenu *tnm = nm;
		    struct TempMenu *tm;
		    WORD i;

		    DI (kprintf ("Make menu of %ld items\n", (LONG) items));

		    for (i = 0; i < items; i++)
		    {
			tm = (struct TempMenu *) node;

			/* Copy the contents over */
			*tnm = *(&(tm->tm_NewMenu));

			/* Next destination NewMenu structure */
			tnm++;

			/* Get the next node */
			node = node->ln_Succ;
		    }

		    /* Remember who we are */
		    wn->wn_GTMenu = nm;
		    wn->wn_Menu = NULL;
		}		/* End of if create temporary menu block */
		else
		{
		    /* Free the list... */
		}
	    }			/* End of if items */
	}			/* End of if starting point */
    }				/* End of if get window node */
}

VOID
AllocTextTableMenuB (struct AppInfo * ai, STRPTR cmd, struct TagItem * attrs)
{
    struct MsgHandler *mh;
    struct WinNode *wn;

    /* Try getting a pointer to the message handler */
    if (!(mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE)))
    {
	mh = (struct MsgHandler *) GetTagData (APSH_MsgHandler, NULL, attrs);
    }

    /* Get the window node to attach the menus to */
    if (mh && (wn = GetWinNode (ai, attrs)))
    {
	struct MHObject *mho = &(mh->mh_Header);
	struct IDCMPInfo *ii = mho->mho_SysData;
	struct RDArgs *ra = &(ai->ai_CmdRDA);	/* ii->ii_MRA); */
	struct List *list = &(wn->wn_MenuList);
	struct TagItem *mtag;
	WORD cntr, start;
	UBYTE level = 0;
	WORD items = 0;
	BOOL going;
	STRPTR cmd;
	STRPTR *table;
	UWORD mnum, inum, snum;

	mnum = NOMENU;
	inum = snum = 0;

	if (mtag = FindTagItem (APSH_MenuTable, attrs))
	{
	    table = (STRPTR *) mtag->ti_Data;
	    start = 0;
	}
	else
	{
	    table = ai->ai_DefText1;
	    if (wn->wn_Flags & APSH_WINF_LOCALTEXT)
	    {
		if (wn->wn_OI->oi_TextTable)
		{
		    table = wn->wn_OI->oi_TextTable;
		}
	    }

	    /* Get the starting point */
	    start = (WORD) GetTagData (APSH_TTMenu, -1L, attrs);
	}

	if (start >= 0)
	{
	    STRPTR name, label, key, command;
	    struct TempMenu *bar = NULL;
	    struct TempMenu *tm;

	    DB (kprintf ("Build menu from text table: start %ld\n", (LONG) start));
	    for (cntr = start, going = TRUE; going; cntr++)
	    {
		if (cmd = table[cntr])
		{
		    struct RDArgs *pa;
		    register LONG i;

		    /* Build the command line to parse */
		    sprintf (ai->ai_TempText, "%s\n", cmd);

		    /* Prepare the ReadArgs */
		    ra->RDA_Source.CS_Buffer = ai->ai_TempText;
		    ra->RDA_Source.CS_Length = strlen (ai->ai_TempText);
		    ra->RDA_Source.CS_CurChr = 0;

		    /* Clear the option buckets */
		    for (i = 0; i < ii->ii_NumOpts; i++)
		    {
			ii->ii_Options[i] = NULL;
		    }

		    /* Parse the command line */
		    DQ(kprintf ("parse %s\n", cmd));
		    if (pa = ReadArgs (ii->ii_Template, ii->ii_Options, ra))
		    {
			label = (STRPTR) ii->ii_Options[6];
			key = (STRPTR) ii->ii_Options[7];
			command = (STRPTR) ii->ii_Options[8];

			if (name = (STRPTR) ii->ii_Options[0])	/* Window */
			{
			    DV (kprintf ("Building menus for %s\n", name));
			}
			else if (name = (STRPTR) ii->ii_Options[1])	/* Title */
			{
			    level = NM_TITLE;

			    if (mnum == NOMENU)
			    {
				mnum = 0;
			    }
			    else
			    {
				mnum++;
			    }

			    inum = NOITEM;
			    snum = NOSUB;

			    if (tm = CreateMenuItem (list, name, level, label, NULL, NULL))
			    {
				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				/* Clear the bar indicator */
				bar = NULL;

				items++;
			    }
			}
			else if (name = (STRPTR) ii->ii_Options[2])	/* Item */
			{
			    level = NM_ITEM;

			    if (inum == NOITEM)
			    {
				inum = 0;
			    }
			    else
			    {
				inum++;
			    }

			    snum = NOSUB;

			    /* Was the previous item a separator bar? */
			    if (bar && (bar->tm_Node.ln_Type == NM_SUB))
			    {
				struct NewMenu *nm = &(bar->tm_NewMenu);

				/* Change the type */
				bar->tm_Node.ln_Type = nm->nm_Type = NM_ITEM;
			    }

			    if (tm = CreateMenuItem (list, name, level, label, key, command))
			    {
				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				/* Prepare the function for disable */
				SetupMenuDisable (ai, wn, command, tm);

				/* Clear the bar indicator */
				bar = NULL;

				items++;
			    }
			}
			else if (name = (STRPTR) ii->ii_Options[3])	/* Sub */
			{
			    level = NM_SUB;

			    if (snum == NOSUB)
			    {
				snum = 0;
			    }
			    else
			    {
				snum++;
			    }

			    if (tm = CreateMenuItem (list, name, level, label, key, command))
			    {
				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				/* Prepare the function for disable */
				SetupMenuDisable (ai, wn, command, tm);

				/* Clear the bar indicator */
				bar = NULL;

				items++;
			    }
			}
			else if (ii->ii_Options[4])	/* Bar */
			{
			    if (level == NM_ITEM)
			    {
				inum++;
			    }
			    else
			    {
				snum++;
			    }

			    if (tm = CreateMenuItem (list, NULL, level, NM_BARLABEL, NULL, NULL))
			    {
				bar = tm;

				tm->tm_MenuNum = FULLMENUNUM (mnum, inum, snum);

				items++;
			    }
			}
			else if (ii->ii_Options[5])	/* End */
			{
			    level = NM_END;

			    if (CreateMenuItem (list, NULL, level, NULL, NULL, NULL))
			    {
				items++;
			    }

			    going = FALSE;
			}

			/* Free the command line */
			FreeArgs (pa);
		    }
		    else
		    {
			DI (kprintf ("failed on [%s]\n", cmd));
		    }
		}
		else
		{
		    level = NM_END;

		    if (CreateMenuItem (list, NULL, level, NULL, NULL, NULL))
		    {
			items++;
		    }

		    going = FALSE;
		}
	    }

	    /* Make sure we have something to make a menu from */
	    if (items)
	    {
		struct NewMenu *nm;
		ULONG msize;

		msize = sizeof (struct NewMenu) * items;

		if (nm = (struct NewMenu *) AllocVec (msize, MEMF_CLEAR))
		{
		    struct Node *node = list->lh_Head;
		    struct NewMenu *tnm = nm;
		    struct TempMenu *tm;
		    WORD i;

		    DI (kprintf ("Make menu of %ld items\n", (LONG) items));

		    for (i = 0; i < items; i++)
		    {
			tm = (struct TempMenu *) node;

			/* Copy the contents over */
			*tnm = *(&(tm->tm_NewMenu));

			/* Next destination NewMenu structure */
			tnm++;

			/* Get the next node */
			node = node->ln_Succ;
		    }

		    /* Remember who we are */
		    wn->wn_GTMenu = nm;
		    wn->wn_Menu = NULL;
		}		/* End of if create temporary menu block */
		else
		{
		    /* Free the list... */
		}
	    }			/* End of if items */
	}			/* End of if starting point */
    }				/* End of if get window node */
}

VOID AllocTextTableMenu (struct AppInfo * ai, STRPTR cmd, ULONG data,...)
{

    AllocTextTableMenuA (ai, cmd, (struct TagItem *) & data);
}

struct WinNode *
AllocWinNode (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * attrs)
{
    struct WinNode *wn;
    STRPTR name;
    ULONG msize;

    DA (kprintf ("AllocWinNode          0x%lx\n", AllocWinNode));

    /* Previous function made sure we had a name tag */
    name = (STRPTR) GetTagData (APSH_NameTag, NULL, attrs);

    /* Calculate space needed for a window node */
    DB (kprintf ("AllocWinNode [%s]\n", name));
    msize = sizeof (struct WinNode) + strlen (name) + 2L;

    /* Allocate the window node */
    if (wn = (struct WinNode *) AllocVec (msize, MEMF_CLEAR))
    {
	struct MHObject *mho = &(mh->mh_Header);
	struct MHObject *who = &(wn->wn_Header);
	struct IDCMPInfo *md = mho->mho_SysData;
	struct List *list = &(mho->mho_ObjList);
	struct ObjectInfo *oi = NULL;
	struct TagItem *ti = NULL;
	struct TextAttr *textattr = NULL;
	ULONG tagv = TAG_IGNORE, tagd = 0L;
	struct Screen *screen = NULL;
	struct Object *objarray;
	struct TagItem *wattrs;
	struct NewWindow *nw;
	struct AppLocContext *alc = NULL;

	/* Initialize the common header information */
	DB (kprintf ("created WinNode for %s\n", name));
	who->mho_Node.ln_Type = APSH_MHO_WINDOW;
	who->mho_Node.ln_Pri = APSH_MH_HANDLER_P;
	who->mho_Node.ln_Name = MEMORY_FOLLOWING (wn);
	strcpy (who->mho_Node.ln_Name, name);

	/* Initialize the parent */
	who->mho_Parent = mho;

	/* Establish the base ID */
	who->mho_ID = APSH_IDCMP_ID;

	/* Point the object data at the correct place */
	who->mho_SysData = wn->wn_Win;

	/* Initialize the window object list */
	NewList (&(who->mho_ObjList));

	/* Set up the master information */
	wn->wn_AI = ai;

	/* Clone the starting attributes */
	DB (kprintf ("before (we) CloneTagItems\n"));
	wn->wn_WinEnv = CloneTagItems (attrs);
	DB (kprintf ("after (we) CloneTagItems\n"));

	/* Add the window node to the list in alphabetical order */
	AlphaEnqueue (list, (struct Node *) wn);

	/* Set the current object pointer */
	mho->mho_CurNode = (struct MHObject *) & (wn->wn_Header);

	/* See if the asked to be placed on the public screen */
	if (ti = FindTagItem (WA_PubScreen, attrs))
	{
	    /* See if they specified a screen */
	    if (ti->ti_Data == NULL)
	    {
		/* Set the data */
		DB (kprintf ("WA_PubScreen was NULL, now 0x%lx\n", md->ii_Screen));
		ti->ti_Data = (LONG) md->ii_Screen;
	    }
	    else
	    {
		DB (kprintf ("WA_PubScreen 0x%lx\n", ti->ti_Data));
	    }
	}
	else
	{
	    DB (kprintf ("No WA_PubScreen specified\n"));
	}

/*********************************/
/* Set up the Window information */
/*********************************/

	if (ai->ai_Pri_Ret == RETURN_OK)
	{
	    /* Get the font information */
	    textattr = (struct TextAttr *)
	      GetTagData (APSH_TextAttr, (ULONG) & (md->ii_FontAttr), attrs);

	    /* Get the destination screen */
	    screen = (struct Screen *)
	      GetTagData (APSH_Screen, (ULONG) md->ii_Screen, attrs);

	    /* Pass Locale context to AppObjects */
	    alc = ai->ai_ALC;

	    /* Did they pass a NewWindow structure? */
	    if (nw = (struct NewWindow *) GetTagData (APSH_NewWindow, NULL, attrs))
	    {
		/* Yes they did, copy their information over */
		wn->wn_NW = *nw;
		nw = &(wn->wn_NW);
	    }
	    else
	    {
		struct Object *obj;

		/* Nope, set up our own */
		nw = &(wn->wn_NW);
		nw->TopEdge = (screen) ?
		  (screen->WBorTop + (screen->Font->ta_YSize + 1)) : 11;
		nw->DetailPen = 0;
		nw->BlockPen = 1;

		/* Get the flags */
		nw->Flags = GetTagData (APSH_DefWinFlags, md->ii_Flags, attrs);

		/* See if we have a sizing gadget... */
		if (obj = GetTagData (APSH_Objects, NULL, attrs))
		{
		    if (GetTagData (WA_SizeGadget, NULL, obj->o_Tags))
		    {
			nw->Flags |= WINDOWSIZING;
		    }
		}
		else if (GetTagData (WA_SizeGadget, NULL, attrs))
		{
		    nw->Flags |= WINDOWSIZING;
		}

		nw->MaxWidth = nw->MaxHeight = ~0;
		nw->IDCMPFlags = md->ii_FastIDCMP;
	    }

	    /* Set the screen type */
	    nw->Type = md->ii_ScrType;
	    nw->Screen = screen;

	    /* Now, honor the user's preference */
	    nw = LoadSnapShot (nw, ai->ai_ConfigDir, name);

	    DW (kprintf ("Load snapshot %s : ", name));
	    DW (kprintf (" %ld,%ld,%ld,%ld\n", (LONG)nw->LeftEdge, (LONG)nw->TopEdge,
					      (LONG)nw->Width, (LONG)nw->Height));

	    /* Get the window attributes */
	    wattrs = (struct TagItem *) GetTagData (APSH_NewWindowTags, NULL, attrs);

	    /* See if the gadgets are described by an object array */
	    if (objarray = (struct Object *) GetTagData (APSH_Objects, NULL, attrs))
	    {
		/* Create the object list */
		DO (kprintf ("NewObjList\n"));
		if (oi = NewObjList (
					APSH_Screen, (LONG) screen,
					APSH_TextAttr, (LONG) textattr,
					APSH_DefText, (LONG) alc,
					tagv, tagd,
					APSH_NewWindow, (LONG) nw,
					APSH_NewWindowTags, (LONG) wattrs,
					APSH_Objects, (LONG) objarray,
					TAG_DONE))
		{
		    /* Remember this moment */
		    DO (kprintf ("NewObjList returns with 0x%lx\n", oi));
		    wn->wn_OI = oi;
		    oi->oi_SystemData = (APTR) wn;

		    /* ... and this one also */
		    wattrs = oi->oi_WindowAttrs;

		    /* Handle the creation setup of the objects */
		    DO (kprintf ("Handle creation setup of objects\n"));
		    HandleList (&(oi->oi_ObjList), doObjList, ai, EG_CREATE);
		}
		else
		{
		    /* Unable to create the object array */
		    DO (kprintf ("Couldn't set up object array\n"));
		    ai->ai_Pri_Ret = RETURN_FAIL;
		    ai->ai_Sec_Ret = APSH_CLDNT_OPEN;
		    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID,
					       ai->ai_Sec_Ret, name);
		}

		DO (kprintf ("Done setting up objects\n"));
	    }
	    /* End of if object array */

	    /* Prepare the NewWindow attributes */
	    wn->wn_NWTags = MakeNewTagList (
					       WA_AutoAdjust, TRUE,
					       TAG_MORE, wattrs,
					       TAG_DONE);
	}

/**********************************/
/* Initialize the IDCMP callbacks */
/**********************************/

	if (ai->ai_Pri_Ret == RETURN_OK)
	{
	    struct NewWindow *nw = &(wn->wn_NW);
	    ULONG cntr;
	    WORD temp;

	    /* Set up the slow IDCMP flags */
	    wn->wn_SlowIDCMP = (md->ii_SlowIDCMP | REFRESHWINDOW);

	    /* Make sure refresh events always make it through */
	    nw->IDCMPFlags |= REFRESHWINDOW;

	    /* Setup all the callback functions */
	    DB (kprintf ("create IDCMP callbacks\n"));
	    for (cntr = APSH_SizeVerify; cntr <= APSH_VanillaKey; cntr++)
	    {
		temp = (WORD) (cntr - APSH_SizeVerify);

		/* See if they have a callback specified for this event */
		if (ti = FindTagItem (cntr, attrs))
		{
		    /* Use the unique callback */
		    wn->wn_Funcs[temp] = ti->ti_Data;
		}
		else
		{
		    /* Use the application default callback */
		    wn->wn_Funcs[temp] = md->ii_Funcs[temp];
		}

		/* Is there a command assigned to this event? */
		if (wn->wn_Funcs[temp])
		{
		    switch (cntr)
		    {
			case APSH_VanillaKey:
			    nw->IDCMPFlags |= VANILLAKEY;
			    break;

			case APSH_SizeVerify:
			    nw->IDCMPFlags |= SIZEVERIFY;
			    break;

			case APSH_MenuVerify:
			    nw->IDCMPFlags |= MENUVERIFY;
			    break;

			case APSH_MouseButtons:
			    nw->IDCMPFlags |= MOUSEBUTTONS;
			    wn->wn_SlowIDCMP |= MOUSEBUTTONS;
			    nw->Flags |= REPORTMOUSE;
			    break;

			case APSH_IntuiTicks:
			    nw->IDCMPFlags |= INTUITICKS;
			    wn->wn_SlowIDCMP |= INTUITICKS;
			    break;

			case APSH_NewSize:
			    nw->IDCMPFlags |= NEWSIZE;
			    wn->wn_SlowIDCMP |= NEWSIZE;
			    break;

			case APSH_ChangeWindow:
			    nw->IDCMPFlags |= CHANGEWINDOW;
			    wn->wn_SlowIDCMP |= CHANGEWINDOW;
			    break;
		    }
		}		/* Command assigned to event? */
	    }

	    /* Set the fast IDCMP flags */
	    wn->wn_FastIDCMP = nw->IDCMPFlags;
	}

/************************************/
/* Initialize keystroke information */
/************************************/

	if (ai->ai_Pri_Ret == RETURN_OK)
	{
	    struct KeyboardCMD *kba;

	    /* Get a pointer to the keystroke table */
	    kba = (struct KeyboardCMD *)
	      GetTagData (APSH_HotKeys, NULL, attrs);

	    /* Initialize the keystroke array */
	    DB (kprintf ("setup_key_array\n"));
	    if (!(setup_key_array (mh, kba)))
	    {
		/* Unable to create the keyboard array */
		DB (kprintf ("Couldn't set up keyboard array\n"));
		ai->ai_Pri_Ret = RETURN_FAIL;
		ai->ai_Sec_Ret = APSH_SETUP_HOTKEYS;
		ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID,
					   ai->ai_Sec_Ret, NULL);
	    }
	}

/*******************************/
/* Initialize menu information */
/*******************************/

	if (ai->ai_Pri_Ret == RETURN_OK)
	{
	    /* Initialize the menu list */
	    NewList (&(wn->wn_MenuList));

	    /* See if we have an Intuition menu array */
	    wn->wn_Menu = (struct Menu *) GetTagData (APSH_Menu, NULL, attrs);

	    /* See if we have a GadTools menu array */
	    wn->wn_GTMenu = (struct NewMenu *) GetTagData (APSH_GTMenu, NULL, attrs);

	    /* See if we have a text table menu array */
	    if (FindTagItem (APSH_TTMenu, attrs) ||
		FindTagItem (APSH_MenuTable, attrs))
	    {
		AllocTextTableMenu (ai, NULL,
				    APSH_MsgHandler, (LONG) mh,
				    TAG_MORE, (LONG) attrs,
				    TAG_DONE);

		/* Show that we have a text table menu */
		wn->wn_Flags |= APSH_WINF_TEXTMENU;
	    }

	    /* Do we have a GadTools menu array? */
	    if (wn->wn_GTMenu)
	    {
		/* Create the Intuition menu strip */
		wn->wn_Menu = CreateMenus (wn->wn_GTMenu,
					   GTMN_FrontPen, 0L,
					   TAG_DONE);

		/* Should make sure we have this... */
		LayoutMenus (wn->wn_Menu, oi->oi_VisualInfo, TAG_DONE);
	    }
	}
    }
    else
    {
	/* Not enough memory */
	DB (kprintf ("Not enough memory\n"));
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
    }

    DB (kprintf ("AllocWinNode exit 0x%lx\n", wn));
    return (wn);
}

BOOL make_idcmp (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct List *list = &(mho->mho_ObjList);
    struct TagItem *attrs;
    BOOL retval = FALSE;

    DA (kprintf ("make_idcmp            0x%lx\n", make_idcmp));

    /* Make sure we have an attribute list to work from */
    if (attrs = (struct TagItem *) GetTagData (APSH_WindowEnv, (ULONG) tl, tl))
    {
	STRPTR name;

	/* We have to have a name for the window that we're going to make */
	if (name = (STRPTR) GetTagData (APSH_NameTag, NULL, attrs))
	{
	    struct WinNode *wn;

	    /* See if the window descriptor already exists */
	    if (wn = (struct WinNode *) FindNameI (list, name))
	    {
		/* Set the current object pointer */
		mho->mho_CurNode = (struct MHObject *) & (wn->wn_Header);

		/* Successfull... */
		retval = TRUE;
	    }
	    else
	    {
		/* Make a new window descriptor */
		if (wn = AllocWinNode (ai, mh, attrs))
		{
		    /* Did it set up all right? */
		    if (ai->ai_Pri_Ret == RETURN_OK)
		    {
			/* Set the current object pointer */
			DB (kprintf ("Window has been made\n"));
			mho->mho_CurNode = (struct MHObject *) & (wn->wn_Header);

			/* Successfull... */
			retval = TRUE;
		    }
		    else
		    {
			DB (kprintf ("Failure in window setup\n"));
		    }
		}		/* If allocate window node */
	    }			/* If window node */
	}
    }

    return (retval);
}

BOOL open_idcmp (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct IDCMPInfo *md = mho->mho_SysData;
    BOOL retval = FALSE;

    /* Clear the error return */
    DB (kprintf ("Primary return = %ld\n", ai->ai_Pri_Ret));
    ai->ai_Pri_Ret = RETURN_OK;

    /* Do we have a screen? */
    DB (kprintf ("open_idcmp            0x%lx\n", open_idcmp));
    DB (kprintf ("md->ii_Screen         0x%lx\n", md->ii_Screen));
    if (!(md->ii_Screen))
    {
	struct TextAttr *ta;

	/* Make sure that we're able to open the environment */
	DB (kprintf ("OpenEnvironment\n"));
	if (!(OpenEnvironment (ai, mh, tl)))
	{
	    return (retval);
	}

	/* Get the default text attribute */
	ta = md->ii_Screen->Font;

	/* Set up the font attribute */
	ta = (struct TextAttr *) GetTagData (APSH_TextAttr, (LONG) ta, tl);

	/* Copy the font attribute */
	md->ii_FontAttr = *ta;

	/* Make this pointer generally available */
	ai->ai_Screen = md->ii_Screen;
    }

    /* Prepare the window environment */
    DB (kprintf ("make_idcmp\n"));
    if (make_idcmp (ai, mh, tl))
    {
	struct WinNode *wn;

	/* Do we have a window node? */
	if (wn = (struct WinNode *) mho->mho_CurNode)
	{
	    /* Is the window open? */
	    if (wn->wn_Win == NULL)
	    {
		struct TagItem *wattrs = wn->wn_NWTags, *ti;
		struct NewWindow *nw = &(wn->wn_NW);
		ULONG idcmp = nw->IDCMPFlags;
		struct Window *win;

		DOW(kprintf("WNC\tNW: x(%d),y(%d),w(%d),h(%d)\n",
					nw->LeftEdge,
					nw->TopEdge,
					nw->Width,
					nw->Height));
		DOW(KPT("WNC\tOWTL:",wattrs));


		/* Clear the IDCMP flags */
		nw->IDCMPFlags = NULL;

		/* Before open function */
		{
		    WORD tmp = (WORD) (APSH_WinBOpen - APSH_SizeVerify);
		    ULONG FuncID;

		    if (FuncID = wn->wn_Funcs[tmp])
		    {
			PerfFunc (ai, FuncID, NULL, tl);
		    }
		}

		if (ti = FindTagItem (WA_PubScreenName, wattrs))
		{
		    if (ti->ti_Data)
		    {
			DQ(kprintf ("WA_PubScreenName %s\n", ti->ti_Data));
			nw->Screen = NULL;
		    }
		}
		else if (ti = FindTagItem (WA_PubScreen, wattrs))
		{
		    if (ti->ti_Data)
		    {
			DQ(kprintf ("WA_PubScreen 0x%lx\n", ti->ti_Data));
			nw->Screen = NULL;
		    }
		}
		else
		{
		    DQ(kprintf ("No WA_Pub in wattrs\n"));
		}

		/* Open the window */
		DOW(kprintf("POW\tNW: x(%d),y(%d),w(%d),h(%d)\n",
					nw->LeftEdge,
					nw->TopEdge,
					nw->Width,
					nw->Height));
		DOW(KPT("POW\tOWTL:",wattrs));
		if (win = OpenWindowTagList (nw, wattrs))
		{
		    /* Remember the new window */
		    DB (kprintf ("Window = 0x%lx\n", win));
		    win->MoreFlags |= WMF_APSH_WINDOW;
		    wn->wn_Win = win;

		    /* Point back to ourself */
		    win->UserData = (APTR) wn->wn_OI;

		    /* Refresh the GadTools gadgets */
		    DB (kprintf ("RefreshWindow\n"));
		    GT_RefreshWindow (win, NULL);

		    /* Do we have a menu? */
		    if (wn->wn_Menu)
		    {
			DB (kprintf ("SetMenuStrip\n"));
			SetMenuStrip (win, wn->wn_Menu);
		    }

		    /* Set the IDCMP port up */
		    win->UserPort = mh->mh_Port;
		    ModifyIDCMP (win, idcmp);

		    /* Increment the open window counter */
		    md->ii_NumWins++;

		    /* Set the current window information */
		    SetCurrentWindow (ai, wn,
		                      APSH_MsgHandler, mh,
		                      TAG_DONE);

		    /* Make the window an AppWindow */
		    HandlerFunc (ai,
				 APSH_Handler, "WB",
				 APSH_Command, APSH_MH_OPEN,
				 APSH_NameTag, wn->wn_Header.mho_Node.ln_Name,
				 TAG_DONE);

		    /* Successful open */
		    retval = TRUE;

		    /* Set up the disable object pointers */
		    DB (kprintf ("SetupDisable 0x%lx 0x%lx\n", ai, wn));
		    SetupDisable (ai, wn);

		    /* After open function */
		    {
			WORD tmp = (WORD) (APSH_WinAOpen - APSH_SizeVerify);
			ULONG FuncID;

			if (FuncID = wn->wn_Funcs[tmp])
			{
			    PerfFunc (ai, FuncID, NULL, tl);
			}
		    }
		}
		else
		{
		    DB (kprintf ("Couldn't open window!!\n"));
		}

		/* Restore the flags */
		nw->IDCMPFlags = idcmp;
	    }
	    else
	    {
		/* Show that the window is open */
		DB (kprintf ("Window is already open\n"));
		retval = TRUE;
	    }
	}
	else
	{
	    DB (kprintf ("No WinNode\n"));
	}
    }
    else
    {
	DB (kprintf ("Couldn't make_idcmp\n"));
    }

    return (retval);
}

BOOL close_idcmp (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct IDCMPInfo *md = mho->mho_SysData;
    struct TagItem *attrs;
    struct WinNode *wn;
    BOOL retval = FALSE;
    STRPTR name = NULL;

    DA (kprintf ("close_idcmp           0x%lx\n", close_idcmp));

    /* Get the correct attribute list to use */
    attrs = (struct TagItem *) GetTagData (APSH_WindowEnv, (ULONG) tl, tl);

    /* Do we have a window to close? */
    if (wn = GetWinNode (ai, attrs))
    {
	struct Window *win;

	/* Get the name of current node */
	name = (STRPTR) (wn->wn_Header.mho_Node.ln_Name);

	/* Make sure the window is actually open... */
	if (win = wn->wn_Win)
	{
	    struct ObjectInfo *oi;

	    /* See if we're using AppObjects */
	    if (oi = wn->wn_OI)
	    {
		/* Abort any keystrokes that are still down */
		AbortKeystroke (oi, win);

		/* Do we have a gadget list? */
		if (oi->oi_GList)
		{
		    /* Remove the gadget list */
		    RemoveGList (win, oi->oi_GList, -1);
		}
	    }			/* End of if appobjects */

	    /* Are we an AppWindow? */
	    if (wn->wn_Flags & APSH_WINF_APPWIN)
	    {
		/* Remove the AppWindow */
		HandlerFunc (ai,
			     APSH_Handler, "WB",
			     APSH_Command, APSH_MH_CLOSE,
			     APSH_NameTag, name,
			     TAG_DONE);

	    }			/* End of if AppWindow */

	    /* In memory snapshot of window position */
	    {
		struct NewWindow *nw = &(wn->wn_NW);
		struct TagItem *ti;

		nw->LeftEdge = win->LeftEdge;
		nw->TopEdge = win->TopEdge;
		nw->Width = win->Width - win->BorderLeft - win->BorderRight;
		nw->Height = win->Height - win->BorderTop - win->BorderBottom;

		/* Set the width */
		if (ti = FindTagItem (WA_InnerWidth, wn->wn_NWTags))
		{
		    ti->ti_Data = (LONG) nw->Width;
		    DC (kprintf ("InnerWidth %ld\n", ti->ti_Data));
		}

		/* Set the height */
		if (ti = FindTagItem (WA_InnerHeight, wn->wn_NWTags))
		{
		    ti->ti_Data = (LONG) nw->Height;
		    DC (kprintf ("InnerHeight %ld\n", ti->ti_Data));
		}
	    }			/* End of snapshot window */

	    /* Is the window locked? */
	    if (wn->wn_Flags & APSH_WINF_LOCKGUI)
	    {
		/* End the locking request */
		DC (kprintf ("EndRequest\n"));
		EndRequest (&(wn->wn_BR), win);

		/* Clear the lock flag */
		wn->wn_Flags &= ~APSH_WINF_LOCKGUI;

	    }			/* End of if window locked */

	    /* Before close function */
	    {
		WORD tmp = (WORD) (APSH_WinBClose - APSH_SizeVerify);
		ULONG FuncID;

		if (FuncID = wn->wn_Funcs[tmp])
		{
		    DC (kprintf ("Before close window function\n"));
		    PerfFunc (ai, FuncID, NULL, tl);
		}
	    }

	    /* Close the window */
	    DC (kprintf ("before CloseWindowSafely\n"));
	    CloseWindowSafely (win);
	    DC (kprintf ("after CloseWindowSafely\n"));

	    /* Mark the window as being closed */
	    wn->wn_Win = NULL;

	    /* Don't let anyone try using us again... */
	    ai->ai_Window = md->ii_Window = NULL;

	    /* Decrement the open window count */
	    md->ii_NumWins--;

	    /* After close function */
	    {
		WORD tmp = (WORD) (APSH_WinAClose - APSH_SizeVerify);
		ULONG FuncID;

		if (FuncID = wn->wn_Funcs[tmp])
		{
		    DC (kprintf ("After close window function\n"));
		    PerfFunc (ai, FuncID, NULL, tl);
		}
	    }

	}			/* End of if window open */

	/* Did we manage to get a window closed */
	retval = TRUE;
    }				/* End of if window node */

    DA (kprintf ("close_idcmp exit\n"));
    return (retval);
}

/* Free the current window node */
BOOL FreeWinNode (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * attrs)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct WinNode *wn;
    BOOL retval = FALSE;

    DA (kprintf ("FreeWinNode           0x%lx\n", FreeWinNode));

    /* See if the window descriptor already exists */
    if (wn = (struct WinNode *) mho->mho_CurNode)
    {
	struct ObjectInfo *oi;

	/* Make sure the window is closed */
	close_idcmp (ai, mh, NULL);

	/* Remove the node from the window list */
	Remove ((struct Node *) wn);

	/* Deallocate the keystroke array */
	shutdown_key_array (mh);

	/* Do we have an AppObjects array */
	if (oi = wn->wn_OI)
	{
	    /* Handle the deletion of the objects */
	    HandleList (&(oi->oi_ObjList), doObjList, ai, EG_DELETE);

	    /* Free the AppObjects list */
	    DisposeObjList (oi);

	    /* Clear the pointer */
	    wn->wn_OI = NULL;
	}

	/* Free the menu */
	FreeTextTableMenu (ai, NULL, attrs);

	/* Do we have a tag list */
	if (wn->wn_NWTags)
	{
	    /* Free the cloned tag item list */
	    DB (kprintf ("free new window attrs 0x%lx\n", wn->wn_NWTags));
	    FreeTagItems (wn->wn_NWTags);
	}

	/* Do we have a tag list */
	if (wn->wn_WinEnv)
	{
	    /* Free the cloned tag item list */
	    DB (kprintf ("free window environ. 0x%lx\n", wn->wn_WinEnv));
	    FreeTagItems (wn->wn_WinEnv);
	}

	/* Free the memory allocated to the node */
	DB (kprintf ("freevec 0x%lx\n", wn));
	FreeVec (wn);

	/* Successfull */
	retval = TRUE;
    }

    DB (kprintf ("FreeWinNode %ld\n", (LONG) retval));
    return (retval);
}

#if 0
BOOL delete_idcmp (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct TagItem *attrs;
    struct WinNode *wn;

    DA (kprintf ("delete_idcmp          0x%lx\n", delete_idcmp));

    /* Determine what attribute list to use */
    if (attrs = (struct TagItem *) GetTagData (APSH_WindowEnv, (ULONG) tl, tl))
    {
	struct List *list = &(mho->mho_ObjList);
	STRPTR name;

	/* See if they specified a name of a window to close */
	if (name = (STRPTR) GetTagData (APSH_NameTag, NULL, attrs))
	{
	    /* See if the window descriptor exists */
	    if (wn = (struct WinNode *) FindNameI (list, name))
	    {
		/* Set the current object pointer */
		SetCurrentWindow (ai, wn, NULL);
	    }
	}
    }

    /* Free the window node */
    return (FreeWinNode (ai, mh, attrs));
}
#endif
