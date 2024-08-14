/****** socket/getdomainname ******************************************
*
*   NAME
*		getdomainname -- get domain name
*
*   SYNOPSIS
*		return = getdomainname( name, namelen)
*
*		int getdomainname (char *, int); 
*
*   FUNCTION
*		Returns the name of the domain into the pointer specified.
*		Name will be null-terminated if sufficient space is available.
*
*	INPUTS
*		name		pointer to character buffer
*	
*		namelen		space available in name
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
*
******************************************************************************
*
*/

#include <config.h>

char *
getdomainname(buf, size)
	char	*buf;
	int	size;
{
	register struct config *cf;
	int len;
    	
	GETCONFIG(cf);
	if(!cf){
		return (char *)-1;
	}

	len = strlen(cf->domain);
	strncpy(buf, cf->domain, (size < len) ? size:len);

	return (char *)0L;
}
