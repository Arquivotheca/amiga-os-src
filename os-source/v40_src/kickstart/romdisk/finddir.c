/* finddir.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* ptr := finddir(dlock,searchname,vector)              */
/*                                                      */
/* Given directory node and a pathname (either 'fred',  */
/* 'bill/fred' or '/fred'), this routine will return    */
/* the key of the specified file/directory name if      */
/* present in (or relative to) current dir.             */
/*							*/
/* New feature: always sets current_node, even on fail  */
/* also leaves last ptr value in path_pos.		*/
/********************************************************/

struct node *
finddir (dptr,string,name)
	struct node *dptr;
	char *string;
	char *name;
{
	int ptr,p,len;
	char *temp;

	temp = strchr(string,':');
	if (temp)
		string = temp+1;

	len = strlen(string);

	ptr = 0;	/* for future searches */

	current_node = dptr;
	/* only exited by return */
	while (1) {
		p = SplitName(string,'/',name,(WORD) ptr,MAX_FILENAME+1);
		if (p < 0)
			return dptr;

		path_pos = p;		/* store in global for readlink */

		if (p == ptr+1)		/* Parent() call */
		{
			/* make sure current node is maintained for readlink */
			current_node = dptr = dptr->back;

			if (dptr == NULL)
			{
				/* always set an error on bad exit */
				fileerr = ERROR_OBJECT_NOT_FOUND;
				return NULL;
			}
			/* I think the next two lines can be eliminated - FIX */
			if (p == len)		/* ends in '/' */
				return dptr;

			ptr = p;
			continue;
		} else
			ptr = p;

		if (!findentry(dptr,name)) return NULL;
		dptr = current_node;

	} /* while(1) */
	/*NOTREACHED*/
}
