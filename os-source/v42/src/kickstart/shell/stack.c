/*
lc -d -j73 -O -o/obj/Stack.o -i/include -v -csf Stack
blink /obj/Stack.o to /bin/Stack sc sd nd
protect /bin/Stack +p
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
/* Command: Stack							*/
/* Author:  John A. Toebes, VIII					*/
/* Change History:							*/
/*  Date    Person	  Action					*/
/* -------  ------------- -----------------				*/
/* 19MAR89  John Toebes   Initial Creation				*/
/* 25OCT89  John Toebes   Revamp for DOS36.21				*/
/* 02DEC89  John Toebes   Final update after code review		*/
/*									*/
/* Notes:								*/
/*  This routine must not believe the stack size given by the user	*/
/*  until it is proven to be within tolerable limits.  DOS needs a	*/
/*  certain minimum in order to function and commands need to be able	*/
/*  to allocate the stack size in order to actually run.  Hence, we	*/
/*  must ensure that we can allocate the stack space before allowing	*/
/*  the user to set it.  Otherwise we can get them in the situation	*/
/*  where they don't have enough stack space to run the stack command   */
/*  to reset the stack space... 					*/
/*----------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"

#include "global.h"

#define SMALL 1

#define TEMPLATE    "SIZE/N"
#define OPT_SIZE    0
#define OPT_COUNT   1

#define MINSTACK 1600

int cmd_stack(void)
{
   struct DosLibrary *DOSBase;
   long opts[OPT_COUNT];
   char *stackptr;
   long stacksize;
   int rc,rc2;
   struct CommandLineInterface *cli;
   struct RDArgs *rdargs;
   char buffer[48];

   rc = RETURN_ERROR;
#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL)PrintFault(IoErr(), NULL);
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
         cli = Cli();
	 rc2 = 0;

	 /*-------------------------------------------------------------*/
	 /* If they speicified a new stack size then we are to set it	*/
	 /*-------------------------------------------------------------*/
	 if (opts[OPT_SIZE])
	 {
	    /*----------------------------------------------------------*/
	    /* Figure out how much they asked for and assume it is too	*/
	    /* small to start with.					*/
	    /*----------------------------------------------------------*/
	    stacksize = *(long *)opts[OPT_SIZE];
	    rc2 = STR_STACK_SMALL;
	    rc = RETURN_ERROR;

	    /*----------------------------------------------------------*/
	    /* Check to see if we were right about it being too small	*/
	    /*----------------------------------------------------------*/
	    if (stacksize >= MINSTACK)
	    {
	       /*-------------------------------------------------------*/
	       /* We seemed to be getting there, now assume it is too	*/
	       /* big unless we can allocate that much memory first	*/
	       /*-------------------------------------------------------*/
	       rc2 = STR_STACK_LARGE;
	       if ((stackptr = AllocVec(stacksize, 0)) != NULL)
	       {
		  /*----------------------------------------------------*/
		  /* They were telling the truth.  Free up the memory	*/
		  /* and reset the return codes.			*/
		  /*----------------------------------------------------*/
		  FreeVec(stackptr);
		  rc2 = 0;
		  rc = RETURN_OK;

		  /*----------------------------------------------------*/
		  /* Store stack size into the CLI structure (if any)   */
		  /* remember that it is given in words not bytes so	*/
		  /* we will round down to the nearest longword 	*/
		  /*----------------------------------------------------*/
		  if (cli)
		     cli->cli_DefaultStack = stacksize>>2;
	       }
	    }
	 }

	 /*-------------------------------------------------------------*/
	 /* None specified, they want us to report the stack size	*/
	 /*-------------------------------------------------------------*/
	 else
	 {
	    /*----------------------------------------------------------*/
	    /* Get the stack size from the CLI structure (if any).      */
	    /* remember that it is in long words			*/
	    /*----------------------------------------------------------*/
	    stacksize = 0;
	    if (cli) stacksize = cli->cli_DefaultStack << 2;

	    /*----------------------------------------------------------*/
	    /* Tell the fall through code to display the size.		*/
	    /*----------------------------------------------------------*/
	    if(Fault(STR_CURRENT_STACK,NULL,buffer,44)) {
	        VPrintf(buffer,&stacksize);
     	        rc = RETURN_OK;
	    }
	 }

	 /*-------------------------------------------------------------*/
	 /* If any failure occured in the processing, we will get here	*/
	 /* with a pointer to a message to print.  Only in the case of	*/
	 /* reporting the current stack size will we have a substition	*/
	 /* to be done.  Since it doesn't hurt, we always use it as     */
	 /* the substitution values.					*/
	 /*-------------------------------------------------------------*/

	 if(rc)PrintFault(rc2,NULL);

	 /*-------------------------------------------------------------*/
	 /* Clean up and exit quietly					*/
	 /*-------------------------------------------------------------*/
	 FreeArgs(rdargs);
	 SetIoErr(rc2);
      }
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL;}
#endif
   return(rc);
}
