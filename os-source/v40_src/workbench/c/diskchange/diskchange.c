;/*
lc -d -j73 -O -o/obj/DiskChange.o -i/include -v -csf DiskChange
blink /obj/DiskChange.o to /bin/DiskChange sc sd nd
protect /bin/DiskChange +p
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
/* Command: DiskChange							     */
/* Author:  James E. Cooper Jr. 					     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 19MAR89  John Toebes   Initial Creation				     */
/* 23MAR89  Dave Baker	  Where's the Beef                                   */
/* 14APR89  Dave Baker	  Cleanup and BUG fixes 			     */
/* 27OCT89  Dave Baker	  Conversion to 1.4				     */
/* 07DEC89  Jim Cooper	  Code cleanup & commenting, final 1.4 conversion    */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "diskchange_rev.h"

#define TEMPLATE     "DEVICE/A" CMDREV
#define OPT_DISKNAME 0
#define OPT_COUNT    1

int cmd_diskchange(void)
{
  struct Library *SysBase = (*((struct Library **) 4));
  struct DosLibrary *DOSBase;
  struct RDargs *rdargs;
  int rc;

  long opts[OPT_COUNT];
  struct MsgPort *diskdev;

  /*-------------------------------------------------------------------------*/
  /* Set the default return code.					     */
  /*-------------------------------------------------------------------------*/
  rc = RETURN_FAIL;

  /*-------------------------------------------------------------------------*/
  /* Try to open the dos.library.					     */
  /*-------------------------------------------------------------------------*/
  if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {

    /*-----------------------------------------------------------------------*/
    /* Ok, we got dos, so let's find out what the user typed.                */
    /*-----------------------------------------------------------------------*/
    memset(opts, 0, sizeof(opts));
    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if (rdargs == NULL) {

      /*---------------------------------------------------------------------*/
      /* If something bad happened in the parsing, tell the user about it    */
      /* and get out!							     */
      /*---------------------------------------------------------------------*/
      PrintFault(IoErr(),NULL);
    } else {

      /*---------------------------------------------------------------------*/
      /* See if what they gave us was a valid device.			     */
      /*---------------------------------------------------------------------*/
      if ((diskdev = DeviceProc((char *)opts[OPT_DISKNAME])) == NULL) {

	/*-------------------------------------------------------------------*/
	/* If we could NOT find the process for this thing, tell the user    */
	/* about it and fall through to exit.				     */
	/*-------------------------------------------------------------------*/
	PrintFault(IoErr(),NULL);
      } else {

	/*-------------------------------------------------------------------*/
	/* If we found the process for this device, try to inhibit it twice, */
	/* which is the recommended way to re-log a disk.		     */
	/*-------------------------------------------------------------------*/
	if ((DoPkt(diskdev, ACTION_INHIBIT,
		   DOSTRUE, NULL, NULL, NULL)) &&
	    (DoPkt(diskdev, ACTION_INHIBIT,
		   DOSFALSE, NULL, NULL, NULL))) {

	  /*-----------------------------------------------------------------*/
	  /* If everything worked, set a good return code.		     */
	  /*-----------------------------------------------------------------*/
	  rc = RETURN_OK;
	} else {

	  /*-----------------------------------------------------------------*/
	  /* If either DoPkt call failed, tell the user why and fall thru.   */
	  /*-----------------------------------------------------------------*/
	  PrintFault(IoErr(),NULL);
	}
      }

      /*---------------------------------------------------------------------*/
      /* Cleanup after the ReadArgs call.				     */
      /*---------------------------------------------------------------------*/
      FreeArgs(rdargs);
    }

    /*-----------------------------------------------------------------------*/
    /* Close the dos library.						     */
    /*-----------------------------------------------------------------------*/
    CloseLibrary((struct Library *)DOSBase);
  } else {
    OPENFAIL;
  }
  return(rc);
}

