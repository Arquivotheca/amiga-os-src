/* parent.c */

#include "ram_includes.h"
#include "ram.h"

struct lock *
parentfh (lock,action)
	struct lock *lock;
	LONG action;
{
	struct node *node;

	node = checklock(lock);
	if (node == NULL)
		return FALSE;

	if (action != ACTION_COPY_DIR_FH && action != ACTION_COPY_DIR)
		node = node->back;	/* parent, parentoffh */

	/* these work the same in ramdisk */
	return (node ? getlock(node,SHARED_LOCK) : NULL);
}
			
