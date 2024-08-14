/* file.c */

#include "ram_includes.h"
#include "ram.h"

/* Routine for 'findoutput' and 'findinput' (and findupdate)   */
/* ram.h has defines for openfile and makefile */

LONG
openmakefile (scb,dlock,string)
	struct FileHandle *scb;
	struct lock *dlock;		/* directory lock */
	char *string;
{
	struct lock *lock;
	struct node *node;

	/* readwrite opens shared, but creates if not existant */
	lock = locate(dlock,string,SHARED_LOCK);

	if (lock)
	{
		node = (struct node *) lock->lock.fl_Key;

		/* all lock fields are already set up by getlock */

		if (node->type != ST_FILE)
		{
			freelock(lock);
			fileerr = ERROR_OBJECT_WRONG_TYPE;
			lock=NULL;
		}
	}

	if (lock)
	{
		/* set up the file handle */
		scb->fh_Arg1 = ((LONG)lock) >> 2;
		return DOS_TRUE;
	}

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

	return DOS_TRUE;
}
