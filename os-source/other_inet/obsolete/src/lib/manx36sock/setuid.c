/*
 * Unix interface compatibility routines.
 */

#include <config.h>

/*
 * User id's.
 */
setuid(u)
	uid_t	u;
{
	return seteuid(u);
}

seteuid(u)
	uid_t	u;
{
	struct config *cf;

	GETCONFIG(cf);
	if(!cf)
		return -1;
		
	cf->uid = u;

	return 0;
}

/*
 * Groups
 */
setgid(g)
	uid_t	g;
{
	setegid(g);
	return 0;
}

setegid(g)
	uid_t	g;
{
	struct config *cf;

	GETCONFIG(cf);
	if(!cf)
		return -1;
		
	cf->gid = g;

	return 0;
}


int
setgroups(num, gid)
	int	num;
	uid_t	*gid;
{
	struct config *cf;
	int i;

	GETCONFIG(cf);
	if(!cf)
		return -1;
		
	for(i = 0; i < num && i < NGROUP; i++){
		cf->gids[i] = gid[i];
	}

	return 0;
}
