/*
 * get user mask
 */

#include <config.h>

/*
 * getumask
 */

int
getumask()
{
	register struct config *cf;

	GETCONFIG(cf);
	if(!cf){
		return -1;
	}

	return cf->umask;
}

