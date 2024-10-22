/* -----------------------------------------------------------------------
 * rlglobal.h    header file for rlogin (as225)
 *
 * $Locker:  $
 *
 * $Id: rlglobal.h,v 37.21 93/02/08 16:04:30 bj Exp $
 *
 * $Revision: 37.21 $
 *
 * $Log:	rlglobal.h,v $
 * Revision 37.21  93/02/08  16:04:30  bj
 * Binary 37.24
 * 
 * Added SA_Interleaved=TRUE tag to screentags[] array
 * 
 * Removed NOCAREREFRESH flag from OpenWindow() call.
 * 
 * Revision 37.20  92/08/10  14:43:41  bj
 * Changed NewWindow and NewScreen structs to ExtNew... to support NewLook
 * Menus (tags needed.)
 * 
 * Added windowtags[] and screentags[] arrays.
 * 
 * Revision 37.19  92/08/08  19:50:49  bj
 * Added "NO_TYPE" define for use with AlterMenu() function.
 * GadTools menus use "NM_END", which is defined as '0' (zero)
 * so a special value was needed to differentiate between
 * "NM_END" and 'no type' in the args passed to the function.
 * 
 * AS225 R2
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/rlglobal.h,v 37.21 93/02/08 16:04:30 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/* rlglobal.h */

#define TEMPLATE "HOST/A,-L=USER/K,-C=SCREEN/S,-X=LEFT/K/N,-Y=TOP/K/N,-W=WIDTH/K/N,-H=HEIGHT/K/N,-T=TERM/K,-U=UNIT/K/N,-R=NORESIZE/S,-J=JUMP/S,-WR=WRAP/S,-MC=MAXCOLS/K/N,-MR=MAXROWS/K/N,-PD=PASTEDELAY/K/N"

#define MAXHOST 128L
#define MAXUSER 64L
#define MAXTERMTYPE 128L 

#define OPT_HOST       0
#define OPT_USER       1
#define OPT_SCREEN     2
#define OPT_LEFT       3
#define OPT_TOP        4
#define OPT_WIDTH      5
#define OPT_HEIGHT     6
#define OPT_TERMTYPE   7
#define OPT_CONUNIT    8
#define OPT_NORESIZE   9
#define OPT_JUMP      10
#define OPT_WRAP      11
#define OPT_MAXWIDTH  12
#define OPT_MAXHEIGHT 13
#define OPT_PASTE     14
#define OPT_COUNT     15

struct RLGlobal {                             /* default*/
	LONG  rl_UseScreen ;                      /*   0    */
	LONG  rl_Width ;                          /*   0    */
	LONG  rl_Height ;                         /*   0    */
	LONG  rl_Unit ;                           /*   3    */
	LONG  rl_Left ;                           /*   0    */
	LONG  rl_Top ;                            /*   0    */
	ULONG rl_vpmid ;                          /*  0     */
	LONG  rl_wbscr_wide ;                     /*  0     */
	LONG  rl_wbscr_high ;                     /*  0     */
	LONG  rl_fontwide ;                       /*  8     */
	LONG  rl_fonthigh ;                       /*  8     */
	LONG  rl_80wide ;                         /*  640   */
	LONG  rl_24high ;                         /*  0     */
	LONG  rl_48high ;                         /*  0     */
	LONG  rl_rows ;                           /*  24    */
	LONG  rl_cols ;                           /*  80   */ 
	LONG  rl_borderwidth ;                    /*   0    */
	LONG  rl_bordertop ;                      /*   0    */
	LONG  rl_borderbottom ;                   /*   0    */
	LONG  rl_resize ;                         /*   1    */
	LONG  rl_sockets_are_setup ;              /*   0    */
	int   rl_socket ;                         /*   0    */
	VOID *rl_glob_ptr ;                       /*   0    */
	struct RDargs   *rl_RDargs ;              /*   0    */
	struct Screen   *rl_Screen ;              /*   0    */
	struct Window   *rl_Window ;              /*   0    */
	struct TextFont *rl_TextFont ;            /*   0    */
	struct TextAttr *rl_TextAttr ;            /*   0    */
	struct Menu     *rl_menu ;                /*   0    */
	BYTE *rl_hostptr ;                        /*   0    */
	BYTE *rl_userptr ;                        /*   0    */
	BYTE *rl_remuserptr ;                     /*   0    */
	BYTE *rl_termtypeptr ;                    /*   0    */
	BYTE rl_Host[MAXHOST+8L] ;                /*   0    */
	BYTE rl_User[MAXUSER+8L] ;                /*   0    */
	BYTE rl_RemUser[MAXUSER+8L] ;             /*   0    */
	BYTE rl_TermType[MAXTERMTYPE+8L] ;        /*   0    */
	BYTE rl_wtitle[128] ;                     /*   0    */
	} ;



#define CURSOR_ON     console_write(cursor_on,3L) 
#define CURSOR_OFF    console_write(cursor_off,4L) 
#define LINEWRAP_ON   console_write(linewrap_on,4L)
#define LINEWRAP_OFF  console_write(linewrap_off,4L)
#define CLOSEGAD_ON   console_write(closegad_on,4L) 
#define CLOSEGAD_OFF  console_write(closegad_off,4L) 
#define LINEFEED_ON   console_write(lfmode_on,4L)
#define LINEFEED_OFF  console_write(lfmode_off,4L)
#define MENU_ON       console_write(menu_on,4L) 
#define RESIZE_ON     console_write(gimme_resize,4L)

VOID   About(struct RLGlobal *g) ;
LONG   MakeClipMine(VOID) ;
struct MsgPort *CreateMsgPort(VOID) ;
VOID   DoConClipStuff(int s) ;
LONG   GetClipChar( UBYTE *thechar ) ;
VOID   KillPaste(VOID) ;
VOID   HandleMenu( struct RLGlobal *g, UBYTE *data ) ;
VOID   handle_oob( struct RLGlobal *g ) ;
VOID   send_window_size( struct RLGlobal *g ) ;
VOID   FlashBar( VOID ) ;
LONG   AskExit( struct RLGlobal *g ) ;
VOID   Getout( BYTE *msg, struct RLGlobal *g ) ;
VOID   flush_buffered_blocks( UBYTE *buffer, REGISTER ULONG charcount) ;
VOID   doscroll(REGISTER ULONG lines) ;
LONG   console_write(UBYTE *buf, LONG len) ; 
ULONG  countlfs(REGISTER UBYTE *b, REGISTER ULONG cnt, REGISTER ULONG wide ) ;
VOID   AlterMenu( APTR userdata, UBYTE type, UWORD flags_on, UWORD flags_off ) ;

#define NO_TYPE (UBYTE)127


   /********************************************/
   /*  defs for main.c specifically            */
   /********************************************/

#ifdef MAIN /* stuff for rlogin.c */


   /* ------------  conclip stuff */  /* ----------- */

#define CLIPBUFSIZE 256

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

#define	MAXADDRSIZE	14
BYTE ghe_hostaddr[MAXADDRSIZE] ; /* the fix ! */

#define CSI               0x9b
#define CON_IDCMP_REPORT  0x7c
#define CON_OPTION_ON     0x68  /* lower case 'h' */
#define CON_OPTION_OFF    0x6c  /* lower case 'l' */
#define CON_SPECIAL_KEY   0x7e  /* tilde */
#define TILDE             0x7e  /* tilde */
#define CON_CURPOS_REPORT 
#define INBUFFSIZE 256
#define	MAX_ALIASES	35
#define MAXSOCKS 5L


UBYTE menu_on[]         = { '\x9b', '1','0','{','\0' } ;
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

 /* Messages sent via out-of-band data. */
#define TIOCPKT_FLUSHREAD    0x01    /* flush pending kybd data    */
#define TIOCPKT_FLUSHWRITE   0x02    /* flush all data to mark     */
#define TIOCPKT_STOP         0x04
#define TIOCPKT_START        0x08
#define TIOCPKT_NOSTOP       0x10
#define TIOCPKT_DOSTOP       0x20
#define TIOCPKT_WINDOW       0x80    /* request window size        */

#define ESC_SEQ_TERMINATOR(x) (((x) >= 0x40) && ((x) <= 0x7e)) 

#define CONCLIP_TERM    0x76  /* 'v'  */

    /* if using a sole '9b' as the escape then these values */
    /* need to be reduced by one.                           */
    /*                        |                 |           */
    /*                        V                 V           */
#define CON_CLOSEGAD(b) (((b)[2] == 0x31 && (b)[3] == 0x31))
#define CON_RESIZE(b)   (((b)[2] == 0x31 && (b)[3] == 0x32))
#define CON_GOTMENU(b)  (((b)[2] == 0x31 && (b)[3] == 0x30))
#define CONCLIP_PENDING(b) (((b)[2] == 0x30 && (b)[3] == 0x20 && (b)[4] == 0x76))
    /*                           |                 |                 |   */
    /*                           |                 |                 |   */
    /* ...and these need to be changed, too. ______/________________/    */
    


#endif /* MAIN */

   /********************************************/
   /*  defs for console.c specifically         */
   /********************************************/

#ifdef CONSOLE_C

#define SIZEGADHEIGHT (10 + 1)

ULONG GetDisplayDims( struct Screen *scrn, struct IBox *box ) ;

		/* NewWindow used for wbscreen rlogin */
STATIC struct ExtNewWindow nw = {
	0, 0, 640, 200, -1, -1, 
	0L,                                            /* IDCMP */
	WINDOWDRAG|WINDOWDEPTH|ACTIVATE|WINDOWCLOSE|WFLG_NW_EXTENDED,   /* flags */
	0, 0, NULL, 0, 0, 640, 100, -1, -1, 
	WBENCHSCREEN,
	NULL
	};

		/* this is the screen used if screen mode is selected */
STATIC struct ExtNewScreen scrdef = { 0,0,0,0,
	2,	/* bit planes */
	0,1,	/* pens */
	0,
	CUSTOMSCREEN | NS_EXTENDED,
	NULL,NULL,NULL,NULL
	};


		/* NewWindow used for custom screen rlogin */
STATIC struct ExtNewWindow bbwin = { 0,0,0,0,
	1,0,
	NULL,
/*	ACTIVATE | NOCAREREFRESH | BORDERLESS | BACKDROP | WFLG_NW_EXTENDED, */
	ACTIVATE | BORDERLESS | BACKDROP | WFLG_NW_EXTENDED,
	NULL,		/* pointer to first gadget */
	NULL, NULL, NULL, NULL, 0,0,-1,-1,
	CUSTOMSCREEN,
	NULL
	};

struct TagItem windowtags[] = {
	{ WA_NewLookMenus, TRUE },
	{ TAG_END,0L }
	} ;

struct TagItem screentags[] = {
	{ 0L, 0L },
	{ SA_Interleaved,TRUE} ,
	{ TAG_END,0L }
	} ; 
	

#endif /* CONSOLE_C */

/* end of file */