/* apsh_functable.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * function table maintainance
 *
 * $Id: apsh_functable.c,v 1.4 1992/09/07 17:54:40 johnw Exp johnw $
 *
 * $Log: apsh_functable.c,v $
 * Revision 1.4  1992/09/07  17:54:40  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:50:49  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:22:02  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

/****** appshell.library/AddFuncEntry **************************************
*
*   NAME
*	AddFuncEntry - Add an entry to the function table
*
*   SYNOPSIS
*	results = AddFuncEntry (ai, entry)
*	D0			D0  D1
*
*	BOOL results;
*	struct AppInfo *ai;
*	struct Funcs *entry;
*
*   FUNCTION
*	This function is used to add a new entry to the application's
*	function table.
*
*	The function table is maintained as an Exec linked list.  The
*	AppShell will free all entries when the application is shutdown.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	entry	- Pointer to a Funcs structure that describes the entry
*		  to be added to the function table.
*
*   RESULT
*	TRUE	- AppShell was able to add the function to the table.
*	FALSE	- Unable to add the entry to the function table.
*
*   SEE ALSO
*	AddFuncEntries(), FreeFuncEntry(), FreeFuncEntries()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL AddFuncEntry (struct AppInfo * ai, struct Funcs * sfe)
{
    extern VOID dispatchHook();
    ULONG mlen, msize, FuncID;
    struct FuncEntry *fe;
    BOOL retval = FALSE;

    /* only add it if we have a valid name */
    if (sfe->fe_Name && (mlen = strlen (sfe->fe_Name)))
    {
	/* compute space needed */
	msize = sizeof (struct FuncEntry) +
	  sizeof (struct FuncIDEntry) + mlen + 2L;

	/* allocate a function table entry node */
	if (fe = (struct FuncEntry *) AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC)))
	{
	    struct List *list;
	    struct FuncIDEntry *fie;

	    /* point the FuncIDEntry at the appropriate memory block */
	    fie = MEMORY_FOLLOWING (fe);

	    /* set up the function entry name */
	    fe->fe_Name = MEMORY_FOLLOWING (fie);
	    strcpy (fe->fe_Name, sfe->fe_Name);

	    /* set up the node information */
	    fe->fe_Node.ln_Name = fe->fe_Name;

	    /* set up the numeric portions of the function entry */
	    fe->fe_Func = sfe->fe_Func;
	    fe->fe_ID = sfe->fe_ID;
	    fe->fe_NumOpts = 0L;
	    fe->fe_HelpID = sfe->fe_HelpID;
	    fe->fe_Flags = sfe->fe_Flags;
	    fe->fe_GroupID = sfe->fe_GroupID;

	    /* check to see if there where any parameters provided */
	    if (sfe->fe_Params)
	    {
		/* compute size of memory block required */
		msize = strlen (sfe->fe_Params) + 2L;

		/* if we can allocate the memory, then clone the parameters */
		if (fe->fe_Params = (STRPTR) AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC)))
		{
#if 0
		    sprintf (fe->fe_Params, "%s %s", fe->fe_Name, sfe->fe_Params);
#else
		    strcpy (fe->fe_Params, sfe->fe_Params);
#endif
		}
	    }

	    /* check to see if a ReadArgs template was provided */
	    if (sfe->fe_Template)
	    {
		/* compute size of memory block required */
		msize = strlen (sfe->fe_Template) + 2L;

		/* if we can allocate the memory, then clone the parameters */
		if ((sfe->fe_NumOpts > 0L) &&
		    (fe->fe_Template = (STRPTR) AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC))))
		{
		    /* copy the template over */
		    strcpy (fe->fe_Template, sfe->fe_Template);

		    /* set the number of options */
		    fe->fe_NumOpts = sfe->fe_NumOpts;

		    /* compute how much memory we need for the option array */
		    msize = sizeof (LONG) * fe->fe_NumOpts;

		    /* allocate the options array */
		    if (!(fe->fe_Options = (LONG *) AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC))))
		    {
			/* get rid of the template */
			fe->fe_NumOpts = 0L;
			FreeVec ((APTR) fe->fe_Template);
			fe->fe_Template = NULL;
		    }
		}
	    }

	    /* increment the function table entry count */
	    FuncID = ai->ai_NumFuncs++;

	    /* if no function ID is assigned to entry, then it is an ALIAS */
	    if (fe->fe_ID == NO_FUNCTION)
	    {
		/* indicate that it is an ALIAS entry */
		fe->fe_Flags |= APSHF_FTE_ALIAS;

		/* save the ID */
		fe->fe_ID = FuncID;
	    }

	    if (sfe->fe_Flags & APSHF_SYSTEM)
	    {
		fe->fe_Entry = dispatchHook;
	    }

	    /* set the function ID variable */
	    FuncID = fe->fe_ID;

	    /* initialize the numeric list entry */
	    fie->fie_FE = fe;
	    sprintf (fie->fie_ID, "%lx", FuncID);
	    fie->fie_Node.ln_Name = fie->fie_ID;

	    /* add the entry to the name list */
	    list = &(ai->ai_FuncList);
	    AlphaEnqueue (list, (struct Node *) fe);

	    /* add the entry to the numeric list */
	    list = &(ai->ai_FuncList2);
	    Enqueue (list, (struct Node *) fie);

	    /* successfully added to the list */
	    retval = TRUE;
	}
    }
    return (retval);
}

/****** appshell.library/FreeFuncEntry *************************************
*
*   NAME
*	FreeFuncEntry - Free an entry from the function table
*
*   SYNOPSIS
*	FreeFuncEntry (ai, f_entry);
*		       D0  D1
*
*	struct AppInfo *ai;
*	struct FuncEntry *f_entry;
*
*   FUNCTION
*	This function is used to free an entry from the application's
*	function table.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*
*	f_entry	- Pointer to a FuncEntry structure that describes the
*		  entry to be added to the function table.
*
*   SEE ALSO
*	AddFuncEntry(), AddFuncEntries(), FreeFuncEntries()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID FreeFuncEntry (struct AppInfo * ai, struct FuncEntry * fe)
{
    struct FuncIDEntry *fie;

    if (fe)
    {
	/* get a pointer to the FuncIDEntry */
	fie = MEMORY_FOLLOWING (fe);

	/* remove the node from the list */
	Remove ((struct Node *) fie);
	Remove ((struct Node *) fe);

	/* free the parameter field, if we allocated it */
	if (fe->fe_Params)
	    FreeVec ((APTR) fe->fe_Params);

	/* free the options array */
	if (fe->fe_Options)
	    FreeVec ((APTR) fe->fe_Options);

	/* free the template field */
	if (fe->fe_Template)
	    FreeVec ((APTR) fe->fe_Template);

	/* free the entry */
	FreeVec ((APTR) fe);
    }
}

/****** appshell.library/AddFuncEntries ************************************
*
*   NAME
*	AddFuncEntries - Add an array of entries to the function table.
*
*   SYNOPSIS
*	results = AddFuncEntries (ai, entries)
*	D0			  D0  D1
*
*	BOOL results;
*	struct AppInfo *ai;
*	struct Funcs *entries;
*
*   FUNCTION
*	This function is used to add a NULL terminated array of entries to
*	the application's function table.
*
*	The function table is maintained as an Exec linked list.  The AppShell
*	will free all entries when the application is shutdown.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	entries	- Pointer to a NULL terminated array of Funcs structures that
*		  describes the entry to be added to the function table.
*
*   RESULT
*	TRUE	- AppShell was able to add the array to the table.
*	FALSE	- Unable to add the array to the function table.
*
*   SEE ALSO
*	AddFuncEntry(), FreeFuncEntry(), FreeFuncEntries()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

BOOL AddFuncEntries (struct AppInfo * ai, struct Funcs * fels)
{
    BOOL retval = TRUE;
    struct Funcs *fes;
    WORD cntr;

    if (fels)
    {
	/* add each entry to the list */
	for (cntr = 0; (fels[cntr].fe_Name != NULL && retval); cntr++)
	{
	    /* get a pointer to the current table entry */
	    fes = &fels[cntr];

	    /* add the entry to the table */
	    retval = AddFuncEntry (ai, fes);
	}
    }
    else
    {
	retval = FALSE;
    }

    return (retval);
}

/****** appshell.library/FreeFuncEntries *************************************
*
*   NAME
*	FreeFuncEntries - Free an application's function table.
*
*   SYNOPSIS
*	FreeFuncEntries (ai)
*			 D0
*
*	struct AppInfo *ai;
*
*   FUNCTION
*	This function is used to free an application's entire function
*	table.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*
*   SEE ALSO
*	AddFuncEntry(), AddFuncEntries(), FreeFuncEntries()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID FreeFuncEntries (struct AppInfo * ai)
{
    struct List *list;
    struct Node *node, *next;

    /* get a pointer to the function table list */
    list = &(ai->ai_FuncList);

    /* make sure that there are entries in the list */
    if (list->lh_TailPred != (struct Node *) list)
    {
	/* step through the list, freeing each entry */
	node = list->lh_Head;
	while (next = node->ln_Succ)
	{
	    /* free the entry */
	    FreeFuncEntry (ai, (struct FuncEntry *) node);

	    /* get the next node */
	    node = next;
	}
    }
}

/****** appshell.library/GetFuncEntry **************************************
*
*   NAME
*	GetFuncEntry - Get a pointer to a function table entry.
*
*   SYNOPSIS
*	entry = GetFuncEntry (ai, cmd, fid)
*	D0		      D0  D1   A0
*
*	struct FuncEntry *entry;
*	struct AppInfo *ai;
*	STRPTR cmd;
*	ULONG fid;
*
*   FUNCTION
*	This function is used to obtain a pointer to a function table entry.
*
*	Currently the FuncEntry structure is private.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	cmd	- Pointer to the command name to locate.  Either cmd or fid
*		  must be provided.
*	fid	- Command ID of the command to locate.
*
*   RESULT
*	entry	- Pointer to a FuncEntry structure that describes the command.
*
*   SEE ALSO
*	GetFuncID(), GetFuncName()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

struct FuncEntry *
GetFuncEntry (struct AppInfo * ai, STRPTR anchor, ULONG fid)
{
    struct FuncEntry *fe = NULL;
    struct FuncIDEntry *fie;
    UBYTE name[12];

    if (anchor)
    {
	fe = (struct FuncEntry *) FindNameI (&(ai->ai_FuncList), anchor);
    }
    else if (fid)
    {
	/* convert number to string */
	sprintf (name, "%lx", fid);

	/* attempt to locate the entry using it's ID */
	if (fie = (struct FuncIDEntry *)
	    FindNameI (&(ai->ai_FuncList2), name))
	{
	    /* get a pointer the FuncEntry node */
	    fe = fie->fie_FE;
	}
    }

    return (fe);
}

/****** appshell.library/GetFuncID *****************************************
*
*   NAME
*	GetFuncID - Given a command name, return the ID for the function.
*
*   SYNOPSIS
*	fid = GetFuncID (ai, cmd)
*	D0		 D0  D1
*
*	ULONG fid;
*	struct AppInfo *ai;
*	STRPTR cmd;
*
*   FUNCTION
*	This function will return the command ID for the function.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	cmd	- Pointer to the command name to locate.
*
*   RESULT
*	fid	- Command ID of the function.
*
*   SEE ALSO
*	GetFuncEntry(), GetFuncName()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

ULONG GetFuncID (struct AppInfo * ai, STRPTR anchor)
{
    struct FuncEntry *fe;
    ULONG FuncID = NO_FUNCTION;

    if (anchor)
    {
	fe = (struct FuncEntry *) FindNameI (&(ai->ai_FuncList), anchor);
	FuncID = fe->fe_ID;
    }

    return (FuncID);
}

/****** appshell.library/GetFuncName ***************************************
*
*   NAME
*	GetFuncName - Given a command ID, return the name of the function.
*
*   SYNOPSIS
*	cmd = GetFuncName (ai, fid)
*	D0		   D0  D1
*
*	STRPTR cmd;
*	struct AppInfo *ai;
*	ULONG fid;
*
*   FUNCTION
*	This function will return the name of the function for the given
*	command ID.
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	fid	- Command ID of the function.
*
*   RESULT
*	cmd	- Pointer to the command name.
*
*   SEE ALSO
*	GetFuncEntry(), GetFuncID()
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

STRPTR GetFuncName (struct AppInfo * ai, ULONG id)
{
    STRPTR retval = NULL;
    struct FuncEntry *fe;
    struct FuncIDEntry *fie;
    UBYTE name[12];

    if (id)
    {
	/* convert number to string */
	sprintf (name, "%lx", id);

	/* attempt to locate the entry using it's ID */
	if (fie = (struct FuncIDEntry *)
	    FindNameI (&(ai->ai_FuncList2), name))
	{
	    /* get a pointer to the FuncEntry node */
	    fe = fie->fie_FE;

	    /* get a pointer to the name of the function */
	    retval = fe->fe_Name;
	}
    }

    return (retval);
}
