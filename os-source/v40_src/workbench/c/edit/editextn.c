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

/*-----------------------------------------------------------------------*/
/* Command: EditExtn.c                                                   */
/* Author:  John A. Toebes, VIII                                         */
/* Change History:                                                       */
/*  Date    Person        Action                                         */
/* -------  ------------- -----------------                              */
/* 03NOV89  John Toebes   Initial Creation                               */
/* 17APR91  John Toebes   Corrected sizes of buffers                     */
/* Notes:                                                                */
/*-----------------------------------------------------------------------*/
#include "edit.h"

char *e_to;
char *e_from;
char *e_work;
char *e_ver;
char *e_with;
char *e_workout;
char *e_workin;
char *e_backup;
struct FILESTRUC *currentoutput;
struct FILESTRUC *currentinput;
struct FILESTRUC *primaryoutput;
struct FILESTRUC *primaryinput;
BPTR textin;
BPTR textout;
BPTR edits;
BPTR verout;
int cfsp;
struct CMDFILE *cfstack[CFMAX];
long maxlinel;
long maxplines;
struct LINE *freelines;
struct LINE *oldestline;
struct LINE *currentline;
long current;    /* Number of current line                       */
long pointer;    /* Current line position pointer                */
BOOL expanded;   /* Line has had all tabs expanded               */
BOOL condensed;  /* Line has had all deleted chars removed       */
BOOL exhausted;  /* We have reached end of file for current file */
BOOL quiet;      /* QUIET mode, don't display output lines       */
BOOL deleting;   /* Delete mode when walking through lines       */
BOOL repeating;  /* Repeat mode - Repeat previous A/B/E command  */
BOOL unchanged;  /* No changes have been made on the line        */
BOOL nosubs;     /* No substitutions have been done on this line */
long ceiling;    /* Halt line number                             */
short *linev;    /* Global pointer to current line buffer        */
long linel;      /* Number of characters on current line         */
char *commbuf;   /* Command line buffer                          */
long commpoint;  /* Current position in command line             */
long commlinel;  /* Length of command line                       */
long comm;       /* Current command character                    */
int delim;       /* Command delimiter character                  */
int cch;         /* Current input character                      */
int sw_comm;     /* Current basic command char                   */
int str_comm;    /* Mode of A/B/E command.  Either 'P' or C_NC   */
int f_qual;      /* Qualifier for current Find command           */
short str_match[SMAX];   /* Current Search string                */
short str_repl[SMAX];    /* Current search replacement           */
short f_match[SMAX];
short z_match[SMAX];
int globcount;
BOOL verifying;
int str_qual;
BOOL trailing;
struct FILESTRUC *filelist;
long *veclist;
BOOL opened;
LEVEL zerolevel;
LEVEL editlevel;
LEVEL quitlevel;
int rc;
short g_qual[GMAX];
short g_type[GMAX];
long g_count[GMAX];
short *g_match[GMAX];
short *g_repl[GMAX];
short svec[SMAX];
char *e_temp;

struct DosLibrary *DOSBase;
struct RDargs *rdargs;
