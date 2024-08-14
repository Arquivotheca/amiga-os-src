/*
lc -d -j73 -O -o/obj/Skip.o -i/include -v -csf Skip
blink /obj/Skip.o to /bin/Skip sc sd nd
protect /bin/Skip +p
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
/* Command: Skip                                                             */
/* Author:  Gordon Keener                                                    */
/* Change History:                                                           */
/*  Date    Person        Action                                             */
/* -------  ------------- -----------------                                  */
/* 19MAR89  John Toebes   Initial Creation                                   */
/* 03DEC89  Gordon Keener Final (hopefully) fixes                            */
/* Notes:                                                                    */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
/* #include "skip_rev.h" */
#include "global.h"

#define SMALL 1

#define TEMPLATE    "LABEL,BACK/S"
#define OPT_LABEL   0
#define OPT_BACK    1
#define OPT_COUNT   2

int cmd_skip(void)
{
   struct DosLibrary            *DOSBase;
   struct RDargs                *rdargs;
   struct CommandLineInterface  *cli;
   int                          rc = RETURN_ERROR;
   long                         opts[OPT_COUNT];
   ULONG                        more;
   char                         buff[256];

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL)PrintFault(IoErr(), NULL);
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
	if ((((cli = Cli()) == NULL)) ||
	    (cli->cli_CurrentInput==cli->cli_StandardInput))
		PrintFault(STR_NOT_EXECUTE,NULL);
        else {
         SelectInput( cli->cli_CurrentInput );

         /* If they say BACK, seek backwards to the beginning of the */
         /* file, or at least as far as we can.                      */
         if (opts[OPT_BACK]) {
	    Flush(Input());
            Seek( Input(), 0, OFFSET_BEGINNING );
         }

         more = 1;

         /* Read lines until we find matching label, or ENDSKIP */
         while ((rc == RETURN_ERROR) && (more)) {
            if (ReadItem(buff, sizeof(buff), NULL) == ITEM_TOKEN) {
               switch( FindArg("LAB,ENDSKIP", buff) ) {
                  case 0:
                     /* LAB */
                     if (ReadItem(buff, sizeof(buff), NULL) != ITEM_NONE) {
			if(opts[OPT_LABEL]) {
                           if (FindArg((char *)opts[OPT_LABEL], buff) == 0)
                               rc = RETURN_OK;
			}
                     }
		     else if(!opts[OPT_LABEL])rc=RETURN_OK;
                     break;

                  case 1:
                     /* ENDSKIP */
                     rc = RETURN_WARN;
                     break;

                  default:
                     break;
               }
            }

            /* eat rest of line */
            more = (ULONG)FGets( Input(), buff, sizeof(buff) );
         }
	 if(rc==RETURN_ERROR) {
	    PrintFault(ERROR_OBJECT_NOT_FOUND,NULL);
	    SetIoErr(ERROR_OBJECT_NOT_FOUND);
	 }
	}
        FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
      }
   else {OPENFAIL;}
#endif
   return(rc);
}
