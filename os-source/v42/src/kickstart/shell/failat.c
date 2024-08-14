/*
lc -d -j73 -O -o/obj/FailAT.o -i/include -v -csf FailAT
blink /obj/FailAT.o to /bin/FailAT sc sd nd
protect /bin/FailAT +p
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

/*----------------------------------------------------------------------*/
/* Command: FailAT							*/
/* Author:  John A. Toebes, VIII					*/
/* Change History:							*/
/*  Date    Person	  Action					*/
/* -------  ------------- -----------------				*/
/* 19MAR89  John Toebes   Initial Creation				*/
/* 25OCT89  John Toebes   Revamp for DOS36.21				*/
/* 02DEC89  John Toebes   Cleanup after code review			*/
/* Notes:								*/
/*----------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
/* #include "failat_rev.h" */
#include "global.h"

#define SMALL 1

#define TEMPLATE "RCLIM/N"
#define OPT_RCLIM 0
#define OPT_COUNT 1

int cmd_failat(void)
{
   struct DosLibrary *DOSBase;
   long opts[OPT_COUNT];

   long failatlev;
   int rc = RETURN_FAIL;
   struct CommandLineInterface *cli;
   struct RDArgs *rdargs;
   char buffer[48];

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
      /*----------------------------------------------------------------*/
      /* Locate our CLI structure (if any).  It doesn't matter because  */
      /* there is nothing we can do if it isn't there                   */
      /*----------------------------------------------------------------*/
      if ((cli = Cli()) != NULL) {
	    /*----------------------------------------------------------*/
	    /* Did they give us a limit to set? 			*/
	    /*----------------------------------------------------------*/
	    if (opts[OPT_RCLIM]) {
	       failatlev = *(long *)opts[OPT_RCLIM];
	       /*-------------------------------------------------------*/
	       /* If the number is not reasonable tell them		*/
	       /*-------------------------------------------------------*/

	       if (failatlev > 0) {
		  /*----------------------------------------------------*/
		  /* We have a valid limit, so set it in the CLI	*/
		  /*----------------------------------------------------*/
		  cli->cli_FailLevel = failatlev;
		  rc = RETURN_OK;

		  /*----------------------------------------------------*/
		  /* Everything worked, so skip the message		*/
		  /*----------------------------------------------------*/
	       }
		else PrintFault(STR_BAD_RETURN_CODE,NULL);
	    }

	    /*----------------------------------------------------------*/
	    /* They just want us to display the current limit		*/
	    /*----------------------------------------------------------*/
	    else {
	       /*-------------------------------------------------------*/
	       /* Extract the limit and set up to display it		*/
	       /*-------------------------------------------------------*/
	       failatlev = cli->cli_FailLevel;
		if(Fault(STR_FAIL_LIMIT,NULL,buffer,44)) {
		   VPrintf(buffer, &failatlev);
	           rc = RETURN_OK;
		}
	    }

	    /*----------------------------------------------------------*/
	    /* Finally we clean up and exit				*/
	    /*----------------------------------------------------------*/
	    FreeArgs(rdargs);
	 }
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
