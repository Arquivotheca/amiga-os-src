; /*
lc -d -rr -j73 -O -o/obj/Delete.o -i/include -v -csf Delete
blink /obj/Delete.o to /bin/Delete sc sd nd
protect /bin/Delete +p
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

/*---------------------------------------------------------------------------*/
/* Command: Delete							     */
/* Author: James E. Cooper Jr.						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 30APR89  Jim Cooper	  Initial Creation				     */
/* 27NOV89  Jim Cooper	  Final Version Completed			     */
/* 08DEC89  Jim Cooper	  Re-Done for V36.42				     */
/* 04MAR90  Jim Cooper	  Modified for loss of "AnchorPath.ap_Buf"...        */
/* 12OCT90  Jim Cooper	  Fixed several bugs... 			     */
/* 06NOV90  John Toebes   Corrected problem with deleteing locked files      */
/* 07NOV90  Jim Cooper	  Fixed John's fix so it reported error messages     */
/*			  correctly.					     */
/* 10DEC90  Jim Cooper	  Added RETURN_WARN return for trying to delete      */
/*			  non-existant file.  (1.3 compat)                   */
/* 03FEB91  Jim Cooper	  Finally fixed problems with printing wrong name    */
/*			  when deleting links.	NameFromLock() was returning */
/*			  incorrect name.				     */
/* 24FEB91  Jim Cooper	  Switched to GetDeviceProc()/FreeDeviceProc() calls */
/*			  from oldstyle DeviceProc().  This should remove    */
/*			  lingering problems with links.		     */
/* 16MAR91  Jim Cooper	  "Fixed" problem with circular links; also fixed    */
/*			  back to use AnchorPath.ap_Buf so I print relative  */
/*			  names just like I (and the 1.3 Delete) used to.    */
/* 16Mar91  Jim Cooper	  Took care of Bill Hawes' Enforcer hit report.  Was */
/*			  not checking to see if a DupLock() succeeded or    */
/*			  failed.  Fixed now.				     */
/* Notes:								     */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "delete_rev.h"

#define MSG_NO_FILES	"No file to delete\n"
#define MSG_DEVICE	"%s is a device and cannot be deleted\n"
#define MSG_DELETED	"  Deleted\n"
#define MSG_NOT_DELETED "  Not Deleted"

#define TEMPLATE	"FILE/M/A,ALL/S,QUIET/S,FORCE/S" CMDREV
#define OPT_PATTERN	0
#define OPT_ALL 	1
#define OPT_QUIET	2
#define OPT_FORCE	3
#define OPT_COUNT	4

/*---------------------------------------------------------------------------*/
/* Global structure used to pass everything between the two routines.	     */
/*---------------------------------------------------------------------------*/
struct Global {
  struct DOSLibrary *DOSBase;
  long opts[OPT_COUNT];
  long dcount;
  BPTR dlock;
  long rc;
  long result2;
};

/*---------------------------------------------------------------------------*/
/* Function prototype for the second routine, so we can use registerized     */
/* parameters.								     */
/*---------------------------------------------------------------------------*/
int KillList(struct Global *, char *, BPTR, STRPTR);

/*---------------------------------------------------------------------------*/
/* The main delete routine.						     */
/*---------------------------------------------------------------------------*/
int cmd_delete(void)
{
  struct Global global;
  struct Library *SysBase = (*((struct Library **) 4));
  struct DosLibrary *DOSBase;
  struct RDargs *rdargs;
  struct DevProc *dproc;
  char *curarg, **argptr;

  /* variables added to deal with absolute drive name problems */
  BPTR *prelocks = NULL;
  STRPTR *names = NULL;
  BPTR *arglock;
  UWORD i,j;
  char oldCh;
  UWORD argcnt;
  BOOL argmatched;
  STRPTR argname;
  UWORD numsources = 0;
  STRPTR *devname;
  BPTR oldCD;

  /*-------------------------------------------------------------------------*/
  /* First, clear the global structure so we don't have any garbage floating.*/
  /*-------------------------------------------------------------------------*/
  memset(&global,0,sizeof(struct Global));

  /*-------------------------------------------------------------------------*/
  /* Set up the default return code.					     */
  /*-------------------------------------------------------------------------*/
  global.rc = RETURN_WARN;

  /*-------------------------------------------------------------------------*/
  /* And open the dos library for our use.				     */
  /*-------------------------------------------------------------------------*/
  if ((DOSBase = (struct DOSLibrary *)OpenLibrary(DOSLIB, DOSVER))) {

    /*-----------------------------------------------------------------------*/
    /* If it opened successfully, store the pointer in the global structure  */
    /* to pass it to the other routine (IF we get that far!).                */
    /*-----------------------------------------------------------------------*/
    global.DOSBase = DOSBase;

    /*-----------------------------------------------------------------------*/
    /* Parse the command line.						     */
    /*-----------------------------------------------------------------------*/
    rdargs = ReadArgs(TEMPLATE, global.opts, NULL);

    /*-----------------------------------------------------------------------*/
    /* If there was an error parsing, print an error message and get out!    */
    /*-----------------------------------------------------------------------*/
    if (rdargs == NULL) {
      PrintFault(IoErr(), NULL);
    } else {

	/* count the number of things to delete */
        argptr = (char **) global.opts[OPT_PATTERN];
        numsources = 0;
        while (*argptr++)
            numsources++;

        /* allocate an array to hold locks on the things to delete */
        if (!(prelocks = AllocVec((ULONG)numsources*4,MEMF_CLEAR|MEMF_PUBLIC)))
        {
            PrintFault(ERROR_NO_FREE_STORE,NULL);
            goto Done;
        }

        /* allocate an array to hold the device names of the things to delete */
        if (!(names = AllocVec((ULONG)numsources*4,MEMF_CLEAR|MEMF_PUBLIC)))
        {
            PrintFault(ERROR_NO_FREE_STORE,NULL);
            goto Done;
        }

        argptr = (char **) global.opts[OPT_PATTERN];
        argcnt = 0;
        while (argptr[argcnt])
        {
	    /* scan for device name */
            argname = argptr[argcnt];
            i = 0;
            while ((argname[i] != ':') && (argname[i]))
                i++;

	    /* got a colon, so its a device name */
            if (argname[i] == ':')
            {
                i++;
                oldCh = argname[i];
                argname[i] = 0;

		/* see if this device name has already been used */
                argmatched = FALSE;
                j = 0;
                while (j <= argcnt)
                {
                    if ((names[j]) && (stricmp(argname,names[j]) == 0))
                    {
                        argmatched = TRUE;
                        break;
                    }
                    j++;
                }

                if (argmatched)
                {
                    prelocks[argcnt] = DupLock(prelocks[j]);
                }
                else
                {
                    prelocks[argcnt] = Lock(argname,ACCESS_READ);
                }

                if (prelocks[argcnt])
                {
                    if (!(names[argcnt] = AllocVec(strlen(argname)+1,MEMF_CLEAR|MEMF_PUBLIC)))
                    {
                        PrintFault(ERROR_NO_FREE_STORE,NULL);
                        goto Done;
                    }
                    strcpy(names[argcnt],argname);

                    argname[i] = oldCh;
                    strcpy(argname,&argname[i]);
                }
            }
            argcnt++;
        }


      arglock = &prelocks[0];
      devname = &names[0];
      argptr = (char **)global.opts[OPT_PATTERN];
      /*---------------------------------------------------------------------*/
      /* The following while loop handles the NEW MultiArgs spec.	     */
      /*---------------------------------------------------------------------*/
      while (curarg = *argptr++) {

	/*-------------------------------------------------------------------*/
	/* If the filespec given doesn't have a device associated with it,   */
	/* something is VERY wrong!  Tell 'em so and exit.                   */
	/*-------------------------------------------------------------------*/
        if (*arglock)
            oldCD = CurrentDir(*arglock);

	if ((dproc = GetDeviceProc(curarg, NULL)) == NULL) {
          if (*arglock)
              CurrentDir(oldCD);
	  PrintFault(IoErr(), NULL);
	  global.dcount++;
	} else {
	  FreeDeviceProc(dproc);
          if (*arglock)
              CurrentDir(oldCD);

	  /*-----------------------------------------------------------------*/
	  /* If the filespec they gave ends with a colon, it is either a     */
	  /* device, such as RAM:, or an assignment, such as C:.  We can't   */
	  /* do anything about these (according to current specs), so we     */
	  /* just complain and exit.					     */
	  /*-----------------------------------------------------------------*/
          if ((!curarg[0]) && (*devname))
          {
	    VPrintf(MSG_DEVICE, (long *)devname);
	    global.dcount++;
	    break;
	  } else {

	    /*---------------------------------------------------------------*/
	    /* Finally!  Everthing SEEMS ok, so we try to delete something.  */
	    /*---------------------------------------------------------------*/
	    if (KillList(&global, curarg, *arglock, *devname) != NULL) {
	      break;
	    }
	  }
	}
	arglock++;
	devname++;
      }
      /*---------------------------------------------------------------------*/
      /* We're ready to exit, so clean up after ourselves.                   */
      /*---------------------------------------------------------------------*/
      FreeArgs(rdargs);
    }

    if (!(global.dcount)) {
      PutStr(MSG_NO_FILES);
      global.rc = RETURN_WARN;
    }

Done:
    if (prelocks)
    {
        i = 0;
        while (i < numsources)
            UnLock(prelocks[i++]);
        FreeVec(prelocks);
    }

    if (names)
    {
        i = 0;
        while (i < numsources)
            FreeVec(names[i++]);
        FreeVec(names);
    }

    if(global.rc)SetIoErr(global.result2);
   CloseLibrary((struct Library *)DOSBase);
  } else {
    OPENFAIL;
  }
  return(global.rc);
}

/*---------------------------------------------------------------------------*/
/* This routine is used to kill all the files which match a specific pattern */
/* whether the pattern includes wildcards or not.			     */
/*---------------------------------------------------------------------------*/
int KillList(struct Global *global, char *name, BPTR lock, STRPTR devname)
{
  struct Library *SysBase = (*((struct Library **) 4));
  struct AnchorPath *ua;
  BOOL skip1,printflag=TRUE;
  LONG temprc=0,trc,loopc;
  struct DosLibrary *DOSBase;
  BPTR oldlock, curlock, flock;
  char *cachename;
  char cachefull[ENVMAX];
  BPTR oldCD;

  DOSBase = global->DOSBase;
  global->rc = RETURN_OK;

  /*-------------------------------------------------------------------------*/
  /* Try to allocate a chunk of memory to use with the pattern match stuff.  */
  /*-------------------------------------------------------------------------*/
  if ((ua = AllocVec(sizeof(struct AnchorPath)+ENVMAX,
		     MEMF_CLEAR|MEMF_PUBLIC)) == NULL) {
    PrintFault(IoErr(),NULL);
    global->rc = RETURN_FAIL;
  } else {

    /*-----------------------------------------------------------------------*/
    /* Now we have to set up the pattern match structure.		     */
    /*-----------------------------------------------------------------------*/
    /* Set the flag which tells the pattern matcher to decode wild cards.    */
    /*-----------------------------------------------------------------------*/
    ua->ap_Flags = APF_DOWILD;
    ua->ap_Strlen = ENVMAX;

    /*-----------------------------------------------------------------------*/
    /* Tell the matcher to stop if the user presses Ctrl-C.		     */
    /*-----------------------------------------------------------------------*/
    ua->ap_BreakBits = SIGBREAKF_CTRL_C;

    /*-----------------------------------------------------------------------*/
    /* Finally, call the matcher.					     */
    /*-----------------------------------------------------------------------*/
    if (lock)
        oldCD = CurrentDir(lock);

    loopc = MatchFirst(name,ua);

    /*-----------------------------------------------------------------------*/
    /* Process all matches returned.					     */
    /*-----------------------------------------------------------------------*/
    while ((loopc == 0) && ((temprc == 0) ||
	    (temprc == ERROR_OBJECT_IN_USE) ||
	    (temprc == ERROR_DIRECTORY_NOT_EMPTY) ||
	    (temprc == ERROR_DELETE_PROTECTED))) {

      /*---------------------------------------------------------------------*/
      /* 'skip1' is a flag which tells us whether or not to try deleting a   */
      /* subdirectory.	Basically, skip it if we have not deleted all the    */
      /* files under it, delete it if we have cleaned it out.  This only has */
      /* validity if the user used the 'ALL' keyword on the command line.    */
      /*---------------------------------------------------------------------*/
      skip1 = FALSE;

      /*---------------------------------------------------------------------*/
      /* Make a copy of this file's name with full path.                     */
      /*---------------------------------------------------------------------*/
      if ((curlock = DupLock(ua->ap_Current->an_Lock)) == NULL) {

	/*-------------------------------------------------------------------*/
	/* If we can't duplicate the lock, something is *seriously* wrong!   */
	/* Let's set some error codes and BAIL OUT!                          */
	/*-------------------------------------------------------------------*/
	temprc = 1L;
	break;

      } else {
	strcpy(cachefull, ua->ap_Buf);
	cachename = FilePart(cachefull);

	if (((ua->ap_Info.fib_DirEntryType > 0) &&
	     (ua->ap_Info.fib_DirEntryType < 3)) &&
	    global->opts[OPT_ALL]) {

	  /*-----------------------------------------------------------------*/
	  /* The flag APF_DIDDIR tells us that we are 'backing out' of a     */
	  /* subdirectory... in other words, we have deleted all sub files   */
	  /* of that directory and we are moving back to its parent.  If     */
	  /* this flag is set, we need to clear it, and leave skip1 alone so */
	  /* this subdir gets deleted as the user specified.		     */
	  /*-----------------------------------------------------------------*/
	  if (!(ua->ap_Flags & APF_DIDDIR)) {

	    /*---------------------------------------------------------------*/
	    /* If we are deleting ALL files, tell the matcher to enter this  */
	    /* directory; we also need to skip trying to delete this file    */
	    /* until we exit it, as explained above.			     */
	    /*---------------------------------------------------------------*/
	    ua->ap_Flags |= APF_DODIR;
	    skip1 = TRUE;
	    UnLock(curlock);
	  }
	  ua->ap_Flags &= ~APF_DIDDIR;
	}
      }

      /*---------------------------------------------------------------------*/
      /* Now get the next file which matches the given pattern. 	     */
      /*---------------------------------------------------------------------*/
      loopc = MatchNext(ua);

      /*---------------------------------------------------------------------*/
      /* Now, unless we are skipping the current file, we need to try to     */
      /* delete it.							     */
      /*---------------------------------------------------------------------*/
      if (!skip1) {

	/*-------------------------------------------------------------------*/
	/* Gather info and set current directory of file we wish to delete.  */
	/*-------------------------------------------------------------------*/
	oldlock = CurrentDir(curlock);
	flock = Lock(cachename,ACCESS_READ);
	if (flock == NULL) {
	  /* There was an error on the current file.. */
	  temprc = IoErr();
	  trc = 0L;
	  if (global->opts[OPT_QUIET] == NULL) {
	    if (devname)
	        PutStr(devname);
	    PutStr(cachefull);
	  }
	  Result2(temprc);
	} else {
	  UnLock(flock);

	  /*-----------------------------------------------------------------*/
	  /* Unless we are being vewy qwiet, print the name of the file we   */
	  /* are about to try to delete.				     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_QUIET] == NULL) {
	    if (devname)
	        PutStr(devname);
	    PutStr(cachefull);
	  }

	  /*-----------------------------------------------------------------*/
	  /* If we got this far, we can assume we are going to exit with a   */
	  /* return code, whether we actually delete the file or not.	     */
	  /*-----------------------------------------------------------------*/
	  global->rc = RETURN_OK;

	  /*-----------------------------------------------------------------*/
	  /* KILL IT!							     */
	  /*-----------------------------------------------------------------*/
	  global->dcount++;

	  trc=DeleteFile(cachename);
	  if(trc == NULL) { /* failed for some reason */
	    global->result2 =IoErr();
	    if((global->result2 == ERROR_DELETE_PROTECTED)&&
		global->opts[OPT_FORCE]) {

	      global->result2=0; /* ready for second try */
	      SetProtection(cachename,0x0); /* try again */
	      trc=DeleteFile(cachename);
	    }
	  }
	} /* Match to flock == NULL else clause above */
	CurrentDir(oldlock);
	UnLock(curlock);

	if(trc != NULL) {
	  /*-----------------------------------------------------------------*/
	  /* If it worked, tell everybody!  Unless we are in QUIET mode, of  */
	  /* course!							     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_QUIET] == NULL) PutStr(MSG_DELETED);
	  global->rc=RETURN_OK;
	  global->result2=0;
	}
	else {
	  /*-----------------------------------------------------------------*/
	  /* If we aren't able to kill the file, we need to tell 'em about   */
	  /* it and go on.						     */
	  /*-----------------------------------------------------------------*/
	  temprc = IoErr();
	  global->result2 = temprc;

	  /*-----------------------------------------------------------------*/
	  /* If we have been quiet up to now, we need to go ahead and print  */
	  /* the name out.  This was a rather annoying ommision of the BCPL  */
	  /* version, in that if you told it to be quiet, it would... except */
	  /* when it printed an error message, you never knew WHERE the      */
	  /* error had occured. 					     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_QUIET] != NULL) {
	    if (devname)
	        PutStr(devname);
	    PutStr(cachefull);
	  }
	  PrintFault(temprc, MSG_NOT_DELETED);
	  printflag=FALSE; /* don't print the message again */
	}
      }
    }

    /*-----------------------------------------------------------------------*/
    /* If the only error we had was that we ran out of files to delete, we   */
    /* are doing fine!	Set the return code so the user knows everything     */
    /* went well.							     */
    /*-----------------------------------------------------------------------*/
    if (temprc == ERROR_NO_MORE_ENTRIES) {
      temprc = NULL;
      global->rc = RETURN_OK;
    }

    /*-----------------------------------------------------------------------*/
    /* If the user hit Ctrl-C, we have to warn him that he is being rude!    */
    /*-----------------------------------------------------------------------*/
    if (temprc == ERROR_BREAK) {
      temprc = NULL;
      PrintFault(IoErr(),NULL);
      global->rc = RETURN_WARN;
      global->dcount++;
    }

    /*-----------------------------------------------------------------------*/
    /* If there was any other error, tell the user about it and leave!	     */
    /*-----------------------------------------------------------------------*/
    if (temprc) {
      if(printflag)PrintFault(IoErr(),NULL);
      global->rc = RETURN_FAIL;
      global->dcount++;
    }

    /*-----------------------------------------------------------------------*/
    /* We have to clean up after ourselves.				     */
    /*-----------------------------------------------------------------------*/
    MatchEnd(ua);

    if (lock)
        CurrentDir(oldCD);

    FreeVec(ua);
  }
  return(global->rc);
}
