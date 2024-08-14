; /*
lc -d -j73 -O -o/obj/ChangeTaskPri.o -i/include -v -csf ChangeTaskPri
blink /obj/ChangeTaskPri.o to /bin/ChangeTaskPri sc sd nd
protect /bin/ChangeTaskPri +p
quit
*/

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

/*----------------------------------------------------------------------*/
/* Command: ChangeTaskPri						*/
/* Author:  John A. Toebes, VIII					*/
/* Change History:							*/
/*  Date    Person	  Action					*/
/* -------  ------------- -----------------				*/
/* 19MAR89  John Toebes   Initial Creation				*/
/* 02DEC89  John Toebes   Final cleanup after code review		*/
/* 24SEP90  John Toebes   Correct process message for bug B9868         */
/* 06NOV90  John Toebes   Added protection around broken FindCliProc    */
/* Notes:								*/
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "changetaskpri_rev.h"

#define MSG_NOPROC "Process %ld does not exist\n"
#define MSG_BADPRI "Priority out of range (-128 to +127)\n"

#define TEMPLATE    "PRI=PRIORITY/A/N,PROCESS/K/N" CMDREV
#define OPT_PRI     0
#define OPT_PROCESS 1
#define OPT_COUNT   2

int cmd_changetaskpri(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   int rc, pri, pnum;
   char *msg;
   long opts[OPT_COUNT];
   struct Process *proc;
   struct RDargs *rdargs;

   rc = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
	 PrintFault(IoErr(), NULL);
      }
      else
      {
	 /*-------------------------------------------------------------*/
	 /* We forbid to prevent any tasks from disappearing out from	*/
	 /* underneath us						*/
	 /*-------------------------------------------------------------*/
	 Forbid();

	 /*-------------------------------------------------------------*/
	 /* We will use msg as a flag that something went wrong.  If	*/
	 /* there was anything at all wrong with the command, set msg	*/
	 /* to point to the diagnostic string and fall through		*/
	 /*-------------------------------------------------------------*/
	 msg = NULL;

	 /*-------------------------------------------------------------*/
	 /* Ok, now choose a default process (ourselves of course)      */
	 /*-------------------------------------------------------------*/
	 proc = THISPROC;

	 /*-------------------------------------------------------------*/
	 /* Parse out the priority they want use to set it to		*/
	 /*-------------------------------------------------------------*/
	 pri = *(long *)opts[OPT_PRI];

	 /*-------------------------------------------------------------*/
	 /* Make sure we are setting it to a legit priority		*/
	 /*-------------------------------------------------------------*/
	 if (pri < -128 || pri > 127)
	    msg = MSG_BADPRI;

	 /*-------------------------------------------------------------*/
	 /* if they specified a process, use it to override the default */
	 /*-------------------------------------------------------------*/
	 else if (opts[OPT_PROCESS])
	 {
	    /*----------------------------------------------------------*/
	    /* For convenience, the substitution will occur with the	*/
	    /* process parameter.					*/
	    /*----------------------------------------------------------*/
	    pnum = *(long *)opts[OPT_PROCESS];
	    opts[OPT_PRI] = pnum;

	    /*----------------------------------------------------------*/
	    /* Now ask for the process from the system			*/
	    /*----------------------------------------------------------*/
	    if ((pnum < 1) || (pnum > MaxCli()) ||
                ((proc = FindCliProc(pnum)) == NULL))
	       msg = MSG_NOPROC;
	 }

	 /*-------------------------------------------------------------*/
	 /* When get to here, we have msg set to NULL if everything went*/
	 /* well.  Otherwise we have a message and substitutions to do	*/
	 /*-------------------------------------------------------------*/
	 if (msg)
	 {
	    /*----------------------------------------------------------*/
	    /* This breaks the forbid but it really doesn't matter      */
	    /*----------------------------------------------------------*/
	    VPrintf(msg, opts);
	 }
	 else
	 {
	    /*----------------------------------------------------------*/
	    /* We got here and have a valid process and priority to set */
	    /*----------------------------------------------------------*/
	    SetTaskPri((struct Task *)proc, pri);
	    rc = RETURN_OK;
	 }
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
