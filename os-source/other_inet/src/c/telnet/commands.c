/* -----------------------------------------------------------------------
 * commands.c   telnet  
 *
 * $Locker:  $
 *
 * $Id: commands.c,v 1.1 91/01/15 18:00:42 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inetx/src/telnet/RCS/commands.c,v 1.1 91/01/15 18:00:42 bj Exp $
 *
 * $Log:	commands.c,v $
 * Revision 1.1  91/01/15  18:00:42  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#define DEBUG 0

#if DEBUG != 0
#define DB(x,y)    printf("DEBUG: %s: %s\n",(x),(y)) ;
#else
#define DB(x,y)  ;
#endif


/*
 * Copyright (c) 1988 Regents of the University of California.
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

#ifdef AZTEC_C
    #ifdef __VERSION
       #if __VERSION == 500
           #ifdef va_start
               #undef va_start
           #endif
           #ifdef va_arg
               #undef va_arg
           #endif
           #ifdef va_end
               #undef va_end
           #endif
           #define __STDARG_H  /* make sure stdarg.h is avoided */
       #endif
   #endif
#endif

#include <varargs.h>



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <signal.h>
#include <netdb.h>
#include <ctype.h>

#include <arpa/telnet.h>

#include "general.h"

#include "ring.h"

#include "externs.h"
#include "defines.h"
#include "types.h"

char	*hostname;

#define Ambiguous(s)	((char *)s == ambiguous)
static char *ambiguous;		/* special return value for command routines */

typedef struct {
	char	*name;		/* command name */
	char	*help;		/* help string */
	int	(*handler)();	/* routine which executes command */
	int	dohelp;		/* Should we give general help information? */
	int	needconnect;	/* Do we need to be connected to execute? */
} Command;

static char line[200];
static int margc;
static char *margv[20];

/* ========================================================
 * Various utility routines.
 * ========================================================
 */

static void
makeargv(void)
{
	register char *cp;
	register char **argp = margv;

	margc = 0;
	cp = line;
	if (*cp == '!') 		/* Special case shell escape */
		{
		*argp++ = "!";		/* No room in string to get this */
		margc++;
		cp++;
		}
	while (*cp) 
		{
		while (isspace(*cp))
		cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
		}
	*argp++ = 0;
}

/*-----------------------------------------------------------------
 */

static char **
genget(char *name, char **table, char **(*next)())
	/* char	*name;		 name to match */
	/* char	**table;		 name entry in table */
	/* char	**(*next)();	 routine to return next entry in table */
{
	register char *p, *q;
	register char **c, **found;
	register int nmatches, longest;

	if (name == 0) 
		{
		return 0;
		}
	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = table; (p = *c) != 0; c = (*next)(c)) 
		{
		for (q = name;(*q == *p) || (isupper(*q) && tolower(*q) == *p); p++, q++)
			{
			if (*q == 0)		/* exact match? */
				{
				return (c);
				}
			}
		if (!*q) 			/* the name was a prefix */
			{
			if (q - name > longest) 
				{
				longest = q - name;
				nmatches = 1;
				found = c;
				} 
			else if (q - name == longest)
				{
				nmatches++;
				}
			}
		}
	if (nmatches > 1)
		{
		return (char **)ambiguous;
		}
	return (found);
}

/* -------------------------------------------------------------
 * Make a character string into a number.
 *
 * Todo:  1.  Could take random integers (12, 0x12, 012, 0b1).
 * -------------------------------------------------------------
 */

static
special( register char *s )
{
	register char c;
	char b;

	switch (*s) 
		{
		case '^':
			b = *++s;
			if (b == '?') 
				{
				c = b | 0x40;		/* DEL */
				} 
			else 
				{
				c = b & 0x1f;
				}
			break;
		default:
			c = *s;
			break;
		}
	return( c );
}

/* -----------------------------------------------
 * Construct a control character sequence
 * for a special character.
 * ----------------------------------------------
 */
 
static char *
control( register int c )
{
	static char buf[3];

	if (c == 0x7f)
		{
		return ("^?");
		}
	if (c == '\377') 
		{
		return "off";
		}
	if (c >= 0x20) 
		{
		buf[0] = c;
		buf[1] = 0;
		} 
	else 
		{
		buf[0] = '^';
		buf[1] = '@'+c;
		buf[2] = 0;
		}
	return (buf);
}



/* -------------------------------------------------------
 *	The following are data structures and routines for
 *	the "send" command.
 * -------------------------------------------------------
 */
 
struct sendlist {
    char	*name;		/* How user refers to it (case independent) */
    int		what;		/* Character to be sent (<0 ==> special) */
    char	*help;		/* Help information (0 ==> no help) */
#ifdef NOT43
    int		(*routine)();	/* Routine to perform (for special ops) */
#else	/* defined(NOT43) */
    void	(*routine)();	/* Routine to perform (for special ops) */
#endif	/* defined(NOT43) */
    };

#define	SENDQUESTION	-1
#define	SENDESCAPE	-3

static struct sendlist Sendlist[] = {
    { "ao", AO, "Send Telnet Abort output" },
    { "ayt", AYT, "Send Telnet 'Are You There'" },
    { "brk", BREAK, "Send Telnet Break" },
    { "ec", EC, "Send Telnet Erase Character" },
    { "el", EL, "Send Telnet Erase Line" },
    { "escape", SENDESCAPE, "Send current escape character" },
    { "ga", GA, "Send Telnet 'Go Ahead' sequence" },
    { "ip", IP, "Send Telnet Interrupt Process" },
    { "nop", NOP, "Send Telnet 'No operation'" },
    { "synch", SYNCH, "Perform Telnet 'Synch operation'", dosynch },
    { "?", SENDQUESTION, "Display send options" },
    { 0 }
};

static struct sendlist Sendlist2[] = {		/* some synonyms */
	{ "break", BREAK, 0 },

	{ "intp", IP, 0 },
	{ "interrupt", IP, 0 },
	{ "intr", IP, 0 },

	{ "help", SENDQUESTION, 0 },

	{ 0 }
};

/* ----------------------------------------------- */

static char **
getnextsend( char *name )
{
	struct sendlist *c = (struct sendlist *) name;

	return (char **) (c+1);
}

/* ---------------------------------------------- */

static struct sendlist *
getsend( char *name )
{
	struct sendlist *sl;

	if ((sl = (struct sendlist *)genget(name, (char **) Sendlist, getnextsend)) != 0) 
		{
		return sl;
		} 
	else 
		{
		return (struct sendlist *)genget(name, (char **) Sendlist2, getnextsend);
		}
}

/* ------------------------------------------- */

static
sendcmd( int argc, char **argv)
{
	int what;		/* what we are sending this time */
	int count;		/* how many bytes we are going to need to send */
	int i;
	int question = 0;	/* was at least one argument a question */
	struct sendlist *s;	/* pointer to current command */

	if (argc < 2) 
		{
		tprintf("need at least one argument for 'send' command\n");
		tprintf("'send ?' for help\n");
		return 0;
		}
	/*
	 * First, validate all the send arguments.
	 * In addition, we see how much space we are going to need, and
	 * whether or not we will be doing a "SYNCH" operation (which
	 * flushes the network queue).
	 */

	count = 0;
	for (i = 1; i < argc; i++) 
		{
		s = getsend(argv[i]);
		if (s == 0) 
			{
			tprintf("Unknown send argument '%s'\n'send ?' for help.\n",argv[i]);
			return 0;
			} 
		else if (Ambiguous(s)) 
			{
			tprintf("Ambiguous send argument '%s'\n'send ?' for help.\n",argv[i]);
			return 0;
			}
		switch (s->what) 
			{
			case SENDQUESTION:
				break;
			case SENDESCAPE:
				count += 1;
				break;
			case SYNCH:
				count += 2;
				break;
			default:
				count += 2;
			break;
			}
		}     /* for */
			/* Now, do we have enough room? */
	if (NETROOM() < count) 
		{
		tprintf("There is not enough room in the buffer TO the network\n");
		tprintf("to process your request.  Nothing will be done.\n");
		tprintf("('send synch' will throw away most data in the network\n");
		tprintf("buffer, if this might help.)\n");
		return 0;
		}
			/* OK, they are all OK, now go through again and actually send */
	for (i = 1; i < argc; i++) 
		{
		if ((s = getsend(argv[i])) == 0) 
			{
			fprintf(stderr, "Telnet 'send' error - argument disappeared!\n");
			quit();
			/*NOTREACHED*/
			}
		if (s->routine) 
			{
			(*s->routine)(s);
			} 
		else 
			{
			switch (what = s->what) 
				{
				case SYNCH:
					dosynch();
					break;
				case SENDQUESTION:
					for (s = Sendlist; s->name; s++) 
						{
						if (s->help) 
							{
							tprintf(s->name);
							if (s->help) 
								{
								tprintf("\t%s", s->help);
								}
							tprintf("\n");
							}
						}
					question = 1;
					break;
				case SENDESCAPE:
					NETADD(escape);
					break;
				default:
					NET2ADD(IAC, what);
					break;
				}
			}
		}     /* for( ;; )  */
	return !question;
}

/* -------------------------------------------------------------
 * The following are the routines and data structures referred
 * to by the arguments to the "toggle" command.
 * ------------------------------------------------------------
 */

static
lclchars( void )
{
	donelclchars = 1;
	return 1;
}

/* ----------------------------------------------------- */

static
togdebug( void )
{
#ifndef	NOT43
	if (net > 0 && (SetSockOpt(net, SOL_SOCKET, SO_DEBUG, debug)) < 0) 
		{
		perror("setsockopt (SO_DEBUG)");
		}
#else	/* NOT43 */
	if (debug) 
		{
		if (net > 0 && SetSockOpt(net, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
			{
			perror("setsockopt (SO_DEBUG)");
			}
		} 
	else
		{
		tprintf("Cannot turn off socket debugging\n");
		}
#endif	/* NOT43 */
	return 1;
}

/* ----------------------------------------------- */

static int
togcrlf( void )
{
	if (crlf) 
		{
		tprintf("Will send carriage returns as telnet <CR><LF>.\n");
		} 
	else 
		{
		tprintf("Will send carriage returns as telnet <CR><NUL>.\n");
		}
	return 1;
}


static int
togbinary( void )
{
	donebinarytoggle = 1;

	if (myopts[TELOPT_BINARY] == 0) 	/* Go into binary mode */
		{
		NET2ADD(IAC, DO);
		NETADD(TELOPT_BINARY);
		printoption("<SENT", doopt, TELOPT_BINARY, 0);
		NET2ADD(IAC, WILL);
		NETADD(TELOPT_BINARY);
		printoption("<SENT", doopt, TELOPT_BINARY, 0);
		hisopts[TELOPT_BINARY] = myopts[TELOPT_BINARY] = 1;
		tprintf("Negotiating binary mode with remote host.\n");
		} 
	else 				/* Turn off binary mode */
		{
		NET2ADD(IAC, DONT);
		NETADD(TELOPT_BINARY);
		printoption("<SENT", dont, TELOPT_BINARY, 0);
		NET2ADD(IAC, DONT);
		NETADD(TELOPT_BINARY);
		printoption("<SENT", dont, TELOPT_BINARY, 0);
		hisopts[TELOPT_BINARY] = myopts[TELOPT_BINARY] = 0;
		tprintf("Negotiating network ascii mode with remote host.\n");
		}
	return 1;
}


/* ----------------------------------------- */

extern int togglehelp();

struct togglelist {
    char	*name;		/* name of toggle */
    char	*help;		/* help message */
    int		(*handler)();	/* routine to do actual setting */
    int		dohelp;		/* should we display help information */
    int		*variable;
    char	*actionexplanation;
};

static struct togglelist Togglelist[] = {
    { "autoflush",
	"toggle flushing of output when sending interrupt characters",
	    0,
		1,
		    &autoflush,
			"flush output when sending interrupt characters" },
    { "autosynch",
	"toggle automatic sending of interrupt characters in urgent mode",
	    0,
		1,
		    &autosynch,
			"send interrupt characters in urgent mode" },
    { "binary",
	"toggle sending and receiving of binary data",
	    togbinary,
		1,
		    0,
			0 },
    { "crlf",
	"toggle sending carriage returns as telnet <CR><LF>",
	    togcrlf,
		1,
		    &crlf,
			0 },
    { "crmod",
	"toggle mapping of received carriage returns",
	    0,
		1,
		    &crmod,
			"map carriage return on output" },
    { "localchars",
	"toggle local recognition of certain control characters",
	    lclchars,
		1,
		    &localchars,
			"recognize certain control characters" },
    { " ", "", 0, 1 },		/* empty line */
    { "debug",
	"(debugging) toggle debugging",
	    togdebug,
		1,
		    &debug,
			"turn on socket level debugging" },
    { "netdata",
	"(debugging) toggle printing of hexadecimal network data",
	    0,
		1,
		    &netdata,
			"print hexadecimal representation of network traffic" },
    { "options",
	"(debugging) toggle viewing of options processing",
	    0,
		1,
		    &showoptions,
			"show option processing" },
    { " ", "", 0, 1 },		/* empty line */
    { "?",
	"display help information",
	    togglehelp,
		1 },
    { "help",
	"display help information",
	    togglehelp,
		0 },
    { 0 }
};


/* --------------------------------------- */

static
togglehelp( void )
{
	struct togglelist *c;

	for (c = Togglelist; c->name; c++) 
		{
		if (c->dohelp) 
			{
			tprintf("%-12s\t%s\n", c->name, c->help);
			}
		}
	return 0;
}

/* --------------------------------- */

static char **
getnexttoggle( char *name)
{
	struct togglelist *c = (struct togglelist *) name;

	return (char **) (c+1);
}

/* ---------------------------------- */

static struct togglelist *
gettoggle( char *name)
{
	return (struct togglelist *)genget(name, (char **) Togglelist, getnexttoggle);
}

/* ---------------------------------- */

static
toggle(int argc, char **argv)
{
	int retval = 1;
	char *name;
	struct togglelist *c;

	if (argc < 2) 
		{
		fprintf(stderr,
			"Need an argument to 'toggle' command.  'toggle ?' for help.\n");
		return 0;
		}
	argc--;
	argv++;
	while (argc--) 
		{
		name = *argv++;
		c = gettoggle(name);
		if (Ambiguous(c)) 
			{
			fprintf(stderr, "'%s': ambiguous argument ('toggle ?' for help).\n",
					name);
			return 0;
			} 
		else if (c == 0) 
			{
			fprintf(stderr, "'%s': unknown argument ('toggle ?' for help).\n",
					name);
			return 0;
			} 
		else 
			{
			if (c->variable) 
				{
				*c->variable = !*c->variable;		/* invert it */
				if (c->actionexplanation) 
					{
					tprintf("%s %s.\n", *c->variable? "Will" : "Won't",
							c->actionexplanation);
					}
				tprintf("%s %s.\n", *c->variable? "Will" : "Won't",
							c->actionexplanation);
				}
			if (c->handler) 
				{
				retval &= (*c->handler)(c);
				}
			}
		}
	return retval;
}

/* -----------------------------------------------
 * The following perform the "set" command.
 * ----------------------------------------------
 */

struct setlist {
    char *name;				/* name */
    char *help;				/* help information */
    char *charp;			/* where it is located at */
};

static struct setlist Setlist[] = {
    { "echo", 	"character to toggle local echoing on/off", &echoc },
    { "escape",	"character to escape back to telnet command mode", &escape },
    { " ", "" },
    { " ", "The following need 'localchars' to be toggled true", 0 },
    { "erase",	"character to cause an Erase Character", &termEraseChar },
    { "flushoutput", "character to cause an Abort Oubput", &termFlushChar },
    { "interrupt", "character to cause an Interrupt Process", &termIntChar },
    { "kill",	"character to cause an Erase Line", &termKillChar },
    { "quit",	"character to cause a Break", &termQuitChar },
    { "eof",	"character to cause an EOF ", &termEofChar },
    { 0 }
};

static char **
getnextset( char *name )
{
	struct setlist *c = (struct setlist *)name;

	return (char **) (c+1);
}

/* ---------------------------------- */

static struct setlist *
getset(char *name)
{
	return (struct setlist *) genget(name, (char **) Setlist, getnextset);
}


/* ---------------------------------- */

static
setcmd(int argc, char **argv)
{
	int value;
	struct setlist *ct;

		/* XXX back we go... sigh */
	if (argc != 3) 
		{
		if ((argc == 2) &&
			((!strcmp(argv[1], "?")) || (!strcmp(argv[1], "help")))) 
			{
			for (ct = Setlist; ct->name; ct++) 
				{
				tprintf("%-12s\t%s\n", ct->name, ct->help);
				}
			tprintf("%-12s\tdisplay help information\n", "?");
			} 
		else 
			{
			tprintf("Format is 'set Name Value'\n'set ?' for help.\n");
			}
		return 0;
		}

	ct = getset(argv[1]);
	if (ct == 0) 
		{
		fprintf(stderr, "'%s': unknown argument ('set ?' for help).\n",
			argv[1]);
		return 0;
		} 
	else if (Ambiguous(ct)) 
		{
		fprintf(stderr, "'%s': ambiguous argument ('set ?' for help).\n",
			argv[1]);
		return 0;
		} 
	else 
		{
		if (strcmp("off", argv[2])) 
			{
			value = special(argv[2]);
			} 
		else 
			{
			value = -1;
			}
		*(ct->charp) = value;
		tprintf("%s character is '%s'.\n", ct->name, control(*(ct->charp)));
		}
	return 1;
}

/* ------------------------------------------------------------
 * The following are the data structures and routines for the
 * 'mode' command.
 * ------------------------------------------------------------
 */

static
dolinemode( void )
{
	if (hisopts[TELOPT_SGA]) 
		{
		wontoption(TELOPT_SGA, 0);
		}
	if (hisopts[TELOPT_ECHO]) 
		{
		wontoption(TELOPT_ECHO, 0);
		}
	return 1;
}

/* --------------------------------------- */

static
docharmode(void)
{
	if (!hisopts[TELOPT_SGA]) 
		{
		willoption(TELOPT_SGA, 0);
		}
	if (!hisopts[TELOPT_ECHO]) 
		{
		willoption(TELOPT_ECHO, 0);
		}
	return 1;
}

static Command Mode_commands[] = {
    { "character",	"character-at-a-time mode",	docharmode, 1, 1 },
    { "line",		"line-by-line mode",		dolinemode, 1, 1 },
    { 0 },
};

/* --------------------------------------- */

static char **
getnextmode(char *name)
{
	Command *c = (Command *) name;

	return (char **) (c+1);
}

/* --------------------------------------- */

static Command *
getmodecmd(char *name)
{
	return (Command *) genget(name, (char **) Mode_commands, getnextmode);
}

/* --------------------------------------- */

static
modecmd(int argc, char **argv)
{
	Command *mt;

	if ((argc != 2) || !strcmp(argv[1], "?") || !strcmp(argv[1], "help")) 
		{
		tprintf("format is:  'mode Mode', where 'Mode' is one of:\n\n");
		for (mt = Mode_commands; mt->name; mt++) 
			{
			tprintf("%-12s\t%s\n", mt->name, mt->help);
			}
		return 0;
		}
	mt = getmodecmd(argv[1]);
	if (mt == 0) 
		{
		fprintf(stderr, "Unknown mode '%s' ('mode ?' for help).\n", argv[1]);
		return 0;
		} 
	else if (Ambiguous(mt)) 
		{
		fprintf(stderr, "Ambiguous mode '%s' ('mode ?' for help).\n", argv[1]);
		return 0;
		} 
	else 
		{
		(*mt->handler)();
		}
	return 1;
}

/* -----------------------------------------------------------
 * The following data structures and routines implement the
 * "display" command.
 * -----------------------------------------------------------
 */

static
display( int argc, char **argv)
{
#define	dotog(tl)	if (tl->variable && tl->actionexplanation) { \
			    if (*tl->variable) { \
					tprintf("will"); \
				} else { \
					tprintf("won't"); \
				} \
				tprintf(" %s.\n", tl->actionexplanation); \
				}

#define	doset(sl)   if (sl->name && *sl->name != ' ') { \
			tprintf("[%s]\t%s.\n", control(*sl->charp), sl->name); \
		    }

	struct togglelist *tl;
	struct setlist *sl;

	if (argc == 1) 
		{
		for (tl = Togglelist; tl->name; tl++) 
			{
			dotog(tl);
			}
		tprintf("\n");
		for (sl = Setlist; sl->name; sl++) 
			{
			doset(sl);
			}
		} 
	else 
		{
		int i;

		for (i = 1; i < argc; i++) 
			{
			sl = getset(argv[i]);
			tl = gettoggle(argv[i]);
			if (Ambiguous(sl) || Ambiguous(tl)) 
				{
				tprintf("?Ambiguous argument '%s'.\n", argv[i]);
				return 0;
				} 
			else if (!sl && !tl) 
				{
				tprintf("?Unknown argument '%s'.\n", argv[i]);
				return 0;
				} 
			else 
				{
				if (tl) 
					{
					dotog(tl);
					}
				if (sl) 
					{
					doset(sl);
					}
				}
			}
		}
	return 1;
#undef	doset
#undef	dotog
}

/* -----------------------------------------------------------------
 * The following are the data structures, and many of the routines,
 * relating to command processing.
 * --------------------------------------------------------------
 */

/*
 * Set the escape character.
 */
 
static
setescape(int argc, char **argv)
{
	register char *arg;
	char buf[50];

	tprintf(
		"Deprecated usage - please use 'set escape%s%s' in the future.\n",
				(argc > 2)? " ":"", (argc > 2)? argv[1]: "");
	if (argc > 2)
		{
		arg = argv[1];
		}
	else 
		{
		tprintf("new escape character: ");
		(void) gets(buf);
		arg = buf;
		}
	if (arg[0] != '\0')
		{
		escape = arg[0];
		}
	if (!In3270) 
		{
		tprintf("Escape character is '%s'.\n", control(escape));
		}
	(void) fflush(stdout);
	return 1;
}

/* ---------------------------------- */

/*VARARGS*/
static
togcrmod( void )
{
	crmod = !crmod;
	tprintf("Deprecated usage - please use 'toggle crmod' in the future.\n");
	tprintf("%s map carriage return on output.\n", crmod ? "Will" : "Won't");
	(void) fflush(stdout);
	return 1;
}

/* ---------------------------------- */

/*VARARGS*/
suspend( void )
{
	setcommandmode();
	/* reget parameters in case they were changed */
	TerminalSaveState();
	setconnmode();
	return 1;
}

/* ---------------------------------- */

/*VARARGS*/
static
bye(int argc, char **argv)
     /* int	argc;		     Number of arguments */
	 /* char	*argv[];	 arguments */
{
	if (connected) 
		{
		(void) shutdown(net, 2);
		tprintf("Connection closed.\n");
		(void) NetClose(net);
		connected = 0;
			/* reset options */
		tninit();
#ifdef TN3270
		SetIn3270();		/* Get out of 3270 mode */
#endif	/* defined(TN3270) */
	    }
	
	DB("commands: bye", "calling 'if(argc != 2..)'") ;
	if ((argc != 2) || (strcmp(argv[1], "fromquit") != 0)) 
		{
		DB("commands/bye", "longjmp(toplevel)") ;
		longjmp(toplevel, 1);
		/* NOTREACHED */
		}
	return 1;			/* Keep lint, etc., happy */
}

/* ---------------------------------- */

/*VARARGS*/
quit( void )
{
	(void) call(bye, "bye", "fromquit", 0);
	Exit(0);
	return 1;			/* just to keep lint happy */
}

/* -------------------------------------------------------
 * Print status about the connection.
 * -------------------------------------------------------
 */
 
/*ARGSUSED*/
static
status( int argc, char **argv)
{
	if (connected) 
		{
		tprintf("Connected to %s.\n", hostname);
		if (argc < 2) 
			{
			tprintf("Operating in %s.\n",
				modelist[getconnmode()].modedescriptions);
			if (localchars) 
				{
				tprintf("Catching signals locally.\n");
				}
			}
		} 
	else 
		{
		tprintf("No connection.\n");
		}
#ifdef TN3270
	tprintf("Escape character is '%s'.\n", control(escape));
	(void) fflush(stdout);
#else /* !defined(TN3270) */
	if ((!In3270) && ((argc < 2) || strcmp(argv[1], "notmuch"))) 
		{
		tprintf("Escape character is '%s'.\n", control(escape));
		}
	(void) fflush(stdout);
	if (In3270) 
		{
		return 0;
		}
#endif /* defined(TN3270) */
	return 1;
}


/* ---------------------------------- */

int
tn(int argc, char **argv)
{
	register struct hostent *host = 0;
	struct sockaddr_in sin;
	struct servent *sp = 0;
	static char	hnamebuf[32];
	unsigned long inet_addr();


	DB("commands/tn", "into") ;

	if (connected) 
		{
		tprintf("?Already connected to %s\n", hostname);
		return 0;
		}
	if (argc < 2) 
		{
		(void) strcpy(line, "Connect ");
		tprintf("(to) ");
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
		}
	if ((argc < 2) || (argc > 3)) 
		{
		tprintf("usage: %s host-name [port]\n", argv[0]);
		return 0;
		}
	sin.sin_addr.s_addr = inet_addr(argv[1]);
	if (sin.sin_addr.s_addr != -1) 
		{
		sin.sin_family = AF_INET;
		(void) strcpy(hnamebuf, argv[1]);
		hostname = hnamebuf;
		} 
	else 
		{
		host = gethostbyname(argv[1]);
		if (host) 
			{
			sin.sin_family = host->h_addrtype;
			memcpy((caddr_t)&sin.sin_addr,
				host->h_addr_list[0], host->h_length);
			hostname = host->h_name;
			} 
		else 
			{
			herror(argv[1]);
			return 0;
			}
		}
	if (argc == 3) 
		{
		DB("tn","sin.sin_port") ;
		sin.sin_port = atoi(argv[2]);
		if (sin.sin_port == 0) 
			{
			sp = getservbyname(argv[2], "tcp");
			if (sp)
			sin.sin_port = sp->s_port;
			else 
				{
				tprintf("%s: bad port number\n", argv[2]);
				return 0;
				}
			} 
		else 
			{
			sin.sin_port = atoi(argv[2]);
			sin.sin_port = htons(sin.sin_port);
			}
		telnetport = 0;
		} 
	else 
		{
		if (sp == 0) 
			{
			DB("tn", "calling getservbyname") ;
			sp = getservbyname("telnet", "tcp");
			if (sp == 0) 
				{
				fprintf(stderr, "telnet: tcp/telnet: unknown service\n");
				return 0;
				}
			sin.sin_port = sp->s_port;
			}
		telnetport = 1;
		}
	tprintf("Trying...\n");
	do {
		net = socket(AF_INET, SOCK_STREAM, 0);
		if (net < 0) 
			{
			perror("telnet: socket");
			return 0;
			}
		if (debug && SetSockOpt(net, SOL_SOCKET, SO_DEBUG, 1) < 0) 
			{
			perror("setsockopt (SO_DEBUG)");
			}

		if (connect(net, (struct sockaddr *)&sin, sizeof (sin)) < 0) 
			{
			if (host && host->h_addr_list[1]) 
				{
				int oerrno = errno;
				extern char *inet_ntoa();

				fprintf(stderr, "telnet: connect to address %s: ",
						inet_ntoa(sin.sin_addr));
				errno = oerrno;
				perror((char *)0);
				host->h_addr_list++;
				memcpy((caddr_t)&sin.sin_addr, 
						host->h_addr_list[0], host->h_length);
				fprintf(stderr, "Trying %s...\n",
					inet_ntoa(sin.sin_addr));
				(void) NetClose(net);
				continue;
				}
			perror("telnet: Unable to connect to remote host");
			return 0;
			}
			connected++;
		} while (connected == 0);

	(void) call(status, "status", "notmuch", 0);
	if (setjmp(peerdied) == 0)
		{
		DB("tn", "calling telnet()") ;
		telnet();
		}
	(void) NetClose(net);
	ExitString("Connection closed by foreign host.\n",1);
	/*NOTREACHED*/
}


#define HELPINDENT (sizeof ("connect"))

static char
	openhelp[] =	"connect to a site",
	closehelp[] =	"close current connection",
	quithelp[] =	"exit telnet",
	statushelp[] =	"print status information",
	helphelp[] =	"print help information",
	sendhelp[] =	"transmit special characters ('send ?' for more)",
	sethelp[] = 	"set operating parameters ('set ?' for more)",
	togglestring[] ="toggle operating parameters ('toggle ?' for more)",
	displayhelp[] =	"display operating parameters",
#ifdef TN3270
	shellhelp[] =	"invoke a subshell",
#endif	/* defined(TN3270) */
	modehelp[] = "try to enter line-by-line or character-at-a-time mode";

extern int	help(), shell();

static Command cmdtab[] = {
	{ "close",	closehelp,	bye,		1, 1 },
	{ "display",	displayhelp,	display,	1, 0 },
	{ "mode",	modehelp,	modecmd,	1, 1 },
	{ "open",	openhelp,	tn,		1, 0 },
	{ "quit",	quithelp,	quit,		1, 0 },
	{ "send",	sendhelp,	sendcmd,	1, 1 },
	{ "set",	sethelp,	setcmd,		1, 0 },
	{ "status",	statushelp,	status,		1, 0 },
	{ "toggle",	togglestring,	toggle,		1, 0 },
#ifdef TN3270
	{ "!",		shellhelp,	shell,		1, 1 },
#endif	/* defined(TN3270) */
	{ "?",		helphelp,	help,		1, 0 },
	0
};

static char	crmodhelp[] =	"deprecated command -- use 'toggle crmod' instead";
static char	escapehelp[] =	"deprecated command -- use 'set escape' instead";

static Command cmdtab2[] = {
	{ "help",	helphelp,	help,		0, 0 },
	{ "escape",	escapehelp,	setescape,	1, 0 },
	{ "crmod",	crmodhelp,	togcrmod,	1, 0 },
	0
};


/* --------------------------------------------------------------
 * Call routine with argc, argv set from args (terminated by 0).
 * --------------------------------------------------------------
 */

/*VARARGS1*/
static
call(va_alist)
va_dcl
{
	va_list ap;
	typedef int (*intrtn_t)();
	intrtn_t routine;
	char *args[100];
	int argno = 0;

	va_start(ap);
	routine = (va_arg(ap, intrtn_t));
	while ((args[argno++] = va_arg(ap, char *)) != 0) 
		{
			;
		}
	va_end(ap);
	return (*routine)(argno-1, args);
}

/* ---------------------------------- */

static char **
getnextcmd( char *name )
{
	Command *c = (Command *) name;

	return (char **) (c+1);
}

/* ---------------------------------- */

static Command *
getcmd(char *name)
{
	Command *cm;

	if ((cm = (Command *) genget(name, (char **) cmdtab, getnextcmd)) != 0) 
		{
		return cm;
		} 
	else 
		{
		return (Command *) genget(name, (char **) cmdtab2, getnextcmd);
		}
}

/* ---------------------------------- */

void
command(int top)
{
	register Command *c;

	setcommandmode();
	if (!top) 
		{
		putchar('\n');
		}
		
#ifdef DEBUG
printf("command: into (;;)\n") ;
#endif

	for (;;) 
		{
		tprintf("%s> ", prompt);
		if (gets(line) == NULL) 
			{
			if (feof(stdin) || ferror(stdin))
			quit();
			break;
			}
		if (line[0] == 0)
			break;
		makeargv();
		c = getcmd(margv[0]);
		if (Ambiguous(c)) 
			{
			tprintf("?Ambiguous command\n");
			continue;
			}
		if (c == 0) 
			{
			tprintf("?Invalid command\n");
			continue;
			}
		if (c->needconnect && !connected) 
			{
			tprintf("?Need to be connected first.\n");
			continue;
			}
		if ((*c->handler)(margc, margv)) 
			{
			break;
			}
		}
	if (!top) 
		{
		if (!connected) 
			{
			DB("commands/command()", "if (!connected) longjmp(toplevel)") ;
			longjmp(toplevel, 1);
			/*NOTREACHED*/
			}
#ifdef TN3270
		if (shell_active == 0) 
			{
			setconnmode();
			}
#else	/* defined(TN3270) */
		setconnmode();
#endif	/* defined(TN3270) */
		}
}

/* --------------------------------------------
 * Help command.
 * -------------------------------------------
 */
 
static
help(int argc, char **argv)
{
	register Command *c;
	register char *arg;

	if (argc == 1) 
		{
		tprintf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c->name; c++)
			{
			if (c->dohelp) 
				{
				tprintf("%-*s\t%s\n", HELPINDENT, c->name,
								    c->help);
				}
			}
		return 0;
		}
	while (--argc > 0) 
		{
		arg = *++argv;
		c = getcmd(arg);
		if (Ambiguous(c))
			{
			tprintf("?Ambiguous help command %s\n", arg);
			}
		else if (c == (Command *)0)
			{
			tprintf("?Invalid help command %s\n", arg);
			}
		else
			{
			tprintf("%s\n", c->help);
			}
		}
	return 0;
}
