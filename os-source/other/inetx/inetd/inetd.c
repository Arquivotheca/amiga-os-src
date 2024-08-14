/* -----------------------------------------------------------------------
 * inetd.c         2.0 for shared socket library  SAS 5.10
 *
 * $Locker:$
 *
 * $Id:$
 *
 * $Revision:$
 *
 * $Log:$
 *
 * $Header:$
 *
 *------------------------------------------------------------------------
 */


#define DEBUG 1

#ifdef DEBUG
        #define DB1(x)  VPrintf((x),NULL)
#else
        #define DB1(x) ;
#endif

#include <exec/types.h>
#include <ss/socket.h>
#include <sys/socket.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <signal.h>

#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>                                                
#include <exec/ports.h>              
#include <ctype.h>
#include <string.h>

#include <libraries/gadtools.h>
#include <utility/tagitem.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>


#include "inetd_rev.h"


#ifdef LATTICE
 #include <ios1.h>
 #include <proto/all.h>
#endif

#ifdef min
   #undef min
#endif

#define MAXSOCKS 3L

#define OPT_DEBUG  0
#define OPT_COUNT  1 
#define TEMPLATE "-D=DEBUG/S"

#include <stdio.h>   /* after the #undef min */
#include <errno.h>
 
#define OUTPUT(x)  VPrintf((x),NULL)
#define MEMFLAGS   (MEMF_PUBLIC|MEMF_CLEAR)
#define PORTNAME   "Inetd_Rendezvous"


extern struct IOStdReq *CreateStdIO();
extern struct MsgPort  *CreatePort();
extern struct Library  *OpenLibrary();


void getout( struct glob *, unsigned char *) ;
void main( void ) ;
void StartTimer( ULONG sec, ULONG micro, struct glob *g) ;
void KillTimer(struct glob *g) ;
void DeleteTimer(struct glob *g) ;
int InitTimer(struct glob *g) ;
void DisplayName(struct List *list, UBYTE *name ) ;
UBYTE *parse( UBYTE *, UBYTE *, UBYTE **) ;
SHORT my_strspn( UBYTE *str, UBYTE *charset) ;
UBYTE *my_strpbrk( UBYTE *str, UBYTE *charset) ;
UBYTE *index( UBYTE *str, UBYTE ch ) ;
struct List *config(struct glob *g) ;

#include "inetd.h"

void 
main( void )
{
	struct glob *glob = AllocVec((long)sizeof(struct glob), MEMFLAGS) ;
	long opts[OPT_COUNT] ;
	struct RDArgs *ra = NULL ;
/*	extern int Enable_Abort ; */
	struct Library *SockBase = NULL, *UtilityBase =NULL ;
	UBYTE *version ;
	version = VERSTAG ;

	if( ! glob )
	{
		VPrintf("allocVec(glob) failed\n", NULL) ;
		exit(20) ;
	}

/********
	Forbid() ;
	glob->g_TagPort = FindPort(PORTNAME) ;
	Permit() ;
	if( glob->g_TagPort ) 
	{
		VPrintf("Inetd is already running,\n", NULL) ;
		FreeVec(glob) ;
		exit(20) ;
	}	
	glob->g_TagPort = CreatePort(PORTNAME,-128L) ;
	if( glob->g_TagPort == NULL) 
	{
		FreeVec(glob) ;
		VPrintf("port creation failed\n", NULL) ;
		exit(20) ;
	}
*********/

	memset((BYTE *)opts,0,sizeof(opts)) ;
	ra = (struct RDArgs *)ReadArgs(TEMPLATE,opts,NULL) ;
	if( ! ra ) 
	{
		VPrintf("ReadArgs failure\n", NULL) ;
		DeletePort(glob->g_TagPort) ;
		FreeVec(glob) ;
		exit(20) ;
	}

	if(opts[OPT_DEBUG])
	{
		glob->g_Debug = 1 ;
	}

	FreeArgs(ra) ;
/*	Enable_Abort = 0 ;    no control-C aborts allowed */

	if( ! InitTimer(glob) )
	{
		DeletePort(glob->g_TagPort) ;
		FreeVec(glob) ;
		exit(20) ;
	}
    /********************************************************************/
    /** you can't exit via getout() prior to initializing the timer!!  **/
    /********************************************************************/

			/* init the timer so getout() code is happy !!! */
	StartTimer(0,100,glob) ;
	WaitIO((struct IOrequest *)glob->g_TimerReq) ;

	if((SockBase = OpenLibrary( "inet:libs/socket.library", 0L )) == NULL) 
	{
		getout(glob,"cannot open socket.library") ;
	}
	setup_sockets( MAXSOCKS, &glob->g_errno );
	glob->g_sockets_are_setup = 1L ;
	glob->g_SockBase = SockBase ;
	DB1("sockbase open\n") ;

	if((UtilityBase = OpenLibrary( "utility.library", 0L )) == NULL) 
	{
		getout(glob, "cannot open utility library") ;
	}
	glob->g_UtilityBase = UtilityBase ;
	DB1("utilitybase open\n") ;

		/*** main loop  ***/

	DB1("calling config in 3 secs\n") ;
	Delay(150L) ;
	config(glob) ;
	if(glob->g_servtab_list == NULL) 
	{
		getout(glob, "config failed.") ;
	}

	DB1("inetd: Main Loop\n") ;


#ifdef DEBUG
	DB1("starting timer delay of 5 seconds\n") ;
	StartTimer(5L,0L,glob) ;
	WaitIO((struct IOrequest *)glob->g_TimerReq) ;
	DB1("back from 5 sec delay\n") ;
#endif

	getout(glob, NULL) ;
}


/* ============================================
 * InitTimer( struct glob *)
 * ============================================
 */

int
InitTimer(struct glob *g)
{
	if(g->g_TimerPort = CreateMsgPort())
	{
		if(g->g_TimerReq = (struct timerequest *)CreateIORequest(g->g_TimerPort, (long)sizeof(struct timerequest)))
		{
			if(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)g->g_TimerReq,0L) == 0L)
			{
				g->g_TimerDevOpen = 1 ;
				g->g_timerbit = 1L << g->g_TimerPort->mp_SigBit ;
				return( 1 ) ;
			}
			DeleteIORequest(g->g_TimerReq) ;
		}
		DeleteMsgPort(g->g_TimerPort) ;
	}
	return(0) ;
}


/* ============================================
 * StartTimer( sec, micro, timermsg
 * ============================================
 */

void
StartTimer( ULONG sec, ULONG micro, struct glob *g)
{
	g->g_TimerReq->tr_node.io_Command = TR_ADDREQUEST ;
	g->g_TimerReq->tr_time.tv_secs  = sec ;
	g->g_TimerReq->tr_time.tv_micro = micro ;
	SendIO((struct IORequest *)g->g_TimerReq) ;
}


/* ============================================
 * KillTimer( glob )
 * ============================================
 */


void 
KillTimer(struct glob *g)
{
	DB1("into KillTimer()\n") ;
	AbortIO((struct IORequest *)g->g_TimerReq) ;
	WaitIO((struct IORequest *)g->g_TimerReq) ;
	DB1("out of KillTimer()\n") ;
}

/* ============================================
 * DeleteTimer( glob )
 * ============================================
 */

void 
DeleteTimer(struct glob *g)
{
	DB1("into DeleteTimer()\n") ;
	KillTimer(g) ;
	if( g->g_TimerDevOpen )
	{
		CloseDevice((struct IORequest *)g->g_TimerReq) ;
		g->g_TimerDevOpen = 0 ;
	}
	if(g->g_TimerReq)
	{
		DeleteIORequest((struct IORequest *)g->g_TimerReq) ;
		g->g_TimerReq = NULL ;
	}
	if(g->g_TimerPort)
	{
		DeleteMsgPort(g->g_TimerPort) ;
		g->g_TimerPort = NULL ;
	}
	DB1("out of DeleteTimer()\n") ;
}


/* ==========================================
 * getout(message) 
 * ==========================================
 */

void
getout( struct glob *g, UBYTE *msg )
{

	struct IntuitionBase *IntuitionBase = NULL ;
	struct Library *SockBase = g->g_SockBase ;
	struct EasyStruct es ;

	es.es_StructSize   = (LONG)sizeof(struct EasyStruct) ;
	es.es_Flags        = 0L ;
	es.es_Title        = (UBYTE *)" Inetd Error Message: " ;
	es.es_TextFormat   = (UBYTE *)" Inetd Error: %s" ;
	es.es_GadgetFormat = (UBYTE *)"    Exit    " ;


	if(g->g_servtab_list)
	{
		DB1("calling FreeEntries\n") ;
		FreeEntries( g ) ;
		g->g_servtab_list = NULL ;
	}

	DB1("calling KillTimer\n") ;	

	KillTimer(g) ;
	DeleteTimer(g) ;
	if( g->g_TagPort)
	{
		DB1("getout: deleting TagPort\n") ;
		DeletePort(g->g_TagPort) ;	
	}

	if( msg )
	{
		DB1("getout: IntuitionBase\n") ;
		IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",37L) ;
		if( IntuitionBase != NULL )
		{
			DB1("getout: doing EasyRequest\n") ;
			EasyRequestArgs( NULL, &es, NULL, (APTR) &msg) ; 
			DB1("getout: closing intuitionbase\n") ;
			CloseLibrary((struct Library *)IntuitionBase ) ;
		}
	}

	if( g->g_UtilityBase )
	{
		DB1("getout: closing utilitybase\n") ;
		CloseLibrary(g->g_UtilityBase) ;
	}

	if( g->g_sockets_are_setup)
	{
		DB1("getout: cleanup_sockets\n") ;
		cleanup_sockets() ;
	}

	if( g->g_SockBase )
	{
		DB1("getout: closing sockbase\n") ;
		CloseLibrary(g->g_SockBase) ;
	}

	FreeVec(g) ;

	exit(msg ? 20 : 0 ) ;
}


