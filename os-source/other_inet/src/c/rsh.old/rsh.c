/* ---------------------------------------------------------------------------------
 * RSH.C
 *
 * $Locker$
 *
 * $Id$
 *
 * $Revision$
 *
 * $Header$
 *
 *-----------------------------------------------------------------------------------
 */

/*
 * Quick hack at simple rsh() command for Amiga.  This version cannot
 * do stdin -> remote as we (still) don't have a non blocking or async
 * protocol for handlers.  Perhaps someday....
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <config.h>
#include <errno.h>
#include <stdio.h>

#define RSH_PORT 514		/* XXXX - should getservent */
#define MAX(a, b) ((a) > (b) ? (a):(b))

static char cmd[128], buf[1024];

#include "rsh_rev.h"
static char *compiler = "MANX36 MANX36" ;
static char *vers = VERSTAG ;

usage()
{
	printf("usage: rsh host [-l username] cmd ...\n");
	exit(1);
}

main(argc, argv)
	int	argc;
	char	**argv;
{
	int s, cc, cmdstart, rdfd2, one, num, nmode = 0;
	char *remote_user=NULL, *local_user;
	fd_set readers, rd;
	struct config *cf;

	if(argc < 3 || argv[1][0] == '-'){
		usage();
	}

	GETCONFIG(cf);
	if(!cf)
		local_user="guest";
	else
		local_user=cf->username;

	remote_user = local_user;

	for(cmdstart = 2; cmdstart < argc; cmdstart++){
		if(strncasecmp(argv[cmdstart], "-l", 2) == 0){
			remote_user = argv[++cmdstart];
			continue;
		} else if(strncasecmp(argv[cmdstart], "-n", 2) == 0){
			nmode++;
			continue;
		} else {
			/* everything else is a command */
			break;
		}
	}

	cmd[0] = '\0';
	for(; cmdstart < argc; cmdstart++){
		strcat(cmd, argv[cmdstart]);
		strcat(cmd, " ");
	}

	s = rcmd(&argv[1], RSH_PORT, local_user, remote_user, cmd, &rdfd2);
	if(s < 0){
		fprintf(stderr, "rsh: connection failed\n");
		exit(1);
	}

	one = 1;
	ioctl(rdfd2, FIONBIO, &one);
	ioctl(s, FIONBIO, &one);
	FD_ZERO(&readers);
	FD_SET(s, &readers);
	FD_SET(rdfd2, &readers);

	/*
	 * Let remote know we're not sending any data.
	 */
	shutdown(s, 1);

	do {
		rd = readers;
		num = select(MAX(rdfd2, s)+1, &rd, 0L, 0L, 0L);
		if(num < 0 && errno == EINTR){
			FD_ZERO(&readers);
		}

		if(FD_ISSET(rdfd2, &rd)){	/* stderr reader */
			do {
				cc = recv(rdfd2, buf, sizeof(buf), 0);
				if(cc > 0){
					write(1, buf, cc);
				}
			} while(cc > 0);
			if(cc == 0){
				FD_CLR(rdfd2, &readers);
			}
		}

		if(FD_ISSET(s, &rd)){		/* stdout reader */
			do {
				cc = recv(s, buf, sizeof(buf), 0);
				if(cc > 0){
					write(1, buf, cc);
				}
			} while(cc > 0);
			if(cc == 0){
				FD_CLR(s, &readers);
			}
		}

	} while(FD_ISSET(rdfd2, &readers) || FD_ISSET(s, &readers));

	close(rdfd2);
	close(s);
}

