%{#include "defs"
static	char *sccsid = "@(#)gram.y	4.1 (Berkeley) 81/02/28";
%}

%term NAME SHELLINE START MACRODEF COLON DOUBLECOLON GREATER
%union
	{
	struct shblock *yshblock;
	struct depblock *ydepblock;
	struct nameblock *ynameblock;
	}

%type <yshblock> SHELLINE, shlist, shellist
%type <ynameblock> NAME, namelist
%type <ydepblock> deplist, dlist


%%

%{
struct depblock *pp;
FSTATIC struct shblock *prevshp;

FSTATIC struct nameblock *lefts[NLEFTS];
struct nameblock *leftp;
FSTATIC int nlefts;

struct lineblock *lp, *lpp;
FSTATIC struct depblock *prevdep;
FSTATIC int sepc;
%}


file:
	| file comline
	;

comline:  START
	| MACRODEF
	| START namelist deplist shellist = {
	    while( --nlefts >= 0)
		{
		leftp = lefts[nlefts];
		if(leftp->septype == 0)
			leftp->septype = sepc;
		else if(leftp->septype != sepc)
			fprintf(stderr, "Inconsistent rules lines for `%s'\n",
				leftp->namep);
		else if(sepc==ALLDEPS && *(leftp->namep)!='.' && $4!=0)
			{
			for(lp=leftp->linep; lp->nxtlineblock!=0; lp=lp->nxtlineblock)
			    if(lp->shp)
				fprintf(stderr, "Multiple rules lines for `%s'\n",
				    leftp->namep);
			}

		lp = ALLOC(lineblock);
		lp->nxtlineblock = NULL;
		lp->depp = $3;
		lp->shp = $4;

		if(! unequal(leftp->namep, ".SUFFIXES") && $3==0)
			leftp->linep = 0;
		else if(leftp->linep == 0)
			leftp->linep = lp;
		else	{
			for(lpp = leftp->linep; lpp->nxtlineblock;
				lpp = lpp->nxtlineblock) ;
				if(sepc==ALLDEPS && leftp->namep[0]=='.')
					lpp->shp = 0;
			lpp->nxtlineblock = lp;
			}
		}
	}
	| error
	;

namelist: NAME	= { lefts[0] = $1; nlefts = 1; }
	| namelist NAME	= { lefts[nlefts++] = $2;
	    	if(nlefts>=NLEFTS) fatal("Too many lefts"); }
	;

deplist:
		{
		char junk[INMAX];
		sprintf(junk, "file %s, line %d", filename, yylineno);
		fatal1("Must be a separator on rules: %s", junk);
		}
	| dlist
	;

dlist:  sepchar	= { prevdep = 0;  $$ = 0; }
	| dlist NAME	= {
			  pp = ALLOC(depblock);
			  pp->nxtdepblock = NULL;
			  pp->depname = $2;
			  if(prevdep == 0) $$ = pp;
			  else  prevdep->nxtdepblock = pp;
			  prevdep = pp;
			  }
	;

sepchar:  COLON 	= { sepc = ALLDEPS; }
	| DOUBLECOLON	= { sepc = SOMEDEPS; }
	;

shellist:	= {$$ = 0; }
	| shlist = { $$ = $1; }
	;

shlist:	SHELLINE   = { $$ = $1;  prevshp = $1; }
	| shlist SHELLINE = { $$ = $1;
			prevshp->nxtshblock = $2;
			prevshp = $2;
			}
	;

%%

char *zznextc;	/* zero if need another line; otherwise points to next char */
int yylineno;
extern FILE * fin;

yylex()
{
    register char *p;
    register char *q;
    char word[INMAX];

    if(zznextc == 0)
	return( nextlin() );

    while( isspace(*zznextc) )
	++zznextc;

    if(*zznextc == '\0')
	return( nextlin() );

    if(*zznextc == ':')
    {
	if(*++zznextc == ':')
	{
	    ++zznextc;
	    return(DOUBLECOLON);
	}
	else	return(COLON);
    }

    if(*zznextc == '>')
    {
	++zznextc;
	return(GREATER);
    }

    if(*zznextc == ';')
	return( retsh(zznextc) );

    p = zznextc;
    q = word;

    while( ! ( funny[*p] & TERMINAL) )
	*q++ = *p++;

    if(p != zznextc)
    {
	*q = '\0';
	if((yylval.ynameblock=srchname(word))==0)
	    yylval.ynameblock = makename(word);
	zznextc = p;
	return(NAME);
    }

    else    {
	fprintf(stderr,"Bad character %c (octal %o), file %s line %d",
	*zznextc,*zznextc,filename,yylineno);
	fatal( (char *) NULL );
    }
    return(0);	/* never executed */
}





retsh(q)
char *q;
{
    register char *p;
    struct shblock *sp;
    char *copys();

    for(p=q+1 ; *p==' '||*p=='\t' ; ++p)  ;

    sp = ALLOC(shblock);
    sp->nxtshblock = NULL;
    sp->shbp = (fin == NULL ? p : copys(p) );
    yylval.yshblock = sp;
    zznextc = 0;
    return(SHELLINE);
}

nextlin()
{
    static char yytext[INMAX];
    static char *yytextl    = yytext+INMAX;
    char *text, templin[INMAX];
    register char c;
    register char *p, *t;
    char lastch, *lastchp;
    extern char **linesptr;
    int incom;
    int kc;

again:

    incom = NO;
    zznextc = 0;

    if(fin == NULL)
    {
	if( (text = *linesptr++) == 0)
	    return(0);
	++yylineno;
    }

    else    {
	for(p = text = yytext ; p<yytextl ; *p++ = kc)
	    switch(kc = getc(fin))
	    {
	    case '\t':
		if(p != yytext)
		    break;
	    case ';':
		incom = YES;
		break;

	    case '#':
		if(! incom)
		    kc = '\0';
		break;

	    case '\n':
		++yylineno;
		if(p==yytext || p[-1]!='\\')
		{
		    *p = '\0';
		    goto endloop;
		}
		p[-1] = ' ';
		while( (kc=getc(fin))=='\t' || kc==' ' || kc=='\n')
		    if(kc == '\n')
			++yylineno;

		if(kc != EOF)
		    break;
	    case EOF:
		if( popfile() ) goto again;
		return(0);
	    }

	fatal("line too long");
    }

endloop:

    if((c = text[0]) == '\t')
	return( retsh(text) );

    if(isalpha(c) || isdigit(c) || c==' ' || c=='.')
	for(p=text+1; *p!='\0'; )
	    if(*p == ':')
		break;
	    else if(*p++ == '=')
	    {
		pushfile( text );
		eqsign(text);
		return(MACRODEF);
	    }

    /* substitute for macros on dependency line up to the semicolon if any */

    for(t = yytext ; *t!='\0' && *t!=';' ; ++t)
	;

    lastchp = t;
    lastch = *t;
    *t = '\0';

    subst(yytext, templin);	/* Substitute for macros on dependency lines */

    if(lastch)
    {
	for(t = templin ; *t ; ++t)
	    ;
	*t = lastch;
	while( *++t = *++lastchp ) ;
    }

    p = templin;
    t = yytext;
    while( *t++ = *p++ )
	;

    for(p = zznextc = text ; *p ; ++p )
	if(*p!=' ' && *p!='\t')
	    return(START);
    goto again;
}



struct fileframe {
    FILE *descriptor;
    char *name;
    int linenum;
} fileframe[NOFILE];

int framecount = 0;

extern dbgflag;

char *filename;
char *index();

pushfile( text )
char *text;
{
    register struct fileframe *fp;
    char name[INMAX];
    FILE *newfin;

    if( ! checkpush( text, name ) ) return( 0 );


    if( dbgflag ) {
	printf( "pushfile/level %d: %s from %s\n",
	    framecount, name, filename );
    }

    if( framecount >= NOFILE ) {
	fatal1( ".INCLUDES nested too deeply: file %s", filename );
    }

    if( (newfin=fopen( name, "r" )) == NULL ) {
	fprintf( stderr, "Warning: Can't open INCLUDE file %s\n", name );
	*text = '\0';
	return( 0 );
    }

    fp = &fileframe[framecount++];

    fp->descriptor = fin;
    fp->name = filename;
    fp->linenum = yylineno;

    fin = newfin;
    filename = copys(name);
    yylineno = 0;

    *text = '\0';
    return( 1 );

}

popfile()
{

    register struct fileframe *fp;


    if( framecount <= 0 ) return( 0 );

    fp = &fileframe[--framecount];

    if( dbgflag ) {
	printf( "popfile/level %d: %s to %s\n",
	    framecount, filename, fp->name );
    }

    fin = fp->descriptor;
    filename = fp->name;
    yylineno = fp->linenum;

    return( 1 );

}

checkpush( name, templin )
char *name, *templin;
{

    register char *sp;
    register len = strlen( ".INCLUDE" );

    /* we are looking for something like " *INCLUDE=pathname".  If we
     * find it we will return a pointer to "pathname"
     */

    while( isspace( *name ) ) name++;

    if( !(sp=index(name,'=')) ) return( 0 );

    if( sp-name != len ) return( 0 );

    if( strncmp( name, ".INCLUDE", len ) ) return( 0 );

    subst( sp + 1, templin );

    return( 1 );
}
