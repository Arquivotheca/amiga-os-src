/*
** recvfrom - receive data from socket
*/

#include "fd_to_fh.h"
#include <socket.h>

recvfrom(s, buf, len, flags, from, fromlen)
	int	s;
	caddr_t	buf;
	int	len;
	int	flags;
	caddr_t	from;
	int	*fromlen;
{
	struct recvfromargs ra;

	GETSOCK(s, ra.fp);
	ra.buf = buf;
	ra.len = len;
	ra.flags = flags;
	ra.from = from;
	ra.fromlen = fromlen? *fromlen:0;
	ra.errno = 0L;
	
	RecvFromAsm(&ra);
	errno = ra.errno;
	if(fromlen){
		*fromlen = ra.fromlen;
	}

	return errno? -1:(int)ra.rval;
}
