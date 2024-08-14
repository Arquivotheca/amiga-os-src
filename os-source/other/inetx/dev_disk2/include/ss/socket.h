/* -----------------------------------------------------------------------
 * ss/socket.h  
 *
 * July 92
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
void s_dev_list(u_long, int);
BYTE s_getsignal ( UWORD );
int s_inherit( void * );
int s_ioctl ( int, int, char * );
void *s_release( int );
void s_dev_list(u_long, int);
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
int s_syslog(int, char *);
char *strerror( int );
mode_t umask ( mode_t );
int reconfig(void) ;

#ifdef UNIX_COMPAT
int s_accept (int, struct sockaddr *, int *);
int s_socket( int, int, int );
#endif

#include <ss/socket_pragmas.h>

#endif /* SS_SOCKET_H */