#include	"lharc.h"
#include	<stdlib.h>

#ifndef	DLIST_INCR
# define	DLIST_INCR	20
#endif

char ** Init_List(char ** list)
{
    char	**l;
    
    l = (char **)xcalloc(DLIST_INCR, sizeof(char *));
    
    l[DLIST_INCR - 1] = (char *)~0;
    
    return l;
}

void Free_List(char ** list)
{
    int		i;

    for (i = 0; list[i] != NULL && list[i] != (char *)~0; i++)
	free(list[i]);

    free(list);
}

int Length_List(char ** list)
{
    int		i;

    for (i = 0; list[i] != NULL && list[i] != (char *)~0; i++)
	;
    
    return i;
}

char ** Add_List(char ** list, char * entry)
{
    int		i;
    int		len;
    
    for (i = 0; list[i] != NULL && list[i] != (char *)~0; i++)
	;
    
    if (list[i] != NULL)
    {
	len = i + 1;
        len += DLIST_INCR;
    
	list = (char **)xrealloc((char *)list, len * sizeof(char *));
    
	for (; i < len; i++)
	    list[i] = NULL;
	
	list[len - 1] = (char *)~0;
    }
    
    list[i] = strdup(entry);
    return list;
}
