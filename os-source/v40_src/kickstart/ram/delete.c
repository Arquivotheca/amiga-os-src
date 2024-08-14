/* delete.c */

#include "ram_includes.h"
#include "ram.h"

/* note: may be called from create to remove an old entry! */

LONG
delete (dptr,string,notify)
	struct node *dptr;
	char *string;
	long notify;		/* whether to notify about deletion */
{
	struct node *node;
	struct lock *lock = NULL;

	/* can't follow links here if we want to delete them! */
	node = findnode(dptr,string,FALSE);	/* don't follow links! */
	if (!node)
		return FALSE;

	/* check if dir empty */
	/* don't do the check unless this is the last link on the file */
	/* FIX! change this if we make links equivalent */
	if (node->type == ST_USERDIR && node->list != NULL)
	{
		fileerr = ERROR_DIRECTORY_NOT_EMPTY;
		return FALSE;
	}

	/* check write protection */
	if (node->prot & FIBF_DELETE)
	{
		fileerr = ERROR_DELETE_PROTECTED;
		return FALSE;
	}

#ifdef SOFTLINKS_SUPPORTED
	if (node->type != ST_SOFTLINK)
#endif
	{
		/* fail if there are any other locks on it */
		lock = getlock(node,EXCLUSIVE_LOCK);
		if (lock == NULL)
			return FALSE;	/* in use (error set by getlock) */

		/* deal with notify nodes */
		/* deleting a hardlink file may cause extra notifies */
		rem_notifies(node,notify);

		/* is it hardlinked? */
		if (node->firstlink &&
		    (node->type == ST_FILE || node->type == ST_USERDIR))
		{
			/* ok, move it to the next link in line */
			/* copies type,list,size,prot,date, and comment */
			node->firstlink->type = node->type;
			node->firstlink->list = node->list;
			node->firstlink->size = node->size;
			node->firstlink->prot = node->prot;
			copydate(node->firstlink->date,node->date);
			node->firstlink->comment = node->comment;

			/* make sure we don't deallocate them */
			node->comment = (void *) node->list = NULL;

			/* revise link ptrs of any chained hardlinks */
			/* also reattach notifies on these files (you get */
			/* one extra notify, sorry).  */
			{
				struct node *n;

				/* don't cause yet another notify */
				find_notifies(node->firstlink,FALSE);

				for (n = node->firstlink->firstlink;
				     n;
				     n = n->firstlink)
				{
					/* make them point at the new master */
					n->list = node->firstlink;

					/* cause notifies to re-attach */
					find_notifies(n,FALSE);
				}
			}

			/* detach this from the nodes that were linked to it */
			node->firstlink = NULL;
		}
	} /* else softlink - never has notifies on it! */

/*    // Update count of blocks used
 *       spaceused := spaceused - ((node!node.size-1)/min.blksize + 1)
 *    // Fix "info lies about ram" bug without screwing up directory deletes - bart
 * //     UNLESS (node!node.type = st.userdir) DO spaceused := spaceused - 1
 * // This line handles all 0 length files and the directories, too - Andy
 *      UNLESS (node!node.size = 0) DO spaceused := spaceused - 1
 */

	/* free the data blocks */
	/* don't free if hard link - FIX if we change links */
	if (node->type != ST_LINKFILE && node->type != ST_LINKDIR)
	{
		/* freelist(0) is safe */
		spaceused -= freelist(node->list);	/* was unloadseg! */
	}

	spaceused -= 1;	/* for the fileheader */

	/* update the change time of the directory it's being removed from */
	/* if we shouldn't notify, don't datestamp - we're being called */
	/* internally. */
	if (notify)
		DateStamp((struct DateStamp *) node->back->date);

	/* remove from structure */
	remove_node(node);

	if (node->comment)
		freevec(node->comment);

	DBUG1("Freeing node at 0x%lx",(LONG)node);
	freevec(node);

	freelock(lock);					/* may be NULL */

	return DOS_TRUE;
}

/* remove a node from it's parent */

void
remove_node (node)
	struct node *node;
{
	struct node *n;

	/* remember we removed something from this directory */
	node->back->delcount++;

	/* evil, but works - counts on next being first field */
	n = (struct node *) &(node->back->list);
	while (n)
	{
		if (n->next == node)
		{
			n->next = node->next;
			break;
		}
		n = n->next;
	}

	if (node->type == ST_LINKFILE || node->type == ST_LINKDIR)
	{
		/* unlink from hardlink list */
		/* node->list points to the thing linked to */
		for (n = node->list; n; n = n->firstlink)
		{
			if (n->firstlink == node)
			{
				n->firstlink = node->firstlink;
				break;
			}
		}
	}
}
