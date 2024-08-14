/* -----------------------------------------------------------------------
 * RLOGIN.C  (Lattice 5.10 )
 *
 * $Locker: bj $
 *
 * $Id: rlogin.c,v 1.13 91/08/06 13:29:35 bj Exp Locker: bj $
 *
 * $Revision: 1.13 $
 *
 * $Log:	rlogin.c,v $
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
 * $Header: HOG:Other/inet/src/c/rlogin/RCS/rlogin.c,v 1.13 91/08/06 13:29:35 bj Exp Locker: bj $ 
 *
 *------------------------------------------------------------------------
 */

#define BETA_COPY 1
#define MAIN 1

/* #define DEBUG 1 */

/* #define BG2 1 */    /* do this if you want background color to be color #2 */
                    /* this requires a matching termcap !!!!!!!  */

#define RDARGS_VERSION 1

#ifdef DEBUG
	#define DB1(x)  PutStr(x)
#else
	#define DB1(x) ;
#endif

#include "rlogin_rev.h"

#include <exec/types.h>
#include <ss/socket.h>
#include <sys/socket.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <devices/conunit.h>
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

#include <ios1.h>
#include <proto/all.h>

#ifdef min
   #undef min
#endif

#include <stdio.h>   /* after the #undef min */
#include <errno.h>

#include "rlglobal.h"

BYTE *version = VERSTAG ;
BYTE clipbuffer[CLIPBUFSIZE] ;
STATIC BYTE buf[INBUFFSIZE] ;  /* buffer for incoming data from net */

/********************************************************
 * this is the stuff that allows jump scrolling
 *******************************************************
 */


ULONG jumpscroll = TRUE ;
ULONG linewrap = TRUE ;
WORD chars_in_blockbuf ;
BYTE *blockptr ;
WORD block_count ;
#define MAXINBLOCKS 10
#define INBUFFSIZE 256
BYTE blockbuf[(INBUFFSIZE * MAXINBLOCKS) + 10] ;  /* buffer to hold incoming */
                                                  /* while we calc scroll jumps */

BYTE *con_stream_delim = ";\n\r\t" ;
WORD oldwin_y ;        /* holds window height for resize op */

extern struct ConUnit *conunit ;

extern struct Window  *win ;

extern struct GfxBase *GfxBase ;
extern struct Library *GadToolsBase ;
extern struct Library *DiskfontBase ;
extern struct Library *SockBase ;
struct MsgPort *userport = NULL ;

struct Menu *menu = NULL ;

ULONG PT_delay = 0L ;
LONG exiting = FALSE ;
struct Task *myself ;

/* ==========================================================================
 * void main(void)
 * ==========================================================================
 */ 

extern struct UFB _ufbs[] ;

VOID main(VOID) ;

VOID 
main(VOID)
{
	LONG waitmask ;
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

	struct RLGlobal *glob ;
	LONG opts[OPT_COUNT] ;

	int dobreak   = 0 ;
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

    CopyMem( "RLamiga/9600", glob->rl_TermType, 12L) ;
    memset((BYTE *)opts,0,sizeof(opts)) ;    
    memset((BYTE *)blockbuf,0,sizeof(blockbuf)) ;    
    
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
			strncpy( glob->rl_User, (BYTE *)opts[OPT_USER], MAXUSER ) ; 
			glob->rl_userptr = &glob->rl_User[0] ;
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


	if( glob->rl_userptr == NULL )   /* done only if  '-l' option was omitted */ 
	{
		glob->rl_userptr = getlogin() ;
	}

	glob->rl_remuserptr = glob->rl_userptr ;

	if((clipsig = (UBYTE)AllocSignal(-1L)) == -1 )
	{
		Getout("AllocSignal Failed\n", glob) ;
	}
	clipevent = 1L << clipsig ;

	myself = FindTask( NULL ) ;

	if((glob->rl_socket = rcmd(&glob->rl_hostptr, 513, glob->rl_userptr, glob->rl_remuserptr, glob->rl_termtypeptr,(int	*)NULL))<0L)
	{
		Getout("RCMD failure!\n", glob) ;
	}

	cc = -1 ;
	if(s_ioctl(glob->rl_socket, FIONBIO, (BYTE *)&cc)<0 || s_ioctl(glob->rl_socket, FIOASYNC, (BYTE *)&cc)<0)
	{
		PutStr("ioctl\n") ;
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

	CLOSEGAD_ON ;
	MENU_ON ;

	if( glob->rl_resize )
	{
		RESIZE_ON ;
	}

	oldwin_y = win->Height ;


#ifdef BETA_COPY
	console_write("*** THIS IS AN INTERMEDIATE RLOGIN. THIS IS A BETA AND IS NOT FOR RELEASE ***\n",78L) ;
	console_write("*** THIS IS AN INTERMEDIATE RLOGIN. THIS IS A BETA AND IS NOT FOR RELEASE ***\n",78L) ;
#endif

#ifdef BG2
	console_write("\n=========================================\n", 43L ) ;
	console_write("*** THIS REQUIRES A SPECIAL TERMCAP!! ***\n", 42L ) ;
	console_write("=========================================\n", 42L ) ;
	console_write( displaycolors, 13 ) ;
#endif

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
							CLOSEGAD_OFF ;
							if((exiting = AskExit()) == 0L)
							{
								CLOSEGAD_ON ;
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

#ifdef SPECIAL  /* ---------------------------------------------- */
                /* -- THIS CODE IS WRONG -- THIS CODE IS WRONG -- */
                /* BE CAREFUL THAT YOU ALLOW FUNCTION KEY STRINGS */
                /* TO GET THROUGH !!                              */
                /*                                                */
                /*  else if( c == CON_SPECIAL_KEY)                */
                /*  {                                             */
                /*      if( hbuf[3] == TILDE )                    */
                /*      {                                         */
                /*          if( hbuf[2] == '?' )                  */
                /*          {                                     */
                /*              /* VPrintf("HELP KEY\n",  NULL) ; */
                /*          }                                     */
                /*      }                                         */
                /*  }                                             */
                /*                                                */
#endif          /* ---------------------------------------------- */

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
			handle_oob( glob ) ;
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

            block_count = chars_in_blockbuf = 0 ;
            blockptr = blockbuf ;
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

				if( jumpscroll == FALSE )
				{
                   /* handle CTRL-G separately so it's less obnoxious   */
                   /* console wants to DisplayBeep( NULL )              */
                   /* This is more than cosmetic with systems that      */
                   /* fight you and send multiple bells. Can bog system */

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
				}
				else
				{
					CopyMem(buf,blockptr,(ULONG)cc) ;
					block_count++ ;
					blockptr += cc ;
					chars_in_blockbuf += cc ;

					if( (cc < sizeof(buf)) || (block_count >= MAXINBLOCKS ) )
					{
						flush_buffered_blocks( blockbuf, (ULONG)chars_in_blockbuf) ;
						Signal( myself, netevent ) ; 
						block_count = chars_in_blockbuf = 0 ;
						blockptr = blockbuf ;

					}
				}

				if( dobreak )
				{
					Signal( myself, netevent ) ; 
					dobreak = 0 ;
					break ;
				}
			}

			if( jumpscroll == TRUE )
			{
				flush_buffered_blocks( blockbuf, (ULONG)chars_in_blockbuf) ;
				block_count = chars_in_blockbuf = 0 ;
				blockptr = blockbuf ;
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
 * flush_buffered_blocks( UBYTE *buffer, ULONG charcount )
 *
 * this function controls the output of the buffered text. This is
 * for jump scrolling.
 * ==========================================================================
 */

VOID
flush_buffered_blocks( UBYTE *buffer, REGISTER ULONG charcount )
{
	REGISTER WORD linefeeds ;
	REGISTER ULONG x ;
	REGISTER UBYTE *bp ;
	REGISTER LONG xmax = (LONG)conunit->cu_XMax + 1 ;
	REGISTER LONG ymax = (LONG)conunit->cu_YMax + 1 ;


	linefeeds = countlfs(buffer,charcount, xmax) ;

	if(linefeeds > ymax)
	{
		bp = buffer ;
		x = 0 ;
		while((charcount >= INBUFFSIZE) && (x < MAXINBLOCKS))
		{
			doscroll( countlfs( bp,(LONG)INBUFFSIZE,(ULONG)xmax)) ;
			console_write(bp,INBUFFSIZE) ;
			bp += INBUFFSIZE ;
			charcount -= INBUFFSIZE ;
			x++ ;
		}
		doscroll(countlfs(bp,charcount,xmax)) ;
		console_write(bp,charcount) ;
	}
	else
	{
		doscroll(linefeeds) ;
		console_write(buffer,charcount) ;
	}
}

/* ======================================================================
 * countlfs( UBYTE *buffer, ULONG charcount, ULONG max_width)
 *
 * This returns the number of "line feeds" in a given block of text.
 * In this case 'line feed' means either a true ascii 10 -or- a 'virtual
 * line feed' which means that any line that is longer than the display
 * is wide will cause the console device to scroll if the cursor is
 * at the bottom of the display. Example :  if the current console
 * window is 80 chars wide and we send a line to it that is 166 chars
 * long, it will want three (3) actual line feeds.
 * ====================================================================== 
 */

ULONG
countlfs(REGISTER UBYTE *b, REGISTER ULONG cnt, REGISTER ULONG wide )
{
	REGISTER UBYTE *bp1 ;
	REGISTER ULONG x,lfs, llen ;

	for( bp1 = b, lfs = llen = x = 0L; x < cnt ; x++, bp1++)
	{
		if( *bp1 == 10 )
		{
			lfs++ ;
			llen = 0L ;
		}
		if( linewrap && (llen > wide) )
		{
			lfs++ ;
			llen =  0L ;
		}
	}
	return(lfs) ;
}

VOID
doscroll(REGISTER ULONG lines)
{

	REGISTER LONG xpos = (LONG)conunit->cu_XCP + 1 ;
	REGISTER LONG ypos = (LONG)conunit->cu_YCP + 1 ;
	REGISTER LONG ymax = (LONG)conunit->cu_YMax + 1 ;

	REGISTER LONG scrollup ;
	REGISTER LONG jump ;
	UBYTE textbuffer[128] ;

	scrollup = (ypos + lines) - ymax ;
	jump = (long)(ypos-scrollup) ; 

	if( (ypos + lines) > ymax + 1 )
	{
        mysprintf( textbuffer, "\x1b[%ld;%ldH",jump,(LONG)xpos) ;
		console_write( textbuffer, (LONG)strlen(textbuffer)) ;
       
			/* scroll */
		mysprintf(textbuffer,"\x1b[%ldS\0", (LONG)scrollup) ; 
		console_write( textbuffer, (LONG)strlen(textbuffer)) ;
	}
}


/* ==========================================================================
 * send_window_size()
 * ==========================================================================
 */

VOID 
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
	send(g->rl_socket, (BYTE *)&ws, sizeof(ws), 0) ;
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
	REGISTER UWORD  subclass ;
	REGISTER UWORD  code ;

  /* --------not currently used -------------------------------------- */
    class    = (ULONG )atol(strtok((BYTE *)&data[2], con_stream_delim)) ;
    subclass = (UWORD)atol(strtok( NULL, con_stream_delim)) ; 
  /* ----------------------------------------------------------------- */
   
	code = (UWORD)atol(strtok( NULL, con_stream_delim)) ;

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
		else if( (strcmp( command, "LWRAP" )) == 0)
		{
			if(linewrap)
			{
				LINEWRAP_OFF ;
				linewrap = FALSE ;
			}
			else
			{
				LINEWRAP_ON ;
				linewrap = TRUE ;
			}
		}
		else if( (strcmp( command, "JSC" )) == 0)
		{
			jumpscroll = jumpscroll ? FALSE : TRUE ;
		}
		else if( (strcmp( command, "ABOUT" )) == 0)
		{
			About() ;
		}
		else if( (strcmp( command, "QUIT" )) == 0)
		{

			CLOSEGAD_OFF ;
			if((exiting = AskExit()) == 1L )
			{
				CLOSEGAD_ON ;
			}
		}
		code = item->NextSelect ;
	}
}

/* ---------------------------------------------------
 * Getout( BYTE *msg, struct RLGlobal *g )
 * ---------------------------------------------------
 */

extern APTR stderr_console_task ;

VOID
Getout( BYTE *msg, struct RLGlobal *g )
{
	struct Process *proc = (struct Process *)FindTask(NULL) ;
	APTR console_task ;
	
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
		
#ifdef OLD_STYLE_EXIT  /* <= 37.14 */
		Close(_ufbs[2].ufbfh) ;
		_ufbs[2].ufbflg = 0L ;
		exit(VPrintf(msg,NULL) ? 20 : 0) ;
#else
		console_task = (APTR)proc->pr_ConsoleTask ;
		proc->pr_ConsoleTask=stderr_console_task ;
		Close(_ufbs[2].ufbfh) ;
		_ufbs[2].ufbflg = 0L ;
		proc->pr_ConsoleTask = console_task ;
		exit(VPrintf(msg,NULL) ? 20 : 0) ;
#endif

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
		(UBYTE *)"  About",
		(UBYTE *)"%s\nVersion %ld.%02ld (%s)",
		(UBYTE *)"  Continue  "
		} ;

	EasyRequest( win, &es, NULL, NAME,VERSION,REVISION,DATE) ;
}

/* -------------------------------------------------
 * AskExit
 * --------------------------------------------------
 */

LONG
AskExit(VOID)
{
	extern BYTE wtitle[] ;

	struct EasyStruct askes = {
		(LONG)sizeof(struct EasyStruct),
		0L,
		(UBYTE *)"  Rlogin",
		(UBYTE *)"%s\nAre You Sure You Want To Quit?",
		(UBYTE *)"  Yes  |  No  "
		} ;

	return(EasyRequest( win, &askes, NULL,(APTR)wtitle,NULL,NULL,NULL)) ;
/*	return((EasyRequest( win, &askes, NULL,(APTR)wtitle,NULL,NULL,NULL) ==0L) ? 0L : 1L) ; */
}
