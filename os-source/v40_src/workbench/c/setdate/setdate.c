; /*
lc -d -j73 -O -o/obj/SetDate.o -i/include -v -csf SetDate
blink /obj/SetDate.o to /bin/SetDate sc sd nd
protect /bin/SetDate +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 460-7430    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*	       John Toebes     Mary Ellen Toebes  Doug Walker	Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: SetDate							     */
/* Author: James E. Cooper Jr.						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 24APR89  Jim Cooper	  Initial Creation				     */
/* 27APR89  Jim Cooper	  FINISHED!					     */
/* 03SEP89  Jim Cooper	  Converted to 1.4 dos.library (I thought!)          */
/* 30OCT89  Jim Cooper	  Conversion to 1.4 proceeding			     */
/* 04NOV89  Jim Cooper	  Comments added, code cleaned up, FINISHED!	     */
/* 08DEC89  Jim Cooper	  Rearranged code to allow more flexibility.	     */
/* 12DEC89  Jim Cooper	  Added wildcard support!			     */
/* 04MAR90  Jim Cooper	  Modified for loss of "AnchorPath.ap_Buf"...        */
/* 11OCT90  Jim Cooper	  Corrected message printing for invalid date	     */
/* 11OCT90  Jim Cooper	  Kludged around StrToDate() bug.                    */
/* 17OCT90  Jim Cooper	  Fixed "multi volumes, same name" bug, bad msg bug. */
/* 03FEB91  Jim Cooper	  Cleaned out some unnecessary code.  Fixed some     */
/*			  code which may have caused an incorrect error      */
/*			  message to be printed.			     */
/* 05APR91  Jim Cooper	  Added new keyword to TEMPLATE -- WEEKDAY to make   */
/*			  SetDate accept output of Date as input.	     */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "setdate_rev.h"

#define MSG_SETDATE_FAILED    "SetDate failed"
#define MSG_SETDATE_FAILED_DT ": Invalid DATE or TIME string!\n"

#define TEMPLATE  "FILE/A,WEEKDAY,DATE,TIME,ALL/S" CMDREV
#define OPT_FILE  0
#define OPT_WKDAY 1
#define OPT_DATE  2
#define OPT_TIME  3
#define OPT_ALL   4
#define OPT_COUNT 5

int cmd_setdate(void)
{
  struct Library *SysBase = (*((struct Library **) 4));
  struct DosLibrary *DOSBase;
  int temprc, rc, i;
  long opts[OPT_COUNT];
  struct DateTime datetime;
  struct RDargs *rdargs;
  struct AnchorPath *ua;
  BPTR oldlock, curlock;

  rc = RETURN_FAIL;
  if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
    memset((char *)opts, 0, sizeof(opts));
    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if (rdargs == NULL) {
      PrintFault(IoErr(), NULL);
    } else {

      /*---------------------------------------------------------------------*/
      /* Set up the DateTime structure, as per the datetime.h include file.  */
      /*---------------------------------------------------------------------*/
      DateStamp((struct DateStamp *)&datetime); /* get the current datestamp */
      datetime.dat_Format  = FORMAT_DOS;      /* Use DOS format of dd-mmm-yy */
      datetime.dat_Flags   = DTF_FUTURE;   /* Monday, etc. are in the future */

      /*---------------------------------------------------------------------*/
      /* In order to get the same functionality as the BCPL command, we have */
      /* to try each string the user typed as both a date and a time.  This  */
      /* gives the user the flexibility to put the options in any order.     */
      /*---------------------------------------------------------------------*/
      for (i=OPT_WKDAY; i<OPT_ALL; i++) {
	if (opts[i]) {
	    datetime.dat_StrDate = (char *)opts[i];
	    datetime.dat_StrTime = NULL;
	/*-------------------------------------------------------------------*/
	/* Now call DOS to convert the string to a valid DateStamp.	     */
	/*-------------------------------------------------------------------*/
	    if (!StrToDate((struct DateTime *)&datetime)) {
	      datetime.dat_StrTime = (char *)opts[i];
	      datetime.dat_StrDate = NULL;
	    if (!StrToDate(&datetime)) {
#if IT_WERE_A_MORE_PERFECT_WORLD

	      /*-------------------------------------------------------------*/
	      /* If StrToDate() followed the rules, it would set an IoErr()! */
	      /*-------------------------------------------------------------*/
	      PrintFault(IoErr(), MSG_SETDATE_FAILED);
#else
	      /*-------------------------------------------------------------*/
	      /* But since StrToDate() DOESN'T set an IoErr() code, we must  */
	      /* simply print a non-informative string. 		     */
	      /*-------------------------------------------------------------*/
	      PutStr(MSG_SETDATE_FAILED);
	      PutStr(MSG_SETDATE_FAILED_DT);
#endif
	      goto Cleanup2;
	    }
	  }
	}
      }

      /*---------------------------------------------------------------------*/
      /* First, try to allocate a chunk of memory to use with the pattern    */
      /* match stuff.							     */
      /*---------------------------------------------------------------------*/
      if ((ua = AllocVec(sizeof(struct AnchorPath),
			 MEMF_CLEAR|MEMF_PUBLIC)) == NULL) {
	PrintFault(IoErr(),NULL);
      } else {

	/*-------------------------------------------------------------------*/
	/* Now we have to set up the pattern match structure.		     */
	/*-------------------------------------------------------------------*/
	/* Set the flag which tells the pattern matcher to decode wild	     */
	/* cards.							     */
	/*-------------------------------------------------------------------*/
	ua->ap_Flags = APF_DOWILD;

	/*-------------------------------------------------------------------*/
	/* Tell the matcher to stop if the user presses Ctrl-C. 	     */
	/*-------------------------------------------------------------------*/
	ua->ap_BreakBits = SIGBREAKF_CTRL_C;

	/*-------------------------------------------------------------------*/
	/* Finally, call the matcher.					     */
	/*-------------------------------------------------------------------*/
	MatchFirst((char *)opts[OPT_FILE], ua);
	temprc = IoErr();

	/*-------------------------------------------------------------------*/
	/* Process all matches returned.  If 'MatchFirst()' returned an      */
	/* error, the 'while()' let's us skip all this and fall through the  */
	/* exit code.							     */
	/*-------------------------------------------------------------------*/
	while (temprc == 0) {

	  if (ua->ap_Info.fib_DirEntryType > 0 && opts[OPT_ALL]) {

	    /*---------------------------------------------------------------*/
	    /* The flag APF_DIDDIR tells us that we are 'backing out' of a   */
	    /* subdirectory... in other words, we have processed all sub     */
	    /* files of that directory and we are moving back to its	     */
	    /* parent.	If this flag is set, we need to clear it.	     */
	    /*---------------------------------------------------------------*/
	    if (!(ua->ap_Flags & APF_DIDDIR)) {

	      /*-------------------------------------------------------------*/
	      /* If we are stamping ALL files, tell the matcher to enter     */
	      /* this directory.					     */
	      /*-------------------------------------------------------------*/
	      ua->ap_Flags |= APF_DODIR;
	    }
	    ua->ap_Flags &= ~APF_DIDDIR;
	  }

	  /*-----------------------------------------------------------------*/
	  /* If we have an error OTHER than that we are out of matches, we   */
	  /* need to exit IMMEDIATELY!					     */
	  /*-----------------------------------------------------------------*/
	  if (temprc && (temprc != ERROR_NO_MORE_ENTRIES)) {
	    break;
	  }

	  curlock = DupLock(ua->ap_Current->an_Lock);
	  oldlock = CurrentDir(curlock);

	  /*-----------------------------------------------------------------*/
	  /* Now set the new datestamp on this file.			     */
	  /*-----------------------------------------------------------------*/
	  if (!(i = SetFileDate(ua->ap_Info.fib_FileName,
			       (struct DateStamp *)&datetime.dat_Stamp))) {
	    temprc = IoErr();
	  }

	  CurrentDir(oldlock);
	  UnLock(curlock);

	  if (!i) {

	    /*---------------------------------------------------------------*/
	    /* If 'SetFileDate()' returned an error, we have to get out and  */
	    /* tell the user about it!					     */
	    /*---------------------------------------------------------------*/
	    break;
	  }

	  /*-----------------------------------------------------------------*/
	  /* Now get the next file which matches the given pattern.	     */
	  /*-----------------------------------------------------------------*/
	  MatchNext(ua);
	  temprc = IoErr();
	}

	/*-------------------------------------------------------------------*/
	/* If the only error we had was that we ran out of files to stamp,   */
	/* we are doing fine!  Set the return code so the user knows	     */
	/* everything went well.					     */
	/*-------------------------------------------------------------------*/
	if (temprc == ERROR_NO_MORE_ENTRIES) {
	  rc = RETURN_OK;
	} else {
	  PrintFault(temprc,MSG_SETDATE_FAILED);

	  /*-----------------------------------------------------------------*/
	  /* If the user hit Ctrl-C, we have to warn him that he is being    */
	  /* rude!							     */
	  /*-----------------------------------------------------------------*/
	  if (temprc == ERROR_BREAK) {
	    rc = RETURN_WARN;
	  } else {

	  /*-----------------------------------------------------------------*/
	  /* If there was any other error, FAIL!!!			     */
	  /*-----------------------------------------------------------------*/
	    rc = RETURN_FAIL;
	  }

	}
Cleanup:
	/*-------------------------------------------------------------------*/
	/* We have to clean up after ourselves. 			     */
	/*-------------------------------------------------------------------*/
	MatchEnd(ua);
      }
      FreeVec(ua);
Cleanup2:
      FreeArgs(rdargs);
    }
    CloseLibrary((struct Library *)DOSBase);
  } else {
    OPENFAIL;
  }
  return(rc);
}
