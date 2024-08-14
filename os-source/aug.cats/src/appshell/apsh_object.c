/* apsh_object.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * object manipulation routines for the AppShell
 *
 */

#include "appshell_internal.h"
#include <exec/libraries.h>

VOID kprintf (VOID *, ...);
#define	DH(x)	;

#define	NEED_LOCKING	FALSE

/* This is what we allocate */
struct AppObject
{
    LONG ao_Kind;
    APTR ao_Object;
};

struct KindAppInfo
{
    struct AppInfo kai_AI;
    struct AppNode kai_AN;
    UBYTE kai_Buffer[512];
};

APTR __asm NewAPSHObject (
    register __a0 struct AppInfo *ai,
    register __d0 LONG kind,
    register __a1 struct TagItem *tl)
{
    struct LibBase *lb = ((struct ExtLibrary *) AppShellBase)->base;
    struct AppObject *ao = NULL;
    struct SignalSemaphore *ss;
    ULONG msize = 0L;
    ULONG mtype;

    /* Set default memory flags */
    mtype = MEMF_CLEAR | MEMF_PUBLIC;

    /* Cache the semaphore pointer */
    ss = &(lb->lb_Lock);

    /* Figure out how much memory we need */
    switch (kind)
    {
	case APSH_KIND_APPINFO:
	    /* How much memory do we allocate for the AppInfo structure */
	    msize = sizeof (struct KindAppInfo);
	    break;

	case APSH_KIND_SIPCMSG:
	    msize = sizeof (struct SIPCMessage);
	    break;

	case APSH_KIND_MSGHANDLER:
	    msize = sizeof (struct MsgHandler);
	    break;

	case APSH_KIND_MHOBJECT:
	    msize = sizeof (struct MHObject);
	    break;

	case APSH_KIND_ANCHORLIST:
	    msize = sizeof (struct AnchorList);
	    break;

	case APSH_KIND_PROJECT:
	    msize = sizeof (struct Project);
	    break;

	case APSH_KIND_PROJNODE:
	    msize = sizeof (struct ProjNode);
	    break;
    }

    /* Do we have a size to allocate? */
    if (msize)
    {
	/* Add in the overhead size */
	msize += sizeof (struct AppObject);

	/* Allocate the object */
	if (ao = (struct AppObject *) AllocVec (msize, mtype))
	{
	    /* Set the object kind */
	    ao->ao_Kind = kind;

	    /* Set up the object pointer */
	    ao->ao_Object = MEMORY_FOLLOWING (ao);

	    /* Do the rest of the initialization */
	    switch (kind)
	    {
		case APSH_KIND_APPINFO:
		    {
			struct KindAppInfo *kai = ao->ao_Object;
			struct AppInfo *tai = &(kai->kai_AI);
			struct AppNode *an = &(kai->kai_AN);
			STRPTR aname = "<no name>";
			struct Funcs *ft;
			struct Hook *h;
			ULONG msize;
			STRPTR tmp;

			tai->ai_AN = an;
			an->an_AI = tai;

			/* Set the FailAt level */
			tai->ai_FailAt = RETURN_WARN;

			/* Get the application name */
			aname = (STRPTR)GetTagData (APSH_AppName, (ULONG) aname, tl);
			an->an_Node.ln_Name = tai->ai_AppName = aname;

			/* See if they are supplying a replacement hook */
			h = (struct Hook *)
			    GetTagData (APSH_DispatchHook, (ULONG)&(lb->lb_Hook), tl);

			/* See if they're new hook smart */
			if (FindTagItem (APSH_HookClass, tl))
			{
			    DH (kprintf ("Use new-style hook\n"));
			    /* Use the new style hook */
			    h = &(lb->lb_NewHook);
			}
			else
			{
			    DH (kprintf ("Use old-style hook\n"));
			}

			/* Copy the hook */
			tai->ai_Hook = *h;

			/* Get the application base name */
			tai->ai_BaseName = NULL;
			if (tmp = (STRPTR)
			    GetTagData (APSH_BaseName, (ULONG) aname, tl))
			{
			    msize = strlen (tmp) + 1;
			    if (tai->ai_BaseName = (STRPTR) AllocVec (msize, MEMF_CLEAR))
			    {
				strcpy (tai->ai_BaseName, tmp);
				strupr (tai->ai_BaseName);
			    }
			}

			/* Initialize the data lock */
			InitSemaphore (&(an->an_Lock));
			InitSemaphore (&(tai->ai_Lock));

			/* Get a pointer to the task */
			tai->ai_Process = (struct Process *) FindTask (NULL);

			/* Get a pointer to the user data */
			if (msize = GetTagData (APSH_UserDataSize, NULL, tl))
			{
			    /* Try allocating space for the user data */
			    if (tai->ai_UserData = (APTR) AllocVec (msize, MEMF_CLEAR))
			    {
				/* Show that we allocated the UserData */
				tai->ai_Flags |= APSHF_USERDATA;
			    }
			    else
			    {
				/* Not enough memory to allocate UserData */
				FreeVec (ao);
				return (NULL);
			    }
			}
			else
			{
			    /* Get a pointer to the user data */
			    tai->ai_UserData = (APTR) GetTagData (APSH_UserData, NULL, tl);
			}

			/* Initialize the command ReadArgs information */
			tai->ai_CmdRDA.RDA_Buffer = kai->kai_Buffer;
			tai->ai_CmdRDA.RDA_BufSiz = sizeof (UBYTE) * 512L;
			tai->ai_CmdRDA.RDA_Flags = (RDAF_NOALLOC | RDAF_NOPROMPT);

			/* compute space needed for options */
			msize = (sizeof (LONG) * MAX_TEMPLATE_ITEMS);
			if (tai->ai_Options = (LONG *) AllocVec (msize, MEMF_CLEAR))
			{
			    /* Initialize the message handler list */
			    NewList (&(tai->ai_MsgList));

			    /* Initialize the project list */
			    NewList (&(tai->ai_Project.p_ProjList));

			    /* Initialize the function table list */
			    NewList (&(tai->ai_FuncList));
			    NewList (&(tai->ai_FuncList2));

			    /* Initialize the preference list */
			    NewList (&(tai->ai_PrefList));

			    /* Initialize the image list */
			    NewList (&(tai->ai_ImageList));

			    /* Initialize the pattern matching stuff */
			    tai->ai_AP.ap_BreakBits = SIGBREAKF_CTRL_C;

			    /* get the function table */
			    if (ft = (struct Funcs *) GetTagData (APSH_FuncTable, NULL, tl))
			    {
				AddFuncEntries (tai, ft);
			    }

			    /* Add the entry to the master list */
			    {
				/* obtain the semaphore */
				ObtainSemaphore (ss);

				/* Add the node to the Application list */
				AddTail (&(lb->lb_AppList), (struct Node *) an);

				/* Increment the application count */
				lb->lb_AppCount++;

				/* Is the AppExchange application running */
				if (lb->lb_Exchange)
				{
				    /* Send a signal */
				    Signal ((struct Task *) lb->lb_Exchange->ai_Process,
					    SIGBREAKF_CTRL_E);
				}

				/* release the lock */
				ReleaseSemaphore (ss);
			    }
			}
			else
			{
			    /* Unable to allocate a component, so free the
			     * whole thing. */
			    FreeVec (ao);
			    return (ao);
			}
		    }
		    break;

		case APSH_KIND_SIPCMSG:
		    {
			struct SIPCMessage *msg = (struct SIPCMessage *)
			ao->ao_Object;

			/* Fill out the Exec message */
			msg->sipc_Msg.mn_Node.ln_Type = NT_MESSAGE;
			msg->sipc_Msg.mn_Length = sizeof (struct SIPCMessage);
		    }
		    break;

		case APSH_KIND_MSGHANDLER:
		    break;

		case APSH_KIND_MHOBJECT:
		    break;

		case APSH_KIND_ANCHORLIST:
		    break;

		case APSH_KIND_PROJECT:
		    break;

		case APSH_KIND_PROJNODE:
		    break;
	    }

	    /* Set the attributes */
	    SetAPSHAttr (ai, ao->ao_Object, tl);
	}
    }

    return (ao->ao_Object);
}

VOID __asm DisposeAPSHObject (
    register __a0 struct AppInfo * ai,
    register __a1 APTR data)
{
    struct LibBase *lb = ((struct ExtLibrary *) AppShellBase)->base;
    ULONG mad = (ULONG) data;
    struct AppObject *ao;

    if (ai && data)
    {
	struct SignalSemaphore *ss = &(lb->lb_Lock);

	/* Obtain the semaphore */
	ObtainSemaphore (ss);

	/* Get a pointer to the AppObject structure */
	ao = (struct AppObject *) (mad - sizeof (struct AppObject));

	switch (ao->ao_Kind)
	{
	    case APSH_KIND_APPINFO:
		{
		    struct AppInfo *tai = (struct AppInfo *) ao->ao_Object;

		    /* Remove the entry from the master list */
		    {
			/* See if we can find the node */
			if (tai->ai_AN)
			{
			    /* Remove the node from the master list */
			    Remove ((struct Node *) tai->ai_AN);
			}

			/* Decrement the application count */
			lb->lb_AppCount--;

			/* Is the AppExchange application running */
			if (lb->lb_Exchange)
			{
			    /* Send a signal */
			    Signal ((struct Task *) lb->lb_Exchange->ai_Process,
				    SIGBREAKF_CTRL_E);
			}
		    }

		    /* Are we in charge of the UserData? */
		    if ((tai->ai_Flags & APSHF_USERDATA) && tai->ai_UserData)
		    {
			FreeVec (tai->ai_UserData);
			tai->ai_UserData = NULL;
		    }

		    /* free the project list */
		    FreeProjects (tai, NULL);

		    /* free the function table */
		    FreeFuncEntries (tai);

		    /* base directory */
		    if (tai->ai_ProgDir)
		    {
			UnLock (tai->ai_ProgDir);
			tai->ai_ProgDir = NULL;
		    }

		    /* configuration directory */
		    if (tai->ai_ConfigDir)
		    {
			UnLock (tai->ai_ConfigDir);
			tai->ai_ConfigDir = NULL;
		    }

		    /* tool icon */
		    if (tai->ai_ProgDO)
		    {
			FreeDiskObject (tai->ai_ProgDO);
			tai->ai_ProgDO = NULL;
		    }

		    /* Base name */
		    FreeVec (tai->ai_BaseName);
		    tai->ai_BaseName = NULL;

		    /* base name */
		    FreeVec (tai->ai_ProgName);
		    tai->ai_ProgName = NULL;

		    /* options buffer */
		    FreeVec ((APTR) tai->ai_Options);
		    tai->ai_Options = NULL;

		    /* Do we have a parsed readargs? */
		    FreeVec (tai->ai_Template);
		    tai->ai_Template = NULL;
		}
		break;

	    default:
		break;
	}

	/* Now free the entire memory block */
	FreeVec (ao);

	/* Release the lock */
	ReleaseSemaphore (ss);
    }
}

ULONG setAppInfoAttrs (struct AppInfo *ai, struct TagItem *tl)
{
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;

    /* process rest */
    tstate = tl;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case APSH_TextRtn:
		ai->ai_TextRtn = (STRPTR) tidata;
		break;

	    case APSH_PrimaryReturn:
		ai->ai_Pri_Ret = (LONG) tidata;
		break;

	    case APSH_SecondaryReturn:
		ai->ai_Sec_Ret = (LONG) tidata;
		break;

	    case APSH_Done:
		ai->ai_Done = (BOOL) tidata;
		break;

	    case APSH_FailAt:
		ai->ai_FailAt = tidata;
		break;

	    case APSH_ActvMH:
		ai->ai_ActvMH = tidata;
		break;

	    case APSH_ActvMessage:
		ai->ai_ActvMsg = (struct Message *) tidata;
		break;
	}
    }

    return (1L);
}

ULONG __asm SetAPSHAttr (
    register __a0 struct AppInfo * ai,
    register __a1 APTR data,
    register __a2 struct TagItem * tl)
{
    ULONG mad = (ULONG) data;
    struct AppObject *ao;
    ULONG retval = FALSE;

    if (ai && data && tl)
    {
	/* Get a pointer to the AppObject structure */
	ao = (struct AppObject *) (mad - sizeof (struct AppObject));

	switch (ao->ao_Kind)
	{
	    case APSH_KIND_APPINFO:
		/* Call the AppInfo stuff */
		retval = setAppInfoAttrs (ai, tl);
		break;

	    case APSH_KIND_SIPCMSG:
		break;

	    case APSH_KIND_MSGHANDLER:
		break;

	    case APSH_KIND_MHOBJECT:
		break;

	    case APSH_KIND_ANCHORLIST:
		break;

	    case APSH_KIND_PROJECT:
		break;

	    case APSH_KIND_PROJNODE:
		break;
	}
    }

    return (retval);
}

ULONG getAppInfoAttrs (struct AppInfo *ai, ULONG attrID, ULONG *storage)
{
    ULONG retval = TRUE;

    switch (attrID)
    {
	/* From the previously public parts of AppInfo */
	case APSH_TextRtn:
	    *storage = (ULONG) ai->ai_TextRtn;
	    break;

	case APSH_PrimaryReturn:
	    *storage = (ULONG) ai->ai_Pri_Ret;
	    break;

	case APSH_SecondaryReturn:
	    *storage = (ULONG) ai->ai_Sec_Ret;
	    break;

	case APSH_Done:
	    *storage = (ULONG) ai->ai_Done;
	    break;

	case APSH_Startup:
	    *storage = (ULONG) ai->ai_Startup;
	    break;

	case APSH_Options:
	    *storage = (ULONG) ai->ai_Options;
	    break;

	case APSH_NumOpts:
	    *storage = (ULONG) ai->ai_NumOpts;
	    break;

	case APSH_ArgsPtr:
	    *storage = (ULONG) ai->ai_ArgsPtr;
	    break;

	case APSH_FileArray:
	    *storage = (ULONG) ai->ai_FileArray;
	    break;

	case APSH_ToolIcon:
	    *storage = (ULONG) ai->ai_ProgDO;
	    break;

	case APSH_ProgramDir:
	    *storage = (ULONG) ai->ai_ProgDir;
	    break;

	case APSH_PrefsDir:
	    *storage = (ULONG) ai->ai_ConfigDir;
	    break;

	case APSH_PrefsList:
	    *storage = (ULONG) &(ai->ai_PrefList);
	    break;

	case APSH_BaseName:
	    *storage = (ULONG) ai->ai_BaseName;
	    break;

	case APSH_ProgName:
	    *storage = (ULONG) ai->ai_ProgName;
	    break;

	case APSH_AppName:
	    *storage = (ULONG) ai->ai_AppName;
	    break;

	case APSH_AppVersion:
	    *storage = (ULONG) ai->ai_AppVersion;
	    break;

	case APSH_AppCopyright:
	    *storage = (ULONG) ai->ai_AppCopyright;
	    break;

	case APSH_AppAuthor:
	    *storage = (ULONG) ai->ai_AppAuthor;
	    break;

	case APSH_AppMsgTitle:
	    *storage = (ULONG) ai->ai_AppMsgTitle;
	    break;

	case APSH_ProjInfo:
	    *storage = (ULONG) &(ai->ai_Project);
	    break;

	case APSH_UserData:
	    *storage = (ULONG) ai->ai_UserData;
	    break;

	case APSH_ScreenName:
	    *storage = (ULONG) ai->ai_ScreenName;
	    break;

	case APSH_ScreenPtr:
	    *storage = (ULONG) ai->ai_Screen;
	    break;

	case APSH_Font:
	    *storage = (ULONG) ai->ai_Font;
	    break;

	case APSH_ActvWindow:
	    *storage = (ULONG) ai->ai_Window;
	    break;

	case APSH_ActvGadget:
	    *storage = (ULONG) ai->ai_Gadget;
	    break;

	case APSH_ActvObject:
	    *storage = (ULONG) ai->ai_CurObj;
	    break;

	case APSH_DrawInfo:
	    *storage = (ULONG) ai->ai_DI;
	    break;

	case APSH_VisualInfo:
	    *storage = (ULONG) ai->ai_VI;
	    break;

	case APSH_MouseX:
	    *storage = (ULONG) ai->ai_MouseX;
	    break;

	case APSH_MouseY:
	    *storage = (ULONG) ai->ai_MouseY;
	    break;

	/* From the private parts of AppInfo */
	case APSH_DefText:
	    *storage = (ULONG) ai->ai_DefText1;
	    break;

	case APSH_NumCommands:
	    *storage = (ULONG) ai->ai_NumCmds;
	    break;

	case APSH_Process:
	    *storage  = (ULONG) ai->ai_Process;
	    break;

	case APSH_FailAt:
	    *storage = (ULONG) ai->ai_FailAt;
	    break;

	case APSH_ActvMH:
	    *storage = (ULONG) ai->ai_ActvMH;
	    break;

	case APSH_ActvMessage:
	    *storage = (ULONG) ai->ai_ActvMsg;
	    break;

	case APSH_LastError:
	    *storage = (ULONG) ai->ai_LastError;
	    break;

	case APSH_LastText:
	    *storage = (ULONG) ai->ai_LastText;
	    break;

	case APSH_ARexxMH:
	    *storage = (ULONG) ai->ai_MH[MH_AREXX];
	    break;

	case APSH_CommandShellMH:
	    *storage = (ULONG) ai->ai_MH[MH_DOS];
	    break;

	case APSH_IntuitionMH:
	    *storage = (ULONG) ai->ai_MH[MH_IDCMP];
	    break;

	case APSH_SIPCMH:
	    *storage = (ULONG) ai->ai_MH[MH_SIPC];
	    break;

	case APSH_ToolMH:
	    *storage = (ULONG) ai->ai_MH[MH_TOOL];
	    break;

	case APSH_WorkbenchMH:
	    *storage = (ULONG) ai->ai_MH[MH_WB];
	    break;

	/* Don't understand this tag */
	default:
	    retval = FALSE;
	    break;
    }

    return (retval);
}

ULONG __asm GetAPSHAttr (
    register __a0 struct AppInfo * ai,
    register __d0 LONG attr,
    register __a1 APTR data,
    register __a2 APTR storage)
{
    ULONG mad = (ULONG) data;
    struct AppObject *ao;
    ULONG retval = FALSE;

    if (ai && attr && data && storage)
    {
	/* Get a pointer to the AppObject structure */
	ao = (struct AppObject *) (mad - sizeof (struct AppObject));

	switch (ao->ao_Kind)
	{
	    case APSH_KIND_APPINFO:
		/* Call the AppInfo stuff */
		retval = getAppInfoAttrs (ai, attr, storage);
		break;
	}
    }

    return (retval);
}
