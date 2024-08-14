/* apsh_lib.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * shared library open/close routines
 *
 * $Id: apsh_lib.c,v 1.4 1992/09/07 17:55:45 johnw Exp johnw $
 *
 * $Log: apsh_lib.c,v $
 * Revision 1.4  1992/09/07  17:55:45  johnw
 * Minor changes.
 *
 * Revision 1.1  91/12/12  14:51:49  davidj
 * Initial revision
 * 
 * Revision 1.1  90/07/02  11:23:32  davidj
 * Initial revision
 *
 *
 */

#include "appshell_internal.h"

void kprintf (void *, ...);
#define	DB(x)	;

static STRPTR LibNames[] =
{
    "rexxsyslib",
    "rexxsupport",
    "asl",
    "commodities",
    "diskfont",
    "dos",
    "gadtools",
    "graphics",
    "icon",
    "intuition",
    "layers",
    "iffparse",
    "translator",
    "utility",
    "workbench",
    "appobjects",
    "amigaguide",
    "prefs",
};

BOOL OpenLibraries (struct AppInfo * ai, struct TagItem *ObjAttrs)
{
    ULONG version = 0L, base = 0L, nametag = 0L;
    BOOL more = TRUE, retval = TRUE, needed = TRUE;
    struct TagItem *ti = NULL, *next_ti = NULL;
    UBYTE name[128];
    STRPTR namep;

    /* initialize the step tag */
    next_ti = ti = ObjAttrs;

    /* go thru the list of tags */
    while (more && retval && ti)
    {
	switch (ti->ti_Tag)
	{
	    case APSH_LibNameTag:
		nametag = ti->ti_Data;
		break;

	    case APSH_LibName:
		namep = (STRPTR) ti->ti_Data;
		strcpy (name, namep);
		break;

	    case APSH_LibVersion:
		version = ti->ti_Data;
		break;

	    case APSH_LibStatus:
		needed = (BOOL) ti->ti_Data;
		break;

	    case APSH_ARexxSys:
	    case APSH_ARexxSup:
	    case APSH_ASL:
	    case APSH_Commodities:
	    case APSH_DiskFont:
	    case APSH_DOS:
	    case APSH_GadTools:
	    case APSH_Gfx:
	    case APSH_Icon:
	    case APSH_Intuition:
	    case APSH_Layers:
	    case APSH_IFF:
	    case APSH_Translate:
	    case APSH_Utility:
	    case APSH_Workbench:
	    case APSH_AppObjects:
	    case APSH_Hyper:
	    case APSH_Prefs:
		nametag = ti->ti_Tag;
		base = ti->ti_Data;
		break;

	    case APSH_LibBase:
		base = ti->ti_Data;
		break;

	    case TAG_DONE:
		more = FALSE;
		break;
	}

	/* Ready to open a library? */
	if (base)
	{
	    if (nametag)
	    {
		nametag -= APSH_ARexxSys;

		if ((nametag >= 0L) &&
		    (nametag <= (APSH_Prefs - APSH_ARexxSys)))
		{
		    namep = LibNames[nametag];
		    sprintf (name, "%s.library", namep);
		}
		else
		{
		    /* Make sure we have an AppInfo structure */
		    if (ai)
		    {
			/* Fill error return */
			ai->ai_Pri_Ret = RETURN_FAIL;
			ai->ai_Sec_Ret = APSH_INVALID_NAMETAG;
			ai->ai_TextRtn =
			    PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
		    }

		    /* indicate error */
		    return (FALSE);
		}

		/* Clear the tag trigger */
		nametag = 0L;
	    }

	    /* open the library */
	    *((ULONG *) base) = (ULONG) OpenLibrary (name, version);

	    /* was it required? */
	    retval = (BOOL) ((needed) ? (*((ULONG *) base)) : TRUE);

	    /* Failure and AppInfo? */
	    if (!retval && ai)
	    {
		ai->ai_Pri_Ret = RETURN_FAIL;
		ai->ai_Sec_Ret = APSH_CLDNT_OPEN;
		ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, (int)name);
	    }

	    /* Clear the temporary variable */
	    base = NULL;
	}

	/* get the next tag */
	ti = NextTagItem (&next_ti);
    }

    return (retval);
}

VOID CloseLibraries (struct AppInfo * ai, struct TagItem * ObjAttrs)
{
    struct TagItem *ti = NULL, *next_ti = NULL;
    BOOL more = TRUE;
    ULONG data;

    /* initialize the step tag */
    next_ti = ti = ObjAttrs;

    /* go thru the list of tags */
    while (more && ti)
    {
	/* get the data */
	data = ti->ti_Data;

	switch (ti->ti_Tag)
	{
	    case APSH_ARexxSys:
	    case APSH_ARexxSup:
	    case APSH_ASL:
	    case APSH_Commodities:
	    case APSH_DiskFont:
	    case APSH_DOS:
	    case APSH_GadTools:
	    case APSH_Gfx:
	    case APSH_Icon:
	    case APSH_Intuition:
	    case APSH_Layers:
	    case APSH_IFF:
	    case APSH_Translate:
	    case APSH_Utility:
	    case APSH_Workbench:
	    case APSH_AppObjects:
	    case APSH_Hyper:
	    case APSH_Prefs:
	    case APSH_LibBase:
		if ((*((ULONG *) data)))
		{
		    CloseLibrary ((struct Library *) (*((ULONG *) data)));
		}
		break;

	    case TAG_DONE:
		more = TRUE;
		break;
	}

	/* get the next tag */
	ti = NextTagItem (&next_ti);
    }
}
