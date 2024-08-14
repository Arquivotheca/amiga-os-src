/*
** Send data to socket
*/

#include "fd_to_fh.h"
#include <socket.h>

sendto(s, buf, len, flags, to, tolen)
	int	s;
	caddr_t	buf;
	int	len;
	int	flags;
	caddr_t	to;
	int	tolen;
{
	struct sendtoargs sa;

	GETSOCK(s, sa.fp);
	sa.buf = buf;
	sa.len = len;
	sa.flags = flags;
	sa.tolen = tolen;
	sa.to = to;
	sa.errno = 0L;

	SendToAsm(&sa);
	errno = sa.errno;

	return errno? -1:(int)sa.rval;
}
