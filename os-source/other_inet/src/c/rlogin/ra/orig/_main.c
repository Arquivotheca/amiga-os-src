 
/* -----------------------------------------------------------------------
 * _main.c  for rlogin  (lattice 5.10)
 *
 * $Locker:  $
 *
 * $Id: _main.c,v 1.7 91/08/07 15:22:46 bj Exp $
 *
 * $Revision: 1.7 $
 *
 * $Header: HOG:Other/inet/src/c/rlogin/RCS/_main.c,v 1.7 91/08/07 15:22:46 bj Exp $
 *
 * $Log:	_main.c,v $
 * Revision 1.7  91/08/07  15:22:46  bj
 * Fixed major bug in previous version that caused CLI to
 * hang and generated enforcer hits.
 * v 37.11 is hosed. This one (v37.12) is ok.
 * 
 * Revision 1.5  91/04/24  15:18:59  bj
 * No changes.
 * 
 * Revision 1.4  90/11/29  15:12:54  bj
 * backed out to 5.05 lattice compile. Crashes under the new version.
 * Unknown cause at this point.
 * 
 * Revision 1.3  90/11/16  19:57:31  bj
 * Fixed problem where the '*' file was not being closed which,
 * in turn, wouldn't allow the parent CLI to close - ever!
 * 
 * Revision 1.2  90/11/13  15:47:33  bj
 * Fixed things to clone Outout() to stderr so that launching
 * Cli window can close.
 * 
 * Added RCS header.
 * 
 *
 *------------------------------------------------------------------------
 */

  
/*      _main.c         Copyright (C) 1985  Lattice, Inc.       */

#include <stdio.h>
#include <fcntl.h>
#include <ios1.h>
#include <string.h>
#include <stdlib.h>
#include <workbench/startup.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>
#include <proto/exec.h>

#define MAXARG 32              /* maximum command line arguments */
#define QUOTE  '"'

#define isspace(c)      ((c == ' ')||(c == '\t') || (c == '\n'))

#ifndef TINY
extern int _fmode,_iomode;
extern int (*_ONBREAK)();
extern int CXBRK();
#endif

extern struct UFB _ufbs[];
int argc;                       /* arg count */
char **targv, *argv[MAXARG];     /* arg pointers */

#define MAXWINDOW 40
extern struct WBStartup *WBenchMsg;

void main(int, char **);

/**
*
* name         _main - process command line, open files, and call "main"
*
* synopsis     _main(line);
*              char *line;     ptr to command line that caused execution
*
* description   This function performs the standard pre-processing for
*               the main module of a C program.  It accepts a command
*               line of the form
*
*                       pgmname arg1 arg2 ...
*
*               and builds a list of pointers to each argument.  The first
*               pointer is to the program name.  For some environments, the
*               standard I/O files are also opened, using file names that
*               were set up by the OS interface module XCMAIN.
*
**/

APTR stderr_console_task ;

void _main(line)
register char *line;
{
register char **pargv;
register int x;
struct Process *process;
struct FileHandle *handle;
APTR cons ;
/*
*
* Build argument pointer list
*
*/
while (argc < MAXARG)
        {
        while (isspace(*line))  line++;
        if (*line == '\0')      break;
        pargv = &argv[argc++];
        if (*line == QUOTE)
                {
                *pargv = ++line;  /* ptr inside quoted string */
                while ((*line != '\0') && (*line != QUOTE)) line++;
                if (*line == '\0')  _exit(1);
                else                *line++ = '\0';  /* terminate arg */
                }
        else            /* non-quoted arg */
                {       
                *pargv = line;
                while ((*line != '\0') && (!isspace(*line))) line++;
                if (*line == '\0')  break;
                else                *line++ = '\0';  /* terminate arg */
                }
        }  /* while */
        
if(argc == 0) exit(20) ;
targv = (char **)&argv[0];


/*
*
* Open standard files
*
*/

process = (struct Process *)FindTask(0) ;

_ufbs[0].ufbfh = Input();
_ufbs[1].ufbfh = Output();
	/* gimme a different console task so I can close */
	/* the launching CLI window */
cons = process->pr_ConsoleTask;
handle = (struct FileHandle *)Output() ;
process->pr_ConsoleTask=(APTR)((struct FileHandle *)BADDR(handle))->fh_Type;
stderr_console_task = process->pr_ConsoleTask ;
_ufbs[2].ufbfh = Open("*", MODE_OLDFILE);
process->pr_ConsoleTask = cons ;
x = UFB_NC;                     /* do not close CLI defaults    */

_ufbs[0].ufbflg |= UFB_RA | O_RAW | x;
_ufbs[1].ufbflg |= UFB_WA | O_RAW | x;
_ufbs[2].ufbflg |= UFB_WA | UFB_RA | O_RAW ; /* no no_close flag ! */

x = (_fmode) ? 0 : _IOXLAT;
stdin->_file = 0;
stdin->_flag = _IOREAD | x;
stdout->_file = 1;
stdout->_flag = _IOWRT | x;
stderr->_file = 2;
stderr->_flag = _IORW | x;

/*      establish control-c handler */

_ONBREAK = CXBRK;

/*
*
* Call user's main program
*
*/

main(argc,targv);              /* call main function */

process->pr_ConsoleTask=stderr_console_task ;
Close(_ufbs[2].ufbfh) ;
_ufbs[2].ufbflg = 0L ;
process->pr_ConsoleTask = cons ;

exit(0);

}
