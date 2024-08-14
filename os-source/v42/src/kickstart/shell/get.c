/*
lc -d -j73 -O -o/obj/GetEnv.o -i/include -v -csf GetEnv
blink /obj/GetEnv.o to /bin/GetEnv sc sd nd
protect /bin/GetEnv +p
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
/* Command: GetEnv							*/
/* Author:  John Toebes 						*/
/* Change History:							*/
/*  Date    Person	  Action					*/
/* -------  ------------- -----------------				*/
/* 19MAR89  John Toebes   Initial Creation				*/
/* 25OCT89  John Toebes   Revamp for DOS36.21				*/
/* 06NOV89  John Toebes   Change to use GetVar				*/
/* 02DEC89  John Toebes   Final changes from code review		*/
/* Notes:								*/
/*----------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
#include "dos/var.h"
#include "global.h"
/* #include "getenv_rev.h" */

#define SMALL 1

#define TEMPLATE    "NAME/A"
#define OPT_NAME    0
#define OPT_COUNT   1

int cmd_getenv(void)
{
   struct DosLibrary *DOSBase;
   struct Library *UtilityBase;
   int rc=RETURN_ERROR;
   int mode;
   int len;
   char envbuf[ENVMAX+1];	/* One extra for a final \n we add	*/
   long opts[OPT_COUNT];
   struct RDArgs *rdargs;
   UBYTE buf[80];
  
#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL) PrintFault(IoErr(), NULL);
      else {
#else
      if(rdargs=getargs(&DOSBase,&UtilityBase,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
     memset((char *)envbuf, 0, sizeof(envbuf));
	 /*-------------------------------------------------------------*/
	 /* Determine whether to set it as a local or global variable.	*/
	 /* unless they specify local we assume global.  Note that we	*/
	 /* do not have to check the global flag at all for this.  This */
	 /* does allow them to (erroneously) specify both LOCAL GLOBAL  */
	 /* and we will treat it as LOCAL				*/
	 /*-------------------------------------------------------------*/
	 mode = GVF_LOCAL_ONLY|LV_VAR;

	 if(GetProgramName(buf,31)) { /* pick the defaults */
	     if(!Stricmp((UBYTE *)FilePart(buf),"getenv"))
		mode = GVF_GLOBAL_ONLY|LV_VAR;
	 }

	 /*-------------------------------------------------------------*/
	 /* Ask DOS to find the variable for us.			*/
	 /*-------------------------------------------------------------*/
	 len = GetVar((char *)opts[OPT_NAME], envbuf, ENVMAX, mode);

	 /*-------------------------------------------------------------*/
	 /* Did he find it?						*/
	 /*-------------------------------------------------------------*/
	 if (len == -1) {
	    /*----------------------------------------------------------*/
	    /* Nope, tell the user, making sure that we preserve the	*/
	    /* secondary return code for the fault command.		*/
	    /*----------------------------------------------------------*/
	    PrintFault(IoErr(),NULL);
	    rc = RETURN_WARN;
	 }
	 else {
	    rc = RETURN_OK;	/* assume things will work out ok	*/
	    len = strlen(envbuf);
	    /*----------------------------------------------------------*/
	    /* Check to see if we got a truncated result back		*/
	    /*----------------------------------------------------------*/
	    if (len == ENVMAX)
	    {
	       /*-------------------------------------------------------*/
	       /* Oh well, tell them we only can handle so many bytes	*/
	       /*-------------------------------------------------------*/
	       rc = RETURN_WARN;
	    }

	    /*----------------------------------------------------------*/
	    /* Add a CR so we get a blank line after it 		*/
	    /* We rely upon the buffer being one byte bigger for this	*/
	    /*----------------------------------------------------------*/
	    if (envbuf[len-1] != '\n') envbuf[len++] = '\n';
	    WriteChars(envbuf, len);
	 }
	 /*-------------------------------------------------------------*/
	 /* Now clean up and exit.					*/
	 /*-------------------------------------------------------------*/
	 FreeArgs(rdargs);
      }
      CloseLibrary(UtilityBase);
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL; }
#endif
   return(rc);
}
