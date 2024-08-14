/*
** getsockopt - get socket options settings
*/

#include "fd_to_fh.h"
#include <socket.h>

int 
getsockopt(s, level, optname, optval, optlenp)
	int	s, level, optname;
	char	*optval;
	int	*optlenp;
{
	struct getsockoptargs gsa;

	GETSOCK(s, gsa.fp);
	gsa.errno = 0L;
	gsa.level = level;
	gsa.name = optname;
	gsa.val = optval;
	gsa.valsize = optlenp? *optlenp:0;
	GetSockOptAsm(&gsa);
	errno = gsa.errno;
	if(optlenp){
		*optlenp = gsa.valsize;
	}

	return errno? -1:0;
}

