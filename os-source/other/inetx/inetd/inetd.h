/* -----------------------------------------------------------------------
 * inetd.h    header file for shared socket library version of inetd
 *
 * $Locker:$
 *
 * $Id:$
 *
 * $Revision:$
 *
 * $Log:$
 *
 * $Header:$
 *
 *------------------------------------------------------------------------
 */

#define MAXBUFFER 256L

#include <exec/types.h>
#include <exec/libraries.h>
#include <devices/timer.h>
#include <dos/dos.h>

struct glob {
	struct Library     *g_UtilityBase ;
	struct Library     *g_SockBase ;
	struct Library     *g_IntuitionBase ;
	struct MsgPort     *g_TimerPort ;
	struct MsgPort     *g_TagPort ;
	struct List        *g_servtab_list ;
	struct timerequest *g_TimerReq ;
	long g_TimerDevOpen ;
	long g_timerbit ;
	long g_Debug ;
	long g_sockets_are_setup ;
	int  g_errno ;
	} ;

#define MAXARGV     5

int echo_dg(), discard_dg(), machtime_stream(), machtime_dg(),
    daytime_stream(), daytime_dg(), chargen_dg();

struct biltin {
	char    *bi_service;         /* internally provided service name */
	int      bi_socktype;        /* type of socket supported         */
	int     (*bi_func)();        /* function serving                 */
} ;

/***************************************************
} biltins[] = {
	echo",       SOCK_DGRAM,    echo_dg,
	discard",    SOCK_DGRAM,    discard_dg,
	time",       SOCK_STREAM,   machtime_stream,
	time",       SOCK_DGRAM,    machtime_dg,
	daytime",    SOCK_STREAM,   daytime_stream,
	daytime",    SOCK_DGRAM,    daytime_dg,
	chargen",    SOCK_DGRAM,    chargen_dg,
	0
};

**********************************************/

struct  servtab {
		struct Node se_Node ;            /* node to link list together */
        UBYTE   *se_proto;               /* protocol used */
        UBYTE   *se_service;             /* name of service */
        UBYTE   *se_user;                /* user name to run as */
        UBYTE   *se_server;              /* server program */
        UBYTE   *se_argv[MAXARGV+1];     /* program arguments */
        UBYTE   *se_line_array ;         /* ptr to the argv line */
        int     se_socktype;             /* type of socket to use */
        int     se_fd;                   /* open descriptor */
        int     se_count;                /* number started since se_time */
        short   se_wait;                 /* single threaded server */
        short   se_checked;              /* looked at during merge */
        struct  biltin *se_bi;           /* if built-in, description */
        struct  sockaddr_in se_ctrladdr; /* bound address */
        struct  timeval se_time;         /* start of se_count */
} ;



/***** end o file  ****/
