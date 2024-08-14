/* test inetd.h */


#define MAXARGV     5

#include <exec/lists.h>
#include <exec/nodes.h>
#include <devices/timer.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct biltin {
        char    *bi_service;            /* internally provided service name */
        int     bi_socktype;            /* type of socket supported */
        int     (*bi_func)();           /* function serving */
} ;


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
