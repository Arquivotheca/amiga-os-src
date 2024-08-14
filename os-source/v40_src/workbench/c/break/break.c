; /*
lc -d -j73 -O -o/obj/Break.o -i/include -v -csf Break
blink /obj/Break.o to /bin/Break sc sd nd
protect /bin/Break +p
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

/*----------------------------------------------------------------------*/
/* Command: Break                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 25OCT89  John Toebes   Revamp for DOS36.21                           */
/* 02DEC89  John Toebes   Final rewrite after code review               */
/* 03MAR90  John Toebes   Corrected B7128 - default to ^C               */
/* 06NOV90  John Toebes   Validated to protect against FindCliProc bug  */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "break_rev.h"

#define MSG_NOPROC  "Process %ld does not exist\n"

#define TEMPLATE    "PROCESS/A/N,ALL/S,C/S,D/S,E/S,F/S" CMDREV
#define OPT_PROCESS 0
#define OPT_ALL     1
#define OPT_C       2
#define OPT_D       3
#define OPT_E       4
#define OPT_F       5
#define OPT_COUNT   6

int cmd_break(void)
{
   struct DosLibrary *DOSBase;
   struct Library *SysBase = (*((struct Library **) 4));
   int                rc;
   long               opts[OPT_COUNT];
   long               sigmask;
   struct Process     *proc;
   struct RDargs      *rdargs;
   int                 clinum;

   rc = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         /*-------------------------------------------------------------*/
         /* Construct the signal mask by adding the flags for each of   */
         /* the options that they specified on the command line.  Note  */
         /* that it is possible to end up with no bits set just by the  */
         /* user not setting any options but we assume that they know   */
         /* what they are doing.                                        */
         /*-------------------------------------------------------------*/
         sigmask = 0;
         if (opts[OPT_C])   sigmask |= SIGBREAKF_CTRL_C;
         if (opts[OPT_D])   sigmask |= SIGBREAKF_CTRL_D;
         if (opts[OPT_E])   sigmask |= SIGBREAKF_CTRL_E;
         if (opts[OPT_F])   sigmask |= SIGBREAKF_CTRL_F;
         if (opts[OPT_ALL]) sigmask  = SIGBREAKF_CTRL_C |
                                       SIGBREAKF_CTRL_D |
                                       SIGBREAKF_CTRL_E |
                                       SIGBREAKF_CTRL_F;

         if (sigmask == 0)  sigmask |= SIGBREAKF_CTRL_C; /* Default it  */
         /*-------------------------------------------------------------*/
         /* We need to forbid so that any process we find doesn't go    */
         /* away before we get to it.                                   */
         /*-------------------------------------------------------------*/
         Forbid();

         /*-------------------------------------------------------------*/
         /* Ask DOS to find the task for the CLI process number.        */
         /*-------------------------------------------------------------*/
         clinum = *(long *)opts[OPT_PROCESS];
         if (clinum > 0 &&              /* validate to protect against  */
             clinum <= MaxCli() &&      /* problems with FindCliProc    */
             (proc = FindCliProc(clinum)))
         {
            /*----------------------------------------------------------*/
            /* We found it, send the signal(s) they requested.          */
            /*----------------------------------------------------------*/
            Signal((struct Task *)proc, sigmask);
            rc = RETURN_OK;
         }
         else
         {
            /*----------------------------------------------------------*/
            /* Well the process wasn't there.  Print a message about it */
            /* note that this breaks the Forbid but it doesn't matter   */
            /*----------------------------------------------------------*/
            VPrintf(MSG_NOPROC, (long *)opts[OPT_PROCESS]);
         }
         /*-------------------------------------------------------------*/
         /* Done, clean up and exit.                                    */
         /*-------------------------------------------------------------*/
         Permit();
         FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      OPENFAIL;
   }
   return(rc);
}
