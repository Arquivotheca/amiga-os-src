/*
 *	Because some systems still don't have getopt, the source to a
 *	public domain version is included below.  This is derived from the
 *	actual AT&T version, which they released as public domain at the
 *	1985 UNIFORUM conference in Dallas.
 *
 */


#include "globals.h"

#if !HAVE_GETOPT

#ifndef NULL
#define NULL 0
#endif

#ifndef EOF
#define EOF (-1)
#endif

static int error PROTO((char **argv, char *s, int c));
static char *strchr PROTO((char *s, int c));

int opterr = 1;
int optind = 1;
int optopt;
char *optarg;

static int error (argv, s, c)
char **argv;
char *s;
int c;
{
    char errbuf[2];

    errbuf[0] = c;
    errbuf[1] = '\n';
    write (2, argv[0], (unsigned) strlen (argv[0]));
    write (2, s, (unsigned) strlen (s));
    write (2, errbuf, (unsigned) 2);
}

/* Some systems don't have strchr, use our own */

static char *strchr (s, c)
char *s;
char c;
{
    char *rtnval = NULL;

    if (s != NULL) {
	while (*s != '\000' && *s != c) {s++;}
	if (*s == c) {
	    rtnval = s;
	}
    }
    return (rtnval);
}

int getopt (argc, argv, opts)
int argc;
char **argv;
char *opts;
{
    static int sp = 1;
    int c;
    char *cp;

    if (sp == 1) {
	if (optind >= argc 
	    || argv[optind][0] != '-' || argv[optind][1] == '\0') {
	    return (EOF);
	} else if (strcmp (argv[optind], "--") == 0) {
	    optind++;
	    return (EOF);
	}
    }
    optopt = c = argv[optind][sp];
    if(c == ':' || ((cp = strchr (opts, c)) == NULL)) {
	if (opterr) {
	    error (argv, ": illegal option -- ", c);
	}
	if(argv[optind][++sp] == '\0') {
	    optind++;
	    sp = 1;
	}
	return ('?');
    }
    if(*++cp == ':') {
	if(argv[optind][sp+1] != '\0') {
	    optarg = &argv[optind++][sp+1];
	} else if (++optind >= argc) {
	    if (opterr) {
		error (argv, ": option requires an argument -- ", c);
	    }
	    sp = 1;
	    return ('?');
	} else {
	    optarg = argv[optind++];
	}
	sp = 1;
    } else {
	if (argv[optind][++sp] == '\0') {
	    sp = 1;
	    optind++;
	}
	optarg = NULL;
    }
    return (c);
}

#else

#if defined(__STDC__)
static int dummy;	/* Avoid "empty translation unit" messages */
#endif

#endif
