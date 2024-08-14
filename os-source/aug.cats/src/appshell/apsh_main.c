/* apsh_main.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * main entry modules for the AppShell
 *
 * $Id: apsh_main.c,v 1.4 1992/09/07 17:56:35 johnw Exp johnw $
 *
 * $Log: apsh_main.c,v $
 * Revision 1.4  1992/09/07  17:56:35  johnw
 * Many minor and not-so-minor changes.
 *
 * Revision 1.1  91/12/12  14:52:09  davidj
 * Initial revision
 *
 * Revision 1.1  90/07/02  11:35:42  davidj
 * Initial revision
 *
 *
 */

#define STRINGARRAY
#include "apsh_catalog.h"

#include "appshell_internal.h"
#include <exec/libraries.h>

void kprintf(void *,...);
#define	DA(x)	;
#define	DB(x)	;
#define	DN(x)	;
#define	DV(x)	;
#define	DT(x)	;
#define	DF(x)	;
#define	DI(x)	;
#define	DS(x)	;
#define	DR(x)	;

STRPTR Template = "Files/M,PubScreen/K,PortName/K,Startup/K,NOGUI/S";
ULONG NumOpts = 5L;
extern struct Funcs handler_funcs[];
extern struct Funcs idcmp_funcs[];

/****** appshell.library/HandleApp *****************************************
*
*   NAME
*	HandleApp - startup function for AppShell application
*
*   SYNOPSIS
*	results = HandleApp (argc, argv, wbm, attrs)
*	D0		     D0    D1    A0   A1
*
*	BOOL results;
*	int argc;
*	char **argv;
*	struct WBStartup *wbm;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function is the application entry point to the AppShell.
*
*   EXAMPLE
*
*	\* ... application description tags ... *\
*
*	extern struct WBStartup *WBenchMsg;
*
*	void main(int argc, char **argv)
*	{
*	    HandleApp(argc, argv, WBenchMsg, tags);
*	}
*
*   INPUTS
*	argc	- Number of Shell arguments being passed.
*
*	argv	- Pointer to the Shell argument list
*
*	wbm	- Pointer to the Workbench startup message.
*
*	attrs	- Pointer to the application's user interface
*		  description.
*
*   RESULT
*	TRUE	- Application was able to initialize and run.
*
*	FALSE	- Application failed.  AppShell has already informed
*		  user of what and where the failure was.
*
*   SEE ALSO
*	HandleAppAsync()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL AddIntMsgHandlerA (struct AppInfo *, struct MsgHandler *, struct TagItem *);
struct MsgHandler *setup_apsh_sipcA (struct AppInfo *, struct TagItem *);

BOOL HandleApp (int argc, char **argv, struct WBStartup * wbm,
		 struct TagItem * anchor)
{
    struct AppInfo *ai = NULL;
    struct TagItem *tags;
    BOOL error = FALSE;
    STRPTR template = NULL;

    DT (kprintf ("AppShellBase 0x%lx Tags 0x%lx\n", AppShellBase, anchor));

/**************************************************************************/
/*                             Initialization                             */
/**************************************************************************/

	/* restore the tag pointer */
	tags = anchor;

	/* initialize the function table & work areas */
	if (ai = (struct AppInfo *) NewAPSHObject(NULL, APSH_KIND_APPINFO, tags))
	{
	    struct TagItem *cur, *hnd;
	    struct MsgPort *sipc_die = NULL;
	    struct RDArgs TempRDA = {NULL};
	    STRPTR cmd;

	    /* clear the error return values */
	    ai->ai_Pri_Ret = RETURN_OK;
	    ai->ai_Sec_Ret = NULL;
	    ai->ai_TextRtn = NULL;

	    /* Get the passed arguements */
	    cmd = (STRPTR) GetTagData (APSH_CmdString, NULL, tags);

	    /* Get the Shell arguments */
	    argc = (int) GetTagData (APSH_NumArgs, (ULONG) argc, tags);
	    argv = (char **) GetTagData (APSH_ArgList, (ULONG) argv, tags);

	    /* Get the Workbench arguments */
	    if (wbm = (struct WBStartup *) GetTagData (APSH_WBStartup, (ULONG) wbm, tags))
	    {
		ai->ai_Startup |= APSHF_START_WB;
	    }

	    /* Get the cloned application control port */
	    if (sipc_die = (struct MsgPort *) GetTagData (APSH_ControlPort, NULL, tags))
	    {
		ai->ai_Startup |= APSHF_START_CLONE;
	    }

	    /* Get the ReadArgs information */
	    ai->ai_NumOpts = (LONG) GetTagData (APSH_NumOpts, NumOpts, tags);
	    ai->ai_Tempargc = (-1L);
	    if (template = (STRPTR) GetTagData (APSH_Template, (ULONG) Template, tags))
	    {
		LONG msize = strlen (template) + 2L;

		if (ai->ai_Template = (STRPTR) AllocVec (msize, MEMF_CLEAR))
		{
		    /* Copy the template */
		    strcpy (ai->ai_Template, template);

		    /* Parse the template */
		    ai->ai_Tempargc =
		      ParseLine (ai->ai_Template, ai->ai_Tempargv);
		}
	    }

	    /* do argument parsing stuff */
	    if ((ai->ai_Startup == NULL) || cmd)
	    {
		struct RDArgs *ra = NULL;

		if (cmd)
		{
		    ra = &(TempRDA); /* &(ai->ai_CmdRDA); */

		    /* Fill in the command buffer */
		    sprintf (ai->ai_TempText, "%s\n", cmd);
		    DR (kprintf("APSH_CmdString, [%s]\n", cmd));
		    ra->RDA_Source.CS_Buffer = ai->ai_TempText;
		    ra->RDA_Source.CS_Length = strlen (ai->ai_TempText);
		    ra->RDA_Source.CS_CurChr = 0;
		}

		/* Parse the startup arguments */
		ai->ai_ArgsPtr = ReadArgs (template, ai->ai_Options, ra);

		if (ai->ai_ArgsPtr == NULL)
		{
		    /* wasn't able to parse, display why */
		    DR (kprintf("ReadArgs failure %ld [%s] %ld\n",
			IoErr(), template, ai->ai_NumOpts));
		    PrintFault (IoErr (), "ReadArgs");
		    error = TRUE;
		}
		else
		{
		    LONG files;

		    /* See if we have Files/M */
		    DR (kprintf("Success [%s]\n", ai->ai_Options[0]));
		    if ((files = WhichOption (ai, "Files/M")) >= 0L)
		    {
			/* Were any files specified? */
			if (ai->ai_Options[files])
			{
			    /* Get the MultiArg array */
			    ai->ai_FileArray = (STRPTR *)
			      ai->ai_Options[files];
			}
		    }
		}

		/* Make sure we catch the ^C */
		if ((SIGBREAKF_CTRL_C & CheckSignal (SIGBREAKF_CTRL_C)))
		{
		    error = TRUE;
		}

	    }			/* end of Shell argument parsing */

	    DB (kprintf ("HA: #1 %ld\n", (LONG) error));

	    /* initialization block #2 */
	    if (!error)
	    {
		/* get the application information */
		ai->ai_AppVersion = (STRPTR) GetTagData (APSH_AppVersion, NULL, tags);
		ai->ai_AppCopyright = (STRPTR) GetTagData (APSH_AppCopyright, NULL, tags);
		ai->ai_AppAuthor = (STRPTR) GetTagData (APSH_AppAuthor, NULL, tags);
		ai->ai_AppMsgTitle = (STRPTR) GetTagData (APSH_AppMsgTitle, (ULONG) ai->ai_AppName, tags);

		/* Setup Localization */
		ai->ai_ALC = OpenLocaleEnv(tags);
		if (ai->ai_ALC)
		{

			/* point to the default application appstrings */
			ai->ai_ALC->alc_AppDef = (struct AppString **) GetTagData(APSH_DefText, NULL, tags);

			/* point to the default appshell appstrings */
			ai->ai_ALC->alc_ApshDef = (struct AppString **) AppStrings;

		}
		else
		{
			/* run screaming */
			error = TRUE;
		}

		/* set up a custom exit routine */
		ai->ai_Funcs[APSH_CUSTOM_EXIT] = GetTagData (APSH_AppExit, NULL, tags);

		/* set up break signal routines */
		ai->ai_Funcs[APSH_SIGC] = GetTagData (APSH_SIG_C, NULL, tags);
		ai->ai_SigBits |= SIGBREAKF_CTRL_C;

		if (ai->ai_Funcs[APSH_SIGD] = GetTagData (APSH_SIG_D, NULL, tags))
		    ai->ai_SigBits |= SIGBREAKF_CTRL_D;
		if (ai->ai_Funcs[APSH_SIGE] = GetTagData (APSH_SIG_E, NULL, tags))
		    ai->ai_SigBits |= SIGBREAKF_CTRL_E;
		if (ai->ai_Funcs[APSH_SIGF] = GetTagData (APSH_SIG_F, NULL, tags))
		    ai->ai_SigBits |= SIGBREAKF_CTRL_F;

		/* open the shared system libraries */
		if ((hnd = (struct TagItem *) GetTagData (APSH_OpenLibraries, NULL, tags)) &&
		    !OpenLibraries (ai, hnd))
		{
		    error = TRUE;
		}

		DB (kprintf ("OpenLibraries %ld\n", (ULONG) error));
	    }

	    /* initialization block #3 */
	    if (!error)
	    {
		/* Get the base information */
		error = GetBaseInfo (ai, argc, argv, wbm);

		/* add the handler functions */
		AddFuncEntries (ai, handler_funcs);
	    }

	    /* Always add the preference message handler */
	    AddIntMsgHandlerA (ai, setup_prefA (ai, NULL), NULL);

	    /* Always add the tool message handler */
	    AddIntMsgHandlerA (ai, setup_toolA (ai, NULL), NULL);

	    /* initialization block #4 */
	    cur = tags;
	    while (!error && cur)
	    {
		/* get the tag data */
		hnd = (struct TagItem *) cur->ti_Data;

		DB (kprintf ("tags: 0x%lx; Tag 0x%lx, Data 0x%lx\n",
			     cur, cur->ti_Tag, cur->ti_Data));

		/* process tags */
		switch (cur->ti_Tag)
		{
		    case APSH_AddARexx_UI:	/* ARexx UI */
			if (!AddIntMsgHandlerA (ai, setup_arexxA (ai, hnd), hnd))
			    error = TRUE;
			DB (kprintf ("AddARexx: %ld %ld\n", hnd, (ULONG) error));
			break;

		    case APSH_AddCmdShell_UI:	/* Command Shell */
			if (!AddIntMsgHandlerA (ai, setup_dosA (ai, hnd), hnd))
			    error = TRUE;
			DB (kprintf ("AddDOS: %ld %ld\n", hnd, (ULONG) error));
			break;

		    case APSH_AddSIPC_UI:
			if (!AddIntMsgHandlerA (ai, setup_sipcA (ai, hnd), hnd))
			    error = TRUE;
			DB (kprintf ("AddSIPC: %ld %ld\n", hnd, (ULONG) error));
			break;

		    //case APSH_AddTool_UI:
			//if (!AddIntMsgHandlerA (ai, setup_toolA (ai, hnd), hnd))
			//    error = TRUE;
			//DB (kprintf ("AddTool: %ld %ld\n", hnd, (ULONG) error));
			//break;

		    case APSH_AddClone_UI:
			if (!AddIntMsgHandlerA (ai, setup_apsh_sipcA (ai, hnd), hnd))
			    error = TRUE;
			DB (kprintf ("AddClone: %ld %ld\n", hnd, (ULONG) error));
			break;

		    case APSH_AddIntui_UI:
			/* Check to see if we use a GUI */
			{
			    BOOL add = TRUE;
			    LONG nogui;

			    /* Is the NOGUI switch used? */
			    if ((nogui = WhichOption (ai, "nogui/s")) >= 0)
			    {
				/* Is the NOGUI switch set? */
				if (ai->ai_Options[nogui])
				{
				    add = FALSE;
				}
			    }

			    /* Should we add the GUI? */
			    if (add)
			    {
				/* Add the Intuition message handler */
				if (!AddIntMsgHandlerA (ai, setup_idcmpA (ai, hnd), hnd))
				{
				    error = TRUE;
				}
			    }
			    else
			    {
				/* Add the functions so nothing crazy happens */
				AddFuncEntries (ai, idcmp_funcs);
			    }
			}
			break;

		    case APSH_AddWB_UI:
			if (!AddIntMsgHandlerA (ai, setup_wbA (ai, hnd), hnd))
			    error = TRUE;
			DB (kprintf ("AddWB: %ld %ld\n", hnd, (ULONG) error));
			break;

		    case APSH_AddHandler:	/* add a message handler */
			DB (kprintf ("AddHandler: B %ld %ld\n", hnd, (ULONG) error));
			if (!AddMsgHandlerA (ai, hnd))
			    error = TRUE;
			DB (kprintf ("AddHandler: A %ld %ld\n", hnd, (ULONG) error));
			break;

		    case APSH_AppInit:	/* got a custom init */
			DB (kprintf ("appinit: B %ld %ld\n", hnd, (ULONG) error));
			DT (kprintf ("AppInit anchor 0x%lx\n", anchor));
			PerfFunc (ai, hnd, NULL, anchor);
			error = ai->ai_Pri_Ret;
			DB (kprintf ("appinit: A %ld %ld\n", hnd, (ULONG) error));
			break;

		    default:	/* unknown initializer */
			DB (kprintf ("default: 0x%lx, %ld\n", cur, hnd, (ULONG) error));
			break;
		}

		/* get the next tag item */
		DB (kprintf ("next: %lx\n", &tags));
		cur = NextTagItem (&tags);
	    }			/* done adding handlers */

	    DB (kprintf ("Done processing %ld\n", (ULONG) error));

	    if (error)
	    {
		NotifyUser (ai, NULL, NULL);
	    }
	    else if (sipc_die)	/* need to add the TOOL SIPC handler */
	    {
		struct TagItem tsipc[3];

		tsipc[0].ti_Tag = APSH_Setup;
		tsipc[0].ti_Data = (ULONG) setup_apsh_sipcA;
		tsipc[1].ti_Tag = APSH_Port;
		tsipc[1].ti_Data = (ULONG) sipc_die;	/* port */
		tsipc[2].ti_Tag = TAG_DONE;
		tsipc[2].ti_Data = NULL;

		if (!AddMsgHandlerA (ai, tsipc))
		{
		    NotifyUser (ai, NULL, NULL);
		    error = TRUE;
		}
	    }

	    DB (kprintf ("ProcessMsgs %ld\n", (ULONG) error));

/**************************************************************************/
/*                               Main Loop                                */
/**************************************************************************/

#if 1
	    if (AmigaGuideBase)
	    {
		AddIntMsgHandlerA (ai, setup_hyperA (ai, NULL), NULL);
	    }
#endif

	    if (!error)
	    {
		struct MsgPort *parent = NULL;
		struct MsgHandler *mh = NULL;
		struct TagItem *act;
		LONG start;

		/* Restore the tag pointer */
		tags = anchor;

		/* See if we have Startup/K */
		if ((start = WhichOption (ai, "startup/k")) >= 0L)
		{
		    /* Were a startup script was specified? */
		    if (ai->ai_Options[start])
		    {
			/* Perform it! */
			PerfFunc (ai, NULL, (STRPTR)ai->ai_Options[start], NULL);
		    }
		}

		/* See if we're a tool */
		if (sipc_die &&
		    (act = FindTagItem (APSH_PortAddr, tags)) &&
		    (parent = (struct MsgPort *) act->ti_Data))
		{
		    /* See if we have a SIPC port */
		    if (mh = HandlerData (ai, APSH_Handler, "SIPC", TAG_DONE))
		    {
			/* Send them our SIPC address */
			act->ti_Data = (LONG) mh->mh_Port;

			/* To call a function with tags */
			HandlerFunc (ai,
				     APSH_Handler, "SIPC",
				     APSH_Command, AH_SENDCMD,
				     APSH_PortAddr, (ULONG) parent,
				     APSH_CmdID, ActiveToolID,
				     APSH_CmdData, (ULONG) tags,
				     TAG_DONE);
		    }
		}

		/* run the application */
		ai = ProcessMsgs (ai);
	    }

/**************************************************************************/
/*                                Shutdown                                */
/**************************************************************************/

	    /* Restore the tag pointer */
	    tags = anchor;

	    /* perform custom exit routine */
	    DB (kprintf ("custom exit\n"));
	    if (ai->ai_Funcs[APSH_CUSTOM_EXIT])
		PerfFunc (ai, ai->ai_Funcs[APSH_CUSTOM_EXIT], NULL, anchor);

	    /* close down shop */
	    DB (kprintf ("DelMsgHandlers 0x%lx\n", ai));
	    DelMsgHandlers (ai);

	    /* close the locale stuff */
	    DB (kprintf ("Closing Locale\n"));
	    CloseLocaleEnv(ai->ai_ALC);

	    /* close the shared system libraries */
	    DB (kprintf ("CloseLibraries\n"));
	    hnd = (struct TagItem *) GetTagData (APSH_OpenLibraries, NULL, tags);
	    if (hnd)
	    {
		CloseLibraries (ai, hnd);
	    }

	    /* close the ReadArgs stuff */
	    DB (kprintf ("FreeArgs 0x%lx\n", ai->ai_ArgsPtr));
	    if (ai->ai_ArgsPtr)
	    {
		FreeArgs (ai->ai_ArgsPtr);
		ai->ai_ArgsPtr = NULL;
	    }

	    /* close down the support data */
	    DisposeAPSHObject(ai, ai);
	}

    /* all done folks */
    return (error);
}

/****** appshell.library/HandleAppAsync ************************************
*
*   NAME
*	HandleAppAsync - startup function for an asynchronous AppShell
*	application.
*
*   SYNOPSIS
*	results = HandleAppAsync (attrs, sipc)
*	D0			  D0	 D1
*
*	BOOL results;
*	struct TagItem *attrs;
*	struct MsgPort *sipc;
*
*   FUNCTION
*	This function is the application entry point for an asynchronous
*	AppShell application.  This function should be called via the
*	Tool Message Handler and allows an AppShell application to have
*	multiple projects each with their own process.  Note that
*	directly calling this function will not cause an asynchronous
*	process to be launched.
*
*   EXAMPLE
*
*	\* This example shows how an application can clone itself to handle
*	 * multiple projects.
*	 *
*	 * APSH_Tool	  the name to give to the cloned process.
*	 * APSH_ToolData  the user interface tag description.
*	 *\
*	VOID CloneFunc(struct AppInfo *ai, STRPTR args, struct TagItem *tl)
*	{
*	    HandlerFunc (ai,
*			 APSH_Handler,   "TOOL",
*			 APSH_Command,   APSH_MH_OPEN,
*			 APSH_Tool,      "AppShell_Clone",
*			 APSH_ToolData,  Cloned_App,
*			 TAG_DONE);
*	}
*
*   INPUTS
*	attrs	- Pointer to the application's user interface
*		  description.
*
*	sipc	- SIPC message port by which the master application
*		  can control the cloned application.
*
*   RESULT
*	TRUE	- Application was able to initialize and run.
*
*	FALSE	- Application failed.  AppShell has already informed
*		  user of what and where the failure was.
*
*   SEE ALSO
*	HandleApp()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL HandleAppAsync (struct TagItem * anchor, struct MsgPort * sipc)
{
    BOOL retval = FALSE;
    struct TagItem tg[3];
    struct TagItem *clone;

    DT (kprintf ("HandleAppAsync 0x%lx, anchor 0x%lx\n", HandleAppAsync, anchor));

    /* The args are passed thru in the tag list */
    /* need to have an example showing how to do that! */

    /* insert the SIPC port address into the tag list */
    tg[0].ti_Tag = APSH_ControlPort;
    tg[0].ti_Data = (ULONG) sipc;
    tg[1].ti_Tag = TAG_MORE;
    tg[1].ti_Data = (ULONG) anchor;
    tg[2].ti_Data = TAG_DONE;

    /* Clone the list */
    if (clone = CloneTagItems (tg))
    {
	/* Call HandleApp */
	retval = HandleApp (0, NULL, NULL, clone);

	/* Free the cloned tags */
	FreeTagItems (clone);
    }

    return (retval);
}

/* handle messages between function handlers */
BOOL HandlerFunc (struct AppInfo * ai, ULONG tags,...)
{
    return (HandlerFuncA (ai, (struct TagItem *) & tags));
}

/* get handler data */
APTR HandlerData (struct AppInfo * ai, ULONG tags,...)
{

    return (HandlerDataA (ai, (struct TagItem *) & tags));
}

/****** appshell.library/HandlerFuncA **************************************
*
*   NAME
*	HandlerFuncA - entry point for a low-level message handler function.
*
*   SYNOPSIS
*	success = HandlerFuncA (ai, attrs)
*	D0			D0  D1
*
*	BOOL success;
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*   FUNCTION
*	Provides an entry point for the low-level message handler functions.
*
*   EXAMPLE
*
*	\* sample stub function to show how to call a low-level message
*	 * handler function.
*	 *
*	 * This example tells the IDCMP message handler to open the window
*	 * (and it's environment) named MAIN.
*	 *\
*	VOID StubFunc(struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    HandlerFunc(ai,
*			APSH_Handler, "IDCMP",
*			APSH_Command, APSH_MH_OPEN,
*			APSH_NameTag, "MAIN",
*			TAG_DONE);
*	}
*
*   INPUTS
*	ai	- pointer to the AppInfo structure for this application.
*	tags	- stack based TagItems.
*
*   SEE ALSO
*	HandlerDataA()
*
**********************************************************************
*
* Created:  ??-???-90, David N. Junod
*
*/

BOOL HandlerFuncA (struct AppInfo * ai, struct TagItem * tl)
{

    STRPTR name = (STRPTR) GetTagData (APSH_Handler, NULL, tl);
    WORD function = (WORD) GetTagData (APSH_Command, NULL, tl);
    BOOL retval = FALSE;
    struct MsgHandler *mh;

    DF (kprintf("HandlerFuncA enter\n"));

    if (mh = (struct MsgHandler *) FindNameI (&(ai->ai_MsgList), name))
    {
	/* call the handler function */
	if (mh->mh_Func[function])
	{
	    DF (kprintf("calling %s function %ld\n", name, (LONG)function));
	    retval = (*mh->mh_Func[function]) (ai, mh, tl);
	}
    }

    DF (kprintf("HandlerFuncA exit\n"));

    return (retval);
}

/****** appshell.library/HandlerDataA **************************************
*
*   NAME
*	HandlerDataA - Obtain a pointer to a message handlers' instance data
*
*   SYNOPSIS
*	mh = HandlerDataA (ai, attrs)
*	D0		   D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*   FUNCTION
*	Used to obtain a pointer to a message handlers' instance data.
*
*   EXAMPLE
*
*	\* sample stub function to show how to obtain a message handlers'
*	 * data
*	 *\
*	VOID StubFunc(struct AppInfo *ai, STRPTR cmd, struct TagItem *tl)
*	{
*	    struct MsgHandler *mh;
*
*	    \* get a pointer to the message handler data *\
*	    if (mh = HandlerData(ai,APSH_Handler,"MYH",TAG_DONE))
*	    {
*		struct MHObject *mho = &(mh->mh_Header);
*		struct myhInfo *md = mho->mho_SysData;
*
*		\* do something with the data now... *\
*	    }
*	}
*
*   INPUTS
*	ai	- pointer to the AppInfo structure for this application.
*	tags	- stack based TagItems.
*
*   RESULTS
*	mh	- Pointer to a MsgHandler structure.
*
*   SEE ALSO
*	HandlerFuncA()
*
**********************************************************************
*
* Created:  ??-???-90, David N. Junod
*
*/

APTR HandlerDataA (struct AppInfo * ai, struct TagItem * tl)
{

    APTR retval = NULL;
    register STRPTR name;

    if (name = (STRPTR) GetTagData (APSH_Handler, NULL, tl))
    {
	retval = (APTR) FindNameI (&(ai->ai_MsgList), name);
    }
#ifdef MULTI_DATA
    /* eventually will traverse the list to return any object node */
    else if (name = (STRPTR) GetTagData (APSH_NameTag, NULL, tl))
    {
	register struct List *list, *nxtlist;
	register struct MHObject *mho;

	list = &(ai->ai_MsgList);
	mho = (struct MHObject *) FindNameI (list, name);
    }
#endif

    return (retval);
}

/****** appshell.library/LockAppInfo ***************************************
*
*   NAME
*	LockAppInfo - Lock an application context for exclusive access.
*
*   SYNOPSIS
*	key = LockAppInfo ( ai );
*	D0		    A1
*
*	LONG key;
*	struct AppInfo *ai;
*
*   FUNCTION
*	This function locks an application context while the application
*	manipulates the AppInfo structure.
*
*   EXAMPLE
*
*	LONG key;
*
*	\* Lock the AppInfo structure for exclusive access *\
*	key = LockAppInfo (ai);
*
*	\* Modify some part of the AppInfo structure *\
*
*	\* Unlock the AppInfo structure *\
*	UnlockAppInfo (key);
*
*   INPUTS
*	ai	- Pointer to a valid AppInfo structure.
*
*   RETURNS
*	key	- Key to be passed to UnlockAppInfo to unlock the
*		  application context.
*
*   SEE ALSO
*	UnlockAppInfo()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

LONG __asm LockAppInfo (register __a1 struct AppInfo *ai)
{
    struct SignalSemaphore *ss = NULL;

    if (ai)
    {
	/* get a pointer to a semaphore */
	ss = &(ai->ai_Lock);

	/* obtain the semaphore */
	ObtainSemaphore (ss);
    }

    /* convert the semaphore to a long */
    return ((LONG) ss);
}

/****** appshell.library/UnlockAppInfo *************************************
*
*   NAME
*	UnlockAppInfo - Unlock a locked application context.
*
*   SYNOPSIS
*	UnlockAppInfo ( key );
*			D0
*
*	LONG key;
*
*   FUNCTION
*	This functions unlocks a locked application context.
*
*   INPUTS
*	key	- Key returned by LockAppInfo.
*
*   SEE ALSO
*	LockAppInfo()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

VOID __asm UnlockAppInfo (register __d0 LONG key)
{

    /* make sure a key was passed */
    if (key)
    {
	struct SignalSemaphore *ss = (struct SignalSemaphore *) key;

	/* release the lock */
	ReleaseSemaphore (ss);
    }
}
