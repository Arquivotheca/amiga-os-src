/* Copyright (C) 1986 by Manx Software Systems, Inc. */

#include <stdio.h>
#include <errno.h>

perror(s)
	char *s;
{
	if (errno < 0 || errno > sys_nerr){
		return -1;
	}
	if (s){
		fprintf (stderr, "%s: ", s);
	}
	fprintf (stderr, "%s\n", sys_errlist[errno]);
	return 0;
}

char *
strerror(num)
	int	num;
{
	static char buf[32];

	if (errno < 0 || errno > sys_nerr){
		sprintf(buf, "Unknown error %d", num);
		return buf;
	}

	return sys_errlist[num];
}
