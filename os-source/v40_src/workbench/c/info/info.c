
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
/* Command: Info							     */
/* Author:  Dave F. Baker						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 19MAR89  John Toebes   Initial Creation				     */
/* 26MAR89  Dave Baker	  Don't call me gutless                              */
/* 07DEC89  Jim Cooper	  Conversion to 1.4, Code Cleanup, Commenting	     */
/* 22FEB90  Jim Cooper	  Final code changes.				     */
/* 11OCT90  Jim Cooper	  Made Info let go of DosList Lock sooner.	     */
/* 18OCT90  Jim Cooper	  Finally updated to use latest include files.	     */
/* 10DEC90  Jim Cooper	  Fixed percentage printing, unfortunately, it	     */
/*			  slowed the code down a bit.			     */
/* 02JAN91  Jim Cooper	  Fixed to show "unreadable" if disk in drive is not */
/*			  one of the valid types.  Used to show "No disk"    */
/*			  unless the disk was specifically marked "BAD\0".   */
/* 28JAN91  Jim Cooper	  Fixed to use Utility library again.  A little bit  */
/*			  bigger, but MUCH faster!  Also, rearranged code a  */
/*			  bit for performance' sake.                         */
/* 06FEB91  Jim Cooper	  FINALLY!!!  Made the device/volume names work!!!   */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#include <internal/commands.h>
#include <libraries/filehandler.h>
#include <dos.h>
#include <string.h>
#include "info_rev.h"

#define MSG_VOL_MOUNTED     "[Mounted]"
#define MSG_DEV_HEADER	    "\nMounted disks:\n\
Unit	  Size	  Used	  Free Full Errs   Status   Name\n"
#define MSG_VOL_HEADER	    "\nVolumes available:\n"
#define MSG_PROT_READ_ONLY  "Read Only"
#define MSG_PROT_VALIDATING "Validating"
#define MSG_PROT_READ_WRITE "Read/Write"
#define MSG_TYPE_NONE	    "No disk present"
#define MSG_TYPE_BAD	    "Unreadable disk"
#define MSG_TYPE_KICK	    "Kickstart disk"
#define MSG_TYPE_NDOS	    "Not a DOS disk"
/* 28-Jan-91  Moved these here because other countries' "Kilo" or "Mega"
   may start with different letters. */
#define MSG_KILOBYTES	    "K"
#define MSG_MEGABYTES	    "M"

#define TEMPLATE  "DEVICE" CMDREV
#define OPT_DRIVE 0
#define OPT_COUNT 1

/*---------------------------------------------------------------------------*/
/* Private structure for keeping track of DosList entries.		     */
/*---------------------------------------------------------------------------*/
struct DosListKeeper {
  struct DosListKeeper *next;
  struct DosList doslist;
  struct InfoData id;
};

/* because <dos/dos.h> has these incorrectly defined in 2.04 */
#ifdef ID_INTER_DOS_DISK
#undef ID_INTER_DOS_DISK
#endif

#ifdef ID_INTER_FFS_DISK
#undef ID_INTER_FFS_DISK
#endif

#define ID_INTER_DOS_DISK (0x444F5302L)	/* 'DOS\2' */
#define ID_INTER_FFS_DISK (0x444F5303L)	/* 'DOS\3' */
#define ID_DC_DOS_DISK    (0x444F5304L)	/* 'DOS\4' */
#define ID_DC_FFS_DISK    (0x444F5305L)	/* 'DOS\5' */


/*---------------------------------------------------------------------------*/
/* Global structure is used to pass info between the two routines.	     */
/*---------------------------------------------------------------------------*/
struct Global {
  struct DosLibrary *DOSBase;
  struct Library *UtilityBase;
};

void PrintInfo(struct Global *, struct InfoData *, struct DosList *, BOOL);

#define DOSBase global.DOSBase
#define UtilityBase global.UtilityBase

int cmd_info(void)
{
  struct Library *SysBase = (*((struct Library **) 4));
  struct Global global;
  long opts[OPT_COUNT];
  int i, rc, stopnow=0, printit=0;
  char *ptr;
  struct DosList *dlist, *tlist, *device, *volume;
  struct DosListKeeper *dkhead, *dktail, *dktemp;
  struct RDargs *rdargs;
  struct InfoData *id;

  memset((char *)&global, 0, sizeof(global));
  rc = RETURN_FAIL;
  dkhead = NULL;

  /*-------------------------------------------------------------------------*/
  /* I need the DOS library and the Utility library.			     */
  /*-------------------------------------------------------------------------*/
  if (((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)) != NULL) &&
      ((UtilityBase = OpenLibrary("utility.library", DOSVER)) != NULL)) {

    memset((char *)opts, 0, sizeof(opts));
    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    /*-----------------------------------------------------------------------*/
    /* If ReadArgs() had a problem, we need to get out.                      */
    /*-----------------------------------------------------------------------*/
    if (rdargs == NULL) {
      PrintFault(IoErr(), NULL);
    }
    else {

      /*---------------------------------------------------------------------*/
      /* Set aside a buffer to use while walking the DosList.		     */
      /*---------------------------------------------------------------------*/
      if ((id = AllocVec(sizeof(struct InfoData),
			 MEMF_CLEAR|MEMF_PUBLIC)) == NULL) {
	PrintFault(IoErr(),NULL);
      } else {
	if (opts[OPT_DRIVE] == NULL) {

	  /*-----------------------------------------------------------------*/
	  /* If the user didn't specify a specific device, do 'em all!       */
	  /*-----------------------------------------------------------------*/
	  dlist = LockDosList(LDF_DEVICES|LDF_READ);
	  tlist = dlist;
	  while (tlist = NextDosEntry(tlist,LDF_DEVICES|LDF_READ)) {

	    /*---------------------------------------------------------------*/
	    /* Andy added this so we can get out of the middle of the loop.  */
	    /* Not compatible with the old Info, but... :-)		     */
	    /*---------------------------------------------------------------*/
	    if (CheckSignal(SIGBREAKF_CTRL_C)) {
	      stopnow=TRUE;
	      PrintFault(ERROR_BREAK,NULL);
	      break;
	    }
	    if (tlist->dol_Task && DoPkt4(tlist->dol_Task, ACTION_DISK_INFO,
					 MKBADDR(id), NULL, NULL, NULL)) {

	      /*-------------------------------------------------------------*/
	      /* Ok, we found one.  Now we have to save it!  Can't hog the   */
	      /* DosList while we print, dontcha know.			     */
	      /*-------------------------------------------------------------*/
	      if ((dktemp = AllocVec(sizeof(struct DosListKeeper),
				     MEMF_CLEAR|MEMF_PUBLIC)) == NULL) {
		PrintFault(IoErr(),NULL);
		break;
	      }

	      /*-------------------------------------------------------------*/
	      /* Make a copy of the DosList for later use.		     */
	      /*-------------------------------------------------------------*/
	      memcpy((char *)&dktemp->doslist,(char *)tlist,
		     sizeof(struct DosList));

	      /*-------------------------------------------------------------*/
	      /* Also need a copy of the InfoData structure.		     */
	      /*-------------------------------------------------------------*/
	      memcpy((char *)&dktemp->id,(char *)id,sizeof(struct InfoData));

	      /*-------------------------------------------------------------*/
	      /* Add it to our private list for later printing. 	     */
	      /*-------------------------------------------------------------*/
	      if (dkhead == NULL) {
		dkhead = dktail = dktemp;
	      } else {
		dktail->next = dktemp;
		dktail = dktemp;
	      }
	    }
	  }

	  /*-----------------------------------------------------------------*/
	  /* Give up the list until we have printed this section.	     */
	  /*-----------------------------------------------------------------*/
	  UnLockDosList(LDF_DEVICES|LDF_READ);

	  /*-----------------------------------------------------------------*/
	  /* Print header for this section.				     */
	  /*-----------------------------------------------------------------*/
	  PutStr(MSG_DEV_HEADER);

	  /*-----------------------------------------------------------------*/
	  /* Walk the list, printing info for each device we found above.  I */
	  /* know we could have inconsistent data (if somebody popped out a  */
	  /* disk since we saved this, for instance), but this is all we     */
	  /* can do if we don't want to block.                               */
	  /*-----------------------------------------------------------------*/
	  while (dkhead) {
	    PrintInfo(&global, (struct InfoData *)&dkhead->id,
		      (struct DosList *)&dkhead->doslist, 0);
	    dktemp = dkhead->next;
	    FreeVec(dkhead);
	    dkhead = dktemp;
	  }

	  /*-----------------------------------------------------------------*/
	  /* If they didn't hit Ctrl-C, do the next section.                 */
	  /*-----------------------------------------------------------------*/
	  if (!stopnow) {
	    dlist = LockDosList(LDF_VOLUMES|LDF_READ);
	    tlist = dlist;
	    while (tlist = NextDosEntry(tlist,LDF_VOLUMES|LDF_READ)) {

	      /*-------------------------------------------------------------*/
	      /* Same thing as above; save things for later.		     */
	      /*-------------------------------------------------------------*/
	      if ((dktemp = AllocVec(sizeof(struct DosListKeeper),
				     MEMF_CLEAR|MEMF_PUBLIC)) == NULL) {
		PrintFault(IoErr(),NULL);
		break;
	      }

	      /*-------------------------------------------------------------*/
	      /* Copy necessary data... 				     */
	      /*-------------------------------------------------------------*/
	      memcpy((char *)&dktemp->doslist,(char *)tlist,
		     sizeof(struct DosList));
	      memcpy((char *)&dktemp->id,(char *)id,sizeof(struct InfoData));

	      /*-------------------------------------------------------------*/
	      /* ...and add it to the list.				     */
	      /*-------------------------------------------------------------*/
	      if (dkhead == NULL) {
		dkhead = dktail = dktemp;
	      } else {
		dktail->next = dktemp;
		dktail = dktemp;
	      }
	    }

	    /*---------------------------------------------------------------*/
	    /* Let go of the DosList.					     */
	    /*---------------------------------------------------------------*/
	    UnLockDosList(LDF_VOLUMES|LDF_READ);

	    /*---------------------------------------------------------------*/
	    /* Print section header...					     */
	    /*---------------------------------------------------------------*/
	    PutStr(MSG_VOL_HEADER);

	    /*---------------------------------------------------------------*/
	    /* ...and everything we have collected in the list. 	     */
	    /*---------------------------------------------------------------*/
	    while (dkhead) {
	      PrintInfo(&global, (struct InfoData *)&dkhead->id,
			(struct DosList *)&dkhead->doslist, 1);
	      dktemp = dkhead->next;
	      FreeVec(dkhead);
	      dkhead = dktemp;
	    }
	  }
	} else {

	  /*-----------------------------------------------------------------*/
	  /* Ok, they DID ask for a specific device.			     */
	  /*-----------------------------------------------------------------*/
	  dlist = LockDosList(LDF_DEVICES|LDF_VOLUMES|LDF_READ);

	  /*-----------------------------------------------------------------*/
	  /* We have to have the device name WITHOUT the trailing ':'.       */
	  /*-----------------------------------------------------------------*/
	  for (ptr=(char *)opts[OPT_DRIVE],i=0;
		i<strlen((char *)opts[OPT_DRIVE]);
		ptr++,i++) {
	    if (*ptr == ':') {
	      *ptr = 0;
	      break;
	    }
	  }

	  /*-----------------------------------------------------------------*/
	  /* Look for it...						     */
	  /*-----------------------------------------------------------------*/
	  if (tlist = FindDosEntry(dlist,(char *)opts[OPT_DRIVE],
				   LDF_DEVICES|LDF_VOLUMES|LDF_READ)) {
	    if (tlist->dol_Task && DoPkt4(tlist->dol_Task, ACTION_DISK_INFO,
					 MKBADDR(id), NULL, NULL, NULL)) {
	      /*-------------------------------------------------------------*/
	      /* Since we are only doing one, we don't need to do any of the */
	      /* contortions of the above loops.  All we need to do is	     */
	      /* decide if we got something, and save it if so. 	     */
	      /*-------------------------------------------------------------*/
	      device = volume = tlist;
	      tlist = dlist;

	      while (tlist = NextDosEntry(tlist,LDF_DEVICES|LDF_VOLUMES)) {
		if (tlist->dol_Task == device->dol_Task) {
		  if (tlist->dol_Type != device->dol_Type) {
		    if (device->dol_Type == DLT_DEVICE) {
		      volume = tlist;
		    } else {
		      device = tlist;
		    }
		    break;
		  }
		}
	      }
	      printit = 1;
	    }
	  }
	  UnLockDosList(LDF_DEVICES|LDF_VOLUMES|LDF_READ);

	  /*-----------------------------------------------------------------*/
	  /* If 'printit' is non-zero, we need to print the node just found, */
	  /* with appropriate headers.					     */
	  /*-----------------------------------------------------------------*/
	  if (printit) {
	    PutStr(MSG_DEV_HEADER);
	    PrintInfo(&global, id, device, 0);
	    PutStr(MSG_VOL_HEADER);
	    PrintInfo(&global, id, volume, 1);
	  }
	}
	FreeVec(id);
	rc = RETURN_OK;
      }
      FreeArgs(rdargs);
    }
    CloseLibrary(UtilityBase);
    CloseLibrary((struct Library *)DOSBase);
  } else {
    if (DOSBase) {
      CloseLibrary((struct Library *)DOSBase);
    }
    OPENFAIL;
  }
  return(rc);
}

#define PID_UNIT      0
#define PID_TYPE      1
#define PID_SIZE      1
#define PID_SIZEMULT  2
#define PID_USED      3
#define PID_FREE      4
#define PID_FULL      5
#define PID_ERR       6
#define PID_STATUS    7
#define PID_NAME      8
#define PID_COUNT     9

#undef DOSBase
#define DOSBase global->DOSBase
#undef UtilityBase
#define UtilityBase global->UtilityBase

void PrintInfo(struct Global *global, struct InfoData *infotoprt,
	       struct DosList *dlist, BOOL volume)
{
  LONG prtinfo[PID_COUNT];
  LONG temp;
  LONG size;
  LONG percent;
  char *ptr;
  char devname[ENVMAX];
  char volname[ENVMAX];

  ptr = (char *)BADDR(dlist->dol_Name);
  memcpy(devname, &ptr[1], (long)ptr[0]);
  devname[ptr[0]] = NULL;
  prtinfo[PID_UNIT] = (long)devname;

  if (volume) {

    /*-----------------------------------------------------------------------*/
    /* Tell whether the volume is Mounted or not.			     */
    /*-----------------------------------------------------------------------*/
    if (dlist->dol_Task) {
      prtinfo[PID_TYPE] = (long)MSG_VOL_MOUNTED;
    } else {
      prtinfo[PID_TYPE] = (long)"";
    }
    VPrintf("%s %s\n", prtinfo);
  } else {

    /*-----------------------------------------------------------------------*/
    /* Since we stripped the ':' in the other routine, we need to put it     */
    /* back on for printing purposes.					     */
    /*-----------------------------------------------------------------------*/
    strcat(devname,":");

    /*-----------------------------------------------------------------------*/
    /* Check to see if it is a normal DOS disk. 			     */
    /*-----------------------------------------------------------------------*/
    if ((infotoprt->id_DiskType != ID_DOS_DISK) &&
	(infotoprt->id_DiskType != ID_FFS_DISK) &&
	(infotoprt->id_DiskType != ID_INTER_DOS_DISK) &&
	(infotoprt->id_DiskType != ID_INTER_FFS_DISK) &&
	(infotoprt->id_DiskType != ID_DC_DOS_DISK) &&
	(infotoprt->id_DiskType != ID_DC_FFS_DISK) &&
	(infotoprt->id_DiskType != ID_MSDOS_DISK)) {

      /*---------------------------------------------------------------------*/
      /* If not, figure out Disk Type.					     */
      /*---------------------------------------------------------------------*/
      switch (infotoprt->id_DiskType) {
	case -1:
	  prtinfo[PID_TYPE] = (long)MSG_TYPE_NONE;
	  break;
	case ID_KICKSTART_DISK:
	  prtinfo[PID_TYPE] = (long)MSG_TYPE_KICK;
	  break;
	case ID_NOT_REALLY_DOS:
	  prtinfo[PID_TYPE] = (long)MSG_TYPE_NDOS;
	  break;
	default:
	  prtinfo[PID_TYPE] = (long)MSG_TYPE_BAD;
	  break;
      }
      VPrintf("%-10s%s\n", prtinfo);
    } else {

      /*---------------------------------------------------------------------*/
      /* It IS a normal DOS disk, so report all necessary info. 	     */
      /*---------------------------------------------------------------------*/
      ptr = (char *)BADDR(infotoprt->id_VolumeNode);
      ptr = (char *)BADDR(((struct DeviceList *)(ptr))->dl_Name);
      memset(volname, 0, ENVMAX);
      memcpy(volname, &ptr[1], (long)ptr[0]);

      prtinfo[PID_NAME] = (long)volname;

      /*---------------------------------------------------------------------*/
      /* Fill in number of blocks used and number of blocks free.	     */
      /*---------------------------------------------------------------------*/
      prtinfo[PID_ERR]	= infotoprt->id_NumSoftErrors;
      prtinfo[PID_USED] = infotoprt->id_NumBlocksUsed;
      prtinfo[PID_FREE] = ((infotoprt->id_NumBlocks) -
			   (infotoprt->id_NumBlocksUsed));

      /*---------------------------------------------------------------------*/
      /* Calculate Disk Size in kilo or mega bytes.  As per discussions      */
      /* with RJesup, it seems appropriate to leave the number of reserved   */
      /* blocks OUT of the calculation of total disk space, since you will   */
      /* NEVER be able to use this space, anyway.			     */
      /*---------------------------------------------------------------------*/
      size = UMult32(infotoprt->id_BytesPerBlock,infotoprt->id_NumBlocks);
      prtinfo[PID_SIZE] = size >> 10;
      prtinfo[PID_SIZEMULT] = (long)MSG_KILOBYTES;
      if (prtinfo[PID_SIZE] > 9999) {
	prtinfo[PID_SIZE] = size >> 20;
	prtinfo[PID_SIZEMULT] = (long)MSG_MEGABYTES;
      }

      /*---------------------------------------------------------------------*/
      /* Calculate Percent Full.					     */
      /*---------------------------------------------------------------------*/
      temp = UMult32(100, infotoprt->id_NumBlocksUsed);
      percent = UDivMod32(temp, (infotoprt->id_NumBlocks));
      if (getreg(1) >= (infotoprt->id_NumBlocks >> 1)) {
	++percent;
      }
      prtinfo[PID_FULL] = percent;

      /*---------------------------------------------------------------------*/
      /* Figure out Disk Status.					     */
      /*---------------------------------------------------------------------*/
      switch (infotoprt->id_DiskState) {
	case ID_WRITE_PROTECTED:
	  prtinfo[PID_STATUS] = (long)MSG_PROT_READ_ONLY;
	  break;
	case ID_VALIDATING:
	  prtinfo[PID_STATUS] = (long)MSG_PROT_VALIDATING;
	  break;
	default:
	  prtinfo[PID_STATUS] = (long)MSG_PROT_READ_WRITE;
	  break;
      }

      /*---------------------------------------------------------------------*/
      /* Finally, print all collected data.				     */
      /*---------------------------------------------------------------------*/
      VPrintf("%-8s%5ld%s%8ld%8ld %3ld%% %3ld  %-10s %-s\n", prtinfo);
    }
  }
}
