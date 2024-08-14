/*
** sendmsg() - send data to socket.
*/

#include "fd_to_fh.h"
#include <socket.h>

sendmsg(s, msg, flags)
	int	s;
	caddr_t	msg;
	int	flags;
{
	struct sendmsgargs sa;

	GETSOCK(s, sa.fp);
	sa.msg = msg;
	sa.flags = flags;
	sa.errno = 0L;

	SendMsgAsm(&sa);
	errno = sa.errno;

	return errno? -1:(int)sa.rval;
}
