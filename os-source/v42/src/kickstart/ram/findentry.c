/* findentry.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* res := findentry(dkey,fname,follow_links)            */
/*                                                      */
/* Given a directory key and a filename, this routine   */
/* searches that directory for the specified filename.  */
/* Returns FALSE or TRUE with pointer to file's key in  */
/* the global 'current.node'                            */
/* follow_links = FALSE will return the link entry      */
/* otherwise hard links return the linked object, soft  */
/* links return ERROR_IS_SOFT_LINK.			*/
/********************************************************/

LONG
findentry (dptr,name,follow_links)
	struct node *dptr;
	char *name;
	LONG follow_links;
{
	if ((dptr == NULL) || (dptr->type != ST_USERDIR))
	{
		fileerr = (dptr == NULL ? ERROR_OBJECT_NOT_FOUND :
					  ERROR_OBJECT_WRONG_TYPE);
		return FALSE;
	}

	current_node = dptr->list;

	while (current_node != NULL) {
		/* be case-insensitive */
		if (stricmp(name,current_node->name) == SAME)
		{
			/* Handle hard and soft links */
			if (follow_links)
			{
			    if (current_node->type == ST_LINKDIR ||
			        current_node->type == ST_LINKFILE)
			    {
				/* I store links in 'list' */
				current_node = current_node->list;

#ifdef SOFTLINKS_SUPPORTED
			    } else if (current_node->type == ST_SOFTLINK) {

				fileerr = ERROR_IS_SOFT_LINK;
				return FALSE;
#endif
			    }
			}
			return TRUE;
		}
		current_node = current_node->next;
	}

	fileerr = ERROR_OBJECT_NOT_FOUND;
	return FALSE;
}
