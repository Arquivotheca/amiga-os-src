#include <ss/socket.h>
#include <sys/socket.h>
#include <ss/socket_pragmas.h>

struct SocketLibrary *SockBase;

extern int errno ;

main()
{
	int result ;
	int x ;

        SockBase = (struct SocketLibrary *)OpenLibrary("inet:libs/socket.library",0);
        if (SockBase)
        {
			setup_sockets( 3, &errno ) ;
			for( x = 0 ; x < 300 ; x++ )
			{
				result = syslog(2,"this is a syslog test file #   222\n") ;
			}
			cleanup_sockets() ;
			CloseLibrary((struct Library *)SockBase) ;
		}
		else
		{
			printf("can't open socket library\n") ;
		}
}
