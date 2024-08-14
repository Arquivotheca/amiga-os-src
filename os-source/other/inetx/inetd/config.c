/*****************************/


#define DEBUG 1

#ifdef DEBUG
        #define DB1(x)  kprintf((x))
#else
        #define DB1(x) ;
#endif

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <string.h>
#include <stdio.h>

#include "inetd.h"

struct List *config(struct glob *g) ;
void DisplayName(struct List *list, UBYTE *name ) ;
UBYTE *parse( UBYTE *, UBYTE *, UBYTE **) ;
SHORT my_strspn( UBYTE *str, UBYTE *charset) ;
UBYTE *my_strpbrk( UBYTE *str, UBYTE *charset) ;
UBYTE *index( UBYTE *str, UBYTE ch ) ;


#define fname "inet:db/inetd.conf"
#define SERVTAB_NODE_ID 0x10101 ;

void
AddEntry( struct List *list, struct servtab *s )
{
	struct servtab *servtab ;
	
	servtab = AllocVec((long)sizeof(struct servtab), MEMF_PUBLIC|MEMF_CLEAR) ;
	if(!servtab)
	{
		VPrintf("out of memory\n", NULL) ;
	}
	else
	{
		*servtab = *s ;
		servtab->se_Node.ln_Name = servtab->se_service ;
		servtab->se_Node.ln_Type = SERVTAB_NODE_ID ;
		servtab->se_Node.ln_Pri  = 0 ;
		AddHead((struct List *)list, (struct Node *)servtab ) ;
	}
}


void
FreeEntries(struct glob *g) 
{
	struct servtab *worktab, *nexttab ;
	struct List *list = g->g_servtab_list ;
		
	worktab = (struct servtab *)(list->lh_Head) ;
	
	while( nexttab = (struct servtab *)(worktab->se_Node.ln_Succ))
	{
		FreeVec(worktab->se_line_array) ;
		FreeVec(worktab) ;
		worktab = nexttab ;
	}
	FreeVec( list ) ;
}
	

void
DisplayEntries( struct List *list) 
{
	struct Node *node ;
	char buffer[256] ;
	
	if( list->lh_TailPred == (struct Node *)list)
	{
		VPrintf("list is empty!\n",NULL) ;
	}
	else
	{
		for( node = list->lh_Head ; node->ln_Succ ; node = node->ln_Succ)
		{
			mysprintf(buffer,"%lx -> %s\n", node, node->ln_Name) ;
			VPrintf(buffer, NULL) ;
		}
	}
}


void 
DisplayName(struct List *list, UBYTE *name )
{
	struct Node *node ;
	char buffer[256] ;
	long foo = (long)&name ;
			
	if( node = FindName(list,name) )
	{
		while(node)
		{
			mysprintf(buffer, "found %s at location %lx\n", node->ln_Name, node) ;
			VPrintf(buffer, NULL) ;
			node = FindName((struct List *)node, name) ;
		}
	}
	else
	{
		VPrintf("Cannot find node named %s\n", (long *)&foo ) ;
	}
}



struct List *
config(struct glob *g)
{
	REGISTER struct List *MyList ;
	REGISTER BPTR fh ;
	REGISTER long len ;
	REGISTER UBYTE *tok ;
	REGISTER int type ;
	REGISTER int x ;
	struct servtab st ;
	UBYTE buffer[256] ;
	UBYTE seps[] = " \t\n" ;
	UBYTE *next_token ;

DB1("into config()\n") ;

	if(MyList = AllocVec(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR))
	{
		NewList( MyList ) ;
		fh = Open(fname, MODE_OLDFILE) ;
		if( !fh )
		{
			VPrintf("Cannot find inet.conf file\n", NULL) ;
			FreeVec(MyList) ;
			return(NULL) ;
		}
		
		memset((UBYTE *)&st,0,(long)sizeof(struct servtab)) ;
		
		while( FGets(fh, buffer, 256L))
		{
		
DB1("FGets()\n") ;

			if( *buffer == '#')
			{
				continue ;
			}
			len = (long)strlen(buffer) ;
			st.se_line_array = (UBYTE *)AllocVec(len+4,MEMF_PUBLIC|MEMF_CLEAR) ;
			if(st.se_line_array == NULL)
			{
				FreeEntries(g) ;
				return(NULL) ;
			}
			CopyMem(buffer, st.se_line_array, len ) ;
			st.se_service = parse(st.se_line_array, seps, &next_token) ;

			tok = parse( next_token, seps, &next_token) ;
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
			st.se_socktype = type ;

			st.se_proto = parse(next_token, seps, &next_token) ;
			tok = parse(next_token, seps, &next_token) ;
			st.se_wait = (short)(strcmp(tok, "wait") == 0 ) ;
			st.se_user = parse(next_token, seps, &next_token) ;
			st.se_server = parse(next_token, seps, &next_token ) ;

			x = 0 ;
			while((tok = parse( next_token, seps, &next_token)) && (x < MAXARGV) )
			{
				st.se_argv[x] = tok ;
				x++ ;
			}

			AddEntry(MyList, &st) ;

			memset((UBYTE *)&st,0,(long)sizeof(struct servtab)) ;
		}
		Close(fh) ;
DB1("calling DisplayEntries()\n") ;
		DisplayEntries(MyList) ;
		
	}
	else
	{
		VPrintf("no mem\n",NULL) ;
		return(NULL) ;
	}
	g->g_servtab_list = MyList ;
	return(MyList) ;
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
parse(UBYTE *buf, UBYTE *separators, UBYTE **nt )
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
		*nt = fromLastTime ;
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
	REGISTER UBYTE *s ; 

	s = str ; 
	while (index(charset, *s)) 
	{
		 s++ ; 
	}
	return(s - str) ; 
} 


UBYTE *my_strpbrk(UBYTE *str, UBYTE *charset)
{ 
        
	REGISTER UBYTE *s ; 

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








