/* -----------------------------------------------------------------------
 * rl.h    include file for rlogin 38 rewrite
 *
 * $Locker:  $
 *
 * $Id: rl.h,v 38.0 93/04/13 13:15:42 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	rl.h,v $
 * Revision 38.0  93/04/13  13:15:42  bj
 * Include file for v38 rlogin
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/rl.h,v 38.0 93/04/13 13:15:42 bj Exp $
 *
 *------------------------------------------------------------------------
 */

	/* includes */
	
#include <ss/socket.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <signal.h>

#include<exec/types.h>
#include<exec/execbase.h>
#include <exec/memory.h>
#include<intuition/intuition.h>
//#include<intuition/intuitionbase.h>
#include<intuition/screens.h>
#include<graphics/modeid.h>
#include<utility/tagitem.h>
#include<graphics/gfxbase.h>
#include<devices/conunit.h>
#include<libraries/gadtools.h>
#include<string.h>
#include<stdlib.h>
#include<dos.h>
#include<dos/rdargs.h>

#include<clib/intuition_protos.h>
#include<clib/graphics_protos.h>
#include<clib/exec_protos.h>
#include<clib/dos_protos.h>
#include<clib/utility_protos.h>
#include<clib/gadtools_protos.h>

#include<pragmas/intuition_pragmas.h>
#include<pragmas/graphics_pragmas.h>
#include<pragmas/exec_pragmas.h>
#include<pragmas/dos_pragmas.h>
#include<pragmas/utility_pragmas.h>
#include<pragmas/gadtools_pragmas.h>

/* EasyStruct stuff */

   	struct ReqArgs {
   		LONG arg1 ;
   		LONG arg2 ;
   		LONG arg3 ;
   		LONG arg4 ;
   		LONG arg5 ;
   		} ;

#define TITLE  "Rlogin"
#define NAME "RLOGIN"
#define MAX_TITLE 40L

#define MAX_USERMODEID         128L
#define MAX_FONTNAMELEN         64L
#define MAX_BUFFER_SIZE        256L 
#define MAX_COLORS               8L 
#define MAX_HOST               128L
#define MAX_USER                64L
#define MAX_DATE                32L
#define MAX_TERMTYPE           128L
#define SCROLLBLOCK_COUNT       10L
#define SCROLLBLOCK_SIZE       256L 
#define SCROLLBUFFER_SIZE      (SCROLLBLOCK_COUNT * SCROLLBLOCK_SIZE)

#define islower(c)     ((c >= 'a' && c <= 'z') ? 1 : 0)
#define toupper(c)     (islower(c)?((c)-('a'-'A')):(c))
#define my_ishex(c)    ((((c >= 'A') && (c <= 'F')) || ((c >= '0') && (c <= '9'))) ? 1 : 0)

struct WBDisplay {
	ULONG cd_modeid ;
	ULONG cd_Depth ;
	ULONG cd_Height ;
	ULONG cd_Width ;
	ULONG cd_fontheight ;
	ULONG cd_fontwidth ;
	ULONG cd_left_border_width ;
	ULONG cd_right_border_width ;
	ULONG cd_top_border_height ;
	ULONG cd_bottom_border_height ;
	ULONG cd_titlebar_height ;
	ULONG cd_vert_borders_total ;
	ULONG cd_horiz_borders_total ;
	struct TextAttr *cd_Font ;
	} ;


struct glob {
	struct DosLibrary    *g_DOSBase ;
	struct GfxBase       *g_GfxBase ;
	struct IntuitionBase *g_IntuitionBase ;
	struct Library       *g_UtilityBase ;
	struct Library       *g_SockBase ;
	struct Library       *g_GadToolsBase ;
	struct Screen        *g_Screen ;        /* the screen (NULL == WB)     */
	struct Window        *g_Window ;        /* the window                  */
	struct WBDisplay      g_WBDisplay ;     /* info re: WB display         */
	struct MsgPort       *g_ConReadPort ;   
	struct MsgPort       *g_ConWritePort ;        
	struct IOStdReq      *g_ConReadReq ;
	struct IOStdReq      *g_ConWriteReq ;
	struct ConUnit       *g_ConUnit ;
	struct Task          *g_myself ;
	struct Menu          *g_Menu ;          /* ptr to menu strip           */
	APTR   g_visualinfo ;                    /* visualinfo ptr              */
	ULONG  g_modeid ;                        /* returned by GetVPModeID()   */
	ULONG  g_depth ;                         /* depth of cust scr (max 3)   */
	ULONG  g_fontheight ;                    /* window's font height        */
	ULONG  g_fontwidth ;                     /* window's font width         */
	ULONG  g_user_width ;                    /* users desired width         */
	ULONG  g_user_height ;                   /* users desired height        */
	ULONG  g_user_left ;                     /* left win pos on wb scrn     */
	ULONG  g_user_top ;                      /* top  win pos on wb scrn     */
	ULONG  g_user_depth ;                    /* users desired depth         */
	ULONG  g_user_conunit ;                  /* console unit user wants     */
	BOOL   g_use_screen ;                    /* true = custom screen        */
	BOOL   g_use_colors ;                    /* true = user's own colors    */
	BOOL   g_console_is_open ;               /* true if console dev open    */
	BOOL   g_menu_is_attached ;              /* true = menu & wind attached */
	BOOL   g_getout ;                        /* true = we are quitting.     */
	ULONG  g_netevent ;                      /* signal bit for network      */
	ULONG  g_urgevent ;                      /* signal bit for network      */
	ULONG  g_conread_was_used ;
	ULONG  g_conwrite_was_used ;
	UBYTE  g_user_configfile[256] ;          /* the user's config file (-f) */
	UBYTE  g_user_argstring[256] ;           /* the user's complete cmd	line*/
	UBYTE  g_user_modeid[MAX_USERMODEID+4] ; /* users mode request (if any) */
	UBYTE  g_buffer[256] ;                   /* debugging buffer            */
	UBYTE  g_netbuffer[MAX_BUFFER_SIZE] ;    /* incoming from net goes here */
	struct ColorSpec g_colors[MAX_COLORS] ;  /* colors                      */
	           /* rlogin specific stuff comes after this line */
	ULONG  g_user_resize ;                   /* size gad? def: wb=1 cust=0  */
	ULONG  g_user_jump ;                     /* jump scroll on/off          */
	ULONG  g_user_linewrap ;                 /* line wrap on/off            */
	ULONG  g_80wide ;                        /* #pixels across 24rowx80col  */
	ULONG  g_24high ;                        /* windo. Borders NOT included */
	ULONG  g_sockets_are_setup ;             /* BOOL                        */
	int    g_socket ;                        /* the main socket             */
	int    g_errno ;                         /* global errno for socket.lib */
	LONG   g_error ;                         /* ultimate return value       */
	ULONG  g_paste_delay ;                   /* paste delay for conclip     */
	UBYTE  g_host[MAX_HOST+4] ;              /* hostname                    */
	UBYTE  g_user[MAX_USER + 4] ;            /* user name                   */
	UBYTE  g_remote_user[MAX_USER+4] ;       /* remote user name            */
	UBYTE  g_termtype[MAX_TERMTYPE+4] ;      /* termtype. def rlamiga/9600  */
	UBYTE  g_window_title[MAX_TITLE+4] ;     /* title for window            */
	BOOL   g_minsize ;                       /* true is user wants 24x80    */
	ULONG  g_minhigh ;                       /* min wide/high of 24x80 ...  */
	ULONG  g_minwide ;                       /* ... _includes_ borders!     */
	BYTE   g_inchar ;                        /* char in from console device */
	UBYTE  g_clipsig ;                       /* signal bit for conclip      */
	ULONG  g_clipevent ;                     /* mask for conclip event      */
	ULONG  g_pcount ;                        /* # of chars read frm clip    */
	BOOL   g_clip_in_progress ;              /* true = we are pasting       */
	struct MinNode *g_pastenode ;            /* conclip support             */
	struct MinList g_paste ;                 /* conclip support             */
	UBYTE  g_verstag[64] ;
	char   g_date[MAX_DATE+4L] ;
	BYTE   g_scroll_buffer[SCROLLBUFFER_SIZE] ;
	} ;


    /* prototypes for rl's functions */

int    rl(void) ;
VOID   mainloop(struct glob *g) ;
BOOL   GetScreen(struct glob *g ) ;
BOOL   GetWindow(struct glob *g ) ;
BOOL   GetWBDisplay( struct glob *g ) ;
ULONG  GetUserModeID(struct glob *g) ;
STRPTR mysprintf(UBYTE *, UBYTE *, ...) ;
BOOL   SetPen( UWORD index, UBYTE *rgb, struct glob *g ) ;
BOOL   SetUpConsole( struct glob *g) ;
VOID   KillConsole( struct glob *g ) ;
VOID   con_write(unsigned char *buf, ULONG len, struct glob *g) ;
BYTE   console_getchar( struct glob *g) ;
BOOL   GetUserPrefs(struct glob *g) ;
BOOL   GetConfig(struct glob *g) ;
LONG   AskExit(struct glob *g) ;
struct glob *InitGlob(VOID) ;
BOOL   InitSocketStuff(struct glob *g) ;
VOID   KillSocketStuff(struct glob *g) ;
BOOL   console_char_ready( struct glob *g ) ;
VOID   handle_oob( struct glob *g ) ;
VOID   send_window_size( struct glob *g ) ;
VOID   get_window_sizes(int *width, int *height, int *cols, int *rows,struct glob *g) ;
VOID   About(struct glob *g) ;
LONG   AskExit(struct glob *g) ;
BOOL   SetUpClipStuff(struct glob *g) ;
LONG   MakeClipMine(struct glob *g) ;
LONG   GetClipChar( UBYTE *thechar, struct glob *g ) ;
VOID   KillPaste(struct glob *g) ;
VOID   KillClipStuff(struct glob *g) ;
VOID   KillLibs(struct glob *g) ;
BOOL   GetLibs(struct glob *g) ;
BOOL   SetUpMenus(struct glob *g) ;
VOID   KillMenus(struct glob *g) ;
VOID   HandleMenu( struct glob *g, UBYTE *data ) ;
VOID   flush_scroll_buffer( BYTE *, ULONG, struct glob *g) ;
ULONG  countlfs( UBYTE *b, ULONG cnt, ULONG wide, struct glob *g ) ;
VOID   doscroll(ULONG lines, struct glob *g) ;
ULONG  mod( ULONG dividend, ULONG divisor, struct Library *UtilityBase) ;
BOOL   RAParse( UBYTE *args, char *template, struct glob *g) ;
BOOL   ReadConfigFile( STRPTR array, struct glob *g) ;
BOOL   FindFileName( struct glob *g) ;
VOID   NewList(struct MinList *) ;
	/* display modes */

struct display_mode {
	UBYTE *dm_option ;    /* options that the user can select */
	ULONG  dm_mode ;      /* corresonding mode id             */
	} ;

	/* readargs stuff */
	

#define TEMPLATE "HOST/A,-L=USER/K,-F=FILE/K,-X=LEFT/K/N,-Y=TOP/K/N,-W=WIDTH/K/N,-H=HEIGHT/K/N,-S=SCREEN/S,-T=TERMTYPE/K,-M=MODE/K,-D=DEPTH/K/N,-U=CONUNIT/K/N,-NR=NORESIZE/S,-NJ=NOJUMP/S,-NW=NOWRAP/S,-P=PASTEDELAY/K/N,-P0=PEN0/K,-P1=PEN1/K,-P2=PEN2/K,-P3=PEN3/K,-P4=PEN4/K,-P5=PEN5/K,-P6=PEN6/K,-P7=PEN7/K,-MS=MINSIZE/S"
#define OPT_HOST        0
#define OPT_USER        1 
#define OPT_FILE        2
#define OPT_LEFT        3 
#define OPT_TOP         4 
#define OPT_WIDE        5 
#define OPT_HIGH        6 
#define OPT_SCREEN      7 
#define OPT_TERMTYPE    8 
#define OPT_MODE        9 
#define OPT_DEPTH       10
#define OPT_CONUNIT     11
#define OPT_NORESIZE    12
#define OPT_NOJUMP      13
#define OPT_NOWRAP      14
#define OPT_PASTE       15
#define OPT_PEN0        16
#define OPT_PEN1        17
#define OPT_PEN2        18
#define OPT_PEN3        19
#define OPT_PEN4        20
#define OPT_PEN5        21
#define OPT_PEN6        22
#define OPT_PEN7        23
	#define OPT_PENMAX OPT_PEN7 + 1
#define OPT_MINSIZE     24
#define OPT_COUNT       25

	/* console sequences */

#define CURSOR_ON     con_write(cursor_on,3L,g) 
#define CURSOR_OFF    con_write(cursor_off,4L,g) 
#define LINEWRAP_ON   con_write(linewrap_on,4L,g)
#define LINEWRAP_OFF  con_write(linewrap_off,4L,g)
#define CLOSEGAD_ON   con_write(closegad_on,4L,g) 
#define CLOSEGAD_OFF  con_write(closegad_off,4L,g) 
#define LINEFEED_ON   con_write(lfmode_on,4L,g)
#define LINEFEED_OFF  con_write(lfmode_off,4L,g)
#define MENU_ON       con_write(menu_on,4L,g) 
#define RESIZE_ON     con_write(gimme_resize,4L,g)


#define CON_IDCMP_REPORT  0x7c
#define MAXSOCKS           5L

#define ESC_SEQ_TERMINATOR(x) (((x) >= 0x40) && ((x) <= 0x7e)) 

#define CONCLIP_TERM    0x76  /* 'v'  */

	/* this program converts the console escape inducer from */
	/* hex '9b' to  '1b + [' as the buffer is filled.        */
    /* if using a sole '9b' as the escape then these values  */
    /* need to be reduced by one.  The first value would be  */
    /* a '1' in the case of using '9b', a '2' if 'esc-9b'    */
    /*                           |                 |         */
    /*                           V                 V         */
#define CON_CLOSEGAD(b)    (((b)[2] == 0x31 && (b)[3] == 0x31))
#define CON_RESIZE(b)      (((b)[2] == 0x31 && (b)[3] == 0x32))
#define CON_GOTMENU(b)     (((b)[2] == 0x31 && (b)[3] == 0x30))
#define CONCLIP_PENDING(b) (((b)[2] == 0x30 && (b)[3] == 0x20 && (b)[4] == 0x76))
    /*                           |                 |                 |   */
    /*                           |                 |                 |   */
    /* ...and these need to be changed, too. ______/________________/    */
    

 /* Messages sent via out-of-band data. */
#define TIOCPKT_FLUSHREAD    0x01    /* flush pending kybd data    */
#define TIOCPKT_FLUSHWRITE   0x02    /* flush all data to mark     */
#define TIOCPKT_STOP         0x04
#define TIOCPKT_START        0x08
#define TIOCPKT_NOSTOP       0x10
#define TIOCPKT_DOSTOP       0x20
#define TIOCPKT_WINDOW       0x80    /* request window size        */


/* library base setup macros */

#define SYSBASE          struct Library *SysBase = (*((struct Library **) 4))
#define SOCKBASE(x)      struct Library *SockBase = (x)->g_SockBase
#define DOSBASE(x)       struct DosLibrary *DOSBase = (x)->g_DOSBase
#define INTUITIONBASE(x) struct IntuitionBase *IntuitionBase = (x)->g_IntuitionBase
#define UTILITYBASE(x)   struct Library *UtilityBase = (x)->g_UtilityBase
#define GFXBASE(x)       struct GfxBase *GfxBase = (x)->g_GfxBase
#define GADTOOLSBASE(x)  struct Library *GadToolsBase = (x)->g_GadToolsBase

/* clip stuff */

#define CLIPBUFSIZE 256

struct CHRSChunk {
    struct MinNode mn;
    BYTE  *text;         /* pointer to start of text */
    BYTE  *freeme;       /* if non-zero, call FreeVec() with this pointer */
    ULONG  size;         /* total # of characters in this block */
    ULONG  flags;        /* may end up needing this at some time */
    ULONG  UserLink;
    BYTE   data[1];      /* really 'n' # of bytes */
};

struct CmdMsg {
    struct Message cm_Msg;
    LONG   cm_type;      /* type of message, and return validation */
    LONG   cm_error;     /* error code return */
    struct MinList cm_Chunks;
};

/* menu IDs */


#define ABOUT   300L
#define QUIT    301L
                    
#define BASEID  200L // +++++++++++++++++++++++++++++++++++++++++++++   
#define PT00    200L // these values (2xx) are used in HandleMenus()   
#define PT10    210L // and need to remain as is.  These values        
#define PT20    220L // are used to calculate the actual paste         
#define PT30    230L // delay time AS WELL AS being the menus          
#define PT40    240L // ID number.  DO NOT CHANGE WITHOUT THOUGHT!!    
#define PT50    250L // +++++++++++++++++++++++++++++++++++++++++++++  

#define RESET   302L
#define LWRAP   303L
#define JSC     304L
#define MINSIZE 305L
#define WIDE80  306L

/* error values for each function. These become the ultimate return */
/* value for the program                                            */

#define ERR_INITGLOB     200L
#define ERR_GETLIBS      201L
#define ERR_GETPREFS     202L
#define ERR_GETWBDISPLAY 203L
#define ERR_GETSCREEN    204L
#define ERR_GETWINDOW    205L
#define ERR_GETCONSOLE   206L
#define ERR_INITSOCKET   207L
#define ERR_SETUPCLIP    208L
#define ERR_SETUPMENUS   209L

/* end of file */

