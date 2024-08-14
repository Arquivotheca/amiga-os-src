/*
 * This file contains the
 * command processing command 
 */
#include	"ed.h"

#define TOUPPER(x) ((x) & 0x5f)

typedef struct  {
        short   k_code;                 /* Key code                     */
        int     (*k_fp)(int f, int n);  /* Routine to handle it         */
        struct IntuiText *k_string;
}       KEYTAB;

extern KEYTAB  keytab[];
extern int nkeytab;

int ecommand(f,n)
int f,n;
{
UBYTE buffer[82];
int s;

/*    cmdmop=NULL; */
    s=mlreply("execute-line: ",buffer,80);
    if(!s)return(s);
    command(buffer,f,n);
    return(TRUE);
}

int command(buffer,f,n)
UBYTE buffer[];
int f,n;
{
int i,s,j;
KEYTAB *ktp;

        cmdmop=NULL;
	if((i=nwspace(buffer) <0 ))return(FALSE);
    	for (j=0; j<nkeytab; j++) {
	    ktp = &keytab[j];
	    if(ktp && ktp->k_string && 
	      (!cmpcmd(&buffer[i],ktp->k_string->IText))) {
		thisflag = 0;
		if ( i=nextarg(&buffer[i])) {
		    cmdmop= &buffer[i];
		    fixcstring(cmdmop);
		}
		s = (int)(*ktp->k_fp)(f, n);
		lastflag = thisflag;
		cmdmop=NULL;
		return(s);
	    }
	}
    	s=FALSE;
    	cmdmop=NULL;
	(void)(*term.t_beep)();
	mlwrite("Command error");
    	return(s);
}

int cmpcmd( a, b )
UBYTE *a, *b;
{
    if(!a || !b)return(1);

    while( TOUPPER(*a) == TOUPPER(*b) ) {
	a++;
        if (( ! *b++ ) || (*b == ' ')) return( 0 );
    }
    if (((*b == ' ') || (! *b == ' ')) && ((*a == ' ') || (! *a))) return(0);
    if( *a < *b ) return( -1 );
    return( 1 );
}

WORD FindCommand(string)
UBYTE *string;
{
KEYTAB *ktp;
int i;

/* we better have a string pointer and have a non-NULL */
if(string && *string) {
    for (i=0; i<nkeytab; i++) {
	ktp = &keytab[i];
	if(ktp && ktp->k_string && (!cmpcmd(string,ktp->k_string->IText))) {
		return((ktp->k_code));
	}
    }
}
return(-1);
}


/* find first non white space in a string */
/* (ignoring leading whitespace, of course) */

int nwspace(s)
UBYTE *s;
{
int i=0;

while((*s == ' ')) {s++; i++;}
if(! *s)return(-1); /* just white space in the line */
return(i);
}

int nextarg(s)
UBYTE *s;
{
int i=0;

while(*s && (*s != ' ')) {s++; i++;}
if(! *s)return(0); /* no whitespace following , therefor no second argument */

/* find the start of the stuff */
while( *s && (*s == ' ')) {s++; i++;}

if(! *s)return(0); /* just more white space in the line */
return(i);
}

int cmdgetc()
{ 
int c;
    if (cmdmop) { 
        if(!(c= *cmdmop++)) {
	    cmdmop=NULL;
	    return(0);
	}
	return(c);
    }
return(-1);
} 

void fixcstring(s)
UBYTE *s;
{
    while(*s) {
	if(*s == '\"' ) {
	    strcpy(s,s+1);	/* eat the quote, advance the string */
	    if(*s == '\"') s++;	/* they want this one */
	}
	s++;
    }
}
