/*
lc -d -j73 -O -o/obj/Prompt.o -i/include -v -csf Prompt
blink /obj/Prompt.o to /bin/Prompt sc sd nd
protect /bin/Prompt +p
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
/* Command: Prompt							     */
/* Author: James E. Cooper Jr.						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 25MAR89  Jim Cooper	  Initial Creation				     */
/* 13APR89  Jim Cooper	  'tweaked' code to reduce size below ARP 1.3        */
/* 28APR89  Jim Cooper	  Oops!  Added line to set result code so WHY works  */
/* 04SEP89  Jim Cooper	  Conversion to 1.4 Completed (I thought!)           */
/* 22OCT89  Jim Cooper	  Conversion to 1.4 Proceeding			     */
/* 01NOV89  Jim Cooper	  Conversion to 1.4 Complete (Really!)               */
/* 03NOV89  Jim Cooper	  Comments added, source cleaned up, FINISHED!	     */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
/* #include "prompt_rev.h" */
#include "global.h"

#define SMALL 1

#define TEMPLATE   "PROMPT"
#define OPT_PROMPT 0
#define OPT_COUNT  1

/*---------------------------------------------------------------------------*/
/* Default prompt string -> '%N> ', same as initial prompt when AmigaDOS     */
/* boots.  AmigaDOS Prompt command has default of '> ', which is not the     */
/* TRUE default.  I count this as a bug fix.				     */
/*---------------------------------------------------------------------------*/
#define DEF_STRING "%N.%S> "

int cmd_prompt(void)
{
  struct DosLibrary *DOSBase;
  int rc=RETURN_ERROR;
  UBYTE *prompt;
  long opts[OPT_COUNT];
  struct RDArgs *rdargs;


#ifndef SMALL
    DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
    memset((char *)opts, 0, sizeof(opts));
    rdargs = ReadArgs(TEMPLATE, opts, NULL);
    if (rdargs == NULL) {

      /*---------------------------------------------------------------------*/
      /* If ReadArgs() didn't like the command line, tell the user!          */
      /*---------------------------------------------------------------------*/
      PrintFault(IoErr(),NULL);

    } else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {	
#endif
      prompt = (UBYTE *)opts[OPT_PROMPT];

      /*---------------------------------------------------------------------*/
      /* If the user didn't give a prompt to use, we must give him a default.*/
      /* See the #define of DEF_STRING above.				     */
      /*---------------------------------------------------------------------*/
      if (prompt == NULL) {
	prompt = DEF_STRING;
      }

      /*---------------------------------------------------------------------*/
      /* Now ask DOS to set this as the user prompt.			     */
      /*---------------------------------------------------------------------*/
      if (!SetPrompt(prompt)) {

	/*-------------------------------------------------------------------*/
	/* If DOS didn't like the prompt, alert the user!                    */
	/*-------------------------------------------------------------------*/
	PrintFault(IoErr(),NULL);

      } else {

	/*-------------------------------------------------------------------*/
	/* If DOS said it worked, give a good return code.		     */
	/*-------------------------------------------------------------------*/
	rc = RETURN_OK;
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
