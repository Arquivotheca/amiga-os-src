#include <exec/libraries.h>

struct Library *InetBase ;

main()
{
	InetBase = (struct Library *)OpenLibrary("inet:libs/inet.library",0L) ;
	if( InetBase )
		{
		printf("inetbase = %08lx\n", InetBase) ;
		CloseLibrary( InetBase ) ;
		}
	else
		{
		printf("Failed!\n") ;
		}
}
