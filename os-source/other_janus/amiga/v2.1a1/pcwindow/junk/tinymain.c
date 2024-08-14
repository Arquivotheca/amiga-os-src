/*      _tinymain.c         Copyright (C) 1985, 1988  Lattice, Inc.    */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include "proto.h"

#include <workbench/startup.h>

#define MAXARG 32              /* maximum command line arguments */
#define QUOTE  '"'

#define isspace(c)      ((c == ' ')||(c == '\t') || (c == '\n'))

extern struct WBStartup *WBenchMsg;

/**
*
* name         _tinymain - process command line, open files, and call "main"
*
* synopsis     _tinymain(line);
*              char *line;     ptr to command line that caused execution
*
* description   This function performs the standard pre-processing for
*               the main module of a C program.  It accepts a command
*               line of the form
*
*                       pgmname arg1 arg2 ...
*
*               and builds a list of pointers to each argument.  The first
*               pointer is to the program name.
*
**/
void _main(line)
register char *line;
{
register char **pargv;
int argc = 0;                    /* arg count */
char **targv, *argv[MAXARG];     /* arg pointers */

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
      if (*line == '\0')  XCEXIT(1);
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

/*
*
* Call user's main program
*
*/

main(argc, argc ? argv : (char **)WBenchMsg);
XCEXIT(0);
}
