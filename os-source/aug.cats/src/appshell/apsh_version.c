/* apsh_version.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * version command
 */

#include "appshell_internal.h"
#include "appshell_rev.h"

/****** appshell.library/__VERSION *******************************************
*
*   NAME
*	VERSION - Uses NotifyUser() to show the current version.
*
*   SYNOPSIS
*	VersionID	Function ID
*
*   FUNCTION
*	Uses NotifyUser() to show the current version of the application or
*	the AppShell.
*
*	As a string command line:
*
*	    VERSION
*	    APPSHELL/S
*
*		APPSHELL	To display the AppShell version.  Defaults
*				to showing the application version.
*
*   BUGS
*	No tags are implemented.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

VOID VersionFunc (struct AppInfo * ai, STRPTR args, struct TagItem * tl)
{
    struct Funcs *f;

    /* set the default value */
    ai->ai_TextRtn = ai->ai_AppVersion;

    if (f = (struct Funcs *) GetTagData (APSH_FuncEntry, NULL, tl))
    {
	if (f->fe_Options[0])
	{
	    ai->ai_TextRtn = VERS;
	}
    }
}
