/* locate.c */

#include "ram_includes.h"
#include "ram.h"

struct lock *
locate (lock,str,mode)
	struct lock *lock;
	char *str;
	LONG mode;
{
	struct node *dptr;

	/* find the node to try to lock (following links) */
	dptr = locatenode(lock,str,TRUE);

	return (dptr ? getlock(dptr,mode) : NULL);
}

/* for when we have a lock */

struct node *
locatenode (lock,str,follow_links)
	struct lock *lock;
	char *str;
	LONG follow_links;
{
	struct node *dptr;

	dptr = checklock(lock);
	if (dptr == NULL)
		return NULL;

	return findnode(dptr,str,follow_links);
}

/* for when we have a dptr (node) */

struct node *
findnode (dptr,str,follow_links)
	struct node *dptr;
	char *str;
	LONG follow_links;
{
	char name[MAX_FILENAME+1];

	/* note: readlink() depends on the behavior of this routine (and of */
	/* finddir()).  It assumes current_node is valid if finddir fails.  */

	dptr = finddir(dptr,str,name);
	if (dptr == NULL)
		return NULL;

	if (name[0] != '\0')
	{
		if (!findentry(dptr,name,follow_links))
			return NULL;
		dptr = current_node;
	}

	return dptr;
}
