/*
lc -d -j73 -O -o/obj/If.o -i/include -v -csf If
blink /obj/If.o to /bin/If sc sd nd
protect /bin/If +p
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

/*-----------------------------------------------------------------------*/
/* Command: If								 */
/* Author:  Gordon Keener						 */
/* Change History:							 */
/*  Date    Person	  Action					 */
/* -------  ------------- -----------------				 */
/* 19MAR89  John Toebes   Initial Creation				 */
/* 03DEC89  Gordon Keener final (hopefully) fixes                        */
/* Notes:								 */
/*-----------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
/* #include "if_rev.h" */
#include "global.h"

#define SMALL 1

#define TEMPLATE    "NOT/S,WARN/S,ERROR/S,FAIL/S,,EQ/K,GT/K,GE/K,VAL/S,EXISTS/K"
#define OPT_NOT 	0
#define OPT_WARN	1
#define OPT_ERROR	2
#define OPT_FAIL	3
#define OPT_FIRST	4
#define OPT_EQ		5
#define OPT_GT		6
#define OPT_GE		7
#define OPT_VAL 	8
#define OPT_EXISTS	9
#define OPT_COUNT	10


int cmd_if(void)
{
   struct DosLibrary		*DOSBase;
   struct Library		*UtilityBase;
   struct CommandLineInterface	*cli;
   int				rc=RETURN_ERROR;
   char 			buff[32];
   long 			opts[OPT_COUNT];
   struct RDArgs		*rdargs;
   char 			*first;
   char 			*second;
   LONG				firstnum;
   LONG				secondnum;
   int				more;
   int				nest;
   int				value;

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL) {
	 PrintFault(IoErr(), NULL);
      }
      else {
#else
      if(rdargs=getargs(&DOSBase,&UtilityBase,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
       if (((cli = Cli()) == NULL) ||
	  (cli->cli_CurrentInput == cli->cli_StandardInput)) {
	  PrintFault(STR_NOT_EXECUTE,NULL);
	  }
       else {
	 /* Just to hold it a bit, in case one of the return code */
	 /* options is selected.				  */

	 value = cli->cli_ReturnCode;

	 if (opts[OPT_EXISTS]) {
	    value = Lock((char *)opts[OPT_EXISTS], SHARED_LOCK);
	    /* unlocking a null is ok */
	    UnLock(value);
	 }
	 else if (opts[OPT_WARN])value = (value >= 5);
	 else if (opts[OPT_ERROR])value = (value >= 10);
	 else if (opts[OPT_FAIL])value = (value >= 20);
	 else if (first = (char *)opts[OPT_FIRST]) {   /* assign! */
	    /* Must have an "A op B" comparison. We handle environment   */
	    /* variables for "A" first, so we don't worry about that     */
	    /* later.							 */
	    /* Find our "B" for the comparison, then we use common code  */
	    /* to do the Getenv() and the (if necessary) Atol().         */
	    if (opts[OPT_EQ])
	       second = (char *)opts[OPT_EQ];
	    else if (opts[OPT_GT])
	       second = (char *)opts[OPT_GT];
	    else if (opts[OPT_GE])
	       second = (char *)opts[OPT_GE];
	    else second="";
	    if (opts[OPT_VAL]) {
	       /* Numeric comparisons. We get the numbers here, but the  */
	       /* common code below the "if" does the actual comparison. */
	       StrToLong(first, &firstnum);
	       StrToLong(second, &secondnum);
	    }
	    else {
	       /* Character comparisons. This is a bit kludgy, but  */
	       /* should cut down on code size. Basically, we get Strcmp */
	       /* to give us a value for "firstnumber", and we use 0 for */
	       /* "secondnumber". This way, we can use the same stupid   */
	       /* code below to figure out whether EQ, GT, or GE was	 */
	       /* specified.						 */
	       /* Where's stricmp() when ya need it?                     */

	       /* I should probably explain this mess. "firstnum" ends   */
	       /* up being the difference between the differing chars	 */
	       /* in the strings "first" and "second". This works because*/
	       /* I always do the subract *before* I check to see if I	 */
	       /* hit a '\0'. That way, I get a meaningful result when   */
	       /* the strings are of unequal lengths.			 */
	       do {
		  firstnum = ToUpper(*first) - ToUpper(*second);
	       } while (*first++ && *second++ && (firstnum == 0));
	       secondnum = 0;
	    }

	    /* Calculate the value, based on the desired operator. */
	    if (opts[OPT_EQ]) value = (firstnum == secondnum);
	    else if (opts[OPT_GT]) value = (firstnum >  secondnum);
	    else value = (firstnum >= secondnum); /* GE */
	 }
	 else {
	    /* If you don't give it any arguments, it takes it as FALSE! */
	    /* Why, then, can't we make it do an "IF <word>", which is   */
	    /* TRUE of the word is non-null, and FALSE otherwise. It	 */
	    /* would indeed make a simple and clean test for whether or  */
	    /* not a particular environment variable is set. Sigh....	 */
	    value = 0;
	 }

	 /* Do the "NOT" here, so we cheaply "NOT" any of the above tests*/
	 /* BTW, "IF NOT" is TRUE!! (implied 0 there, you know  */
	 if (opts[OPT_NOT])
	    value = !value;

	 /* If the value of the "If" expression is false, eat statements */
	 if (!value) {
	    /* Read lines until we find matching ELSE or ENDIF */
	    SelectInput( cli->cli_CurrentInput );
	    nest = 0;
	    more = 1;

	    while((nest >= 0) && (more > 0)) {
	       if (ReadItem(buff, sizeof(buff), NULL) == ITEM_TOKEN) {
		  switch( FindArg("IF,ELSE,ENDIF", buff) ) {
		     case 0:
			/* IF */
			nest++;
			break;

		     case 1:
			/* ELSE */
			/* Looks ugly, but gets us out of the loop fast */
			if (nest) break;
                        /* Fall through to decrement the nest value to  */
                        /* get us out of the loop.                      */

		     case 2:
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
                  if (more == -1)
                     break;
               }
	    }

	    /* Hey, "If" doesn't complain about missing else/endifs,  */
	    /* but "Else" does! Ludicrous, what? They should either   */
	    /* both complain, or neither.			      */
	    /* This code is left in and #ifed out so it can be easily */
	    /* added. Let the optimizer fix the extraneous assigns to */
	    /* rc, that's what it's there for!                        */
#if 1
	    if (nest >= 0)PrintFault(STR_MISSING_ELSE,NULL);

	    else
	    /* Currently, If *always* returns RETURN_OK here */
#endif
	    rc = RETURN_OK;
	 }
	 else {
	    /* Value is TRUE, we just continue */
	    rc = RETURN_OK;
	 }
       }
      FreeArgs(rdargs);
      }
      CloseLibrary(UtilityBase);
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL;}
#endif
   return(rc);
}
