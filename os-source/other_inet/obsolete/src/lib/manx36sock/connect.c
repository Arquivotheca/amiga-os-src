/*
** connect(s, name, len)	- connect name to given socket.
*/

#include "fd_to_fh.h"
#include <socket.h>

connect(s, name, len)
	register int s;
	caddr_t name;
	int	len;
{
	struct connectargs ca;

	ca.errno = 0L;
	GETSOCK(s, ca.fp);
	ca.name = name;
	ca.namelen = len;
	ConnectAsm(&ca);
	errno = ca.errno;

	return errno? -1:0;
}
