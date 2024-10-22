/* wbarg.c
 * How to get an icon from a Workbench argument
 * Copyright (C) 1990, Commodore-Amiga, Inc.
 * Written by David N. Junod, 27-Aug-90
 */

#if 1
#include "appshell_internal.h"
#else
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <libraries/dos.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/wb_protos.h>
#include <pragmas/wb.h>
#include <string.h>

extern struct Library *IconBase;
#endif

/****** appshell.library/IconFromWBArg ****************************************
*
*   NAME
*	IconFromWBArg - Obtains the icon from a Workbench argument.
*
*   SYNOPSIS
*	dob = IconFromWBArg (wbarg)
*	D0		     D0
*
*	struct DiskObject *dob;
*	struct WBArg *wbarg;
*
*   FUNCTION
*	Given a valid Workbench argument, this command will return the
*	proper icon.  This call uses the new (V36) GetDiskObjectNew call
*	to ensure that an icon will be returned.
*
*   INPUTS
*	wbarg	- Pointer to a filled in WBArg structure.
*
*   RESULTS
*	dob	- Pointer to a DiskObject structure.  Application must call
*		  FreeDiskObject when done with the icon.
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

struct DiskObject *GetDiskObjectNew (STRPTR name);
struct DiskObject *GetDiskObject (STRPTR name);
#pragma libcall IconBase GetDiskObjectNew 84 801

struct DiskObject *IconFromWBArg (struct WBArg * arg)
{
    BPTR old, new;
    UBYTE work_name[34];
    struct DiskObject *dob;

    /* Copy the WBArg contents */
    strcpy (work_name, arg->wa_Name);
    new = DupLock (arg->wa_Lock);

    /* If we don't have a name, then get one. Only Tools & Projects have
     * a valid name.
     */
    if (strlen (work_name) == 0)
    {
	LONG msize = sizeof (struct FileInfoBlock);
	struct FileInfoBlock *info;

	/* This block needs to be longword aligned, so allocate it */
	if (info = (struct FileInfoBlock *) AllocVec (msize, MEMF_CLEAR))
	{
	    /* Examine the lock, so that we can figure out the name */
	    if (Examine (new, info))
	    {
		/* Copy the name of the lock */
		strcpy (work_name, info->fib_FileName);

		/* See if the lock is a directory */
		if (info->fib_DirEntryType > 0)
		{
		    /* Unlock the existing lock */
		    UnLock (new);

		    /* Go to the parent drawer */
		    if ((new = ParentDir (arg->wa_Lock)) == NULL)
		    {
			/* A disk icon was passed */
			strcpy (work_name, "disk");
			new = DupLock (arg->wa_Lock);
		    }		/* End of if disk */
		}		/* End of if directory */
	    }			/* End of examine */

	    /* Free the temporary block */
	    FreeVec (info);
	}
    }

    /* go to the directory where the icon resides */
    old = CurrentDir (new);

    dob = GetDiskObjectNew (work_name);

    /* go back to where we used to be */
    CurrentDir (old);

    /* release the duplicated lock */
    UnLock (new);

    /* return the icon */
    return (dob);
}
