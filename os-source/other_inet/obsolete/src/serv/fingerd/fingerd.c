/*
 * Copyright (c) 1983 The Regents of the University of California.
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

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static char line[1024], cmdbuf[sizeof(line)+32], *strtok(), ch;

main(argc, argv)
	char	**argv;
{
	register FILE *fp, *popen();
	register char *lp;
	char *strtok();

	if(daemon_start(0, &argc, &argv) < 0){
		fatal("ftpd: could not establish socket\n");
	}
	if(!fgets(line, sizeof(line), stdin)){
		fatal("fgets");
	}
	strcpy(cmdbuf, "inet:c/finger ");
	for(lp = strtok(line, " \t\r\n"); lp; lp = strtok(NULL, " \t\r\n")){
		if(strcasecmp(lp, "/w") == 0){
			strcat(cmdbuf, "-l ");
		} else {
			strcat(cmdbuf, lp);
		}
	}
	fp = popen(cmdbuf, "r");
	if(!fp){
		fatal("fdopen");
	}
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n'){
			send(0, "\r", 1, 0);
		}
		send(0, &ch, 1, 0);
	}
	exit(0);
}

fatal(msg)
	char *msg;
{
	extern int errno;
	char *strerror();

	fprintf(stderr, "fingerd: %s: %s\r\n", msg, strerror(errno));
	exit(1);
}
