/*
lc -d -j73 -O -o/obj/Why.o -i/include -v -csf Why
blink /obj/Why.o to /bin/Why sc sd nd
protect /bin/Why +p
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

/*---------------------------------------------------------------------------*/
/* Command: Why                                                              */
/* Author:  John G. Mainwaring                                               */
/* Change History:                                                           */
/*  Date    Person         Action                                            */
/* -------  -------------  -----------------                                 */
/* 19MAR89  John Toebes    Initial Creation                                  */
/* 28SEP89 John Mainwaring first implementation                              */
/* 30NOV89 John Mainwaring symbolic RETURN_OK, new copyright notice          */
/* Notes:                                                                    */
/*---------------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
#include "global.h"

#define SMALL 1

#define TEMPLATE    ""
#define OPT_DUMMY   0
#define OPT_COUNT   1

int cmd_why(void)
{
   struct DosLibrary *DOSBase;
   int rc = RETURN_ERROR;
   long faultcode;
   long opts[OPT_COUNT];
   struct RDArgs *rdargs;
   struct CommandLineInterface *cli;
   char buffer[48];

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL) PrintFault(IoErr(), NULL);
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
         /* Need original fault code, IoErr() gets it from somewhere else. */
         cli = Cli();
         faultcode = cli->cli_Result2;

         if (faultcode) {
	    Fault(STR_LAST_MSG_FAILED,NULL,buffer,44);
            PrintFault(faultcode, buffer);
         }
	 else PrintFault(STR_NO_RETURN_CODE,NULL);
         FreeArgs(rdargs);

         /* This result finds its way back to cli->Result2 - do this so 'Why' */
         /* won't stomp the fault code, which would be a no-no.               */
        SetIoErr(faultcode);
	rc = RETURN_WARN ; /* if RC is set to 0, IoErr gets reset */
      }
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL;}
#endif
   return(rc);
}
