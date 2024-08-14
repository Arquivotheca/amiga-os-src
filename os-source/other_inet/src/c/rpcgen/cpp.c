/* ---------------------------------------------------------------------------------
 * CPP.C
 *
 * $Locker:  $
 *
 * $Id: cpp.c,v 1.1 90/11/09 13:44:13 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: Hog:Other/inet/src/c/rpcgen/RCS/cpp.c,v 1.1 90/11/09 13:44:13 bj Exp $
 *
 *-----------------------------------------------------------------------------------
 */




/*
** Amiga support routines for GNU CPP
*/

#include <libraries/dos.h>

char *
realloc(ptr, size)
	char	*ptr;
	int	size;
{
	extern char *malloc();
	register char *newp;

	newp = malloc(size);
	if(newp != (char *)0L){
 		bcopy(ptr, newp, size);
	}
	free(ptr);

	return newp;
}

abort()
{
	exit(RETURN_FAIL);
}
