#include <ss/socket.h>
#include <sys/socket.h>
#include "socket_pragmas.h"

struct SocketLibrary *SockBase;

main()
{

	int result ;
	int x ;

        SockBase = (struct SocketLibrary *)OpenLibrary("inet:libs/socket.library",0);
        if (SockBase)
        {
			result = reconfig() ;
			CloseLibrary((struct Library *)SockBase) ;
		}
		else
		{
			printf("can't open socket library\n") ;
		}
}
