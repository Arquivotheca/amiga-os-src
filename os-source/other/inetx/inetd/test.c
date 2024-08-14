/* -----------------------------------------------------------------------
 * ifr.c    inetd config file reader
 *
 * Test File only!!!!!!!!!!!!!!!
 * 
 * #define MAXARGV     5
 * #define MXARGLEN   31
 * #define MAXSERVER  15
 * #define MAXSERVICE 15
 * #define MAXPROTO   15
 * #define MAXUSER    15
 *
 *------------------------------------------------------------------------
 */

#define DEBUG 1

#ifdef DEBUG
#define DB(x) printf(x)
#else
#define DB(x) ;
#endif


#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/lists.h>

#include <libraries/dos.h>
#include <exec/ports.h>              
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include "inetd_test.h"

#define fname "inet:db/inetd.conf"

UBYTE seps[] = " \t\n" ;
UBYTE *next_token ;
#define STRING_END "\0" 

UBYTE *parse( UBYTE *, UBYTE *) ;
SHORT my_strspn( UBYTE *str, UBYTE *charset) ;
UBYTE *my_strpbrk( UBYTE *str, UBYTE *charset) ;
UBYTE *parse(UBYTE *buf, UBYTE *separators) ;
UBYTE *index( UBYTE *str, UBYTE ch ) ;
void  print_entries( struct MinList *tablist ) ;
void  clear_entries( struct servtab *s) ;
void free_entries( struct MinList *tab ) ;

void
main(void)
{

	BPTR fh ;
	UBYTE buffer[256] ; 
	UBYTE *tok = NULL ;
	struct  MinList tablist ;
	long len ;
	int type, x ;
	struct servtab *servtab ;

	servtab = (struct servtab *)AllocVec((long)sizeof(struct servtab), MEMF_PUBLIC|MEMF_CLEAR) ;
	if( ! servtab )
	{
		exit(20) ;
	}
	NewList((struct List *)&tablist) ;
	
	fh = Open(fname, MODE_OLDFILE) ;
	if( !fh )
	{
		VPrintf("file open failed\n",NULL) ;
		FreeVec(servtab) ;
		exit(20) ;
	}
	
	while( FGets(fh, buffer, 256L))
	{
		if( *buffer == '#')
		{
			continue ;
		}
		len = (long)strlen(buffer) ;	
		servtab->se_line_array = (UBYTE *)AllocVec(len+4,MEMF_PUBLIC|MEMF_CLEAR) ;
		CopyMem(buffer, servtab->se_line_array, len ) ;
		servtab->se_service = parse(servtab->se_line_array, seps) ;

		tok = parse( next_token, seps) ;
		if( strcmp(tok, "stream") == 0)
		{
			type = SOCK_STREAM ;
		}
		else if( strcmp(tok, "dgram") == 0)
		{
			type = SOCK_DGRAM ;
		}
		else if( strcmp(tok, "rdm") == 0)
		{
			type = SOCK_RDM ;
		}
		else if( strcmp(tok, "seqpacket") == 0)
		{
			type = SOCK_SEQPACKET ;
		}
		else if( strcmp(tok, "raw") == 0)
		{
			type = SOCK_RAW ;
		}
		else type = -1 ;
		servtab->se_socktype = type ;

		servtab->se_proto = parse(next_token, seps) ;
		tok = parse(next_token, seps) ;
		servtab->se_wait = (short)(strcmp(tok, "wait") == 0 ) ;
		servtab->se_user = parse(next_token, seps) ;
		servtab->se_server = parse(next_token, seps ) ;
		
		x = 0 ;
		while((tok = parse( next_token, seps)) && (x < MAXARGV) )
		{
			servtab->se_argv[x] = tok ;
			x++ ;
		}
		
		if(AddEntry(tablist, servtab))
		{
			DB("main: AddEntry success\n") ;
		}
		
		memset((UBYTE *)servtab,0,(long)sizeof(struct servtab)) ;
	}
	Close(fh) ;
	print_entries(&tablist) ;
	free_entries(&tablist) ;
	FreeVec(servtab) ;
	exit() ;
}

/* ========================================================
 * AddEntry()
 * =======================================================
 */

int AddEntry( struct MinList *l, struct servtab *s )
{
	struct servtab *st = AllocVec(sizeof(struct servtab),MEMF_PUBLIC|MEMF_CLEAR) ;
		
		
	DB("AddEntry\n") ;
			
	if(st)
	{
		*st = *s ;
		AddHead( (struct List *)l, (struct Node *)st ) ;
		return(1) ;
	}
	
return(0) ;	
}
	

/* ========================================================
 * FreeTab
 * =======================================================
 */

void free_entries( struct MinList *tab )
{
	struct servtab *worktab, *nexttab ;
	
	DB("free_entries\n") ;
	
	worktab = (struct servtab *)tab->mlh_Head ;
	
	while( nexttab = (struct servtab *)(worktab->se_mn.mln_Succ))
	{
		FreeVec(worktab->se_line_array) ;
		FreeVec(worktab) ;
	}
}


/* =======================================================
 * print_entries
 * ====================================================
 */
 
void print_entries( struct MinList *tab )
{
	int x ;
	struct servtab *s ;
	struct MinNode *mn ; ;
	
	DB("print entries\n") ;

	if( tab->mlh_TailPred = (struct MinNode *)tab )
	{
		return ;   /* list is empty */
	}
	
	mn = (struct MinNode *)tab->mlh_Head ;

	do {
		s = (struct servtab *)mn ;
	
		printf("\nservice  = %s\n", s->se_service)  ;
		printf("socktype = %d\n", s->se_socktype) ;
		printf("proto    = %s\n", s->se_proto) ;
		printf("wait     = %s\n", s->se_wait ? "wait" : "nowait") ;
		printf("user     = %s\n", s->se_user) ;
		printf("server   = %s\n", s->se_server) ;
		for( x = 0 ; x < MAXARGV && s->se_argv[x] ; x++ ) 
		{
			printf("arg %d    = %s\n", x, s->se_argv[x] ) ;
		}
	}
	while((mn = mn->mln_Succ) != NULL) ;
}

 

/********* EXAMPLE CALLING CODE FOR PARSER ******************
 *  parse( str, seperators ) will return a pointer the the 
 *  NULL TERMINATED 'next word' in the string 'str' using the
 *  characters in the 'seperators' array to define 'word'
 *  boundaries. The global pointer 'next_token' points to the
 * remainder of the string.
 * 
 *
 *  main()  
 *  {
 *
 *  TEXT *tokep ;
 *
 *  tokep = strtok( s, seperators ) ;
 *  if( tokep ) {
 *      printf( "Token   = %s\n", tokep ) ;
 *      if( next_token )
 *         printf( "Next    = %s\n\n", next_token ) ;
 *      }
 *  }
 *
 ************************************************************/

 
UBYTE * 
parse(UBYTE *buf, UBYTE *separators)
{
    register UBYTE *token, *end, *tmp ;     /* Start and end of token. */
    UBYTE    *my_strpbrk() ; 
    static   UBYTE *fromLastTime ; 

    if (token = buf ? buf : fromLastTime) 
    {
        token += my_strspn(token, separators) ;     /* Find token! */ 
        if (*token == '\0') 
        {
            return(NULL) ; 
        }
        tmp = ((end = my_strpbrk(token,separators))) ? &end[1] : NULL ; 
        while( tmp && (*tmp == ' ') )
        {
            tmp++ ;
        }
        fromLastTime = tmp ? tmp : NULL ;
        next_token = fromLastTime ;
        *end = '\0' ;
    } 
    return(token) ; 
} 


/*---------------------------------------------*/

/* Return the number of characters from "charset" that are at the BEGINNING 
 * of string "str". 
*/ 
 
SHORT
my_strspn( UBYTE *str, UBYTE *charset )
{ 
   UBYTE *s ; 

     s = str ; 
     while (index(charset, *s)) 
     {
         s++ ; 
     }
     return(s - str) ; 
} 


UBYTE *my_strpbrk(UBYTE *str, UBYTE *charset)
{ 
        
   UBYTE *s ; 

     s = str ; 
     while ((*s != '\0') && (!index(charset, *s))) 
     {
         s++ ; 
     }
     return((*s!='\0') ? s : NULL) ; 
} 

UBYTE *
index( UBYTE *str, UBYTE ch )
{
	register UBYTE *p = str ;

	while(*p)
	{
		if(*p == ch)
		{
			return(p) ;
		}
		p++ ;
	}
	return(NULL) ;
}
