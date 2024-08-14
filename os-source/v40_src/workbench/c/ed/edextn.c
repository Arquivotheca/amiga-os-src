/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 481-6436    */
/* | o  | ||   John Toebes     John Mainwaring    Jim Cooper                 */
/* |  . |//    Bruce Drake     Gordon Keener      Dave Baker                 */
/* ======      Doug Walker                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*----------------------------------------------------------------------*/
/* Command: EditExtn.c                                                  */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 03NOV89  John Toebes   Initial Creation                              */
/* 30MAY90  John Toebes   Added _stack variable                         */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "Ed.h"

/*----------------------------------------------------------------------*/
/* GLOBAL DATA                                                          */
/*----------------------------------------------------------------------*/
struct LINE *line_first; /* Start of first line in buffer        */
struct LINE *line_last;  /* Start of last line in buffer         */
struct LINE *line_ptr[MAXLINES];
                                /* line pointers to lines on screen     */
char line_mods[MAXLINES];/* Vector of modified lines to display  */
char current_line[LINELENGTH];
                                /* Current line buffer                  */
int current_size;        /* Length of current line               */
struct LINE *prev_list;  /* Start of backwards list of lines     */
int line_num;            /* Highest index of line on screen      */
int line_max;            /* max usable lines on screen           */
int screen_width;        /* highest char pos on a line           */
char *line_lastword;     /* last word held in store              */
/* Cursor locations     */
int phys_x, phys_y;      /* actual positions                     */
int log_x, log_y;        /* where user thinks cursor is          */
char *worktop;
char file_name[LINELENGTH];
BOOL extracted;          /* TRUE if current line extracted from store    */
int window_base;         /* Offset of screen window              */
char *cmdline;           /* Extended command line store          */
char cmdbuf[LINELENGTH];
char lastcmd[LINELENGTH];           /* Last executed command                */
int cmdptr;              /* Ptr within it                        */
int tab_stop;            /* Current tab setting                  */
struct LINE *block_start;/* Pointer to block start               */
struct LINE *block_end;  /* Pointer to block end                 */
char status_msg[LINELENGTH];
                                /* Generic status line message          */
int msg_num;             /* Message number for pending error     */
int left_margin;         /* Left margin                          */
int right_margin;        /* Right margin                         */
int term_width;          /* User specified terminal width        */
int term_height;         /* User specified terminal height       */
/* Error handling       */
LEVEL err_level;         /* Level for LONGJMP                    */
#define ERRLABEL 1              /* Label for LONGJMP                    */
/* Space for arguments  */
/* Note that the next two are BSTRs with a null terminator.             */
char search_vec[LINELENGTH];        /* Space for search string              */
char insert_vec[LINELENGTH];        /* Space for insert string              */
BOOL extend_margin;      /* TRUE if margin extended              */
/* TRIPOS additions     */
BOOL data_changed;       /* Flag to indicate file has changed    */
BPTR console_stream;     /* Console window for editing           */
struct StandardPacket *read_pkt;
struct MsgPort *reply_port;
BOOL forcecase;          /* Flag to make search case insensitive */
char vud_buf[VUD_SIZE];  /* Buffer for terminal output           */
int vud_bpos;            /* Position within that buffer          */
struct DosLibrary *DOSBase;
struct Window *window;   /* Window for current editing           */
char *keydef[MAX_PFKEY]; /* PFKey definitions.                   */
struct MsgPort *rexx_port;
struct CMDLEVEL *pending;/* Pending command levels               */
int outstanding_rexx_commands;
struct FileInfoBlock *fib;
struct Library *AslBase;
struct Library *GadToolsBase;
struct IntuitionBase *IntuitionBase;
void *vi;                /* Visual info for gadtools             */
int menuattached;        /* Flag to indicate attached menu       */
struct Menu *menu;       /* Attached menu structure              */
struct NewMenu newmenu[MAXMENU];
int requestflag;         /* Allow the file requester to be used  */
struct FileRequester *FileRequester;
char portname_buf[10];   
struct RxsLib *RexxSysBase;     /* this is the rexx library base */
long _stack = 4096;         /* Minimum stack size */
struct Library *IconBase;
struct DiskObject *diskobj;

