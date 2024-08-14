/* seek.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* ptr := seek(scb, n, wrt)                             */
/*                                                      */
/* code to handle seek on the RAM disk                  */
/********************************************************/

LONG
seek (lock,dest,frompos,truncate)
	struct lock *lock;
	LONG dest;
	LONG frompos;
	LONG truncate;	/* true/false */
{
	struct node *node;
	struct data *p,*lastp;
	char *mem;
	LONG count,ptr,newcount;

	node   = (struct node *) lock->lock.fl_Key;
	count  = lock->cpos;

	/* find out where we're going */
	if (frompos == OFFSET_CURRENT)		
		dest += count;
	else if (frompos == OFFSET_END)
		dest += node->size;
	else if (frompos == OFFSET_BEGINNING) {
		/*NULL*/ ;
	} else {
		/* bad seek mode or invalid seek - don't change pos! */
seek_error:
		fileerr = ERROR_SEEK_ERROR;
		return -1;
	}

	/* no seek before start or past end */
	/* except for seek past end for truncate! */
	if (dest < 0 || (!truncate && dest > node->size))
		goto seek_error;

	/* we have position in file needed (dest) and current (count) */
	/* NOTE: lock->block cannot be NULL any longer */

	if (dest > count)
	{
		/* going farther into file */
		lastp = p = (struct data *) lock->block;
		ptr   = count - (lock->offset - FIRSTBUFFPOS);
	} else {
		/* going towards beginning of file */
		lastp = p = (struct data *) node->list;
		ptr = 0;
	}

	/* main seek loop */
	while (1) {
		if (p == NULL)
		{
			/* end of file */
			if (!truncate)
				goto seek_error;   /* should NEVER happen */

			/* extend file */
			/* first move to end of file */
			lock->block  = lastp;
			lock->offset = lastp->size + 1;
			lock->cpos   = node->size;

			/* get blank memory */
			mem = AllocMem(DATA_BLOCK_SIZE*4,MEMF_CLEAR);
			if (!mem)
				goto seek_error;

			/* write in datablock-sized chunks */
			/* write will add blocks as needed */
			while (ptr+(DATA_BLOCK_SIZE*4) < dest)
			{
				if (write(lock,(CPTR)mem,(DATA_BLOCK_SIZE*4)) !=
				    (DATA_BLOCK_SIZE*4))
				{
					FreeMem(mem,(DATA_BLOCK_SIZE*4));
					goto seek_error;
				}
				ptr += DATA_BLOCK_SIZE*4;
			}
			/* partial (final) block */
			newcount = write(lock,(CPTR)mem,dest-ptr);
			FreeMem(mem,DATA_BLOCK_SIZE*4);
			if (newcount != dest-ptr)
				goto seek_error;

			/* the write calls leave us at the end of the */
			/* extended file.  Exit with success. */
			return dest;
		}

		/* the meat of seek */
		/* a block with FIRSTBUFFPOS-1 size has no bytes in it */
		newcount = ptr + p->size - (FIRSTBUFFPOS-1);
		if (newcount >= dest)
			break; 		/* it's in this block */

		ptr = newcount;		/* remember size at block beginning */
		lastp = p;		/* remember last data block */
		p = p->next;		/* do next block */
	}

	/* seek successful.  Check for truncate. */
	/* have to check other locks on file since it can be shared! */
	if (truncate)
	{
		struct lock *curr;

		/* only bother with check if file being made smaller */
		if (dest < node->size)
		{
		    /* check for any lockers positioned further in file */
		    for (curr = BADDR(lock_list);
			 curr;
			 curr = BADDR(curr->lock.fl_Link))
		    {
			if (curr != lock)	/* don't check our own lock */
			{
			    if (curr->lock.fl_Key == lock->lock.fl_Key &&
			        curr->cpos > dest)
			    {
				/* it's past us on same file! */
				/* set truncate position to there */
				dest = curr->cpos;
				p = curr->block;
				ptr = dest + FIRSTBUFFPOS - curr->offset;
			    }
			}
		    }
		}

		/* we now know where we're going.  Adjust position and node */

		/* free the blocks */
		spaceused -= freelist((struct node *) p->next);
		p->next    = NULL;
		p->size    = dest-ptr + (FIRSTBUFFPOS-1);
		node->size = dest;

		lock->flags |= LOCK_MODIFY;		/* file was changed */

		/* return new end of file for SetFileSize! */
		count = dest;

		if (lock->cpos > dest)
		{
			/* must force file position to new, smaller size */
			goto do_seek;
		}
	} else {
do_seek:	/* real seek, change file position */
		lock->block  = p;
		lock->offset = dest-ptr + FIRSTBUFFPOS;  /* to next byte */
		lock->cpos   = dest;
	}

	return count;	/* return old position in file for Seek() */
}
