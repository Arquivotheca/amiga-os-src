/*
** shutdown(s, how)	- shutdown a socket
*/

#include "fd_to_fh.h"
#include <socket.h>

shutdown(s, how)
	register int s, how;
{
	struct shutdownargs sa;

	errno = 0L;
	GETSOCK(s, sa.fp);
	sa.how = how;
	ShutdownAsm(&sa);
	errno = sa.errno;

	return errno? -1:0;
}
