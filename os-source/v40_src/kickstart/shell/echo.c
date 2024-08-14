/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*----------------------------------------------------------------------*/
/* Command: Echo                                                        */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 22MAR89  John Toebes   Initial Implementation                        */
/* 19OCT89  John Toebes   Complete rewrite for new ReadArgs             */
/* 25OCT89  John Toebes   Revamp for DOS36.21                           */
/* 02DEC89  John Toebes   Final Touches after code review               */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "global.h"

#define SMALL 1

#define TEMPLATE    "/M,NOLINE/S,FIRST/K/N,LEN/K/N,TO/K"
#define OPT_STRING  0
#define OPT_NOLINE  1
#define OPT_FIRST   2
#define OPT_LEN     3
#define OPT_TO	    4
#define OPT_COUNT   5

int cmd_echo(void)
{
   struct DosLibrary *DOSBase;
   int cnt, rc = RETURN_ERROR, first, len;
   char *msg;
   long opts[OPT_COUNT];
   struct RDargs *rdargs;
   char **argptr;
   BPTR fh;
   BOOL loop=0;

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL) {
         PrintFault(IoErr(), NULL);
      }
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
         if (opts[OPT_TO]) {
            if (!(fh = Open((char *)opts[OPT_TO], MODE_NEWFILE)))
               goto ERROR;
         }
         else fh = Output();
         rc = RETURN_OK;

         /*-------------------------------------------------------------*/
         /* When no string is given then we need to skip the output     */
         /*-------------------------------------------------------------*/
         if (opts[OPT_STRING]) {
	   argptr = (char **) opts[OPT_STRING];
	   while (msg = *argptr++) {

            /*----------------------------------------------------------*/
            /* Figure out how long it is to start with                  */
            /*----------------------------------------------------------*/
            cnt = strlen(msg);

            /*----------------------------------------------------------*/
            /* Did they give us a FIRST option                          */
            /*----------------------------------------------------------*/
            if (opts[OPT_FIRST]) {
               first = *(long *)opts[OPT_FIRST]-1;

               /*-------------------------------------------------------*/
               /* Make sure the start position doesn't push us          */
	       /* past the end of the string                            */
               /*-------------------------------------------------------*/
               if (first >= cnt) {
                  /*----------------------------------------------------*/
                  /* If so, just max it out                             */
                  /*----------------------------------------------------*/
                  first = cnt-1;
               }
   
               /*-------------------------------------------------------*/
               /* Now position us into the string appropriately         */
               /*-------------------------------------------------------*/
               if (first >= 0)
               {
                  msg += first;
                  cnt -= first;
               }
            }

            /*----------------------------------------------------------*/
            /* Did they give us a length option to limit the string ?   */
            /*----------------------------------------------------------*/
            if (opts[OPT_LEN]) {
               len = *(long *)opts[OPT_LEN];
               /*-------------------------------------------------------*/
               /* If the length is greater than the number of characters*/
	       /* in the string then we need to just use the full string*/
               /*-------------------------------------------------------*/
               if (len < cnt) {
                  /*----------------------------------------------------*/
                  /* Otherwise in the absence of the FIRST option,      */
		  /* use the right most characters in the string.       */
                  /*----------------------------------------------------*/
                  if (opts[OPT_FIRST] == NULL) msg += (cnt-len);
                  cnt = len;
               }
            }

            /*----------------------------------------------------------*/
            /* We have calculated the lengths so write out the string   */
            /*----------------------------------------------------------*/
	    if(loop)FPutC(fh,' ');
            FWrite(fh,msg, 1, cnt);
	    loop=TRUE;
	   }
            /*----------------------------------------------------------*/
            /* Unless they wanted a newline on the string...            */
            /*----------------------------------------------------------*/
            if (opts[OPT_NOLINE] != DOSTRUE)FPutC(fh,'\n');
	    else Flush(fh);
	   }
         /*-------------------------------------------------------------*/
         /* All is done, free the args and exit                         */
         /*-------------------------------------------------------------*/
	if(opts[OPT_TO])Close(fh);
ERROR:
         FreeArgs(rdargs);
      }

      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else
   {
      OPENFAIL;
   }
#endif
   return(rc);
}
