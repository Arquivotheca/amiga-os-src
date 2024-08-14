/*
 * main code
 */

#include <exec/types.h>
#include <rpc/types.h>
#include <netdb.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>

#include "nfsd.h"

#include "nfsd_rev.h"

static char *nfsdvers = VERSTAG ;

struct Library *SockBase ;
int MAXSOCKS = 32;

extern BPTR debug_fh ;

struct RxsLib *RexxSysBase;
/* extern int Enable_Abort; */

/* local functions */
void main(void);
void cleanup(int code);
void clean_up(struct RexxMsg *rm);


/*
 * Initialize other modules, and start server routine.  All other initialization
 * will be done by the controlling Rexx script
 */

void main()
{

/*	Enable_Abort = 0;  Manx: we will handle our own ^C */
	RexxSysBase = (struct RxsLib *)OpenLibrary(RXSNAME, 0L);
	if(!RexxSysBase){
		(void)requester(0L," OK ","Could not open %s",RXSNAME);
		exit(RETURN_FAIL);
	}

    if ((SockBase = OpenLibrary( "inet:libs/socket.library", 0L ))==NULL) {
        (void)requester(0L," OK ","%s","Opening of socket library failed.");
        exit(RETURN_FAIL);
    }

    /* initialize for MAXSOCKS sockets */
    setup_sockets( MAXSOCKS, &errno );

	debug("main - startup\n") ;
	init_svc();
	init_time();
	init_io();
	init_pmap();
	svc_run();

	cleanup(RETURN_OK);
}

/*
 * cleanup state in various subsystems, close rexx library and quit.
 */
void cleanup(int code)
{
	clean_svc();
	clean_pmap();
	clean_time();
	clean_bitmap();
	clean_cache();
	clean_map();

	if(RexxSysBase){
		CloseLibrary(RexxSysBase);
		RexxSysBase = 0;
	}

	cleanup_sockets();
	CloseLibrary(SockBase);
	if(debug_fh)
	{
		debug("cleanup()\n") ;
		Close(debug_fh) ;
	}

	exit(code);
}

/*
 * clean_up is called by the Rexx script when it wants the server
 * to terminate.  clean_up() does its own ReplyMsg since this proc never
 * returns to rexx_parse().
 */
void clean_up(struct RexxMsg *rm)
{
	ReplyMsg(rm);
	cleanup(RETURN_OK);
}


/*
 * Catch fprintf from RPC library
 */
void fprintf(f, fmt, a, b, c, d, e)
{
	(void)requester(0L, " OK ", fmt, a, b, c, d, e);
}
