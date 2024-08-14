/*
** setsockopt
*/

#include "fd_to_fh.h"
#include <socket.h>

int 
setsockopt(s, level, optname, optval, optlen)
	int	s, level, optname;
	char	*optval;
	int	optlen;
{
	struct setsockoptargs ssa;

	GETSOCK(s, ssa.fp);
	ssa.errno = 0L;
	ssa.level = level;
	ssa.name = optname;
	ssa.valsize = optlen;	
	ssa.val = optval;
	SetSockOptAsm(&ssa);
	errno = ssa.errno;

	return errno? -1:0;
}

