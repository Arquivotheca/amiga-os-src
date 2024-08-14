/* DEFAULT RULES FOR UNIX */

/***********************************************************************\
*																		*
*	KNK		CMDSUFFIX and CMDPREFIX macro handling included				*
*																		*
\***********************************************************************/

/* #include <proto/all.h> */

char *builtin[] =
{
    ".SUFFIXES : .o .c ",
    "LFLAGS=",
    "CC=lc",
    "CC1=lc1 -.",
    "CC3=lc2 -.",
    "AS=casm -a",
    "CFLAGS=",
    "CMDPREFIX=SIGNAL ON ERROR; SIGNAL ON BREAK_C;",
	"CMDSUFFIX=;EXIT 0; ERROR: EXIT RC; BREAK_C: EXIT 1700",
    ".c.o :",
    "\t@\"$(CC1) $(C1FLAGS) $(CFLAGS) -oQUAD: $*\"; \"$(CC3) $(C3FLAGS) -o$*.o QUAD:$*.q\"",

    0 };

#include "defs.h"
#include	<exec/memory.h>


TIMETYPE exists(filename)
char *filename;
{
	BPTR					TheFileLock;
	struct FileInfoBlock	*theInfo;
	ULONG					TheTime;
	int						returncode;
    register char *s;

    for(s = filename ; *s!='\0' && *s!='(' ; ++s);

	TheTime = 0L;
	TheFileLock = Lock(filename, SHARED_LOCK);
	if(TheFileLock) {
		if (!(theInfo = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR | MEMF_PUBLIC))) {
			UnLock(TheFileLock);
			fatal("Out of memory");
			}
		if(returncode = Examine(TheFileLock, theInfo))
			TheTime = (ULONG)theInfo->fib_Date.ds_Tick / 60L +
						(ULONG)theInfo->fib_Date.ds_Minute * 3600L +
						  (ULONG)theInfo->fib_Date.ds_Days * 24L * 60L * 3600L;
		FreeMem(theInfo, sizeof(struct FileInfoBlock));
		UnLock(TheFileLock);
		}
	return(TheTime);
}


TIMETYPE prestime()
{
    TIMETYPE t;
    time(&t);
    return(t);
}



FSTATIC char nbuf[MAXNAMLEN + 1];
FSTATIC char *nbufend	= &nbuf[MAXNAMLEN];



struct depblock *srchdir(pat, mkchain, nextdbl)
register char	*pat; 					/* pattern to be matched in directory */
int				 mkchain;				/* nonzero if results to be remembered */
struct depblock *nextdbl;  				/* final value for chain */
{
	char				*dirname,
						*dirpref,
						*endir,
						*filepat,
						*p,
						*p1,
						*p2,
						 temp[100],
						 fullname[100];
	BPTR				 dirf,
						 Lock();
   	int					 success,
						 cldir;
    struct nameblock	*q;
    struct depblock		*thisdbl;
    struct dirhdr		*od;
    struct pattern		*patp;
   struct FileInfoBlock *dptr;



    thisdbl = NULL;

    if(mkchain == NO)
		for(patp=firstpat ; patp ; patp = patp->nxtpattern)
		    if(! unequal(pat, patp->patval))
				return(0);

    patp = ALLOC(pattern);
    patp->nxtpattern = firstpat;
    firstpat = patp;
    patp->patval = copys(pat);

    endir = NULL;

    for(p=pat; *p!='\0'; ++p)
		if(*p=='/') endir = p;

    if(endir==NULL) {
		if(!(dirname = (char *)AllocMem(MAXNAMLEN + 1, MEMF_PUBLIC | MEMF_CLEAR)))
			fatal ("cannot allocate memory for directory name");
		if(!getcwd(dirname,MAXNAMLEN)) {
			FreeMem(dirname, MAXNAMLEN + 1);
			fatal("Could not find current directory");
			}

		if (dbgflag)
			printf("current directory name is (hopefully) \"%s\".\n",dirname);

		dirpref = "";
		filepat = pat;
	    }
    else    {
		dirname = pat;
		*endir = '\0';
		dirpref = concat(dirname, "/", temp);
		filepat = endir+1;
	    }

    dirf = NULL;
    cldir = NO;

    for(od = firstod; od; od = od->nxtopendir)
		if(! unequal(dirname, od->dirn) ) {
		    dirf = od->dirfc;
		    break;
			}
    if(dirf == NULL) {
		dirf = Lock(dirname, SHARED_LOCK);
		if(nopdir >= MAXDIR)
		    cldir = YES;
		else	{
		    ++nopdir;
		    od = ALLOC(dirhdr);
		    od->nxtopendir = firstod;
		    firstod = od;
		    od->dirfc = dirf;
		    od->dirn = copys(dirname);
			}
    	}

    if(dirf == NULL) {
		fprintf(stderr, "Directory %s: ", dirname);
		FreeMem(dirname, MAXNAMLEN+1);
		fatal("Cannot open");
	    }

    else
		if((dptr = ALLOC(FileInfoBlock)) == NULL) {
			fprintf(stderr, "Directory %s: ", dirname);
			FreeMem(dirname, MAXNAMLEN+1);
			fatal("cannot allocate memory for FileInfoBlock");
			}
		
		for (success = Examine(dirf, dptr); success; success = ExNext(dirf, dptr)) {
			p1 = dptr->fib_FileName;
			p2 = nbuf;
			while( (p2<nbufend) && (*p2++ = *p1++)!='\0' );
			if( amatch(nbuf,filepat) ) {
			    concat(dirpref,nbuf,fullname);
			    if( (q=srchname(fullname)) ==0)
					q = makename(copys(fullname));
	    		if(mkchain) {
					thisdbl = ALLOC(depblock);
					thisdbl->nxtdepblock = nextdbl;
					thisdbl->depname = q;
					nextdbl = thisdbl;
	    			}
				}
    		}

	if(endir != 0)
		*endir = '/';

    if(cldir) {
		UnLock(dirf);
		FreeMem(dirname, MAXNAMLEN+1);
		dirf = NULL;
	    }

	free(dptr);
    return(thisdbl);
}

/* stolen from glob through find */

static amatch(s, p)
char *s, *p;
{
    register int cc, scc, k;
    int c, lc;

    scc = *s;
    lc = 077777;
    switch (c = *p) {

    	case '[':
			k = 0;
			while (cc = *++p) {
	    		switch (cc) {

	    			case ']':
						if (k)
			    			return(amatch(++s, ++p));
						else
			    			return(0);
	
	    			case '-':
						k |= (lc <= scc)  & (scc <= (cc=p[1]) ) ;
	    			}
	    		if (scc==(lc=cc)) k++;
				}
			return(0);

    	case '?':
		caseq:
			if(scc) return(amatch(++s, ++p));
			return(0);

		case '*':
			return(umatch(s, ++p));

		case 0:
			return(!scc);
    	}
    if (c==scc)
		goto caseq;
    return(0);
}

static umatch(s, p)
char *s, *p;
{
    if(*p==0) return(1);
    while(*s)
		if (amatch(s++,p))
			return(1);
    return(0);
}

eqstr(a,b,n)
register char *a, *b;
int n;
{
    register int i;
    for(i = 0 ; i < n ; ++i)
		if(*a++ != *b++)
		    return(NO);
    return(YES);
}
