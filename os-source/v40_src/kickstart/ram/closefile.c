/* closefile.c */

#include "ram_includes.h"
#include "ram.h"

LONG
closefile (lock)
	struct lock *lock;
{
	struct node *node;
	struct data *temp;

	/* optimize small files if possible (less than 1 block) */
	node = (struct node *) lock->lock.fl_Key;
	if (node->size < MIN_BLKSIZE-4 && node->size != 0)
	{
		temp = swapblock((struct data *) node->list,
				 node->size + DATA_EXTRA*sizeof(LONG),
				 node->size + DATA_EXTRA*sizeof(LONG));
		if (temp)
			node->list = (struct node *) temp;
	}

	/* if the file was changed, update the datestamp */
	/* FIX! use stub for DateStamp? (probably not worth it) */
	if (lock->flags & LOCK_MODIFY)
		DateStamp((struct DateStamp *) (node->date));

	/* notify if file has changed is done by freelock */
	return freelock(lock);
}

/* releases oldblock, returns new, fixes all locks */

struct data *
swapblock (oldblock,size,xfersize)
	struct data *oldblock;
	LONG size;		/* new size in bytes */
	LONG xfersize;		/* same or smaller, in bytes */
{
	struct lock *p;
	struct data *newd;

	newd = (struct data *) mygetvec(size);

	if (newd)
	{
		/* copy the relevant portions */
		/* may read past end of source block */
		xfer((char *) oldblock, (char *) newd, xfersize);

		/* now fix any other locks on this file */
		for (p = BADDR(lock_list);
		     p;
		     p = (struct lock *) BADDR(p->lock.fl_Link))
		{
			if (p->block == oldblock)
				p->block = newd;
		}

		/* drop old data and fix pointer */
		freevec(oldblock);
	}
	return newd;
}
