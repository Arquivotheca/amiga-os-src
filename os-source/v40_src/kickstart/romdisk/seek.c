/* seek.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* ptr := seek(scb, n, wrt)                             */
/*                                                      */
/* code to handle seek on the RAM disk                  */
/********************************************************/

LONG seek(struct lock *lock,LONG dest,LONG frompos)
{
struct node *node;
LONG count;

	node   = (struct node *) lock->lock.fl_Key;
	count  = lock->cpos;

	/* find out where we're going */
	if (frompos == OFFSET_CURRENT) dest += count;
	else if (frompos == OFFSET_END) dest += node->size;
	else if (frompos != OFFSET_BEGINNING) dest = -1;

	/* no seek before start or past end */
	/* except for seek past end for truncate! */
	if (dest < 0 || dest > node->size)
	{
		fileerr = ERROR_SEEK_ERROR;
		count = -1;
	}
	else
	{
		lock->block=(struct data *) node->list;
		lock->offset=dest+FIRSTBUFFPOS;
		lock->cpos=dest;
	}

	return count;	/* return old position in file for Seek() */
}
