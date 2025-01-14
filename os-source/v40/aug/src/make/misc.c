#include "defs.h"
#include <proto/all.h>

FSTATIC struct nameblock *hashtab[HASHSIZE];
FSTATIC int nhashed = 0;


/* simple linear hash.	hash function is sum of
** characters mod hash table size.
*/

int hashloc(char *s)
{
    register int i;
    register int hashval;
    register char *t;

    hashval = 0;

    for(t=s; *t!='\0' ; ++t)
		hashval += *t;

    hashval %= HASHSIZE;

    for(i=hashval; hashtab[i]!=0 && unequal(s,hashtab[i]->namep); i = (i+1)%HASHSIZE ) ;

    return(i);
}


struct nameblock *srchname(char *s)
{
    return( hashtab[hashloc(s)] );
}



struct nameblock *makename(char *s)
{
    /* make a fresh copy of the string s */

    char						*copys();
    register struct nameblock	*p;

    if(nhashed++ > HASHSIZE-3)
		fatal("Hash table overflow");

	p				= ALLOC(nameblock);
    p->nxtnameblock = firstname;
    p->namep		= copys(s);
    p->linep		= 0;
    p->done			= 0;
    p->septype		= 0;
    p->modtime		= 0;
    firstname		= p;

    if(mainname == NULL)
		if(s[0]!='.' || hasslash(s) )
	    	mainname = p;

    hashtab[hashloc(s)] = p;

    return(p);
}



int hasslash(char *s)
{
    for( ; *s ; ++s)
		if(*s == '/')
		    return(YES);

    return(NO);
}


char *copys(register char *s)
{
    char			*calloc();
    register char	*t,
					*t0;

    if( (t = t0 = calloc( strlen(s)+1 , sizeof(char)) ) == NULL)
		fatal("out of memory");

    while(*t++ = *s++);
    return(t0);
}



char *concat(register char *a, register char *b, char *c)   /* c = concatenation of a and b */
{
    register char *t;

    t = c;

    while(*t = *a++)
		t++;
    while(*t++ = *b++);
    return(c);
}



int suffix(register char *a, register char *b,register char *p)  /* is b the suffix of a?	 if so, set p = prefix */
{
    char *a0,*b0;

    a0 = a;
    b0 = b;

    while(*a++);
    while(*b++);

    if( (a-a0) < (b-b0) )
		return(0);

    while(b>b0)
		if(*--a != *--b)
			return(0);

    while(a0<a) *p++ = *a0++;
	    *p = '\0';

    return(1);
}






int *ckalloc(register int n)
{
    register int *p;

    if( p = (int *) calloc(1,n) )
		return(p);

    fatal("out of memory");

    /* NOTREACHED */
}




/* copy string a into b, substituting for arguments */
char *subst(register char *a, register char *b)
{
    static int		 depth = 0;
    register char	*s;
    char			 vname[100],
					 closer;
    struct varblock *varptr(),
					*vbp;

    if(++depth > 100)
		fatal("infinitely recursive macro?");

    if(a!=0)  while(*a) {
		if(*a != '$')
			*b++ = *a++;
		else if(*++a=='\0' || *a=='$')
		    *b++ = *a++;
		else {
		    s = vname;
		    if( *a=='(' || *a=='{' ) {
				closer = ( *a=='(' ? ')' : '}');
				++a;
				while(*a == ' ')
					++a;
				while(*a!=' ' && *a!=closer && *a!='\0')
					*s++ = *a++;
				while(*a!=closer && *a!='\0')
					++a;
				if(*a == closer)
					++a;
			    }
	    	else
				*s++ = *a++;

		    *s = '\0';
		    if( (vbp = varptr(vname)) ->varval != 0) {
				b = subst(vbp->varval, b);
				vbp->used = YES;
			    }
			}
	    }

    *b = '\0';
    --depth;
    return(b);
}


int setvar(char *v,char *s)
{
    struct varblock *varptr(), *p;

    p = varptr(v);
    if(p->noreset == 0) {
		p->varval = s;
		p->noreset = inarglist;
		if(p->used && unequal(v,"@") && unequal(v,"*") && unequal(v,"<") && unequal(v,"?") )
		    fprintf(stderr, "Warning: %s changed after being used\n",v);
	    }
    
    return(0);
}


int eqsign(char *a)   /*look for arguments with equal signs but not colons */
{
    register char *s, *t;

    while(*a == ' ')
		++a;

    for(s=a  ;	 *s!='\0' && *s!=':'  ; ++s)
		if(*s == '=') {
		    for(t = a ; *t!='=' && *t!=' ' && *t!='\t' ;  ++t );
			    *t = '\0';

		    for(++s; *s==' ' || *s=='\t' ; ++s);
		    setvar(a, copys(s));
		    return(YES);
			}

    return(NO);
}


struct varblock *varptr(char *v)
{
    register struct varblock *vp;

    for(vp = firstvar; vp ; vp = vp->nxtvarblock)
		if(! unequal(v , vp->varname))
		    return(vp);

    vp = ALLOC(varblock);
    vp->nxtvarblock = firstvar;
    firstvar = vp;
    vp->varname = copys(v);
    vp->varval = 0;
    return(vp);
}


int fatal1(char *s, char *t)
{
    char buf[BUFSIZ];

    sprintf(buf, s, t);
    fatal(buf);
    return(0);
}



int fatal(char *s)
{
    if(s)
		fprintf(stderr, "Make: %s.  Stop.\n", s);
    else
		fprintf(stderr, "\nStop.\n");

	exit(10);
	return(0);
}



int yyerror(char *s)
{
    char		buf[50];
    extern int	yylineno;

    sprintf(buf, "file %s, line %d: %s", filename, yylineno, s);
    fatal(buf);
    return(0);
}



struct chain *appendq(struct chain *head, char *tail)
{
    register struct chain *p, *q;

    p = ALLOC(chain);
    p->datap = tail;

    if(head) {
		for(q = head ; q->nextp ; q = q->nextp);
		q->nextp = p;
		return(head);
	    }
    else
		return(p);
}





char *mkqlist(struct chain *p)
{
    register char	*qbufp,
					*s;
    static char		 qbuf[QBUFMAX];

    if(p == NULL) {
		qbuf[0] = '\0';
		return(0);
	    }

    qbufp = qbuf;

    for( ; p ; p = p->nextp) {
		s = p->datap;
		if(qbufp+strlen(s) > &qbuf[QBUFMAX-3]) {
		    fprintf(stderr, "$? list too long\n");
		    break;
			}
		while (*s)
		    *qbufp++ = *s++;
		*qbufp++ = ' ';
	    }
    *--qbufp = '\0';
    return(qbuf);
}
