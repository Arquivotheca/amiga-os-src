/* FreePathListLVO.c
 *
 */

#include "amigaguidebase.h"

VOID ASM LVOFreePathList (REG (a6) struct AGLib *ag, REG (a0) BPTR p)
{
    BPTR next = p;
    BPTR lock;

    /* Check for NULL */
    if (p == NULL)
	return;

    /* funky bptr-list following! */
    while (p = (BPTR) BADDR (next))
    {
	/* Get the next path node */
	next = ((LONG *) p)[0];

	/* Convert to a lock */
	lock = (BPTR) (((LONG *) p)[1]);

	/* Unlock the lock */
	UnLock (lock);

	/* Free the path entry */
	FreeVec ((APTR) p);
    }
}
