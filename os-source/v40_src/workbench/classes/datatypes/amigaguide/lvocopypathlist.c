/* CopyPathListLVO.c
 *
 */

#include "amigaguidebase.h"

BPTR ASM LVOCopyPathList (REG (a6) struct AGLib *ag, REG (a0) BPTR p)
{
    BPTR list = NULL;		/* fake terminating node */
    BPTR *liste = &list;	/* ptr to link of previous node */
    LONG *node;
    BPTR lock;

    /* Step through the source list */
    while (p = (BPTR) BADDR (p))
    {
	/* Allocate a path entry */
	if (node = AllocVec (sizeof (struct Path), MEMF_CLEAR))
	{
	    /* Get a pointer to the lock */
	    lock = (BPTR) (((LONG *) p)[1]);

	    /* Duplicate the lock */
	    node[1] = DupLock (lock);

	    /* Add the entry to the destination list */
	    *liste = MKBADDR (node);
	    liste = node;

	    /* Get the next path entry from the source list */
	    p = ((LONG *) p)[0];
	}
	else
	{
	    break;
	}
    }

    return (list);
}
