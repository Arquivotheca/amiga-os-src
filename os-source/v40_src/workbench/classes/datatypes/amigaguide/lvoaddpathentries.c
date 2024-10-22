/* AddPathEntriesLVO.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

BPTR ASM LVOAddPathEntries (REG (a6) struct AGLib *ag, REG (a0) BPTR path, REG (d0) STRPTR * argptr)
{
    struct Process *pr = (struct Process *) FindTask (NULL);
    struct Path *nextNode, *tempNode, *pathNode;
    struct FileInfoBlock *fib;
    BPTR retval;
    STRPTR name;
    ULONG cntr;
    BOOL fail;
    BPTR tn;

    /* Default to the AmigaGuide path */
    retval = (path) ? path : ag->ag_GlobalPath;

    /* Allocate a FileInfoBlock, because it has to be longword aligned */
    if (fib = AllocDosObject (DOS_FIB, TAG_DONE))
    {
	/* Step through the argument array */
	cntr = 0;
	while (name = argptr[cntr])
	{
	    /* Default to failed */
	    fail = TRUE;

	    /* Allocate a path node */
	    if (tempNode = AllocVec (sizeof (struct Path), MEMF_CLEAR))
	    {
		/* Get a lock on the name */
		if (tempNode->path_Lock = Lock (name, ACCESS_READ))
		{
		    /* Examine the file */
		    if (Examine (tempNode->path_Lock, fib))
		    {
			/* Make sure it is a directory */
			if (fib->fib_DirEntryType > 0)
			{
			    /* Make it into a BPTR */
			    tn = MKBADDR (tempNode);

			    /* Add the entry to the end of the list */
			    if (retval && (nextNode = (struct Path *) BADDR (retval)))
			    {
				/* Check the list for duplicates */
				while (TRUE)
				{
				    /* See if it is already on the list */
				    if (SameLock (nextNode->path_Lock, tempNode->path_Lock) == LOCK_SAME)
					break;

				    pathNode = nextNode;
				    nextNode = (struct Path *) BADDR (nextNode->path_Next);
				    if (nextNode == NULL)
				    {
					pathNode->path_Next = tn;
					fail = FALSE;
					break;
				    }
				}
			    }
			    else
			    {
				/* No other entries, so just put it at the
				 * beginning of the list. */
				fail = FALSE;
				retval = tn;
			    }
			}
		    }

		    /* Wasn't added, so unlock it */
		    if (fail)
			UnLock (tempNode->path_Lock);
		}

		/* Wasn't added so free it */
		if (fail)
		    FreeVec (tempNode);
	    }
	    else
	    {
		pr->pr_Result2 = ERROR_NO_FREE_STORE;
	    }

	    /* Next name */
	    cntr++;
	}

	/* Free the FileInfoBlock */
	FreeDosObject (DOS_FIB, fib);
    }

    return (retval);
}
