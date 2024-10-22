/* apsh_stdfuncs.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * generic Standard functions
 *
 * $Id: apsh_stdfuncs.c,v 1.4 1992/09/07 17:59:16 johnw Exp johnw $
 *
 * $Log: apsh_stdfuncs.c,v $
 * Revision 1.4  1992/09/07  17:59:16  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:54:06  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:38:17  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"
#include <dos/dostags.h>

void kprintf (void *,...);

#define	DN(x)	;
#define	DB(x)	;
#define	DF(x)	;
#define	DQ(x)	;
#define	DE(x)	;
#define	DD(x)	;

struct Funcs handler_funcs[] =
{
    {"Alias", AliasFunc, AliasID, "NAME,COMMAND/F", 2L, APSHF_LOCKON | APSHF_SYSTEM,},
    {"Disable", DisableFunc, DisableID, "NAMES/M", 1L, APSHF_LOCKON | APSHF_SYSTEM,},
    {"Enable", EnableFunc, EnableID, "NAMES/M,FORCE/S", 2L, APSHF_LOCKON | APSHF_SYSTEM,},
    {"Fault", FaultFunc, FaultID, "/N", 1L, APSHF_LOCKON | APSHF_SYSTEM,},
    {"Help", HelpFunc, HelpID, "COMMAND,PROMPT/S", 2L, APSHF_LOCKON | APSHF_SYSTEM,},
    {"Stub", StubFunc, StubID, NULL, NULL, APSHF_LOCKON | APSHF_SYSTEM,},
    {"Version", VersionFunc, VersionID, NULL, NULL, APSHF_LOCKON | APSHF_SYSTEM,},
    {"Why", WhyFunc, WhyID, ",", 0L, APSHF_LOCKON | APSHF_SYSTEM,},
    {NULL, NO_FUNCTION,}	/* end of array */

#if 0

    {"ExecMacro", ExecMacroFunc, ExecMacroID,},
    {"Fault", FaultFunc, FaultID, NULL, NULL, APSHF_LOCKON,},
    {"GetAttr", GetFunc, GetID, NULL, NULL, APSHF_LOCKON,},
    {"Group", GroupFunc, GroupID, NULL, NULL, APSHF_LOCKON,},
    {"Help", HelpFunc, HelpID, NULL, NULL, APSHF_LOCKON,},
    {"Learn", LearnFunc, LearnID, NULL, NULL, APSHF_LOCKON,},
    {"LoadMacro", LoadMacroFunc, LoadMacroID, NULL, NULL, APSHF_LOCKON,},
    {"Priority", PriorityFunc, PriorityID, NULL, NULL, APSHF_LOCKON,},
    {"SaveMacro", SaveMacroFunc, SaveMacroID, NULL, NULL, APSHF_LOCKON,},
    {"Select", SelectFunc, SelectID, NULL, NULL, APSHF_LOCKON,},
    {"SetAttr", SetFunc, SetID, NULL, NULL, APSHF_LOCKON,},
    {"Status", StatusFunc, StatusID, NULL, NULL, APSHF_LOCKON,},
    {NULL, NO_FUNCTION,}	/* end of array */

#endif

};

/****** appshell.library/__ALIAS *********************************************
*
*   NAME
*	ALIAS - Used to build new commands from existing commands.
*
*   SYNOPSIS
*	AliasID		Function ID
*
*   FUNCTION
*	This command is used to build new commands from existing commands
*	in the function table.
*
*	As a string command line:
*
*	    ALIAS
*	    NAME,COMMAND/F
*
*		NAME	New name to assign to command.
*		COMMAND	Command, and its arguments to assign to NAME.
*
*   EXAMPLE
*
*	The following command line would assign the command OPEN to the
*	new command name README, and would use READ.ME as the parameter
*	to pass.
*
*	    ALIAS README OPEN READ.ME
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID AliasFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct Funcs *f;

    if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
    {
	struct FuncEntry *old = NULL;
	STRPTR name, cmd;

	/* pull out the options */
	name = (STRPTR) f->fe_Options[0];
	cmd = (STRPTR) f->fe_Options[1];

	/* see if the function already exists */
	old = GetFuncEntry (ai, name, NULL);
	if (old)
	{
	    ai->ai_Pri_Ret = RETURN_WARN;
	    /* ZZZ: function is already in the function table */
	}
	else
	{
	    STRPTR clone = NULL, argv[MAXARG];
	    struct Funcs entry = {0};
	    ULONG argc = 0L;
	    WORD i = 0;

	    /* parse the new command line */
	    clone = BuildParseLine (cmd, &argc, argv);

	    /* get the function entry for the aliased command */
	    old = GetFuncEntry (ai, argv[0], NULL);

	    if (old && !(old->fe_Flags & APSHF_PRIVATE))
	    {
		/* copy the necessary fields */
		entry.fe_Name = name;
		entry.fe_Func = old->fe_Func;
		entry.fe_ID = old->fe_ID;
		entry.fe_Template = old->fe_Template;
		entry.fe_NumOpts = old->fe_NumOpts;
		entry.fe_Flags = old->fe_Flags | APSHF_ALIAS;
		entry.fe_HelpID = old->fe_HelpID;
		entry.fe_GroupID = old->fe_GroupID;

		/* see if parameters where passed */
		entry.fe_Params = old->fe_Params;
		if (argc > 1L)
		{
		    /*
		     * find out where we start in args to get the parameters
		     */
		    i = (WORD) (argv[1] - clone);

		    /* point at the parameters */
		    entry.fe_Params = (STRPTR) f->fe_Options[1];
		}

		/* add the alias to the function table */
		AddFuncEntry (ai, &(entry));
	    }
	    else
	    {
		ai->ai_Pri_Ret = RETURN_WARN;
		/* ZZZ: No such command */
	    }

	    /* free the temporary parse line */
	    FreeParseLine (clone);
	}			/* end of add to function table */
    }				/* end of if APSH_FuncEntry */
}

/****** appshell.library/__DISABLE *******************************************
*
*   NAME
*	DISABLE - Disable a function and its interfaces.
*
*   SYNOPSIS
*	DisableID	Function ID
*
*   FUNCTION
*	This function allows an application or user to disable a function.
*
*	If the function is attached to a gadget or menu item, then that item
*	is also disabled.  This function does wildcard expansion.
*
*	As a string command line:
*
*	    DISABLE
*	    NAMES/M
*
*		NAMES	The list of valid function names to disable.
*
*	As a TagItem attribute list:
*
*	    APSH_NameTag, <name>
*		where <name> is a valid function name.
*
*   EXAMPLE
*	The following command would disable all commands in an application
*	except for those that would be available before the users selects
*	NEW.
*
*	    DISABLE #?
*	    ENABLE NEW OPEN ABOUT QUIT PREF#?
*
*   SEE ALSO
*	ENABLE
*
*   BUGS
*	Doesn't disable menu items yet.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID able_func (struct AppInfo * ai, struct FuncEntry * fe, BOOL stat)
{

    if (fe)
    {
	if (!(fe->fe_Flags & APSHF_PRIVATE))
	{
	    ULONG oflags = fe->fe_Flags;

	    /* twiddle status */
	    fe->fe_Flags &= ~APSHF_DISABLED;
	    if (stat && !(fe->fe_Flags & APSHF_LOCKON))
	    {
		fe->fe_Flags |= APSHF_DISABLED;
	    }

	    /* Enable/disable gadget */
	    if ((fe->fe_Flags != oflags) && fe->fe_Object && fe->fe_WinNode)
	    {
		struct ObjectNode *con;
		struct Gadget *gadg;
		struct Window *win;

		/* Get a pointer to the gadget */
		con = fe->fe_Object;
		gadg = con->on_Gadget;

		/* Get a pointer to the window */
		win = (fe->fe_WinNode)->wn_Win;

		/* disable the gadget */
		if (gadg && win)
		{
		    if (IsGadToolObj (&(con->on_Object)))
		    {
			GT_SetGadgetAttrs (gadg, win, NULL,
					   GA_DISABLED, stat,
					   TAG_DONE);
		    }
		    else
		    {
			/* set the gadget to the correct rectangle */
			SetGadgetAttrs (gadg, win, NULL,
					GA_SELECTED, FALSE,
					GA_DISABLED, stat,
					TAG_END);
		    }
		}		/* end of make sure gadg & win */
	    }			/* end of if object */

	    /* Enable/disable gadget */
	    if ((fe->fe_Flags != oflags) && fe->fe_MenuNumber && fe->fe_WinNode)
	    {
		struct Window *win;
		USHORT mnum;

		/* Get a pointer to the menu item */
		mnum = fe->fe_MenuNumber;

		/* Get a pointer to the window */
		win = (fe->fe_WinNode)->wn_Win;
		DQ (kprintf ("WinNode 0x%lx [%s]\n", fe->fe_WinNode, fe->fe_WinNode->wn_Header.mho_Node.ln_Name));

		/* Manipulate the menu */
		if (win)
		{
		    if (stat)
		    {
			DQ (kprintf ("OffMenu [%s] 0x%lx 0x%lx\n", fe->fe_Name, win, (ULONG) mnum));
			OffMenu (win, mnum);
		    }
		    else
		    {
			DQ (kprintf (" OnMenu [%s] 0x%lx 0x%lx\n", fe->fe_Name, win, (ULONG) mnum));
			OnMenu (win, mnum);
		    }
		}
	    }			/* end of if menu */

	}			/* end of if private */
    }				/* end of if function entry */
}

VOID
AbleFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl, BOOL stat)
{
    struct FuncEntry *fe;
    struct Funcs *f;
    STRPTR *names = NULL, name = NULL;
    BOOL force = FALSE;

    DB (kprintf ("AbleFunc 0x%lx 0x%lx 0x%lx\n", ai, args, tl));
    if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
    {
	/* get the name array */
	names = (STRPTR *) f->fe_Options[0];

	/* Enabling? */
	if (!stat)
	{
	    /* FORCE switch used? */
	    force = (BOOL) f->fe_Options[1];
	}
    }
    else
    {
	/* get the function name from the tag list */
	name = (STRPTR) GetTagData (APSH_NameTag, NULL, tl);
    }

    if (names)
    {
	struct AnchorList al = {NULL};
	struct List *list = &(ai->ai_FuncList);
	STRPTR *sptr = names;
	LONG retval = RETURN_ERROR;

	while (*sptr)
	{
	    DN (kprintf ("AF: *sptr [%s] ", *sptr));

#if 1
	    /* Show that we need caseless compares. */
	    al.al_Flags |= APSHF_ANCHOR_NOCASE;
#else
	    /* the doc's say case-insensitive. but it isn't */
	    strupr (*sptr);
#endif

	    DN (kprintf ("strupr(*sptr) [%s]\n", *sptr));

	    for (retval = LMatchFirst (list, *sptr, &al);
		 retval == 0L;
		 retval = LMatchNext (&al))
	    {
		/* cast the current node to a FuncEntry node */
		fe = (struct FuncEntry *) al.al_CurNode;

		/* Disable? */
		if (stat)
		{
		    if (fe->fe_Disable == 0)
		    {
			/* disable the function */
			able_func (ai, fe, stat);
		    }

		    fe->fe_Disable++;
		}
		else
		{
		    if (fe->fe_Disable > 0)
			fe->fe_Disable--;

		    if ((fe->fe_Disable == 0) || force)
		    {
			/* Enable the function */
			able_func (ai, fe, stat);

			/* Clear the disable counter */
			fe->fe_Disable = 0;
		    }
		}
	    }

	    DN (kprintf ("AF: ME retval %ld\n", retval));

	    /* end the pattern match */
	    LMatchEnd (&al);

	    /* next pattern */
	    sptr++;
	}
    }
    else if (name)
    {
#if 0
	/* disable the function */
	able_func (ai, name, stat);
#endif
    }
}

VOID DisableFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    AbleFunc (ai, args, tl, TRUE);
}


/****** appshell.library/__ENABLE ********************************************
*
*   NAME
*	ENABLE - Enable a function and its interfaces.
*
*   SYNOPSIS
*	EnableID	Function ID
*
*   FUNCTION
*	This function allows an application or user to enable a function.
*
*	If the function is attached to a gadget or menu item, then that item
*	is also enabled.  This function does wildcard expansion.
*
*	As a string command line:
*
*	    ENABLE
*	    NAMES/M
*
*		NAMES	The list of valid function names to enable.
*
*	As a TagItem attribute list:
*
*	    APSH_NameTag, <name>
*		where <name> is a valid function name.
*
*   SEE ALSO
*	DISABLE
*
*   BUGS
*	Doesn't enable menu items yet.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID EnableFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    AbleFunc (ai, args, tl, FALSE);
}

VOID ExecMacroFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

/****** appshell.library/__FAULT *********************************************
*
*   NAME
*	FAULT - Return the text for an error number.
*
*   SYNOPSIS
*	FaultID		Function ID
*
*   FUNCTION
*	This function will return the text assigned to an error number.
*
*	As a string command line:
*
*	    FAULT
*	    /N
*
*		/N	Number to return the text for.
*
*	This function is implemented by the ARexx message handler.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID FaultFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct Funcs *f;
    ULONG id = ai->ai_LastError;

    /* See if we have a parsed command */
    if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
    {
	/* See if they passed a number */
	id = *((ULONG *) f->fe_Options[0]);
    }

    ai->ai_TextRtn = PrepText (ai, APSH_USER_ID, id, NULL);
}

VOID GetFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

VOID GroupFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

VOID HelpFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct FuncEntry *fe;
    struct Funcs *f;
    STRPTR cmd;

    /* See if we have a parsed command */
    if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
    {
	cmd = (STRPTR) f->fe_Options[0];

	/* See if it's a command... */
	if (fe = GetFuncEntry (ai, cmd, NULL))
	{
	    ai->ai_Pri_Ret = RETURN_OK;
	    ai->ai_Sec_Ret = NULL;

	    if (fe->fe_Template)
	    {
		sprintf (ai->ai_WorkText, "%s: %s", fe->fe_Name, fe->fe_Template);
	    }
	    else
	    {
		/* Needs to be localized */
		sprintf (ai->ai_WorkText, "%s: <no template>", fe->fe_Name);
	    }

	    if (fe->fe_Params)
	    {
		strcat (ai->ai_WorkText, ": ");
		strcat (ai->ai_WorkText, fe->fe_Params);
	    }

	    ai->ai_TextRtn = ai->ai_WorkText;
	}
	else
	{
	    ai->ai_Pri_Ret = RETURN_WARN;
	    ai->ai_Sec_Ret = APSH_UNKNOWN_COMMAND;
	    ai->ai_TextRtn =
		PrepText (ai, APSH_MAIN_ID, APSH_UNKNOWN_COMMAND, NULL);
	}
    }

    return;
}

/* learn a macro */
VOID LearnFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

#if 0
    STRPTR argv[MAXARG];
    UWORD argc;

    /* parse the command line */
    if (args)
    {
	/* parse the command line */
	argc = ParseLine (args, argv);

	if (strcmp (argv[1], "?") == 0)
	    ai->ai_TextRtn = "LEARN <filename>/STOP";
	else if (strcmpi (argv[1], "STOP") == 0)
	{
	    /* close the macro file */
	    if (ai->ai_MacroFH)
		Close (ai->ai_MacroFH);

	    /* turn off the learn flag */
	    ai->ai_Flags &= ~APSHF_LEARN;
	}
	else
	{
	    if (ai->ai_MacroFH = Open (argv[1], MODE_NEWFILE))
	    {
		ai->ai_Flags |= APSHF_LEARN;

		/* write ARexx header */
		Write (ai->ai_MacroFH, (APTR) "/* LEARN macro file */\n", 23L);
	    }
	    else
		NotifyUser (ai, "Could not create file", NULL);
	}
    }
#endif
}

VOID LoadMacroFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

VOID PriorityFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

VOID SaveMacroFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

VOID SelectFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

VOID SetFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

VOID StatusFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}

/****** appshell.library/__STUB *********************************************
*
*   NAME
*	STUB - A do nothing function.
*
*   SYNOPSIS
*	StubID		Function ID
*
*   FUNCTION
*	This function does absolutely nothing.
*
*	As a string command line:
*
*	    STUB
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

/* do nothing function */
VOID StubFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    return;
}


/****** appshell.library/__WHY ************************************************
*
*   NAME
*	WHY - Return information on the last error.
*
*   SYNOPSIS
*	WhyID		Function ID
*
*   FUNCTION
*	For scripting purpose, the primary return value of a command should
*	return an error level.  This limits the error return values to small
*	ranges (to be consistent with DOS).  Therefore, if commands use the
*	secondary return field to contain information on the actual error,
*	then this function will allow scripts to obtain information on what
*	the last error actually was.
*
*	This command takes no parameters.
*
*	This function is implemented by the ARexx message handler.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID WhyFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{

    ai->ai_Pri_Ret = RETURN_OK;
    ai->ai_Sec_Ret = 0L;
    ai->ai_TextRtn = ai->ai_LastText;
}
