/*
 * Get domain name
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
		return -1;
	}

	len = strlen(cf->domain);
	strncpy(buf, cf->domain, (size < len) ? size:len);

	return (char *)0L;
}
