/* locate.c */

#include "ram_includes.h"
#include "ram.h"

struct lock *locate(struct lock *lock,char *str,LONG mode)
{
struct node *dptr;
char name[MAX_FILENAME+1];

	if (dptr=checklock(lock))
	{
		/* note: readlink() depends on the behavior of this routine (and of */
		/* finddir()).  It assumes current_node is valid if finddir fails.  */

		if (dptr=finddir(dptr,str,name))
		{
			if (name[0]!='\0')
			{
				if (findentry(dptr,name)) dptr=current_node;
				else dptr=NULL;
			}
		}
	}

	return (dptr ? getlock(dptr,mode) : NULL);
}
