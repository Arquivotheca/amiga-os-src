/*
 *	make [-f makefile] [-ins] [target(s) ...]
 *
 *	(Better than EON mk but not quite as good as UNIX make)
 *
 *	-f makefile name
 *	-i ignore exit status
 *	-n Pretend to make
 *	-p Print all macros & targets
 *	-q Question up-to-dateness of target.  Return exit status 1 if not
 *	-r Don't not use inbuilt rules
 *	-s Make silently
 *	-t Touch files instead of making them
 *	-m Change memory requirements (EON only)
 */

/* note: stderr in this file changed to stdout for amiga use */

#include <stdio.h>
#include "h.h"

#ifdef unix
#include <sys/errno.h>
#endif
#ifdef eon
#include <sys/err.h>
#endif
#ifdef os9
#include <errno.h>
#endif
#ifdef amiga
#include <errno.h>
#endif

#ifdef LATTICE
#include <errno.h>
#include <string.h>
void usage();

int CXBRK(void) {return(0);}

#endif

#ifdef eon
#define MEMSPACE	(16384)
#endif


char           *myname;
char           *makefile;	/* The make file  */
#ifdef eon
unsigned        memspace = MEMSPACE;
#endif

FILE           *ifd;		/* Input file desciptor  */
bool            domake = TRUE;	/* Go through the motions option  */
bool            ignore = FALSE;	/* Ignore exit status option  */
bool            silent = FALSE;	/* Silent option  */
bool            print = FALSE;	/* Print debuging information  */
bool            rules = TRUE;	/* Use inbuilt rules  */
bool            dotouch = FALSE;/* Touch files instead of making  */
bool            quest = FALSE;	/* Question up-to-dateness of file  */


void main(argc, argv)
    int             argc;
    char          **argv;
{
    register char  *p;		/* For argument processing  */
    int             estat = 0;	/* For question  */
    register struct name *np;
    void            prt(), circh();


    myname = (argc-- < 1) ? "make" : *argv++;

    while ((argc > 0) && (**argv == '-')) {
	argc--;			/* One less to process  */
	p = *argv++;		/* Now processing this one  */

	while (*++p != '\0') {
	    switch (*p) {
	    case 'f':		/* Alternate file name  */
		if (*++p == '\0') {
		    if (argc-- <= 0)
			usage();
		    p = *argv++;
		}
		makefile = p;
		goto end_of_args;
#ifdef eon
	    case 'm':		/* Change space requirements  */
		if (*++p == '\0') {
		    if (argc-- <= 0)
			usage();
		    p = *argv++;
		}
		memspace = atoi(p);
		goto end_of_args;
#endif
	    case 'n':		/* Pretend mode  */
		domake = FALSE;
		break;
	    case 'i':		/* Ignore fault mode  */
		ignore = TRUE;
		break;
	    case 's':		/* Silent about commands  */
		silent = TRUE;
		break;
	    case 'p':
		print = TRUE;
		break;
	    case 'r':
		rules = FALSE;
		break;
	    case 't':
		dotouch = TRUE;
		break;
	    case 'q':
		quest = TRUE;
		break;
	    default:		/* Wrong option  */
		usage();
	    }
	}
end_of_args:;
    }


#ifdef amiga
    if ((ifd = fopen("s:builtins.make", "r")) != (FILE *) 0) {
	input(ifd);
	fclose(ifd);
    } else
#endif
    makerules();

#ifdef eon
    if (initalloc(memspace) == 0xffff)	/* Must get memory for alloc  */
	fatal("Cannot initalloc memory");
#endif

    if (!makefile) {	/* If no file, then use default */
#ifdef amiga
	ifd = fopen(DEFN0, "r");
#endif
	if ((ifd ==(FILE *) 0) && ((ifd = fopen(DEFN1, "r")) == (FILE *) 0))
#ifdef eon
	    if (errno != ER_NOTF)
		fatal("Can't open %s; error %02x", DEFN1, errno);
#endif
#ifdef unix
	if (errno != ENOENT)
	    fatal("Can't open %s; error %02x", DEFN1, errno);
#endif
#ifdef amiga
	if (errno != ENOENT)
	    fatal("Can't open %s; error %02x", DEFN0, errno);
#endif
#ifdef DEFN2
	if ((ifd == (FILE *) 0)
	    && ((ifd = fopen(DEFN2, "r")) == (FILE *) 0))
	    fatal("Can't open %s", DEFN2);
#else
	else
	    fatal("Can't open %s", DEFN1);
#endif
    } else if (strcmp(makefile, "-") == 0)	/* Can use stdin as makefile  */
	ifd = stdin;
    else if ((ifd = fopen(makefile, "r")) == (FILE *) 0)
	fatal("Can't open %s", makefile);

    setmacro("$", "$");

    while (argc && (p = index(*argv, '='))) {
	char            c;

	c = *p;
	*p = '\0';
	setmacro(*argv, p + 1);
	*p = c;

	argv++;
	argc--;
    }

    input(ifd);			/* Input all the gunga  */
    fclose(ifd);		/* Finished with makefile  */
    lineno = 0;			/* Any calls to error now print no line
				 * number */

    if (print) prt();			/* Print out structures  */

    np = newname(".SILENT");
    if (np->n_flag & N_TARG)
	silent = TRUE;

    np = newname(".IGNORE");
    if (np->n_flag & N_TARG)
	ignore = TRUE;

    precious();

    if (!firstname) fatal("No targets defined");

    circh();			/* Check circles in target definitions  */

    if (!argc)
	estat = make(firstname, 0);
    else
	while (argc--) {
	    estat |= make(newname(*argv++), 0);
	}

    if (quest)exit(estat);
    else
	exit(0);
}


void usage()
{
    fprintf(stdout, "Usage: %s [-f makefile] [-inpqrst] [macro=val ...] [target(s) ...]\n", myname);
    exit(1);
}


void
fatal(msg, a1, a2, a3, a4, a5, a6)
    char           *msg;
{
    fprintf(stdout, "%s: ", myname);
    fprintf(stdout, msg, a1, a2, a3, a4, a5, a6);
    fputc('\n', stdout);
    exit(1);
}
