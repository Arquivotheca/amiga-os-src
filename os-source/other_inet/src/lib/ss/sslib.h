/* -----------------------------------------------------------------------
 * sslib.h
 *
 * $Locker:  $
 *
 * $Id: sslib.h,v 1.5 93/03/24 15:21:26 kcd Exp $
 *
 * $Revision: 1.5 $
 *
 * $Log:	sslib.h,v $
 * Revision 1.5  93/03/24  15:21:26  kcd
 * Cleaned up prototypes.
 *
 * Revision 1.4  92/08/21  19:41:53  kcd
 * Added prototypes for the DNS function calls.
 *
 * Revision 1.3  91/10/23  16:39:13  bj
 * Added the 'debugging' field to the library base structure.
 * This allows us to turn debugging info on/off at will while
 * the library is in use. Still requires a library written to
 * take advantage of it -and- an external program to turn it
 * on/off.
 *
 * Revision 1.2  91/08/07  14:34:48  bj
 * added rcs header.
 * removed ONE_GLOBAL_SECTION stuff.
 * Added Semaphores to library base.
 * Added ml_config (ptr to config struct in memory)
 *      to library base.
 *
 *
 * $Header: AS225:src/lib/ss/RCS/sslib.h,v 1.5 93/03/24 15:21:26 kcd Exp $
 *
 *------------------------------------------------------------------------
 */
/*
** SSLIB.H  include file for all functions in the Shared Socket Library
** Not for external distribution
*/

#ifndef SSLIB_H
#define SSLIB_H


#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/semaphores.h>
#include <sys/types.h>
#include <socket.h>
#include <errno.h>
#include <config.h>
#include <netinet/in.h>
#include <netinet/inet.h>
#include <proto/all.h>
#include <proto/inet.h>

#define MAXALIASES	5	/* maximum number of aliases for host names, services, protocols, and networks */

/* These globals will be unique for every OpenLibrary */

extern int *ss_errno ;				/* pointer to the application's errno integer */
extern long *socks ;		/* array of pointers to socket structs */
extern BYTE Socket_sigurg ; 		/* urgent signal */
extern BYTE Socket_sigio ;			/* used for async I/O */
extern UWORD Num_socket ;			/* number of sockets open */
extern UWORD MAXSOCKS ;				/* max number of sockets that can be open */
extern UWORD Timeropen ;			/* indicates timer status */
extern BYTE Select_timerbit ;
extern struct timerequest Timereq;	/* timer for selectwait() */
extern BPTR debugfh;

struct SocketLibrary {
		struct       Library ml_Lib;
		ULONG        ml_SegList;
		ULONG        ml_Flags;
		APTR         ml_ExecBase;      /*              pointer to exec base */
		BPTR         ml_SyslogWindowFH ;
		struct SignalSemaphore *ml_SyslogSemaphore ;
		struct SignalSemaphore *ml_ConfigSemaphore ;
		struct config *ml_config ;
		long *       ml_relocs;        /*             pointer to relocs.    */
		struct SocketLibrary  *ml_origbase; /* pointer to original library base  */
		long         ml_numjmps;
		UWORD		 configured;
		UWORD        debugging;
};

/* GETSOCK() gets the socket structure from the socket descriptor */
#define GETSOCK(s, sock) {\
	if((s) < 0 || (s) >= MAXSOCKS || socks[s]==NULL){\
		*ss_errno = EBADF;\
		return( -1 ) ;\
	}\
	(sock) = socks[s];\
}

/* PUTSOCK() associates a socket descriptor with a socket structure */
#define PUTSOCK(s, sock){\
	if((s) < 0 || (s) >= MAXSOCKS){\
		*ss_errno = EBADF ;\
		return( -1) ;\
	}\
	Num_socket++;\
	socks[s] = (sock) ;\
}

#define isdigit(i)	(i>='0' && i<='9')
#define isxdigit(i) (isdigit(i) || (i>='a'&&i<='f') || (i>='A'&&i<='F'))
#define isspace(i)  (i==' ' || i=='\t' || i=='\n' || i=='\r' || i=='\v' || i=='\f')

/* V37 stuff */
#define stricmp(a,b)	Stricmp(a,b)
#define strnicmp(a,b,n)	Strnicmp(a,b,n)
#define tolower(a)		ToLower(a)
#define toupper(a)		ToUpper(a)

/* prototypes */
/* accept.c */
int __saveds __asm accept(register __d0 int s,
	register __a0 struct sockaddr *name,
	register __a1 int *lenp);
/* bind.c */
int __saveds __asm bind(register __d0 int s,
	register __a1 struct sockaddr *name,
	register __d1 int len);
/* config.c */
uid_t __saveds getuid(void);
gid_t __saveds getgid(void);
int __saveds __asm getgroups(register __d0 int num,register __a0 gid_t *gp);
char * __saveds getlogin(void);
short __saveds get_tz(void);
int __saveds __asm getdomainname(register __a1 char *buf,register __d1 int size);
mode_t __saveds getumask(void);
mode_t __saveds __asm umask(register __d0 int new_mask);
int __saveds __asm gethostname(register __a0 char *p,register __d0 int plen);
void __saveds __asm unconfig(register __a6 struct SocketLibrary *);
/* connect.c */
int __saveds __asm connect(register __d0 int s,
	register __a1 struct sockaddr *name,
	register __d1 int len);

/* debug.c */
void Dprintf(char *fmt, ...);

/* error.c */
char * __saveds __asm strerror(register __d1 int num);

/* functions.c *.
int __saveds __asm getpeername( register __d0 int s,
	register __a0 struct sockaddr *name,
	register __a1 int *lenp);
int __saveds __asm getsockname( register __d0 int s,
	register __a0 struct sockaddr *name,
	register __a1 int *lenp);

/* gethost.c */
void __saveds __asm sethostent(register __d1 int flag);
void __saveds endhostent(void);
struct hostent * __saveds gethostent(void);
struct hostent * __saveds __asm gethostbyname(register __a0 char *name);
struct hostent * __saveds __asm gethostbyaddr(
	register __a0 char *addr,
	register __d0 int len,
	register __d1 int type);

/* getnet.c */
void __saveds __asm setnetent( register __d1 int flag);
void __saveds endnetent( void );
struct netent * __saveds getnetent( void );
struct netent * __saveds __asm getnetbyaddr(
	register __d0 long net,
	register __d1 int type);
struct netent * __saveds __asm getnetbyname(register __a0 char *name);

/* getproto.c */
void __saveds __asm setprotoent( register __d1 int flag);
void __saveds endprotoent( void );
struct protoent * __saveds getprotoent( void );
struct protoent * __saveds __asm getprotobyname(register __a0 char *name);
struct protoent * __saveds __asm getprotobynumber(register __d0 int proto);

/* getservent.c */
struct servent * __saveds getservent(void);
struct servent * __saveds __asm getservbyname(register __a0 char *name,register __a1 char *proto);
void __saveds __asm setservent(register __d1 int flag);
void __saveds endservent( void );
struct servent * __saveds __asm getservbyport( register __d0 u_short port,register __a0 char *proto);

/* inet_addr.c */
u_long __saveds __asm inet_addr(register __a1 char *cp);
struct in_addr __saveds __asm inet_makeaddr(
	register __d0 int net,
	register __d1 int host);
int  __saveds __asm inet_lnaof( register __d1 struct in_addr in);
int __saveds __asm inet_netof( register __d1 struct in_addr in);
int __saveds __asm inet_network( register __a1 char *cp);
char * __saveds __asm inet_ntoa( register __d1 u_long l );

/* ioctl.c */
int __saveds __asm s_ioctl(register __d0 int s,
    register __d1 int cmd,
    register __a0 char *data);
/* listen.c */
int __saveds __asm listen(register __d0 int s,register __d1 int backlog);

/* password.c */
struct passwd * __saveds __asm getpwuid(register __d1 uid_t uid);
struct passwd * __saveds __asm getpwnam(register __a0 char *name);
struct passwd * __saveds getpwent(void);
void __saveds __asm setpwent(register __d1 int flag);
void __saveds endpwent(void);

/* rcmd.c */
int __saveds __asm rcmd(register __d0 char **ahost,
	register __d1 u_short rport,
	register __a0 char *locuser,
	register __a1 char *remuser,
	register __a2 char *cmd,
	register __d2 int *fd2p);

/* recv.c */
int __saveds __asm recv (register __d0 int s,
	register __a0 char *buf,
	register __d1 int len,
	register __d2 int flags);
int __saveds __asm recvfrom (register __d0 int s,
	register __a0 char *buf,
	register __d1 int len,
	register __d2 int flags,
	register __a1 struct sockaddr *from,
	register __a2 int *fromlen);
int __saveds __asm recvmsg (register __d0 int s,
	register __a0 struct msghdr *msg,
	register __d1 int flags);
/* select.c */
int __saveds __asm select( register __d0 int numfds,
	register __a0 fd_set *readfds,
	register __a1 fd_set *writefds,
	register __a2 fd_set *exceptfds,
	register __d1 struct timeval *tvp);
int __saveds __asm selectwait( register __d0 int numfds,
	register __a0 fd_set *readfds,
	register __a1 fd_set *writefds,
	register __a2 fd_set *exceptfds,
	register __d1 struct timeval *tvp,
	register __d2 long *umask);
/* send.c */
int __saveds __asm send(register __d0 int s,
	register __a0 char *buf,
	register __d1 int len,
	register __a1 int flags);
int __saveds __asm sendto(register __d0 int s,
	register __a0 char * buf,
	register __d1 int len,
	register __d2 int flags,
	register __a1 struct sockaddr *to,
	register __d3 int tolen);
int __saveds __asm sendmsg(register __d0 s,
	register __a0 struct msghdr *msg,
	register __d1 int flags);
/* shutdown.c */
int __saveds __asm shutdown(register __d0 int s,register __d1 int how);
/* socket.c */
ULONG __saveds __asm setup_sockets(
	register __d1 UWORD maxsocks,
	register __a0 int *apps_errno );
void __saveds cleanup_sockets( void );
int __saveds __asm socket( register __d0 int af,
	register __d1 int type,
	register __d2 int pf);
int __saveds __asm s_close( register __d0 int fd);
BYTE __saveds __asm s_getsignal( register __d1 UWORD type);
/* sockopt.c */
int __saveds __asm setsockopt(register __d0 int s,
	register __d1 int level,
	register __d2 int optname,
	register __a0 char *optval,
	register __d3 int optlen);
int __saveds __asm getsockopt(register __d0 int s,
	register __d1 int level,
	register __d2 int optname,
	register __a0 char *optval,
	register __a1 int *optlenp);
/* syslog.c */
int __saveds __asm syslog(register __d0 int pri,
	register __a0 char *message);

/* res_comp.c */
int __saveds __asm dn_expand(register __a0 u_char *msg,
	register __a1 u_char *eomorig,
	register __a2 u_char *comp_dn,
	register __a3 u_char *exp_dn,
	register __d0 int length);

int __saveds __asm dn_comp(register __a0 u_char *exp_dn,
	register __a1 u_char *comp_dn,
	register __d0 int length,
	register __a2 u_char **dnptrs,
	register __a3 u_char **lastdnptr);

int __saveds __asm __dn_skipname(register __a0 u_char *comp_dn,
	register __a1 u_char *eom);

static int __saveds __asm dn_find(register __a0 u_char *exp_dn,
	register __a1 u_char *msg,
	register __a2 u_char **dnptrs,
	register __a3 u_char **lastdnptr);

u_short __saveds __asm _getshort(register __a0 u_char *msgp);

u_long __saveds __asm getlong(register __a0 u_char *msgp);

void __saveds __asm __putshort(register __d0 u_short s,
	register __a0 u_char *msgp);

void __saveds __asm __putlong(register __d0 u_long l,
	register __a0 u_char *msgp);

/* res_send.c */
int __saveds __asm res_send(register __a0 char *buf,
	register __d0 int buflen,
	register __a1 char *answer,
	register __d1 anslen);

void __saveds __asm _res_close(void);

/* res_init.c */
int __saveds __asm res_init(void);

/* res_query.c */
int __saveds __asm res_query(register __a0 char *name,
	register __d0 int class,
	register __d1 int type,
	register __a1 u_char *answer,
	register __d2 int anslen);

int __saveds __asm res_search(register __a0 char *name,
	register __d0 int class,
	register __d1 int type,
	register __a1 u_char *answer,
	register __d2 int anslen);

int __saveds __asm res_querydomain(register __a0 char *name,
	register __a1 char *domain,
	register __d0 int class,
	register __d1 int type,
	register __a2 u_char *answer,
	register __d2 int anslen);

char * __saveds	__asm __hostalias(register __a0 char *name);

/* res_mkquery.c */
int __saveds __asm res_mkquery(register __d0 int op,
	register __a0 char *dname,
	register __d1 int class,
	register __d2 int type,
	register __a1 char *data,
	register __d3 int datalen,
	register __a2 struct rrec *newrr,
	register __a3 char *buf,
	register __d4 int buflen);

struct hostent * __saveds __asm _gethtbyaddr(
	register __a0 char *addr,
	register __d0 int len,
	register __d1 int type);

struct hostent * __saveds __asm
_gethtbyname(register __a0 char *name);

struct hostent * __saveds _gethtent(void);
void __saveds __asm _sethtent(register __d1 int flag);
void __saveds _endhtent(void);

#endif /* SSLIB_H */

/* ---- end ---- */
