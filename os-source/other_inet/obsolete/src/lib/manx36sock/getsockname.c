/*
** getsockname(s, name, lenp)	- get socket name
*/

#include "fd_to_fh.h"
#include <socket.h>

getsockname(s, name, lenp)
	register int s;
	caddr_t name;
	int	*lenp;
{
	struct getsocknameargs gsa;

	gsa.errno = 0L;
	GETSOCK(s, gsa.fp);
	gsa.asa = name;
	gsa.len = lenp? *lenp:0;
	GetSockNameAsm(&gsa);
	errno = (int)gsa.errno;
	if(lenp){
		*lenp = gsa.len;
	}

	return errno? -1:0;
}
