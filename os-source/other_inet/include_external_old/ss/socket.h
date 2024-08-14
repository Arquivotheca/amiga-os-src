/* -----------------------------------------------------------------------
 * ss/socket.h  (ssfunctions.h - see lmkfile
 *
 * $Locker:  $
 *
 * $Id: ssfunctions.h,v 1.3 92/03/12 23:19:46 martin Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	ssfunctions.h,v $
 * Revision 1.3  92/03/12  23:19:46  martin
 * added s_inherit() and s_release()
 * changed syslog() to s_syslog()
 * 
 * Revision 1.2  92/02/16  23:45:55  martin
 * added syslog and reconfig
 * 
 * Revision 1.2  91/08/07  14:27:52  bj
 * added rcs header.
 * added syslog() and reconfig() entries.
 * 
 *
 * $Header: Work1:net_work/lib/ss/RCS/ssfunctions.h,v 1.3 92/03/12 23:19:46 martin Exp $
 *
 *------------------------------------------------------------------------
 */
/*
**	ss/socket.h
**
**	shared socket library prototypes
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef SS_SOCKET_H
#define SS_SOCKET_H

#include <exec/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

extern struct Library *SockBase;

/* prototypes */

int accept (int, struct sockaddr *, int *);
int bind (int, struct sockaddr *, int );
void cleanup_sockets ( void ) ;
int connect (int, struct sockaddr *, int);
void endhostent ( void );
void endnetent ( void );
void endprotoent ( void );
void endpwent( void );
void endservent ( void );
int getdomainname (char *, int);
gid_t getgid (void);
int getgroups (int, gid_t *);
struct hostent *gethostbyaddr ( char *, int, int );
struct hostent *gethostbyname ( char * );
struct hostent *gethostent ( void );
int gethostname (char *, int);
char *getlogin (void);
struct netent *getnetbyaddr ( long, int );
struct netent *getnetbyname ( char * );
struct netent *getnetent ( void );
int getpeername ( int, struct sockaddr *, int * );
struct protoent *getprotobyname ( char * );
struct protoent *getprotobynumber ( int );
struct protoent *getprotoent ( void );
struct passwd *getpwent( void ) ;
struct passwd *getpwnam( char * );
struct passwd *getpwuid( uid_t );
struct servent *getservent ( void );
struct servent *getservbyname ( char *, char * );
struct servent *getservbyport ( u_short, char * );
int getsockname ( int, struct sockaddr *, int * );
int getsockopt ( int, int, int, char *, int * );
uid_t getuid (void);
mode_t getumask (void);
short get_tz(void);
u_long inet_addr ( char * );
int inet_lnaof ( struct in_addr );
struct in_addr inet_makeaddr ( int, int );
int inet_netof ( struct in_addr );
int inet_network ( char * );
char *inet_ntoa ( struct in_addr );
int listen (int , int);
int rcmd( char **, u_short, char *, char *, char *, int *);
int recv(int, char *, int, int );
int recvfrom( int, char *, int, int, struct sockaddr *, int *);
int recvmsg(int, struct msghdr *, int );
int s_close ( int ) ;
BYTE s_getsignal ( UWORD );
int s_inherit( void * );
int s_ioctl ( int, int, char * );
void *s_release( int );
int s_syslog(int, char *);
int select( int, fd_set *, fd_set *, fd_set *, struct timeval *);
int selectwait (int, fd_set *, fd_set *, fd_set *, struct timeval *, long *);
int send (int, char *, int, int );
int sendto (int, char *, int, int, struct sockaddr *, int );
int sendmsg (int, struct msghdr *, int );
void sethostent ( int );
void setnetent ( int );
void setprotoent ( int );
void setpwent( int );
void setservent ( int );
int setsockopt ( int, int, int, char *, int );
ULONG setup_sockets ( UWORD, int * );
int shutdown (int, int);
int socket( int, int, int );
char *strerror( int );
mode_t umask ( mode_t );
int reconfig(void) ;

#include <ss/socket_pragmas.h>

#endif /* SS_SOCKET_H */