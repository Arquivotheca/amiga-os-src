/*---------------------------------------------
 *  CON_WINDOW.C Demo  in Manx C   Brian Jackson
 *
 * This demo shows how to read text clips that the user wants you
 * to insert from the conclip process.  As of this date this is
 * "cheating" in that this read should actually come from the 
 * ClipBoard Device via IFFParse.  This reads the clip directly
 * from the conclip process.
 *
 * Requires 2.0
 *
 * May 19, 1991
 *-------------------------------------------------------------
 */

 #include <exec/types.h>
 #include <exec/io.h>
 #include <exec/libraries.h>
 #include <exec/ports.h>
 #include <exec/memory.h>
 #include <exec/execbase.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <intuition/intuition.h>
 #include <intuition/screens.h>
 #include <devices/timer.h>
 #include <devices/conunit.h>
 #include <devices/console.h>
 #include <devices/serial.h>
 #include <graphics/rastport.h>
 #include <graphics/gfxbase.h>
 #include <graphics/text.h>
 #include <dos/var.h>
 #include <libraries/dos.h>
 #include <utility/tagitem.h>
 
 #include <proto/all.h>

 #define  RN          36L
 #define  CTRL_C      03
 #define  CR          13
 #define  LF          10
 #define  CNAM        ("console.device")
 #define  WAITMASK    (cons_kybd_sigF | conclip_event | SIGBREAKF_CTRL_C)
 #define  SIZE(x)     ((long)sizeof(x))
 
 #define scroll "\x1b[5S\0"
 
 typedef struct IOStdReq  * IOSTDREQPTR ;
 typedef struct IORequest * IOREQPTR ;
 typedef struct Message   * MSGPTR ;
 typedef struct MinNode   * MINNODEPTR ;
 typedef struct Library   * LIBPTR ;
 typedef struct MsgPort   * MSGPORTPTR ;
 typedef struct List      * LISTPTR ;
 typedef struct CHRSChunk * CHRSPTR ;
 typedef struct Node      * NODEPTR ;

        /* ------------ */      /* conclip stuff */  /* ---------------- */

 struct CHRSChunk {
	struct MinNode mn;
	char *text;             /* pointer to start of text                  */
	char *freeme;           /* if non-zero, call FreeVec() with this ptr */
	ULONG  size;            /* total # of characters in this block       */
	ULONG  flags;           /* may end up needing this at some time      */
	ULONG  UserLink;
	char  data[1];          /* really 'n' # of bytes                     */
	} ;

 struct cmdMsg {
	struct Message cm_Msg;
	long                cm_type;    /* type o msg, and return validation */
	long                cm_error;   /* error code return                 */
	struct MinList      cm_Chunks;  /* the text chunks                   */
	} ;

	struct MinList paste ;
	struct MinNode *pastenode ;
	struct cmdMsg cm = {0} ;

	int   clip_in_progress ;
	int   cliperror ;
	BYTE  clipsig = -1 ;
	ULONG conclip_event ;
	ULONG pcount  = 0L ;

 #define CONCLIP_TERM      0x76  /* 'v'  */
 #define CONCLIP_PENDING(b) (((b)[2]==0x30 && (b)[3]==0x20 && (b)[4]==0x76))
 #define USER_WANTS_CLIP(c,b) ((c)==CONCLIP_TERM && CONCLIP_PENDING((b)))

	struct Task *myself ;

 #define ESC_SEQ_TERMINATOR(x) (((x) >= 0x40) && ((x) <= 0x7e)) 

	/* if using a sole '9b' as the escape then these values */
	/* need to be reduced by one */

 #define CON_CLOSEGAD(b) (((b)[2] == 0x31 && (b)[3] == 0x31))
 #define CON_RESIZE(b)   (((b)[2] == 0x31 && (b)[3] == 0x32))

 #define CSI               0x9b
 #define CON_IDCMP_REPORT  0x7c
 #define CON_OPTION_ON     0x68  /* lower case 'h' */
 #define CON_OPTION_OFF    0x6c  /* lower case 'l' */
 #define CON_SPECIAL_KEY   0x7e  /* tilde */
 #define TILDE             0x7e  /* tilde */

 unsigned char closegad_on[]   = { '\x9b', '1','1','{','\0' } ;
 unsigned char closegad_off[]  = { '\x9b', '1','1','}','\0' } ;
 unsigned char resize_on[]     = { '\x9b', '1','2','{','\0' } ;
 
 unsigned char linewrap_on[]   = { '\x9b', '\x3f', '\x37', '\x68' } ;
 unsigned char linewrap_off[]  = { '\x9b', '\x3f', '\x37', '\x6c' } ;
 unsigned char lfmode_on[]     = { '\x9b', '\x32', '\x30', '\x68' } ; 
 unsigned char lfmode_off[]    = { '\x9b', '\x32', '\x30', '\x6c' } ; 
 unsigned char cursor_off[]    = { '\x9b', '\x30', '\x20', '\x70' } ;
 unsigned char cursor_on[]     = { '\x9b', '\x20', '\x70'} ;


	struct IntuitionBase *IntuitionBase ;
	struct GfxBase       *GfxBase ;

	struct MsgPort  *crp, *cwp ;       /* console read/write ports     */
	struct IOStdReq *crreq, *cwreq ;   /* con read/write requests      */
	
	int   cons_kybd_sigF ;             /* signal mask for console read */
	int   console_is_open ;
	LONG  conunit = 3L ;               /* need #3 for cut&paste        */
	LONG  event ;                      /* holds signal ret'd by Wait() */
	ULONG waitmask ;                   /* OR'd signals for Wait() call */
  
	char *title = " Window Title " ; 
  
	struct NewWindow newwindow = {
		0,50,                              /* left edge, top edge   */        
		639,100,                           /* width, height         */        
		0,1,                               /* detail pen, blk pen   */
		0L,                                /* IDCMP flags (console! */
		WINDOWSIZING | SIZEBRIGHT          /* Flags                 */
		  | WINDOWDEPTH | WINDOWDRAG
		  | SMART_REFRESH | ACTIVATE 
		  | NOCAREREFRESH | WINDOWCLOSE,
		NULL,                              /* firstgadget           */
		NULL,                              /* checkmark             */
		(UBYTE *)" Window Title ",         /* title                 */
		NULL,                              /* Screen Pointer        */
		NULL,                              /* bitmap pointer        */
		180,50,                            /* minwidth, minheight   */
		-1,-1,                             /* maxwidth, maxheight   */
		WBENCHSCREEN                       /* screentype            */
		} ;

	unsigned char hbuf[96] ;               /* ctrl seq buffer       */

	char  console_getchar( VOID ) ;
	int   console_write(unsigned char *buf, int len) ;
	VOID  main( VOID ) ;
	ULONG MakeClipMine(VOID) ;
	UBYTE GetClipChar( int *errval ) ;
	VOID  KillPaste( VOID ) ;

	VOID _cli_parse( struct Process *p, char *a) {} 
	VOID _wb_parse( struct Process *p, struct WBStartup *wbm ) {}


VOID
main(VOID)
{
	ULONG class;
	struct Window *win = NULL ;
	int exiting        = 0 ;
	int incontrol      = 0 ;
	int bcnt           = 0 ;
	UBYTE clipchar ;
	unsigned char c ;
	
	if(IntuitionBase = OpenLibrary("intuition.library",RN ))
	{
		if( GfxBase  = (struct GfxBase *)OpenLibrary( GRAPHICSNAME,RN) )
		{
			if ((win = (struct Window *)OpenWindow(&newwindow)))
			{
				crp     = CreateMsgPort() ;
				cwp     = CreateMsgPort() ;
				crreq   = CreateStdIO(crp) ;
				cwreq   = CreateStdIO(cwp) ;
				clipsig = AllocSignal( -1L ) ;   
				if(crreq && cwreq && cwp && crp && (clipsig != -1))
				{
					conclip_event    = 1L << clipsig ;
					cons_kybd_sigF   = 1L << crp->mp_SigBit ;
					cwreq->io_Data   = (APTR)win;
					cwreq->io_Length = sizeof(*win);
					if(!OpenDevice( CNAM, conunit, (IOREQPTR)cwreq, 0L))
					{
						*crreq = *cwreq;
						crreq->io_Message.mn_ReplyPort    = crp;
						crreq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
						myself = FindTask(NULL) ;
						console_getchar();                /* start the read */
						console_write( lfmode_on, 4 ) ;   /* LF = CRLF      */
						console_write( closegad_on, 4 ) ; /* Hear CloseGads */
						console_write( resize_on, 4 ) ;   /* Hear resizes   */
						waitmask = WAITMASK ;             /* careful here!  */
						NewList( (LISTPTR)&paste ) ;

						do  /* main loop */
						{
							event = Wait(waitmask) ;
							if( event & SIGBREAKF_CTRL_C ) /* external */
							{
								if( clip_in_progress ) 
								{
									KillPaste() ;
								}
								exiting = 1 ;
							}
							if( event & conclip_event )
							{
								clip_in_progress = 1 ;
								clipchar = GetClipChar(&cliperror) ;
								if( cliperror == 0 )
								{
									console_write(&clipchar, 1) ;
									Signal(myself, conclip_event) ;
								}
								else
								{
									KillPaste() ;
								}
							}
							if( event & cons_kybd_sigF )
							{
								c = (UBYTE)(console_getchar() & 0xff) ;
								
									/* if we're grabbing a ctrl sequence */
								if( incontrol ) 
								{              
									hbuf[bcnt] = c ;
									bcnt++ ;
										/* valid ESC sequence terminator? */
										/* then end & send */
									if( ESC_SEQ_TERMINATOR(c) )
									{
										if( c == CON_IDCMP_REPORT ) 
										{
											if( CON_CLOSEGAD( hbuf ))
											{
												VPrintf("con_close_gad\n", NULL) ;
												exiting = 1 ;
											}
											else if( CON_RESIZE( hbuf ))
											{
												VPrintf("resize\n",NULL) ;
												console_write( lfmode_on, 4 ) ;
											}
										}
										else if(USER_WANTS_CLIP((c),(hbuf)))
										{
											MakeClipMine() ;
										}
										else  /* default */
										{
											console_write( &hbuf[0], bcnt ) ;
										}
										bcnt = incontrol = 0 ;
									}
								}
								else if( c == 0x9b )  /* start ctrl seq? */
								{
									incontrol = 1 ;
									hbuf[bcnt] = 0x1b ;
									bcnt++ ;
									hbuf[bcnt] = '[' ;
									bcnt++ ;
								}
								else 
								{                /* user hit CTRL-C ?? */
									if( c == CTRL_C && clip_in_progress )
									{
										KillPaste() ;
									}
									else if( c == 'J' )
									{
										console_write(scroll, (long)strlen(scroll)) ;
									}
									else
									{
										if( c == CR )
										{
											c = LF ;
										}
										console_write( &c, 1) ;
										bcnt = incontrol = 0 ;
									}
								}
							}
						}
						while( !exiting ) ;  /* do...while */

						AbortIO((IOREQPTR)cwreq) ;
						WaitIO((IOREQPTR)cwreq) ;
						AbortIO((IOREQPTR)crreq) ;
						WaitIO((IOREQPTR)crreq) ;
						CloseDevice((IOREQPTR)cwreq) ;
					}
				}
				if( clipsig != -1 )
				{
					FreeSignal( (long)clipsig ) ;
					clipsig = -1 ;
				}
				if( crreq )
				{
					DeleteStdIO(crreq) ;  
				}
				if( cwreq )
				{
					DeleteStdIO(cwreq) ;    
				}
				if( crp )
				{
					DeleteMsgPort( crp ) ;
				}
				if( cwp )
				{
					DeleteMsgPort ( cwp ) ;
				}
				CloseWindow( win ) ;
			}        /* if win */
			CloseLibrary((LIBPTR)GfxBase ) ;
		}
		CloseLibrary((LIBPTR)IntuitionBase ) ;
	}  
}       /* main */


                        
   
/* =============================================================
 * console_getchar()
 * =============================================================
 */

char
console_getchar( VOID )
{
	static char c;
	char rtn;

	if(!CheckIO((IOREQPTR)crreq))
	{
		WaitIO((IOREQPTR)crreq);
	}

	rtn = c;
	crreq->io_Command = CMD_READ;
	crreq->io_Data    = (APTR)&c;
	crreq->io_Length  = sizeof(c);

	SendIO((IOREQPTR)crreq);

	return( rtn ) ;
}


/* =============================================================
 * console_write()
 * =============================================================
 */

unsigned char cwarray[88] ;

int
console_write(unsigned char *buf, int len)
{

	/* break buf into small pieces */ 

	while ( len > 80 ) 
	{
		cwreq->io_Data    = (APTR)buf ;
		cwreq->io_Length  = 80L ;
		cwreq->io_Command = CMD_WRITE;
		DoIO((IOREQPTR)cwreq);
		buf += 80;
		len -= 80;
	}

	if (len) 
	{
		cwreq->io_Data    = (APTR)buf ;
		cwreq->io_Length  = (long)len ;
		cwreq->io_Command = CMD_WRITE ;
		DoIO((IOREQPTR)cwreq) ;
	}

	return( len ) ;
}


/* =====================================================
 * MakeClipMine()
 * =====================================================
 */


#define TYPE_READ   25
#define TYPE_REPLY  27
        
ULONG
MakeClipMine(VOID)
{
	MSGPORTPTR pasteport ;
	MSGPORTPTR conclipport ;
	MINNODEPTR node ;
	ULONG totalchars = 0L ;
	
	pcount = 0L ;
	if(pasteport = (MSGPORTPTR)CreateMsgPort()) ;
	{
		cm.cm_Msg.mn_Node.ln_Type = 0 ;
		cm.cm_Msg.mn_Length       = (long)sizeof( struct cmdMsg ) ;
		cm.cm_Msg.mn_ReplyPort    = pasteport ;
		cm.cm_type                = TYPE_READ ;
		
		Forbid() ;
		if( conclipport = FindPort("ConClip.rendezvous"))
		{
			PutMsg(conclipport,(MSGPTR)&cm) ;
			Permit() ;
			WaitPort(pasteport) ;
			if(cm.cm_error == 0 && cm.cm_type == TYPE_REPLY)
			{
				/* pull pastenodes off conclip list & add to ours */
				node = (MINNODEPTR)RemHead((LISTPTR)&cm.cm_Chunks) ;
				while( node )
				{
					AddTail((LISTPTR)&paste,(NODEPTR)node) ;
					node = (MINNODEPTR)RemHead((LISTPTR)&cm.cm_Chunks) ;
				}
				pastenode=(MINNODEPTR)RemHead((LISTPTR)&paste) ;
			}
			clip_in_progress = 1 ;
			Signal(myself, conclip_event) ;
		}
		else
		{
			Permit() ;
		}
		DeleteMsgPort( pasteport ) ;
	}
}

/* =========================================================================
 * GetClipChar()
 *
 * This function is called only by main() when the conclip_event signal
 * gets set.  This version gets a single character at a time.
 * =========================================================================
 */

UBYTE
GetClipChar( int *errval )
{
	UBYTE ch = 0 ;

	if( pastenode )
	{
		ch = ((CHRSPTR)pastenode)->text[pcount++] ;
		if( pcount > ((CHRSPTR)pastenode)->size )
		{
			if(((CHRSPTR)pastenode)->freeme)
			{
				FreeVec(((CHRSPTR)pastenode)->freeme) ;
			}
			FreeVec(pastenode) ;
			pastenode = (MINNODEPTR)RemHead((LISTPTR)&paste) ;
		}
		*errval = 0 ;
		return(ch) ;
	}
	*errval = 1 ;
	return(0) ;
}


/* ===============================================================
 * KillPaste()
 *
 * this func gets called when the program needs to abort any
 * 'in progress' clips.  When exiting the program or if the user
 * hits CTRL-C during a clip, this function will free any pending
 * pastenodes that require freeing (in this case, all of them ).
 * ===============================================================
 */


VOID
KillPaste(VOID)
{
	while(pastenode) 
	{
		if( ((CHRSPTR)pastenode)->freeme)
		{
			FreeVec(((CHRSPTR)pastenode)->freeme);
		}
		FreeVec(pastenode);
		pastenode=(MINNODEPTR)RemHead((LISTPTR)&paste);
	}
	pcount = 0L ;                                
	clip_in_progress = 0 ;
}
