/* transfer.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* nbytes := transfer(lock,buf,bsize,readflag)          */
/*                                                      */
/* General read/write routine                           */
/********************************************************/

#ifdef DOWN_CODED
LONG
read (lock,buf,bsize)
	struct lock *lock;
	CPTR buf;
	LONG bsize;
{
	struct data *pos;
	struct data *next;
	LONG offset;
	LONG max,count;
	LONG avail,needed,tsize;

	/* get data about the file */
	pos    = (struct data *) lock->block;		/* current block */
	offset = lock->offset;		/* offset of next valid position */

	if (pos == NULL)
		return 0;

	max = pos->size;

	/* number of bytes read */
	count  = 0;
	needed = bsize;

	while (needed > 0) {
		avail  = max - offset + 1;	/* space in this block */
		if (avail <= 0)			/* next block needed */
		{
			next = pos->next;
			if (next == NULL)	/* new block needed */
			{
				break;	/* EOF */
			}
			pos    = next;
			max    = pos->size;
			offset = FIRSTBUFFPOS;
			continue;
		}
		tsize = (avail < needed ? avail : needed);

		/* src, dest,size (bytes) */
		xfer(((char *) pos) + offset,
		     ((char *) buf) + count, tsize);

		count  += tsize;
		offset += tsize;
		needed -= tsize;
	}

	/* save end positions */
	lock->block  = (struct node *) pos;
	lock->offset = offset;
	lock->cpos   += count;

	return count;	/* number of bytes read/written */
}
#endif

LONG
write (lock,buf,bsize)
	struct lock *lock;
	CPTR buf;
	LONG bsize;
{
	struct node *node;
	struct data *pos;
	struct data *next;
	LONG offset,curpos;
	LONG max,count;
	LONG avail,needed,tsize;

	/* get data about the file */
	/* NOTE: now that makefile() in file.c forces all data files to have */
	/* a non-null node->list, we can no longer have a null lock->block.  */

	node   = (struct node *) lock->lock.fl_Key;
	pos    = (struct data *) lock->block;		/* current block */
	offset = lock->offset;		/* offset of next valid position */
	curpos = lock->cpos;			/* current file position */

	max = MAX_WRITE;

	/* number of bytes read or written */
	count = 0;

	/* deal with "short block" files */
	/* short blocks can only be the first block of a file! */
	if (pos == (struct data *) node->list)
	{
		/* replace short block (if it is) with regular one */
		pos = growblock(pos,node);
		if (pos == NULL)
			return -1;
	}

	while (count < bsize) {
		needed = bsize - count;		/* number still to be r/w */
		avail  = max - offset + 1;	/* space in this block */
		if (avail <= 0)			/* next block needed */
		{
			next = pos->next;
			if (next == NULL)	/* new block needed */
			{
				next = (struct data *) 
					mygetvec(sizeof(struct data));
				if (next == NULL)
				{
					count = -1;	/* gets returned */
					goto exit;	/* fix fields & exit */
				}

				next->next = NULL;
				next->size = FIRSTBUFFPOS - 1;
				pos->next  = next;

				/* update spaceused */
				spaceused += 1;
			}
			pos    = next;
			max    = MAX_WRITE;
			offset = FIRSTBUFFPOS;
			avail  = max - FIRSTBUFFPOS + 1;
		}
		tsize = (avail < needed ? avail : needed);

		/* src, dest,size (bytes) */
		xfer(((char *) buf) + count,
		     ((char *) pos) + offset, tsize);

		count  += tsize;
		offset += tsize;
		curpos += tsize;

		/* update number of bytes available in current block if needed*/
		if (offset > pos->size)
			pos->size = offset-1; /* max valid byte */
	}

	/* save end positions */
	lock->block  = pos;
	lock->offset = offset;
	lock->cpos   = curpos;

exit:
	/* update file size if needed, modify flag */
	lock->flags |= LOCK_MODIFY;

	if (curpos > node->size)
		node->size = curpos;

	return count;	/* number of bytes read/written */
}


/* deal with short files */
/* nasty equation to get size of block - note '-1' is in longs */
/* +4 is for the requirements of AllocVec (size + 1 lw) */
/* can only be the first block of a 1-block file! */

struct data *
growblock (p,node)
	struct data *p;
	struct node *node;
{
	if (*(((long *) p) - 1) != sizeof(struct data) + 4)
	{
		/* have to grow a compressed block */
		p = swapblock(p,sizeof(struct data),
			      node->size + DATA_EXTRA*sizeof(LONG));
		if (p)
			node->list = (struct node *) p;
	}
	return p;
}
