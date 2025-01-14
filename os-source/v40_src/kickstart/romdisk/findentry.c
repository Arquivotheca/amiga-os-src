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

LONG findentry(struct node *dptr,char *name)
{
	if ((dptr == NULL) || (dptr->type != ST_USERDIR))
	{
		fileerr = (dptr == NULL ? ERROR_OBJECT_NOT_FOUND :
					  ERROR_OBJECT_WRONG_TYPE);
		return FALSE;
	}

	current_node = dptr->list;

	while (current_node != NULL)
	{
		/* be case-insensitive */
		if (stricmp(name,current_node->name) == SAME)
		{
			return TRUE;
		}
		current_node = current_node->next;
	}

	fileerr = ERROR_OBJECT_NOT_FOUND;
	return FALSE;
}
