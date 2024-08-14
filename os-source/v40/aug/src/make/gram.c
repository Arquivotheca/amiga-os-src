#include "defs.h"
static	char *sccsid = "@(#)gram.y	4.1 (Berkeley) 81/02/28";

#define YYFLAG		-1000
#define YYLAST		18
#define YYNPROD		19
#define YYERRCODE	256
#define NAME		257
#define SHELLINE	258
#define START		259
#define MACRODEF	260
#define COLON		261
#define DOUBLECOLON	262
#define GREATER		263
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define yyclearin	yychar = -1
#define yyerrok		yyerrflag = 0

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif





typedef union {
	struct shblock		*yshblock;
	struct depblock		*ydepblock;
	struct nameblock	*ynameblock;
} YYSTYPE;





struct fileframe {
    FILE				*descriptor;
    char				*name;
    int					 linenum;
} fileframe[NOFILE];





short yyexca[]	={	  -1,	 1,		0,	  -1,	 -2,	 0,};
short yyact[]	={	   9,	11,	   18,	   5,	 12,	13,	   3,	  4,	16,
					  17,	 7,		2,	   1,	 10,	 8,	   6,	 14,	15 };
short yypact[]	={ -1000, -253, -1000,  -247, -1000, -1000, -257, -1000,  -250,
				   -1000, -248, -1000, -1000, -1000, -1000, -256, -1000, -1000,
				   -1000};
short yypgo[]	={	   0,	17,	   16,	  15,	 14,	13,	  12,	 11,	 1 };
short yyr1[]	={	   0,	 6,		6,	   7,	  7,	 7,	   7,	  3,	 3,
					   4,	 4,		5,	   5,	  8,	 8,	   2,	  2,	 1,
					   1 };
short yyr2[]	={	   0,	 0,		2,	   1,	  1,	 4,	   1,	  1,	 2,
					   0,	 1,		1,	   2,	  1,	 1,	   0,	  1,	 1,
					   2 };
short yychk[]	={ -1000,	-6,	   -7,	 259,   260,   256,	  -3,   257,	-4,
					 257,	-5,	   -8,	 261,	262,	-2,	  -1,	258,   257,
					 258 };
short yydef[]	={	   1,	-2,		2,	   3,	  4,	 6,	   9,	  7,	15,
					   8,	10,	   11,	  13,	 14,	 5,	  16,	 17,	12,
					  18 };




#ifndef lint
static	char yaccpar_sccsid[] = "@(#)yaccpar 1.3 86/10/08 SMI"; /* from UCB 4.1 83/02/11 */
#endif

#ifdef YYDEBUG
int yydebug = 1; /* 1 for debugging */
#endif






YYSTYPE						yylval,
							yyval,
							yyv[YYMAXDEPTH]; /* where the values are stored */

int							 yychar		= -1, /* current input token number */
							 yynerrs	= 0,  /* number of errors */
							 framecount	= 0;
short						 yyerrflag	= 0;  /* error recovery flag */

int							 yylineno;

extern int					 dbgflag,
							 yychar;
extern short				 yyerrflag;
extern FILE					*fin;

char						*filename,
							*index(),
							*zznextc;	/* zero if need another line; otherwise points to next char */

struct depblock				*pp;
struct nameblock			*leftp;
struct lineblock			*lp, *lpp;

FSTATIC struct shblock		*prevshp;
FSTATIC struct nameblock	*lefts[NLEFTS];
FSTATIC struct depblock		*prevdep;
FSTATIC int					 nlefts,
							 sepc;






int yylex(void)
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

    if(*zznextc == ':') {
		if(*++zznextc == ':') {
		    ++zznextc;
		    return(DOUBLECOLON);
			}
		else
		return(COLON);
    	}

    if(*zznextc == '>') {
		++zznextc;
		return(GREATER);
	    }

    if(*zznextc == ';')
		return( retsh(zznextc) );

    p = zznextc;
    q = word;

    while( ! ( funny[*p] & TERMINAL) )
		*q++ = *p++;

    if(p != zznextc) {
		*q = '\0';
		if((yylval.ynameblock=srchname(word))==0)
	    	yylval.ynameblock = makename(word);
		zznextc = p;
		return(NAME);
	    }
    else {
		fprintf(stderr,"Bad character %c (octal %o), file %s line %d",
		*zznextc,*zznextc,filename,yylineno);
		fatal( (char *) NULL );
    	}

    return(0);	/* never executed */
}






int retsh(char *q)
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






int nextlin(void)
{
    static char		 yytext[INMAX];
    static char 	*yytextl = yytext+INMAX;

    char			*text,						 templin[INMAX];
    char			 lastch,					*lastchp;

    register char	 c,							*p,					*t;
    extern char		**linesptr;
    int				 incom,						kc;

again:

    incom = NO;
    zznextc = 0;

    if(fin == NULL) {
		if( (text = *linesptr++) == 0)
		    return(0);
		++yylineno;
    	}
    else {
		for(p = text = yytext ; p<yytextl ; *p++ = kc)
		    switch(kc = getc(fin)) {
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
					if(p==yytext || p[-1]!='\\') {
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
		    else if(*p++ == '=') {
				pushfile( text );
				eqsign(text);
				return(MACRODEF);
			    }

    /* substitute for macros on dependency line up to the semicolon if any */

    for(t = yytext ; *t!='\0' && *t!=';' ; ++t);

    lastchp = t;
    lastch = *t;
    *t = '\0';

    subst(yytext, templin);	/* Substitute for macros on dependency lines */

    if(lastch) {
		for(t = templin ; *t ; ++t);
		*t = lastch;
		while( *++t = *++lastchp ) ;
	    }

    p = templin;
    t = yytext;
    while( *t++ = *p++ );

    for(p = zznextc = text ; *p ; ++p )
		if(*p!=' ' && *p!='\t')
		    return(START);
    goto again;
}






int	pushfile( char *text )
{
    register struct fileframe *fp;
    char name[INMAX];
    FILE *newfin;

    if( ! checkpush( text, name ) )
		return( 0 );


    if( dbgflag )
		printf( "pushfile/level %d: %s from %s\n", framecount, name, filename );

    if( framecount >= NOFILE )
		fatal1( ".INCLUDES nested too deeply: file %s", filename );

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





int	popfile(void)
{

    register struct fileframe *fp;


    if( framecount <= 0 )
		return( 0 );

    fp = &fileframe[--framecount];

    if( dbgflag )
		printf( "popfile/level %d: %s to %s\n", framecount, filename, fp->name );

    fin = fp->descriptor;
    filename = fp->name;
    yylineno = fp->linenum;

    return( 1 );

}





int checkpush( char *name, char *templin )
{

    register char *sp;
    register len = strlen( ".INCLUDE" );

    /* we are looking for something like " *INCLUDE=pathname".  If we
    ** find it we will return a pointer to "pathname"
    */

    while( isspace( *name ) )
		name++;

    if( !(sp=index(name,'=')) )
		return( 0 );

    if( sp-name != len )
		return( 0 );

    if( strncmp( name, ".INCLUDE", len ) )
		return( 0 );

    subst( sp + 1, templin );

    return( 1 );
}





int yyparse(void)
{

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

yystack:    /* put a state and value onto the stack */

#	ifdef YYDEBUG
		if( yydebug  )
			printf( "state %d, char 0%o\n", yystate, yychar );
#		endif

	if( ++yyps>= &yys[YYMAXDEPTH] ) {
		yyerror( "yacc stack overflow" );
		return(1);
		}

	*yyps = yystate;
	++yypv;
	*yypv = yyval;

yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG )
		goto yydefault; /* simple state */

	if( yychar<0 )
		if( (yychar=yylex())<0 )
			yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST )
		goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 )
			--yyerrflag;
		goto yystack;
		}

yydefault:

	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 )
			if( (yychar=yylex())<0 )
				yychar = 0;

		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 )
			if( *yyxi == yychar )
				break;

		if( (yyn = yyxi[1]) < 0 )
			return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

			case 0:   /* brand new error */
				yyerror( "syntax error" );
yyerrlab:
				++yynerrs;

			case 1:
			case 2: /* incompletely recovered error ... try again */

				yyerrflag = 3;

				/* find a state where "error" is a legal shift action
				*/

				while ( yyps >= yys ) {
					yyn = yypact[*yyps] + YYERRCODE;
					if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
						yystate = yyact[yyn];  /* simulate a shift of "error" */
						goto yystack;
						}
					yyn = yypact[*yyps];

					/* the current yyps has no shift onn "error",
					** pop stack
					*/

#					ifdef YYDEBUG
						if( yydebug )
							printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#						endif
					--yyps;
					--yypv;
					}

					/* there is no state on the stack with an error shift ...
					** abort
					*/

yyabort:
					return(1);


			case 3:  /* no shift yet; clobber input char */

#				ifdef YYDEBUG
					if( yydebug )
						printf( "error recovery discards char %d\n", yychar );
#					endif

				if ( yychar == 0 )
					goto yyabort; /* don't discard EOF, quit */

				yychar = -1;
				goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#	ifdef YYDEBUG
		if( yydebug )
			printf("reduce %d\n",yyn);
#		endif
	yyps -= yyr2[yyn];
	yypvt = yypv;
	yypv -= yyr2[yyn];
	yyval = yypv[1];
	yym=yyn;

	/* consult goto table to find next state */

	yyn = yyr1[yyn];
	yyj = yypgo[yyn] + *yyps + 1;
	if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn )
		yystate = yyact[yypgo[yyn]];

	switch(yym){
		case 5:
			while( --nlefts >= 0) {
				leftp = lefts[nlefts];
				if(leftp->septype == 0)
					leftp->septype = sepc;
				else if(leftp->septype != sepc)
					fprintf(stderr, "Inconsistent rules lines for `%s'\n", leftp->namep);
				else if(sepc==ALLDEPS && *(leftp->namep)!='.' && yypvt[-0].yshblock!=0) {
					for(lp=leftp->linep; lp->nxtlineblock!=0; lp=lp->nxtlineblock)
						if(lp->shp)
							fprintf(stderr, "Multiple rules lines for `%s'\n", leftp->namep);
					}

				lp = ALLOC(lineblock);
				lp->nxtlineblock = NULL;
				lp->depp = yypvt[-1].ydepblock;
				lp->shp = yypvt[-0].yshblock;

				if(! unequal(leftp->namep, ".SUFFIXES") && yypvt[-1].ydepblock==0)
					leftp->linep = 0;
				else if(leftp->linep == 0)
					leftp->linep = lp;
				else	{
					for(lpp = leftp->linep; lpp->nxtlineblock; lpp = lpp->nxtlineblock) ;
					if(sepc==ALLDEPS && leftp->namep[0]=='.')
						lpp->shp = 0;
					lpp->nxtlineblock = lp;
					}
				}
			break;

		case 7:
			lefts[0] = yypvt[-0].ynameblock;
			nlefts = 1;
			break;

		case 8:
			lefts[nlefts++] = yypvt[-0].ynameblock;
	    	if(nlefts>=NLEFTS)
				fatal("Too many lefts");
			break;

		case 9:
			{
				char junk[INMAX];

				sprintf(junk, "file %s, line %d", filename, yylineno);
				fatal1("Must be a separator on rules: %s", junk);
				}
			break;

		case 11:
			prevdep = 0;
			yyval.ydepblock = 0;
			break;

		case 12:
			  pp = ALLOC(depblock);
			  pp->nxtdepblock = NULL;
			  pp->depname = yypvt[-0].ynameblock;
			  if(prevdep == 0) yyval.ydepblock = pp;
			  else  prevdep->nxtdepblock = pp;
			  prevdep = pp;
			  break;

		case 13:
			sepc = ALLDEPS;
			break;

		case 14:
			sepc = SOMEDEPS;
			break;

		case 15:
			yyval.yshblock = 0;
			break;

		case 16:
			yyval.yshblock = yypvt[-0].yshblock;
			break;

		case 17:
			yyval.yshblock = yypvt[-0].yshblock;
			prevshp = yypvt[-0].yshblock;
			break;

		case 18:
			yyval.yshblock = yypvt[-1].yshblock;
			prevshp->nxtshblock = yypvt[-0].yshblock;
			prevshp = yypvt[-0].yshblock;
			break;
		}
	goto yystack;  /* stack new state and value */
}
