/****** socket/getsockopt ******************************************
*
*   NAME
*		getsockopt -- get socket options
*
*   SYNOPSIS
*		return = getsockopt( s, level, optname, optval, optlen )
*
*		int getsockopt ( int, int, int, char *, int * ); 
*
*   FUNCTION
*		Returns the option specified by optname for socket s.
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
*		optval	pointer to the buffer which gets the value
*
*		optlen	initially sizeof(optval). reset to proper value on return
*
*		mode		file mode
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
*
******************************************************************************
*
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

