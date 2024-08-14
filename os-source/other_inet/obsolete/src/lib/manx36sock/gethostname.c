/*
 * Unix interface library
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

