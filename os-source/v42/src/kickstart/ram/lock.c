/* lock.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* key := checklock( lock )                             */
/*                                                      */
/* Returns key from specified lock                      */
/********************************************************/

struct node *
checklock (lock)
	struct lock *lock;
{
	struct lock *p;
	BPTR *last = NULL;		/* ptr to last lock ptr */

	if (lock == NULL)		/* zero lock */
		return root;

	/* keeps last pointing at the pointer to the current node */
	for (p = BADDR(lock_list);
	     p && p != lock;
	     last = &(p->lock.fl_Link), p = BADDR(p->lock.fl_Link))
		; /* NULL */

	if (p == NULL)
	{
		fileerr = ERROR_INVALID_LOCK;
		return FALSE;
	}

	/* we found it, it's a real lock.  Move it to the head of the list */
	if (last)
	{
		/* only if it's not the head already */
		*last = p->lock.fl_Link;	/* remove it */
		p->lock.fl_Link = lock_list;	/* attach the rest to it */
		lock_list = TOBPTR(p);		/* replace the list head ptr */
	}

	return (struct node *) lock->lock.fl_Key;
}

struct lock *
getlock (ptr,access)
	struct node *ptr;
	LONG access;
{
	struct lock *p,*lock;

	/* search for exclusive locks on object, or any locks if */
	/* we want an exclusive lock ourselves.			 */
	/* assumes no shared locks if there is also an exclusive */

	p = BADDR(lock_list);
	p = find_lock(p,ptr);
	if (p != NULL && (p->lock.fl_Access == EXCLUSIVE_LOCK ||
				     access == EXCLUSIVE_LOCK))
	{
		fileerr = ERROR_OBJECT_IN_USE;
		return NULL;
	}

	lock = getvec(sizeof(struct lock));
	if (!lock)
		return NULL;

	/* add to head of lock list */
	lock->lock.fl_Link   = lock_list;
	lock->lock.fl_Key    = (LONG) ptr;
	lock->lock.fl_Access = access;
	lock->lock.fl_Task   = MyPort;
	lock->lock.fl_Volume = ((LONG) volumenode) >> 2;

	/* so others don't have to do this all over the place */
	/* NOTE: for a file, block CANNOT be null any more    */

	lock->block  = (struct data *) ptr->list;   /* may also be nodelist */
	lock->offset = FIRSTBUFFPOS;
	lock->cpos   = 0;
	lock->flags  = 0;

	lock_list = TOBPTR(lock);

	return lock;
}

LONG
freelock (lock)
	struct lock *lock;
{
	struct lock *p;

	if (lock == NULL)
		return DOS_TRUE;

	/* evil magic, clone of such in BCPL version. */
	/* Luckily, fl_Link is the first field of a lock */

	p = (struct lock *) &(lock_list);
	while (p->lock.fl_Link != NULL && BADDR(p->lock.fl_Link) != lock)
		p = (struct lock *) BADDR(p->lock.fl_Link);

	if (p->lock.fl_Link == NULL)
	{
		fileerr = ERROR_INVALID_LOCK;
		return FALSE;
	}

	/* remove from list */
	p->lock.fl_Link = lock->lock.fl_Link;

	/* do we need to notify on this lock? */
	/* (Must happen AFTER removing lock from list, for copy_data to work) */
	/* (Copy_Data needs to get a lock, and if this is exclusive, ...)     */
	if (lock->flags & LOCK_MODIFY)
	{
		notify(lock);
		/* clear archive bit, as if backup programs are used on ram.. */
		((struct node *) lock->lock.fl_Key)->prot &= ~FIBF_ARCHIVE;
	}

	/* null out fl_Task to avoid stale lock problems */
	lock->lock.fl_Task = NULL;

	freevec(lock);

	return DOS_TRUE;
}

struct lock *
find_lock (p,node)
	struct lock *p;
	struct node *node;
{
	while (p != NULL && p->lock.fl_Key != (BPTR) node)
		p = (struct lock *) BADDR(p->lock.fl_Link);

	return p;
}

LONG
change_lock (lock,newmode)
	struct lock *lock;
	LONG newmode;
{
	struct lock *p;

	/* make sure it's mine */
	if (!lock || !checklock(lock))
		return FALSE;

	/* if he has exclusive, give him anything he wants */
	if (newmode == EXCLUSIVE_LOCK &&
	    lock->lock.fl_Access != EXCLUSIVE_LOCK)
	{
		/* he has a shared lock, must be trying for an exclusive lock */
		for (p = BADDR(lock_list);
		     p;
		     p = (struct lock *) BADDR(p->lock.fl_Link))
		{
			/* if going for exclusive, allow no other locks */
			if (p->lock.fl_Key == lock->lock.fl_Key &&
			    p != lock)
			{
				fileerr = ERROR_OBJECT_IN_USE;
				return FALSE;
			}
		}
	}
lock_ok:
	lock->lock.fl_Access = newmode;
	return DOS_TRUE;
}
