/* rename.c */

#include "ram_includes.h"
#include "ram.h"

LONG
rename (from_dir,from_str,to_dir,to_str)
	struct lock *from_dir,*to_dir;
	char *from_str,*to_str;
{
	char to_name[MAX_FILENAME+1],from_name[MAX_FILENAME+1];
	struct node *from_dptr,*to_dptr,*from_node;
	struct lock *lock;
	int samename;

	/* locate original file */
	from_dptr = checklock(from_dir);
	if (from_dptr == NULL)
		return FALSE;

	/* we must return IS_SOFT_LINK errors from finddir */

	from_dptr = finddir(from_dptr,from_str,from_name);
	if (!from_dptr)
		return FALSE;
	if (!findentry(from_dptr,from_name,FALSE))	/* don't follow links!*/
		return FALSE;
	from_node = current_node;

	/* locate position for new file */
	to_dptr = checklock(to_dir);
	if (to_dptr == NULL)
		return FALSE;

	to_dptr = finddir(to_dptr,to_str,to_name);
	if (to_dptr == NULL)
		return FALSE;

	/* allow same name, but different cases */
	samename = (from_dptr == to_dptr &&
		    stricmp(from_name,to_name) == SAME);

	/* can't rename over existing file */
	if (!samename && findentry(to_dptr,to_name,FALSE))
	{
		fileerr = ERROR_OBJECT_EXISTS;
		return FALSE;
	}

	/* if destination already exists, is the destination a directory? */
	if (to_dptr->type < 0)			/* < 0 means file */
	{
		fileerr = ERROR_OBJECT_WRONG_TYPE;
		return FALSE;
	}

	/* is the source a directory?  If so, check for recursive rename */
	if (from_node->type >= 0)
	{
		/* directory - make sure it isn't being renamed into itself */
		struct node *tmp;

		for (tmp = to_dptr->back; tmp; tmp = tmp->back)
		{
			if (tmp == from_node)
			{
				fileerr = ERROR_OBJECT_IN_USE;
				return FALSE;
			}
		}
	}

	/* note I copied the code of rename from BCPL - it does get a shared */
	/* lock, even though the comment says exclusive - FIX! */

	/* get exclusive lock */
	lock = getlock(from_node,SHARED_LOCK);
	if (lock == NULL)
		return FALSE;

	/* check for name in use */
	if (!checkname(to_name))
	{
		goto free_exit;
	}

	/* check for naming self to self, or dir being named into itself */
	if (to_dptr == from_node)
	{
		fileerr = ERROR_OBJECT_IN_USE;
free_exit:	freelock(lock);
		return FALSE;
	}

	/* update the directory change date of the destination and source */
	DateStamp((struct DateStamp *) (to_dptr->date));
	DateStamp((struct DateStamp *) (from_node->back->date));

	/* unlink FROM slot */
	remove_node(from_node);

	/* link into new place */
	from_node->next = to_dptr->list;
	from_node->back = to_dptr;
	to_dptr->list   = from_node;


	strcpy(from_node->name,to_name);

	/* move old notifies to orphaned list */
	rem_notifies(from_node,TRUE);

	/* scan orphan list for nodes under new name and add to node */
	/* also notifies them all */
	find_notifies(from_node,TRUE);

	freelock(lock);

	return DOS_TRUE;
}
