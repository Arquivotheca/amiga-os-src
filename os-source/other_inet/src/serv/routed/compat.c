/*
** Compatibility routines
*/

#include <fcntl.h>

/*
** Not really "right" in that fd's get Close()'ed twice.
*/
dup2(fd, alias)
{
	/* FILL ME IN */
}

sigmask()
{
}

sigblock()
{
}

sigsetmask()
{
}
