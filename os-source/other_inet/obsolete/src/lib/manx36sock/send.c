/*
** send - send data to specified socket
*/

#include "fd_to_fh.h"
#include <socket.h>

send(s, buf, len, flags)
	int	s;
	caddr_t	buf;
	int	len;
	int 	flags;
{
	struct sendargs sa;

	GETSOCK(s, sa.fp);
	sa.buf = buf;
	sa.len = len;
	sa.flags = flags;
	sa.errno = 0l;
	SendAsm(&sa);
	errno = sa.errno;

	return errno? -1:(int)sa.rval;
}
