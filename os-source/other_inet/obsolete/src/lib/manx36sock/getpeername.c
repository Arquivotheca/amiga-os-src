/*
** getpeername(s, name, lenp)	- get peer name
*/

#include "fd_to_fh.h"
#include <socket.h>

getpeername(s, name, lenp)
	register int s;
	caddr_t name;
	int	*lenp;
{
	struct getpeernameargs gpa;

	gpa.errno = 0L;
	gpa.asa = name;
	gpa.len = lenp? *lenp:0;
	GETSOCK(s, gpa.fp);
	GetPeerNameAsm(&gpa);
	errno = gpa.errno;
	if(lenp){
		*lenp = gpa.len;
	}

	return errno? -1:0;
}
