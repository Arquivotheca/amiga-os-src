/* -----------------------------------------------------------------------
 * inetd.c   manx36
 *
 * $Locker:  $
 *
 * $Id: inetd.c,v 1.1 90/11/12 17:07:59 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/serv/inetd/RCS/inetd.c,v 1.1 90/11/12 17:07:59 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Copyright 1989 Ameristar Technology, Inc.
 */

/*
 * Inetd - Internet super-server
 *
 * This program invokes all internet services as needed.
 * connection-oriented services are invoked each time a
 * connection is made, by creating a process.  This process
 * is passed the connection as file descriptor 0 and is
 * expected to do a getpeername to find out the source host
 * and port.
 *
 * Datagram oriented services are invoked when a datagram
 * arrives; a process is created and passed a pending message
 * on file descriptor 0.  Datagram servers may either connect
 * to their peer, freeing up the original socket for inetd
 * to receive further messages on, or ``take over the socket'',
 * processing all arriving datagrams and, eventually, timing
 * out.	 The first type of server is said to be ``multi-threaded'';
 * the second type of server ``single-threaded''. 
 *
 * Inetd uses a configuration file which is read at startup
 * and, possibly, at some later time in response to a hangup signal.
 * The configuration file is ``free format'' with fields given in the
 * order shown below.  Continuation lines for an entry must being with
 * a space or tab.  All fields must be present in each entry.
 *
 *	service name			must be in /etc/services
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			must be in /etc/protocols
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (5)
 *
 * Comment lines are indicated by a `#' in column 1.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libraries/dosextens.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>
#include <pwd.h>
#include <stdio.h>
#include <strings.h>

#include <libraries/dos.h>

#include "message.h"
#include "inetd_rev.h"

#define	TOOMANY		40		/* don't start more than TOOMANY */
#define	CNT_INTVL	60		/* servers in CNT_INTVL sec. */
#define	RETRYTIME	(60*10)		/* retry after bind or server fail */

static char *mcompiler = "manx36" ;
static char *inetdvers = VERSTAG ;

extern	int errno;

int	retry(), config(), quit();
char	*index();
char	*malloc();

int	debug = 0;
int	nsock, maxsock;
fd_set	allsock;
int	options;
int	timingout;
struct	servent *sp;
extern int _alarm_timerbit;

struct Process *FindTask() ;
APTR DeviceProc() ;

#define MAXARGV 5
struct	servtab {
	char	*se_service;		/* name of service */
	int	se_socktype;		/* type of socket to use */
	char	*se_proto;		/* protocol used */
	short	se_wait;		/* single threaded server */
	short	se_checked;		/* looked at during merge */
	char	*se_user;		/* user name to run as */
	struct	biltin *se_bi;		/* if built-in, description */
	char	*se_server;		/* server program */
	char	*se_argv[MAXARGV+1];	/* program arguments */
	int	se_fd;			/* open descriptor */
	struct	sockaddr_in se_ctrladdr;/* bound address */
	int	se_count;		/* number started since se_time */
	struct	timeval se_time;	/* start of se_count */
	struct	servtab *se_next;
} *servtab;

int	echo_dg(), discard_dg(), machtime_stream(), machtime_dg(),
	daytime_stream(), daytime_dg(), chargen_dg();

struct biltin {
	char	*bi_service;		/* internally provided service name */
	int	bi_socktype;		/* type of socket supported */
	int	(*bi_func)();		/* function serving */
} biltins[] = {
	"echo",		SOCK_DGRAM,	echo_dg,
	"discard",	SOCK_DGRAM,	discard_dg,
	"time",		SOCK_STREAM,	machtime_stream,
	"time",		SOCK_DGRAM,	machtime_dg,
	"daytime",	SOCK_STREAM,	daytime_stream,
	"daytime",	SOCK_DGRAM,	daytime_dg,
	"chargen",	SOCK_DGRAM,	chargen_dg,
	0
};

#define PORTNAME "Inetd_Rendezvous" 

#define NUMINT	(sizeof(intab) / sizeof(struct inent))
char	*CONFIG = INETDFILE;

char msgbuff[256] ;
struct MsgPort *tagport = NULL, *CreatePort() ;

main(argc, argv)
	int argc;
	char *argv[];
{
	register struct servtab *sep;
	extern char *optarg;
	long ev, timerevent;
	extern int optind;
    	fd_set readable;
    	int n, ctrl, ch;
	extern int Enable_Abort;
	struct Process *me = NULL ;
		
	if( FindPort(PORTNAME) ) {
		Message( "Inetd deamon already running...", WINDOW_ERROR_DISPLAY) ;
		exit( 5 ) ;
		}

	if((tagport = CreatePort(PORTNAME, 0L)) == NULL ) {
		Message( "Can't create inetd port...", WINDOW_ERROR_DISPLAY) ;
		exit( 5 ) ;
		}

	me = FindTask( 0L ) ;
	me->pr_ConsoleTask = DeviceProc("RAM:") ;


    Enable_Abort = 0 ;
    	while ((ch = getopt(argc, argv, "d")) != EOF){
		switch(ch) {
		case 'd':
		case 'D':
			debug = 1;
			options |= SO_DEBUG;
			break;
		case '?':
		default:
			Message( "usage: inetd [-d]", WINDOW_ERROR_DISPLAY );
			quit(0) ;
		}
	}

	argc -= optind;	argv += optind;
	if (argc > 0){
		CONFIG = argv[0];
	}

	timerevent = 1L << _alarm_timerbit;
	signal(SIGALRM, retry);
	signal(SIGHUP, config);
	signal(SIGINT, quit);
	config();

for (;;){
	readable = allsock;
	ev = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F | timerevent;
    	n = selectwait(maxsock + 1, &readable, 0L, 0L, 0L, &ev);
	if(ev & SIGBREAKF_CTRL_C){
/*		take_signal(SIGINT);      *** NO CTRL_C for now 8-24-90 Manx _detab[] !! */
	}
	if(ev & SIGBREAKF_CTRL_D){
		take_signal(SIGHUP);
	}
	if(ev & SIGBREAKF_CTRL_F){
#ifdef DEBUG	
		if( debug )
			Message(" ", WINDOW_CLOSE ) ;
#endif			
	}
	if(ev & timerevent){
		take_signal(SIGALRM);
	}
	if(n < 0){
		if(errno == EBADF){
			Message("selectwait\n", WINDOW_OPEN ) ;
			quit(0);
		}	
		continue;
	}

    	for (sep = servtab; n && sep; sep = sep->se_next){
    		if (FD_ISSET(sep->se_fd, &readable)) {
			n--;
			if (debug){
				sprintf(msgbuff, "someone wants %s\n", sep->se_service);
				Message( msgbuff, WINDOW_OPEN) ;
			}
			if(sep->se_socktype == SOCK_STREAM){
				ctrl = accept(sep->se_fd, 0L, 0L);
				if (debug){
					sprintf(msgbuff, "accept, ctrl %d\n", ctrl);
					Message( msgbuff, WINDOW_OPEN ) ;
				}
				if (ctrl < 0) {
					if (errno == EINTR){
						continue;
					}
					syslog(LOG_WARNING, "accept: %m");
					continue;
				}
			} else {
				ctrl = sep->se_fd;
			}
			startit(sep, ctrl);
		}
	}
}
}

/*
 * quit - make server quit
 */
quit( val )
int val ;
{
	if( tagport )
		DeletePort(tagport) ;
	if( debug )
		Message("Termination!\n", WINDOW_ERROR_DISPLAY) ;
	exit( val );
}

/*
 * startit - startup given server.
 */
startit(sep, ctrl)
	register struct servtab *sep;
	int ctrl;
{
	static char cmdbuf[256];
	static char port[32];
	static int id;
	int i;

	if (sep->se_bi){
		(*sep->se_bi->bi_func)(ctrl, sep);
		close(ctrl);
	} else {
		sprintf(port, "%s%06x", sep->se_service, ++id);
		sprintf(cmdbuf, "run >nil: %s %s ", sep->se_server, port);
		for(i = 0; sep->se_argv[i]; i++){
			strcat(cmdbuf, sep->se_argv[i]);
			strcat(cmdbuf, " ");
		}
		Execute(cmdbuf, 0L, 0L);
		if(daemon_startupmsg(port, ctrl) < 0){
			if(debug){
				sprintf(msgbuff, "inetd: Could not start %s\n", 
							sep->se_service);
				Message(msgbuff, WINDOW_OPEN) ;		
			}
		}
	}

	/*
	 * By convention datagram sockets are closed by the daemon which
	 * we've just started up, so we need to set up the listener process
	 * again.
	 */
	if(sep->se_socktype == SOCK_DGRAM){
		FD_CLR(sep->se_fd, &allsock);
		setup(sep);
	}
}

config()
{
	register struct servtab *sep, *cp, **sepp;
	struct servtab *getconfigent(), *enter();
	long omask;

	if (!setconfig()) {
		syslog(LOG_ERR, "%s: %m", CONFIG);
		return;
	}
	for (sep = servtab; sep; sep = sep->se_next){
		sep->se_checked = 0;
	}

	while (cp = getconfigent()) {
		for (sep = servtab; sep; sep = sep->se_next){
			if (strcmp(sep->se_service, cp->se_service) == 0 &&
			    strcmp(sep->se_proto, cp->se_proto) == 0){
				break;
			}
		}
		if (sep != 0) {
			int i;

			if (cp->se_bi == 0){
				sep->se_wait = cp->se_wait;
			}
#define SWAP(a, b) { char *c = a; a = b; b = c; }
			if (cp->se_user){
				SWAP(sep->se_user, cp->se_user);
			}
			if (cp->se_server){
				SWAP(sep->se_server, cp->se_server);
			}
			for (i = 0; i < MAXARGV; i++){
				SWAP(sep->se_argv[i], cp->se_argv[i]);
			}
			freeconfig(cp);
			if (debug){
				print_service("REDO", sep);
			}
		} else {
			sep = enter(cp);
			if (debug){
				print_service("ADD ", sep);
			}
		}
		sep->se_checked = 1;
		sp = getservbyname(sep->se_service, sep->se_proto);
		if (sp == 0) {
			syslog(LOG_ERR, "%s/%s: unknown service",
			    sep->se_service, sep->se_proto);
			continue;
		}
		if (sp->s_port != sep->se_ctrladdr.sin_port) {
			sep->se_ctrladdr.sin_port = sp->s_port;
			if (sep->se_fd != -1){
				(void) close(sep->se_fd);
			}
			sep->se_fd = -1;
		}
		if (sep->se_fd == -1){
			setup(sep);
		}
	}
	endconfig();
	/*
	 * Purge anything not looked at above.
	 */
	sepp = &servtab;
	while (sep = *sepp) {
		if (sep->se_checked) {
			sepp = &sep->se_next;
			continue;
		}
		*sepp = sep->se_next;
		if (sep->se_fd != -1) {
			FD_CLR(sep->se_fd, &allsock);
			nsock--;
			(void) close(sep->se_fd);
		}
		if (debug){
			print_service("FREE", sep);
		}
		freeconfig(sep);
		free((char *)sep);
	}
}

retry()
{
	register struct servtab *sep;

	timingout = 0;
	for (sep = servtab; sep; sep = sep->se_next){
		if (sep->se_fd == -1){
			setup(sep);
		}
	}
}

setup(sep)
	register struct servtab *sep;
{
	int on = 1;

	if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
		syslog(LOG_ERR, "%s/%s: socket: %m",
		    sep->se_service, sep->se_proto);
		return;
	}
#define	turnon(fd, opt) \
setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))
	if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
	    turnon(sep->se_fd, SO_DEBUG) < 0){
		syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
	}
	if (turnon(sep->se_fd, SO_REUSEADDR) < 0){
		syslog(LOG_ERR, "setsockopt (SO_REUSEADDR): %m");
	}
#undef turnon
	if(bind(sep->se_fd, &sep->se_ctrladdr, sizeof (sep->se_ctrladdr))<0){
		syslog(LOG_ERR, "%s/%s: bind: %m", sep->se_service, sep->se_proto);
		(void) close(sep->se_fd);
		sep->se_fd = -1;
		if (!timingout) {
			timingout = 1;
			alarm(RETRYTIME);
		}
		return;
	}
	if (sep->se_socktype == SOCK_STREAM){
		listen(sep->se_fd, 10);
	}
	FD_SET(sep->se_fd, &allsock);
	nsock++;
	if (sep->se_fd > maxsock){
		maxsock = sep->se_fd;
	}
}

struct servtab *
enter(cp)
	struct servtab *cp;
{
	register struct servtab *sep;
	long omask;

	sep = (struct servtab *)malloc(sizeof (*sep));
	if (sep == (struct servtab *)0) {
		syslog(LOG_ERR, "Out of memory.");
		quit(-1);
	}
	*sep = *cp;
	sep->se_fd = -1;
	sep->se_next = servtab;
	servtab = sep;
	return (sep);
}

FILE	*fconfig = NULL;
struct	servtab serv;
char	line[256];
char	*skip(), *nextline();

setconfig()
{

	if (fconfig != NULL) {
		fseek(fconfig, 0L, L_SET);
		return (1);
	}
	fconfig = fopen(CONFIG, "r");
	return (fconfig != NULL);
}

endconfig()
{
	if (fconfig) {
		(void) fclose(fconfig);
		fconfig = NULL;
	}
}

struct servtab *
getconfigent()
{
	register struct servtab *sep = &serv;
	int argc;
	char *cp, *arg, *strdup();

more:
	while ((cp = nextline(fconfig)) && *cp == '#'){
		;
	}

	if (cp == NULL){
		return ((struct servtab *)0);
	}
	sep->se_service = strdup(skip(&cp));
	arg = skip(&cp);
	if (strcmp(arg, "stream") == 0)
		sep->se_socktype = SOCK_STREAM;
	else if (strcmp(arg, "dgram") == 0)
		sep->se_socktype = SOCK_DGRAM;
	else if (strcmp(arg, "rdm") == 0)
		sep->se_socktype = SOCK_RDM;
	else if (strcmp(arg, "seqpacket") == 0)
		sep->se_socktype = SOCK_SEQPACKET;
	else if (strcmp(arg, "raw") == 0)
		sep->se_socktype = SOCK_RAW;
	else
		sep->se_socktype = -1;
	sep->se_proto = strdup(skip(&cp));
	arg = skip(&cp);
	sep->se_wait = strcmp(arg, "wait") == 0;
	sep->se_user = strdup(skip(&cp));
	sep->se_server = strdup(skip(&cp));
	if (strcmp(sep->se_server, "internal") == 0) {
		register struct biltin *bi;

		for (bi = biltins; bi->bi_service; bi++){
			if (bi->bi_socktype == sep->se_socktype &&
			    strcmp(bi->bi_service, sep->se_service) == 0){
				break;
			}
		}
		if (bi->bi_service == 0) {
			syslog(LOG_ERR, "internal service %s unknown\n",
				sep->se_service);
			goto more;
		}
		sep->se_bi = bi;
	} else {
		sep->se_bi = NULL;
	}
	argc = 0;
	for (arg = skip(&cp); cp; arg = skip(&cp)){
		if (argc < MAXARGV){
			sep->se_argv[argc++] = strdup(arg);
		}
	}
	while (argc <= MAXARGV){
		sep->se_argv[argc++] = NULL;
	}
	return (sep);
}

freeconfig(cp)
	register struct servtab *cp;
{
	int i;

	if (cp->se_service){
		free(cp->se_service);
	}
	if (cp->se_proto){
		free(cp->se_proto);
	}
	if (cp->se_user){
		free(cp->se_user);
	}
	if (cp->se_server){
		free(cp->se_server);
	}
	for (i = 0; i < MAXARGV; i++){
		if (cp->se_argv[i]){
			free(cp->se_argv[i]);
		}
	}
}

char *
skip(cpp)
	char **cpp;
{
	register char *cp = *cpp;
	char *start;

again:
	while (*cp == ' ' || *cp == '\t'){
		cp++;
	}
	if (*cp == '\0') {
		char c;

		c = getc(fconfig);
		(void) ungetc(c, fconfig);
		if (c == ' ' || c == '\t'){
			if (cp = nextline(fconfig)){
				goto again;
			}
		}
		*cpp = (char *)0;
		return ((char *)0);
	}
	start = cp;
	while (*cp && *cp != ' ' && *cp != '\t'){
		cp++;
	}
	if (*cp != '\0'){
		*cp++ = '\0';
	}
	*cpp = cp;
	return (start);
}

char *
nextline(fd)
	FILE *fd;
{
	char *cp;

	if (fgets(line, sizeof (line), fd) == NULL){
		return ((char *)0);
	}
	cp = index(line, '\n');
	if (cp){
		*cp = '\0';
	}
	return (line);
}

char *
strdup(cp)
	char *cp;
{
	char *new;

	if (cp == NULL){
		cp = "";
	}
	new = malloc((unsigned)(strlen(cp) + 1));
	if (new == (char *)0) {
		syslog(LOG_ERR, "Out of memory.");
		quit(-1);
	}
	(void)strcpy(new, cp);
	return (new);
}

/*
 * print_service:
 *	Dump relevant information to stderr
 */
print_service(action, sep)
	char *action;
	struct servtab *sep;
{
	sprintf(msgbuff,
	    "%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n",
	    action, sep->se_service, sep->se_proto,
	    sep->se_wait, sep->se_user, (int)sep->se_bi, sep->se_server);
	Message( msgbuff, WINDOW_OPEN ) ;    
}

echo_dg(s, sep)			/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZ];
	int i, size;
	struct sockaddr sa;

	size = sizeof(sa);
	if ((i = recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size)) < 0){
		return;
	}
	(void) sendto(s, buffer, i, 0, &sa, sizeof(sa));
}

discard_dg(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZ];

	(void) read(s, buffer, sizeof(buffer));
}

#include <ctype.h>
#define LINESIZ 72
char ring[128];
char *endring;

initring()
{
	register int i;

	endring = ring;

	for (i = 0; i <= 128; ++i){
		if (isprint(i)){
			*endring++ = i;
		}
	}
}

chargen_dg(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	struct sockaddr sa;
	static char *rs;
	int len, size;
	char text[LINESIZ+2];

	if (endring == 0) {
		initring();
		rs = ring;
	}

	size = sizeof(sa);
	if (recvfrom(s, text, sizeof(text), 0, &sa, &size) < 0)
		return;

	if ((len = endring - rs) >= LINESIZ)
		bcopy(rs, text, LINESIZ);
	else {
		bcopy(rs, text, len);
		bcopy(ring, text + len, LINESIZ - len);
	}
	if (++rs == endring)
		rs = ring;
	text[LINESIZ] = '\r';
	text[LINESIZ + 1] = '\n';
	(void) sendto(s, text, sizeof(text), 0, &sa, sizeof(sa));
}

machtime_dg(s, sep)
	int s;
	struct servtab *sep;
{
	long result;
	struct sockaddr sa;
	int size;

	size = sizeof(sa);
	if (recvfrom(s, (char *)&result, sizeof(result), 0, &sa, &size) < 0)
		return;
	result = machtime();
	(void) sendto(s, (char *) &result, sizeof(result), 0, &sa, sizeof(sa));
}

daytime_dg(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	struct sockaddr sa;
	int size;
	char *ctime();

	clock = time((time_t *) 0);

	size = sizeof(sa);
	if (recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size) < 0)
		return;
	(void) sprintf(buffer, "%.24s\r\n", ctime(&clock));
	(void) sendto(s, buffer, strlen(buffer), 0, &sa, sizeof(sa));
}

/*
 * Return a machine readable date and time, in the form of the
 * number of seconds since midnight, Jan 1, 1900.  Since gettimeofday
 * returns the number of seconds since midnight, Jan 1, 1970,
 * we must add 2208988800 seconds to this figure to make up for
 * some seventy years Bell Labs was asleep.
 */
long
machtime()
{
	struct timeval tv;

	if (gettimeofday(&tv, (struct timezone *)0) < 0) {
		if( debug )
			Message("Unable to get time of day\n", WINDOW_OPEN) ;
		return (0L);
	}
	return (htonl((long)tv.tv_sec + 2208988800));
}

machtime_stream(s, sep)
	int s;
	struct servtab *sep;
{
	long result;

	result = machtime();
	(void) write(s, (char *) &result, sizeof(result));
}

daytime_stream(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	char *ctime();

	clock = time((time_t *) 0);

	(void) sprintf(buffer, "%.24s\r\n", ctime(&clock));
	(void) write(s, buffer, strlen(buffer));
}


/*----------------------------------------------
 * 'mess' is the string that you want printed in the window
 * 'wincmd' is one of three flags:
 *
 *  2 = ERROR. Display me for 5 seconds and then exit the program.
 *	1 = open me if not open already
 *  0 = close me if open already
 *
 *  enum {  WINDOW_CLOSE = 0, WINDOW_OPEN, WINDOW_ERROR_DISPLAY } ;
 * -------------------------------------------
 */


void
Message( mess, wincmd )
char *mess ;
int wincmd ;
{
		
	static BPTR fh = NULL ;	
		
	switch( wincmd ) {
		case WINDOW_CLOSE :
			if( fh ) 
				{
				Close( fh ) ;
				fh = NULL ;
				break ;
				}
			break ;	
		case WINDOW_OPEN:
		case WINDOW_ERROR_DISPLAY:				
			if( fh == NULL )		
				{
				fh = Open("CON:000/020/600/140/  INETD Message : ", MODE_OLDFILE ) ;
				if( fh == NULL )
					break ;
				}
			if( mess )
				{
				Write( fh, mess, (long)strlen(mess)) ;
				if( wincmd == WINDOW_ERROR_DISPLAY )
					{
					Delay( 300L ) ;   /* approx 5 secs */  
					Close( fh ) ;
					fh = NULL ;
					}
				break ;
				}	
		default:
			break ;
		}		
}					


