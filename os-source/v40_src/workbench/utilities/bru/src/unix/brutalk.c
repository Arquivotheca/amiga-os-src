/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	brutalk.c    simple program to talk to a pair of fifos for bru
 *
 *  SCCS
 *
 *	@(#)brutalk.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Simple minded program to talk to a pair of fifos.  Use as:
 *
 *		brutalk [-t tty] </dev/bru.q >/dev/bru.r
 *	or
 *
 *		brutalk [-t tty] /dev/bru.q /dev/bru.r
 *
 *	Normally brutalk will attempt to communicate with the
 *	user via /dev/tty.  The -t option can be used to select
 *	any tty type stream to open in place of /dev/tty.
 *
 *	We fork to create two processes.  The child reads from
 *	the fifo where queries are posted and writes to the terminal.
 *	The parent reads the replies from the terminal and writes
 *	to the fifo where replies are expected.
 *
 *	There are basically three normal ways to exit:
 *
 *		Before attempting to open explicitly named input and
 *		output fifos, we set a timer.  If the timer goes off
 *		because bru has not yet opened the other end of the
 *		fifos, then we get a SIGALRM and exit.  This basically
 *		means that either bru has exited normally, or has not
 *		yet requested any interaction.  Note that the timeout
 *		is only effective until bru opens the fifos for the 
 *		first time.  If you answer one query using brutalk,
 *		then interrupt brutalk, and come back and run it again
 *		later, it will wait until the next query is available.
 *		The initial timeout can be avoided by using the
 *		redirection form, since it is actually the shell that
 *		is opening the fifos in that case.
 *
 *		The child dies when the program posting queries exits,
 *		closing the write side of the query fifo.  The parent
 *		gets notified that the child has died by a SIGCLD, and
 *		itself exits.
 *
 *		If the user types a ^D (EOF) then the parent sends the
 *		child a kill signal and exits.  The original client
 *		at the other end of the fifos continues to execute.
 *		If it knows how to deal gracefully with that fact that
 *		we have exited (by closing and reopening the fifos),
 *		then we can reattach at a later time to answer more
 *		queries.
 *
 *	This program is normally used to interact with bru when bru
 *	is executed from cron by a command that includes the arguments:
 *
 *		-I q,/dev/bru.q		(send queries to fifo bru.q)
 *		-I r,/dev/bru.r		(read replies from fifo bru.r)
 *		-I l,/tmp/brulog	(send verbosity info here)
 *
 *	The main function returns 1 to satisfy lint and also to cover
 *	the "impossible case" where exit returns rather than exiting.
 *
 *  AUTHOR
 *
 *	Fred Fish
 *
 */

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#ifndef SIGCLD
#define SIGCLD SIGCHLD		/* If no SIGCLD or SIGCHLD, we croak */
#endif

extern void perror ();
extern void exit ();
extern unsigned alarm ();

static void openfiles ();
static void passdata ();

static int rfifo;
static int wfifo;
static int ttyf;

int main (argc, argv)
int argc;
char *argv[];
{
    int optchar;
    char *infifo = NULL;
    char *outfifo = NULL;
    char *tty = NULL;
    extern int optind;
    extern char *optarg;

    while ((optchar = getopt (argc, argv, "t:")) != EOF) {
	switch (optchar) {
	    case 't':
		tty = optarg;
		break;
	    case '?':
		exit (1);
		break;
	}
    }
    for ( ; optind < argc; optind++) {
	if (infifo == NULL) {
	    infifo = argv[optind];
	} else if (outfifo == NULL) {
	    outfifo = argv[optind];
	} else if (tty == NULL) {
	   tty = argv[optind];
	} else {
	    fprintf (stderr, "brutalk: too many arguments\n");
	    (void) fprintf (stderr, "usage: brutalk readfifo writefifo\n");
	    exit (1);
	}
    }
    if (tty == NULL) {
	tty = "/dev/tty";
    }
    (void) signal (SIGALRM, exit);
    (void) alarm (5);
    openfiles (infifo, outfifo, tty);
    (void) alarm (0);
    passdata ();
    exit (0);
    return (1);
}


/*
 *	Open the read and write fifos, and the terminal stream.
 *	Any failure is fatal with an error message printed.
 */

static void openfiles (rname, wname, ttyname)
char *rname;
char *wname;
char *ttyname;
{
    if (rname == NULL) {
	rfifo = 0;
    } else if ((rfifo = open (rname, O_RDONLY)) == -1) {
	(void) fprintf (stderr, "brutalk: can't open '%s'", rname);
	perror ("");
	exit (1);
    }
    if (wname == NULL) {
	wfifo = 1;
    } else if ((wfifo = open (wname, O_WRONLY)) == -1) {
	(void) fprintf (stderr, "brutalk: can't open '%s'", wname);
	perror ("");
	exit (1);
    }
    if ((ttyf = open (ttyname, O_RDWR)) == -1) {
	(void) fprintf (stderr, "brutalk: can't open '%s'", ttyname);
	perror ("");
	exit (1);
    }
}


/*
 *	Fork to create bidirectional data paths and then loop
 *	passing data through each path until a termination
 *	condition occurs.
 */

static void passdata ()
{
    int inbytes;
    int outbytes;
    int pid;
    char inbuf[1024];
    char outbuf[1024];
    
    if ((pid = fork ()) == -1) {
	perror ("brutalk: can't fork");
	exit (1);
    } else if (pid == 0) {
	while ((inbytes = read (rfifo, inbuf, sizeof (inbuf))) > 0) {
	    (void) write (ttyf, inbuf, (unsigned) inbytes);
	}
    } else {
	(void) signal (SIGCLD, exit);
	while ((outbytes = read (ttyf, outbuf, sizeof (outbuf))) > 0) {
	    (void) write (wfifo, outbuf, (unsigned) outbytes);
	}
	(void) kill (pid, SIGTERM);
    }
}

