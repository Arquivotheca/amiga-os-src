/* -----------------------------------------------------------------------
 * RLOGIN.C  (Lattice 5.10 )
 *
 * $Locker:  $
 *
 * $Id: rlogin.c,v 1.14 91/08/09 14:20:47 bj Exp $
 *
 * $Revision: 1.14 $
 *
 * $Log:	rlogin.c,v $
 * Revision 1.14  91/08/09  14:20:47  bj
 * fixed TEMPLATE bug ('rlogin ?' would display template twice.)
 * First round of support for different background colors (alpha
 *   source code and NOT functional in this 37.13 exe file.
 *   Commented out.)
 * 
 * Revision 1.13  91/08/06  13:29:35  bj
 * V 37.11
 * Added ReadArgs support.
 * Global data structure.
 * About() function.
 * Shared Socket Library version !!!!!
 * 
 * Revision 1.12  91/06/19  12:25:13  bj
 * Added conclip support.  Fixed font bug. assorted details.
 * This version commences the 2.0x ONLY versions of rlogin.
 * This will NOT run under 1.x
 * 
 * Revision 1.11  91/04/24  15:20:40  bj
 * Fixed all PAL/NTSC display mode incompatibilities. Now works
 * in all PAL modes whether on the WBench screen or on a custom
 * screen.
 * 
 * Revision 1.10  91/02/14  16:13:01  bj
 * Details and formatting. Nothing functional.
 * 
 * Revision 1.9  91/02/13  16:44:11  bj
 * Fixed Ctrl-C fixes. 
 * Fixed loss of escape sequences in vi and emacs.
 * Different OOB Data handling.
 * 
 * Revision 1.8  91/01/31  13:42:58  bj
 * details of no functional value.
 * 
 * Revision 1.7  91/01/31  12:06:18  bj
 * Switched to Lattice 5.10
 * Fixes to allow better (faster) control-c handling when
 *   aborting something on the net (like ls).
 * Fixed handling of command line completion.
 * version # bumped to 37 to match the OS stuff.
 * 
 * Revision 1.6  90/12/07  13:47:39  bj
 * Changed termtype default from 'network' to 'RLamiga'
 * to support a unique termcap entry on large systems.
 * The only change made for v14.
 *
 * Revision 1.5  90/12/02  06:02:54  martin
 * replaced selectwait() with Wait() call
 * made async, fixed occassional pauses
 * faster and 3.5K smaller
 * 
 * Revision 1.4  90/11/29  15:09:34  bj
 * Had to go back to 5.05 Lattice to get rid of a crash
 * caused by unknown 5.10x artifacts (still unknown)
 * 
 * Revision 1.3  90/11/28  00:32:20  bj
 * Fixed bug whereby rlogin would crash on 68000 machines. This was due to a non-word
 * aligned array in the hostent structure used in gethostent() in the socket library.
 * Made this array a global under rlogin and extracted the gethostenet() code from
 * the socket library to be used here in rlogin.  Seems to work ok now.
 *          
 *
 * $Header: HOG:Other/inet/src/c/rlogin/RCS/rlogin.c,v 1.14 91/08/09 14:20:47 bj Exp $ 
 *
 *------------------------------------------------------------------------
 */

/* #define DEBUG 1 */

/* define BG2 1 */  /* do this is you want background color to be color #2 */
                    /* this requires a matching termcap !!!!!!!  */

#define RDARGS_VERSION 1

#ifdef DEBUG
	#define DB1(x)  printf(x)
#else
	#define DB1(x) ;
#endif

#include "rlogin_rev.h"

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


#ifdef LATTICE
 #include <ios1.h>
 #include <proto/all.h>
#endif

#ifdef min
   #undef min
#endif

#define MAXSOCKS 10L

#include <stdio.h>   /* after the #undef min */
#include <errno.h>
 
   /* ------------  conclip stuff */  /* ----------- */

struct CHRSChunk {
    struct MinNode mn;
    BYTE  *text;        /* pointer to start of text */
    BYTE * freeme;      /* if non-zero, call FreeVec() with this pointer */
    ULONG  size;        /* total # of characters in this block */
    ULONG  flags;       /* may end up needing this at some time */
    ULONG  UserLink;
    BYTE  data[1];      /* really 'n' # of bytes */
};

struct cmdMsg {
    struct Message cm_Msg;
    LONG   cm_type;     /* type of message, and return validation */
    LONG   cm_error;    /* error code return */
    struct MinList	cm_Chunks;
};

struct MinList paste ;
struct MinNode *pastenode ;
struct cmdMsg cm = {0} ;
struct MinList ClipList ;

BOOL   clip_in_progress ;
BYTE   clipsig = -1 ;
ULONG  clipevent ;
ULONG  pcount = 0L ;


VOID   About(VOID) ;
LONG   MakeClipMine(VOID) ;
struct MsgPort *CreateMsgPort(VOID) ;
VOID   DoConClipStuff(int s) ;
LONG   GetClipChar( UBYTE *thechar ) ;
VOID   KillPaste(VOID) ;
VOID   HandleMenu( struct RLGlobal *g, UBYTE *data ) ;
VOID   handle_oob( struct RLGlobal *g ) ;
int    send_window_size( struct RLGlobal *g ) ;
VOID   FlashBar( VOID ) ;
BOOL   AskExit( LONG wide, LONG high ) ;
VOID   Getout( BYTE *msg, struct RLGlobal *g ) ;

BYTE clipbuffer[256] ;


#include "rlglobal.h"

 /* Messages sent via out-of-band data. */
#define TIOCPKT_FLUSHREAD    0x01    /* flush pending kybd data    */
#define TIOCPKT_FLUSHWRITE   0x02    /* flush all data to mark     */
#define TIOCPKT_STOP         0x04
#define TIOCPKT_START        0x08
#define TIOCPKT_NOSTOP       0x10
#define TIOCPKT_DOSTOP       0x20
#define TIOCPKT_WINDOW       0x80    /* request window size        */

#define ASKEXIT(x) (AskExit((LONG)(20 + 20 +(((x)->rl_fontwide) * 29)), 50L ))
#define ESC_SEQ_TERMINATOR(x) (((x) >= 0x40) && ((x) <= 0x7e)) 

	/* if using a sole '9b' as the escape then these values */
	/* need to be reduced by one */

#define CON_CLOSEGAD(b) (((b)[2] == 0x31 && (b)[3] == 0x31))
#define CON_RESIZE(b)   (((b)[2] == 0x31 && (b)[3] == 0x32))
#define CON_GOTMENU(b)  (((b)[2] == 0x31 && (b)[3] == 0x30))
#define CONCLIP_TERM    0x76  /* 'v'  */
#define CONCLIP_PENDING(b) (((b)[2] == 0x30 && (b)[3] == 0x20 && (b)[4] == 0x76))

    /* -------------------------------------------------------*/
	/* for gethostent - using this local version rather than  */
	/* the libray version.  Library version was crashing on   */
	/* 68000 machines. This was caused by the                 */
	/*                                                        */
	/*          char ghe_hostaddr[MAXADDRSIZE];               */
	/*                                                        */
	/* array being NOT word-aligned. When gethostent() tried  */
	/* write a LONG value into (LONG *)ghe_hostaddr, BANG!    */
	/* Solved (for now) by making the ghe_hostaddr[] array a  */
	/* global for RLOGIN.  The question remains as to why the */
	/* compiler is not aligning this.                         */
	/* -------------------------------------------------------*/

#define	MAX_ALIASES	35
#define	MAXADDRSIZE	14

BYTE ghe_hostaddr[MAXADDRSIZE] ; /* the fix ! */

#define OUTPUT(x) (VPrintf((x),NULL))

#define CSI               0x9b
#define CON_IDCMP_REPORT  0x7c
#define CON_OPTION_ON     0x68  /* lower case 'h' */
#define CON_OPTION_OFF    0x6c  /* lower case 'l' */
#define CON_SPECIAL_KEY   0x7e  /* tilde */
#define TILDE             0x7e  /* tilde */

BYTE *version = VERSTAG ;

UBYTE menu_on[]         = { '\x9b', '1','0','{', } ;
UBYTE closegad_on[]     = { '\x9b', '1','1','{','\0' } ;
UBYTE closegad_off[]    = { '\x9b', '1','1','}','\0' } ;
UBYTE gimme_resize[]    = { '\x9b', '1','2','{','\0' } ;

UBYTE linewrap_on[]     = { '\x9b', '\x3f', '\x37', '\x68' } ;
UBYTE linewrap_off[]    = { '\x9b', '\x3f', '\x37', '\x6c' } ;
UBYTE lfmode_on[]       = { '\x9b', '\x32', '\x30', '\x68' } ; 
UBYTE lfmode_off[]      = { '\x9b', '\x32', '\x30', '\x6c' } ; 
UBYTE cursor_off[]      = { '\x9b', '\x30','\x20', '\x70' } ;
UBYTE cursor_on[]       = { '\x9b', '\x20', '\x70'} ;
UBYTE displaycolors[]   = { '\x9b', '\x33','\x30',';','\x34','\x32',';','\x32','\x32',';','>','\x32','m' } ;

STATIC BYTE buf[256];

BYTE *con_stream_delim = ";\n\r\t" ;
WORD oldwin_y ;        /* holds window height for resize op */

extern struct Window  *win ;

extern struct GfxBase *GfxBase ;
extern struct Library *GadToolsBase ;
extern struct Library *DiskfontBase ;
extern struct Library *SockBase ;

struct MsgPort *userport = NULL ;

APTR vi = NULL ;   /* visualinfo pointer */
struct Menu *menu = NULL ;

ULONG PT_delay = 0L ;
BOOL exiting = FALSE ;
struct Task *myself ;

/* ==========================================================================
 * main()
 * ==========================================================================
 */ 

extern struct UFB _ufbs[] ;

main(int argc, BYTE **argv)
{
	LONG waitmask = 0L ;
	extern int cons_kybd_sigF ;
	extern int socket_sigio, socket_sigurg ;
	int cc ;
	ULONG event, netevent, urgevent;
	UBYTE hbuf[256] ;
	int bcnt = 0 ;
	int incontrol = 0 ;
	int cnt ;
    UBYTE clipchar ;
	UBYTE *bp ;
	UBYTE c ;
	
	struct RLGlobal *glob = NULL ;
	LONG opts[OPT_COUNT] ;
	
	int dobreak = 0 ;
    
    if((glob = (struct RLGlobal *)AllocVec((LONG)sizeof(struct RLGlobal),MEMF_PUBLIC|MEMF_CLEAR))==NULL)
    {
    	Getout("Insufficient memory\n", NULL) ;
    }
    
    glob->rl_glob_ptr = glob ;

		/* set up non-zero/null default values in the global structure */
	glob->rl_UseScreen = 100L ;
    glob->rl_Unit      = 3L ;
    glob->rl_rows      = 24L ;
    glob->rl_cols      = 80L ;
    glob->rl_resize    = 1L ;
    glob->rl_80wide    = 640L ;
    glob->rl_fontwide  = 8L ;
    glob->rl_fonthigh  = 8L ;
    glob->rl_sockets_are_setup = 0L ;

    CopyMem( "RLamiga/9600", glob->rl_TermType, 12L ) ;
    memset((BYTE *)opts,0,sizeof(opts)) ;    
    
    if((glob->rl_RDargs=(struct RDArgs *)ReadArgs(TEMPLATE,opts,NULL))==NULL)
	{
		Getout("\n", glob) ;
	}
	else
	{
		if( opts[OPT_HOST] )
		{
			strncpy( glob->rl_Host, (BYTE *)opts[OPT_HOST], MAXHOST ) ; 
			glob->rl_hostptr = &glob->rl_Host[0] ;
		}
		if( opts[OPT_USER] )
		{
			strncpy( glob->rl_RemUser, (BYTE *)opts[OPT_USER], MAXUSER ) ; 
			glob->rl_remuserptr = &glob->rl_RemUser[0] ;
		}
		if( opts[OPT_SCREEN] )
		{
			glob->rl_UseScreen = 1L ;
		}
		if( opts[OPT_LEFT] )
		{
			glob->rl_Left = atol((BYTE *)opts[OPT_LEFT]) ;
		}
		if( opts[OPT_TOP] )
		{
			glob->rl_Top = atol((BYTE *)opts[OPT_TOP]) ;
		}
		if( opts[OPT_WIDTH] )
		{
			glob->rl_Width = atol((BYTE *)opts[OPT_WIDTH]) ;
		}
		if( opts[OPT_HEIGHT] )
		{
			glob->rl_Height = atol((BYTE *)opts[OPT_HEIGHT]) ;
		}
		if( opts[OPT_CONUNIT] )
		{
			glob->rl_Unit = atol((BYTE *)opts[OPT_CONUNIT]) ;
		}
		if( opts[OPT_NORESIZE] )
		{
			glob->rl_resize = 0L ;
		}
		if( opts[OPT_TERMTYPE] )
		{
			strncpy( glob->rl_TermType, (BYTE *)opts[OPT_TERMTYPE], MAXTERMTYPE ) ; 
			*(glob->rl_TermType + MAXTERMTYPE - 1) = '\0' ;
		}
		glob->rl_termtypeptr = &glob->rl_TermType[0] ;

		FreeArgs( glob->rl_RDargs ) ;

		if( glob->rl_Top || glob->rl_Left || glob->rl_Width || glob->rl_Height || glob->rl_resize == 0L )
		{
 			if( glob->rl_UseScreen == 100L )
			{
				glob->rl_UseScreen = 0L ;
			}
		}
			
		if( glob->rl_Unit != 1L && glob->rl_Unit != 3L && glob->rl_Unit != 0L ) 
		{
			glob->rl_Unit = 100L ;
		}
	}


	if((SockBase = OpenLibrary( "inet:libs/socket.library", 0L )) == NULL) 
	{
		Getout("Error opening inet:libs/socket.library\n", glob) ;
	}
	
	setup_sockets( MAXSOCKS, &errno );
	glob->rl_sockets_are_setup = 1L ;

	glob->rl_userptr = getlogin() ;
	strncpy( glob->rl_User, glob->rl_userptr, MAXUSER ) ; 
	

	if( glob->rl_remuserptr == NULL )   /* if  '-l' option was not used */ 
	{
		strncpy( glob->rl_RemUser, glob->rl_userptr, MAXUSER ) ; 
		glob->rl_remuserptr = &glob->rl_RemUser[0] ;
	}
	
	PutStr("remote user = >") ;
	PutStr( glob->rl_RemUser ) ;
	PutStr( glob->rl_remuserptr ) ;
	PutStr("<\n\n") ;
	
	PutStr("local user = >") ;
	PutStr( glob->rl_User ) ;
	PutStr( glob->rl_userptr ) ;
	PutStr("<\n") ;


/***	glob->rl_remuserptr = glob->rl_userptr ;  ***/

	if((clipsig = (UBYTE)AllocSignal(-1L)) == -1 )
	{
		Getout("AllocSignal Failed\n", glob) ;
	}
	clipevent = 1L << clipsig ;
	
	myself = FindTask( NULL ) ;
	
#ifdef FOOBAR
	printf("&rl_hostptr    = %ld\n", (LONG)(&glob->rl_hostptr) ) ;
	printf("rl_userptr     = %ld\n", (long)glob->rl_userptr) ;
	printf("rl_remuserptr  = %ld\n", (long)glob->rl_remuserptr) ;
	printf("rl_termtypeptr = %ld\n", (long)glob->rl_termtypeptr) ;
#endif	

	if((glob->rl_socket = rcmd(&glob->rl_hostptr, 513, glob->rl_userptr, glob->rl_remuserptr, glob->rl_termtypeptr,(int	*)NULL))<0L)
	{	
		Getout("RCMD failure!\n", glob) ;
	}

	cc = -1 ;
	if(s_ioctl(glob->rl_socket, FIONBIO, (BYTE *)&cc)<0 || s_ioctl(glob->rl_socket, FIOASYNC, (BYTE *)&cc)<0)
	{
		OUTPUT("ioctl\n") ;
	}

	netevent = (ULONG)(1L << s_getsignal( SIGIO )) ;
	urgevent = (ULONG)(1L << s_getsignal( SIGURG )) ;
	
	if( netevent == 0L || urgevent == 0L )
	{
		Getout("Could not initialize signals\n", glob) ;
	}		


	if(console_init( glob ) < 0)
	{
		Getout("Could not initialize console\n", glob) ;
	}

	console_write( closegad_on, 4 ) ;
	console_write( menu_on, 4 ) ;
#ifdef BG2	
	console_write( displaycolors, 13 ) ;
#endif
	if( glob->rl_resize )
	{
		console_write( gimme_resize, 4 ) ;
	}
	oldwin_y = win->Height ;


	NewList( (struct List *)&paste ) ;

	waitmask = netevent | urgevent | cons_kybd_sigF | clipevent | SIGBREAKF_CTRL_C ;

	do 
	{
		event = Wait( waitmask ) ;
		if( event & SIGBREAKF_CTRL_C )
		{
			exiting = TRUE ;
		}
		if(event & cons_kybd_sigF)
		{
			c = (UBYTE)(console_getchar() & 0xff) ;
			
					/* if we're grabbing a ctrl sequence */
			if( incontrol ) 
			{              
				hbuf[bcnt] = c ;
				bcnt++ ;
					/* valid ESC sequence terminator? then end & send */
				if( ESC_SEQ_TERMINATOR(c) )
				{
					if( c == CON_IDCMP_REPORT ) 
					{
						if( CON_CLOSEGAD( hbuf ))
						{
							console_write( closegad_off, 4 ) ;
							if((exiting = ASKEXIT(glob)) == FALSE)
							{
								console_write( closegad_on, 4 ) ;
							}
						}
						else if( CON_GOTMENU( hbuf ))
						{
							HandleMenu( glob, hbuf ) ;
						}
						else if( CON_RESIZE( hbuf ))
						{
							send_window_size( glob ) ;
							if( glob->rl_Unit == 0L )
							{
								if( win->Height < oldwin_y )
								{
									send(glob->rl_socket, "\r", 1, 0) ;
								}
							}
							oldwin_y = win->Height ;
						}
					}
					
#ifdef SPECIALCHAR /* ---------------------------------------------- */
	/* BE CAREFUL THAT YOU ALLOW FUNCTION KEY STRINGS TO GET THROUGH !! */
					else if( c == CON_SPECIAL_KEY)
					{
						if( hbuf[3] == TILDE )
						{
							if( hbuf[2] == '?' )
							{
								/* VPrintf("HELP KEY\n",  NULL) ; */
							}
						}
					}
#endif /* ------------------------------------------------ */					
					else if( c == CONCLIP_TERM && CONCLIP_PENDING(hbuf))
					{
						if(MakeClipMine())
						{
							Signal(myself, clipevent) ;
						}
					}
					else 
					{
						send( glob->rl_socket, &hbuf[0], bcnt, 0 ) ;
					}
					bcnt = incontrol = 0 ;
				}
			}
			else if( c == 0x9b )    /* start of a control sequence ? */
			{
				incontrol = 1 ;
				hbuf[bcnt] = 0x1b ;
				bcnt++ ;
				hbuf[bcnt] = '[' ;
				bcnt++ ;
			}
			else 
			{
				if( c == 3 && clip_in_progress )
				{
					KillPaste() ;
					clip_in_progress = 0 ;
				}
				else
				{
					send(glob->rl_socket, &c, 1, 0);
					bcnt = incontrol = 0 ;
				}
			}
		}
			
		if( event & urgevent )
		{
			(VOID)handle_oob( glob ) ;
		}
			
		if( event & netevent)
		{
					/* ============================================= */
					/* check for any keypresses in this loop so that */
					/* any control-C's can get handled. If the user  */
					/* hits ANY key while this loop is in effect, we */
					/* break out so that the keypress get's handled  */
					/* in a timely fashion                           */
					/* ============================================= */

			while( (cc = recv(glob->rl_socket, buf, sizeof(buf), 0)) > 0)
			{
				if((SetSignal(0L,0L)) & urgevent )
				{
					dobreak = 1 ;
				}
				if(console_char_ready())
				{
					dobreak = 1 ;
				}                   

				for( cnt = 0, bp = (UBYTE *)&buf ; cnt < cc ; cnt++, bp++  )
				{
					if( (*bp > 127) && (*bp < 160) )
					{
						*bp &= 0x7f ;
					}
				}

					/* handle CTRL-G separately so it's less obnoxious */
					/* console wants to DisplayBeep( NULL ) */
				if( (*buf != 0x07 ) || (glob->rl_UseScreen == 1) )
				{
					console_write( buf, (int)(cc) ) ;
				}
				else
				{
					FlashBar() ;
					if( cc > 1 )
					{
						console_write( &buf[1], cc-1 ) ;
					}
				}

				if( dobreak )
				{
					Signal( myself, netevent ) ; 
					dobreak = 0 ;
					break ;
				}
			}

			if(errno != EWOULDBLOCK)
			{
				if( errno == 0 && cc <= 0 )
				{
					exiting = TRUE ;
				}
			} 
		}

		if( event & clipevent) 
		{
			if( GetClipChar(&clipchar) == 0L )
			{
				if( clipchar )
				{
					send(glob->rl_socket, &clipchar, 1, 0) ;
					if( clipchar <= 13 )
					{
						Delay(PT_delay) ;
					}
				}
				Signal(myself, clipevent) ; /* set the signal again ! */
			}
			else
			{
				clip_in_progress = 0 ;
				KillPaste() ;
			}
		}
	} while ( exiting == FALSE ) ;

	Getout(NULL, glob) ;
}

/* ==========================================================================
 * send_window_size()
 * ==========================================================================
 */

int 
send_window_size( struct RLGlobal *g )
{
	struct {
		BYTE  ws_magic[4] ;         /* magic escape sequence */
		UWORD ws_row, ws_col ;      /* screen size in chars  */
		UWORD ws_width, ws_height ; /* screen size in pixels */
	} ws;
	
	int rows, cols, width, height ;

	console_get_window_sizes(&width, &height, &cols, &rows, g->rl_fontwide, g->rl_fonthigh ) ;

	ws.ws_magic[0] = 0377 ;
	ws.ws_magic[1] = 0377 ;
	ws.ws_magic[2] = 's' ;
	ws.ws_magic[3] = 's' ;
	ws.ws_row      = htons(rows) ;
	ws.ws_col      = htons(cols) ;
	ws.ws_width    = htons(width) ;
	ws.ws_height   = htons(height) ;
	return((int)send(g->rl_socket, (BYTE *)&ws, sizeof(ws), 0) != sizeof(ws) ? -1 : 0) ;
}

/* ==========================================================================
 * handle_oob()
 * ==========================================================================
 */

VOID
handle_oob( struct RLGlobal *g )
{
	BYTE c ;
	STATIC BYTE waste[1024] ;
	
	while(recv(g->rl_socket, &c, sizeof(c), MSG_OOB) < 0)
	{ 
		DB1("oob: recv msg_oob\n") ;
		if (errno != EWOULDBLOCK) 
		{
			return ;
		}
		recv(g->rl_socket,waste,sizeof(waste),0) ;
	}

	if(c & TIOCPKT_WINDOW)
	{
		send_window_size( g ) ;
	}

	if(c & TIOCPKT_FLUSHWRITE)
	{
		for (;;) 
		{
			LONG atmark ;
			int n ;

			atmark = 0; 
			if (s_ioctl(g->rl_socket, SIOCATMARK, (BYTE *)&atmark) < 0) 
			{
				DB1("ioctl\n") ;
				break ;
			}
			if (atmark)
			{
				break ;
			}

			if((n = recv(g->rl_socket, waste, sizeof(waste),0)) <= 0)
			{
				break ;
			}
		}
	}
	if(c & TIOCPKT_NOSTOP)
	{
		DB1("oob: nostop\n") ;
		/* Not implemented */
	}
	if(c & TIOCPKT_DOSTOP)
	{
		DB1("oob: dostop\n") ;
		/* Not implemented */
	}
	if(c & TIOCPKT_FLUSHREAD)
	{
		DB1("oob: flushread\n") ;
		/* Not implemented */
	}
	if(c & TIOCPKT_STOP)
	{
		DB1("oob: stop\n") ;
		/* Not implemented */
	}
	
	if(c & TIOCPKT_START)
	{
		DB1("oob: start\n") ;
		/* Not implemented */
	}
}


/* ========================================================================
 * flash the menu bar as a replacement for the highly obnoxious 
 * DisplayBeep( NULL ) that the console device does when it's fed a CTRL-G
 * ========================================================================
 */
 
VOID
FlashBar( VOID )
{
	REGISTER struct RastPort *rp = win->RPort ;
	REGISTER LONG w = (LONG)win->Width ;
	REGISTER LONG h = (LONG)win->BorderTop ;

	ClipBlit(rp,0L, 0L, rp, 0L, 0L, w, h, 0x50L ) ;
	Delay(10L) ;
	ClipBlit(rp,0L, 0L, rp, 0L, 0L, w, h, 0x50L ) ;
}


/* ========================================================================
 *  Build & Display an AutoRequestor. Works as written under 2.0 and 1.3
 *  Return BOOLEAN  1 for 'yes', 0 for 'no'
 * ========================================================================
 */

#define BODYLEFTOFFSET 20

BOOL
AskExit(  LONG wide, LONG high )
{

	STATIC struct IntuiText pos = {
		0, 1, JAM1, 2, 3, NULL,
		(UBYTE *)" YES",
		NULL
		} ;

	STATIC struct IntuiText neg = {
		0, 1, JAM1, 6, 3, NULL,
		(UBYTE *)"NO",
		NULL
		} ;

	STATIC struct IntuiText body = {
		0, 1, JAM1, BODYLEFTOFFSET, 6, NULL,
		(UBYTE *)" YOU SURE YOU WANT TO QUIT ? ",
		NULL
		} ;

	return((AutoRequest(win,&body,&pos, &neg, 0L,0L, wide, high)) ? TRUE : FALSE) ;
}


/* ========================================================================
 * replacement CXBRK() for Lattice 
 * ========================================================================
 */

int
CXBRK(VOID)
{
	return(0);
}



/* ================================================================
 * DoConClipStuff()
 * ===============================================================
 */

#define TYPE_READ   25
#define TYPE_REPLY  27
	
LONG
MakeClipMine(VOID)
{
	REGISTER struct MsgPort *pasteport ;
	REGISTER struct MsgPort *conclipport ;
	REGISTER struct MinNode *node ;
	LONG retval = 0L ;
			
	if(pasteport = CreateMsgPort()) ;
	{
		cm.cm_Msg.mn_Node.ln_Type = 0 ;
		cm.cm_Msg.mn_Length       = (LONG)sizeof( struct cmdMsg ) ;
		cm.cm_Msg.mn_ReplyPort    = pasteport ;
		cm.cm_type                = TYPE_READ ;
		
		Forbid() ;
		if( conclipport = FindPort("ConClip.rendezvous"))
		{
			PutMsg(conclipport,(struct Message *)&cm) ;
			Permit() ;
			WaitPort(pasteport) ;
			if(cm.cm_error == 0 && cm.cm_type == TYPE_REPLY)
			{
				/* pull pastenodes off conclip list & add them to ours */
				node = (struct MinNode *)RemHead((struct List *)&cm.cm_Chunks) ;
				while( node )
				{
					AddTail((struct List *)&paste,(struct Node *)node) ;
					node = (struct MinNode *)RemHead((struct List *)&cm.cm_Chunks) ;
				}
				pastenode=(struct MinNode *)RemHead((struct List *)&paste) ;
				clip_in_progress = 1 ;
				retval = 1L ;
			}
		}
		else
		{
			Permit() ;
			clip_in_progress = 0 ;
		}
		DeleteMsgPort( pasteport ) ;
	}	                         
	return( retval ) ;
}

/* =========================================================================
 * GetClipChar()
 *
 * This function is called only by main() when the clipevent signal
 * gets set. 
 * =========================================================================
 */

LONG
GetClipChar( UBYTE *thechar )
{
	UBYTE ch ;
	
	if( pastenode )
	{
		ch = ((struct CHRSChunk *)pastenode)->text[pcount++] ;
		if( pcount > ((struct CHRSChunk *)pastenode)->size )
		{
			if(((struct CHRSChunk *)pastenode)->freeme)
			{
				FreeVec(((struct CHRSChunk *)pastenode)->freeme) ;
			}
			FreeVec(pastenode) ;
			pastenode = (struct MinNode*)RemHead((struct List *)&paste) ;
			pcount = 0L ;
		}
		*thechar = ch ;
		return(0L) ;
	}					
	*thechar = 0 ;
	return(1L) ;
}

/* =====================================================================
 * KillPaste( VOID )
 * =====================================================================
 */

VOID
KillPaste(VOID)
{
	while(pastenode)	
	{
		if(((struct CHRSChunk *)pastenode)->freeme)
		{
			FreeVec(((struct CHRSChunk *)pastenode)->freeme) ;
		}
		FreeVec(pastenode) ;
		pastenode = (struct MinNode*)RemHead((struct List *)&paste) ;
	}
	pcount = 0L ;
	clip_in_progress = 0 ;
}

/* ==========================================================
 * HandleMenu()
 * =======================================================
 */

VOID
HandleMenu( struct RLGlobal *g, UBYTE *data )
{
	REGISTER BYTE   *command ;
	REGISTER struct MenuItem *item ;
	REGISTER ULONG  class ;
	REGISTER USHORT subclass ;
	REGISTER USHORT code ;

	class    = (ULONG )atol(strtok((BYTE *)&data[2], con_stream_delim)) ;
	subclass = (USHORT)atol(strtok( NULL, con_stream_delim)) ; 
	code     = (USHORT)atol(strtok( NULL, con_stream_delim)) ;

	while( code != MENUNULL )
	{
		item = (struct MenuItem *)ItemAddress( menu, (LONG)code ) ;
		command = (BYTE *)GTMENUITEM_USERDATA( item ) ;

		if( ! strncmp(command, "PT", 2 ))
		{
			PT_delay = atol( &command[2] ) ;
		}
		else if( (strcmp( command, "RESET" )) == 0)
		{
			console_write( "\xF", 1 ) ;
		}
		else if( (strcmp( command, "ABOUT" )) == 0)
		{
			About() ;
		}
		else if( (strcmp( command, "QUIT" )) == 0)
		{
			console_write( closegad_off, 4 ) ;
			if((exiting = ASKEXIT(g)) == FALSE)
			{
				console_write( closegad_on, 4) ;
			}
		}
		code = item->NextSelect ;
	}
}

/* ---------------------------------------------------
 * Getout( BYTE *msg, struct RLGlobal *g )
 * ---------------------------------------------------
 */

VOID
Getout( BYTE *msg, struct RLGlobal *g )
{
	if( (g != NULL) && (g->rl_sockets_are_setup == 1L) )
	{
		cleanup_sockets() ;
	}
	if( SockBase )
	{
		CloseLibrary( SockBase ) ;
		SockBase = NULL ;
	}
	
	console_close( ) ;

	if( g )
	{
		FreeVec( g ) ;
	}
	if( clipsig != -1 )
	{
		FreeSignal((LONG)clipsig) ;
	}
	if( msg )  /* no message means no error so we */
	{          /* we return to fall off the end of main() */
		Close(_ufbs[2].ufbfh) ;
		_ufbs[2].ufbflg = 0L ;
		exit(VPrintf(msg,NULL) ? 20 : 0) ;
	}
}


/* -------------------------------------------------
 * About
 * -------------------------------------------------
 */

#define NAME "        RLOGIN        "

VOID
About(VOID)
{
	struct EasyStruct es = {
		(LONG)sizeof(struct EasyStruct),
		0L,
		(UBYTE *)"  About  ",
		(UBYTE *)"%s\nVersion %ld.%02ld (%s)",
		(UBYTE *)"  Continue  "
		} ;
	
	EasyRequest( win, &es, NULL, NAME,VERSION,REVISION,DATE) ;
}
