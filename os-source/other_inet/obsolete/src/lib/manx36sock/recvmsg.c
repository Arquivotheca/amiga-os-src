/*
** recvmsg - receive data from socket
*/

#include "fd_to_fh.h"
#include <socket.h>

recvmsg(s, msg, flags)
	int	s;
	caddr_t	msg;
	int	flags;
{
	struct recvmsgargs ra;

	GETSOCK(s, ra.fp);
	ra.msg = (struct msghdr *)msg;
	ra.flags = flags;
	ra.errno = 0L;
	
	RecvMsgAsm(&ra);
	errno = ra.errno;

	return errno? -1:(int)ra.rval;
}
