/* file.c */

#include "ram_includes.h"
#include "ram.h"

/* Routine for 'findoutput' and 'findinput' (and findupdate)   */
/* ram.h has defines for openfile and makefile */

LONG
openmakefile (scb,dlock,string,type)
	struct FileHandle *scb;
	struct lock *dlock;		/* directory lock */
	char *string;
	int type;		/* 0 = makefile, 1 = openfile, 2 = readwrite */
{
	struct lock *lock;
	struct node *node;
	struct data *data = NULL;
	char created = FALSE;

	if (type == 0)	/* newfile */
	{
		data = mygetvec(sizeof(struct data));
		if (!data) return FALSE;

		lock = create(dlock,string,EXCLUSIVE_LOCK,FALSE);
		created = TRUE;
	} else {
		/* readwrite opens shared, but creates if not existant */
		lock = locate(dlock,string,SHARED_LOCK);

		/* don't try to create if it's already locked */
		if (!lock && type == 2 &&
		    fileerr == ERROR_OBJECT_NOT_FOUND)
		{
			data = mygetvec(sizeof(struct data));
			if (!data)
				return FALSE;

			lock = create(dlock,string,SHARED_LOCK,FALSE);
			created = TRUE;
		}
	}

	if (lock == NULL)
		goto failure;

	DBUG1("Open: got lock 0x%lx:",(LONG) lock);

	node = (struct node *) lock->lock.fl_Key;
	DBUG1("Open: got node 0x%lx:",(LONG) node);

	/* all lock fields are already set up by getlock */

	if (created) {
		/* make new file */
		node->type  = ST_FILE;
		lock->flags = LOCK_MODIFY;

	} else if (node->type != ST_FILE) {

		freelock(lock);
		fileerr = ERROR_OBJECT_WRONG_TYPE;
		goto failure;
	}

	if (data)	/* means we had to create() the file */
	{
		/* create 0-length data for file (close will shrink it) */
		node->list  = (struct node *) data;
		data->next  = NULL;
		data->size  = FIRSTBUFFPOS - 1;
		lock->block = data;		/* lock gotten above */
		spaceused++;
	}

	/* set up the file handle */
	scb->fh_Arg1 = ((LONG)lock) >> 2;

	/* DateStamp(root->date); */	/* huh??? */

	return DOS_TRUE;

failure:
	if (data)
		freevec(data);
	return FALSE;
}

LONG
openfromlock (scb,lock)
	struct FileHandle *scb;
	struct lock *lock;		/* lock on object */
{
	struct node *node;

	/* all lock fields are already set up by getlock */
	node = (struct node *) lock->lock.fl_Key;

	if (node->type != ST_FILE)
	{
		fileerr = ERROR_OBJECT_WRONG_TYPE;
		return FALSE;
	}

	/* set up the file handle */
	scb->fh_Arg1 = ((LONG)lock) >> 2;

	/* DateStamp(root->date); */	/* huh??? */

	return DOS_TRUE;
}
