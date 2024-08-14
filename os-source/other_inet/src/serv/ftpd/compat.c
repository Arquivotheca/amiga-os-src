/*
** Compatibility routines for FTPd
*/

#include <stdio.h>

initgroups(name, gid)
	char	*name;
	int	gid;
{
	return 0;
}

chroot(dir)
	char	*dir;
{
	return 0;
}

FILE *
ftpd_popen(prog, type)
	char	*prog, *type;
{
	extern FILE *popen();

	return popen(prog, type);
}

fchown(fd, owner, group)
	int	fd, owner, group;
{
	return 0;
}

