/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 382-8265    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

#include "internal/commands.h"
#include <exec/memory.h>
#include <setjmp.h>

typedef jmp_buf *LEVEL;
#define LEVELBUF jmp_buf

#define QUITLAB 1
#define EDITLAB 1

#define CHARSIZE 2   /* Because we store each character in a word */
#define ENDSTREAMCH -1
#define U_ARGS(a) a

/**
*
*  Manifest constants
*
**/
#define SMAX    80      /* Maximum length of an input string            */
#define GMAX    10      /* Maximum number of GLOBAL statements pending  */
#define CFMAX    5      /* Maximum nesting of Command files             */
#define FMAX   120      /* Maxumum length of a file name                */
#define LMAX   120      /* Default Maximum number of lines              */
#define PMAX    40      /* Default Maximum number of lines              */

/**
*
*  Mode flags for the I/O routines
*
**/
#define S_IN  0         /* Open file for input                          */
#define S_OUT 1         /* Open file for output                         */

/**
*
*  Flags to AlterGlobal function
*
**/
#define G_ENABLE        0
#define G_DISABLE       1
#define G_CANCEL        2

/**
*
*  File Structure
*
**/
struct FILESTRUC {
   struct FILESTRUC *f_lk;      /* Link to next file in list            */
   BPTR f_sp;                   /* AmigaDOS file handle                 */
   long f_lc;                   /* Current line number in file          */
   BOOL f_ex;                   /* exhausted state flag                 */
   int  f_io;                   /* I/O mode of file - S_IN or S_OUT     */
   char f_fn[FMAX];             /* Name of file as opened               */
};

/**
*
*  Command File Structure
*
**/
struct CMDFILE {
   LEVEL cf_el;         /* Edit level.  This is a jumpbuf...            */
   BPTR cf_sp;          /* Current command file                         */
   long cf_cp;          /* Current position in the command buffer       */
   long cf_cl;          /* Maximum length of the command buffer         */
   char cf_cb[2];       /* Current text of the command buffer           */
};

/**
*
*  Line structure
*
**/
struct LINE {
   struct LINE *l_next;
   struct LINE *l_prev;
   long l_numb;         /* Line number                                  */
   short l_cch;         /* Current character?                           */
   short l_len;         /* What a trick we have here.  This lets us use */
                        /* it As either a length prefix or not          */
   short l_buf[1];      /* Note that they need to store more than 8     */
                        /* bits into an entry.  We might be able to     */
                        /* make this more efficient by using a char,    */
                        /* but for now at least we are not using the    */
                        /* longs that it used to use                    */
};

/**
*
*   Error Codes
*
**/
#define ERROR_UC        0
#define ERROR_UDC       1
#define ERROR_BRA       2
#define ERROR_CNTX      3
#define ERROR_PM        4
#define ERROR_NUM       5
#define ERROR_LINE      6
#define ERROR_FNX       7
#define ERROR_STR       8
#define ERROR_NOM       9
#define ERROR_REP       10
#define ERROR_NOIN      11
#define ERROR_NOPR      12
#define ERROR_CR        13
#define ERROR_GLOB      14
#define ERROR_FF        15
#define ERROR_FFA       16
#define ERROR_ARG       17
#define ERROR_OPT       18
#define ERROR_RN        19
#define ERROR_GV        20
#define ERROR_CFV       21
#define ERROR_QW        22
#define ERROR_BRK       23
#define ERROR_UQL       24
#define ERROR_IQL       25
#define ERROR_MEM       26

/**
*
*  Special flag value to indicate no command.  This is only passed as a
*       parameter and is never stored in any line buffer
*
**/
#define C_NC (('N'<<8) | 'C')
#define NO_QUAL 'O'     /* Indicate that there is no qualifier          */

extern char *e_to;
extern char *e_from;
extern char *e_work;
extern char *e_ver;
extern char *e_with;
extern char *e_workout;
extern char *e_workin;
extern char *e_backup;
extern struct FILESTRUC *currentoutput;
extern struct FILESTRUC *currentinput;
extern struct FILESTRUC *primaryoutput;
extern struct FILESTRUC *primaryinput;
extern BPTR textin;
extern BPTR textout;
extern BPTR edits;
extern BPTR verout;
extern int cfsp;
extern struct CMDFILE *cfstack[CFMAX];
extern long maxlinel;
extern long maxplines;
extern struct LINE *freelines;
extern struct LINE *oldestline;
extern struct LINE *currentline;
extern long current;    /* Number of current line                       */
extern long pointer;    /* Current line position pointer                */
extern BOOL expanded;   /* Line has had all tabs expanded               */
extern BOOL condensed;  /* Line has had all deleted chars removed       */
extern BOOL exhausted;  /* We have reached end of file for current file */
extern BOOL quiet;      /* QUIET mode, don't display output lines       */
extern BOOL deleting;   /* Delete mode when walking through lines       */
extern BOOL repeating;  /* Repeat mode - Repeat previous A/B/E command  */
extern BOOL unchanged;  /* No changes have been made on the line        */
extern BOOL nosubs;     /* No substitutions have been done on this line */
extern long ceiling;    /* Halt line number                             */
extern short *linev;    /* Global pointer to current line buffer        */
extern long linel;      /* Number of characters on current line         */
extern char *commbuf;   /* Command line buffer                          */
extern long commpoint;  /* Current position in command line             */
extern long commlinel;  /* Length of command line                       */
extern long comm;       /* Current command character                    */
extern int delim;       /* Command delimiter character                  */
extern int cch;         /* Current input character                      */
extern int sw_comm;     /* Current basic command char                   */
extern int str_comm;    /* Mode of A/B/E command.  Either 'P' or C_NC   */
extern int f_qual;      /* Qualifier for current Find command           */
extern short str_match[SMAX];   /* Current Search string                */
extern short str_repl[SMAX];    /* Current search replacement           */
extern short f_match[SMAX];
extern short z_match[SMAX];
extern int globcount;
extern BOOL verifying;
extern int str_qual;
extern BOOL trailing;
extern struct FILESTRUC *filelist;
extern long *veclist;
extern BOOL opened;
extern LEVEL zerolevel;
extern LEVEL editlevel;
extern LEVEL quitlevel;
extern int rc;
extern short g_qual[GMAX];
extern short g_type[GMAX];
extern long g_count[GMAX];
extern short *g_match[GMAX];
extern short *g_repl[GMAX];
extern short svec[SMAX];
extern struct DosLibrary *DOSBase;
extern struct RDargs *rdargs;
extern char *e_temp;
extern struct Library *SysBase;   /* Contained in Cres.o */

/* EDIT1.c */
int __stdargs _main  U_ARGS((void));
void start           U_ARGS((void));
char *rdn            U_ARGS((char *p, long *val));
char *tempname       U_ARGS((char *string));
char *gettemp        U_ARGS((char *file));
void isinteractive   U_ARGS((BPTR stream));
void openstreams     U_ARGS((void));
void closestreams    U_ARGS((void));
void rewind          U_ARGS((void));
void windup          U_ARGS((void));
void closeout        U_ARGS((BPTR stream));
void closein         U_ARGS((BPTR stream));
void *newvec         U_ARGS((long n));
void discardvec      U_ARGS((void *v));
void __stdargs MemCleanup U_ARGS((void));

/* EDIT2.c */
void edit            U_ARGS((long n));

/* EDIT3.c */

void checkvalidchar  U_ARGS((void));
void checkspaceornl  U_ARGS((void));
void readcommline    U_ARGS((void));
int  commrdch        U_ARGS((void));
void uncommrdch      U_ARGS((void));
void nextcomm        U_ARGS((void));
BOOL readplusminus   U_ARGS((void));
int  commreadn       U_ARGS((void));
int  numarg          U_ARGS((int add, int opt, int def));
void readcontext     U_ARGS((short *v));
void abe_args        U_ARGS((long c));
int  getstring       U_ARGS((short *v, BOOL qsw));
void lf_arg          U_ARGS((int c));
int  getdelim        U_ARGS((BOOL flag));
int  readfiletitle   U_ARGS((char *v));
struct FILESTRUC *addfilespec     U_ARGS((char *v, int type));
struct FILESTRUC *findfilespec    U_ARGS((char *v, long type));
void losefilespec    U_ARGS((struct FILESTRUC *pf));
void closefile       U_ARGS((void));
void changecom       U_ARGS((void));
void revertcom       U_ARGS((void));
void changeout       U_ARGS((void));
void changein        U_ARGS((void));
void showdata        U_ARGS((long qual,short *string ));

/* EDIT4.c */

void renumber        U_ARGS((long n));
void split           U_ARGS((long p));
void concatenate     U_ARGS((void));
void insert          U_ARGS((void));
void readline        U_ARGS((void));
void writeline       U_ARGS((void));
void getline         U_ARGS((void));
void putline         U_ARGS((void));
void nextline        U_ARGS((void));
void prevline        U_ARGS((void));
void move            U_ARGS((long n));
void ver             U_ARGS((long c, long n));
int  vch1            U_ARGS((int));
int  vch2            U_ARGS((int));
int  vch3            U_ARGS((int));
int  hex             U_ARGS((int));
void wrl             U_ARGS((int (*vch)(int)));
void verline         U_ARGS((long c));

/* EDIT5.c */

void error           U_ARGS((int n, ...));
BOOL truncate        U_ARGS((int p));
void expand          U_ARGS((void));
void compress        U_ARGS((void)); 
void condense        U_ARGS((void));
BOOL incrementp      U_ARGS((void));
void subst           U_ARGS((int p, int q, short *v));
int  index           U_ARGS((short *, int p, int q, short *, int qual));
void readglobal      U_ARGS((void));
void alterglobal     U_ARGS((int val));
void changeglobal    U_ARGS((int i));


void __stdargs kprintf(char *, ...);
int __stdargs kgetc(void);

