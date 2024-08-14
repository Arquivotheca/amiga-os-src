;/* snoopstrip.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 snoopstrip.c
Blink FROM LIB:c.o,snoopstrip.o TO snoopstrip LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif


#ifdef DEBUG
#define bug printf
#define D(x)	x
#else
#define D(x) ;
#endif

char *vers = "\0$VER: snoopstrip 37.1";
char *Copyright = 
  "snoopstrip v37.1\nCopyright (c) 1992 Commodore-Amiga, Inc.  All Rights Reserved";
char *usage = 
 "usage: snoopstrip <snoopfile >outfile [discard] (discard unrecognized lines)\n";

#define BUFFERSIZE	256
typedef unsigned long ulong;

int linenum;


/* OLD format of snoop lines
** 0         1         2         3         4         5         6         7
** 01234567890123456789012345678901234567890123456789012345678901234567890
** $02D930=AllocMem(   756,$00002) A:FD3E76 C:0021FE   "Workbench"
** $0002F8= FreeMem($02D930,  756) A:FD3E90 C:0021FE   "Workbench"
**
*/


/* NEW format of snoop lines (32-bit addresses)
** 0         1         2         3         4         5         6         7
** 01234567890123456789012345678901234567890123456789012345678901234567890
** $0002D930=AllocMem(   756,$00000002) A:00FD3E76 C:000021FE   "Workbench"
** $000002F8= FreeMem($0002D930,   756) A:00FD3E90 C:000021FE   "Workbench"
**
*/

struct linecheck {
    int offset;
    char value;
} linecheck[] = {
   {	0,	'$' },
   {	9,	'=' },
   {	18,	'(' },
   {	35,	')' },
   {	37,	'A' },
   {	38,	':' },
   {	48,	'C' },
   {	49,	':' },
   {	61,	'"' },
   {	-1,	'\0' }
};

struct linecheck oldlinecheck[] = {
   {	0,	'$'	},
   {	7,	'='	},
   {	16,	'('	},
   {	-1,	'\0' 	}
   };

struct entry {
    struct entry *next, *prev;
    ulong address;
    ulong size;
    ulong caller;
    ulong linenum;
    char *process;
    ulong functype;
};

#define AMEM 0
#define FMEM 1
#define AVEC 2
#define FVEC 3

char *fdesc[] = { "AllocMem","FreeMem ","AllocVec","FreeVec " };

struct entry *firstentry, *lastentry, magicentry;
struct entry *freeentries;

BOOL OldWarn = FALSE, Discard = FALSE;

void init(void);
void process(void);
void flush(void);
int checkline( char *buf, struct linecheck *check, int max );
void doalloc( char *buf );
void dofree( char *buf );
int gethex( char *s );
int getdec( char *s );
ULONG MALLOC( ULONG size );
struct entry *getentry(void);
struct entry *findentry(ULONG addr);
char *lookup(char *s);
void freeentry( register struct entry *entry );
void appendentry( register struct entry *new );
struct entry *findentry( ULONG addr );
void removeentry( struct entry *e );
char *allocstring( char *s, int len );
char *findstring( char *s, int len );;
UBYTE toUpper(UBYTE c);
BOOL strEqu(UBYTE *p, UBYTE *q);

void main( int argc, char **argv )
{

    if((argc>2)||((argc==2)&&(argv[1][0] =='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(0L);
	}
    else if((argc==2)&&(strEqu(argv[1],"discard")))	Discard=TRUE;

    init();

    process();

    flush();
}

void init()
{
    /* make sure there is always one entry in the list */
    firstentry = lastentry = &magicentry;
    magicentry.address = -1;
}



void process()
{
    char linebuf[BUFFERSIZE];

    while( fgets( linebuf, BUFFERSIZE, stdin ) ) {
	if(SetSignal(0,0) & SIGBREAKF_CTRL_C)	break;
	linenum++;

	/* consistency check */
	if( ! checkline( linebuf, linecheck, strlen( linebuf ) ) ) {
notaline:
	    if(!OldWarn) {
	    	if(checkline( linebuf, oldlinecheck, strlen(linebuf) )) {
		    printf("NOTE: Snoopstrip 37.1 requires at least Snoop 36.11\n");
		    OldWarn=TRUE;
		    }
		}
	    if(!Discard) printf( "? %s", linebuf );
	    continue;
	}

	/* parse the line */
	if( ! strncmp( &linebuf[10], "Alloc", 5 ) ) {
	    doalloc( linebuf );
	} else if( ! strncmp( &linebuf[10], " Free", 5 ) ) {
	    dofree( linebuf );
	} else goto notaline;

    }
    
}


int checkline( char *buf, struct linecheck *check, int max )
{
    int index;

    while( (index = check->offset) != -1 ) {

	if( index >= max || buf[index] != check->value ) {


 D(bug("checkline: failed at %ld: got %c, wanted %c\n",
	index, buf[index], check->value ));


	    return( 0 );
	}

	check++;
    }
    return( 1 );
}


/* format of a line (32-bit addresses)
** 0         1         2         3         4         5         6         7
** 01234567890123456789012345678901234567890123456789012345678901234567890
** $0002D930=AllocMem(   756,$00000002) A:00FD3E76 C:000021FE   "Workbench"
** $000002F8= FreeMem($0002D930,   756) A:00FD3E90 C:000021FE   "Workbench"
**
*/

void doalloc( char *buf )
{
    struct entry *e = getentry();

    e->address = gethex( &buf[1] );
    e->size = getdec( &buf[19] );
    e->caller = gethex( &buf[39] );
    e->linenum = linenum;
    e->process = lookup( &buf[62] );

    if(buf[15] == 'V') e->functype = AVEC;
    else e->functype = AMEM;

    appendentry( e );

    D(bug("doalloc: 0x%lx, %ld, 0x%lx, '%s', functype=%ld, line %ld\n",
     e->address, e->size, e->caller, e->process, e->functype, linenum ));


}

void dofree( char *buf )
{
    ulong address;
    ulong size;
    ulong functype;
    struct entry *e;

    address = gethex( &buf[20] );
    size = getdec( &buf[29] );

    if(buf[15] == 'V') functype = FVEC;
    else functype = FMEM;

    D(bug("free functype = %ld, buf[15] = $%lx\n",functype,buf[15]));

    e = findentry( address );

    if( e ) {
	if( e->size != size ) {
	    printf(
"Wrong size. %s 0x%lx/%ld/'%s'/0x%lx, l %ld\n            %s 0x%lx/%ld/'%s'/0x%lx, l %ld\n",
	fdesc[e->functype],e->address, e->size, e->process, e->caller, e->linenum,
	fdesc[functype], address, size, lookup( &buf[62] ), e->caller, linenum );
	}
	if(e->functype != (functype-1)) {
	    printf(
"Wrong Func. %s 0x%lx/%ld/'%s'/0x%lx, l %ld\n            %s 0x%lx/%ld/'%s'/0x%lx, l %ld\n",
	fdesc[e->functype],e->address, e->size, e->process, e->caller, e->linenum,
	fdesc[functype], address, size, lookup( &buf[62] ), e->caller, linenum );
	}

	removeentry( e );
    } else {
	printf( "NoAlloc: $%08lx %6ld \"%s\" from $%08lx, line %ld\n",
	    address, size, lookup( &buf[62] ), gethex( &buf[39] ), linenum );
    }


}


int gethex( char *s )
{
    char c;
    int result = 0;

    while( c = *s++ ) {
	if( c >= '0' && c <= '9' ) {
	    result = (result << 4) + c - '0';
	} else if( c >= 'a' && c <= 'f' ) {
	    result = (result << 4) + c - 'a' + 10;
	} else if( c >= 'A' && c <= 'F' ) {
	    result = (result << 4) + c - 'A' + 10;
	} else break;
    }

    return( result );
}


int getdec( char *s )
{
    char c;
    int result = 0;

    /* skip leading spaces */
    while( *s == ' ' ) s++;

    while( c = *s++ ) {
	if( c >= '0' && c <= '9' ) {
	    result = result *10 + c - '0';
	} else break;
    }

    return( result );
}

ULONG
MALLOC( ULONG size )
{
    ULONG buf;

    buf = (ULONG)malloc( size );
    if( ! buf ) {
	fprintf( stderr, "out of memory\n" );
	exit( 1 );
    }
    return( buf );
}

struct entry *
getentry()
{
    struct entry *entry;

    if( freeentries ) {
	entry = freeentries;
	freeentries = entry->next;
    } else {
	entry = (struct entry *) MALLOC( sizeof( struct entry ) );
    }
    return( entry );
}


void freeentry( register struct entry *entry )
{
    entry->next = freeentries->next;
    freeentries = entry;
}


void appendentry( register struct entry *new )
{

    new->next = 0;
    new->prev = lastentry;

    lastentry->next = new;
    lastentry = new;
}


struct entry *
findentry( ULONG addr )
{
    register struct entry *entry;

    for( entry = lastentry; entry; entry = entry->prev ) {
	if( entry->address == addr ) return( entry );
    }

    return( 0 );
}

void removeentry( struct entry *e )
{
    if( e->next ) {
	e->next->prev = e->prev;
    } else {
	lastentry = e->prev;
    }

    if( e->prev ) {
	e->prev->next = e->next;
    } else {
	firstentry = e->next;
    }
}

struct stringnode {
    struct stringnode *next;
    char *string;
} *stringlist;


char *
allocstring( char *s, int len )
{

    char *string;
    struct stringnode *node;

    string = (char *) MALLOC( len + 1 );
    strncpy( string, s, len );
    string[len] = '\0';

    node = (struct stringnode *) MALLOC( sizeof( struct stringnode ) );
    node->string = string;
    node->next = stringlist;
    stringlist = node;

    return( string );
}

char *
findstring( char *s, int len )
{
    struct stringnode *node;

    for( node = stringlist; node; node = node->next ) {
	if( strncmp( s, node->string, len ) == 0 ) {
	    return( node->string );
	}
    }
    return( NULL );
}

char *
lookup( char *s )
{
    char *string;
    char *quote = NULL;
    int len,k;


    if(s)
    {
	for(k=0; s[k] && (s[k] != '"'); k++);
	if(s[k]) quote = &s[k];
    }


    if( ! quote ) quote = s + strlen( s );

    len = quote - s;

    if( ! ( string = findstring( s, len )) ) {
	string = allocstring( s, len );
    }

    return( string );
}


void flush(void)
{
    struct entry *e, *next;

    /* skip over place holder */
    e = firstentry->next;

    while( e ) {
	printf( "Unfreed: $%08lx %6ld \"%s\" from $%08lx, line %ld\n",
	    e->address, e->size, e->process, e->caller, e->linenum );

	next = e->next;
	removeentry( e );
	e = next;
    }
}


BOOL strEqu(UBYTE *p, UBYTE *q)
   { 
   while(toUpper(*p) == toUpper(*q))
      {
      if (*(p++) == 0)  return(TRUE);
      ++q; 
      }
   return(FALSE);
   } 

UBYTE toUpper(UBYTE c)
   {
   UBYTE u = c;
   if(((u>='a')&&(u<='z'))||((u>=0xe0)&&(u<=0xfe))) u = u & 0xDF;
   return(u);
   }

