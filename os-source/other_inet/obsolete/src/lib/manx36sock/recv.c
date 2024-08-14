/*
** recv - receive data from socket
*/

#include "fd_to_fh.h"
#include <socket.h>

recv(s, buf, len, flags)
	int	s;
	caddr_t	buf;
	int	len;
	int	flags;
{
	struct recvargs ra;

	GETSOCK(s, ra.fp);
	ra.buf = buf;
	ra.len = len;
	ra.flags = flags;
	ra.errno = 0L;
	
	RecvAsm(&ra);
	errno = ra.errno;

	return errno? -1:(int)ra.rval;
}
