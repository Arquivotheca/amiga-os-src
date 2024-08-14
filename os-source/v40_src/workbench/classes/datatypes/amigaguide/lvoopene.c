/* OpenELVO.c
 *
 */

#include "amigaguidebase.h"

BPTR ASM LVOOpenE (REG (a6) struct AGLib * ag, REG (a0) BPTR p, REG (d1) STRPTR name, REG (d2) LONG mode)
{
    struct Process *pr = (struct Process *) FindTask (NULL);
    BPTR retval = NULL;
    BPTR lock;
    BPTR old;

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

				if (GetProgramDir () && (retval = Open (path, mode)))
				    break;
			    }
			    else
			    {
				doProgDir = TRUE;
				strcpy (path, "HELP:");
				AddPart (path, lang, sizeof (path));
				AddPart (path, name, sizeof (path));

				if (retval = Open (path, mode))
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

	/* Try opening the file. */
	if (retval = Open (name, mode))
	{
	    /* Clear the error return */
	    SetIoErr (0);

	    /* Return the path */
	    return (retval);
	}

	/* Try relative to progdir */
	strcpy (path, "PROGDIR:");
	AddPart (path, name, sizeof (path));
	oldWP = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR) - 1;
	retval = Open (path, mode);
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

	/* This function can not create a file */
	if (mode != MODE_NEWFILE)
	{
	    /* funky bptr-list following! */
	    while ((p = (BPTR) BADDR (p)) && (retval == NULL))
	    {
		/* Convert to a lock */
		lock = (BPTR) (((LONG *) p)[1]);

		/* Go to the new place */
		if (old = CurrentDir (lock))
		{
		    /* Try opening the file. */
		    retval = Open (name, mode);

		    /* Go back to the previous directory */
		    CurrentDir (old);
		}

		/* Get the next path node */
		p = ((LONG *) p)[0];
	    }
	}
	else
	{
	    pr->pr_Result2 = ERROR_BAD_NUMBER;
	}
    }
    else
    {
	pr->pr_Result2 = ERROR_INVALID_COMPONENT_NAME;
    }

    return (retval);
}
