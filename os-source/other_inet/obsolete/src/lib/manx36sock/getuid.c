/*
 * get uid's
 */

#include <config.h>

int
getuid()
{
	return geteuid();
}

int
geteuid()
{
	register struct config *cf;

	GETCONFIG(cf);
	if(!cf){
		return -1;
	}

	return (int)cf->uid;
}

