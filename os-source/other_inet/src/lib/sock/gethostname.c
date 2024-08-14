/****** socket/gethostname ******************************************
*
*   NAME
*		gethostname -- get the hostname of your Amiga
*
*   SYNOPSIS
*		return = gethostname( name, length )
*
*		int gethostname (char *, int); 
*
*   FUNCTION
*		Returns a null-terminated hostname in "name".
*
*	INPUTS
*		name	pointer to character array
*
*		length	size of character array
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will be set to the specific error code.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		sethostname()
*
******************************************************************************
*
*/

#include <sys/types.h>
#include <config.h>

gethostname(p, plen)
	register char	*p;
	register int	plen;
{
	register struct config *cf;
	int len;

	*p = 0;
	GETCONFIG(cf);
	if(!cf || !cf->host[0]){
		return -1;
	}

	len = strlen(cf->host) + 1;
	len = MIN(len, plen);
	strncpy(p, cf->host, len);

	return 0;
}

