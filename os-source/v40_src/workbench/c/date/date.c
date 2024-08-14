; /*
lc -d -j73 -O -o/obj/date.o -i/include -v -csf date
blink /obj/date.o to /bin/date sc sd nd
protect /bin/date +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 382-8265    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*	       John Toebes     Mary Ellen Toebes  Doug Walker	Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: Date							     */
/* Author:  Andrew N. Mercier, Jr.					     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 19MAR89  John Toebes   Initial Creation				     */
/* 09APR89  Andy Mercier  Created Parser				     */
/* 29OCT89  Andy Mercier  Update to DOS V1.4				     */
/* 05DEC89  Andy Mercier  Code completed and tested			     */
/* 23MAY90  Andy Mercier  Cleaned up return codes from DOS functions         */
/* 02NOV90  John Toebes   Cleaned up an IO_Torture warning                   */
/*									     */
/* Notes:  Future enhancement can just show the day of the week, BUT not     */
/*	   change the system date...using just a switch.		     */
/*									     */
/*	   The original command has an interesting property.  If you type... */
/*									     */
/*			     date tomorrow tomorrow			     */
/*	   you will set the date forward two days.  You can only type two    */
/*	   otherwise you'll get bad args.                                          */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include <devices/timer.h>
#include "date_rev.h"

#define TICKSPERSEC    50
#define SECS_IN_DAY 86400
#define SECS_IN_MIN    60

#define MSG_BAD_HDR   "***Bad args\n"
#define MSG_BAD_ARGS  "\
- use DD-MMM-YY or <dayname> or yesterday etc. to set date\n\
      HH:MM:SS OR HH:MM to set time\n"

#define TEMPLATE    "DAY,DATE,TIME,TO=VER/K" CMDREV
#define OPT_DAY	    0
#define OPT_DATE    1
#define OPT_TIME    2
#define OPT_TO	    3
#define OPT_COUNT   4

int cmd_date(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   struct Library *UtilityBase;
   int rc, option;
   LONG parray[3];
   char *msg;
   LONG opts[OPT_COUNT];
   char day_of_week[LEN_DATSTRING];
   char date_str[LEN_DATSTRING];
   char time_str[LEN_DATSTRING];

   struct DateTime dt;

   struct timerequest tr;

   struct RDargs *rdargs;

   #define msgblock tr.tr_node.io_Message

   BPTR out_file;
   rc = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *) OpenLibrary(DOSLIB, DOSVER)) &&
       (UtilityBase = OpenLibrary("utility.library",0))) {
      memset(opts, 0, sizeof(opts));
      if ((rdargs = ReadArgs(TEMPLATE, opts, NULL)) == NULL) {
	 PrintFault(IoErr(), NULL);
	 PutStr(MSG_BAD_ARGS);
      }
      else {
	 msg = "%s %s %s\n";
	 rc = RETURN_OK;

	 if (opts[OPT_TIME] || opts[OPT_DATE] || opts[OPT_DAY]) {
	    /*-----------------------------------------------------------*/
	    /* Get the timer device so we can set the date/time 	 */
	    /*-----------------------------------------------------------*/
            if (OpenDevice(TIMERNAME, UNIT_MICROHZ, &tr, 0)) {
               rc = RETURN_FAIL;
            }
	    msgblock.mn_Node.ln_Type = NT_MESSAGE;
	    msgblock.mn_Node.ln_Pri = 0;
	    msgblock.mn_Node.ln_Name = NULL;
	    msgblock.mn_ReplyPort = NULL;

	    /*-----------------------------------------------------------*/
	    /* Read the current date/time				 */
	    /*-----------------------------------------------------------*/
	    DateStamp((long *) &dt.dat_Stamp);
	    dt.dat_Format = FORMAT_DOS;
	    dt.dat_Flags = DTF_FUTURE;

	    /*-----------------------------------------------------*/
	    /* Date & time getting set, don't want any output      */
	    /*-----------------------------------------------------*/
	    if ((char *) opts[OPT_TO] == NULL) {
	       msg = NULL;
	    }
	    for (option = OPT_DAY; option <= OPT_TIME; option++) {
	       dt.dat_StrDate = (char *) opts[option];
		  dt.dat_StrTime = NULL;
	       /*-------------------------------------------------------------*/
	       /* Now call DOS to convert the string to a valid DateStamp.    */
	       /*-------------------------------------------------------------*/
	       if (!StrToDate((struct DateTime *) &dt)) {
		  dt.dat_StrTime = (char *) opts[option];
		  dt.dat_StrDate = NULL;
		  if (!StrToDate((struct DateTime *) &dt)) {
                     rc = RETURN_FAIL;
                     break;
                  }
	       }
	       if (IoErr()) {
                  rc = RETURN_FAIL;
                  break;
               }
	    }
	    if (rc) {
	       /*----------------------------------------------------------*/
	       /* Warning...Didn't set the Date/Time totally               */
	       /*----------------------------------------------------------*/
	       if (option == OPT_TIME) {
		   rc = RETURN_WARN;
	       }
	       else {
		   rc = RETURN_FAIL;
		   msg = NULL;
	       }
	       PutStr(MSG_BAD_HDR MSG_BAD_ARGS);
	    }
	    else {
               /* Keep IO_Torture happy */
               tr.tr_node.io_Message.mn_Node.ln_Type  = NT_UNKNOWN;
	       tr.tr_node.io_Command = TR_SETSYSTIME;
	       tr.tr_time.tv_secs = UMult32(dt.dat_Stamp.ds_Days, SECS_IN_DAY)
				    + UMult32(dt.dat_Stamp.ds_Minute, SECS_IN_MIN)
				    + UDivMod32(dt.dat_Stamp.ds_Tick, TICKSPERSEC);
	       tr.tr_time.tv_micro = 0;
	       if (DoIO(&tr))
                  rc = RETURN_FAIL;
	    }

	    /*----------------------------------------------------------------*/
	    /* All done with our timer device				      */
	    /*----------------------------------------------------------------*/
	    CloseDevice(&tr);
	 }

	 memset((char *)&dt, 0, sizeof(struct DateTime));
	 DateStamp((long *) &dt.dat_Stamp);
	 dt.dat_Format = FORMAT_DOS;
	 dt.dat_Flags = NULL;
	 dt.dat_StrDay = day_of_week;
	 dt.dat_StrDate = date_str;
	 dt.dat_StrTime = time_str;

	 /*----------------------------------------------------------------*/
	 /* Convert the date/time structure to a string for output either  */
	 /* to the screen or to the output file 			   */
	 /*----------------------------------------------------------------*/
	 if (!DateToStr(&dt))
            rc = RETURN_FAIL;
	 parray[0] = (LONG) dt.dat_StrDay;
	 parray[1] = (LONG) dt.dat_StrDate;
	 parray[2] = (LONG) dt.dat_StrTime;

	 if (((char *) opts[OPT_TO] != NULL) && (rc != RETURN_FAIL)) {
	    if ((out_file = Open((char *) opts[OPT_TO], MODE_NEWFILE)) == NULL) {
	       PutStr((char *) opts[OPT_TO]);
	       PrintFault(IoErr(), "");
	       if( (opts[OPT_DAY]) || (opts[OPT_DATE]) || (opts[OPT_TIME])) {
		  rc = RETURN_WARN;	/* I just couldn't send to the file
					   but the time and date may have been
					   set */
	       }
	       else rc = RETURN_FAIL;
	       msg = NULL;
	    }
	    else {
	       VFPrintf(out_file, msg, parray);
	       Close(out_file);
	       msg = NULL;
	    }	/* End Else for Open file */
	 }	/* End If that checks for the OPT_TO */

	 if (msg) {
	    VPrintf(msg, parray);
	 }
	 FreeArgs(rdargs);
      } 	/* End of Else for good readargs */

      CloseLibrary(UtilityBase);
      CloseLibrary((struct Library *) DOSBase);
   }	/* End of If for open lib */
   else { OPENFAIL;}
   return(rc);
}

#if 0
int aprintf( fmt, args )
char *fmt, *args;
{
struct Library *SysBase = (*((struct Library **) 4));
   struct Library *DOSBase; 
   DOSBase=OpenLibrary("dos.library",36);
   VFPrintf( Output(), fmt, (LONG *)&args );
   CloseLibrary(DOSBase);
   return(0);
}
#endif
