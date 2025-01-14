/*
lc -d -j73 -O -o/obj/Else.o -i/include -v -csf Else
blink /obj/Else.o to /bin/Else sc sd nd
protect /bin/Else +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 382-8265    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*	       John Toebes     Mary Ellen Toebes  Doug Walker		     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: Else							     */
/* Author:  Gordon Keener						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 19MAR89  John Toebes   Initial Creation				     */
/* 03DEC89  Gordon Keener Final (hopefully) fixes                            */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
/* #include "else_rev.h" */
#include "global.h"

#define SMALL 1

int cmd_else(void)
{
   struct DosLibrary		*DOSBase;
   int				rc = RETURN_ERROR;
   char 			buff[256];
   struct CommandLineInterface	*cli;
   int				nest;
   int                          more;

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
#else
      getargs(&DOSBase,NULL,NULL,NULL,0);
#endif
      if (((cli = Cli()) == NULL) ||
	  (cli->cli_CurrentInput == cli->cli_StandardInput)) {
	 PrintFault(STR_NOT_EXECUTE,NULL);
      }
      else {
	 /* Read lines until we find matching ENDIF */
	 SelectInput( cli->cli_CurrentInput );
	 nest = 0;

	 while (nest >= 0) {
	    if (ReadItem(buff, sizeof(buff), NULL) == ITEM_TOKEN) {
	       switch( FindArg("IF,ENDIF", buff) ) {
		  case 0:
		     /* IF */
		     nest++;
		     break;

		  case 1:
		     /* ENDIF */
		     nest--;
		     break;

		  default:
		     break;
	       }
	    }

            /* eat the rest of the line */
            while ((more = FGetC(Input())) != '\n')
            {
               if (more < 0) goto quitl;
            }
	 }


quitl:
	 /* Did we drop off the world, or discover America?	      */
	 /* NOTE: Else complains about missing ENDIFs, but If doesn't */
	 if (nest >= 0)PrintFault(STR_MISSING_ELSE,NULL);
	 else rc = RETURN_OK;
      }

      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL; }
#endif
   return(rc);
}
