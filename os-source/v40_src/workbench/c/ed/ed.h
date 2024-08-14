#include "internal/commands.h"
#include "intuition/intuition.h"
#include "libraries/gadtools.h"
#include "clib/gadtools_protos.h"
#include "libraries/asl.h"
#include "clib/asl_protos.h"
#include "clib/intuition_protos.h"
#include "pragmas/asl_pragmas.h"
#include "pragmas/gadtools_pragmas.h"

extern __stdargs kprintf(char *, ...);

#include <setjmp.h>

#define U_ARGS(a) a

#define CONSOLE_IS_FIXED   1
typedef jmp_buf *LEVEL;
#define LEVELBUF jmp_buf

#define ENDSTREAMCH -1

/***
*
* Default configurations
*
***/
#define BACKUPNAME      ":t/ed-backup"
#define LINELENGTH      256     /* size to allocate to hold a line      */
#define MAX_LINE        255     /* Maximum line length                  */
#define MAXMENU         100     /* Maxumum number of user menus         */
#define SAFETYAREA      40      /* Overrun margin for safety            */
#define MAXLINES        65      /* MAXIMUM number of lines allowed      */
#define DEFAULT_SIZE    60000   /* Default buffer editing size          */
#define MINSIZE         20000   /* Smallest buffer allowed              */
#define DEFAULT_TABS    3       /* Default tab stops                    */
#ifdef CONSOLE_IS_FIXED
#define WINDOWNAME      "RAW:0/0/639/199/Ed 2.00/CLOSE"
#else
#define WINDOWNAME      "RAW:0/0/639/199/Ed 2.00/SMART/CLOSE"
#endif
#define VUD_SIZE        200     /* Size of buffered terminal output     */
#define KEY_LEV         0x4000  /* Magic level for keys                 */
#define PFKEY_LEFT      20
#define PFKEY_RIGHT     21
#define PFKEY_UP        22
#define PFKEY_DOWN      23
#define PFKEY_DEL       24
#define PFKEY_CTRL2     25
#define PFKEY_CTRLA     26
#define PFKEY_CTRLB     27
#define PFKEY_CTRLC     28
#define PFKEY_CTRLD     29
#define PFKEY_CTRLE     30
#define PFKEY_CTRLF     31
#define PFKEY_CTRLG     32
#define PFKEY_CTRLH     33
#define PFKEY_CTRLI     34
#define PFKEY_CTRLJ     35
#define PFKEY_CTRLK     36
#define PFKEY_CTRLL     37
#define PFKEY_CTRLM     38
#define PFKEY_CTRLN     39
#define PFKEY_CTRLO     40
#define PFKEY_CTRLP     41
#define PFKEY_CTRLQ     42
#define PFKEY_CTRLR     43
#define PFKEY_CTRLS     44
#define PFKEY_CTRLT     45
#define PFKEY_CTRLU     46
#define PFKEY_CTRLV     47
#define PFKEY_CTRLW     48
#define PFKEY_CTRLX     49
#define PFKEY_CTRLY     50
#define PFKEY_CTRLZ     51
#define PFKEY_ESC       52
#define PFKEY_CTRLSLASH 53
#define PFKEY_CTRLRBRAK 54
#define PFKEY_CTRLCARET 55
#define PFKEY_CTRLMINUS 56
#define MAX_PFKEY       57      /* Maximum PF key available             */

#define HOST_PORT_NAME  "Ed"    /* Default Rexx Port name               */
#define ALT_PORT_NAME   "Ed_1"  /* Alternate Rexx Port name             */
#define REXX_EXTENSION  "ed"    /* Default extension for Rexx macros    */

/***
*
* Line store layout
*
***/
struct LINE
{
   struct LINE *prev;
   char data[1];
};

struct CMDLEVEL
{
   struct CMDLEVEL *link;
   LEVEL            save_level;
   int              save_ptr;
   char            *save_buf;
   struct RexxMsg  *rxmsg;
   char             buf[LINELENGTH];
};

/***
*
* Line storage Manipulation
*   NEXTLINE() is used to advance to the next line in storage.
*   LINELEN()  calculates how much storage a line of a given length takes
*
***/
#define NEXTLINE(l) ((struct LINE *)(char *)&l->data[(l->data[0]+4)&~3])
#define LINELEN(l)  (4+((l+4) & ~3))

/***
*
* Program termination function
*
***/
void XCEXIT U_ARGS((int));
#define EXIT(x)     XCEXIT(x)

/***
*
* VDU functions
*
***/
#define OP_LEFT        'D'
#define OP_RIGHT       'C'
#define OP_UP          'A'
#define OP_DOWN        'B'
#define OP_SETCURSOR   'H'
#define OP_SCROLLUP    'S'
#define OP_SCROLLDOWN  'T'
#define OP_INSERTLINE  'L'
#define OP_DELETELINE  'M'
#define OP_INSERTCHAR  '@'
#define OP_DELETECHAR  'P'
#define OP_DEOL        'K'

/***
*
* Screen management routines
*
***/
#define VDU_INIT()
#define VDU_UNINIT()
#define VDU_LENGTH()            make_window_enquiry(FALSE)
#define VDU_WIDTH()             make_window_enquiry(TRUE)
#define VDU_GETCHAR(f)          vud_rdch(f)
#define VDU_SCROLLUP(n)         screenopv(OP_SCROLLUP,n)
#define VDU_SCROLLDOWN(n)       screenopv(OP_SCROLLDOWN,n)
#define VDU_SETCURSOR(x,y)      vdu_setcursor(x,y)
#define VDU_INSL()              screenops(OP_INSERTLINE)
#define VDU_DELL()              screenops(OP_DELETELINE)
#define VDU_HIGHLIGHTON()       vud_wrline("\x1b[33m", 5)
#define VDU_HIGHLIGHTOFF()      vud_wrline("\x1b[31m", 5)
#define VDU_INSCH()             screenops(OP_INSERTCHAR)
#define VDU_DELCH()             screenops(OP_DELETECHAR)
#define VDU_DEOL()              screenops(OP_DEOL)
#define VDU_UP()                screenops(OP_UP)
#define VDU_DOWN()              screenops(OP_DOWN)
#define VDU_RIGHT()             screenops(OP_RIGHT)
#define VDU_LEFT()              screenops(OP_LEFT)
#define VDU_CLEAR()             vud_wrch('\f');

/***
*
* Characters returned by vud_rdch
*
***/
#define C_NOTREADY  -1
#define C_AREXX     -2
#define C_RUBOUT    0x7F
#define C_ESC       ('['-'@')
#define C_CR        ('M'-'@')
#define C_EXECUTE   ('G'-'@')  /* ctrl g          */
#define C_LEFT      0x08
#define C_RIGHT     0x18
#define C_UP        0x0B
#define C_DOWN      0x0A

/***
*
* Error message codes
*
***/
#define ERROR_DELLAST       1
#define ERROR_NOROOM        2
#define ERROR_NEWFILE       3
#define ERROR_LINESTR       4
#define ERROR_TOPFILE       5
#define ERROR_ENDFILE       6
#define ERROR_TOOLONG       7
#define ERROR_UNKNOWN       8
#define ERROR_BRACKET       9
#define ERROR_ABANDON       10
#define ERROR_SYNTAX        11
#define ERROR_BADFILE       12
#define ERROR_NUMBER        13
#define ERROR_NOTSET        14
#define ERROR_INVALID       15
#define ERROR_BLKERR        16
#define ERROR_NOTFOUND      17
#define ERROR_TABSEXPANDED  18
#define ERROR_FILEHASBINARY 19
#define ERROR_NO_REXX       20
#define ERROR_NO_MEMORY     21
#define ERROR_STRING        22

/***
*
* Return codes from Readfile subroutine.
* Note that ordering is significant
*
***/
#define RDF_OK          0
#define RDF_TABS        1
#define RDF_TRUNC       2
#define RDF_FATAL       3 /* Above here are fatal */
#define RDF_BIN         3
#define RDF_TOOBIG      4

extern struct WBStartup *WBenchMsg;

/*----------------------------------------------------------------------*/
/* GLOBAL DATA                                                          */
/*----------------------------------------------------------------------*/
extern struct LINE *line_first; /* Start of first line in buffer        */
extern struct LINE *line_last;  /* Start of last line in buffer         */
extern struct LINE *line_ptr[MAXLINES];
                                /* line pointers to lines on screen     */
extern char line_mods[MAXLINES];/* Vector of modified lines to display  */
extern char current_line[LINELENGTH];
                                /* Current line buffer                  */
extern int current_size;        /* Length of current line               */
extern struct LINE *prev_list;  /* Start of backwards list of lines     */
extern int line_num;            /* Highest index of line on screen      */
extern int line_max;            /* max usable lines on screen           */
extern int screen_width;        /* highest char pos on a line           */
extern char *line_lastword;     /* last word held in store              */
/* Cursor locations     */
extern int phys_x, phys_y;      /* actual positions                     */
extern int log_x, log_y;        /* where user thinks cursor is          */
extern char *worktop;
extern char file_name[LINELENGTH];
extern BOOL extracted;          /* TRUE if current line extracted from store    */
extern int window_base;         /* Offset of screen window              */
extern char *cmdline;           /* Extended command line store          */
extern char cmdbuf[LINELENGTH];
extern char lastcmd[LINELENGTH];           /* Last executed command                */
extern int cmdptr;              /* Ptr within it                        */
extern int tab_stop;            /* Current tab setting                  */
extern struct LINE *block_start;/* Pointer to block start               */
extern struct LINE *block_end;  /* Pointer to block end                 */
extern char status_msg[LINELENGTH];
                                /* Generic status line message          */
extern int msg_num;             /* Message number for pending error     */
extern int left_margin;         /* Left margin                          */
extern int right_margin;        /* Right margin                         */
extern int term_width;          /* User specified terminal width        */
extern int term_height;         /* User specified terminal height       */
/* Error handling       */
extern LEVEL err_level;         /* Level for LONGJMP                    */
#define ERRLABEL 1              /* Label for LONGJMP                    */
/* Space for arguments  */
/* Note that the next two are BSTRs with a null terminator.             */
extern char search_vec[LINELENGTH];        /* Space for search string              */
extern char insert_vec[LINELENGTH];        /* Space for insert string              */
extern BOOL extend_margin;      /* TRUE if margin extended              */
/* TRIPOS additions     */
extern BOOL data_changed;       /* Flag to indicate file has changed    */
extern BPTR console_stream;     /* Console window for editing           */
extern struct StandardPacket *read_pkt;
extern struct MsgPort *reply_port;
extern BOOL forcecase;          /* Flag to make search case insensitive */
extern char vud_buf[VUD_SIZE];  /* Buffer for terminal output           */
extern int vud_bpos;            /* Position within that buffer          */
extern struct DosLibrary *DOSBase;
extern struct Window *window;   /* Window for current editing           */
extern char *keydef[MAX_PFKEY]; /* PFKey definitions.                   */
extern struct MsgPort *rexx_port;
extern struct CMDLEVEL *pending;/* Pending command levels               */
extern int outstanding_rexx_commands;
extern struct FileInfoBlock *fib;
extern struct Library *AslBase;
extern struct Library *GadToolsBase;
extern struct IntuitionBase *IntuitionBase;
extern void *vi;                /* Visual info for gadtools             */
extern int menuattached;        /* Flag to indicate attached menu       */
extern struct Menu *menu;       /* Attached menu structure              */
extern struct NewMenu newmenu[MAXMENU];
extern int requestflag;         /* Allow the file requester to be used  */
extern struct FileRequester *FileRequester;
extern char portname_buf[10];   
extern struct RxsLib *RexxSysBase;     /* this is the rexx library base */
extern struct Library *SysBase;        /* Provided for in the startup code */
extern struct Library *IconBase;
extern struct DiskObject *diskobj;

/*--------------------------------------*/
/*-------- Prototypes for Ed1.c --------*/
/*--------------------------------------*/
void  shuffle_up        U_ARGS((int             /* n            */
                              ));


void  get_next          U_ARGS((struct LINE *   /* p            */
                              ));

void  store_prev        U_ARGS((void));


void  shuffle_down      U_ARGS((int             /* n            */
                              ));

BOOL  next_line         U_ARGS((void));

BOOL  prev_line         U_ARGS((void));

void  empty_buffer      U_ARGS((void));

void  delete_line       U_ARGS((int             /*              */
                              ));

void  delete_and_shuffle U_ARGS((int            /* n            */,
                                struct LINE *   /* p            */,
                                int             /* l            */
                              ));
void  adjust_pointers   U_ARGS((int             /* n            */,
                                int             /* m            */,
                                long            /* a            */
                              ));
void  insert_line       U_ARGS((int             /* n            */
                              ));

void  extract_current   U_ARGS((int             /* n            */
                              ));

void  replace_current   U_ARGS((int             /* n            */
                              ));

                        /* Mark line as changed   */
void  flag_line         U_ARGS((int             /*              */
                              ));

/*--------------------------------------*/
/*-------- Prototypes for Ed2.c --------*/
/*--------------------------------------*/
void  edit              U_ARGS((void));

BOOL  window_event      U_ARGS((int             /* ch           */
                              ));

int   parse_ansi        U_ARGS((int             /* ch           */,
                                long []         /* args         */
                              ));

void  resize_window     U_ARGS((void));

                        /* Replace current line                 */
void  do_replace        U_ARGS((void));

                        /* Extract current line                 */
void  do_extract        U_ARGS((void));

void  do_verify         U_ARGS((void));

void  do_invertcase     U_ARGS((void));

                        /* Move cursor to end(s) of lines       */
void  cursor_eol        U_ARGS((void));

void  cursor_eop        U_ARGS((void));

                        /* Set horizontal scroll window         */
void  set_screen_window U_ARGS((int             /* col          */
                              ));

/*--------------------------------------*/
/*-------- Prototypes for Ed3.c --------*/
/*--------------------------------------*/
void  copy_text_up      U_ARGS((struct LINE *   /* p            */,
                                long            /* n            */
                              ));

void  copy_text_down    U_ARGS((struct LINE *   /* p            */,
                                long            /* n            */
                              ));

                        /* Checks if sufficient free store in buffer    */
BOOL  room_for          U_ARGS((long            /* n            */
                              ));

void  move_cursor       U_ARGS((int             /* x            */,
                                int             /* y            */
                              ));

                        /* Delete line n                        */
void  vud_delete_line   U_ARGS((int             /* n            */
                              ));

                        /* Insert before line n                 */
void  vud_insert_line   U_ARGS((int             /* n            */
                              ));

int   display           U_ARGS((BOOL            /* wait         */
                              ));

int   display_line      U_ARGS((int             /* i            */
                              ));

                        /* give message to user   */
void  msg               U_ARGS((int             /*              */
                              ));

void  display_msg       U_ARGS((void));

char *get_msg           U_ARGS((void));

/*--------------------------------------*/
/*-------- Prototypes for Ed4.c --------*/
/*--------------------------------------*/
long  get_size          U_ARGS((char *          /* name         */
                              ));

int   gettoolval        U_ARGS((char *          /* name         */,
                                char **         /* tt           */,
                                int             /* dflt         */
                               ));

                        /* Initialisation       */
void  init_ed           U_ARGS((void));

BOOL  readfile          U_ARGS((BPTR            /* stream       */,
                                struct LINE *   /* base         */,
                                BOOL            /* startup      */
                              ));

void  initpkts          U_ARGS((void));

void  init_keys         U_ARGS((void));

int   vud_rdch          U_ARGS((BOOL            /* wait         */
                              ));

void  vud_flush         U_ARGS((void));

void  vud_wrch          U_ARGS((int             /* ch           */
                              ));

                        /* Write a string to screen             */
void  vdu_writes        U_ARGS((int             /* x            */,
                                int             /* y            */,
                                char *          /* s            */
                              ));
void  vud_wrline        U_ARGS((char *          /* buf          */,
                                int             /* len          */
                              ));

void  tidyup            U_ARGS((int             /* rc           */
                              ));

                        /* give message and stop                */
void  fatal             U_ARGS((char *          /*              */
                              ));

/*--------------------------------------*/
/*-------- Prototypes for Ed5.c --------*/
/*--------------------------------------*/
int __stdargs _main     U_ARGS((void));

void __stdargs MemCleanup U_ARGS((void));

void __stdargs __fpinit U_ARGS((void));

void __stdargs __fpterm U_ARGS((void));

BOOL  cursor_left       U_ARGS((void));

void  cursor_right      U_ARGS((void));

void  cursor_up         U_ARGS((void));

void  cursor_down       U_ARGS((void));

                        /* Perform tab operation  */
void  do_tab            U_ARGS((void));

int   check_next        U_ARGS((void));

                        /* cursor to next word    */
void  cursor_nxw        U_ARGS((void));

int   check_prev        U_ARGS((void));

                        /* cursor to previous word        */
void  cursor_prw        U_ARGS((void));

void  remove_trailing_spaces    U_ARGS((void));

                        /* Perform newline        */
void  do_cr             U_ARGS((void));

                        /* Delete character at cursor           */
void  do_delch          U_ARGS((void));

                        /* Delete word    */
void  do_delw           U_ARGS((void));

                        /* Insert character                     */
void  do_ins            U_ARGS((int             /* ch           */
                              ));

                        /* Delete to end of line  */
void  do_deol           U_ARGS((void));

                        /* Move n lines previous  */
void  do_prevline       U_ARGS((int             /* n            */
                              ));

                        /* Move n lines forward   */
void  do_nextline       U_ARGS((int             /* n            */
                              ));

/*--------------------------------------*/
/*-------- Prototypes for Ed6.c --------*/
/*--------------------------------------*/
int   do_prompt         U_ARGS((char *          /* prompt       */,
                                char *          /* buf          */
                              ));
                        /* Read extended (non immediate) command*/
void  do_extended       U_ARGS((void));

void  push_command      U_ARGS((char *,         /* buf          */
                                int             /* len          */
                              ));

void  execute_series    U_ARGS((char *          /* str          */
                              ));

                        /* Execute definition for a function key*/
void  execute_pfkey     U_ARGS((int             /* key          */
                              ));

int   pushlev           U_ARGS((void));

void  poplev            U_ARGS((void));

int   lose_edits        U_ARGS((void));

                        /* Execute the commands in a given file */
int   ReadCmdFile       U_ARGS((char *          /* name         */
                              ));

                        /* Execute extended command line        */
void  execute_extended  U_ARGS((int             /* edit_level   */
                              ));

                        /* prompt the user for input            */
int   do_query          U_ARGS((char *          /* msg          */
                              ));

int   capch             U_ARGS((void));

int   capitalch         U_ARGS((int             /* c            */
                              ));

int   getn              U_ARGS((void));

int   gets              U_ARGS((char *          /* v            */,
                                int             /* flag         */
                              ));

BOOL  checkomitted      U_ARGS((void));

int   getch             U_ARGS((void));

void  replace           U_ARGS((void));

/*--------------------------------------*/
/*-------- Prototypes for Ed7.c --------*/
/*--------------------------------------*/
                        /* Write out edit buffer to a file      */
BOOL  do_writebuf       U_ARGS((struct LINE *   /* line         */,
                                struct LINE *   /* endl         */,
                                char *          /* name         */,
                                char *          /* oldfile      */
                              ));

void  mymess            U_ARGS((char *          /* s            */
                              ));

                        /* extended J command                   */
void  do_join           U_ARGS((void));

void  set_pointers      U_ARGS((struct LINE *   /* p            */
                              ));

                        /* Move to top of file                  */
void  do_tof            U_ARGS((void));

                        /* Move to bottom of file               */
int   do_bof            U_ARGS((int             /* n            */
                              ));

void  fill_previous     U_ARGS((struct LINE *   /* p            */
                              ));

                        /* extended SB command                  */
void  do_showblock      U_ARGS((void));

                        /* Check block identified               */
void  check_block       U_ARGS((void));

                        /* return ptr to current line           */
struct LINE *current    U_ARGS((void));

void  insert_block      U_ARGS((struct LINE *   /* source       */,
                                struct LINE *   /* dest         */,
                                long            /* size         */,
                                struct LINE *   /* lastptr      */
                              ));

                        /* extended IB command                  */
void  do_insblock       U_ARGS((void));

                        /* extended DB command                  */
void  do_delblock       U_ARGS((void));

void  copy_current      U_ARGS((char *          /* s            */
                              ));

                        /* extended A command                   */
void  do_append         U_ARGS((char *          /* s            */
                              ));

                        /* extended I command                   */
void  do_insert         U_ARGS((void));

                        /* extended IF command                  */
void  do_insfile        U_ARGS((void));

int   load_file         U_ARGS((char *          /* file_name    */
                              ));

int   locate_string     U_ARGS((char *          /* s            */,
                                int             /* p            */,
                                char *          /* t            */,
                                BOOL            /* forwards     */
                              ));

                        /* Locate string in text                */
void  find_string       U_ARGS((int             /* n            */
                              ));

                        /* extended E command                   */
void  do_exchange       U_ARGS((BOOL            /* query        */
                              ));

void  display_part      U_ARGS((int             /* line         */,
                                struct LINE *   /* p            */,
                                char *          /* s            */,
                                char []         /* v            */
                              ));

                        /* Display current settings             */
void  do_show           U_ARGS((void));

/*--------------------------------------*/
/*-------- Prototypes for Ed8.c --------*/
/*--------------------------------------*/
void  screenops         U_ARGS((int             /* code         */
                              ));

void  screenopv         U_ARGS((int             /* code         */,
                                int             /* val          */
                              ));

void  vdu_setcursor     U_ARGS((int             /* x            */,
                                int             /* arg2         */
                              ));

int   make_window_enquiry U_ARGS((BOOL /* width */
                              ));

int   vsprintf          U_ARGS((char *          /* buf          */,
                                char *          /* ctl          */,
                                long []         /* args         */
                              ));

/*--------------------------------------*/
/*------ Prototypes for EdRexx.c -------*/
/*--------------------------------------*/
void do_rexx_cmd        U_ARGS((struct Message */* msg          */
                              ));

struct MsgPort *initrexx U_ARGS((void));

void shut_down_rexx     U_ARGS((void));

int send_rexx_command   U_ARGS((char *,         /* buf          */
                                int             /* len          */
                              ));

void free_rexx_command  U_ARGS((struct RexxMsg */* rexxmessage  */
                              ));

void execute_command    U_ARGS((struct RexxMsg */* rexxmessage  */
                              ));

void reply_rexx_command U_ARGS((struct RexxMsg */* rexxmessage  */,
                                long            /* primary      */,
                                long            /* secondary    */,
                                char *          /* result       */
                              ));
void set_rexx_vars      U_ARGS((char *          /* stem         */
                              ));
void setnum             U_ARGS((char *,         /* stem         */
                                int,            /* len          */
                                char *,         /* suffix       */
                                int             /* val          */
                              ));
/*--------------------------------------*/
/*------ Prototypes for EdMenu.c -------*/
/*--------------------------------------*/
void init_menus         U_ARGS((void));
void enable_menu        U_ARGS((void));
void set_menu_item      U_ARGS((int,            /* num          */
                                int,            /* type         */
                                char *,         /* str          */
                                char *          /* cmd          */
                              ));
void free_menus         U_ARGS((void));

