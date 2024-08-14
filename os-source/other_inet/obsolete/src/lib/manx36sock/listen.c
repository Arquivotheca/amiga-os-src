/*
** listen(s, backlog)	- set listen mode on socket
*/

#include "fd_to_fh.h"
#include <socket.h>

listen(s, backlog)
	register int s, backlog;
{
	struct listenargs la;

	la.errno = 0L;
	GETSOCK(s, la.fp);
	la.backlog = backlog;
	ListenAsm(&la);
	errno = la.errno;

	return errno? -1:0;
}
