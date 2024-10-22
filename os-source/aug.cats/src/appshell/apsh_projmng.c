/* apsh_projmng.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * project management routines
 *
 * $Id: apsh_projmng.c,v 1.4 1992/09/07 17:58:34 johnw Exp johnw $
 *
 * $Log: apsh_projmng.c,v $
 * Revision 1.4  1992/09/07  17:58:34  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:53:30  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:37:25  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

void kprintf (void *,...);

#define	DB(x)	;
#define	DA(x)	;
#define	DN(x)	;
#define	DS(x)	;
#define	DE(x)	;

BPTR GetPrefsDrawer (STRPTR basename, ULONG flags);

/****** appshell.library/ExpandPattern *************************************
*
*    NAME
*	ExpandPattern - Expand pattern into files.
*
*    SYNOPSIS
*	success = ExpandPattern (ai, cmd, attrs)
*	D0			 A1  A2   A3
*
*	struct AppInfo *ai;
*	STRPTR cmd;
*	struct TagItem *attrs;
*
*    FUNCTION
*	Expands a pattern into a list of file names, the names are added
*	to a project list.
*
*	Valid TagItems are:
*
*	    APSH_ArgList, <list>
*	    NULL terminated argument list, as returned by ReadArgs for a
*	    /M argument.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	cmd	- Pointer to an optional command string.
*	attrs	- Pointer to the TagItem array.
*
*    RESULT
*	success	- TRUE to indicate success, FALSE for failure.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

/* This function will be moving into the AppShell... */
BOOL __asm ExpandPattern (
			      register __a1 struct AppInfo * ai,
			      register __a2 STRPTR cmd,
			      register __a3 struct TagItem * tl)
{
    BOOL success = FALSE;
    STRPTR *sptr;
    UBYTE path[512];
    UBYTE node[128];

    /* Get the name array */
    if (sptr = (STRPTR *)
	GetTagData (APSH_ArgList, (ULONG) ai->ai_FileArray, tl))
    {
	struct AnchorPath *ap;

	/* Allocate an anchor path, must be longword aligned */
	if (ap = (struct AnchorPath *)
	    AllocVec (sizeof (struct AnchorPath), (MEMF_CLEAR | MEMF_PUBLIC)))
	{
	    LONG retval = RETURN_ERROR;

	    while (*sptr)
	    {
		for (retval = MatchFirst (*sptr, ap);
		     retval == 0L;
		     retval = MatchNext (ap))
		{
		    if (ap->ap_Info.fib_DirEntryType > 0)
		    {
/* DON'T go into directories!!! */
#if 0
			/* it's a directory */
			if (!(ap->ap_Flags & APF_DIDDIR))
			    ap->ap_Flags |= APF_DODIR;

			/* clear the completed directory flag */
			ap->ap_Flags &= ~APF_DIDDIR;
#endif
		    }
		    else
		    {
			BPTR lock;

			/* Parse the name */
			stcgfp (path, ap->ap_Info.fib_FileName);
			stcgfn (node, ap->ap_Info.fib_FileName);

			/* Get a lock on the file */
			if (lock = Lock (path, ACCESS_READ))
			{
			    struct WBArg wbarg;

			    wbarg.wa_Lock = lock;
			    wbarg.wa_Name = node;

			    /* Add the project */
			    AddProjects (ai, 1L, &wbarg, NULL);

			    UnLock (lock);
			}
		    }
		}

		/* end of pattern matching session */
		MatchEnd (ap);

		/* validate retval */
		if (retval != ERROR_NO_MORE_ENTRIES)
		{
		    /* Object node found */
		    if (retval == 205)
		    {
			/* For now, just add it */
			NewProject (ai, *sptr, NULL);
		    }
		    else
		    {
			PrintFault (retval, *sptr);
		    }
		}

		/* next name/pattern */
		sptr++;
	    }

	    success = TRUE;

	    /* Deallocate the work area */
	    FreeVec ((APTR) ap);
	}
    }

    DB (kprintf ("ExpandPattern exit %ld\n", (LONG) success));
    return (success);
}

BOOL GetBaseInfo (struct AppInfo * ai, int argc, char **argv, struct WBStartup * wbm)
{
    struct Process *proc = (struct Process *) FindTask (NULL);
    BOOL retval = TRUE;
    LONG error, len;
    STRPTR name;

    DA (kprintf ("GetBaseInfo           0x%lx\n", GetBaseInfo));

    /* Get a lock on the program directory */
    ai->ai_ProgDir = NULL;
    if (proc->pr_HomeDir)
    {
	ai->ai_ProgDir = DupLock (proc->pr_HomeDir);
    }

    if (argc > 0)
    {
	/* The application was called from the Shell */
	len = strlen (argv[0]) + 2L;

	if (ai->ai_ProgName = AllocVec (len, MEMF_CLEAR))
	{
	    /* get the program name */
	    name = FilePart (argv[0]);
	    strcpy (ai->ai_ProgName, name);
	}

	/* Expand the Files/M */
	if (ai->ai_FileArray)
	{
	    ExpandPattern (ai, NULL, NULL);
	}
    }
    else if (wbm)
    {
	/* The application was called from Workbench */

	struct WBArg *wbarg = &(wbm->sm_ArgList[0]);

	/* Get the length of the name */
	len = strlen (wbarg->wa_Name) + 2L;

	/* Allocate room for the name */
	if (ai->ai_ProgName = AllocVec (len, MEMF_CLEAR))
	    strcpy (ai->ai_ProgName, wbarg->wa_Name);

	/* handle the Workbench startup */
	if (IconBase)
	{
	    AddProjects (ai, (wbm->sm_NumArgs - 1L), &(wbm->sm_ArgList[1]), NULL);
	}
    }
    else
    {
	struct Task *task = (struct Task *) & (proc->pr_Task);

	/* Get the name */
	name = task->tc_Node.ln_Name;

	DN (kprintf ("name [%s]\n", name));

	/* The application is a clone */
	len = strlen (name) + 2L;

	if (ai->ai_ProgName = AllocVec (len, MEMF_CLEAR))
	{
	    /* get the program name */
	    strcpy (ai->ai_ProgName, name);
	}
    }

    /* Get the configuration directory */
    if (ai->ai_ProgName)
    {
	/* indicate that everything is kosher */
	retval = FALSE;

	/* Get a lock on the configuration drawer */
	ai->ai_ConfigDir = GetPrefsDrawer (ai->ai_BaseName, NULL);

	/* See if any errors occurred */
	error = IoErr ();
	if (error > 0L)
	{
	    switch (error)
	    {
		case ERROR_DISK_WRITE_PROTECTED:
		    ai->ai_Flags |= APSHF_RO;
		    break;

		case ERROR_DIRECTORY_NOT_EMPTY:
		    break;

		default:
		    /* the directory doesn't already exist, so it's an error */
		    Fault (error, NULL, ai->ai_WorkText, 80);

		    ai->ai_Pri_Ret = RETURN_FAIL;
		    ai->ai_Sec_Ret = APSH_IOERR;
		    ai->ai_TextRtn = (STRPTR) & ai->ai_WorkText;

		    retval = TRUE;
		    break;
	    }
	}

	/* get the icon for our application */
	if (ai->ai_ProgDir && (retval == FALSE) && IconBase)
	{
	    BPTR tmp;

	    /* Go into the program directory */
	    if (tmp = CurrentDir (ai->ai_ProgDir))
	    {
		/* Try getting the tools icon */
		if (!(ai->ai_ProgDO = GetDiskObject (ai->ai_ProgName)))
		{
		    /* Use the default tool icon */
		    ai->ai_ProgDO = GetDefDiskObject (WBTOOL);
		}

		/* Return back home */
		CurrentDir (tmp);
	    }
	}
    }
    else
    {
	DB (kprintf ("No name, no lock\n"));
    }

    return (retval);
}

/****** appshell.library/NewProject ****************************************
*
*    NAME
*	NewProject - Create a project node and add it to the project list.
*
*    SYNOPSIS
*	proj = NewProject (ai, name, attrs)
*	D0		   D0  D1    A0
*
*	struct ProjNode *proj;
*	struct AppInfo *ai;
*	STRPTR name;
*	struct TagItem *attrs;
*
*    FUNCTION
*	Creates a project node of the given name and adds it to the requested
*	project list.  The project list defaults to the main project list.
*	If the name is NULL, the function will determine the name by finding
*	the next available Unnamed slot.
*
*	Valid TagItems are:
*
*	    APSH_ProjInfo, <project>
*		Where <project> is a valid pointer to a Project structure.
*		Defaults to the AppInfo project structure.
*
*	    APSH_NameTag, <name>
*		Where <name> is the name to be added to the project list.
*
*	    APSH_TextID, <text ID>
*		Where <text ID> is the text ID from the applications text
*		table to use for the prefix when a name isn't given.  Defaults
*		to Unnamed from the AppShell text table.  The resulting
*		prefix will be truncated to twenty characters.
*
*	    APSH_BaseID, <base ID>
*		Where <base ID> is the text table to use when APSH_TextID is
*		supplied.  Defaults to APSH_USER_ID.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	name	- Pointer to the name to give to the project.
*	attrs	- Pointer to an optional TagItem array.
*
*    RESULT
*	proj	- Pointer to a filled in ProjNode structure
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

struct ProjNode *NewProject (struct AppInfo * ai, STRPTR anchor,
			      struct TagItem * tl)
{
    STRPTR name = anchor;
    struct ProjNode *pn = NULL;
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    DA (kprintf ("NewProject            0x%lx\n", NewProject));

    /* make sure we have a pointer at least */
    if (pn = (struct ProjNode *) AllocVec (sizeof (struct ProjNode), MEMF_CLEAR))
    {
	struct TagItem *ti;
	struct Project *pi;
	struct List *list;
	ULONG baseID = APSH_MAIN_ID;
	ULONG textID = APSH_UNNAMED;
	UBYTE prefix[32], pname[32];
	WORD i;

	/* default to the AppInfo Project structure */
	pi = &(ai->ai_Project);

	/* see if there was a tag for a project list */
	if (tl)
	{
	    pi = (struct Project *) GetTagData (APSH_ProjInfo, (ULONG) pi, tl);
	    name = (STRPTR) GetTagData (APSH_NameTag, (ULONG) name, tl);
	}

	/* initialize the list pointer */
	list = &(pi->p_ProjList);

	if (name)
	{
	    /* they supplied a name */
	    strcpy (pn->pn_Name, name);
	}
	else
	{
	    /* get the tags describing the prefix */
	    if (ti = FindTagItem (APSH_TextID, tl))
	    {
		textID = ti->ti_Data;
		baseID = GetTagData (APSH_BaseID, APSH_MAIN_ID, tl);
	    }

	    /* get the prefix to use */
	    strncpy (prefix, GetText (ai, baseID, textID, NULL), 20);

	    /* we have to create a name */
	    for (i = 1; i < 99999; i++)
	    {
		/* create a name */
		sprintf (pname, "%s%ld", prefix, (LONG) i);

		if (!(FindNameI (list, pname)))
		    break;
	    }

	    /* copy the name */
	    strcpy (pn->pn_Name, pname);

	    /* CURRENTLY, use the default project icon */
	    pn->pn_DObj = GetDefDiskObject (WBPROJECT);
	}

	/* establish the node name */
	pn->pn_Node.ln_Name = pn->pn_Name;

	/* set the project ID */
	pn->pn_ID = pi->p_MaxID;

	/* add the project information to the list */
	AddTail (list, (struct Node *) pn);

	/* increment the project counter */
	pi->p_NumProjs++;

	/* increment the ID counter */
	pi->p_MaxID++;

	/* total refresh mode */
	pi->p_State = 3;
    }

    UnlockAppInfo (key);

    return (pn);
}

/****** appshell.library/RenumProject **************************************
*
*    NAME
*	RenumProject - Renumber the project list.
*
*    SYNOPSIS
*	RenumProject (ai, cmd, attrs)
*		      D0  D1    A0
*
*	struct AppInfo *ai;
*	STRPTR cmd;
*	struct TagItem *attrs;
*
*    FUNCTION
*	Renumbers the project list.
*
*	Valid TagItems are:
*
*	    APSH_ProjInfo, <project>
*		Where <project> is a valid pointer to a Project structure.
*		Defaults to the AppInfo project structure.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	cmd	- Pointer to the command string.
*	attrs	- Pointer to the TagItem array.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID RenumProjects (struct AppInfo * ai, STRPTR cmd, struct TagItem * tl)
{
    ULONG id;
    struct Node *node;
    struct ProjNode *pn;
    struct Project *pi;
    struct List *list;
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    /* default to the AppInfo Project structure */
    pi = &(ai->ai_Project);

    /* see if there was a tag for a project list */
    if (tl)
    {
	/* get the project list to work with */
	pi = (struct Project *) GetTagData (APSH_ProjInfo, (ULONG) pi, tl);
    }

    /* get a pointer to our list */
    list = &(pi->p_ProjList);

    /* renumber all the projects */
    id = 0;
    if (list->lh_TailPred != (struct Node *) list)
    {
	node = list->lh_Head;
	while (node->ln_Succ)
	{
	    /* cast to a Project node */
	    pn = (struct ProjNode *) node;

	    /* get the next node pointer */
	    node = node->ln_Succ;

	    /* renumber the project */
	    pn->pn_ID = id++;
	}			/* end of while node */
    }				/* end of if list */

    UnlockAppInfo (key);
}

/****** appshell.library/RemoveProject *************************************
*
*    NAME
*	RemoveProject - Remove a project node from the project list.
*
*    SYNOPSIS
*	success = RemoveProject (ai, name, attrs)
*	D0			 D0  D1    A0
*
*	BOOL success;
*	struct AppInfo *ai;
*	STRPTR name;
*	struct TagItem *attrs;
*
*    FUNCTION
*	Removes a project node of the given name from the requested project
*	list.  Defaults to removing the current project node from the main
*	project list.
*
*	Valid TagItems are:
*
*	    APSH_ProjInfo, <project>
*		Where <project> is a valid pointer to a Project structure.
*		Defaults to the AppInfo project structure.
*
*	    APSH_NameTag, <name>
*		Where <name> is a valid project name to be removed.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	name	- Pointer to the name of the project to be removed.
*	attrs	- Pointer to an optional TagItem array.
*
*    RESULT
*	TRUE	- Able to remove the project from the list.
*	FALSE	- Unable to remove the project.
*
*    BUGS
*	name is not currently implemented.  Just removes the current project.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL RemoveProject (struct AppInfo * ai, STRPTR cmd, struct TagItem * tl)
{
    BOOL retval = FALSE;
    struct ProjNode *pn;
    STRPTR name = cmd;
    struct Project *pi;
    struct List *list;
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    /* default to the AppInfo Project structure */
    pi = &(ai->ai_Project);

    /* see if there was a tag for a project list */
    if (tl)
    {
	/* get the project list to work with */
	pi = (struct Project *) GetTagData (APSH_ProjInfo, (ULONG) pi, tl);

	/* get the name to remove */
	name = (STRPTR) GetTagData (APSH_NameTag, (ULONG) name, tl);
    }

    /* get a pointer to our list */
    list = &(pi->p_ProjList);

    /* if there was a name provided then align to it */
    if (name)
    {
	/* find the named project node */
	pi->p_CurProj = (struct ProjNode *) FindNameI (list, name);
    }

    if ((pn = pi->p_CurProj) && (list->lh_TailPred != (struct Node *) list))
    {
	/* remove the project node */
	FreeProject (pn);

	/* decrement the project count */
	pi->p_NumProjs--;
	pi->p_CurLine--;
	pi->p_MaxID--;

	/* renumber the project list */
	RenumProjects (ai, NULL, tl);

	/* we deleted it, so set it to NULL */
	pi->p_CurProj = NULL;

	/* able to remove the project */
	retval = TRUE;
    }

    UnlockAppInfo (key);

    return (retval);
}

/****** appshell.library/AddProjects ***************************************
*
*    NAME
*	AddProjects - Add an array of WBArgs to the project list.
*
*    SYNOPSIS
*	result = AddProjects (ai, numargs, args, attrs)
*	D0		      D0  D1       A0    A1
*
*	BOOL result;
*	struct AppInfo *ai;
*	LONG numargs;
*	struct WBArg *args;
*	struct TagItem *attrs;
*
*    FUNCTION
*	Add an array of WBArgs to the given project list.  Defaults to the
*	main project list.
*
*	Valid TagItems are:
*
*	    APSH_ProjInfo,<project>
*		Where <project> is a valid pointer to a Project structure.
*		Defaults to the AppInfo project structure.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	numargs	- Number of arguments in the array.
*	args	- Pointer to an array of WBArg structures.
*	attrs	- Pointer to an optional TagItem array.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL AddProjects (struct AppInfo * ai, LONG numargs, struct WBArg * args, struct TagItem * tl)
{
    struct Project *pi = NULL;
    UBYTE name[512];
    struct List *list;
    struct WBArg *arg = args;
    struct ProjNode *pn;
    LONG i, cur = -1, msize;
    BOOL retval = TRUE;
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    /* default to the AppInfo Project structure */
    pi = &(ai->ai_Project);

    /* see if there was a tag for a project list */
    if (tl)
	pi = (struct Project *) GetTagData (APSH_ProjInfo, (ULONG) pi, tl);

    /* initialize the list pointer */
    list = &(pi->p_ProjList);

    for (i = 0L; i < numargs; i++)
    {
	/* build the complete name */
	NameFromLock (arg->wa_Lock, name, 512);
	AddPart (name, arg->wa_Name, 512);

	/* find the project node */
	pn = (struct ProjNode *) FindNameI (list, name);

	if (!pn)
	{
	    /* It isn't in the list, so add it */

	    /* set the current project to the first in the list */
	    if (cur == (-1))
		pi->p_CurLine = cur = pi->p_NumProjs;

	    /* figure out how much memory we need */
	    msize = sizeof (struct ProjNode);

	    /* allocate the project node */
	    if (pn = (struct ProjNode *) AllocVec (msize, MEMF_CLEAR))
	    {
		msize = strlen (name) + 2L;

		/* allocate room for the project name */
		if (pn->pn_ProjPath = (STRPTR) AllocVec (msize, MEMF_CLEAR))
		{
		    /* copy the name component */
		    strcpy (pn->pn_Name, arg->wa_Name);

		    /* copy the project name */
		    strcpy (pn->pn_ProjPath, name);

		    /* point the project name at the name portion of the path */
		    pn->pn_ProjName = &(pn->pn_ProjPath[(WORD) (strlen (name) - strlen (arg->wa_Name))]);

		    /* point the node to the complete name */
		    pn->pn_Node.ln_Name = pn->pn_ProjPath;

		    /* copy the project directory lock */
		    if (arg->wa_Lock)
			pn->pn_ProjDir = DupLock (arg->wa_Lock);

		    /* get the icon for the project */
		    pn->pn_DObj = IconFromWBArg (arg);

		    /* set the project ID */
		    pn->pn_ID = pi->p_MaxID;

		    /* add the project information to the list */
		    AddTail (list, (struct Node *) pn);

		    /* increment the project counter */
		    pi->p_NumProjs++;

		    /* increment the ID counter */
		    pi->p_MaxID++;
		}
		else
		{
		    FreeVec ((APTR) pn);
		    pn = NULL;
		    retval = FALSE;
		}
	    }
	    else
	    {
		/* could not allocate a project node */
		retval = FALSE;
	    }
	}			/* end of if could not find name */
	else
	{
	    /* it is already in the list */
	    if (cur == (-1))
		pi->p_CurLine = cur = pn->pn_ID;
	}

	/* get the next argument */
	arg++;
    }				/* end of argument loop */

    /* set the current display value */
    if (cur == (-1))
	cur = pi->p_CurLine;

    /* total refresh mode */
    pi->p_State = 3;

    UnlockAppInfo (key);

    return (retval);
}

/****** appshell.library/FreeProject ***************************************
*
*    NAME
*	FreeProject - Removes and deallocate a project from the project list.
*
*    SYNOPSIS
*	FreeProject ( proj )
*		      D0
*
*	struct ProjNode *proj;
*
*    FUNCTION
*	Removes and deallocates a project node from the project list.
*
*    INPUTS
*	proj	- Pointer to a valid ProjNode structure.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID FreeProject (struct ProjNode * pn)
{

    if (pn)
    {
	/* remove the node from the list */
	Remove ((struct Node *) pn);

	/* free the diskobject if there is one */
	DB (kprintf ("free diskobject 0x%lx\n", pn->pn_DObj));
	if (pn->pn_DObj)
	    FreeDiskObject (pn->pn_DObj);

	/* unlock the project's directory */
	DB (kprintf ("unlock 0x%lx\n", pn->pn_ProjDir));
	if (pn->pn_ProjDir)
	    UnLock (pn->pn_ProjDir);

	/* Free the project path */
	DB (kprintf ("free project path 0x%lx\n", pn->pn_ProjPath));
	if (pn->pn_ProjPath)
	    FreeVec (pn->pn_ProjPath);

	/* free the project comment */
	DB (kprintf ("free project comment 0x%lx\n", pn->pn_ProjComment));
	if (pn->pn_ProjComment)
	    FreeVec (pn->pn_ProjComment);

	/* free the project node */
	DB (kprintf ("free project node 0x%lx\n", pn));
	FreeVec (pn);
    }
    DB (kprintf ("freeproject exit\n"));
}

LONG __asm UpdateProject (
    register __a1 struct AppInfo * ai,
    register __a2 struct ProjNode * pn,
    register __a3 struct WBArg * arg)
{
    LONG retval = TRUE;
    UBYTE name[512];
    LONG msize;
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    if (pn && arg)
    {

/****************************/
/* Free the old information */
/****************************/

	/* Clear the name momentarily */
	pn->pn_Node.ln_Name = NULL;

	/* free the diskobject if there is one */
	if (pn->pn_DObj)
	{
	    FreeDiskObject (pn->pn_DObj);
	    pn->pn_DObj = NULL;
	}

	/* unlock the project's directory */
	if (pn->pn_ProjDir)
	{
	    UnLock (pn->pn_ProjDir);
	    pn->pn_ProjDir = NULL;
	}

	/* Free the project path */
	if (pn->pn_ProjPath)
	{
	    FreeVec (pn->pn_ProjPath);
	    pn->pn_ProjPath = NULL;
	}

	/* free the project comment */
	if (pn->pn_ProjComment)
	{
	    FreeVec (pn->pn_ProjComment);
	    pn->pn_ProjComment = NULL;
	}

/****************************/
/* Allocate new information */
/****************************/

	/* build the complete name */
	NameFromLock (arg->wa_Lock, name, 512);
	AddPart (name, arg->wa_Name, 512);

	msize = strlen (name) + 2L;

	/* allocate room for the project name */
	if (pn->pn_ProjPath = (STRPTR) AllocVec (msize, MEMF_CLEAR))
	{
	    /* copy the name component */
	    strcpy (pn->pn_Name, arg->wa_Name);

	    /* copy the project name */
	    strcpy (pn->pn_ProjPath, name);

	    /* point the project name at the name portion of the path */
	    pn->pn_ProjName = &(pn->pn_ProjPath[(WORD) (strlen (name) - strlen (arg->wa_Name))]);

	    /* point the node to the complete name */
	    pn->pn_Node.ln_Name = pn->pn_ProjPath;

	    /* copy the project directory lock */
	    if (arg->wa_Lock)
		pn->pn_ProjDir = DupLock (arg->wa_Lock);

	    /* get the icon for the project */
	    pn->pn_DObj = IconFromWBArg (arg);
	}
	else
	{
	    /* Remove the node from the list */
	    Remove ((struct Node *) pn);

	    FreeVec ((APTR) pn);
	    pn = NULL;
	    retval = FALSE;
	}

    }

    UnlockAppInfo (key);

    return (retval);
}

/****** appshell.library/FreeProjects **************************************
*
*    NAME
*	FreeProjects - Remove and deallocate all the projects in a project list.
*
*    SYNOPSIS
*	FreeProjects (ai, attrs)
*		      D0  D1
*
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*    FUNCTION
*	Removes and deallocate all the projects in a project list.  Defaults
*	to the main project list.
*
*	Valid TagItems are:
*
*	    APSH_ProjInfo,<project>
*		Where <project> is a valid pointer to a Project structure.
*		Defaults to the AppInfo project structure.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	attrs	- Pointer to an optional TagItem array.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID FreeProjects (struct AppInfo * ai, struct TagItem * tl)
{
    struct List *list;
    struct Node *node, *next;
    struct ProjNode *pn;
    struct Project *pi;
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    DB (kprintf ("FreeProjects enter\n"));
    /* default to the AppInfo Project structure */
    pi = &(ai->ai_Project);

    /* see if there was a tag for a project list */
    if (tl)
	pi = (struct Project *) GetTagData (APSH_ProjInfo, (ULONG) pi, tl);

    /* shutdown our project list and its information */
    list = &(pi->p_ProjList);
    if (list->lh_TailPred != (struct Node *) list)
    {
	node = list->lh_Head;
	while (next = node->ln_Succ)
	{
	    /* get a pointer to the project node */
	    pn = (struct ProjNode *) node;

	    /* free the project node */
	    FreeProject (pn);

	    /* get the next node */
	    node = next;
	}
    }
    DB (kprintf ("FreeProjects exit\n"));

    UnlockAppInfo (key);
}

static VOID InsertProjNode (struct List * list, struct ProjNode * node)
{
    struct ProjNode *cur_node = NULL;
    struct ProjNode *nxt_node = NULL;

    if (list->lh_TailPred == (struct Node *) list)
    {
	AddTail (list, (struct Node *) node);
    }
    else
    {
	cur_node = (struct ProjNode *) list->lh_Head;
	while (nxt_node = (struct ProjNode *) (cur_node->pn_Node.ln_Succ))
	{
	    if (node->pn_ID <= cur_node->pn_ID)
		break;
	    else
		cur_node = nxt_node;
	}
	Insert (list, (struct Node *) node,
		(struct Node *) cur_node->pn_Node.ln_Pred);
    }
}

/****** appshell.library/GetProjNode ***************************************
*
*    NAME
*	GetProjNode - Get the requested project node.
*
*    SYNOPSIS
*	proj = GetProjNode (ai, id, attrs)
*	D0		    D0  D1  A0
*
*	struct ProjNode *proj;
*	struct AppInfo *ai;
*	LONG id;
*	struct TagItem *attrs;
*
*    FUNCTION
*	Given a project ID, find the correct ProjNode.
*
*	Valid TagItems are:
*
*	    APSH_ProjInfo,<project>
*		Where <project> is a valid pointer to a Project structure.
*		Defaults to the AppInfo project structure.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	id	- Project ID to locate.
*	attrs	- Pointer to an optional TagItem array.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

struct ProjNode *GetProjNode (struct AppInfo * ai, LONG cnt, struct TagItem * tl)
{
    struct Node *node = NULL;
    struct Project *pi;
    struct List *list;
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    /* default to the AppInfo Project structure */
    pi = &(ai->ai_Project);

    /* see if there was a tag for a project list */
    if (tl)
	pi = (struct Project *) GetTagData (APSH_ProjInfo, (ULONG) pi, tl);

    /* shutdown our project list and its information */
    list = &(pi->p_ProjList);

    if (list && list->lh_TailPred != (struct Node *) list)
    {
	node = list->lh_Head;
	while (node->ln_Succ)
	{
	    if (((struct ProjNode *) node)->pn_ID == cnt)
	    {
		pi->p_CurProj = (struct ProjNode *) node;
		return ((struct ProjNode *) node);
	    }
	    node = node->ln_Succ;
	}
    }

    /* not found, so NULL the current entry */
    pi->p_CurProj = NULL;

    UnlockAppInfo (key);

    return (NULL);
}

/****** appshell.library/SwapProjNodes *************************************
*
*    NAME
*	SwapProjNodes - Swap two project nodes within the same Project list.
*
*    SYNOPSIS
*	SwapProjNodes (ai, pn1, pn2, attrs)
*		       D0  D1   A0   A1
*
*	struct AppInfo *ai;
*	struct ProjNode *pn1;
*	struct ProjNode *pn2;
*	struct TagItem *attrs;
*
*    FUNCTION
*	This function will swap two project nodes within the same project
*	list.
*
*	Valid TagItems are:
*
*	    APSH_ProjInfo,<project>
*		Where <project> is a valid pointer to a Project structure.
*		Defaults to the AppInfo project structure.
*
*    INPUTS
*	ai	- Pointer to the application AppInfo structure.
*	pn1	- Pointer to a valid ProjNode structure.
*	pn2	- Pointer to a valid ProjNode structure.
*	attrs	- Pointer to an optional TagItem array.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID SwapProjNodes (struct AppInfo * ai,
		     struct ProjNode * sn1, struct ProjNode * sn2,
		     struct TagItem * tl)
{
    LONG key;

    /* Lock the AppInfo structure */
    key = LockAppInfo (ai);

    /* make sure we have valid ProjNodes to switch */
    if ((sn1 != sn2) && sn1 && sn2)
    {
	LONG hold;
	struct Project *pi;

	/* default to the AppInfo Project structure */
	pi = &(ai->ai_Project);

	/* see if there was a tag for a project list */
	if (tl)
	    pi = (struct Project *) GetTagData (APSH_ProjInfo, (ULONG) pi, tl);

	/* remove the nodes from the list */
	Remove ((struct Node *) sn1);
	Remove ((struct Node *) sn2);

	/* swap their ID's */
	hold = sn1->pn_ID;
	sn1->pn_ID = sn2->pn_ID;
	sn2->pn_ID = hold;

	/* add them back in the proper order */
	InsertProjNode (&(pi->p_ProjList), sn1);
	InsertProjNode (&(pi->p_ProjList), sn2);
    }

    UnlockAppInfo (key);
}
