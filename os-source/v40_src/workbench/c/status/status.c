; /*
lc -d -j73 -O -o/obj/Status.o -rr -i/include -v -csf Status
blink /obj/Status.o to /bin/Status sc sd nd lib lib:lcr.lib
protect /bin/Status +p
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

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: Status                                                      */
/* Author:  John Toebes                                                 */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 25OCT89  John Toebes   Revamp for DOS36.21                           */
/* 05DEC89  John Toebes   Last crunch to make it right                  */
/* 03MAR90  John Toebes   Corrected B6527 - off by 1 limit on CLI count */
/* 10DEC90  John Toebes   Corrected B10925 - Does not respect ^c        */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "status_rev.h"

int stricmp(char *, char *);

#define MSG_NOPROC   "Process %ld does not exist\n"
#define MSG_PROCNUM  "Process %2ld: "
#define MSG_PROCINFO "stk %5ld, gv %3ld, pri %3ld "
#define MSG_LOADAS   "Loaded as command: %s\n"
#define MSG_NOCMD    "No command loaded\n"

#define TEMPLATE    "PROCESS/N,FULL/S,TCB/S,CLI=ALL/S,COM=COMMAND/K" CMDREV
#define OPT_PROCESS 0
#define OPT_FULL    1
#define OPT_TCB     2
#define OPT_CLI     3
#define OPT_COMMAND 4
#define OPT_COUNT   5

int cmd_status(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   int rc, i;
   char *msg;
   long opts[OPT_COUNT];
   struct Process *proc;
   char cmdbuf[128];
   long args[4];
   struct CommandLineInterface *cli;
   int clicount;
   struct RDargs *rdargs;

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
         /* Check and set up the default options.                       */
         /* FULL implies TCB and CLI                                    */
         /*-------------------------------------------------------------*/
         if (opts[OPT_FULL])
            opts[OPT_TCB] = opts[OPT_CLI] = opts[OPT_FULL];
         /*-------------------------------------------------------------*/
         /* For no options given, we default to CLI                     */
         /*-------------------------------------------------------------*/
         else if (!opts[OPT_TCB])
            opts[OPT_CLI] = 1;

         /*-------------------------------------------------------------*/
         /* Determine the range of items we will have to search         */
         /*-------------------------------------------------------------*/
         if (opts[OPT_PROCESS])
         {
            /*----------------------------------------------------------*/
            /* If the specified one, we go just for that single one     */
            /*----------------------------------------------------------*/
            i = clicount = *(long *)opts[OPT_PROCESS];
	    opts[OPT_PROCESS] = clicount;
            msg = MSG_NOPROC;
         }
         else
         {
            /*----------------------------------------------------------*/
            /* None specified, run through them all.                    */
            /*----------------------------------------------------------*/
            i = 1;
            clicount = MaxCli();
            msg = NULL;
         }

         /*-------------------------------------------------------------*/
         /* When we are searching for a specific command, default the   */
         /* Return code to a warning in case we don't find the command  */
         /*-------------------------------------------------------------*/
         if (opts[OPT_COMMAND]) 
            rc = RETURN_WARN;

         /*-------------------------------------------------------------*/
         /* Now we loop through all the processes checking for a match  */
         /*-------------------------------------------------------------*/
         for (; i <= clicount; i++)
         {
            if (proc = FindCliProc(i))
            {
               /*-------------------------------------------------------*/
               /* We have a process, now see if we need to print it out */
               /* First see if anything is loaded for it                */
               /*-------------------------------------------------------*/
               cli = (struct CommandLineInterface *)BADDR(proc->pr_CLI);
               cmdbuf[0] = 0;
               if (cli->cli_Module != NULL)
               {
                  msg = (char *)BADDR(cli->cli_CommandName);
                  memcpy(cmdbuf, msg+1, msg[0]);
                  cmdbuf[msg[0]] = 0;
               }
               if (opts[OPT_COMMAND])
               {
                  /*----------------------------------------------------*/
                  /* Here we are looking for a specific command         */
                  /*----------------------------------------------------*/
                  if (!stricmp((char *)opts[OPT_COMMAND], cmdbuf))
                  {
                     args[0] = (long)i;
                     VPrintf("%2ld\n", args);
                     rc = RETURN_OK;
                  }
               }
               else
               {
                  /*----------------------------------------------------*/
                  /* We got here so there must be a command to do       */
                  /*----------------------------------------------------*/
                  args[0] = (long)i;
                  VPrintf(MSG_PROCNUM, args);
                  if (opts[OPT_TCB])
                  {
                     args[0] = (long)(cli->cli_DefaultStack << 2);
                     args[1] = *(long*)proc->pr_GlobVec;
                     args[2] = (long)proc->pr_Task.tc_Node.ln_Pri;
                     VPrintf(MSG_PROCINFO, args);
                  }
                  msg = "\n";
                  if (opts[OPT_CLI])
                  {
                     msg = MSG_NOCMD;
                     if (cmdbuf[0])
                     {
                        msg = MSG_LOADAS;
                     }
                  }
                  args[0] = (long)cmdbuf;
                  VPrintf(msg, args);
                  rc = RETURN_OK;
               }
             msg = NULL;
             }
             /*---------------------------------------------------------*/
             /* See if they hit a ^C at any time in the process and exit*/
             /*---------------------------------------------------------*/
             if (CheckSignal(SIGBREAKF_CTRL_C))
             {
                PrintFault(ERROR_BREAK, NULL);
                msg = NULL;
                break;
             }

          }
         if (msg)
            VPrintf(msg, opts);
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
