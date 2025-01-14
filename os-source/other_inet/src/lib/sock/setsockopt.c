/****** socket/setsockopt ******************************************
*
*   NAME
*		setsockopt -- set socket options
*
*   SYNOPSIS
*		return = setsockopt( s, level, optname, optval, optlen )
*
*		int setsockopt ( int, int, int, char *, int * ); 
*
*   FUNCTION
*		Sets the option specified by optname for socket s.
*
*	INPUTS
*		s		socket descriptor
*
*		level	protocol level.  valid levels are
*				IPPROTO_IP		IP options
*				IPPROTO_TCP		TCP options
*				SOL_SOCKET		socket options
*
*		optname	option name
*
*		optval	pointer to the buffer with the new value
*
*		optlen	initially sizeof(optval). reset to new value on return
*
*   RESULT
*		0 is returned if successful.  -1 is returned on error.
*		errno will be set to the specific error code.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		getsockopt()
*
******************************************************************************
*
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

