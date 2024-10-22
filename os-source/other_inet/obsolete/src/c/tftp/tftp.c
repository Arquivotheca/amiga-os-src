/* ---------------------------------------------------------------------------------
 * TFTP.C
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

#ifndef lint
static char sccsid[] = "@(#)tftp.c	5.7 (Berkeley) 6/29/88";
#endif /* not lint */

/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Protocol Machines
 */
 
#include "tftp_rev.h"
 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/tftp.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <setjmp.h>

extern	int errno;

extern  struct sockaddr_in sin;         /* filled in by main */
extern  int     f;                      /* the opened socket */
extern  int     trace;
extern  int     verbose;
extern  int     rexmtval;
extern  int     maxtimeout;

static char *compiler = "MANX36 MANX36" ;
static char *tftpvers = VERSTAG ;

#define PKTSIZE    SEGSIZE+4
char    ackbuf[PKTSIZE];
int	timeout;
jmp_buf	toplevel;
jmp_buf	timeoutbuf;

timer()
{

	timeout += rexmtval;
	if (timeout >= maxtimeout) {
		printf("Transfer timed out.\n");
		longjmp(toplevel, -1);
	}
	longjmp(timeoutbuf, 1);
}

/*
 * Send the requested file.
 */
sendfile(fd, name, mode)
	int fd;
	char *name;
	char *mode;
{
	register struct tftphdr *ap;       /* data and ack packets */
	struct tftphdr *r_init(), *dp;
	register int block = 0, size, n;
	register unsigned long amount = 0;
	struct sockaddr_in from;
	int fromlen;
	int convert;            /* true if doing nl->crlf conversion */
	FILE *file, *fdopen();

	startclock();           /* start stat's clock */
	dp = r_init();          /* reset fillbuf/read-ahead code */
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "r");
	convert = !strcmp(mode, "netascii");

	signal(SIGALRM, timer);
	do {
		if (block == 0)
			size = makerequest(WRQ, name, dp, mode) - 4;
		else {
			size = readit(file, &dp, convert);
			if (size < 0) {
				nak(errno + 100);
				break;
			}
			dp->th_opcode = htons((u_short)DATA);
			dp->th_block = htons((u_short)block);
		}
		timeout = 0;
		(void) setjmp(timeoutbuf);
send_data:
		if (trace)
			tpacket("sent", dp, size + 4);
		n = sendto(f, dp, size + 4, 0, (caddr_t)&sin, sizeof (sin));
		if (n != size + 4) {
			perror("tftp: sendto");
			goto abort;
		}
		read_ahead(file, convert);
		for ( ; ; ) {
			alarm(rexmtval);
			do {
				fromlen = sizeof (from);
				n = trecvfrom(f, ackbuf, sizeof (ackbuf), 0,
				    (caddr_t)&from, &fromlen);
			} while (n <= 0);
			alarm(0);
			if (n < 0) {
				perror("tftp: recvfrom");
				goto abort;
			}
			sin.sin_port = from.sin_port;   /* added */
			if (trace)
				tpacket("received", ap, n);
			/* should verify packet came from server */
			ap->th_opcode = ntohs(ap->th_opcode);
			ap->th_block = ntohs(ap->th_block);
			if (ap->th_opcode == ERROR) {
printf("Error in location 1\n");
				printf("Error code %d: %s\n", ap->th_code,
					ap->th_msg);
				goto abort;
			}
			if (ap->th_opcode == ACK) {
				int j;

				if (ap->th_block == block) {
					break;
				}
				/* On an error, try to synchronize
				 * both sides.
				 */
				j = synchnet(f);
				if (j && trace) {
					printf("discarded %d packets\n",
							j);
				}
				if (ap->th_block == (block-1)) {
					goto send_data;
				}
			}
		}
		if (block > 0)
			amount += size;
		block++;
	} while (size == SEGSIZE || block == 1);
abort:
	fclose(file);
	stopclock();
	if (amount > 0)
		printstats("Sent", amount);
}

/*
 * Receive a file.
 */
recvfile(fd, name, mode)
	int fd;
	char *name;
	char *mode;
{
	register struct tftphdr *ap;
	struct tftphdr *dp, *w_init();
	register int block = 1, n, size;
	unsigned long amount = 0;
	struct sockaddr_in from;
	int fromlen, firsttrip = 1;
	FILE *file;
	int convert;                    /* true if converting crlf -> lf */

	startclock();
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "w");
	convert = !strcmp(mode, "netascii");

	signal(SIGALRM, timer);
	do {
		if (firsttrip) {
			size = makerequest(RRQ, name, ap, mode);
			firsttrip = 0;
		} else {
			ap->th_opcode = htons((u_short)ACK);
			ap->th_block = htons((u_short)(block));
			size = 4;
			block++;
		}
		timeout = 0;
		(void) setjmp(timeoutbuf);
send_ack:
		if (trace)
			tpacket("sent", ap, size);
		if (sendto(f, ackbuf, size, 0, (caddr_t)&sin,
		    sizeof (sin)) != size) {
			alarm(0);
			perror("tftp: sendto");
			goto abort;
		}
		write_behind(file, convert);
		for ( ; ; ) {
			alarm(rexmtval);
			do  {
				fromlen = sizeof (from);
				n = trecvfrom(f, dp, PKTSIZE, 0,
				    (caddr_t)&from, &fromlen);
			} while (n <= 0);
			alarm(0);
			if (n < 0) {
				perror("tftp: recvfrom");
				goto abort;
			}
			sin.sin_port = from.sin_port;   /* added */
			if (trace)
				tpacket("received", dp, n);
			/* should verify client address */
			dp->th_opcode = ntohs(dp->th_opcode);
			dp->th_block = ntohs(dp->th_block);
			if (dp->th_opcode == ERROR) {
printf("error in loc 2\n");
				printf("Error code %d: %s\n", dp->th_code,
					dp->th_msg);
				goto abort;
			}
			if (dp->th_opcode == DATA) {
				int j;

				if (dp->th_block == block) {
					break;          /* have next packet */
				}
				/* On an error, try to synchronize
				 * both sides.
				 */
				j = synchnet(f);
				if (j && trace) {
					printf("discarded %d packets\n", j);
				}
				if (dp->th_block == (block-1)) {
					goto send_ack;  /* resend ack */
				}
			}
		}
		size = writeit(file, &dp, n - 4, convert);
		if (size < 0) {
			nak(errno + 100);
			break;
		}
		amount += size;
	} while (size == SEGSIZE);
abort:                                          /* ok to ack, since user */
	ap->th_opcode = htons((u_short)ACK);    /* has seen err msg */
	ap->th_block = htons((u_short)block);
	(void) sendto(f, ackbuf, 4, 0, &sin, sizeof (sin));
	write_behind(file, convert);            /* flush last buffer */
	fclose(file);
	stopclock();
	if (amount > 0)
		printstats("Received", amount);
}

makerequest(request, name, tp, mode)
	int request;
	char *name, *mode;
	struct tftphdr *tp;
{
	register char *cp;

	tp->th_opcode = htons((u_short)request);
	cp = tp->th_stuff;
	strcpy(cp, name);
	cp += strlen(name);
	*cp++ = '\0';
	strcpy(cp, mode);
	cp += strlen(mode);
	*cp++ = '\0';
	return (cp - (char *)tp);
}

struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,	"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ -1,		0 }
};

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
nak(error)
	int error;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;
	extern char *sys_errlist[];

	tp = (struct tftphdr *)ackbuf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = sys_errlist[error - 100];
		tp->th_code = EUNDEF;
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg) + 4;
	if (trace)
		tpacket("sent", tp, length);
	if (sendto(f, ackbuf, length, 0, &sin, sizeof (sin)) != length)
		perror("nak");
}

tpacket(s, tp, n)
	char *s;
	struct tftphdr *tp;
	int n;
{
	static char *opcodes[] =
	   { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR" };
	register char *cp, *file;
	u_short op = ntohs(tp->th_opcode);
	char *index();

	if (op < RRQ || op > ERROR)
		printf("%s opcode=%x ", s, op);
	else
		printf("%s %s ", s, opcodes[op]);
	switch (op) {

	case RRQ:
	case WRQ:
		n -= 2;
		file = cp = tp->th_stuff;
#ifdef AMIGA
		while(*cp){
			cp++;
		}
#else
		cp = index(cp, '\0');
#endif
		printf("<file=%s, mode=%s>\n", file, cp + 1);
		break;

	case DATA:
		printf("<block=%d, %d bytes>\n", ntohs(tp->th_block), n - 4);
		break;

	case ACK:
		printf("<block=%d>\n", ntohs(tp->th_block));
		break;

	case ERROR:
		printf("<code=%d, msg=%s>\n", ntohs(tp->th_code), tp->th_msg);
		break;
	}
}

struct timeval tstart;
struct timeval tstop;
struct timezone zone;

startclock() {
	gettimeofday(&tstart, &zone);
}

stopclock() {
	gettimeofday(&tstop, &zone);
}

printstats(direction, amount)
	char *direction;
	unsigned long amount;
{
	long tenths;

	tenths = ((tstop.tv_sec*10)+(tstop.tv_usec/100000)) -
		((tstart.tv_sec*10)+(tstart.tv_usec/100000));
	printf("%s %d bytes in %d.%d seconds", direction, amount, 
			tenths/10, tenths%10);
	if (verbose){
		printf(" [%d bytes/sec]", (amount * 10) / tenths);
	}
	putchar('\n');
}

