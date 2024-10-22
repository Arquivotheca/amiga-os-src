/* -----------------------------------------------------------------------
 * rcmd.c
 *
 * $Locker:  $
 *
 * $Id: rcmd.c,v 1.3 92/07/21 17:07:22 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	rcmd.c,v $
 * Revision 1.3  92/07/21  17:07:22  bj
 * socket.library 4.0
 * changed cast in bind() call in rresvport() function.
 * 
 * Revision 1.2  91/08/07  13:10:28  bj
 * RCS header.
 * Debug() calls removed.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/rcmd.c,v 1.3 92/07/21 17:07:22 bj Exp $
 *
 *------------------------------------------------------------------------
 */

/****** socket.library/rcmd *************************************************
*
*   NAME
*	rcmd - Allow superuser to execute commands on remote machines
*
*   SYNOPSIS
*	rem = rcmd( ahost, inport, luser, ruser, cmd, fd2p )
*	D0          D0	   D1	   A0     A1     A2   D2
*
*	int rcmd( char **, u_short, char *, char *, char *, int *);
*
*   FUNCTION
*	This function is used by rsh and rcp to communicate with rshd.
*
*	The rcmd subroutine looks up the host *ahost using gethostbyname,
*	returning (-1) if the host does not exist. Otherwise *ahost is
*	set to the standard name of the host and a connection is
*	established to a server residing at the well-known Internet port
*	inport.
*
*	If the call succeeds, a socket of type SOCK_STREAM is returned to
*	the caller and given to the remote command as stdin and stdout.
*	If fd2p is nonzero, then an auxiliary channel to a control
*	process will be set up, and a descriptor for it will be placed in
*	*fd2p. The control process will return diagnostic output from the
*	command (unit 2) on this channel, and will also accept bytes on
*	this channel as being UNIX signal numbers, to be forwarded to the
*	process group of the command. If fd2p is 0, then the stderr (unit
*	2 of the remote command) will be made the same as the stdout and
*	no provision is made for sending arbitrary signals to the remote
*	process, although you may be able to get its attention by using
*	out-of-band data.
*
*   INPUTS
*	ahost  		- pointer to a pointer to a host name.
*	inport 		- an Internet port.
*	luser  		- the local user's name.
*	ruser  		- the remote user's name.
*	cmd    		- the command string to be executed.
*	fd2p   		- a flag telling whether to use an auxillary channel.
*
*   RESULT
*	rem 		- socket of type SOCK_STREAM if successful, else -1.
*
*
********************************************************************
*
*
*/

/*
 * rcmd.c
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "sslib.h"

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>


#define bzero(a,n)	memset(a,'\0',n)
#define bcmp(a,b,n)	memcmp(a,b,n)
#define bcopy(from,to,len) memcpy(to,from,len)

/* prototypes */
int rresvport(int *alport);
int sprintf (char *, char *,...);

/*
ruserok( char *rhost, int superuser, char *ruser, char *luser);
_validuser( FILE *hostf, char *rhost, char *luser, char *ruser,int baselen);
static _checkhost( char *rhost, char *lhost, int len);
int sigmask( int arg1 );
long sigblock( int arg1 );
int sigsetmask( int mask );
int fcntl( int fd, int request, int arg);
*/

int __saveds __asm rcmd(register __d0 char **ahost,
	register __d1 u_short rport,
	register __a0 char *locuser,
	register __a1 char *remuser,
	register __a2 char *cmd,
	register __d2 int *fd2p)
{
	int s, timo = 1;
	struct sockaddr_in sin, from;
	char c;
	int lport = IPPORT_RESERVED - 1;
	struct hostent *hp;
	fd_set reads;


	/* look up host name in host table */
	hp = gethostbyname(*ahost);
	if (hp == (struct hostent *)0) {
		*ss_errno = EHOSTUNREACH;
		return (-1);
	}

	*ahost = hp->h_name;

	for (;;) {
		s = rresvport(&lport);
		if (s < 0)
			return (-1);

		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], (char *)&sin.sin_addr, hp->h_length);
		sin.sin_port = rport;
		if (connect(s, &sin, sizeof (sin)) >= 0)
			break;
		s_close(s);
		if (*ss_errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (*ss_errno == ECONNREFUSED && timo <= 16) {
			Delay(timo*50);
			timo *= 2;
			continue;
		}

		/* if there is an alias, try that */
		if (hp->h_addr_list[1] != NULL) {
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (char *)&sin.sin_addr,hp->h_length);
			continue;
		}

		return (-1);
	}

	/* we got a connection */

	lport--;

	if (fd2p == 0) {
		/* tell remote connection that we have no secondary stream */
		send(s,"",1,0);
		lport = 0;
	} else {	/* open up auxiliary channel */
		char num[8];
		int s2 = rresvport(&lport), s3;
		int len = sizeof (from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);

		/* send lport to remote connection as secondary channel */
		(void) sprintf(num, "%ld", lport);
		if (send(s, num, strlen(num)+1, 0) != strlen(num)+1) {
			(void) s_close(s2);
			goto bad;
		}

		FD_ZERO(&reads);
		FD_SET(s, &reads);
		FD_SET(s2, &reads);
		if (select(32, &reads, 0, 0, 0) < 1 || !FD_ISSET(s2, &reads)) {
			(void) s_close(s2);
			goto bad;
		}
		s3 = accept(s2, &from, &len);
		(void) s_close(s2);
		if (s3 < 0) {
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED ||
		    from.sin_port < IPPORT_RESERVED / 2) {
			goto bad2;
		}
	}

	(void) send (s, locuser, strlen(locuser)+1, 0);
	(void) send (s, remuser, strlen(remuser)+1, 0);
	(void) send (s, cmd, strlen(cmd)+1, 0);
	if (recv(s, &c, 1, 0) != 1) {
		goto bad2;
	}
	if (c != 0) {
		while (recv(s, &c, 1, 0) == 1) {
			(void) send(2, &c, 1, 0);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
	return (s);
bad2:
	if (lport)
		(void) s_close(*fd2p);
bad:
	(void) s_close(s);
	return (-1);
}



int rresvport(int *alport)
{
	struct sockaddr_in sin;
	int s;

    /* first get a TCP socket */
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);


	/* now bind it to a reserved port */
	for (;;) {
		/* first try the port we asked for... */
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			return (s);

		/* check to see if we had a real error */
		if (*ss_errno != EADDRINUSE) {
			s_close(s);
			return (-1);
		}

		/* lets try a smaller port :^) */
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			s_close(s);
			*ss_errno = EAGAIN;
			return (-1);
		}
	}
}

