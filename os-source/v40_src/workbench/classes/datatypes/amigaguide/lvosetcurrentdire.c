/* lvosetcurrentdire.c
 * amigaguide.datatype
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

/*****************************************************************************/

BPTR IsFileThere (struct AGLib * ag, STRPTR path)
{
    BPTR retval = NULL;
    BPTR parent;
    BPTR lock;

    if (lock = Lock (path, ACCESS_READ))
    {
	parent = ParentDir (lock);
	UnLock (lock);
	retval = CurrentDir (parent);
    }

    return (retval);
}

/*****************************************************************************/

BPTR ASM LVOSetCurrentDirE (REG (a6) struct AGLib * ag, REG (a0) BPTR p, REG (d1) STRPTR name)
{
    struct Process *pr = (struct Process *) FindTask (NULL);
    BPTR retval = NULL;
    BPTR old, lock;

    STRPTR lang = NULL;
    struct Locale *loc;
    UWORD i, maxLoop;
    char path[100];
    BOOL doProgDir;
    UBYTE cnt = 0;
    APTR oldWP;


    /* Make sure we have a name at least. */
    if (name)
    {
	/* Do we have locale.library opened? */
	if (LocaleBase)
	{
	    if (loc = OpenLocale (NULL))
	    {
		maxLoop = 10;
		for (i = 0; i < maxLoop; i++)
		{
		    if (lang)
		    {
			oldWP = pr->pr_WindowPtr;
			pr->pr_WindowPtr = (APTR) - 1;
			doProgDir = TRUE;
			if (!(lock = Lock ("PROGDIR:", ACCESS_READ)))
			    if (lock = Lock ("HELP:", ACCESS_READ))
				doProgDir = FALSE;
			UnLock (lock);
			pr->pr_WindowPtr = oldWP;

			cnt = 3;
			while (--cnt)
			{
			    if (doProgDir)
			    {
				doProgDir = FALSE;
				strcpy (path, "PROGDIR:Help");
				AddPart (path, lang, sizeof (path));
				AddPart (path, name, sizeof (path));

				if (GetProgramDir () && (retval = IsFileThere (ag, path)))
				    break;
			    }
			    else
			    {
				doProgDir = TRUE;
				strcpy (path, "HELP:");
				AddPart (path, lang, sizeof (path));
				AddPart (path, name, sizeof (path));

				if (retval = IsFileThere (ag, path))
				    break;
			    }
			}

			/* we found something! */
			if (cnt)
			    break;
		    }

		    if (i < 10)
			lang = loc->loc_PrefLanguages[i];
		}

		CloseLocale (loc);
	    }
	}

	if (cnt)
	{
	    /* Clear the error return */
	    SetIoErr (0);

	    /* Return the path */
	    return retval;
	}

	/* See if it exists in the current directory */
	if (retval = Lock (name, ACCESS_READ))
	{
	    /* Release the lock */
	    UnLock (retval);

	    /* Get the path */
	    if (retval = DupLock (pr->pr_CurrentDir))
		retval = CurrentDir (retval);

	    /* Clear the error return */
	    SetIoErr (0);

	    /* Return the path */
	    return (retval);
	}

	/* See if it is in PROGDIR */
	strcpy (path, "PROGDIR:");
	AddPart (path, name, sizeof (path));
	oldWP = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR) - 1;
	retval = IsFileThere (ag, path);
	pr->pr_WindowPtr = oldWP;
	if (retval)
	{
	    /* Clear the error return */
	    SetIoErr (0);

	    /* Return the path */
	    return (retval);
	}

	/* See if they provided a path pointer */
	if (p == NULL)
	{
	    /* No, use ours. */
	    p = ag->ag_GlobalPath;
	}

	/* funky bptr-list following! */
	while ((p = (BPTR) BADDR (p)) && (retval == NULL))
	{
	    /* Convert to a lock and duplicate it */
	    lock = (BPTR) (((LONG *) p)[1]);

	    /* Go to the new place */
	    if (old = CurrentDir (lock))
	    {
		/* Try locking the name */
		if (retval = Lock (name, ACCESS_READ))
		{
		    /* We found the file, so return its directory */
		    UnLock (retval);

		    /* Unwind */
		    CurrentDir (old);

		    /* Put an unlockable lock in as current directory */
		    if (retval = DupLock (lock))
			retval = CurrentDir (retval);

		    /* Clear the error return */
		    SetIoErr (0);

		    return (retval);
		}

		/* Go back to the previous directory */
		CurrentDir (old);
	    }

	    /* Get the next path node */
	    p = ((LONG *) p)[0];
	}

	SetIoErr (ERROR_OBJECT_NOT_FOUND);
    }
    else
    {
	SetIoErr (ERROR_INVALID_COMPONENT_NAME);
    }

    return (NULL);
}
