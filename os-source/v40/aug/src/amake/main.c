static	char *sccsid = "@(#)main.c  4.5 (Berkeley) 83/03/03";

#include "defs.h"
#include	<exec/lists.h>

/*
command make to update programs.
Flags:	'd'  print out debugging comments
    'p'	 print out a version of the input graph
    's'	 silent mode--don't print out commands
    'f'	 the next argument is the name of the description file;
	 "makefile" is the default
    'i'	 ignore error codes from the shell
    'S'	 stop after any command fails (normally do parallel work)
    'n'	  don't issue, just print, commands
    't'	  touch (update time of) files but don't issue command
    'q'	  don't do anything, but check if object is up to date;
	  returns exit code 0 if up to date, -1 if not
*/

struct nameblock *mainname  = NULL;
struct nameblock *firstname = NULL;
struct lineblock *sufflist  = NULL;
struct varblock *firstvar   = NULL;
struct pattern *firstpat    = NULL;
struct dirhdr *firstod	    = NULL;

#include <signal.h>
int sigivalue	= 0;
int sigqvalue	= 0;
int waitpid = 0;

int dbgflag = NO;
int prtrflag	= NO;
int silflag = NO;
int noexflag	= NO;
int keepgoing	= NO;
int noruleflag	= NO;
int touchflag	= NO;
int questflag	= NO;
int ndocoms = NO;
int ignerr  = NO;    /* default is to stop on error */
int okdel   = YES;
int inarglist;
#ifdef pwb
char *prompt	= ">";	/* other systems -- pick what you want */
#else
char *prompt	= "";	/* other systems -- pick what you want */
#endif
int nopdir  = 0;
char junkname[20];
char funny[128];
char	options[26 + 1] = { 
    '-' };
    
#ifdef	AMIGA
#	include	<exec/lists.h>
#	include	<exec/ports.h>
/* #	include	<proto/all.h> */
#	include	<rexx/rxslib.h>

	struct RxsLib	*RexxSysBase;
	struct	List	 PortList;
	struct	MsgPort	*MyPort, *OpenPublicPort();
	struct	MsgPort	*REXXPort, *FindPort(), *CreatePort();
#endif

main(argc,argv)
int argc;
char *argv[];
{
    register struct nameblock *p;
    int i, j;
    int descset, nfargs;
    TIMETYPE tjunk;
    char c, *s;
    static char onechar[2] = "X";
#ifdef unix
    int intrupt();
#endif

    char *op = options + 1;
    char *arglist;

#ifdef METERFILE
    meter(METERFILE);
#endif

#ifdef AMIGA
	RexxSysBase = (struct RxsLib *)OpenLibrary(RXSNAME, 0L);
	if (!RexxSysBase)
		fatal("Could not open the REXX library.");
	if ( !(REXXPort = FindPort("REXX")) )
		fatal("Could not find AREXX.  Are you sure it's loaded?");
	InitList(&PortList);
	MyPort = CreatePort(NULL, 0);
#endif


    descset = 0;

    funny['\0'] = (META | TERMINAL);
    for(s = "=|^();&<>*?[]:$`'\"\\\n" ; *s ; ++s)
		funny[*s] |= META;
    for(s = "\n\t ;&>|" ; *s ; ++s)
		funny[*s] |= TERMINAL;


    inarglist = 1;

    /* compute how much room we need for the argument list */
    for( i = 1, j = 1; i < argc; i++ )
		if( argv[i] != 0 && argv[i][0] != '-' )
		    j += strlen( argv[i] ) + 1;

    if( (arglist = (char *)calloc( j, sizeof(char) )) == NULL )
		fatal( "out of memory" );
	
    for(i=1; i<argc; ++i)
		if( argv[i]!=0 ) {
	    	if( argv[i][0] == '-' ) {
				/* scan for a -f flag */
				if( index( argv[i], 'f' ) )
		    		i++;
				}
	    	else {
				strcat( arglist, " " );
				strcat( arglist, argv[i] );
				if( eqsign(argv[i]) )
		    		argv[i] = 0;
	    		}
			}

    while( *arglist == ' ' )
		arglist++;

    setvar("$","$");
    setvar("MARGS", arglist );
    inarglist = 0;

    for (i=1; i<argc; ++i)
		if (argv[i]!=0 && argv[i][0]=='-') {
	    	for (j=1 ; (c=argv[i][j])!='\0' ; ++j) {
				*op++ = c;
				switch (c) {

					case 'd':
		    			dbgflag = YES;
		    			break;

					case 'p':
		    			prtrflag = YES;
		    			break;

					case 's':
		    			silflag = YES;
		    			break;

					case 'i':
		    			ignerr = YES;
		    			break;

					case 'S':
		    			keepgoing = NO;
		    			break;
		
					case 'k':
		    			keepgoing = YES;
		    			break;

					case 'n':
		    			noexflag = YES;
		    			break;
	
					case 'r':
		    			noruleflag = YES;
		    			break;

					case 't':
		    			touchflag = YES;
		    			break;

					case 'q':
		    			questflag = YES;
		    			break;

					case 'f':
		    			op--;	/* don't pass this one */
		    			if(i >= argc-1)
							fatal("No description argument after -f flag");
		    			if( argv[i+1] && rddescf(argv[i+1]) )
							fatal1("Cannot open %s", argv[i+1]);
		    			argv[i+1] = 0;
		    				++descset;
		    			break;

					default:
		    			onechar[0] = c; /* to make lint happy */
		    			fatal1("Unknown flag argument %s", onechar);
					}
			    }
		    argv[i] = 0;
			}

   	*op++ = '\0';
    if (strcmp(options, "-") == 0)
		*options = '\0';
    setvar("MFLAGS", options);	    /* MFLAGS=options to make */

    if( !descset )
#ifdef UNIX
		if( rddescf("makefile") )
			rddescf("Makefile");
#endif
#ifdef AMIGA
		if( rddescf("makefile") )
			rddescf("Makefile");
#endif
#ifdef gcos
    	rddescf("makefile");
#endif

    if(prtrflag) printdesc(NO);

    if( srchname(".IGNORE") )
		++ignerr;
    if( srchname(".SILENT") )
		silflag = 1;
    if(p=srchname(".SUFFIXES"))
		sufflist = p->linep;
    if( !sufflist )
		fprintf(stderr,"No suffix list.\n");

#ifdef unix
    sigivalue = (int) signal(SIGINT, SIG_IGN) & 01;
    sigqvalue = (int) signal(SIGQUIT, SIG_IGN) & 01;
    enbint(intrupt);
#endif

    nfargs = 0;

    for(i=1; i<argc; ++i)
		if((s=argv[i]) != 0) {
			if((p=srchname(s)) == 0)
				p = makename(s);
			++nfargs;
	    	doname(p, 0, &tjunk);
			if(dbgflag)
				printdesc(YES);
			}

    /*
    If no file arguments have been encountered, make the first
    name encountered that doesn't start with a dot
    */

		if(nfargs == 0)
			if(mainname == 0)
				fatal("No arguments or description file");
			else	{
				doname(mainname, 0, &tjunk);
				if(dbgflag)
					printdesc(YES);
				}
#ifdef	AMIGA
	CleanUp();
#endif
	exit(0);
}

#ifdef UNIX
#include <sys/stat.h>
#endif

#ifdef unix
intrupt()
{
    struct varblock *varptr();
    char *p;
    TIMETYPE exists();
    struct stat sbuf;

    if(okdel && !noexflag && !touchflag &&
	(p = varptr("@")->varval) &&
	(stat(p, &sbuf) >= 0 && (sbuf.st_mode&S_IFMT) == S_IFREG) &&
	!isprecious(p) )
    {
	fprintf(stderr, "\n***	%s removed.", p);
	unlink(p);
    }

    if(junkname[0])
	unlink(junkname);
    fprintf(stderr, "\n");
    exit(2);
}




isprecious(p)
char *p;
{
    register struct lineblock *lp;
    register struct depblock *dp;
    register struct nameblock *np;

    if(np = srchname(".PRECIOUS"))
	for(lp = np->linep ; lp ; lp = lp->nxtlineblock)
	    for(dp = lp->depp ; dp ; dp = dp->nxtdepblock)
		if(! unequal(p, dp->depname->namep))
		    return(YES);

    return(NO);
}


enbint(k)
int (*k)();
{
    if(sigivalue == 0)
	signal(SIGINT,k);
    if(sigqvalue == 0)
	signal(SIGQUIT,k);
}
#endif

extern char *builtin[];

char **linesptr = builtin;

FILE * fin;
int firstrd = 0;

rddescf(descfile)
char *descfile;
{
    FILE * k;

    /* read and parse description */

    if( !firstrd++ )
    {
	if( !noruleflag ) {
	    filename = "<builtin rules>";
	    rdd1( (FILE *) NULL);
	}

#ifdef pwb
	{
	    char *nlog, s[100];
	    nlog = logdir();
	    if ( (k=fopen( concat(nlog,"/makecomm",s), "r")) != NULL)
		rdd1(k);
	    else if ( (k=fopen( concat(nlog,"/Makecomm",s), "r")) != NULL)
		rdd1(k);

	    if ( (k=fopen("makecomm", "r")) != NULL)
		rdd1(k);
	    else if ( (k=fopen("Makecomm", "r")) != NULL)
		rdd1(k);
	}
#endif

    }
    if(! unequal(descfile, "-")) {
	filename = "<stdin>";
	return( rdd1(stdin) );
    }

    if( (k = fopen(descfile,"r")) != NULL) {
	filename = copys(descfile);
	return( rdd1(k) );
    }

    return(1);
}




rdd1(k)
FILE * k;
{
    extern int yylineno;
    extern char *zznextc;

    fin = k;
    yylineno = 0;
    zznextc = 0;

    if( yyparse() )
	fatal("Description file error");

    if(fin != NULL)
	fclose(fin);

    return(0);
}

printdesc(prntflag)
int prntflag;
{
    struct nameblock *p;
    struct depblock *dp;
    struct varblock *vp;
#ifdef UNIX
    struct dirhdr *od;
#endif
    struct shblock *sp;
    struct lineblock *lp;

#ifdef UNIX
    if(prntflag)
    {
	printf("Open directories:\n");
	for (od = firstod; od; od = od->nxtopendir)
	    printf("\t%d: %s\n", od->dirfc->dd_fd, od->dirn);
    }
#endif

    if(firstvar != 0) printf("Macros:\n");
    for(vp = firstvar; vp ; vp = vp->nxtvarblock)
	printf("\t%s = %s\n" , vp->varname , vp->varval);

    for(p = firstname; p; p = p->nxtnameblock)
    {
	printf("\n\n%s",p->namep);
	if(p->linep != 0) printf(":");
	if(prntflag) printf("  done=%d",p->done);
	if(p==mainname) printf("  (MAIN NAME)");
	for(lp = p->linep ; lp ; lp = lp->nxtlineblock)
	{
	    if( dp = lp->depp )
	    {
		printf("\n depends on:");
		for(; dp ; dp = dp->nxtdepblock)
		    if(dp->depname != 0)
			printf(" %s ", dp->depname->namep);
	    }

	    if(sp = lp->shp)
	    {
		printf("\n commands:\n");
		for( ; sp!=0 ; sp = sp->nxtshblock)
		    printf("\t%s\n", sp->shbp);
	    }
	}
    }
    printf("\n");
    fflush(stdout);
    return(0);
}

#ifdef AMIGA
CleanUp()
{
	if (MyPort)
		DeletePort(MyPort);
	return(0);
}
#endif
