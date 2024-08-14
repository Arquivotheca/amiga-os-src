/* makelink.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* success := makelink(lock,string,type,arg)            */
/*                                                      */
/* make a hard or soft link				*/
/********************************************************/

LONG
makelink (lock,string,arg,soft)
	struct lock *lock;
	UBYTE *string;
	LONG arg;	/* may be string or lock */
	LONG soft;
{
	struct node *newnode,*node;
	struct lock *new;
	LONG type;

/****FIX */
/*
kprintf("Makelink of %s, ",string);
if (soft)
kprintf("to %s (soft)\n",arg);
else
kprintf("to lock 0x%lx (%s)\n",arg,
((struct node *)(((struct lock *)BADDR(arg))->lock.fl_Key))->name);
*/
	if (!soft)
	{
		node = checklock((struct lock *) BADDR(arg));
		if (!node)
			return FALSE;	/* bad lock */
	}
#ifndef SOFTLINKS_SUPPORTED
	  else {
		fileerr = ERROR_NOT_IMPLEMENTED;
		return FALSE;
	}
#endif
	/* Don't overwrite! (what about following links?) */
	/* We want to find if something exists so we don't overwrite it */
	/* (unless it's a softlink) */

	newnode = locatenode(lock,string,FALSE);	/* don't follow links */
	if (newnode)
	{
		/* node exists - should we overwrite if both soft? FIX */
		fileerr = ERROR_OBJECT_EXISTS;
		return FALSE;
	}
	/* Either it doesn't exist, in which case create it, or it */
	/* exists, overwrite it (causes old string to be deleted)  */
	new = create(lock,string,EXCLUSIVE_LOCK,FALSE);
	if (!new)
		return FALSE;

	newnode = (struct node *) new->lock.fl_Key;

#ifdef SOFTLINKS_SUPPORTED
	if (soft)
	{
		/* soft link */
		newnode->type = ST_SOFTLINK;

		/* make it a 'file' of data containing the link string */
		write(new,(CPTR) arg,strlen((char *) arg));
	} else
#endif
	{
		/* hard link */
		node = (struct node *) ((struct lock *)BADDR(arg))->lock.fl_Key;
		type = node->type;

		if (type < 0)
			newnode->type = ST_LINKFILE;
		else
			newnode->type = ST_LINKDIR;

		/* keep a single-linked list of links to object */
		newnode->firstlink = node->firstlink;
		node->firstlink    = newnode;

		/* set pointer to real node */
		newnode->list = node;
	}

	freelock(new);

	return DOS_TRUE;
}

/* read the value of a soft link into a buffer  */
/* return -1 for most errors, -2 for not enough space (res2 will have file */
/* size).  String returned is null-terminated.  Returns size of string. */

LONG
readlink (lock,string,buffer,size)
	struct lock *lock;
	UBYTE *string;
	UBYTE *buffer;
	ULONG size;
{
#ifndef SOFTLINKS_SUPPORTED
	fileerr = ERROR_NOT_IMPLEMENTED;
	return FALSE;
#else
	struct node *node;
	LONG is_dir = FALSE;
	LONG res,len;

	/* finddir maintains current_node for us. */
	/* failure is expected, unless it's a file link, in which case */
	/* the findentry call will be needed.  Check for the right error.   */

	/* we make assumptions about how locatenode works here! */
	node = locatenode(lock,string,FALSE);

	if (!node)
	{
		if (fileerr == ERROR_IS_SOFT_LINK)
		{
			is_dir = TRUE;		/* remember... */
			node = current_node;
		} else
			return -1;		/* failure */
	}

	if (node->type != ST_SOFTLINK)
	{
		fileerr = ERROR_OBJECT_WRONG_TYPE;
		return -1;
	}

	/* finddir leaves offset past last dir in path_pos */
	if (is_dir)
	{
		/* must deal with '/' and ':' correctly in the soft link */
		/* code below counts on these assigns! */

		string = &string[path_pos];
		len = strlen(string) + 1;	/* + 1 for slash */
	} else
		len = 0;		/* softlink is last part of string */

	if (node->size + 1 + len >= size)
	{
		/* not enough space */
		/* this may be 1 larger than needed!! (':' stuff) */

		fileerr = node->size + 1 + len;
		return -2;		/* indicates not enough space */
	}

	/* need a lock on it to do a read */
	lock = getlock(node,SHARED_LOCK);
	if (!lock)
		return -1;

	res = read(lock,(CPTR) buffer,size);

	if (res >= 0)
	{
		/* add on the rest of the name to make new full path */
		if (is_dir)
		{
			/* string already points to rest of path */
			/* len = strlen(string)+1 from above */
			if (buffer[res-1] != ':')
				buffer[res++] = '/';

			/* null-terminates for us */
			strcpy(&(buffer[res]),string);
			res += len-1;
		} else {
			/* else no more slashes, so MUST be end of path */
			buffer[res] = '\0';
		}
	}

	freelock(lock);
	return res;
#endif
}
