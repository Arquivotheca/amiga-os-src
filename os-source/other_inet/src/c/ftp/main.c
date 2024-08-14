/* -----------------------------------------------------------------------
 * main.c  SAS C for ftp (as225 R2)
 *
 * $Locker:  $
 *
 * $Id: main.c,v 2.5 94/03/24 21:54:02 jesup Exp $
 *
 * $Revision: 2.5 $
 *
 * $Log:	main.c,v $
 * Revision 2.5  94/03/24  21:54:02  jesup
 * BJ didn't check this in
 * 
 * Revision 2.4  92/12/09  16:08:31  bj
 * Binary 2.8
 * 
 * Changed the new getcommand() function to use FGets instead
 * of an FGetC loop. The tradeoff is FGetC() gives you control
 * over an _immediate_ control-C at the command prompt while
 * FGets requires you to hit 'return' after the control-C
 * for it to take effect.  On the other hand, FGets() gives you
 * command history with the cursor keys while FGetC() does not.
 * 
 * Revision 2.3  92/12/08  17:33:16  bj
 * binary 2.7
 * 
 * Added the function "getcommand()" that allows us to handle
 * control-C entries at the command prompt. 
 * 
 * Revision 2.2  92/10/30  16:16:41  bj
 * Binary 2.5
 * 
 * Fixed bug where init_sock() was being called without checking it's
 * return value. Init_sock() opens the socket.library so failure leads
 * to enforcer hits.
 * 
 * Euro bug report.
 * 
 * Revision 2.1  92/10/30  11:27:51  bj
 * Binary 2.3
 * 
 * Removed extra copyright notices
 * 
 * Revision 2.0  92/07/20  14:06:27  bj
 * Complete rewrite for V2.0. For as225 release 2.
 * MH source.
 * 
 *
 * $Header: Hog:Other/inet/src/c/ftp/RCS/main.c,v 2.5 94/03/24 21:54:02 jesup Exp $
 *
 *------------------------------------------------------------------------
 */
/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

//#define DEBUG 1
#ifdef DEBUG
	#define DB(x) kprintf(x)
#else
	#define DB(x) ;
#endif

/*
 * FTP User Program -- Command Interface.
 */
#include <sys/types.h>
#include "ftp.h"
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <arpa/ftp.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <pwd.h>

//extern int (*_ONBREAK)();
//extern int ftp_break();

/* local function prototypes */
int main(int, char **);
void intr(void);
void lostpeer(void);
void cmdscanner(int top);
struct cmd *getcmd(register char *name);
void makeargv(void);
char *slurpstring(void);
int help(int, char **);

int isatty( int fd ) ;
int getcommand(int fromatty, char *line) ;

extern	char *home;
char	*getlogin();
long save_dir(void);

long StartDir=0;

#include "ftp_rev.h"
static char version[] = VERSTAG;


main(argc, argv)
	char *argv[];
{
	register char *cp;
	int top;
	struct passwd *pw = NULL;
	char homedir[MAXPATHLEN];

	if(init_sock() == -1)
	{
		fprintf(stderr, "ftp: couldn't open socket.library.\n") ;
		exit(1) ;
	}

	(void) signal(SIGINT, exit);
	StartDir = save_dir();

	sp = getservbyname("ftp", "tcp");
	if (sp == 0) {
		fprintf(stderr, "ftp: ftp/tcp: unknown service\n");
		exit(1);
	}
	doglob = 1;
	interactive = 1;
	autologin = 1;
	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		for (cp = *argv + 1; *cp; cp++)
			switch (*cp) {

			case 'd':
				options |= SO_DEBUG;
				debug++;
				break;
			
			case 'v':
				verbose++;
				break;

			case 't':
				trace++;
				break;

			case 'i':
				interactive = 0;
				break;

			case 'n':
				autologin = 0;
				break;

			case 'g':
				doglob = 0;
				break;

			default:
				fprintf(stdout,
				  "ftp: %c: unknown option\n", *cp);
				exit(1);
			}
		argc--, argv++;
	}
	fromatty = isatty(fileno(stdin));
	if (fromatty)
		verbose++;
	cpend = 0;	/* no pending replies */
	proxy = 0;	/* proxy not active */
	crflag = 1;	/* strip c.r. on ascii gets */
	sendport = -1;	/* not using ports */
	/*
	 * Set up the home directory in case we're globbing.
	 */
	cp = getlogin();
	if (cp != NULL) {
		pw = getpwnam(cp);
	}
	if (pw == NULL)
		pw = getpwuid(getuid());
	if (pw != NULL) {
		home = homedir;
		(void) strcpy(home, pw->pw_dir);
	}

	if (argc > 0) {
		if (setjmp(toplevel))
			exit(0);

//		(void) signal(SIGINT, intr);
//		(void) signal(SIGPIPE, lostpeer);
		setpeer(argc + 1, argv - 1);
	}
	top = setjmp(toplevel) == 0;
	if (top) {
//		(void) signal(SIGINT, intr);
//		(void) signal(SIGPIPE, lostpeer);
	}
	FGets(Input(), line, sizeof(line)) ;
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
}

void
intr()
{

	longjmp(toplevel, 1);
}

void
lostpeer()
{
	extern FILE *cout;
	extern int data;

	if (connected) {
		if (cout != NULL) {
			(void) shutdown(fileno(cout), 1+1);
			(void) fclose(cout);
			cout = NULL;
		}
		if (data >= 0) {
			(void) shutdown(data, 1+1);
			(void) close(data);
			data = -1;
		}
		connected = 0;
	}
	pswitch(1);
	if (connected) {
		if (cout != NULL) {
			(void) shutdown(fileno(cout), 1+1);
			(void) fclose(cout);
			cout = NULL;
		}
		connected = 0;
	}
	proxflag = 0;
	pswitch(0);
}


/*
 * Command parser.
 */
void cmdscanner(int top)
{
	register struct cmd *c;
	register int l;
	struct cmd *getcmd();

	if (!top)
		(void) putchar('\n');
	for (;;) {
		getcommand(fromatty, line) ;
		if(SetSignal(0L,0L) & SIGBREAKF_CTRL_C)
		{
			quit() ;
		}
//		if (fromatty) {
//			printf("ftp> ");
//			(void) fflush(stdout);
//		}
//		if (fgets(line, sizeof line, stdin) == NULL)
//			quit();
		l = strlen(line);
		if (l == 0)
			break;
		if (line[--l] == '\n') {
			if (l == 0)
				break;
			line[l] = '\0';
		} else if (l == sizeof(line) - 2) {
			printf("sorry, input line too long\n");
			while ((l = getchar()) != '\n' && l != EOF)
				/* void */;
			break;
		} /* else it was a line without a newline */
		makeargv();
		if (margc == 0) {
			continue;
		}
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			continue;
		}
		if (c == 0) {
			printf("?Invalid command\n");
			continue;
		}
		if (c->c_conn && !connected) {
			printf("Not connected.\n");
			continue;
		}
		(*c->c_handler)(margc, margv);
		if (bell && c->c_bell)
			(void) putchar('\007');
		if (c->c_handler != help)
			break;
	}
//	(void) signal(SIGINT, intr);
//	(void) signal(SIGPIPE, lostpeer);
}

struct cmd *
getcmd(name)
	register char *name;
{
	extern struct cmd cmdtab[];
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; p = c->c_name; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */

int slrflag;

void makeargv()
{
	char **argp;

	margc = 0;
	argp = margv;
	stringbase = line;		/* scan from first of buffer */
	argbase = argbuf;		/* store from first of buffer */
	slrflag = 0;
	while (*argp++ = slurpstring())
		margc++;
}

/*
 * Parse string into argbuf;
 * implemented with FSM to
 * handle quoting and strings
 */
char *
slurpstring()
{
	int got_one = 0;
	register char *sb = stringbase;
	register char *ap = argbase;
	char *tmp = argbase;		/* will return this if token found */

	if (*sb == '!' || *sb == '$') {	/* recognize ! as a token for shell */
		switch (slrflag) {	/* and $ as token for macro invoke */
			case 0:
				slrflag++;
				stringbase++;
				return ((*sb == '!') ? "!" : "$");
				/* NOTREACHED */
			case 1:
				slrflag++;
				altarg = stringbase;
				break;
			default:
				break;
		}
	}

S0:
	switch (*sb) {

	case '\0':
		goto OUT;

	case ' ':
	case '\t':
		sb++; goto S0;

	default:
		switch (slrflag) {
			case 0:
				slrflag++;
				break;
			case 1:
				slrflag++;
				altarg = sb;
				break;
			default:
				break;
		}
		goto S1;
	}

S1:
	switch (*sb) {

	case ' ':
	case '\t':
	case '\0':
		goto OUT;	/* end of token */

	case '\\':
		sb++; goto S2;	/* slurp next character */

	case '"':
		sb++; goto S3;	/* slurp quoted string */

	default:
		*ap++ = *sb++;	/* add character to token */
		got_one = 1;
		goto S1;
	}

S2:
	switch (*sb) {

	case '\0':
		goto OUT;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S1;
	}

S3:
	switch (*sb) {

	case '\0':
		goto OUT;

	case '"':
		sb++; goto S1;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S3;
	}

OUT:
	if (got_one)
		*ap++ = '\0';
	argbase = ap;			/* update storage pointer */
	stringbase = sb;		/* update scan pointer */
	if (got_one) {
		return(tmp);
	}
	switch (slrflag) {
		case 0:
			slrflag++;
			break;
		case 1:
			slrflag++;
			altarg = (char *) 0;
			break;
		default:
			break;
	}
	return((char *)0);
}

#define HELPINDENT (sizeof ("directory"))

/*
 * Help command.
 * Call each command handler with argc == 0 and argv[0] == name.
 */
int help(argc, argv)
	int argc;
	char *argv[];
{
	extern struct cmd cmdtab[];
	register struct cmd *c;

	if (argc == 1) {
		register int i, j, w, k;
		int columns, width = 0, lines;
		extern int NCMDS;

		printf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
			int len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				if (c->c_name && (!proxy || c->c_proxy)) {
					printf("%s", c->c_name);
				}
				else if (c->c_name) {
					for (k=0; k < strlen(c->c_name); k++) {
						(void) putchar(' ');
					}
				}
				if (c + lines >= &cmdtab[NCMDS]) {
					printf("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					(void) putchar('\t');
				}
			}
		}
		return 0;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf("?Ambiguous help command %s\n", arg);
		else if (c == (struct cmd *)0)
			printf("?Invalid help command %s\n", arg);
		else
			printf("%-*s\t%s\n", HELPINDENT,
				c->c_name, c->c_help);
	}
	return 0;
}


long save_dir()
{
	return((long)DupLock(((struct Process*)FindTask(0))->pr_CurrentDir));
}

void restore_dir(long dir)
{
	BPTR oldlock;
	oldlock = CurrentDir((BPTR)dir);
	UnLock(oldlock);
}

int chkabort(void)
{
	return(0) ;
}

#define PROMPT "ftp> "
                             
BPTR console_fh = NULL ;

int getcommand(int fromatty, char *line)
{
	BPTR fout = Output() ;
	BPTR fin  = Input() ;

	if( fromatty)
	{
		FPuts(fout, PROMPT) ;
		Flush(fout) ;
	}
	return((int)FGets(fin, line, 200L)) ;
}
	