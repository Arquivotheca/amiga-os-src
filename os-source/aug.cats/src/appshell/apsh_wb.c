/* apsh_wb.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Workbench message handling routines
 *
 * $Id: apsh_wb.c,v 1.4 1992/09/07 18:00:46 johnw Exp johnw $
 *
 * $Log: apsh_wb.c,v $
 * Revision 1.4  1992/09/07  18:00:46  johnw
 * Minor changes
 *
 * Revision 1.1  91/12/12  14:55:10  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:41:48  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

void kprintf (void *,...);

#define	DB(x)	;
#define	DF(x)	;

/* AppWindow/AppIcon functions */
#define	APSH_WBA_OPEN	0	/* after AppWindow is added to system */
#define	APSH_WBA_BDROP	1	/* called before processing icons */
#define	APSH_WBA_DDROP	2	/* called for each icon in the drop */
#define	APSH_WBA_ADROP	3	/* after the icons have been added to the list */
#define	APSH_WBA_CLOSE	4	/* called before the window is closed */
#define	APSH_WBA_REMOVE	5	/* called before removing the project list */
#define	APSH_WBA_DBLCLICK 6	/* called when the icon is double-clicked */

struct WBInfo
{
    BOOL wbi_OpenLib;		/* did we open the library? */
};

struct WBNode
{
    struct MHObject wbn_Header;	/* header */
    struct Window *wbn_Win;	/* pointer to the window */
    APTR wbn_AS;		/* pointer to App... structure */
    ULONG wbn_Flags;		/* flags */
    ULONG wbn_Funcs[10];	/* functions */
    UWORD wbn_Type;		/* type of node */
    struct Project wbn_Project;	/* embedded project structure */

    STRPTR wbn_WinName;		/* pointer to name of window */
    struct ObjNode *wbn_ObjNode;/* pointer to the object node */
    struct DiskObject *wbn_DOB;	/* current icon */

    UBYTE wbn_Name[1];		/* name of the node */
};

/****** appshell.library/setup_wbA *****************************************
*
*   NAME
*	setup_wbA - Initialize the Workbench message handler.
*
*   SYNOPSIS
*	mh = setup_wbA (ai, tags)
*	D0		D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *tags;
*
*   FUNCTION
*	This is the low-level Workbench message handler function used to
*	initialize the Workbench message hander.  This handles the AppIcon,
*	AppWindow and AppMenuItem features of Workbench.
*
*	NOTE: This call should not be called directly by the application,
*	but should invoked by using the APSH_AddWB_UI tag.
*
*	Valid tagitems to use at INITIALIZATION are:
*
*	    APSH_Status, <flags>
*		Where flags can be APSHP_INACTIVE or NULL.
*
*	    Any of the OPEN tags can be used to make a Workbench object
*	    active immediately.
*
*	Valid tagitems to use at OPEN are:
*
*	    These tags are used when adding a new entry to the Workbench
*	    message handler object list.
*
*	      APSH_AppWindowEnv, <tag array>
*		Where <tag array> is a tag array that describes the
*		AppWindow to add.
*
*	      APSH_AppIconEnv, <tag array>
*		Not implemented.
*
*	      APSH_AppMenuEnv, <tag array>
*		Not implemented.
*
*	    These tags are used to specify functions for a new entry in the
*	    Workbench message handler object list.
*
*	      APSH_AppOpen, <func ID>
*		Function to call after the object is opened, whenever it is
*		opened.
*
*	      APSH_AppBDrop, <func ID>
*		Function to call when an AppMessage is received, before any
*		further processing.
*
*	      APSH_AppDDrop, <func ID>
*		Function to call for each WBArg in an AppMessage argument
*		list.  The APSH_WBF_NOLIST must be set for this function
*		to be called.
*
*	      APSH_AppADrop, <func ID>
*		Function to call after the AppMessage has been processed.
*
*	      APSH_AppClose, <func ID>
*		Function to call before closing a Workbench message handler
*		object.
*
*	      APSH_AppRemove, <func ID>
*		Function to call before removing a Workbench message handler
*		object.
*
*	      APSH_AppDblClick, <func ID>
*		Function to call when an AppIcon has been double-clicked.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	tags	- Pointer to an array of TagItem attributes for the
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

struct MsgHandler *setup_wbA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh;
    register struct MHObject *mho;
    register struct WBInfo *md;
    ULONG msize, hstatus;
    STRPTR pname;

    DB (kprintf ("setup_wbA:\n"));

    /* make sure Workbench is around */
    if (WorkbenchBase)
    {
	/* get the port name */
	strcpy (ai->ai_WorkText, ai->ai_BaseName);
	strcat (ai->ai_WorkText, "_WB");
	strupr (ai->ai_WorkText);
	pname = (STRPTR) GetTagData (APSH_Port, (ULONG) ai->ai_WorkText, tl);

	/* compute the size of our memory block */
	msize = sizeof (struct MsgHandler)
	  + sizeof (struct WBInfo)
	  + (4L * sizeof (ULONG));

	/* allocate message handler memory block */
	if (mh = (struct MsgHandler *)
	    AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
	{
	    ai->ai_MH[MH_WB] = mh;

	    /* get a pointer on the object node */
	    mho = &(mh->mh_Header);

	    /* initialize the message handler node structure */
	    mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	    mho->mho_Node.ln_Pri = (APSH_MH_HANDLER_P + 1);
	    mho->mho_Node.ln_Name = "WB";

	    /* initialize the message handler object list */
	    NewList (&(mho->mho_ObjList));

	    /* set the message handler base ID */
	    mho->mho_ID = APSH_WB_ID;

	    /* set up the message handler data area */
	    mho->mho_SysData = md = MEMORY_FOLLOWING (mh);

	    /* set up the message handler function table */
	    mh->mh_NumFuncs = 4;
	    mh->mh_Func = MEMORY_FOLLOWING (md);
	    mh->mh_Func[APSH_MH_OPEN] = open_wb;
	    mh->mh_Func[APSH_MH_HANDLE] = handle_wb;
	    mh->mh_Func[APSH_MH_CLOSE] = close_wb;
	    mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_wb;

	    /* get the activation status */
	    hstatus = GetTagData (APSH_Status, NULL, tl);

	    /* set up the port name */
	    msize = strlen (pname) + 6L;
	    mh->mh_PortName = (STRPTR) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC);
	    strcpy (mh->mh_PortName, pname);

	    /* only continue if we have Workbench library opened */
	    if (WorkbenchBase)
	    {
		/* make sure the port doesn't already exist */
		Forbid ();

#if 0
		if (FindPort (mh->mh_PortName))
		{
		    /* re-enable multi-tasking */
		    Permit ();

		    /* set up the error return values */
		    ai->ai_Pri_Ret = RETURN_FAIL;
		    ai->ai_Sec_Ret = APSH_PORT_ACTIVE;
		    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID,
				       ai->ai_Sec_Ret, (int) mh->mh_PortName);
		}
		else
		{
		    /* set up a port for WB replys */
		    if (mh->mh_Port = CreatePort (mh->mh_PortName, NULL))
		    {
			/* re-enable multi-tasking */
			Permit ();

			/* set up the signal bits */
			mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

			/* open immediately? */
			if (hstatus & APSHP_INACTIVE)
			    return (mh);
			else
			{
			    if (open_wb (ai, mh, tl))
			    {
				return (mh);
			    }
			}

			/* clean failure path */
			DeletePort (mh->mh_Port);
			mh->mh_Port = NULL;
		    }
		    else
		    {
			/* re-enable multi-tasking */
			Permit ();

			/* set up the error return values */
			ai->ai_Pri_Ret = RETURN_FAIL;
			ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
			ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID,
				       ai->ai_Sec_Ret, (int) mh->mh_PortName);
		    }
		}		/* end of if FindPort */
#else
		/* set up a port for WB replys */
		if (mh->mh_Port = CreatePort (NULL, NULL))
		{
		    /* re-enable multi-tasking */
		    Permit ();

		    /* set up the signal bits */
		    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

		    /* open immediately? */
		    if (hstatus & APSHP_INACTIVE)
			return (mh);
		    else
		    {
			if (open_wb (ai, mh, tl))
			{
			    return (mh);
			}
		    }

		    /* clean failure path */
		    DeletePort (mh->mh_Port);
		    mh->mh_Port = NULL;
		}
		else
		{
		    /* re-enable multi-tasking */
		    Permit ();

		    /* set up the error return values */
		    ai->ai_Pri_Ret = RETURN_FAIL;
		    ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
		    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID,
				       ai->ai_Sec_Ret, (int) mh->mh_PortName);
		}
#endif
	    }			/* end of if WorkbenchBase */

	    /* free the name if we allocated it */
	    if (mh->mh_PortName)
		FreeVec ((APTR) mh->mh_PortName);

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
    }
    else
    {
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_AVAILABLE;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret,
				   (int) "workbench.library");
    }

    return (mh);
}

BOOL open_wb (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct MHObject *cmho = NULL;
    struct List *list = &(mho->mho_ObjList);
    struct WBNode *wbn;
    struct Window *win = NULL;
    struct TagItem *wbe;
    STRPTR name;
    ULONG msize, flags, cntri, cntrj;

    /* set our status */
    mho->mho_Status |= APSHF_OPEN;

    if (wbe = (struct TagItem *) GetTagData (APSH_AppWindowEnv, NULL, tl))
    {
	/* get the window name to attach this to */
	name = (STRPTR) GetTagData (APSH_NameTag, (ULONG) "MAIN", wbe);
	flags = GetTagData (APSH_CmdFlags, NULL, wbe);

	/* see if this entry already exists */
	if (!(wbn = (struct WBNode *) FindNameI (list, name)))
	{
	    /* compute the memory requirements */
	    msize = sizeof (struct WBNode) + strlen (name) + 2L;

	    /* allocate a WBNode structure */
	    if (wbn = (struct WBNode *) AllocVec (msize, MEMF_CLEAR))
	    {
		/* copy the name */
		strcpy (wbn->wbn_Name, name);

		/* initialize the node information */
		cmho = (struct MHObject *) wbn;
		cmho->mho_Node.ln_Name = wbn->wbn_Name;
		cmho->mho_Parent = mho;

		/* set the flags */
		wbn->wbn_Flags = flags;

		/* set the callback functions */
		for (cntri = APSH_WBA_OPEN, cntrj = APSH_AppOpen;
		     cntrj <= APSH_AppDblClick;
		     cntri++, cntrj++)
		{
		    wbn->wbn_Funcs[cntri] =
		      GetTagData (cntrj, NO_FUNCTION, wbe);
		}

		/* get a pointer to MHObject structure */
		cmho = &(wbn->wbn_Header);

		/* initialize the lists */
		NewList (&(cmho->mho_ObjList));
		NewList (&(wbn->wbn_Project.p_ProjList));

		/* set the current node to point to this one */
		mho->mho_CurNode = (struct MHObject *) wbn;

		/* say that is a AppWindow */
		wbn->wbn_Type = MTYPE_APPWINDOW;

		/* add the node to the list */
		AlphaEnqueue (list, (struct Node *) wbn);
	    }			/* end of if AllocVec */
	}			/* end of if FindNameI */

    }				/* end of if AppWindowEnv */

    /* get the Workbench node */
    if ((name = (STRPTR) GetTagData (APSH_NameTag, NULL, tl)))
    {
	wbn = (struct WBNode *) FindNameI (list, name);
    }
    else
    {
	wbn = (struct WBNode *) mho->mho_CurNode;
    }

    /* make sure we got the node */
    if (wbn)
    {
	/* get the name */
	name = wbn->wbn_Name;

	/* get a pointer to the window */
	if (APSHGetWindowInfo (ai, name, (ULONG *) & win))
	{
	    struct WinNode *wn;

	    /* keep a pointer to the window */
	    wbn->wbn_Win = win;

	    /* tell the IDCMP handler that there is a Workbench hook */
	    wn = (struct WinNode *) win->UserData;
	    wn->wn_Flags |= APSH_WINF_APPWIN;

	    /* add it, depending on type */
	    switch (wbn->wbn_Type)
	    {
		case MTYPE_APPWINDOW:
		    /* try to add the AppWindow */
		    if (wbn->wbn_AS =
		     AddAppWindow (NULL, (ULONG) wbn, win, mh->mh_Port, NULL))
		    {
			ULONG FuncID;

			/* perform the open callback */
			if ((FuncID = wbn->wbn_Funcs[APSH_WBA_OPEN]) != NO_FUNCTION)
			    PerfFunc (ai, FuncID, NULL, NULL);
		    }
		    else
		    {
		    }		/* end of if AddAppWindow */
		    break;
		default:
		    break;
	    }
	}
    }

    return (TRUE);
}

VOID StepThruList (struct AppInfo * ai, struct WBNode * wbn,
		    struct AppMessage * msg)
{
    struct Window *win = wbn->wbn_Win;
    ULONG FuncID;

    if ((FuncID = wbn->wbn_Funcs[APSH_WBA_DDROP]) != NO_FUNCTION)
    {
	UWORD i, j;
	struct TagItem tag[8];

	/* initialize the tag list */
	tag[0].ti_Tag = APSH_WinPointer;
	tag[0].ti_Data = (ULONG) win;
	tag[1].ti_Tag = APSH_NameTag;
	tag[1].ti_Data = (ULONG) wbn->wbn_Header.mho_Node.ln_Name;
	tag[2].ti_Tag = APSH_WBArg;
	tag[3].ti_Tag = APSH_MsgMouseX;
	tag[3].ti_Data = (LONG) (win->MouseX);
	tag[4].ti_Tag = APSH_MsgMouseY;
	tag[4].ti_Data = (LONG) (win->MouseY);
	tag[5].ti_Tag = TAG_DONE;

	/* step through the argument list */
	j = (UWORD) msg->am_NumArgs;
	for (i = 0; i < j; i++)
	{
	    /* get a pointer to the current drop unit */
	    tag[2].ti_Data = (ULONG) & (msg->am_ArgList[i]);

	    /* perform the drop function */
	    PerfFunc (ai, FuncID, NULL, tag);
	}
    }
}

/* handle WB messages */
BOOL handle_wb (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct AppMessage *msg;
    struct WBNode *wbn;
    struct Window *win;
    struct TagItem tag[5];
    ULONG FuncID;

    /* Set the active message handler ID */
    ai->ai_ActvMH = mho->mho_ID;

    /* handle all the Workbench messages */
    while (msg = (struct AppMessage *) GetMsg (mh->mh_Port))
    {
	/* Set the active message */
	ai->ai_ActvMsg = (struct Message *) msg;

	/* get the WBNode from the user data field */
	wbn = (struct WBNode *) msg->am_UserData;

	/* activate the window that the drop was for */
	if (win = wbn->wbn_Win)
	{
	    /* initialize the tag types */
	    tag[0].ti_Tag = APSH_WinPointer;
	    tag[0].ti_Data = (ULONG) win;
	    tag[1].ti_Tag = TAG_DONE;

	    /* set the wait pointer */
	    APSHSetWaitPointer (ai, tag);

	    /* activate the window */
/*	    PerfFunc (ai, ActivateID, NULL, tag); */

	    ActivateWindow (win);
	}

	/* initialize the tag types */
	tag[0].ti_Tag = APSH_ProjInfo;
	tag[1].ti_Tag = TAG_DONE;

	/* different types do different things */
	switch (msg->am_Type)
	{
	    case MTYPE_APPWINDOW:
		/* Before drop function */
		if ((FuncID = wbn->wbn_Funcs[APSH_WBA_BDROP]) != NO_FUNCTION)
		    PerfFunc (ai, FuncID, NULL, NULL);

		if (wbn->wbn_Flags & APSH_WBF_PROJLIST)
		{
		    /* add the projects to the Workbench project list */
		    AddProjects (ai, msg->am_NumArgs, msg->am_ArgList, NULL);
		}
		else if (wbn->wbn_Flags & APSH_WBF_NOLIST)
		{
		    /* step through the list and call DDROP for each */
		    StepThruList (ai, wbn, msg);
		}
		else
		{
		    /* add the projects to the project component list */
		    tag[0].ti_Data = (ULONG) & (wbn->wbn_Project.p_ProjList);
		    AddProjects (ai, msg->am_NumArgs, msg->am_ArgList, tag);
		}

		/* After the drop function */
		if ((FuncID = wbn->wbn_Funcs[APSH_WBA_ADROP]) != NO_FUNCTION)
		    PerfFunc (ai, FuncID, NULL, NULL);
		break;

	    case MTYPE_APPICON:
	    case MTYPE_APPMENUITEM:
	    default:
		break;
	}

	/* clear the pointer */
	if (win = wbn->wbn_Win)
	{
	    /* initialize the tag types */
	    tag[0].ti_Tag = APSH_WinPointer;
	    tag[0].ti_Data = (ULONG) win;
	    tag[1].ti_Tag = TAG_DONE;

	    /* set the wait pointer */
	    APSHClearPointer (ai, tag);
	}

	/* reply to the message */
	ReplyMsg ((struct Message *) msg);
    }

    /* Clear the active stuff */
    ai->ai_ActvMsg = NULL;
    ai->ai_ActvMH = NULL;

    return (TRUE);
}

BOOL close_wb (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);
    STRPTR name;

    /* get the name of the App... to remove */
    if (name = (STRPTR) GetTagData (APSH_NameTag, NULL, tl))
    {
	struct List *list;
	struct WBNode *wbn;
	ULONG FuncID;

	list = &(mho->mho_ObjList);
	if (wbn = (struct WBNode *) FindNameI (list, name))
	{
	    /* before closing */
	    if ((FuncID = wbn->wbn_Funcs[APSH_WBA_CLOSE]) != NO_FUNCTION)
		PerfFunc (ai, FuncID, NULL, NULL);

	    switch (wbn->wbn_Type)
	    {
		case MTYPE_APPWINDOW:
		    /* Do we have something to remove */
		    if (wbn->wbn_AS)
		    {
			RemoveAppWindow (wbn->wbn_AS);
			wbn->wbn_AS = NULL;
		    }
		    break;

		case MTYPE_APPICON:
		case MTYPE_APPMENUITEM:
		default:
		    break;
	    }
	}
    }

    return (TRUE);
}

BOOL shutdown_wb (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho = &(mh->mh_Header);

    if (mh)
    {
	struct List *list;

	/* remove the message handler from the list */
	Remove ((struct Node *) mh);

	/* remove all the App... stuff */
	list = &(mho->mho_ObjList);
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct WBNode *wbn;
	    struct Node *node, *next;
	    STRPTR name;
	    struct TagItem tag[3];
	    ULONG FuncID;

	    tag[1].ti_Tag = TAG_DONE;

	    node = list->lh_Head;
	    while (next = node->ln_Succ)
	    {
		/* cast the node pointer to a WBNode pointer */
		wbn = (struct WBNode *) node;

		/* get the name of current node */
		name = wbn->wbn_Header.mho_Node.ln_Name;
		tag[0].ti_Tag = APSH_NameTag;
		tag[0].ti_Data = (ULONG) name;

		/* remove the App... */
		close_wb (ai, mh, tag);

		/* before removing the project list */
		if ((FuncID = wbn->wbn_Funcs[APSH_WBA_REMOVE]) != NO_FUNCTION)
		    PerfFunc (ai, FuncID, NULL, NULL);

		/* remove the projects from its list */
		tag[0].ti_Tag = APSH_ProjInfo;
		tag[0].ti_Data = (ULONG) & (wbn->wbn_Project.p_ProjList);
		FreeProjects (ai, tag);

		/* remove the node from the list */
		Remove ((struct Node *) wbn);

		/* free the memory used */
		FreeVec (wbn);

		/* go on to the next node */
		node = next;
	    }
	}

	ai->ai_MH[MH_WB] = NULL;

	/* delete the message port */
	if (mh->mh_Port)
	    RemoveMsgPort (mh->mh_Port);

	/* free the name if we allocated it */
	if (mh->mh_PortName)
	    FreeVec ((APTR) mh->mh_PortName);

	/* free our data */
	FreeVec ((APTR) mh);
    }

    return (TRUE);
}
