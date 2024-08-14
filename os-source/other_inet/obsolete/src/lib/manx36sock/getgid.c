/*
** get group ID's
*/

#include <config.h>

int
getgid()
{
	return getegid();
}

int
getegid()
{
	register struct config *cf;

	GETCONFIG(cf);
	if(!cf){
		return -1;
	}

	return cf->gid;
}

getgroups(num, gp)
	int	num, *gp;
{
	register struct config *cf;
	register int i;

	GETCONFIG(cf);
	if(!cf){
		return -1;
	}

	for(i = 0; i < num && i < cf->num_gid; i++){
		gp[i] = cf->gids[i];
	}

	return (int)i;
}

