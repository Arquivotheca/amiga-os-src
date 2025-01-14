/* apsh_idcmp.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Intuition processing. Gadget/Menu and Keyboard.
 *
 */

#include "appshell_internal.h"

#define	DB(x)	;
#define	DQ(x)	;
#define	DW(x)	;
#define	DH(x)	;
#define	DE(x)	;
#define	DF(x)	;
#define	DR(x)	;
#define	DM(x)	;
#define	DA(x)	;
#define	DX(x)	;
#define	DI(x)	;
#define	DG(x)	;
#define	DUP(x)	;

/* information for the new image bob
** structure defined in gfxextend.h
*/
NEWIMAGEBOB myNewIBob =
{
    NULL,			/* initial image */
    SAVEBACK | OVERLAY,		/* vsprite flags */
    0,				/* dbuf (0 == false) */
    2,				/* raster depth */
    160, 50,			/* x,y position */
    0, 0,			/* hit mask, me mask */
};

#define	FROM_TEXT	FALSE

extern BOOL FreeWinNode (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem *attrs);

VOID SetupDisable (struct AppInfo * ai, struct WinNode * wn);
VOID SetCurrentWindowA (struct AppInfo *, struct WinNode *, struct TagItem *);
VOID SetCurrentWindow (struct AppInfo *, struct WinNode *, ULONG,...);
extern struct ConsoleDevice *ConsoleDevice;
void kprintf (void *,...);
VOID doObjList (struct ObjectNode * on, ULONG cnt, VOID * ho);
VOID HandleGadgetHelp (struct AppInfo *, struct MsgHandler *, struct IntuiMessage *, struct TagItem *);
VOID HandleMenuHelp (struct AppInfo *, struct MsgHandler *, struct IntuiMessage *, struct TagItem *);

extern struct Funcs idcmp_funcs[];

UBYTE mentem[] = "Window/K,Title/K,Item/K,Sub/K,Bar/S,End/S,Label/K,Key/K,Cmd/K";

/****** appshell.library/setup_idcmpA **************************************
*
*   NAME
*	setup_idcmpA - Initializes the Intuition message handler.
*
*   SYNOPSIS
*	mh = setup_idcmpA (ai, attrs)
*	D0		   D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This is the low-level function used to initialize the Intuition,
*	or GUI, message handler.
*
*	NOTE: This call should not be called directly by the application,
*	but should invoked by using the APSH_AddIntui_UI tag.
*
*	Valid tagitems to use at INITIALIZATION are:
*
*	    APSH_Status, <flags>
*	    Where flags can be:
*
*		APSHP_INACTIVE
*		When set, then the window environment, that is passed in
*		with the initialization tags, will be not be initialized.
*
*	    The following tags allow the programmer to specify a function
*	    to be executed whenever the corresponding IDCMP event occurs.
*
*		APSH_SizeVerify, <id>
*		APSH_NewSize, <id>
*		APSH_RefreshWindow, <id>
*		APSH_MouseButtons, <id>
*		APSH_ReqSet, <id>
*		APSH_CloseWindow, <id>
*		APSH_ReqVerify, <id>
*		APSH_ReqClear, <id>
*		APSH_MenuVerify, <id>
*		APSH_DiskInserted, <id>
*		APSH_DiskRemoved, <id>
*		APSH_ActiveWindow, <id>
*		APSH_InactiveWindow, <id>
*		APSH_IntuiTicks, <id>
*		APSH_MouseMove, <id>
*
*	    APSH_WinBOpen, <id>
*	    Function to execute before opening a window.
*
*	    APSH_WinAOpen, <id>
*	    Function to execute after opening a window.
*
*	    APSH_WinBClose, <id>
*	    Function to execute before closing a window.
*
*	    APSH_WinAClose, <id>
*	    Function to execute after closing a window.
*
*	    APSH_RefreshData, <id>
*	    Function to execute whenever the data for gadgets needs
*	    refreshing.  Because GadTools doesn't support relative
*	    gadgets, the AppShell has to free the gadgets and build new
*	    ones whenever the user resizes a sizeable window.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	attrs	- Pointer to an array of TagItem attributes for the
*		  function.
*
*   RESULT
*	mh	- Pointer to a MsgHandler structure.
*
*   SEE ALSO
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

struct MsgHandler *setup_idcmpA (struct AppInfo * ai, struct TagItem * tl)
{
    struct MsgHandler *mh = NULL;
    struct TagItem *work_tag;
    struct MHObject *mho;
    struct IDCMPInfo *md;
    ULONG msize, hstatus;
    ULONG cntr;

    DA (kprintf ("setup_idcmpA          0x%lx\n", setup_idcmpA));

    /* calculate size of memory block */
    msize = sizeof (struct MsgHandler) +
      sizeof (struct IDCMPInfo) +
      (6L * sizeof (ULONG));

    /* allocate the message handlers' memory block */
    if (mh = (struct MsgHandler *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* Set the cache */
	ai->ai_MH[MH_IDCMP] = mh;

	/* get a pointer to our object node */
	mho = &(mh->mh_Header);

	/* initialize the message handler node information */
	mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	mho->mho_Node.ln_Pri = (APSH_MH_HANDLER_P + 1);
	mho->mho_Node.ln_Name = "IDCMP";

	/* initialize the object list */
	NewList (&(mho->mho_ObjList));

	/* set up the message handler ID */
	mho->mho_ID = APSH_IDCMP_ID;

	/* set up the message handler data */
	mho->mho_SysData = md = MEMORY_FOLLOWING (mh);

	/* set up the function table */
	mh->mh_NumFuncs = 4;
	mh->mh_Func = MEMORY_FOLLOWING (md);
	mh->mh_Func[APSH_MH_OPEN] = open_idcmp;
	mh->mh_Func[APSH_MH_HANDLE] = handle_idcmp;
	mh->mh_Func[APSH_MH_CLOSE] = close_idcmp;
	mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_idcmp;
	mh->mh_Func[IH_DELWIN] = FreeWinNode;

	/* Setup the menu stuff */
	md->ii_Template = mentem;
	md->ii_NumOpts = 9L;

	/* get the opening status */
	hstatus = GetTagData (APSH_Status, NULL, tl);

	/* create the IDCMP message port */
	DB (kprintf ("setup_idcmpA: CreatePort\n"));
	if (mh->mh_Port = CreatePort (NULL, NULL))
	{
	    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

	    /* add the handler-specific functions to the table */
	    DB (kprintf ("setup_idcmpA: AddFuncEntries 0x%lx\n", idcmp_funcs));
	    AddFuncEntries (ai, idcmp_funcs);

	    /* Initialize the active list */
	    NewList (&(md->ii_ActvList));

	    /* preset the screen type */
	    md->ii_ScrType = WBENCHSCREEN;
	    md->Open_Screen = FALSE;

	    /* set the default IDCMP flags */
	    md->ii_FastIDCMP = (IDCMP_flagF | REFRESHWINDOW);
	    md->ii_SlowIDCMP = (IDCMP_flagS | REFRESHWINDOW);

	    /* establish the default window flags */
	    md->ii_Flags = DEFWINFLAGS;

	    /* setup all the default callback functions */
	    DB (kprintf ("setup_idcmpA: callbacks\n"));
	    for (cntr = APSH_SizeVerify; cntr <= APSH_VanillaKey; cntr++)
	    {
		WORD temp;

		temp = (WORD) (cntr - APSH_SizeVerify);

		if (work_tag = FindTagItem (cntr, tl))
		{
		    md->ii_Funcs[temp] = work_tag->ti_Data;

		    if (cntr == APSH_VanillaKey)
		    {
			md->ii_FastIDCMP |= VANILLAKEY;
			md->ii_SlowIDCMP |= VANILLAKEY;
		    }
		}
		else
		{
		    md->ii_Funcs[temp] = NO_FUNCTION;
		}
	    }

	    if (!(GetTagData (APSH_WindowEnv, NULL, tl)) ||
		(hstatus & APSHP_INACTIVE))
	    {
		return (mh);
	    }
	    else
	    {
		if (open_idcmp (ai, mh, tl))
		{
		    return (mh);
		}
		else
		{
		    struct MHObject *mho = &(mh->mh_Header);
		    struct IDCMPInfo *md = mho->mho_SysData;

		    /* Do we have a lock on a public screen? */
		    if (md->ii_PubScreen)
		    {
			/* Unlock it */
			UnlockPubScreen (NULL, md->ii_PubScreen);
			md->ii_PubScreen = NULL;
		    }
		}
	    }

	    /* clean failure path */
	    DeletePort (mh->mh_Port);
	    mh->mh_Port = NULL;
	}
	else
	{
	    ai->ai_Pri_Ret = RETURN_FAIL;
	    ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret,
				       (int) "IDCMP");
	}

	/* clean failure path */
	FreeVec ((APTR) mh);
	mh = NULL;
    }
    else
    {
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
    }

    DB (kprintf ("setup_idcmpA: exit 0x%lx\n", mh));

    return (mh);
}

/* Intuition message processing */
BOOL handle_idcmp (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * ptl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct IDCMPInfo *md = mho->mho_SysData;
    struct IntuiMessage *imsg;	/* incoming Intuition messages */
    struct IntuiMessage *nmsg;	/* Pointer to copy of IntuiMessage */
    struct ObjectInfo *oi;
    struct WinNode *wn;
    struct TagItem *tl;
    BOOL activate;
    ULONG FuncID;

    /* Set the current message handler */
    ai->ai_ActvMH = mho->mho_ID;

    /* Point to the temporary message */
    nmsg = &(md->ii_Msg);
    ai->ai_ActvMsg = (struct Message *) nmsg;

    /* set the tag types for an entire IntuiMessage */
    md->ii_Tags[0].ti_Tag = APSH_MsgClass;
    md->ii_Tags[1].ti_Tag = APSH_MsgCode;
    md->ii_Tags[2].ti_Tag = APSH_MsgQualifier;
    md->ii_Tags[3].ti_Tag = APSH_MsgIAddress;
    md->ii_Tags[4].ti_Tag = APSH_MsgMouseX;
    md->ii_Tags[5].ti_Tag = APSH_MsgMouseY;
    md->ii_Tags[6].ti_Tag = APSH_MsgSeconds;
    md->ii_Tags[7].ti_Tag = APSH_MsgMicros;
    md->ii_Tags[8].ti_Tag = APSH_MsgWindow;
    md->ii_Tags[9].ti_Tag = APSH_IntuiMessage;

    /* No activate message */
    activate = FALSE;

    /* we use GadTools for the message processing */
    while (imsg = (struct IntuiMessage *) GT_GetIMsg (mh->mh_Port))
    {
	/* Clear the function index */
	FuncID = NO_FUNCTION;

	/* copy the message so that we can reply to it */
	md->ii_Msg = *imsg;

	/* set the tag data for the entire IntuiMessage */
	tl = md->ii_Tags;
	md->ii_Tags[0].ti_Data = (ULONG) imsg->Class;
	md->ii_Tags[1].ti_Data = (ULONG) imsg->Code;
	md->ii_Tags[2].ti_Data = (ULONG) imsg->Qualifier;
	md->ii_Tags[3].ti_Data = (ULONG) imsg->IAddress;
	md->ii_Tags[4].ti_Data = (ULONG) imsg->MouseX;
	md->ii_Tags[5].ti_Data = (ULONG) imsg->MouseY;
	md->ii_Tags[6].ti_Data = imsg->Seconds;
	md->ii_Tags[7].ti_Data = imsg->Micros;
	md->ii_Tags[8].ti_Data = (ULONG) imsg->IDCMPWindow;
	md->ii_Tags[9].ti_Data = (ULONG) nmsg;
	md->ii_Tags[10].ti_Tag = TAG_DONE;
	if (imsg->Class == IDCMPUPDATE)
	{
	    md->ii_Tags[10].ti_Tag = TAG_MORE;
	    md->ii_Tags[10].ti_Data = (ULONG) imsg->IAddress;
	    DUP (kprintf ("Class==IDCMPUPDATE\n"));
	}

	/* Get a pointer to the user data */
	oi = (struct ObjectInfo *) imsg->IDCMPWindow->UserData;
	wn = (struct WinNode *) oi->oi_SystemData;

	/* get our current window */
	SetCurrentWindow (ai, wn, NULL);

	/* set the current mouse coordinates */
	ai->ai_MouseX = nmsg->MouseX;
	ai->ai_MouseY = nmsg->MouseY;

	/* What function is it? */
	switch (imsg->Class)
	{
	    case SIZEVERIFY:
		RemoveObjList (oi, ai->ai_Window, NULL);
		FuncID = wn->wn_Funcs[(APSH_SizeVerify - APSH_SizeVerify)];
		break;

	    case MENUVERIFY:
		FuncID = wn->wn_Funcs[(APSH_MenuVerify - APSH_SizeVerify)];
		DM (kprintf ("MenuVerify %ld\n", FuncID));
		break;

	    case REQVERIFY:
		FuncID = wn->wn_Funcs[(APSH_ReqVerify - APSH_SizeVerify)];
		break;
	}

	/* Well, do we have a function to perform? */
	if (FuncID != NO_FUNCTION)
	{
	    PerfFunc (ai, FuncID, NULL, tl);
	    FuncID = NO_FUNCTION;
	}

	/* Reply to the message now that we're done with it */
	GT_ReplyIMsg (imsg);

	/* Process Intuition events */
	switch (nmsg->Class)
	{
	    case CHANGEWINDOW:
		FuncID = wn->wn_Funcs[(APSH_ChangeWindow - APSH_SizeVerify)];
		break;

	    case NEWSIZE:
		{
		    struct Window *win = ai->ai_Window;
		    struct IBox *nb = (struct IBox *) & (win->LeftEdge);
		    struct IBox *ob = &(oi->oi_WinBox);

		    /* Only do this if the window DIDN'T change */
		    if ((nb->Width == ob->Width) &&
			(nb->Height == ob->Height))
		    {
			if (oi->oi_Flags & AOIF_REMOVED)
			{
			    /* Remove the list */
			    AddGList (win, oi->oi_GList, -1L, -1L, NULL);

			    /* Don't remove it again */
			    oi->oi_Flags &= ~AOIF_REMOVED;
			}
		    }
		}
		FuncID = wn->wn_Funcs[(APSH_NewSize - APSH_SizeVerify)];
		break;

		/* refresh the window */
	    case REFRESHWINDOW:
		GT_BeginRefresh (ai->ai_Window);
		GT_EndRefresh (ai->ai_Window, TRUE);

		/* Handle the creation setup of the objects */
		HandleList (&(oi->oi_ObjList), doObjList, ai, EG_DELETE);

		if (RefreshObjList (oi, ai->ai_Window, NULL))
		{
		    /* Get things back in sync */
		    SetupDisable (ai, wn);

		    /* Handle the creation setup of the objects */
		    HandleList (&(oi->oi_ObjList), doObjList, ai, EG_CREATE);

		    /* Refresh gadget data */
		    {
			WORD tmp = (WORD) (APSH_RefreshData - APSH_SizeVerify);
			ULONG FuncID;

			if (FuncID = wn->wn_Funcs[tmp])
			{
			    PerfFunc (ai, FuncID, NULL, tl);
			}
		    }
		}

		FuncID = wn->wn_Funcs[(APSH_RefreshWindow - APSH_SizeVerify)];
		break;

		/* get the active window */
	    case ACTIVEWINDOW:
		/* Remember the activate message */
		FuncID = wn->wn_Funcs[(APSH_ActiveWindow - APSH_SizeVerify)];

#if 0
		/* We got an activate message */
		activate = TRUE;
#endif

		break;

		/* window went inactive */
	    case INACTIVEWINDOW:
		if (md->ii_Selected && !(md->ii_Selected->Flags & BOBNIX))
		{
		    struct Window *win = ai->ai_Window;
		    struct Screen *scr = win->WScreen;
		    struct RastPort *rp = &(scr->RastPort);
		    struct ViewPort *vp = &(scr->ViewPort);
		    struct Layer_Info *li = &(scr->LayerInfo);

		    DQ (kprintf ("remove bob\n"));
		    RemBob (md->ii_Selected);
		    bobDrawGList (rp, vp);
		    freeBob (md->ii_Selected, md->ii_NewIBob.nib_RasDepth);
		    md->ii_Selected = NULL;

		    /* Turn menu operations back on */
		    win->Flags &= ~RMBTRAP;

		    /* Unlock layers */
		    UnlockLayers (li);
		}

		/* Always do this when you change the input focus */
		AbortKeystroke (oi, ai->ai_Window);
		FuncID = wn->wn_Funcs[(APSH_InactiveWindow - APSH_SizeVerify)];
		break;

		/* Handle the close window gadget */
	    case CLOSEWINDOW:
		FuncID = wn->wn_Funcs[(APSH_CloseWindow - APSH_SizeVerify)];
		if (FuncID != NO_FUNCTION)
		{
		    /* Call their close window function. */
		    PerfFunc (ai, FuncID, NULL, tl);
		    FuncID = NULL;

		    /* Close the window */
		    close_idcmp (ai, mh, tl);
		}
		else if ((nmsg->Qualifier & SHIFTED) || (md->ii_NumWins > 1L))
		{
		    /* Close the Window */
		    close_idcmp (ai, mh, tl);
		}
		else
		{
		    FuncID = QuitID;
		}
		activate = FALSE;
		break;

		/* Handle keyboard events */
	    case RAWKEY:
		if (imsg->Code == 95)
		{
		    if (AmigaGuideBase)
		    {
			HandleGadgetHelp (ai, mh, nmsg, tl);
		    }
		}
		else
		{
		    HandleKeyEvent (ai, mh, nmsg, tl);
		}
		activate = FALSE;
		break;

	    case VANILLAKEY:
		FuncID = wn->wn_Funcs[(APSH_VanillaKey - APSH_SizeVerify)];
		break;

		/* Handle events that pertain to gadgets */
	    case INTUITICKS:
	    case MOUSEMOVE:
	    case MOUSEBUTTONS:
	    case GADGETDOWN:
	    case GADGETUP:
	    case IDCMPUPDATE:
		if (!activate)
		{
		    HandleGadgEvent (ai, mh, nmsg, tl);
		    activate = FALSE;
		}
		break;

	    case IDCMP_MENUHELP:
		DM (kprintf ("MenuHelp\n"));
		if (AmigaGuideBase)
		{
		    HandleMenuHelp (ai, mh, nmsg, tl);
		}
		activate = FALSE;
		break;

		/* Handle menu events */
	    case MENUPICK:
		DM (kprintf ("MenuPick\n"));
		AbortKeystroke (oi, ai->ai_Window);
		HandleMenuEvent (ai, mh, nmsg, tl);
		activate = FALSE;
		break;

	    case DISKINSERTED:
		FuncID = wn->wn_Funcs[(APSH_DiskInserted - APSH_SizeVerify)];
		break;

	    case DISKREMOVED:
		FuncID = wn->wn_Funcs[(APSH_DiskRemoved - APSH_SizeVerify)];
		break;
	}			/* End of event switch */

	/* Is there a function associated with this event? */
	if (FuncID != NO_FUNCTION)
	{
	    DE (kprintf ("idcmp %ld, func %ld\n", (LONG) nmsg->Class, FuncID));
	    PerfFunc (ai, FuncID, NULL, tl);
	}
    }				/* End of while messages */

    /* Clear the active stuff */
    ai->ai_ActvMsg = NULL;
    ai->ai_ActvMH = NULL;

    return (TRUE);
}

/* free resources for IDCMP message handler */
BOOL shutdown_idcmp (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct IDCMPInfo *md = mho->mho_SysData;
    struct List *list;
    struct Node *node, *nxtnode;

    /* Clear the cache */
    ai->ai_MH[MH_IDCMP] = NULL;

    /* This doesn't really matter... */
    if (md->ii_Screen && (FindTagItem (SA_PubName, md->ii_NSTags)))
    {
	/* Make the screen private */
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
	    SetCurrentWindow (ai, (struct WinNode *) node, NULL);

	    /* Close the window */
	    close_idcmp (ai, mh, NULL);

	    /* Delete the window */
	    FreeWinNode (ai, mh, NULL);

	    /* Next window node */
	    node = nxtnode;
	}
    }

#if 0
    /* free all the windows */
    list = &(mho->mho_ObjList);
    if (list->lh_TailPred != (struct Node *) list)
    {
	node = list->lh_Head;
	while (nxtnode = node->ln_Succ)
	{
	    /* set the current window */
	    SetCurrentWindow (ai, (struct WinNode *) node, NULL);

	    /* Delete the window */
	    FreeWinNode (ai, mh, NULL);

	    /* Next window node */
	    node = nxtnode;
	}
    }
#endif

    /* close screen if it was a CUSTOMSCREEN */
    DF (kprintf ("close screen\n"));
    if (md->Open_Screen && md->ii_Screen)
    {
	struct RastPort *rp = &(md->ii_Screen->RastPort);
	BOOL status;

	/* Make sure we can close the screen */
	while (md->ii_Screen->FirstWindow)
	{
	    NotifyUser (ai, "Close all windows", NULL);
	}

	if (rp->GelsInfo)
	{
	    cleanupGelSys (rp->GelsInfo, rp);
	}

	/* Try closing the screen */
	while ((status = CloseScreen (md->ii_Screen)) == FALSE)
	{
	    NotifyUser (ai, "Release all screen locks\n", NULL);
	}

	/* Say that the screen is closed */
	md->ii_Screen = NULL;
	md->Open_Screen = FALSE;
    }

    DF (kprintf ("remove message handler\n"));
    Remove ((struct Node *) mh);

    /* Free the cloned screen tags */
    if (md->ii_NSTags)
    {
	DF (kprintf ("remove screen tags\n"));
	FreeTagItems (md->ii_NSTags);
    }

    /* free public screen */
    if (md->ii_PubScreen)
    {
	DF (kprintf ("free public screen\n"));
	UnlockPubScreen (NULL, md->ii_PubScreen);
	md->ii_PubScreen = NULL;
    }

    /* remove the message port */
    if (mh->mh_Port)
    {
	DF (kprintf ("free message port\n"));
	RemoveMsgPort (mh->mh_Port);
    }

    DF (kprintf ("free message handler 0x%lx\n", mh));
    FreeVec ((APTR) mh);

    DF (kprintf ("delete_idcmp exit\n"));
    return (TRUE);
}

/* safely close a window that shares a message port with another window */
VOID CloseWindowSafely (struct Window * win)
{
    struct IntuiMessage *msg, *succ;

    /* Disable multitasking */
    Forbid ();

    /* Clear out all outstanding Intuition messages */
    msg = (struct IntuiMessage *) win->UserPort->mp_MsgList.lh_Head;
    while (succ = (struct IntuiMessage *) msg->ExecMessage.mn_Node.ln_Succ)
    {
	if (msg->IDCMPWindow == win)
	{
	    Remove ((struct Node *) msg);
	    ReplyMsg ((struct Message *) msg);
	}
	msg = succ;
    }

    /* Shutdown the IDCMP port */
    win->UserPort = NULL;
    ModifyIDCMP (win, NULL);

    Permit ();

    /* Clear the menu strip */
    if (win->MenuStrip)
    {
	ClearMenuStrip (win);
    }

    /* Close the window */
    CloseWindow (win);
}

/* Allocate resources for handling keyboard input */
BOOL setup_key_array (struct MsgHandler * mh, struct KeyboardCMD * KeyArray)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct WinNode *wn = (struct WinNode *) mho->mho_CurNode;
    struct Hotkeys *hk;
    WORD cntr, num;
    ULONG msize;

    DA (kprintf ("setup_key_array       0x%lx\n", setup_key_array));
    DB (kprintf ("setup_key_array 0x%lx 0x%lx\n", mh, KeyArray));
    msize = sizeof (struct Hotkeys) * MAXKEYS;

    /* Allocate memory for the keyboard function map */
    if (wn->wn_HKey = (struct Hotkeys *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	if (KeyArray)
	{
	    /* read in the key assignments */
	    for (cntr = 0; KeyArray[cntr].kbc_Key != NULL; cntr++)
	    {
		num = KeyArray[cntr].kbc_Key;
		hk = &wn->wn_HKey[num];
		hk->hk_FuncID = KeyArray[cntr].kbc_FuncID;
	    }
	}
	return (TRUE);
    }

    return (FALSE);
}

/* Release resources for handling keyboard input */
VOID shutdown_key_array (struct MsgHandler * mh)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct IDCMPInfo *md = mho->mho_SysData;
    struct WinNode *wn;

    DA (kprintf ("shutdown_key_array    0x%lx\n", shutdown_key_array));

    if (md)
    {
	wn = (struct WinNode *) mho->mho_CurNode;

	if (wn->wn_HKey)
	    FreeVec ((APTR) wn->wn_HKey);
	wn->wn_HKey = NULL;
    }
}

/* Convert Raw keys to Vanilla keys */
LONG DeadKeyConvert (msg, kbuffer, kbsize, kmap)
    struct IntuiMessage *msg;
    UBYTE *kbuffer;
    LONG kbsize;
    struct KeyMap *kmap;
{
    struct InputEvent ievent;

    if (msg->Class != RAWKEY)
	return (-2);

    ievent.ie_NextEvent = NULL;
    ievent.ie_Class = IECLASS_RAWKEY;
    ievent.ie_SubClass = 0;
    ievent.ie_Code = msg->Code;
    ievent.ie_Qualifier = msg->Qualifier;
    ievent.ie_position.ie_addr = *((APTR *) msg->IAddress);

    return (RawKeyConvert (&ievent, kbuffer, kbsize, kmap));
}

/* Keyboard handling routines */
VOID HandleKeyEvent (struct AppInfo * ai, struct MsgHandler * mh,
		      struct IntuiMessage * msg, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct WinNode *wn = (struct WinNode *) mho->mho_CurNode;
    ULONG FuncID = NO_FUNCTION;
    struct ObjectNode *on;
    WORD cur = 0;

    if (on = LookupKeystroke (wn->wn_OI, msg))
    {
	FuncID = on->on_Current;
    }
    else
    {
	UBYTE __aligned buffer[26];
	struct Hotkeys *hk;
	WORD key;

	strcpy (buffer, "                       ");
	key = (WORD) DeadKeyConvert (msg, buffer, 25L, 0L);

	if (key > 0)
	{
	    /* Get the ASCII value of the key that was pressed */
	    cur = buffer[0];

	    /*
	     * Check to see if it was a special key; like a Function key, Help, or the Arrow keys.  Could also do additional
	     * checking for ALT, AMIGA and CTRL qualifiers by adding in a proportional weight for each one.
	     */
	    if (key > 1 && cur == 155)
	    {
		cur = SPECIAL + buffer[1];
	    }

	    /* Get the function number assigned to this key */
	    hk = &wn->wn_HKey[cur];
	    FuncID = hk->hk_FuncID;
	}
    }

    /* Make sure there is a function assigned to it */
    if (FuncID != NO_FUNCTION)
    {
	PerfFunc (ai, FuncID, NULL, tl);
    }
    else
    {
	struct ObjectInfo *oi = (struct ObjectInfo *) ai->ai_Window->UserData;

	if ((cur == 9) &&
	    (oi->oi_Active) &&
	    !(oi->oi_Active->Flags & GADGDISABLED))
	{
	    ActivateGadget (oi->oi_Active, ai->ai_Window, NULL);
	}
    }

    /*
     * If we have an object, and this is a key up and the object is intended to close the window, then close it.
     */
    if (on &&
	(wn->wn_OI->oi_PriorObj == NULL) &&
	(on->on_Object.o_Flags & APSH_OBJF_CLOSEWINDOW))
    {
	/* Restore current window */
	SetCurrentWindowA (ai, wn, NULL);

	close_idcmp (ai, mh, tl);
    }
}

VOID bobDrawGList (struct RastPort * rport, struct ViewPort * vport)
{

    SortGList (rport);
    DrawGList (rport, vport);
}


VOID MoveBob (struct RastPort * rp, struct ViewPort * vp,
	       struct Bob * Bob, SHORT X, SHORT Y)
{

    Bob->BobVSprite->X = X;
    Bob->BobVSprite->Y = Y;
    bobDrawGList (rp, vp);
}

int dumpTagList (UBYTE * str, struct TagItem * tags)
{
    struct TagItem *ti;
    struct TagItem *NextTagItem ();

    kprintf ("\n");
    if (str)
	kprintf ("%s\n", str);
    kprintf ("tags at %lx, prevtag %08lx - %08lx\n",
	     tags, tags[-1].ti_Tag, tags[-1].ti_Data);

    while (ti = NextTagItem (&tags))
    {
	kprintf ("%08lx - %08lx\n", ti->ti_Tag, ti->ti_Data);
    }
    return (0);
}

VOID SendIcon (struct AppInfo * ai, struct Window * dwin, struct ObjectNode * on, WORD x, WORD y)
{
    struct ObjectInfo *doi;
    struct WinNode *dwn;
    struct AppInfo *dai;
    STRPTR name;

    doi = (struct ObjectInfo *) dwin->UserData;
    dwn = (struct WinNode *) doi->oi_SystemData;
    name = dwn->wn_Header.mho_Node.ln_Name;
    dai = dwn->wn_AI;

    DB (kprintf ("%s : %s : %s\n", dai->ai_AppName, name, on->on_Object.o_Name));

    if (dai)
    {
	struct AppNode *dan = dai->ai_AN;
	struct SignalSemaphore *ss = &(dan->an_Lock);
	struct MsgHandler *mh;

	/* Lock the list for a little bit */
	ObtainSemaphore (ss);

	/* See if they have a SIPC port */
	if (mh = HandlerData (dai, APSH_Handler, "SIPC", TAG_DONE))
	{
	    ai->ai_TmpTags[0].ti_Tag = APSH_WinName;
	    ai->ai_TmpTags[0].ti_Data = (LONG) name;
	    ai->ai_TmpTags[1].ti_Tag = APSH_ObjName;
	    ai->ai_TmpTags[1].ti_Data = (LONG) on->on_Object.o_Name;
	    ai->ai_TmpTags[2].ti_Tag = APSH_MsgIAddress;
	    ai->ai_TmpTags[2].ti_Data = (LONG) on;
	    ai->ai_TmpTags[3].ti_Tag = APSH_MsgMouseX;
	    ai->ai_TmpTags[3].ti_Data = (LONG) (x - dwin->LeftEdge);
	    ai->ai_TmpTags[4].ti_Tag = APSH_MsgMouseY;
	    ai->ai_TmpTags[4].ti_Data = (LONG) (y - dwin->TopEdge);
	    ai->ai_TmpTags[5].ti_Tag = APSH_MsgWindow;
	    ai->ai_TmpTags[5].ti_Data = (LONG) dwin;
	    ai->ai_TmpTags[6].ti_Tag = TAG_DONE;

	    /* Send the command via the SIPC port */
	    HandlerFunc (ai,
			 APSH_Handler, (ULONG) "SIPC",
			 APSH_Command, AH_SENDCMD,
			 APSH_NameTag, (ULONG) mh->mh_PortName,
			 APSH_CmdID, DropIconID,
			 APSH_CmdData, (ULONG) ai->ai_TmpTags,
			 TAG_DONE);
	}
	else
	{
	    DB (kprintf ("doesn't have a sipc port\n"));
	    DisplayBeep (dwin->WScreen);
	}

	/* release the lock */
	ReleaseSemaphore (ss);
    }
}

struct ObjectNode *GetObjectFromID (struct ObjectInfo * oi, ULONG id)
{
    struct List *list = &(oi->oi_ObjList);
    struct ObjectNode *on;
    ULONG cid;

    /* Make sure there are entries in the list */
    if (list->lh_TailPred != (struct Node *) list)
    {
	struct Node *node, *nxtnode;

	/* Let's start at the very beginning... */
	node = list->lh_Head;

	/* Continue while there are still nodes */
	while (nxtnode = node->ln_Succ)
	{
	    /* Cast the node to an object */
	    on = (struct ObjectNode *) node;
	    cid = on->on_Object.o_ObjectID;

	    /* Compare ID's */
	    if ((cid == id) ||
		((cid - APSH_USER_ID) == id) ||
		((cid - APSH_MAIN_ID) == id))
	    {
		return (on);
	    }

	    /* Go on to the next node */
	    node = nxtnode;
	}
    }

    return (NULL);
}

/* Determine if a point is within a rectangle */
static ULONG PointInBox (struct IBox * point, struct IBox * box)
{

    if ((point->Left >= box->Left) &&
	(point->Left <= (box->Left + box->Width)) &&
	(point->Top >= box->Top) &&
	(point->Top <= (box->Top + box->Height)))
    {
	return (1L);
    }

    return (0L);
}

/* Find the rectangle of a gadget */
static VOID gadgetBox (struct Gadget * g, struct IBox * domain, struct IBox * box)
{

    /* Set the 'normal' rectangle */
    box->Left = g->LeftEdge;
    box->Top = g->TopEdge;
    box->Width = g->Width;
    box->Height = g->Height;

    /* Check for relativity */
    if (g->Flags & GRELRIGHT)
	box->Left += domain->Width - 1;
    if (g->Flags & GRELBOTTOM)
	box->Top += domain->Height - 1;
    if (g->Flags & GRELWIDTH)
	box->Width += domain->Width;
    if (g->Flags & GRELHEIGHT)
	box->Height += domain->Height;
}

static BOOL getfakebox (struct IBox * wbox, struct IBox * dst)
{
    struct IBox domain = *wbox;
    struct Gadget gad;
    BOOL rel = FALSE;

    /* Clear the domain to upper-left of 0,0 */
    domain.Left = 0;
    domain.Top = 0;

    /* Clear the flags */
    gad.Flags = NULL;

    /* Make a dummy gadget box */
    if ((gad.LeftEdge = dst->Left) < 0)
    {
	gad.Flags |= GRELRIGHT;
    }

    if ((gad.TopEdge = dst->Top) < 0)
    {
	gad.Flags |= GRELBOTTOM;
    }

    if ((gad.Width = dst->Width) < 0)
    {
	gad.Flags |= GRELWIDTH;
    }

    if ((gad.Height = dst->Height) < 0)
    {
	gad.Flags |= GRELHEIGHT;
    }

    /* It's relative */
    if (gad.Flags)
    {
	/* Get the correct gadget box */
	gadgetBox (&gad, &domain, dst);

	rel = TRUE;
    }

    return (rel);
}

/* Give help on a gadget */
VOID HandleGadgetHelp (struct AppInfo * ai, struct MsgHandler * mh,
		        struct IntuiMessage * msg, struct TagItem * tl)
{
    struct Window *win = msg->IDCMPWindow;
    struct ObjectInfo *oi = (struct ObjectInfo *) win->UserData;
    struct List *list = &(oi->oi_ObjList);
    STRPTR con1 = NULL, con2 = NULL;
    struct Node *node, *nxtnode;
    struct IBox b, point;
    struct ObjectNode *on;
    struct Object *o;

    /* Set the pointer position */
    point.Left = msg->MouseX;
    point.Top = msg->MouseY;

    /* Make sure there are entries in the list */
    if (list->lh_TailPred != (struct Node *) list)
    {
	/* Let's start at the very beginning... */
	node = list->lh_Head;

	/* Continue while there are still nodes */
	while ((nxtnode = node->ln_Succ) && !con1)
	{
	    on = (struct ObjectNode *) node;
	    o = (struct Object *) & (on->on_Object);
	    b = *(&(o->o_Outer));

	    if (o->o_Type == OBJ_Window)
	    {
		con2 = o->o_Name;
	    }

	    /* Adjust the box */
	    getfakebox ((struct IBox *) & win->LeftEdge, &b);

	    /* Is the pointer within this gadget? */
	    if (PointInBox (&point, &b))
	    {
		con2 = o->o_Name;
		if ((o->o_Type > 0) && (o->o_Type < OBJ_Image))
		{
		    con1 = con2;
		}
	    }

	    /* Go on to the next node */
	    node = nxtnode;
	}
    }

    if (!con1)
    {
	con1 = con2;
    }

    if (con1)
    {
	/* Build the command string */
	sprintf (ai->ai_WorkText, "LINK %s", con1);

	/* Send the command */
	DM (kprintf ("SendAmigaGuideCmd [%s]\n", ai->ai_WorkText));
	SendAmigaGuideCmd (ai->ai_HyperText, ai->ai_WorkText, NULL);
    }
}

/* Gadget handling routine */
VOID HandleGadgEvent (struct AppInfo * ai, struct MsgHandler * mh,
		       struct IntuiMessage * msg, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct IDCMPInfo *md = mho->mho_SysData;
    struct Window *win = msg->IDCMPWindow;
    struct Screen *scr = win->WScreen;
    struct RastPort *rp = &(scr->RastPort);
    struct ViewPort *vp = &(scr->ViewPort);
    struct Layer_Info *li = &(scr->LayerInfo);
    struct Layer *l;
    struct ObjectInfo *oi = (struct ObjectInfo *) win->UserData;
    struct WinNode *wn = (struct WinNode *) oi->oi_SystemData;
    struct Gadget *gad = (struct Gadget *) msg->IAddress;
    ULONG FuncID = NO_FUNCTION;
    static ULONG glsecs = 0L, glmics = 0L;	/* For Gadget DoubleClick */
    struct ObjectNode *on;
    BOOL clear = FALSE;

    DX (kprintf ("HandleGadget 0x%lx 0x%lx\n", msg, (ULONG) msg->Class));

    if (on = md->ii_CurObj)
    {
	DE (kprintf ("Handle %s %ld ", on->on_Node.ln_Name, (LONG) msg->Class));
    }

    switch (msg->Class)
    {

	    /*
	     * Check to see if there is a function to perform on downpress or double-click of the gadget.
	     */
	case GADGETDOWN:

	    DI (kprintf ("GadgetDown %ld\n", (LONG) gad->GadgetID));

	    /*
	     * Set the active gadget.  Only one gadget can EVER be active at a time (Intuition rule).
	     */
	    ai->ai_Gadget = gad;
	    md->ii_CurObj = on = gad->UserData;
	    ai->ai_CurObj = (struct MHObject *) on;

	    /* Tell the system that we need IntuiTicks & MouseMoves now */
	    ModifyIDCMP (md->ii_Window, wn->wn_SlowIDCMP);

	    /* get the downpress function */
	    FuncID = on->on_Funcs[EG_DOWNPRESS];

	    /* Needs all sorts of tests ... */
	    if (on->on_Object.o_Flags & APSH_OBJF_DRAGGABLE)
	    {
		/* Make sure the screen supports drag-ability */
		if (win->WScreen->RastPort.GelsInfo)
		{
		    struct Image *im = NULL;

		    if (gad->GadgetText)
		    {
			WORD label = (gad->Flags & LABELMASK);

			if (label == LABELIMAGE)
			{
			    im = (struct Image *) gad->GadgetText;
			}
		    }
		    else if (gad->Flags & GADGIMAGE)
		    {
			im = (struct Image *) gad->GadgetRender;
		    }

		    DX (kprintf ("object draggable 0x%lx\n", im));
		    if (im)
		    {
			/* Turn off menu operations... */
			win->Flags |= RMBTRAP;

			/* Copy the bob template */
			md->ii_NewIBob = *(&(myNewIBob));

			/* Initialize our portion */
			md->ii_NewIBob.nib_RasDepth = win->WScreen->BitMap.Depth;
			md->ii_NewIBob.nib_Image = im;

			/* Create the bob */
			md->ii_Selected = makeImageBob (&md->ii_NewIBob);

			/* Lock layers */
			LockLayers (li);

			/* Add the bob to the display list */
			AddBob (md->ii_Selected, rp);

			/* Establish its position */
			md->ii_X = im->Width / 2;	/* msg->MouseX - gad->LeftEdge; */
			md->ii_Y = im->Height / 2;	/* msg->MouseY - gad->TopEdge; */

			md->ii_oX = -1;
			md->ii_oY = -1;

			MoveBob (rp, vp, md->ii_Selected,
				 (SHORT) (scr->MouseX - md->ii_X),
				 (SHORT) (scr->MouseY - md->ii_Y));
		    }
		}
	    }
	    else
	    {
		DX (kprintf ("object not draggable 0x%lx\n", on->on_Object.o_Flags));
	    }

	    break;

	case IDCMPUPDATE:
	    DB (kprintf ("IDCMP Update\n"));
	    {
		struct TagItem *tags;
		ULONG gid;

		if (tags = (struct TagItem *) msg->IAddress)
		{
		    gid = GetTagData (GA_ID, 0L, tags);
		    DUP (kprintf ("IDCMPUpdate gid %ld\n", gid));
		    if (on = GetObjectFromID (oi, gid))
		    {
			ai->ai_Gadget = gad = on->on_Gadget;
			md->ii_CurObj = on;
			ai->ai_CurObj = (struct MHObject *) on;
			FuncID = on->on_Funcs[EG_UPDATE];
			DUP (kprintf ("on [%s] Func %ld\n",
				      on->on_Object.o_Name, FuncID));
		    }
		    else
		    {
			DUP (kprintf ("on = NULL\n"));
		    }
		}
	    }
	    break;

	    /*
	     * Check to see if gadget is being held down and if there is a function to perform while it is being held.
	     */
	case MOUSEMOVE:
	case INTUITICKS:

	    DB (kprintf ("Tick/Move on 0x%lx\n", on));
	    if (on)
	    {
		if (on->on_Gadget->Flags & SELECTED)
		{
		    /* gadget being held or dragged */
		    FuncID = on->on_Funcs[EG_HOLD];
		}

		if (msg->Qualifier & IEQUALIFIER_RBUTTON)
		{
		    /* Right mouse button pressed, abort operation */
		    FuncID = on->on_Funcs[EG_ABORT];

		    /* Show that it should be cleared */
		    clear = TRUE;

		    /* only watch for necessary messages */
		    ModifyIDCMP (md->ii_Window, wn->wn_FastIDCMP);
		}
	    }

	    if (md->ii_Selected)
	    {
		DB (kprintf ("%ld %ld\n", (LONG) scr->MouseX, (LONG) scr->MouseY));

		if ((md->ii_oX != scr->MouseX) || (md->ii_oY != scr->MouseY))
		{
		    MoveBob (rp, vp, md->ii_Selected,
			     (SHORT) (scr->MouseX - md->ii_X),
			     (SHORT) (scr->MouseY - md->ii_Y));

		    md->ii_oX = scr->MouseX;
		    md->ii_oY = scr->MouseY;
		}
	    }

	    if (FuncID == NO_FUNCTION)
	    {
		if (msg->Class == MOUSEMOVE)
		{
		    DB (kprintf ("Get MouseMove ID\n"));
		    FuncID = wn->wn_Funcs[(APSH_MouseMove - APSH_SizeVerify)];
		}
		else if (msg->Class == INTUITICKS)
		{
		    DB (kprintf ("Get IntuiTick ID\n"));
		    FuncID = wn->wn_Funcs[(APSH_IntuiTicks - APSH_SizeVerify)];
		}
	    }

	    break;

	    /* Indicate that there is no active gadget now. */
	case MOUSEBUTTONS:

	    DB (kprintf ("Mouse buttons\n"));

	    /* Set the function to mouse button event */
	    FuncID = wn->wn_Funcs[(APSH_MouseButtons - APSH_SizeVerify)];

	    DQ (kprintf ("MouseButton 0x%lx\n", (LONG) msg->Code));

	    if (md->ii_Selected && !(md->ii_Selected->Flags & BOBNIX))
	    {
		RemBob (md->ii_Selected);
		bobDrawGList (rp, vp);
		freeBob (md->ii_Selected, md->ii_NewIBob.nib_RasDepth);

		if (msg->Code == SELECTUP)
		{
		    struct Window *dwin;
		    WORD X = scr->MouseX;
		    WORD Y = scr->MouseY;

		    /* Which layer was the icon dropped in? */
		    l = WhichLayer (li, X, Y);

		    /* Is it in an AppShell window? */
		    if (l &&
			(dwin = (struct Window *) l->Window) &&
			(dwin->MoreFlags & WMF_APSH_WINDOW) &&
			(dwin->UserData))
		    {
			SendIcon (ai, dwin, on, X, Y);
		    }
		    else
		    {
			DisplayBeep (scr);
		    }
		}
		else
		{
		    /* Right mouse button pressed, abort operation */
		    FuncID = on->on_Funcs[EG_ABORT];
		}

		/* Turn menu operations back on */
		win->Flags &= ~RMBTRAP;

		/* Unlock layers */
		UnlockLayers (li);
	    }

	    /* Clear the selected item */
	    md->ii_Selected = NULL;

	    if ((msg->Code == SELECTUP) || (msg->Code == MENUDOWN))
	    {
		/* clear the active gadget */
		clear = TRUE;

		/* only watch for necessary messages */
		ModifyIDCMP (md->ii_Window, wn->wn_FastIDCMP);
	    }
	    else if (msg->Code == SELECTDOWN)
	    {
		/* Tell the system that we need IntuiTicks & MouseMoves now */
		ModifyIDCMP (md->ii_Window, wn->wn_SlowIDCMP);
	    }
	    break;

	    /*
	     * Check to see if there is a function to perform on release of the gadget.
	     */
	case GADGETUP:

	    DI (kprintf ("GadgetUp\n"));
	    /* we may never have gotten a downpress message... */
	    ai->ai_Gadget = gad;
	    md->ii_CurObj = on = gad->UserData;
	    ai->ai_CurObj = (struct MHObject *) on;

	    /* only watch for necessary messages */
	    ModifyIDCMP (md->ii_Window, wn->wn_FastIDCMP);

	    if (md->ii_Selected)
	    {
		RemBob (md->ii_Selected);
		bobDrawGList (rp, vp);
		freeBob (md->ii_Selected, md->ii_NewIBob.nib_RasDepth);
		md->ii_Selected = NULL;

		/* Turn menu operations back on */
		win->Flags &= ~RMBTRAP;

		/* Unlock layers */
		UnlockLayers (li);
	    }

	    if (msg->Qualifier & SHIFTED)
		FuncID = on->on_Funcs[EG_SHIFTHIT];
	    else if (msg->Qualifier & ALTED)
		FuncID = on->on_Funcs[EG_ALTHIT];
	    else
		FuncID = on->on_Funcs[EG_RELEASE];

	    if (on->on_Funcs[EG_DBLCLICK] != NO_FUNCTION)
	    {

		/*
		 * If the gadget has a double-click function, then check to see if it has been double-clicked.  Notice that if
		 * there is a double-click function, then it over-rules the release function.
		 */
		if (DoubleClick (glsecs, glmics, msg->Seconds, msg->Micros))
		    FuncID = on->on_Funcs[EG_DBLCLICK];
		else
		{
		    glsecs = msg->Seconds;
		    glmics = msg->Micros;
		}
	    }

	    /* Clear the active gadget variable */
	    clear = TRUE;
	    break;
    }

    if (on)
    {
	DE (kprintf ("Handle %s %ld ", on->on_Node.ln_Name, (LONG) msg->Class));
    }

    /* Perform the function if there is one. */
    if (FuncID != NO_FUNCTION)
    {
	DE (kprintf ("func %ld\n", FuncID));

	PerfFunc (ai, FuncID, NULL, tl);
    }

    /* if this was a close window gadget, then close the window */
    if (msg->Class == GADGETUP)
    {
	if (on->on_Object.o_Flags & APSH_OBJF_CLOSEWINDOW)
	{
	    /* Restore current window */
	    SetCurrentWindowA (ai, wn, NULL);

	    close_idcmp (ai, mh, tl);
	}
    }

    if (clear)
    {
	ai->ai_CurObj = NULL;
	md->ii_CurObj = NULL;
    }

    DB (kprintf ("Gadget exit\n"));
}

VOID HandleMenuHelp (struct AppInfo * ai, struct MsgHandler * mh,
		      struct IntuiMessage * msg, struct TagItem * tl)
{
    struct Window *win = msg->IDCMPWindow;
    struct EMenuItem *item;
    WORD mnum, inum, snum;
    BOOL text_menu = FALSE;
    struct WinNode *wn;

    /* Try getting the Window node */
    if (wn = GetWinNode (ai, NULL))
    {
	/* Is this a text menu? */
	if (wn->wn_Flags & APSH_WINF_TEXTMENU)
	{
	    text_menu = TRUE;
	}
    }

    /* Get the items */
    mnum = MENUNUM (msg->Code);
    inum = ITEMNUM (msg->Code);
    snum = SUBNUM (msg->Code);

    DM (kprintf ("m %d i %d s %d\n", mnum, inum, snum));

    /* Get the MenuItem structure address */
    if ((item = (struct EMenuItem *) ItemAddress (win->MenuStrip, (LONG) msg->Code)) &&
	item->emi_MenuID)
    {
	/* Is the command embedded in the UserData? */
	if (text_menu)
	{
	    register WORD i, j;

	    /* Build the command string */
	    strcpy (ai->ai_TempText, item->emi_MenuID);
#if 1
	    j = strlen (ai->ai_TempText);
	    for (i = 0; i < j; i++)
	    {
		if (ai->ai_TempText[i] == ' ')
		{
		    ai->ai_TempText[i] = 0;
		    i = j;
		}
	    }
#endif
	    sprintf (ai->ai_WorkText, "LINK %s", ai->ai_TempText);

	    /* Send the command */
	    DM (kprintf ("SendAmigaGuideCmd [%s]\n", ai->ai_WorkText));
	    SendAmigaGuideCmd (ai->ai_HyperText, ai->ai_WorkText, NULL);
	}
	else
	{
	    /* Set the AmigaGuide context */
	    DM (kprintf ("Set context %ld\n", item->emi_MenuID));
	    SetAmigaGuideContext (ai->ai_HyperText, item->emi_MenuID, NULL);

	    /* Display the node */
	    SendAmigaGuideContext (ai->ai_HyperText, NULL);
	}

    }
    else
    {
	/* No selectable item where help was pressed */
	DM (kprintf ("No item here\n"));
    }
}

/* Menu handling routine */
VOID HandleMenuEvent (struct AppInfo * ai, struct MsgHandler * mh,
		       struct IntuiMessage * msg, struct TagItem * tl)
{
    UWORD selection = msg->Code;
    struct EMenuItem *item;
    BOOL text_menu = FALSE;
    struct WinNode *wn;
    LONG cmd;

    /*
     * Shut down menu events while we're processing these. Any function that opens its own window and ignores events from the main
     * window, should also set a busy pointer in the main window.
     */
    msg->IDCMPWindow->Flags |= RMBTRAP;

    /* Try getting the Window node */
    if (wn = GetWinNode (ai, NULL))
    {
	/* Is this a text menu? */
	if (wn->wn_Flags & APSH_WINF_TEXTMENU)
	{
	    text_menu = TRUE;
	}
    }

    /* Process all menu events */
    while ((selection != MENUNULL) && (!ai->ai_Done))
    {
	/* Get the Extended MenuItem structure address */
	item = (struct EMenuItem *)
	  ItemAddress (msg->IDCMPWindow->MenuStrip, (LONG) selection);

	/* See if we have a command or a function ID */
	cmd = (LONG) item->emi_MenuID;

	/* Is the command embedded in the UserData? */
	if (text_menu)
	{
	    DM (kprintf ("PerfFunc %ld [%s]\n", cmd, item->emi_MenuID));
	    PerfFunc (ai, NULL, (STRPTR) item->emi_MenuID, tl);
	}
	else
	{
	    DM (kprintf ("PerfFunc %ld\n", cmd));
	    PerfFunc (ai, cmd, NULL, tl);
	}

	/* Get the next selection */
	selection = item->emi_Item.NextSelect;
    }

    /* Turn menu events back on. */
    msg->IDCMPWindow->Flags &= ~RMBTRAP;
}

/****** appshell.library/APSHGetGadgetInfo *********************************
*
*   NAME
*	APSHGetGadgetInfo - Obtain a pointer to a gadget and its window.
*
*   SYNOPSIS
*	success = APSHGetGadgetInfo (ai, winname, gadname, winptr, gadptr)
*
*	BOOL success;
*	struct AppInfo *ai;
*	STRPTR winname, gadname;
*	ULONG *winptr, *gadptr;
*
*   FUNCTION
*	Obtain a pointer to a gadget and the window that it belongs in.
*
*	Always use this function to get a pointer to a gadget before
*	manipulating it.  Never store the gadget or window pointers in your
*	UserData for the application.  It is possible that the window is
*	closed or been closed since the time you last obtained a gadget
*	pointer and are therefore referencing invalid memory locations.
*
*	This function is implemented by the IDCMP message handler.
*
*   EXAMPLE
*
*	\* Sample function showing how to use APSHGetGadgetInfo *\
*	VOID StubFunc (struct AppInfo * ai, STRPTR cmd, struct TagItem * tl)
*	{
*	    struct Gadget *gad;
*	    struct Window *win;
*
*	    \* Get a pointer to the main window, named MAIN, and the Scrolling
*	     * List gadget, which is named LIST.
*	     *\
*	    if (APSHGetGadgetInfo (ai, "MAIN", "LIST",
*				   (ULONG *)&win, (ULONG *)&gad))
*	    {
*		\* ... do something with the fields ... *\
*	    }
*	}
*
*
*   INPUTS
*	ai	- Pointer to the AppInfo structure
*	winname	- Pointer to the name of the window that the gadget is
*		  supposed to be located in.
*	gadname	- Pointer to the name of the gadget.
*	winptr	- Address of the variable to place the window pointer in.
*	gadptr	- Address of the variable to place the gadget pointer in.
*
*   RETURN
*	success	- TRUE indicates that the system was able to locate
*		  the gadget and that gadptr is set.  winptr can be
*		  used to determine if the window is actually open.
*
*		  FALSE indicates that the system was unable to locate
*		  the gadget.
*
*   BUGS
*	This version of the AppShell has stolen the window and gadget
*	UserData fields.  Until a method is provided to access per window
*	and gadget data, don't use the window->UserData or gadget->UserData.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL APSHGetGadgetInfo (struct AppInfo * ai,
			 STRPTR wname, STRPTR gname,
			 ULONG * window, ULONG * gadget)
{
    BOOL success = FALSE;
    struct MHObject *mho;
    struct WinNode *wn;
    struct List *list;

    /* get a pointer to the message handler data */
    DG (kprintf ("APSHGetGadgetInfo(0x%lx, %s, %s, 0x%lx, 0x%lx)\n", ai, wname, gname, window, gadget));
    if (wname && (mho = HandlerData (ai, APSH_Handler, "IDCMP", TAG_DONE)))
    {
	/* get a pointer on the data */
	list = &(mho->mho_ObjList);
	if (wn = (struct WinNode *) FindNameI (list, wname))
	{
	    /* Do we have a place to put the window pointer? */
	    if (window)
	    {
		/* Set the window pointer */
		*window = (ULONG) wn->wn_Win;
	    }

	    /* Are we supposed to get a gadget pointer? */
	    if (gname && gadget)
	    {
		struct ObjectInfo *oi = wn->wn_OI;
		struct ObjectNode *on;

		list = &(oi->oi_ObjList);

		mho = (struct MHObject *) wn;

		/* Can we find the gadget? */
		if (on = (struct ObjectNode *) FindNameI (list, gname))
		{
		    *gadget = (ULONG) on->on_Gadget;

		    if (wn->wn_Win)
		    {
			success = TRUE;
		    }
		}
		else
		{
		    DG (kprintf ("Can't find gadget [%s] in 0x%lx\n", gname, list));
		}
	    }
	    else if (wn->wn_Win)
	    {
		success = TRUE;
	    }
	}
	else
	{
	    DG (kprintf ("Can't find window [%s] in 0x%lx\n", wname, list));
	}
    }
    else
    {
	DG (kprintf ("Can't get handle on IDCMP\n"));
    }

    return (success);
}

/****** appshell.library/APSHGetWindowInfo *********************************
*
*   NAME
*	APSHGetWindowInfo - Obtain a pointer to a window.
*
*   SYNOPSIS
*	success = APSHGetWindowInfo (ai, winname, winptr)
*
*	BOOL success;
*	struct AppInfo *ai;
*	STRPTR winname;
*	ULONG *winptr;
*
*   FUNCTION
*	Obtain a pointer to the named window.
*
*	Always use this function to get a pointer to a window before
*	manipulating it.  Never store the window pointers in the application's
*	UserData.  It is possible that the window is closed or been closed
*	since the time you last obtained its pointer and are therefore
*	referencing invalid memory locations.
*
*	This function is implemented by the IDCMP message handler.
*
*   EXAMPLE
*
*	\* Sample function showing how to use APSHGetWindowInfo *\
*	VOID StubFunc (struct AppInfo * ai, STRPTR cmd, struct TagItem * tl)
*	{
*	    struct Window *win;
*
*	    \* Get a pointer to the main window, named MAIN *\
*	    if (APSHGetWindowInfo (ai, "MAIN", (ULONG)&win))
*	    {
*		\* ... do something with the fields ... *\
*	    }
*	}
*
*
*   INPUTS
*	ai	- Pointer to the AppInfo structure
*	winname	- Pointer to the name of the window.
*	winptr	- Address of the variable to place the window pointer in.
*
*   RETURN
*	success	- TRUE indicates that the system was able to locate
*		  the named window and that it was open.
*
*		  FALSE indicates that the system was unable to locate
*		  the named window or that it wasn't open.
*
*   BUGS
*	This version of the AppShell has stolen the window and gadget
*	UserData fields.  Until a method is provided to access per window
*	and gadget data, don't use the window->UserData or gadget->UserData.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL APSHGetWindowInfo (struct AppInfo * ai, STRPTR wname, ULONG * window)
{

    return (APSHGetGadgetInfo (ai, wname, NULL, window, NULL));
}
