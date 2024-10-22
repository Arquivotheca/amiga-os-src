/*
lc -d -j73 -O -o/obj/Ask.o -i/include -v -csf Ask
blink /obj/Ask.o to /bin/Ask sc sd nd
protect /bin/Ask +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: Ask                                                         */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 25OCT89  John Toebes   Revamp for DOS36.21                           */
/* 02DEC89  John Toebes   Final update after code inspection            */
/* 09DEC89  John Toebes   Check in to libraries                         */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "global.h"
/* #include "ask_rev.h" */

#define SMALL 1

#define TEMPLATE    "PROMPT/A"
#define OPT_PROMPT  0
#define OPT_COUNT   1

#define QUERY       "Y=YES/S,N=NO/S"
#define QUERY_YES   0
#define QUERY_NO    1
#define QUERY_COUNT 2

int cmd_ask(void)
{
   struct DosLibrary *DOSBase;
   int                rc = RETURN_ERROR;
   struct RDargs     *rdargs;
   struct RDargs     *qrdargs;
   long               opts[OPT_COUNT];
   long               query[QUERY_COUNT];

#ifndef SMALL
   DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL)PrintFault(IoErr(), NULL);
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
         /*-------------------------------------------------------------*/
         /* Loop until we get an answer.  We might consider adding a    */
         /* ^C check at some point in time if we decide to allow        */
         /* aborting the operation.                                     */
         /*-------------------------------------------------------------*/
         do
         {
            /*----------------------------------------------------------*/
            /* Display the prompt that they specified                   */
            /*----------------------------------------------------------*/
	    VPrintf("%s ", opts);
            Flush(Output());    /* Make sure it is there for the prompt */

            /*----------------------------------------------------------*/
            /* Let ReadArgs prompt for an answer                        */
            /*----------------------------------------------------------*/
            query[QUERY_YES] = query[QUERY_NO] = 0;
	    qrdargs = ReadArgs(QUERY, query, NULL);

            /*----------------------------------------------------------*/
            /* Until they respond correctly                             */
            /*----------------------------------------------------------*/
	 } while(qrdargs == NULL);
         FreeArgs(qrdargs);

         /*-------------------------------------------------------------*/
         /* See what they responded with.  If they didn't say YES, we   */
         /* will assume a NO.  Note that this allows them to say YES NO */
         /* and we will still take that as a YES                        */
         /*-------------------------------------------------------------*/
         rc = RETURN_OK;
         if (query[QUERY_YES])
            rc = RETURN_WARN;

         /*-------------------------------------------------------------*/
         /* Done, free the memory and go home.                          */
         /*-------------------------------------------------------------*/
         FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else {
      /* We couldn't open DOS.library so let them know */
      OPENFAIL;
   }
#endif
   return(rc);
}
