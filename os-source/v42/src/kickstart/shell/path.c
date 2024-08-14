/*
lc -d -j73 -O -o/obj/Path.o -i/include -v -csf Path
blink /obj/Path.o to /bin/Path sc sd nd
protect /bin/Path +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: Path                                                             */
/* Author: James E. Cooper Jr.                                               */
/* Change History:                                                           */
/*  Date    Person        Action                                             */
/* -------  ------------- -----------------                                  */
/* 26MAR89  Jim Cooper    Initial Creation                                   */
/* 14APR89  Jim Cooper    Cleanup & Comment                                  */
/* 04SEP89  Jim Cooper    Conversion to 1.4 Completed (I thought!)           */
/* 22OCT89  Jim Cooper    Conversion to 1.4 Proceeding                       */
/* 01NOV89  Jim Cooper    Interim version                                    */
/* 04NOV89  Jim Cooper    Comments added, code cleaned up, waiting on '/M'   */
/* 28NOV89  Jim Cooper    Added '/M' support, FINALLY finished!              */
/* 11MAR90  Jim Cooper    Added 'SUBTRACT' switch, fixed to prevent multiple */
/*                        instances of a single node.                        */
/* Notes:                                                                    */
/*---------------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
#include "global.h"

/* #include "path_rev.h" */

#define SMALL 1

#define TEMPLATE  "PATH/M,ADD/S,SHOW/S,RESET/S,REMOVE/S,QUIET/S"

#define OPT_PATH     0
#define OPT_ADD      1
#define OPT_SHOW     2
#define OPT_RESET    3
#define OPT_SUBTRACT 4
#define OPT_QUIET    5
#define OPT_COUNT    6

/*---------------------------------------------------------------------------*/
/* The PathNode structure is not documented anywhere I could find, but I     */
/* have had occasion to access it in the past.  Obviously, it works!  Q.E.D. */
/*---------------------------------------------------------------------------*/
struct PathNode {
  BPTR next;
  BPTR lock;
};

int cmd_path(void)
{
  struct DosLibrary *DOSBase;
  int rc = RETURN_ERROR, i;
  BPTR tn;
  struct PathNode *pathNode, *nextNode, *tempNode;
  long opts[OPT_COUNT];
  UBYTE nodeName[ENVMAX]; /* path node name storage */
  struct RDArgs *rdargs;
  struct FileInfoBlock fi;
  struct CommandLineInterface *cli_ptr;
  char *curarg, **argptr;
  struct Process *tid=THISPROC;
  struct Window *wp;



#ifndef SMALL
    DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
    memset((char *)opts, 0, sizeof(opts));
    rdargs = ReadArgs(TEMPLATE, opts, NULL);
    if (rdargs == NULL)PrintFault(IoErr(),NULL);
    else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
    wp=tid->pr_WindowPtr;
    cli_ptr = Cli();   /* pointer to current CLI */
      /*---------------------------------------------------------------------*/
      /* Check input.  If user just typed the command name, we must show     */
      /* the current path.  Easiest way is to set the 'SHOW' option, and     */
      /* fall through the code.                                              */
      /*---------------------------------------------------------------------*/
	/* QUIET is a special case, so we leave it out */
      for (i=0; i<OPT_QUIET; i++) if (opts[i]) break;

      if (i >= OPT_QUIET) opts[OPT_SHOW] = -1;

#ifdef EXTRAINFO
      /*---------------------------------------------------------------------*/
      /* If they told us to both ADD and SUBTRACT, tell them they made a       */
      /* mistake in their input.                                             */
      /*---------------------------------------------------------------------*/
      if ((opts[OPT_ADD] != NULL) && (opts[OPT_SUBTRACT] != NULL)) {
	PrintFault(118,NULL);
        goto Err;
      }
#endif

      /*---------------------------------------------------------------------*/
      /* get the pointer to the head of the PathNode list from the CLI struct*/
      /*---------------------------------------------------------------------*/
      pathNode = (struct PathNode *)BADDR(cli_ptr->cli_CommandDir);

      /*---------------------------------------------------------------------*/
      /* If the 'RESET' switch was used, we have to kill the current path    */
      /* before we do anything else.                                         */
      /*---------------------------------------------------------------------*/
      if (opts[OPT_RESET]) {

        /*-------------------------------------------------------------------*/
        /* Step through the list, killing each node.                         */
        /*-------------------------------------------------------------------*/
        while (pathNode) {
          nextNode = (struct PathNode *)BADDR(pathNode->next);
          UnLock(pathNode->lock);
          FreeVec(pathNode);
          pathNode = nextNode;
        }

        /*-------------------------------------------------------------------*/
        /* Make sure there IS no path, so that the next routine works        */
        /* properly.                                                         */
        /*-------------------------------------------------------------------*/
        cli_ptr->cli_CommandDir = NULL;
      }

      /*---------------------------------------------------------------------*/
      /* Set up a temporary variable to save some typing and a little bit of */
      /* code size on the output.                                            */
      /*---------------------------------------------------------------------*/
      argptr = (char **)opts[OPT_PATH];
      tempNode = NULL;

      /*---------------------------------------------------------------------*/
      /* The following while loop handles the NEW MultiArgs spec.            */
      /*---------------------------------------------------------------------*/
      while (argptr && (curarg = *argptr++)) {

        /*-------------------------------------------------------------------*/
        /* Get a piece of memory.                                            */
        /*-------------------------------------------------------------------*/
        if ((tempNode = AllocVec(sizeof(struct PathNode),
                                 MEMF_PUBLIC|MEMF_CLEAR)) == NULL) {
          PrintFault(IoErr(),NULL);
          goto Err;
        }

        /*-------------------------------------------------------------------*/
        /* Try to get a lock on the specified path, and Examine() it.        */
        /*-------------------------------------------------------------------*/
        if (((tempNode->lock = Lock(curarg,ACCESS_READ)) == NULL) ||
             (Examine(tempNode->lock, &fi) == NULL)) {
          PrintFault(IoErr(),curarg);
          goto Err;
        }

        /*-------------------------------------------------------------------*/
        /* Now, is it a directory, or is it a file?                          */
        /*-------------------------------------------------------------------*/
        if (fi.fib_DirEntryType < NULL) {
          SetIoErr(ERROR_OBJECT_WRONG_TYPE);
          PrintFault(IoErr(),fi.fib_FileName);
          goto Err;
        }

        /*-------------------------------------------------------------------*/
        /* Shades of the Past!  We must convert the pointer to the new       */
        /* node into a BPTR, for storage in the PathNode structure.          */
        /*-------------------------------------------------------------------*/
        tn = MKBADDR(tempNode);

        /*-------------------------------------------------------------------*/
        /* Now, add it to the end of the list.                               */
        /*-------------------------------------------------------------------*/
        nextNode = (struct PathNode *)BADDR(cli_ptr->cli_CommandDir);
        if (nextNode) {

#ifdef COMPAT
          /*-----------------------------------------------------------------*/
          /* Find the end of the list.                                       */
          /*-----------------------------------------------------------------*/
          while (nextNode->next) {
            nextNode = (struct PathNode *)BADDR(nextNode->next);
          }
          nextNode->next = tn;                  /* link new node to list end */
        } else {
          cli_ptr->cli_CommandDir = tn;
        }
        tempNode = NULL;
#else
          /*-----------------------------------------------------------------*/
          /* Search the list, checking for duplicates.  If we are adding,    */
          /* skip if we find a duplicate.  If we are removing, delete the    */
          /* duplicate (its the one we want!).  If we are adding and we      */
          /* reach the end of the list, simply tack the new node on.         */
          /*-----------------------------------------------------------------*/
          pathNode = nextNode;
          while (TRUE) {
            if (SameLock(nextNode->lock, tempNode->lock) == LOCK_SAME) {
              if (opts[OPT_SUBTRACT] == NULL) {
#ifdef NASTY
                SetIoErr(ERROR_LOCK_COLLISION);
                PrintFault(IoErr(),curarg);
                goto Err;
#else
                break;
#endif
              } else {

                /*-----------------------------------------------------------*/
                /* Ok, we found a match, and we are removing.  Take it out   */
                /* of the path list, and free it.                            */
                /*-----------------------------------------------------------*/
                if (pathNode == nextNode) {
                  cli_ptr->cli_CommandDir = nextNode->next;
                } else {
                  pathNode->next = nextNode->next;
                }
                UnLock(nextNode->lock);
                FreeVec(nextNode);
                break;
              }
            }
            pathNode = nextNode;
            nextNode = (struct PathNode *)BADDR(nextNode->next);
            if (nextNode == NULL) {
              if (opts[OPT_SUBTRACT] == NULL) {
                pathNode->next = tn;
                tempNode = NULL;
              }
              break;
            }
          }
        } else {
          if (opts[OPT_SUBTRACT] == NULL) {
            cli_ptr->cli_CommandDir = tn;
            tempNode = NULL;
          }
        }
#endif
      }

      rc = RETURN_OK;

      /*---------------------------------------------------------------------*/
      /* If the 'SHOW' option was set, by them OR us, print each node,       */
      /* adding the default of the current directory at the top, and 'C:' at */
      /* the end.                                                            */
      /*---------------------------------------------------------------------*/
      if (opts[OPT_SHOW]) {
	/* if quiet, turn off requesters */
	if(opts[OPT_QUIET])tid->pr_WindowPtr = (struct Window *)(-1);

	PrintFault(STR_CURRENT_DIR,NULL); /* Path ALWAYS starts in current dir */
        nextNode = (struct PathNode *)BADDR(cli_ptr->cli_CommandDir);
        while (nextNode) {                  /* Walk the path, printing names */
	     if(CheckSignal(SIGBREAKF_CTRL_C)) {
	    	PrintFault(304,0);
		goto Err;
	     }
	    if(NameFromLock(nextNode->lock, nodeName, ENVMAX)) {
                PutStr(nodeName);
                PutStr("\n");
	    }
	  else PrintFault(IoErr(), NULL);
          nextNode = (struct PathNode *)BADDR(nextNode->next);
        }
	PutStr("C:\n");                  /* Path ends at 'C:' assignment */
      }
Err:
      /*---------------------------------------------------------------------*/
      /* We have to clean up after ourselves, whether there was an error or  */
      /* everything worked!                                                  */
      /*---------------------------------------------------------------------*/
	/* restore window pointer always */
      tid->pr_WindowPtr = wp;

      if (tempNode) {
        if (tempNode->lock) {
          UnLock(tempNode->lock);
        }
        FreeVec(tempNode);
      }
      FreeArgs(rdargs);
    }
    CloseLibrary((struct Library *)DOSBase);
#ifdef ram
  } 
  else {
    OPENFAIL;
  }
#endif
  return(rc);
}
