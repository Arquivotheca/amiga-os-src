/* getprefsdrawer.c
 * Preference directory routines
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * Written by David N. Junod
 */

#include "prefsbase.h"

#define	DB(x)	;
void kprintf(void *, ...);

/****** Prefs/GetPrefsDrawer ***********************************************
*
*   NAME
*	GetPrefsDrawer - Locate the preference directory for an application.
*
*   SYNOPSIS
*	dir = GetPrefsDrawer (base, flags)
*	d0		      a1    d0
*
*	BPTR dir;
*	STRPTR base;
*	ULONG flags;
*
*   FUNCTION
*	This function will return a lock on the directory that the
*	preference files should be stored in, based on the application's
*	base name.
*
*	This will default to creating the directory if it doesn't exist.
*
*	The programmer must use UnLock(dir) to free the directory lock when
*	he is through with it.
*
*	This function can ONLY be called from a process.
*
*   INPUTS
*	base	- Pointer to the NULL terminated base name of the
*		  application.  If a NULL is passed for the name, then
*		  return a lock on the system preference drawer.
*
*	flags	- Used to modify the actions performed.
*
*		PREFS_GLOBAL_GPDF
*		Indicate that you want to get a lock on the global
*		preference directory, instead of a per-user directory.
*
*		PREFS_READONLY_GPDF
*		Indicate that you only want to obtain preference files.
*		This will cause GetPrefsDrawer() to fail if the directory
*		doesn't already exist (unless the PREFS_FALLBACK_GPDF
*		flag is set).
*
*		PREFS_FALLBACK_GPDF
*		If the directory doesn't exist, then return a lock on the
*		system preference area.
*
*
*   RESULTS
*	dir	- A lock on the directory.  If NULL, then use IoErr()
*		  to obtain the reason why the directory wasn't returned.
*
*********************************************************************
*
* Created:  10-Sep-90, David N. Junod
*
*/

BPTR __asm
GetPrefsDrawer (register __a1 STRPTR basename, register __d0 ULONG flags)
{
    struct LibBase *lb = ((struct ExtLibrary *) PrefsBase)->el_Base;
    struct Process *proc = (struct Process *) FindTask (NULL);
    struct FileInfoBlock *fib;
    STRPTR template = "SYS:Prefs";
    STRPTR source = "ENV:%s";
    STRPTR work = NULL;
    BPTR prefs = NULL;
    LONG msize;

    DB (kprintf("GPD(%s,0x%lx)\n", basename, flags));

    /* Clear the error return value */
    proc->pr_Result2 = 0L;

    /* Calculate how much memory we need */
    msize = sizeof (struct FileInfoBlock) +
      MIN ((strlen (basename) + 10L), 82L);

    /* Allocate our temporary memory block */
    if (fib = (struct FileInfoBlock *) AllocMem (msize, MEMF_CLEAR))
    {
	/* Build the configuration drawer name */
	work = ((VOID *) ((fib) + 1));
	sprintf (work, source, basename);

	/* Get a lock on the configuration drawer */
	if (prefs = Lock (work, ACCESS_READ))
	{
	    /* Examine the lock */
	    if (Examine (prefs, fib) != 0)
	    {
		/* Make sure it's a directory */
		if (fib->fib_DirEntryType < 0)
		{
		    /* Free the lock */
		    UnLock (prefs);
		    prefs = NULL;

		    /* Set the error return value */
		    proc->pr_Result2 = ERROR_OBJECT_WRONG_TYPE;
		}
	    }
	    else
	    {
		/* Unable to examine, so free the lock */
		UnLock (prefs);
		prefs = NULL;
	    }
	}
	else if (flags & PREFS_FALLBACK_GPDF)
	{
	    /* Return a lock on the system directory */
	    prefs = DupLock (lb->lb_SysPrefs);
	}
	else if (flags & PREFS_READONLY_GPDF)
	{
	}
	else
	{
	    /* No configuration directory, let's make one */
	    if (prefs = CreateDir (work))
	    {
		/* Unlock the exclusive access on the directory */
		UnLock (prefs);

		/* Reopen the directory as shared */
		if (prefs = Lock (work, ACCESS_READ))
		{
		    struct Library *IconBase;

		    /* if icon.library is open, then create an icon */
		    if (IconBase = OpenLibrary ("icon.library", 31))
		    {
			struct DiskObject *dob;

			/* Figure out what to use as the template icon */
			if (SysBase->lib_Version < 36)
			    template = "SYS:Empty";

			/* Get the template directory icon */
			if (dob = GetDiskObject (template))
			{
			    /* Free the icon's position */
			    dob->do_CurrentX = NO_ICON_POSITION;
			    dob->do_CurrentY = NO_ICON_POSITION;

			    /* save the icon */
			    PutDiskObject (work, dob);

			    /* get rid of the icon now that we're done with it */
			    FreeDiskObject (dob);

			}	/* end of if get template icon */

			/* Close the icon shared system library */
			CloseLibrary (IconBase);

		    }		/* end of if icon.library open */
		}		/* end of if lock directory */
	    }			/* end of if create prefs directory */
	}			/* end of lock failed */

	/* Free the temporary work buffer */
	FreeMem ((APTR) fib, msize);
    }

    return (prefs);
}
