/*
lc -d -j73 -O -o/obj/Fault.o -i/include -v -csf Fault
blink /obj/Fault.o to /bin/Fault sc sd nd
protect /bin/Fault +p
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
/* Command: Fault							 */
/* Author:  John G. Mainwaring						 */
/* Change History:							 */
/*  Date    Person	   Action					 */
/* -------  -------------  -----------------				 */
/* 19MAR89  John Toebes    Initial Creation				 */
/* 27SEP89 John Mainwaring first implementation 			 */
/* 02DEC89 John Mainwaring zero ops[] before calling ReadArgs()          */
/* Notes:								 */
/*-----------------------------------------------------------------------*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
/* #include "fault_rev.h" */
#include "global.h"

#define SMALL 1

/* note no newline at end of string - there will be more on same line	 */
#define MSG_FAULT  "Fault %3ld"

#define TEMPLATE "/N/M"
#define OPT_NUMBER	0
#define OPT_COUNT 1

int cmd_fault(void)
{
   struct DosLibrary *DOSBase;
   LONG opts[OPT_COUNT];
   int faultcode = 0;
   int rc = RETURN_ERROR;
   long **argptr;
   long *msg;
   struct RDArgs *rdargs;

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL) {
	 PrintFault(faultcode, NULL); 	 /* user blew it - tell him why */
      }
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
	rc = RETURN_OK;

	 /* ReadArgs put numbers into some of the opts and left the rest */
	 /* alone.  Loop till we get to a NULL entry in opts.		 */
	 if(opts[OPT_NUMBER]) {
	   argptr = (long **) opts[OPT_NUMBER];
	   while (msg = *argptr++) {
	        rc = RETURN_WARN; /* if set to OK, the faultcode will get reset */
		faultcode = *(long *)msg;
	        /* Our real reason for existance - print a fault code	 */
	        /* followed by the standard Dos descriptive text.		 */
	        VPrintf(MSG_FAULT, &faultcode);
	        PrintFault(faultcode, ""); /* Let PrintFault() print the ': '*/
	    }
	 }
	 FreeArgs(rdargs);
      }
      /* One function of Fault is to set the error code   */
      /* to the last value supplied in the command line, now found in	 */
      /* faultcode.  */
      SetIoErr(faultcode);
      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else {
      OPENFAIL;
   }
#endif
   return(rc);
}
