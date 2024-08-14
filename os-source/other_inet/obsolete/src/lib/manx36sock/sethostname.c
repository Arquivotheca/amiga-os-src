/*
 * sethostname.
 */

#include <config.h>

int
sethostname(name, namelen)
	char	*name;
	int	namelen;
{
	struct config *cf;

	if(namelen <= 0){
		return -1;
	}

	GETCONFIG(cf);
	if(!cf)
		return -1;

	if(namelen >= MAXHOSTNAMELEN){
		namelen = MAXHOSTNAMELEN-2;
	}
	strncpy(cf->host, name, namelen);

	return 0;
}
