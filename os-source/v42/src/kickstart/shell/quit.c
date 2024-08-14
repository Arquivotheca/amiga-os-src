/*
lc -d -j73 -O -o/obj/Quit.o -i/include -v -csf Quit
blink /obj/Quit.o to /bin/Quit sc sd nd
protect /bin/Quit +p
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
/* Command: Quit							     */
/* Author:  Gordon Keener						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 19MAR89  John Toebes   Initial Creation				     */
/* 03DEC89  Gordon Keener Final (hopefully) fixes                            */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
/* #include "quit_rev.h" */
#include "global.h"

#define SMALL 1

#define TEMPLATE    "RC/N"
#define OPT_COUNT   1

int cmd_quit(void)
{
   struct DosLibrary		*DOSBase;
   struct CommandLineInterface	*cli;
   int				rc=RETURN_ERROR;
   long 			opts[OPT_COUNT];
   long				faultcode;
   struct RDArgs		*rdargs;
   char 			eat[MSGBUFSIZE]; /* convenient size */

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)PrintFault(IoErr(), NULL);
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
	 if(opts[0])rc = *(long *)opts[0];
	 else rc=RETURN_OK;

	 if (rc >= 0) {
	    /* Eat the rest of the execute script, if we're in one     */
	    /* NOTE: Quit doesn't complain about not being in a script */
	    /* It also doesn't "quit" if it doesn't like its argument  */
	    if((cli = Cli())) {
	        faultcode=cli->cli_Result2; /* reset fault code */
	        if(cli->cli_CurrentInput != cli->cli_StandardInput) {
	           SelectInput( Cli()->cli_CurrentInput );
	           while( FGets(Input(), eat, sizeof(eat)));
		}
	    }
	 }
	 else {
	    faultcode=120;
	    PrintFault(120,NULL); /* argument invalid */
	    rc = RETURN_ERROR;
	 }
      FreeArgs(rdargs);
      SetIoErr(faultcode);
      }
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL; }
#endif
   return(rc);
}

