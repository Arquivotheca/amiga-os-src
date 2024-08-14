/* -----------------------------------------------------------------------
 * ntoa.c
 *
 * $Locker:  $
 *
 * $Id: ntoa.c,v 1.2 91/08/07 14:17:41 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	ntoa.c,v $
 * Revision 1.2  91/08/07  14:17:41  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/ntoa.c,v 1.2 91/08/07 14:17:41 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/*  Amiga includes  */
#include <exec/exec.h>

/*  Amiga prototypes and pragmas:  */
#ifdef AZTEC_C
#include <functions.h>
#endif
#ifdef LATTICE
#include <proto/all.h>
#include <dos.h>
#endif

/*  Socket library includes:  */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ss/socket.h>

/*  ANSI includes:  */
#include <stdio.h>

struct Library *SockBase;

main()
{

struct hostent *hostent;
struct sockaddr_in *server;
struct sockaddr_in storage;
char name[30] = "cbmvax";

        /*  Open Shared Socket library:
        */
        if(SockBase = OpenLibrary( "inet:libs/socket.library", 0L ))
        {
                setup_sockets( 10, &errno );
        }else
        {
                printf("Can't open socket.library.");
                exit(RETURN_ERROR);
        }

	printf("Testing ntoa().\n");

	server = &storage;
	bzero(server, sizeof(storage));

	if( (server->sin_addr.s_addr = inet_addr(name)) == INADDR_NONE)
	{
	printf("calling gethostbyname\n");
		if( (hostent=gethostbyname(name)) == NULL)
		{
			printf("Can't find a host named '%s'.\n", name);
			goto clean;
		}
	printf("\tGot address -- name='%s' hostent='%s'\thex='%lx' len=%ld\n",name,
		inet_ntoa(*(u_long *)hostent->h_addr), *(long *)(hostent->h_addr), hostent->h_length);

		/*  Found it by name, copy hp into server. */
		memcpy( (char *)&server->sin_addr, (char *)hostent->h_addr,
			hostent->h_length);
	}
	server->sin_family = AF_INET;
	server->sin_port = htons(6969);

	printf("\tGot address -- name='%s' server ='%s'\thex='%lx'\n", name,
		inet_ntoa((u_long)(server->sin_addr.s_addr)), (u_long)server->sin_addr.s_addr);
	printf("\tGot address -- name='%s' hostent='%s'\thex='%lx' len=%ld\n",name,
		inet_ntoa(*(u_long *)hostent->h_addr), *(long *)hostent->h_addr, hostent->h_length);

clean:
        cleanup_sockets();
        CloseLibrary(SockBase);
}

