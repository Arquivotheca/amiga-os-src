/* apsh_stdidcmp.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * standard Intuition functions
 *
 * $Id: apsh_stdidcmp.c,v 1.4 1992/09/07 17:59:38 johnw Exp $
 *
 * $Log: apsh_stdidcmp.c,v $
 * Revision 1.4  1992/09/07  17:59:38  johnw
 * Changes to Screen and Window code.
 *
 * Revision 1.1  91/12/12  14:54:28  davidj
 * Initial revision
 *
 * Revision 1.1  90/07/02  11:38:59  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

#include "dbugmac.h"

struct WinNode *GetWinNode (struct AppInfo * ai, struct TagItem * tl);
void kprintf (void *,...);
extern VOID KPT (char *, struct TagItem *);

#define	DB(x)	;
#define	DQ(x)	;
#define	DA(x)	;
#define	DW(x)	;
#define DS(x)	x;

/* our handler-specific functions */
struct Funcs idcmp_funcs[] =
{
    {"Activate", ActivateFunc, ActivateID, ",", 0L, APSHF_SYSTEM,},
    {"Deactivate", DeActivateFunc, DeActivateID, ",", 0L, APSHF_SYSTEM,},
    {"Window", WindowFunc, WindowID, "NAMES/M,TITLE/k,OPEN/s,CLOSE/s,SNAPSHOT/s,ACTIVATE/s,MIN/s,MAX/s,FRONT/s,BACK/s,ZOOM/s,UNZOOM/s,LOCK/s,UNLOCK/s", 14L, APSHF_SYSTEM,},
    {"WindowToBack", ToBackFunc, WindowToBackID, "NAME", 1L, APSHF_SYSTEM,},
    {"WindowToFront", ToFrontFunc, WindowToFrontID, "NAME", 1L, APSHF_SYSTEM,},

#if 0
    {"HOTKEY", HotKeyFunc, HotKeyID, "KEY,CMD/k,WINDOW/k,PROMPT/s,GLOBAL/s", 5L,},
#endif

    {NULL, NO_FUNCTION}		/* end of array */
};

/****** appshell.library/__ACTIVATE ******************************************
*
*   NAME
*	ACTIVATE - Restore the GUI to the state prior to DEACTIVATE
*
*   SYNOPSIS
*	ActivateID	Function ID
*
*   FUNCTION
*	Restores the GUI to the state prior to deactivating the window.
*	This will also reload user preferences.  This function accepts
*	no parameters.
*
*	As a string command line:
*
*	    ACTIVATE
*	    ,
*
*	This function is implemented by the IDCMP message handler.
*
*   SEE ALSO
*	DEACTIVATE
*
*   BUGS
*	Doesn't handle tools properly.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID ActivateFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct MsgHandler *mh;

    DBUG_ENTER("ActivateFunc");

    /* Restore the Intuition interface */
    DA (kprintf ("Activate: HandlerData\n"));
    if (mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE))
    {
	struct MHObject *mho = &(mh->mh_Header);
	struct IDCMPInfo *md = mho->mho_SysData;
	struct List *list = &(md->ii_ActvList);

    struct TagItem tags[2];
    LONG scr_ec = 0L;

    tags[0].ti_Tag = SA_ErrorCode;
    tags[0].ti_Data = (ULONG) &scr_ec;
    tags[1].ti_Tag = TAG_MORE;
    tags[1].ti_Data = NULL;

	/* We're activating... */
   	DS(kprintf("Activate: NS=0x%lx, NST=0x%lx\n",md->ii_NS,md->ii_NSTags));

    /* If we have the makings of a screen, then reopen it */
    if (md->ii_NS || md->ii_NSTags)
    {
    	struct TagItem *clone = NULL;

		if (md->ii_NSTags)
		{
		    /* Make sure we have tags to clone */
		    if (md->ii_NSTags->ti_Data)
		    {
		    tags[1].ti_Data = (ULONG) md->ii_NSTags;
			clone = CloneTagItems ((struct TagItem *) tags);
			FreeTagItems( md->ii_NSTags );
			md->ii_NSTags = clone;
		    }
		}

		/* Set the screen type */
		if (md->ii_NS)
		{
		    md->ii_ScrType = md->ii_NS->Type;
		}
		md->ii_ScrType = GetTagData (SA_Type, (LONG) md->ii_ScrType, md->ii_NSTags);
		DS(kprintf("Activate: Screentype = 0x%lx\n",md->ii_ScrType));

		/* Attempt to open the screen */
		DS(KPT("Activate: OpenSTL",md->ii_NSTags));
		md->ii_Screen = OpenScreenTagList (md->ii_NS, md->ii_NSTags);
		DS(kprintf("Activate: Screen = 0x%lx\n",md->ii_Screen));
		DS(kprintf("Activate: ScrEC = 0x%lx\n",scr_ec));
		if (md->ii_Screen)
		{
		    struct RastPort *rp = &(md->ii_Screen->RastPort);
		    struct GelsInfo *ginfo = NULL;

		    /*
		     * Don't set up a Gel System if there is already one for this
		     * screen
		     */
		    if (rp->GelsInfo == NULL)
		    {
				/* Setup the Gels for icons */
				ginfo = setupGelSys (rp, 0xFC);
	    	}

		    /* See if it is a public screen */
		    if (FindTagItem (SA_PubName, md->ii_NSTags))
		    {
				/* Set status here */
				PubScreenStatus (md->ii_Screen, NULL);

				/* Set the screen type */
				md->ii_ScrType = PUBLICSCREEN;
		    }

			/* Indicate that we opened the screen */
			md->Open_Screen = TRUE;
		}
	    else
	    {
			/* Open on the default public screen */
			lockscreen (ai, md, NULL);
			DS(kprintf("Activate: Opening on WB screen\n"));
	    }
    }

	/* See if there is anything in the activation list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *nxtnode;
	    struct ActiveRec *ar;
	    struct TagItem *we, attrs[2];

	    /* Set up the attributes */
	    attrs[0].ti_Tag = APSH_WindowEnv;
	    attrs[0].ti_Data = NULL;
	    attrs[1].ti_Tag = TAG_DONE;

	    /* Get a pointer to the first node in the list */
	    node = list->lh_Head;

	    /* Step through the entire list */
	    while (nxtnode = node->ln_Succ)
	    {
		/* Remove the node from the list */
		Remove (node);

		/* Cast the node to an ActiveRec */
		ar = (struct ActiveRec *) node;

		/* Get a pointer to the window environment that we want to open */
		we = ar->ar_WE;
		attrs[0].ti_Data = (LONG) we;

		/* Open the window environment */
		DA (kprintf ("open_idcmp\n"));
		open_idcmp (ai, (struct MsgHandler *) mho, attrs);
		DA (kprintf ("okay...\n"));

		/* Done with the window environment */
		if (we)
		    FreeTagItems (we);

		/* Deallocate the node itself */
		FreeVec ((APTR) node);

		/* next memory node */
		node = nxtnode;
	    }
	}

	/* Reinitialize the list */
	NewList (list);
    }

    /* If the command shell was open, reopen it */
    if (ai->ai_Flags & APSHF_CMDSHELL)
    {
	/* Reset the flag */
	ai->ai_Flags &= ~APSHF_CMDSHELL;

	/* close the command shell */
	PerfFunc (ai, NULL, "CMDSHELL OPEN", NULL);
    }

    /* Do we have a screen? */
    if (ai->ai_Screen)
    {
	/* Bring it to front */
	ScreenToFront (ai->ai_Screen);
    }

/*** Should really remember which window was the last active one ***/

    /* See if we have a window */
    if (ai->ai_Window)
    {
	/* Bring it to front */
	WindowToFront (ai->ai_Window);

	/* Activate it */
	ActivateWindow (ai->ai_Window);
    }

    DBUG_VOID_RETURN;
}

/****** appshell.library/__DEACTIVATE ****************************************
*
*   NAME
*	DEACTIVATE - Shut down the GUI.
*
*   SYNOPSIS
*	DeActivateID	Function ID
*	DeActivateFunc	Function prototype
*
*   FUNCTION
*	This function will shut down the graphical user interface of the
*	application.
*
*	This function accepts no parameters.
*
*	As a string command line:
*
*	    DEACTIVATE
*	    ,
*
*	This function is implemented by the IDCMP message handler.
*
*   SEE ALSO
*	ACTIVATE
*
*   BUGS
*	Currently, only tracks the main process (no tools or clones
*	receive the DEACTIVATE message yet) and the command shell.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID DeActivateFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct MsgHandler *mh;

    DBUG_ENTER("DeActivateFunc");

    /* Check the command shell */
    if (mh = HandlerData (ai, APSH_Handler, "DOS", TAG_DONE))
    {
	struct MHObject *mho = &(mh->mh_Header);
	struct DOSInfo *md = mho->mho_SysData;

	if (md)
	{
	    if (md->acons)
	    {
		md->dstatus = AS_GOING;
		ai->ai_Flags |= APSHF_CMDSHELL;

		/* close the command shell */
		PerfFunc (ai, NULL, "CMDSHELL CLOSE", NULL);
	    }
	}
    }

    /* Check the intuition handler */
    if (mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE))
    {
	struct MHObject *mho = &(mh->mh_Header);
	struct IDCMPInfo *md = mho->mho_SysData;
	struct List *list;
	struct Node *node, *nxtnode;
	struct WinNode *wn;

	DS(kprintf("Deactivate: NS=0x%lx NST=0x%lx\n",md->ii_NS,md->ii_NSTags));
	/* See if we have a public screen */
	if (md->ii_Screen && (FindTagItem (SA_PubName, md->ii_NSTags)))
	{
	    /* Try to make the screen private */
	    PubScreenStatus (md->ii_Screen, PSNF_PRIVATE);
	}

	/* close all the windows */
	list = &(mho->mho_ObjList);
	if (list->lh_TailPred != (struct Node *) list)
	{
	    node = list->lh_Head;
	    while (nxtnode = node->ln_Succ)
	    {
		/* set the current window */
		wn = (struct WinNode *) node;
		mho->mho_CurNode = (struct MHObject *) node;
		md->ii_Window = wn->wn_Win;

		/* See if the window is open */
		if (wn->wn_Win && wn->wn_WinEnv)
		{
		    LONG asize = sizeof (struct ActiveRec);
		    struct List *alist = &(md->ii_ActvList);
		    struct ActiveRec *ar;

		    /* We need to remember to reopen this window */
		    if (ar = (struct ActiveRec *) AllocVec (asize, MEMF_CLEAR))
		    {
			ar->ar_WE = CloneTagItems (wn->wn_WinEnv);
			AddTail (alist, (struct Node *) ar);
		    }
		}

		/* Close the window */
		close_idcmp (ai, (struct MsgHandler *) mho, NULL);

		/* Free the keyboard array */
		shutdown_key_array ((struct MsgHandler *) mho);

#if 1
		DisposeObjList (wn->wn_OI);
		wn->wn_OI = NULL;
#else
		FreeWindowDef (md, wn, wn->wn_WinEnv);
#endif

		/* deallocate resources */
		Remove (node);

		/* free the cloned TagList */
		if (wn->wn_NWTags)
		    FreeTagItems (wn->wn_NWTags);

		/* free the cloned gadget tags */
		if (wn->wn_WinEnv)
		    FreeTagItems (wn->wn_WinEnv);

		/* free the Window node */
		FreeVec ((APTR) node);

		/* next memory node */
		node = nxtnode;
	    }
	}

	/* free public screen */
	if (md->ii_PubScreen)
	    UnlockPubScreen (NULL, md->ii_PubScreen);
	md->ii_PubScreen = NULL;

	/* close screen if it was a CUSTOMSCREEN */
	if (md->ii_NS || md->ii_NSTags)
	{
	    BOOL closed = TRUE;

	    /* make sure all windows are closed before we close the screen */
	    while (md->ii_Screen->FirstWindow)
			NotifyUser (ai, "Close all windows", NULL);

	    closed = CloseScreen(md->ii_Screen);
		DS(kprintf("Close screen = %ld\n",(LONG)closed));
	    md->ii_Screen = NULL;
	    md->Open_Screen = FALSE;
	}

	/* Reset the screen type */
	/* md->ii_ScrType = WBENCHSCREEN; */
    }

    DBUG_VOID_RETURN;
}

/*o**** appshell.library/__HOTKEY ********************************************
*
*   NAME
*	HOTKEY - Bind a keyboard command to a function.
*
*   SYNOPSIS
*	HotKeyID	Function ID
*
*   FUNCTION
*	Provides a mechanism to bind a function to a keystroke or sequence
*	of keystrokes.
*
*	As a string command line:
*
*	    KEYSTROKE KEY,CMD/K,WINDOW/K,PROMPT/S,GLOBAL/S
*
*		KEY	Keystroke to edit.
*		CMD	Command to assign to the keystroke.
*		WINDOW	Window to assign the keystroke to.
*		PROMPT	Activate the GUI hotkey editor.
*		GLOBAL	Make the a key a global hotkey (commodity).
*
*	As a TagItem attribute list:
*
*
*	    APSH_CmdData, <key>
*		where <key> is the keystroke.
*
*	    APSH_NameTag, <function name>
*		where <function name> is a valid function name.
*
*	This function is implemented by the IDCMP message handler.
*
*   BUGS
*	Currently doesn't support anything but single, shifted and unshifted,
*	alphabetic characters [when called from the command line].
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID HotKeyFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
	struct Funcs *f;

	DBUG_ENTER("HotKeyFunc");

	if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
	{
	    struct MsgHandler *mh;
	    STRPTR key, name, winname;
	    WORD num;

	    /* get the name array */
	    key = (STRPTR) f->fe_Options[0];
	    name = (STRPTR) f->fe_Options[1];
	    winname = (STRPTR) f->fe_Options[2];
	    num = (WORD) key[0];

	    /* make sure that we have a handle on the window */
	    if (winname)
	    {
		HandlerFunc (ai,
			     APSH_Handler, "IDCMP",
			     APSH_Command, APSH_MH_OPEN,
			     APSH_NameTag, winname,
			     TAG_DONE);
	    }

	    if (mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE))
	    {
		struct MHObject *mho = &(mh->mh_Header);
		struct Hotkeys *hk;
		struct WinNode *wn;
		struct FuncEntry *fe;

		/* get a pointer on the Window node */
		wn = (struct WinNode *) mho->mho_CurNode;

		/* get the function entry node */
		fe = GetFuncEntry (ai, name, NULL);

		/* make sure it isn't a private function */
		if (!(fe->fe_Flags & APSHF_PRIVATE))
		{
		    /* get a pointer to the hotkey */
		    hk = &(wn->wn_HKey[num]);

		    /* set the key to the function */
		    hk->hk_FuncID = fe->fe_ID;
		}
	    }			/* end of if handler data */
	}			/* end of APSH_FuncEntry */

	DBUG_VOID_RETURN;
}

/****** appshell.library/__WINDOWTOBACK **************************************
*
*   NAME
*	WINDOWTOBACK - Send an AppShell window to the back.
*
*   SYNOPSIS
*	WindowToBackID	Function ID
*
*   FUNCTION
*	Provides a mechanism to send an AppShell window to the back of the
*	other windows on the same screen.
*
*	As a string command line:
*
*	    WINDOWTOBACK
*	    NAME
*
*		NAME	Name of the window to send to the back.  Defaults
*			to MAIN.
*
*	As a TagItem attribute list:
*
*	    APSH_NameTag, <name>
*		where <name> is a valid window name.
*
*	    APSH_WinPointer, <pointer>
*		where <pointer> points to a valid window structure.
*
*	This function is implemented by the IDCMP message handler.
*
*   SEE ALSO
*	WINDOWTOFRONT, WINDOW
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID movewin (struct AppInfo * ai, STRPTR args, struct TagItem * tl, BOOL dir)
{
    struct WinNode *wn;

    DBUG_ENTER("movewin");

    if (wn = GetWinNode (ai, tl))
    {
	/* send the window somewhere */
	if (wn->wn_Win)
	{
	    if (dir)
		WindowToBack (wn->wn_Win);
	    else
		WindowToFront (wn->wn_Win);
	}
    }

    DBUG_VOID_RETURN;
}

VOID ToBackFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    DBUG_ENTER("ToBackFunc");

    movewin (ai, args, tl, TRUE);

    DBUG_VOID_RETURN;
}

/****** appshell.library/__WINDOWTOFRONT *************************************
*
*   NAME
*	WINDOWTOFRONT - Bring an AppShell window to the front.
*
*   SYNOPSIS
*	WindowToFrontID	Function ID
*
*   FUNCTION
*	Provides a mechanism to bring an AppShell window in front of the
*	other windows on the same screen.
*
*	As a string command line:
*
*	    WINDOWTOFRONT
*	    NAME
*
*		NAME	Name of the window to bring to the front.  Defaults
*			to MAIN.
*
*	As a TagItem attribute list:
*
*	    APSH_NameTag, <name>
*		where <name> is a valid window name.
*
*	    APSH_WinPointer, <pointer>
*		where <pointer> points to a valid window structure.
*
*	This function is implemented by the IDCMP message handler.
*
*   SEE ALSO
*	WINDOWTOBACK, WINDOW
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID ToFrontFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
	DBUG_ENTER("ToFrontFunc");

    movewin (ai, args, tl, FALSE);

    DBUG_VOID_RETURN;
}

/****** appshell.library/__WINDOW ********************************************
*
*   NAME
*	WINDOW - Open/Close an AppShell window
*
*   SYNOPSIS
*	WindowID	Function ID
*
*   FUNCTION
*	Provides a mechanism to open or close an AppShell window.
*
*	As a string command line:
*
*	    WINDOW
*	    NAME,TITLE/k,OPEN/s,CLOSE/s,SNAPSHOT/s,ACTIVATE/s,MIN/s,MAX/s,
*	    FRONT/s,BACK/s,ZOOM/s,UNZOOM/s,LOCK/s,UNLOCK/s
*
*		NAME		Name of the window to manipulate.
*		TITLE		Title to assign to the window.
*		OPEN		Open the window.
*		CLOSE		Close the window.
*		SNAPSHOT	Save the window information to disk.
*		ACTIVATE	Activate the window.
*		MIN		Set the window to minimum size.
*		MAX		Set the window to maximum size.
*		FRONT		Send the window to the front.
*		BACK		Send the window to the back.
*		ZOOM		Set the window to its zoom size.
*		UNZOOM		Set the window to its normal size.
*		LOCK		Lock out input to the window.
*		UNLOCK		Unlock a locked window.
*
*	As a TagItem attribute list:
*
*	    APSH_NameTag, <name>
*		where <name> is a valid window name.
*
*	    APSH_WinPointer, <pointer>
*		where <pointer> points to a valid window structure.
*
*	    APSH_Command, <cmd>
*		where <cmd> is a valid command.
*		    APSH_MH_OPEN	To open the window.
*		    APSH_MH_CLOSE	To close the window.
*
*	This function is implemented by the IDCMP message handler.
*
*   SEE ALSO
*	WINDOWTOBACK, WINDOWTOFRONT
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID lock_window (struct AppInfo * ai, struct WinNode * wn)
{
    struct Requester *r = &(wn->wn_BR);
    struct Window *w = wn->wn_Win;

    DBUG_ENTER("lock_window");

    /* Clear the requester fields */
    InitRequester (r);

    /* Set the required fields */
    r->LeftEdge = (-1);		/* w->BorderLeft; */
    r->TopEdge = (-1);		/* w->BorderTop; */
    r->Width = 1;
    r->Height = 1;
    r->Flags = SIMPLEREQ | NOREQBACKFILL;

    /* Bring up the locking requester */
    Request (r, w);

    DBUG_VOID_RETURN;
}

VOID windowfunc (struct AppInfo *ai, STRPTR name, struct WinNode *wn, struct Funcs *f)
{
	DBUG_ENTER("windowfunc");

    if (ai && wn && f)
    {
	    struct TagItem tag[4];
	    struct Window *win;
	    LONG cmd = (-1);

	    /* Check open flag */
	    if (f->fe_Options[2])
	    {
		cmd = APSH_MH_OPEN;
	    }

	    /* Check close flag */
	    if (f->fe_Options[3])
	    {
		cmd = APSH_MH_CLOSE;
	    }

	    DW (kprintf ("window name [%s] o/c %ld\n", name, f->fe_Options[3]));

	    /* set the active window */
	    if (cmd != (-1))
	    {
		/* Do whatever to the window */
		HandlerFunc (ai,
			     APSH_Handler, "IDCMP",
			     APSH_Command, cmd,
			     APSH_NameTag, name,
			     TAG_DONE);
	    }

	    /* do something with the window */
	    if ((win = wn->wn_Win) && ((cmd == APSH_MH_OPEN) || cmd == (-1)))
	    {
		SHORT deltax = 0, deltay = 0;

		/* Set up the tag attributes */
		tag[0].ti_Tag = APSH_WinPointer;
		tag[0].ti_Data = (ULONG) win;
		tag[1].ti_Tag = TAG_DONE;

		/* set the window title */
		if (f->fe_Options[1])
		{
		    /* Copy the window title */
		    strcpy (wn->wn_Title, (UBYTE *) f->fe_Options[1]);

		    /* Set the window title */
		    SetWindowTitles (win, wn->wn_Title, (UBYTE *) (-1));
		}

		/* Snapshot the window position */
		if (f->fe_Options[4])
		    SaveSnapShot (win, ai->ai_ConfigDir,
				  wn->wn_Header.mho_Node.ln_Name);

		/* activate the window */
		if (f->fe_Options[5])
		{
		    ActivateWindow (win);
		}

		/* minimize the window */
		if (f->fe_Options[6])
		{
		    deltax = win->MinWidth - win->Width;
		    deltay = win->MinHeight - win->Height;
		}

		/* maximize the window */
		if (f->fe_Options[7])
		{
		    deltax = win->MaxWidth - win->Width;
		    deltay = win->MaxHeight - win->Height;
		}

		/* size the window if it needs it */
		if (deltax && deltay)
		    SizeWindow (win, deltax, deltay);

		/* send the window to the front */
		if (f->fe_Options[8])
		    WindowToFront (win);

		/* send the window to the back */
		if (f->fe_Options[9])
		    WindowToBack (win);

		/* zoom the window */
		if (f->fe_Options[10] && !(win->Flags & ZOOMED))
		    ZipWindow (win);

		/* unzoom the window */
		if (f->fe_Options[11] && (win->Flags & ZOOMED))
		    ZipWindow (win);

		/* lock the window */
		if (f->fe_Options[12] &&
		    !(wn->wn_Flags & APSH_WINF_LOCKGUI))
		{
		    /* Set the lock flag */
		    wn->wn_Flags |= APSH_WINF_LOCKGUI;

		    /* set the wait pointer */
		    APSHSetWaitPointer (ai, tag);

		    /* REALLY lock the window user input */
		    lock_window (ai, wn);
		}

		/* unlock the window */
		if (f->fe_Options[13] &&
		    (wn->wn_Flags & APSH_WINF_LOCKGUI))
		{
		    /* Unlock the window */
		    EndRequest (&(wn->wn_BR), wn->wn_Win);

		    /* Clear the lock flag */
		    wn->wn_Flags &= ~APSH_WINF_LOCKGUI;

		    /* set the wait pointer */
		    APSHClearPointer (ai, tag);

		    /* Refresh the window */
		    GT_RefreshWindow (wn->wn_Win, NULL);
		}
	    }			/* if APSH_FuncEntry */
    }

    DBUG_VOID_RETURN;
}

VOID WindowFunc (struct AppInfo * ai, STRPTR cmd, struct TagItem * tl)
{
    STRPTR name = NULL, *sptr = NULL;
    struct MsgHandler *mh = NULL;
    struct WinNode *wn = NULL;
    struct Funcs *f = NULL;
    struct TagItem *attrs;

    DBUG_ENTER("WindowFunc");

    /* Try getting a pointer to the Intuition message handler */
    mh = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE);

    if (attrs = (struct TagItem *) GetTagData (APSH_WindowEnv, (LONG) tl, tl))
    {

	if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, attrs))
	{
	    sptr = (STRPTR *) f->fe_Options[0];
	}
	else
	{
	    name = (STRPTR) GetTagData (APSH_NameTag, NULL, attrs);
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
		windowfunc (ai, name, wn, f);
	    }
	}
	else if (sptr)
	{
	    struct AnchorList al = {NULL};
	    LONG retval;

	    while (*sptr)
	    {
		/* Search through the list */
		for (retval = LMatchFirst (list, *sptr, &al);
		     retval == 0L;
		     retval = LMatchNext (&al))
		{
		    /* Get the name */
		    name = al.al_CurNode->ln_Name;

		    /* Find the WinNode */
		    if (wn = (struct WinNode *) FindNameI (list, name))
		    {
			windowfunc (ai, name, wn, f);
		    }
		}

		/* End the pattern matching */
		LMatchEnd (&al);

		/* Get the next name */
		sptr++;
	    }
	}
	else
	{
	    /* No name given, so use the current window */
	    if (wn = (struct WinNode *) mho->mho_CurNode)
	    {
		name = wn->wn_Header.mho_Node.ln_Name;
		windowfunc (ai, name, wn, f);
	    }
	}
    }

    DBUG_VOID_RETURN;
}

