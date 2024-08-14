/* create.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* ptr := create(dir,name,mode,is_dir)                  */
/*                                                      */
/* Create a new entry in the specified directory        */
/* The entry is by default a directory.			*/
/* Gives a lock on the object				*/
/********************************************************/

struct lock *
create (lock,string,mode,is_dir)
	struct lock *lock;
	char *string;
	LONG mode;		/* SHARED_LOCK or EXCLUSIVE_LOCK */
	LONG is_dir;		/* true if creating a directory  */
{
	struct node *node;
	struct node *dptr,*save;
	char name[MAX_FILENAME+1];
	char *comm = NULL;
	LONG prot = 0;

	DBUG1("Creating %s",(LONG)string);

	dptr = checklock(lock);
	if (dptr == NULL)
		return NULL;

	dptr = finddir(dptr,string,name);
	if (dptr == NULL)
		return NULL;
	DBUG1("Found dir 0x%lx",(LONG)dptr);

	if (!checkname(name))
		return NULL;

	/* save comment and protection fields */
	if (findentry(dptr,name,TRUE))
	{
		if (is_dir || current_node->type == ST_USERDIR)
		{
			/* don't delete file on CreateDir! */
			/* don't change dir into a file! */
			fileerr = ERROR_OBJECT_EXISTS;
			return NULL;
		}

		prot = current_node->prot;
		comm = current_node->comment;
		current_node->comment = NULL;	/* so delete won't free it */
		save = current_node;
	}

	/* attempt to delete previous entry */
	/* delete won't update the dir date if notify is FALSE */
	if (!(delete(dptr,name,FALSE)) &&
	    (fileerr != ERROR_OBJECT_NOT_FOUND))
	{
		/* restore comment since delete failed */
		save->comment = comm;
		return NULL;
	}

	DBUG("Removed previous entry");
	node = getvec(sizeof(*node));
	if (!node) {
		/* ack!  couldn't allocate (very unlikely), but file	*/
		/* is already gone.  Oh well.				*/
		freevec(comm);
		return NULL;
	}

	/* increment size used */
	spaceused += 1;

	/* Fill in next back type list size */
	node->next  = dptr->list;
	node->back  = dptr;
	node->type  = ST_USERDIR;
	node->list  = NULL;
	node->firstlink = NULL;
	node->size  = 0;
	node->prot  = prot;
	node->comment = comm;
	NewList((struct List *) &(node->notify_list));
	NewList((struct List *) &(node->record_list));
	NewList((struct List *) &(node->rwait_list));

	/* link in at head */
	dptr->list = node;

	/* set creation date */
	DateStamp((struct DateStamp *) (node->date));

	/* update root modified time (to,from) */
	/* WHY????? */
	/*copydate(root->date,node->date);*/		/* macro */

	/* update parents date */
	/* dptr cannot be null */
	copydate(dptr->date,node->date);	/* macro */

	/* notify anyone waiting that the directory has changed */
	notifynode(dptr);

	/* update directory archive, not that it's useful even on a real disk */
	/* too useless for rom */
	/*dptr->prot &= ~FIBF_ARCHIVE;*/

	strcpy(node->name,name);

	DBUG1("Returning lock on node 0x%lx",(LONG)node);

	/* find notify requests, attach, don't notify yet */
	find_notifies(node,FALSE);

	/* get lock, mark file/dir as modified */
	lock = getlock(node,mode);
	if (lock)
		lock->flags |= LOCK_MODIFY;
	else {
		/* Oops, tried to lock it, failed (probably low mem)    */
		/* Since we're going to return failure, free the thing. */
		/* Update dir change date, since the file is gone.	*/
		delete(dptr,name,TRUE);	/* ignore return code */

		/* we now return lock, which is NULL */
	}

	return lock;
}
