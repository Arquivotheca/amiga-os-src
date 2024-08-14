; /*
lc -d -j73 -O -o/obj/Lock.o -i/include -v -csf Lock
blink /obj/Lock.o to /bin/Lock sc sd nd
protect /bin/Lock +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 382-8265    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: Lock							     */
/* Author:  Dave F. Baker						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 19MAR89  John Toebes   Initial Creation				     */
/* 24MAR89  Dave Baker	  It takes guts 				     */
/* 27OCT89  Dave Baker	  Conversion to 1.4				     */
/* 07DEC89  Jim Cooper	  Final code cleanup & commenting		     */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "lock_rev.h"

#define MSG_LOCKED   "%s locked\n"
#define MSG_UNLOCKED "%s unlocked\n"
#define MSG_NOLOCK "Attempt to lock drive %s failed\n"

#define TEMPLATE  "DRIVE/A,ON/S,OFF/S,PASSKEY" CMDREV
#define OPT_DRIVE 0
#define OPT_ON	  1
#define OPT_OFF   2
#define OPT_PKEY  3
#define OPT_COUNT 4

#define BUF_SIZE  31

int cmd_lock(void)
{
  struct Library *SysBase = (*((struct Library **) 4));
  struct DosLibrary *DOSBase;
  struct RDargs *rdargs;
  int rc;
  int rc2;
  int status;
  long opts[OPT_COUNT];
  long myargs0;
  long myargs1;
  struct MsgPort *driveproc;
  long pos;

  /*-------------------------------------------------------------------------*/
  /* Set up default return code.					     */
  /*-------------------------------------------------------------------------*/
  rc = RETURN_FAIL;

  /*-------------------------------------------------------------------------*/
  /* Can we open the dos library?					     */
  /*-------------------------------------------------------------------------*/
  if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {

    /*-----------------------------------------------------------------------*/
    /* Ok, dos was home.  Initialize the options for the ReadArgs call.      */
    /*-----------------------------------------------------------------------*/
    memset((char *)opts, 0, sizeof(opts));
    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if (rdargs == NULL) {

      /*---------------------------------------------------------------------*/
      /* If there was an error parsing the command line, tell user and EXIT! */
      /*---------------------------------------------------------------------*/
      PrintFault(IoErr(), NULL);
    } else {

      /*-------------------------------------------------------------------*/
      /* If they didn't type ON, default to OFF.                           */
      /*-------------------------------------------------------------------*/
      myargs0 = DOSFALSE;

      /*---------------------------------------------------------------------*/
      /* See if they typed in ON or OFF.				     */
      /*---------------------------------------------------------------------*/
      if (opts[OPT_ON]) {

	/*-------------------------------------------------------------------*/
	/* If they typed ON, FORCE it, no matter what else they typed.	     */
	/*-------------------------------------------------------------------*/
	myargs0 = DOSTRUE;
      }
      myargs1 = pos = NULL;

      /*---------------------------------------------------------------------*/
      /* See if they typed in a PassKey.				     */
      /*---------------------------------------------------------------------*/

      if (opts[OPT_PKEY]) {

	/*-------------------------------------------------------------------*/
	/* Ok, so they want security.  Copy the supplied passkey into my     */
	/* argument #1, so we can send it to dos.			     */
	/*-------------------------------------------------------------------*/
	while (((char *)opts[OPT_PKEY])[pos]) {
	   /*-----------------------------------------------------------*/
	   /* Yes, this does a multiply by 10 as (x*4+x)*2 = (x*5)*2    */
	   /* It saves the space of using utility.library or even the	*/
	   /* Multiply routine from the lattice libraries		*/
	   /*-----------------------------------------------------------*/
	   myargs1 = ((myargs1 << 2) + myargs1) << 1;
	   myargs1 += ((char *)opts[OPT_PKEY])[pos++];
	}
      }

      if ((driveproc = DeviceProc((char *)opts[OPT_DRIVE])) == NULL) {

	/*-------------------------------------------------------------------*/
	/* If DeviceProc() returns a NULL, dos couldn't find the specified   */
	/* device/volume, even after prompting the user.		     */
	/*-------------------------------------------------------------------*/
	PrintFault(IoErr(),NULL);
      } else {

	/*-------------------------------------------------------------------*/
	/* Ok, everything worked so far.  We have the device/volume, a	     */
	/* password (if any), and we are ready to ask the handler to         */
	/* write-protect this device.					     */
	/*-------------------------------------------------------------------*/
	if (!(status = DoPkt(driveproc, ACTION_WRITE_PROTECT,
					   myargs0, myargs1, 0, 0))) {

	  /*-----------------------------------------------------------------*/
	  /* Oops!  The handler didn't recognise an ACTION_WRITE_PROTECT     */
	  /* packet, or something else equally catastrophic happened.  Bail  */
	  /* out after telling the user what went wrong.		     */
	  /*-----------------------------------------------------------------*/
	  rc2 = IoErr();
	  if(ErrorReport(rc2, REPORT_LOCK, NULL, driveproc))
		VPrintf(MSG_NOLOCK,(long *)&opts[OPT_DRIVE]);
	  }

	 /*-----------------------------------------------------------------*/
	 /* Tell the user we Locked/Unlocked his device.  Incidentally, the */
	 /* BCPL Lock command would simply return without a message of any  */
	 /* kind if it succeeded in unlocking, whereas it printed a message */
	 /* of some sort if it locked!	I like to see messages both ways,   */
	 /* so, Voila!						     */
	 /*-----------------------------------------------------------------*/
	if (status)
	{
	  VPrintf(((myargs0) ? MSG_LOCKED : MSG_UNLOCKED),
		  (long *)&opts[OPT_DRIVE]);
	  rc = RETURN_OK;
	}
      }
      FreeArgs(rdargs);
    }
    CloseLibrary((struct Library *)DOSBase);
  } else {
    OPENFAIL;
  }
  return(rc);
}

