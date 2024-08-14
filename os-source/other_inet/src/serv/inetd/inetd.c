
/* -----------------------------------------------------------------------
 * inetd.c
 *
 * $Locker:  $
 *
 * $Id: inetd.c,v 1.10 92/09/28 16:51:21 bj Exp $
 *
 * $Revision: 1.10 $
 *
 * $Log:	inetd.c,v $
 * Revision 1.10  92/09/28  16:51:21  bj
 * 1.5 binary version.
 * a. added to code check Readargs() return (grrrr Martin).
 * b. added "-D" (debug) flag to the Readargs Template.
 * c. added extra error reporting to the user.
 * 
 * Revision 1.9  92/09/11  17:31:53  bj
 * binary version 1.4.  Remake and checkin. Minor changes.
 * 
 *
 * $Header: AS225:src/serv/inetd/RCS/inetd.c,v 1.10 92/09/28 16:51:21 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/*
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
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (5)
 *
 * Comment lines are indicated by a `#' in column 1.
 */

#define MAXSOCKS    32
#define FD_SETSIZE	MAXSOCKS

#include <ss/socket.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
/*
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <arpa/inet.h>
*/
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <syslog.h>
#include <stdarg.h>
#include "inetd_rev.h"

struct Library *SockBase ;
extern struct Library *DOSBase ;
/*extern struct Library *SysBase = (*((struct Library **) 4)) ;*/

#define PORTNAME	"inetd"

void startit(register struct servtab *, int) ;
void config(void) ;
void setup( register struct servtab *sep ) ;
struct servtab *enter(struct servtab *cp ) ;
int setconfig(void) ;
void endconfig(void) ;
struct servtab *getconfigent(void) ;
void freeconfig(register struct servtab *cp ) ;
void cleanup(void) ;
char *skip(char **cpp) ;
char *nextline(BPTR fh) ;
char *strdup(char *cp) ;
void print_service(char *, struct servtab *) ;

void Dprintf(char *fmt, ...) ;
void syslog(int pri, ...) ;

int	debug = 0 ;
BPTR debugfh ;

int	maxsock ;
fd_set	allsock ;
struct	servent *sp ;
int errno ;

#define MAXARGV 5
struct	servtab {
	char	*se_service ;		/* name of service */
	int	se_socktype ;		/* type of socket to use */
	char	*se_proto ;		/* protocol used */
	short	se_wait ;		/* single threaded server */
	short	se_checked ;		/* looked at during merge */
	char	*se_server ;		/* server program */
	char	*se_argv[MAXARGV+1] ;	/* program arguments */
	int	se_fd ;			/* open descriptor */
	struct	sockaddr_in se_ctrladdr ;/* bound address */
	struct	servtab *se_next ;
} *servtab ;


char	*CONFIG = INETDFILE ;

struct inetmsg {
	struct Message	msg ;
	ULONG	id ;
} ;

struct MsgPort *inet_port ;

#define CMDREV  "\0$VER: " VSTRING
#define TEMPLATE    "-D=DEBUG/S" CMDREV
#define OPT_DEBUG    0
#define OPT_COUNT    1

int main(void)
{
	fd_set readable ;
	int n, ev, ctrl ;
	register struct servtab *sep ;
	ULONG port_sig ;
	struct inetmsg *msg ;
	long opts[OPT_COUNT] ;
	struct RDargs *rdargs ;

	memset((char *)opts, 0, sizeof(opts)) ;
	if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
	{
		if(opts[OPT_DEBUG])
		{
			debug=1 ;
		}
		FreeArgs(rdargs) ;
	}
	else
	{
		Dprintf("readargs failure\n") ;
		Close(debugfh) ;
		Exit(20) ;
	}

	Forbid() ;
	if( FindPort(PORTNAME) ) 
	{
		Permit() ;
		Exit(20) ;
	}
	inet_port = CreatePort(PORTNAME,0) ;
	Permit() ;
	if(inet_port==NULL)
	{
		Exit(20) ;
	}

	port_sig = 1 << inet_port->mp_SigBit ;
	if((SockBase = OpenLibrary( "inet:libs/socket.library", 4L )) == NULL)
	{
		Dprintf("Error opening socket.library\n") ;
		Close(debugfh) ;
		DeletePort(inet_port) ;
		Exit(20) ;
	}

	setup_sockets( MAXSOCKS, &errno ) ;


	config() ;


	while(1) 
	{
		readable = allsock ;
		ev = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | port_sig ;
		n = selectwait(maxsock+1,&readable,0L,0L,0L,&ev) ;

		if( ev & SIGBREAKF_CTRL_C )
		{
			cleanup() ;
		}
/*		if( ev & SIGBREAKF_CTRL_D )
		{
			config() ;
		}
*/
		if( ev & SIGBREAKF_CTRL_D ) 
		{
			if(debug)
			{
				debug=0 ;
			}
			else
			{
				debug = 1 ;
			}
		}
		if( ev & port_sig ) 
		{
			if( msg = (struct inetmsg *)GetMsg(inet_port) ) 
			{
				if(debug)
				{
					Dprintf("got message with id=%ld\n",msg->id) ;
				}
				setup((struct servtab *)msg->id) ;
				ReplyMsg((struct Message *)msg) ;
			}
		}

		if( n < 0 ) 
		{
			if( errno == EBADF ) 
			{
				syslog(LOG_WARNING,"INETD: Selectwait: %s\n",strerror(errno)) ;
				cleanup() ;
			}
			continue ;
		}
		for( sep = servtab ;n && sep ; sep = sep->se_next) 
		{
			if( FD_ISSET(sep->se_fd,&readable)) 
			{
				n-- ;
				if (debug)
				{
					Dprintf("someone wants %s\n",sep->se_service) ;
				}
				if( sep->se_socktype == SOCK_STREAM) 
				{
					ctrl = accept(sep->se_fd,(struct sockaddr *)0,0) ;
					if (debug)
					{
						Dprintf("accept, ctrl=%ld  fd=%ld\n",ctrl,sep->se_fd) ;
					}
					if ( ctrl < 0 ) 
					{
						if (errno == EINTR)
						{
							continue ;
						}
						syslog(LOG_WARNING,"INETD: accept: %s\n",strerror(errno)) ;
						continue ;
					}
				} else 	/* UDP socket */
					ctrl = sep->se_fd ;
				startit(sep,ctrl) ;
			}
		}
	}
}

void startit(register struct servtab *sep, int ctrl)
{
	char cmdbuf[256] ;
	void *sockptr ;
	int i ;
	long id=0 ;

	if(sep->se_wait || sep->se_socktype==SOCK_DGRAM) 
	{
		FD_CLR(sep->se_fd,&allsock) ;
		id = (long)sep ;
	}

	sockptr = s_release(ctrl) ;
	sprintf(cmdbuf, "run >nil: %s %s %ld %ld ", sep->se_server,
		sep->se_service, sockptr, id) ;
	for(i = 0 ; sep->se_argv[i] ; i++) 
	{
		strcat(cmdbuf, sep->se_argv[i]) ;
		strcat(cmdbuf," ") ;
	}
	if(debug) 
	{
		Dprintf("executing %s\n",cmdbuf) ;
	}
	Execute(cmdbuf, 0L, 0L) ;
}


void config(void)
{
	register struct servtab *sep, *cp, **sepp ;

	if (!setconfig()) 
	{
		syslog(LOG_ERR,"INETD: cannot open config file %s\n",CONFIG) ;
		return ;
	}
	for (sep = servtab ; sep ; sep = sep->se_next)
	{
		sep->se_checked = 0 ;
	}

	while (cp = getconfigent()) 
	{
		for (sep = servtab ; sep ; sep = sep->se_next)
		{
			if (strcmp(sep->se_service, cp->se_service) == 0 &&
			    strcmp(sep->se_proto, cp->se_proto) == 0)
			{
				break ;
			}
		}
		if (sep != 0) 
		{
			int i ;

#define SWAP(a, b) { char *c = a ; a = b ; b = c ; }
			if (cp->se_server)
			{
				SWAP(sep->se_server, cp->se_server) ;
			}
			for (i = 0 ; i < MAXARGV ; i++)
			{
				SWAP(sep->se_argv[i], cp->se_argv[i]) ;
			}
			freeconfig(cp) ;
			if (debug)
			{
				print_service("REDO", sep) ;
			}
		} 
		else 
		{
			sep = enter(cp) ;
			if (debug)
			{
				print_service("ADD ", sep) ;
			}
		}
		sep->se_checked = 1 ;
		sp = getservbyname(sep->se_service, sep->se_proto) ;
		if (sp == 0) 
		{
			syslog(LOG_ERR,"INETD: %s/%s: unknown service\n",
			    sep->se_service, sep->se_proto) ;
			sep->se_checked=0 ;
			continue ;
		}
		if (sp->s_port != sep->se_ctrladdr.sin_port) 
		{
			sep->se_ctrladdr.sin_port = sp->s_port ;
			if (sep->se_fd != -1)
			{
				(void)s_close(sep->se_fd) ;
			}
			sep->se_fd = -1 ;
		}
		if (sep->se_fd == -1)
		{
			setup(sep) ;
		}
	}
	endconfig() ;
	/*
	 * Purge anything not looked at above.
	 */
	sepp = &servtab ;
	while (sep = *sepp) {
		if (sep->se_checked) 
		{
			sepp = &sep->se_next ;
			continue ;
		}
		*sepp = sep->se_next ;
		if (sep->se_fd != -1) 
		{
			FD_CLR(sep->se_fd, &allsock) ;
			(void) s_close(sep->se_fd) ;
		}
		if (debug)
		{
			print_service("FREE", sep) ;
		}
		freeconfig(sep) ;
		FreeMem(sep,sizeof(struct servtab)) ;
	}
}

void setup( register struct servtab *sep )
{
	int on = 1 ;

	if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) 
	{
		syslog(LOG_ERR,"INETD: socket: %s/%s\n",
		    sep->se_service, sep->se_proto) ;
		return ;
	}
#define	turnon(fd, opt) \
setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))

/*	if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
	    turnon(sep->se_fd, SO_DEBUG) < 0)
	{
		syslog(LOG_ERR, "inetd: setsockopt (SO_DEBUG)\n") ;
	}
*/
	if (turnon(sep->se_fd, SO_REUSEADDR) < 0)
	{
		syslog(LOG_ERR, "INETD: setsockopt (SO_REUSEADDR)\n") ;
	}
#undef turnon
	if(bind(sep->se_fd, (struct sockaddr *)&sep->se_ctrladdr,sizeof(sep->se_ctrladdr))<0)
	{
		syslog(LOG_ERR,"INETD: bind: %s/%s\n", sep->se_service, sep->se_proto) ;
		(void) s_close(sep->se_fd) ;
		sep->se_fd = -1 ;
		return ;
	}
	if (sep->se_socktype == SOCK_STREAM)
	{
		listen(sep->se_fd, 10) ;
	}
	FD_SET(sep->se_fd, &allsock) ;
	if (sep->se_fd > maxsock)
	{
		maxsock = sep->se_fd ;
	}
}


struct servtab *enter(struct servtab *cp)
{
	register struct servtab *sep ;

	sep = (struct servtab *)AllocMem(sizeof(struct servtab),MEMF_PUBLIC) ;
	if (sep == (struct servtab *)0) 
	{
		syslog(LOG_ERR, "INETD: Out of memory.\n") ;
		cleanup() ;  
	}
	*sep = *cp ;
	sep->se_fd = -1 ;
	sep->se_next = servtab ;
	servtab = sep ;
	return (sep) ;
}

BPTR fconfig = NULL ;
struct	servtab serv ;
char	line[256] ;

int setconfig(void)
{

	if (fconfig != NULL) 
	{
		(void)Seek(fconfig, 0L, OFFSET_BEGINNING) ;
		return (1) ;
	}
	fconfig = Open(CONFIG, MODE_OLDFILE) ;
	return (fconfig != NULL) ;
}

void endconfig(void)
{
	if (fconfig) 
	{
		(void)Close(fconfig) ;
		fconfig = NULL ;
	}
}

struct servtab *getconfigent(void)
{
	register struct servtab *sep = &serv ;
	int argc ;
	char *cp, *arg ;
more:
	while ((cp = nextline(fconfig)) && *cp == '#')
	{
		 ;
	}

	if (cp == NULL)
	{
		return ((struct servtab *)0) ;
	}
	sep->se_service = strdup(skip(&cp)) ;
	arg = skip(&cp) ;
	if (strcmp(arg, "stream") == 0)
		sep->se_socktype = SOCK_STREAM ;
	else if (strcmp(arg, "dgram") == 0)
		sep->se_socktype = SOCK_DGRAM ;
	else if (strcmp(arg, "rdm") == 0)
		sep->se_socktype = SOCK_RDM ;
	else if (strcmp(arg, "seqpacket") == 0)
		sep->se_socktype = SOCK_SEQPACKET ;
	else if (strcmp(arg, "raw") == 0)
		sep->se_socktype = SOCK_RAW ;
	else
		sep->se_socktype = -1 ;
	sep->se_proto = strdup(skip(&cp)) ;
	arg = skip(&cp) ;
	sep->se_wait = strcmp(arg, "wait") == 0 ;
	sep->se_server = strdup(skip(&cp)) ;
	argc = 0 ;
	for (arg = skip(&cp) ; cp ; arg = skip(&cp))
	{
		if (argc < MAXARGV)
		{
			sep->se_argv[argc++] = strdup(arg) ;
		}
	}
	while (argc <= MAXARGV)
	{
		sep->se_argv[argc++] = NULL ;
	}
	return (sep) ;
}

void freeconfig(register struct servtab *cp )
{
	int i ;

	if (cp->se_service)
	{
		FreeVec(cp->se_service) ;
	}
	if (cp->se_proto)
	{
		FreeVec(cp->se_proto) ;
	}
	if (cp->se_server)
	{
		FreeVec(cp->se_server) ;
	}
	for (i = 0 ; i < MAXARGV ; i++)
	{
		if (cp->se_argv[i])
		{
			FreeVec(cp->se_argv[i]) ;
		}
	}
}

void cleanup(void)
{
	struct servtab *nsep, *sep = servtab ;

	do 
	{
		nsep = sep->se_next ;
		if( sep->se_fd != -1 )
		{
			(void)s_close(sep->se_fd) ;
		}
		freeconfig(sep) ;
		FreeMem(sep,sizeof(struct servtab)) ;
	} 
	while ( sep = nsep) ;

	if(debugfh) Close(debugfh) ;
	cleanup_sockets() ;
	CloseLibrary( SockBase ) ;
	DeletePort(inet_port) ;
	Exit(0) ;
}


char *skip(char **cpp)
{
	register char *cp = *cpp ;
	char *start ;

again:
	while (*cp == ' ' || *cp == '\t')
	{
		cp++ ;
	}
	if (*cp == '\0') 
	{
		char c ;

		c = FGetC(fconfig) ;
		(void) UnGetC(fconfig,c) ;
		if (c == ' ' || c == '\t')
		{
			if (cp = nextline(fconfig))
			{
				goto again ;
			}
		}
		*cpp = (char *)0 ;
		return ((char *)0) ;
	}
	start = cp ;
	while (*cp && *cp != ' ' && *cp != '\t')
	{
		cp++ ;
	}
	if (*cp != '\0')
	{
		*cp++ = '\0' ;
	}
	*cpp = cp ;
	return (start) ;
}


char *nextline(BPTR fh)
{
	char *cp ;

	if (FGets(fh, line, sizeof (line)) == NULL)
	{
		return (NULL) ;
	}
	cp = strchr(line, '\n') ;
	if (cp)
	{
		*cp = '\0' ;
	}
	return (line) ;
}

char *strdup(char *cp)
{
	char *new ;

	if (cp == NULL)
	{
		cp = "" ;
	}
	new = AllocVec((ulong)(strlen(cp) + 1),0) ;
	if (new == (char *)0) 
	{
		syslog(LOG_ERR, "INETD: Out of memory.\n") ;
		cleanup() ;
	}
	(void)strcpy(new, cp) ;
	return (new) ;
}

/*
 * print_service:
 *	Dump relevant information to stderr
 */
void print_service(char *action, struct servtab *sep)
{
	Dprintf("%s: %s proto=%s, wait=%ld, server=%s\n",
	    action, sep->se_service, sep->se_proto,
	    sep->se_wait, sep->se_server) ;

}


void Dprintf(char *fmt, ...)
{

    va_list args ;

    if (debugfh==NULL) 
    {
		debugfh = Open("con:0/0/640/100/INETD/AUTO/WAIT", MODE_NEWFILE) ;
    }

    va_start(args,fmt) ;
	VFPrintf(debugfh,fmt,(LONG *)args) ;

    va_end(args) ;


}


/*
void syslog(int pri, ...)
{
	va_list args ;
	char *fmt, buf[256] ;
	
	va_start(args,pri) ;
	fmt = va_arg(args,char *) ;
	vsprintf(buf,fmt,args) ;
	va_end(args) ;

	syslog(pri,buf) ;
}
*/



