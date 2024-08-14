/* deletedir.c */

#include "ram_includes.h"
#include "ram.h"

/* delete a directory and all subdirs - doesn't check protection! */

void
deletedir (node)
	struct node *node;
{
	struct node *next;

	while (node != NULL) {
		next = node->next;

		if (node->type == ST_USERDIR)
			deletedir(node->list);

		freelist(node->list);	/* linked similar to seglists */
		freevec(node);

		node = next;
	}
}
