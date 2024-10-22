/* -----------------------------------------------------------------------
 * RSHD.C   Manx 36  *ONLY*
 *
 * $Locker:  $
 *
 * $Id: rshd.c,v 1.4 90/11/26 16:00:14 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: HOG:Other/inet/src/serv/rshd/RCS/rshd.c,v 1.4 90/11/26 16:00:14 bj Exp $
 *
 * $Log:	rshd.c,v $
 * Revision 1.4  90/11/26  16:00:14  bj
 * ...and this version put the DeviceProc code back for now.
 * 
 * Revision 1.3  90/11/14  14:15:21  bj
 * This version simply removed the DeviceProc trick that we
 * were using to allow the cli window that launched the deamon
 * to close. We are now doing it another way. See _main.c file.
 * 
 * Added RCS headers.
 * 
 *
 *------------------------------------------------------------------------
 */

/*
 * Modified Berkeley rshd. 
 *
 * Copyright 1989 Ameristar Technology
 *
 * Implement someday:
 *
 *	1. network -> stdin (needs non blocking handler I/O)
 *	2. signals	    (""     "     "       "     ")
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <netdb.h>
#include <syslog.h>
#include "fd_to_fh.h"
#include <libraries/dosextens.h>

int	errno;
int	keepalive = 1;
char	*index(), *rindex(), *strncat();
int	error();
int dont_close = 0;

main(argc, argv)
	int argc;
	char **argv;
{
	extern int opterr, optind, _check_rhosts_file;
	struct linger linger;
	int ch, on = 1, fromlen;
	struct sockaddr_in from;
	struct Process *me;

	me = FindTask(0L);
	me->pr_ConsoleTask = DeviceProc("ram:");

	if(daemon_start(0, &argc, &argv) < 0){
		perror("rshd");
		exit(1);
	}

	openlog("rsh", LOG_PID | LOG_ODELAY, LOG_DAEMON);

	opterr = 0;
	while ((ch = getopt(argc, argv, "ln")) != EOF){
		switch((char)ch) {
		case 'l':
			_check_rhosts_file = 0;
			break;
		case 'n':
			keepalive = 0;
			break;
		case '?':
		default:
			syslog(LOG_ERR, "usage: rshd [-l]");
			break;
		}
	}
	argc -= optind;
	argv += optind;

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
	    sizeof(on)) < 0){
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	}

	linger.l_onoff = 1;
	linger.l_linger = 60;			/* XXX */
	if (setsockopt(0, SOL_SOCKET, SO_LINGER, (char *)&linger,
	    sizeof (linger)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_LINGER): %m");
	doit(&from);
}

doit(fromp)
	struct sockaddr_in *fromp;
{
	char cmdbuf[128], *cp, remotehost[2 * MAXHOSTNAMELEN + 1];
	char pname[32];
	char locuser[16], remuser[16];
	char buf[512];
	struct passwd *pwd;
	struct hostent *hp;
	struct FileHandle *fh;
	int s, cc, result, on = 1;
	char *hostname;
	short port;

	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	if (fromp->sin_family != AF_INET) {
		syslog(LOG_ERR, "malformed from address\n");
		exit(1);
	}
#ifdef IP_OPTIONS
      {
	u_char optbuf[BUFSIZ/3], *cp;
	char lbuf[BUFSIZ], *lp;
	int optsize = sizeof(optbuf), ipproto;
	struct protoent *ip;

	if ((ip = getprotobyname("ip")) != NULL)
		ipproto = ip->p_proto;
	else
		ipproto = IPPROTO_IP;
	if (getsockopt(0, ipproto, IP_OPTIONS, (char *)optbuf, &optsize) == 0 &&
	    optsize != 0) {
		lp = lbuf;
		for (cp = optbuf; optsize > 0; cp++, optsize--, lp += 3)
			sprintf(lp, " %2.2x", *cp);
		syslog(LOG_NOTICE,
		    "Connection received using IP options (ignored):%s", lbuf);
		if (setsockopt(0, ipproto, IP_OPTIONS,
		    (char *)NULL, &optsize) != 0) {
			syslog(LOG_ERR, "setsockopt IP_OPTIONS NULL: %m");
			exit(1);
		}
	}
      }
#endif

	if (fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, "Connection from %s on illegal port",
			inet_ntoa(fromp->sin_addr));
		exit(1);
	}

	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;

		if ((cc = read(0, &c, 1)) != 1) {
			if (cc < 0)
				syslog(LOG_NOTICE, "read: %m");
			shutdown(0, 1+1);
			exit(1);
		}
		if (c == 0){
			break;
		}
		port = port * 10 + c - '0';
	}

	(void) alarm(0);
	if (port != 0) {
		int lport = IPPORT_RESERVED - 1;
		s = rresvport(&lport);
		if (s < 0) {
			syslog(LOG_ERR, "can't get stderr port: %m");
			exit(1);
		}
		if (port >= IPPORT_RESERVED) {
			syslog(LOG_ERR, "2nd port not reserved\n");
			exit(1);
		}
		fromp->sin_port = htons((u_short)port);
		if (connect(s, fromp, sizeof (*fromp)) < 0) {
			syslog(LOG_INFO, "connect second port: %m");
			exit(1);
		}
	}

	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		if (local_domain(hp->h_name)) {
			strncpy(remotehost, hp->h_name, sizeof(remotehost) - 1);
			remotehost[sizeof(remotehost) - 1] = 0;
			hp = gethostbyname(remotehost);
			if (hp == NULL) {
				syslog(LOG_INFO,
				    "Couldn't look up address for %s",
				    remotehost);
				error("Couldn't look up address for your host");
				exit(1);
			} else for (; ; hp->h_addr_list++) {
				if (!bcmp(hp->h_addr_list[0],
				    (caddr_t)&fromp->sin_addr,
				    sizeof(fromp->sin_addr)))
					break;
				if (hp->h_addr_list[0] == NULL) {
					syslog(LOG_NOTICE,
					  "Host addr %s not listed for host %s",
					    inet_ntoa(fromp->sin_addr),
					    hp->h_name);
					error("Host address mismatch");
					exit(1);
				}
			}
		}
		hostname = hp->h_name;
	} else {
		hostname = inet_ntoa(fromp->sin_addr);
	}

	getstr(remuser, sizeof(remuser), "remuser");
	getstr(locuser, sizeof(locuser), "locuser");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
	setpwent();
	pwd = getpwnam(locuser);
	if (pwd == NULL) {
		error("Login incorrect.\n");
		exit(1);
	}
	endpwent();
	if (chdir(pwd->pw_dir) < 0) {
		(void) chdir(":");
	}

	if (pwd->pw_passwd != 0 && *pwd->pw_passwd != '\0' &&
	    ruserok(hostname, pwd->pw_uid == 0, remuser, locuser) < 0) {
		error("Permission denied.\n");
		exit(1);
	}

	(void) write(0, "\0", 1);

/* special hack to get rcp working */

	if(!strncmp(cmdbuf,"rcp ",4)) {
		dont_close++;
		strcpy(buf,"run >nil: ");
		strcat(buf,cmdbuf);
		sprintf(cmdbuf," %ld",_socks[0]);
		strcat(buf,cmdbuf);
		Execute(buf,0L,0L);
		return;
	}
	result = my_popen(cmdbuf, pname);
	if(!result){
		error("execute failed\n");
		exit(1);
	}
	if ((fh = Open(pname, MODE_OLDFILE))== NULL) {
		error("can't find pipe\n");
		exit(1);
	}

	do {
		cc = Read(fh,buf,sizeof(buf));
		if (cc > 0)
			write(0,buf,cc);
	} while(cc > 0);

	Close(fh);
	fh = NULL;

}

error(fmt, a1, a2, a3)
	char *fmt;
	int a1, a2, a3;
{
	char buf[BUFSIZ];

	buf[0] = 1;
	(void) sprintf(buf+1, fmt, a1, a2, a3);
	(void) write(0, buf, strlen(buf));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
			error("%s too long\n", err);
			exit(1);
		}
	} while (c != 0);
}

/*
 * Check whether host h is in our local domain,
 * as determined by the part of the name following
 * the first '.' in its name and in ours.
 * If either name is unqualified (contains no '.'),
 * assume that the host is local, as it will be
 * interpreted as such.
 */
local_domain(h)
	char *h;
{
	char localhost[MAXHOSTNAMELEN];
	char *p1, *p2 = index(h, '.');

	(void) gethostname(localhost, sizeof(localhost));
	p1 = index(localhost, '.');
	if (p1 == NULL || p2 == NULL || !strcasecmp(p1, p2)){
		return(1);
	}
	return(0);
}
