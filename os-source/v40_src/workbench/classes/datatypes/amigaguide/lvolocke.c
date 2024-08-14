/* LockELVO.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

/*****************************************************************************/

BPTR ASM LVOLockE (REG (a6) struct AGLib * ag, REG (a0) BPTR p, REG (d1) STRPTR name, REG (d2) LONG mode)
{
    struct Process *pr = (struct Process *) FindTask (NULL);
    BPTR retval = NULL;
    BPTR lock;
    BPTR old;

    BOOL colon = FALSE;
    STRPTR lang = NULL;
    struct Locale *loc;
    WORD i, maxLoop;
    char path[100];
    BOOL doProgDir;
    UBYTE cnt = 0;
    APTR oldWP;

    /* Make sure we have a name at least. */
    if (name)
    {
	/* See if there is a volume component to the name */
	for (i = strlen (name) - 1; i >= 0; i--)
	{
	    if (name[i] == ':')
	    {
		colon = TRUE;
		break;
	    }
	}

	/* If the name doesn't have a colon in it, then see if we have a localized version of the file. */
	if ((colon == FALSE) && LocaleBase)
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

				if (GetProgramDir () && (retval = Lock (path, mode)))
				    break;

				if (IoErr() != ERROR_OBJECT_NOT_FOUND)
				    return (retval);
			    }
			    else
			    {
				doProgDir = TRUE;
				strcpy (path, "HELP:");
				AddPart (path, lang, sizeof (path));
				AddPart (path, name, sizeof (path));

				if (retval = Lock (path, mode))
				    break;

				if (IoErr() != ERROR_OBJECT_NOT_FOUND)
				    return (retval);
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
	if (retval = Lock (name, mode))
	{
	    /* Clear the error return */
	    SetIoErr (0);

	    /* Return the path */
	    return (retval);
	}

	if (IoErr() != ERROR_OBJECT_NOT_FOUND)
	    return (retval);

	/* See if it is in PROGDIR: */
	strcpy (path, "PROGDIR:");
	AddPart (path, name, sizeof (path));
	oldWP = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR) - 1;
	retval = Lock (path, mode);
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
	    /* Convert to a lock */
	    lock = (BPTR) (((LONG *) p)[1]);

	    /* Go to the new place */
	    if (old = CurrentDir (lock))
	    {
		/* Try locking the name */
		retval = Lock (name, mode);

		/* Go back to the previous directory */
		CurrentDir (old);
	    }

	    /* Get the next path node */
	    p = ((LONG *) p)[0];
	}
    }
    else
    {
	pr->pr_Result2 = ERROR_INVALID_COMPONENT_NAME;
    }
    return (retval);
}
