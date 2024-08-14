/*
** New interface socklib
*/

#include <sys/types.h>
#include <netinet/inet.h>
#include <errno.h>

extern void *OpenLibrary();
struct InetBase *InetBase;

#define LONGINT (sizeof(int) == sizeof(long))
static char *inet_lib = "work:src/l/inet.library/src/inet.library";

socket(af, type, protocol)
	int	af, type, protocol;
{
	if(!InetBase){
		InetBase = (struct InetBase *)OpenLibrary(inet_lib, 0L);
		if(!InetBase){
			errno = ENETDOWN;
			return -1;
		}
	}
		
	return ASMsocket((long)af, (long)type, (long)protocol);
}

