; /*
FailAt 21
Lc -d -j73 -rr -O -o/obj/Dir.o -i/include -v -cfimst -q1e99w Dir.c
Blink /obj/Dir.o to Dir.ld lib lib:lcr.lib sc sd nd batch
Protect /bin/Dir +p
Quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 460-7430    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*	       John Toebes     Mary Ellen Toebes  Doug Walker  Mike Whitcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*---------------------------------------------------------------------------*/
/* Command: Dir 							     */
/* Author:  Bruce M. Drake						     */
/*									     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 06APR89  Bruce Drake   Initial Creation				     */
/* 30OCT89    "     "     Rewritten to use DOS 36                            */
/* 03NOV89    "     "     Rewritten again to use new algorithms made         */
/*			    possible with DOS 36			     */
/* 27NOV89    "     "     Added Global structure variable, simplifies        */
/*			    registerized parameters			     */
/* 03DEC89    "     "     Completed implementation of MatchFirst, MatchNext  */
/*			    but interactive mode needs work.		     */
/* 08DEC89    "     "     Complete and tested, needs interactive Delete      */
/*			    code added. Problem with Match...() prevents     */
/*			    implementation until problems are resolved.      */
/* 13DEC89    "     "     Last minute documentation and bug fixes.  What I   */
/*			    would call Version 0.9			     */
/* 17DEC89    "     "     All functions implemented, ready for first beta.   */
/* 06JAN90    "     "     Fixed problem with MatchFirst() and with OPT       */
/*			    parameter parsing				     */
/* 08MAR90    "     "     Fixed problem with interactive not flushing to     */
/*			    screen.  Also re-arranged code to drop size.     */
/* 11MAR90    "     "     Fixed problem with wildcards and interactive mode. */
/* 19MAY90    "     "     Verified bug free and FEATURE FROZEN! Modified     */
/*			    error message to reflect better phraseology.     */
/*			    Good ol' 1.0!                                    */
/* 23MAY90    "     "     Last second tweaks, ready to ship (I hope! F=3108) */
/* 24MAY90    "     "     Revamped error printing mechanism (F=3104)         */
/* 10JUL90    "     "     Fixed bugs precipitated by changes to V36.         */
/* 12JUL90    "     "     Enhanced operation to emulate the 1.3 Dir more     */
/*			    closely.  Switch to C 5.10.  (F=3104)            */
/* 31AUG90    "     "     Completed program by adding last second patches.   */
/*			    (Filesize = 3108)                                */
/* 11DEC90    "     "     Added code to test for circular links              */
/* 13DEC90    "     "     Patched bug in circular link code, handles empty   */
/*			    directories now!				     */
/* 30JAN91    "     "     Added ^C event robustness replacing routines that  */
/*			    was lost somehow in the shuffle and will not     */
/*			    stomp on low memory!			     */
/* 13APR91    "     "     Stepped back to more stable version and added some */
/*			    other bug fixes and the ability to show links.   */
/* 14APR91    "     "     Corrected esoteric collating bug.                  */
/* 26APR91  M. Taillefer  Corrected #? and <dir>/#? behavior                 */
/*									     */
/* Notes:								     */
/*   This code is being redesigned for 2.1.  There were some major problems  */
/*  found in the code pertaining to complex wildcard patterns.	There has    */
/*  not been any solution to the problem found by anyone that the problem    */
/*  was described to.  Fortunately, the problem is relatively rare.	     */
/*---------------------------------------------------------------------------*/

/* Options */
#define SEE_LINKS  1   /* shows when an entry is a link and what type */

#include "internal/commands.h"
#include "dir_rev.h"

#include "fctype.h"

/*===============================================================*/
/* These string definitions below are close to the BCPL version. */
/*===============================================================*/
#define MSG_OPTION_IGNORED   " option ignored\n"
#define MSG_OUT_OF_MEM_INC   "List incomplete - "
#define MSG_NOT_A_DIR	     "%s is not a directory\n"
#define MSG_DELETED	     "Deleted\n"
#define MSG_COMMAND_PROMPT   "\nCommand ? "
#define MSG_INVALID_RESPONSE "Invalid response - try again\n"
#define MSG_NO_INFO_FOR      "Could not get information for %s\n"
#define MSG_CIRCULAR_DIR     "Error, circular directory entry found\n"

/*===============================================*/
/* Template for the main section of the command. */
/*===============================================*/
#define TEMPLATE	  "DIR,OPT/K,ALL/S,DIRS/S,FILES/S,INTER/S" CMDREV
#define OPT_DIR 	  0
#define OPT_OPT 	  1
#define OPT_ALL 	  2
#define OPT_DIRS	  3
#define OPT_FILES	  4
#define OPT_INTER	  5
#define OPT_COUNT	  6

/*====================================================================*/
/* Template for the interactive mode when examining a file, not a dir */
/*====================================================================*/
#define INTER_FILE_TEMPLATE \
	"T=TYPE/S,B=BACK/S,DEL=DELETE/S,Q=QUIT/S,C=COM/S,COMMAND"

/*====================================================================*/
/* Template for the interactive mode when examining a dir, not a file */
/*====================================================================*/
#define INTER_DIR_TEMPLATE \
	"E=ENTER/S,B=BACK/S,DEL=DELETE/S,Q=QUIT/S,C=COM/S,COMMAND"

#define OPT_INTER_ENTER_TYPE  0    /* behold the reusable flag! */
#define OPT_INTER_BACK	      1
#define OPT_INTER_DELETE      2
#define OPT_INTER_QUIT	      3
#define OPT_INTER_COM	      4
#define OPT_INTER_COMMAND     5
#define INTER_COUNT	      6

/*==============================================================*/
/* Flag definitions below apply to the global.optflags variable */
/*==============================================================*/
#define ALL_FLAG	  1
#define DIRS_FLAG	  2
#define FILES_FLAG	  4
#define INTER_FLAG	  8
#define OPT_BACK_FLAG	  16
#define OPT_ENTER_FLAG	  32
#define OPT_QUIT_FLAG	  64
#define REPEAT_ENTRY_FLAG 128
#define DELETE_DIR_FLAG   256

/*==================*/
/* General defines  */
/*==================*/
#define NO_ERROR	  0
#define BREAKBITS	  (SIGBREAKF_CTRL_C)
#define FILENAME_SIZE	  32
#define FILEPATH_SIZE	  128
#define BUFFER_SIZE	  2048
#define TEMPVECTOR_SIZE   5
#define TYPE_DIR	  0
#define TYPE_FILE	  1
#define EXEC_BUFF_SIZE	  128

/*====================*/
/* Private structures */
/*======================================================================*/
/* This structure forms a linked list of filenames for sorting purposes */
/*======================================================================*/
struct FNameList {
  struct FNameList *fnl_next;        /* must be first field in structure due to sorting algorithm */
  LONG		    fnl_filetype;
  char		    fnl_name[FILENAME_SIZE];
};

/*==============================================================*/
/* This structure makes it possible to check for circular links */
/* Added: bmd 12/10/90						*/
/*==============================================================*/
struct Lookback {
  struct Lookback *next;
  BPTR		   lock;
};

/*==========================================================*/
/* This structure gives us the facility of global variables */
/* while keeping the code pure and resident-able.	    */
/*==========================================================*/
struct Global {
  struct Library    *SysBase;
  struct DOSLibrary *DOSBase;
  struct AnchorPath ap;
  LONG		    optflags;
  LONG		    errorcode;
  LONG		    returncode;
  LONG		    opts[OPT_COUNT];
  UBYTE 	    *buffer;
  UBYTE 	    filename[FILEPATH_SIZE];
  UBYTE 	    fntemp[2][FILEPATH_SIZE];
  LONG		    tempvector[TEMPVECTOR_SIZE];
  struct FNameList  *fnametemp,
		    *fnamewalk,
		    *fnameprev;
  struct Lookback   *lookwalk;
};

/*=======================*/
/*  Function Prototypes  */
/*=======================*/
void do_dir(struct Global *, int, struct Lookback *);
void dointeract(struct Global *, BPTR, int);
int vsprintf(char *, char *, long []);

#pragma syscall RawDoFmt 20a ba9804
extern void __builtin_emit(UWORD);
static void __regargs prbuf(char c);


/*=============================================================*/
/*********************** Main program **************************/
/*=============================================================*/
int cmd_dir(void)
{
  struct Library    *SysBase = (*((struct Library **) 4));
  struct DOSLibrary *DOSBase;
  struct Global      global;
  struct RDArgs     *argsptr;
  UBYTE 	    *text;

  /*===========================================================*/
  /* Initialize global data structure and return code variable */
  /*===========================================================*/
  memset((char *)&global, 0, sizeof(struct Global));
  global.SysBase = SysBase;		/* Added: bmd 12/10/90 */
  global.returncode = RETURN_FAIL;
  global.errorcode = NO_ERROR;

  if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {

    global.DOSBase = DOSBase;

    /*==================================================================*/
    /* AllocVec changed to syntax of old AllocVecType().  I try to trap */
    /* ALL instances of memory allocation failures. buffer is just some */
    /* general work space for building strings and stuff like that.	*/
    /*==================================================================*/
    if (!(global.buffer = AllocVec(BUFFER_SIZE, MEMF_CLEAR | MEMF_PUBLIC))) {
      global.errorcode = IoErr();
    }
    else {

      /*==================================================================*/
      /* If ReadArgs() sees anything but zeros passed to it in elements   */
      /* of this array, ReadArgs() will assume that they are defaults.    */
      /*==================================================================*/
      argsptr = ReadArgs(TEMPLATE, global.opts, NULL);

      /*=================================================================*/
      /* argsptr will be NULL if ReadArgs() failed, the secondary result */
      /* code is fetched by IoErr().                                     */
      /*=================================================================*/
      if (argsptr == NULL) global.errorcode = IoErr();
      else {

	/*=================================================================*/
	/* We need to check various OPT options.  We build the		   */
	/* global.optflags variable here to reflect user specified options */
	/* and set our defaults. No check is made for mutual exclusion.    */
	/*=================================================================*/
	if (global.opts[OPT_OPT] != NULL) {
	  text = (char *)global.opts[OPT_OPT];
	  while (*text) {

	    switch(*text++)
	    {
	      case 'i': case 'I':       /**** Interactive ***/
		global.opts[OPT_INTER] = 1;
		break;

	      case 'a': case 'A':       /**** All ***********/
		global.opts[OPT_ALL]   = 1;
		break;

	      case 'd': case 'D':       /**** Dirs only *****/
		global.opts[OPT_DIRS]  = 1;
		break;

	      case 'f': case 'F':       /**** Files only ****/
		global.opts[OPT_FILES] = 1;
		break;

	      default:
		/*======================================================*/
		/* Should get here only if an invalid option was found. */
		/*======================================================*/
		FPutC(Output(), text[-1]);
		PutStr(MSG_OPTION_IGNORED);
		break;
	    }
	  }
	}

	/*=============================================*/
	/* Set global flags based on specified options */
	/*=============================================*/
	if (global.opts[OPT_ALL])    global.optflags |= ALL_FLAG;
	if (global.opts[OPT_INTER])  global.optflags |= INTER_FLAG;
	if (global.opts[OPT_FILES])  global.optflags |= FILES_FLAG;
	if (global.opts[OPT_DIRS])   global.optflags |= DIRS_FLAG;

	/*===================*/
	/* Set flag defaults */
	/*===================*/
	if (global.opts[OPT_FILES] == 0 && global.opts[OPT_DIRS] == 0)
	  global.optflags |= DIRS_FLAG | FILES_FLAG;

	/*===========================================================*/
	/* Initialize AnchorPath flags that Match...() need to do    */
	/* the directory scan.	We always assume we want to do	     */
	/* wildcard analysis and CTRL-C trapping.		     */
	/*===========================================================*/
	global.ap.ap_BreakBits = BREAKBITS;
	global.ap.ap_Flags = APF_DOWILD;

	/*================================================================*/
	/* MatchFirst() doesn't like a NULL passed to it as the first     */
	/* parameter.  Since ReadArgs() returns to us a NULL instead of   */
	/* a pointer to a NULL when the user does not specify any dir, we */
	/* must make a pointer to a NULL.  Otherwise, MatchFirst() dies.  */
	/* This cures several bugs reported to CBM, fixed on 06JAN90.	  */
	/*================================================================*/
	if (global.opts[OPT_DIR] == NULL) global.opts[OPT_DIR] = (LONG)"";

	/*===================================================================*/
	/* Here is the only MatchFirst() call.  MatchFirst() does several    */
	/* things: 1) initializes the AnchorPath structure, 2) checks	     */
	/* wildcard status on the input dirname (which can be a NULL to      */
	/* indicate the desire to scan the current directory), 3) returns to */
	/* us things like FileInfoBlocks and a lock the filename is relative */
	/* to. It returns a ZERO (NULL) to indicate success, otherwise the   */
	/* error codes match those in ...dos.h and ...dosasl.h. 	     */
	/* See ...dosasl.h for more.					     */
	/*===================================================================*/
	if (MatchFirst((char *)global.opts[OPT_DIR], &global.ap)) {
	  global.errorcode = IoErr();
	  if( (global.errorcode != ERROR_BREAK) &&
	      !(global.ap.ap_Flags & APF_ITSWILD) )
	    VPrintf(MSG_NO_INFO_FOR, &global.opts[OPT_DIR]);
	}
	else
	{
	  /*================================================================*/
	  /* Check to see if we examined a directory or not.  If we are not */
	  /* processing wildcards and if the examine revealed a file, we    */
	  /* can't go any further.                                          */
	  /*================================================================*/
	  if ((global.ap.ap_Info.fib_DirEntryType < 0) &&
	      (!(global.ap.ap_Flags & APF_ITSWILD)))
	  {
	    global.errorcode = ERROR_DIR_NOT_FOUND;
	    VPrintf(MSG_NOT_A_DIR, &global.opts[OPT_DIR]);
	  }
	  else
	  {
	    /*=========================================*/
	    /* Go and get the directory or directories */
	    /*=========================================*/
	    global.ap.ap_Flags &= ~APF_DIDDIR;
	    do_dir(&global, 0, NULL);
	  }
	}  /*====== end if MatchFirst() =====*/

	/*========================================================*/
	/* This function cleans up the AnchorPath structure list. */
	/*========================================================*/
	MatchEnd(&global.ap);

	/*=================================================================*/
	/* Always call FreeArgs() after successfully calling ReadArgs() to */
	/* recover any allocated memory and to perform cleanup functions.  */
	/*=================================================================*/
	FreeArgs(argsptr);
      }

      FreeVec(global.buffer);
    }
    /*==============*/
    /* Cleanup code */
    /*==============*/
    if ((global.errorcode != NO_ERROR)) {

      /*=============================================================*/
      /* The only time we print an error message is when we have an  */
      /* error other than ERROR_NO_MORE_ENTRIES. We suppress setting */
      /* Result2 if we have ERROR_BREAK.			     */
      /*=============================================================*/
      if (global.errorcode != ERROR_BREAK)
	SetIoErr(global.errorcode);

      if (global.errorcode != ERROR_NO_MORE_ENTRIES)
	PrintFault(global.errorcode, NULL);
    }
    CloseLibrary((struct Library *)DOSBase);
  }
  else { OPENFAIL; }
  return(global.returncode);
}


/*================================================================*/
/*  Function: do_dir()                                            */
/*   Purpose: provide a routine to gather and display directories */
/*	      in a recursive manner.				  */
/*================================================================*/

/*=============================================*/
/* Special #defines for library base pointers. */
/* Used as substitutes for clarity of code.    */
/*=============================================*/
#define SysBase     global->SysBase
#define DOSBase     global->DOSBase
#define tempvector  global->tempvector
#define fnametemp   global->fnametemp
#define fnamewalk   global->fnamewalk
#define fnameprev   global->fnameprev

void do_dir(struct Global *global, int reclvl, struct Lookback *lookback)
{
  struct FNameList    *fnamehead = NULL;
  struct Lookback      previous;
  int		       rc,
		       matchrc,
		       column;
  BPTR		       dirlock = NULL,
		       lock;

  /*================================================*/
  /* Properly assign DOSBase to keep our code pure. */
  /*================================================*/
  global->returncode = RETURN_FAIL;
  matchrc = 0;
  previous.next = lookback;

  /*========================================================================*/
  /* Reset the flag that tells us to enter the next directory we encounter. */
  /*========================================================================*/
  global->optflags &= ~OPT_ENTER_FLAG;

  /*==============================================================*/
  /* Set up the flags and the loop for MatchNext(). If we have a  */
  /* directory, we have to perform a work around.  Normally, we   */
  /* set the APF_DODIR flag to tell Match...() to enter the       */
  /* directory. But we have to make the setting of the flag	  */
  /* conditional on if MatchFirst() returns a file or a dir.      */
  /*==============================================================*/
  if ((global->ap.ap_Info.fib_DirEntryType > 0)) {

    if (!((reclvl == 0) && (global->ap.ap_Flags & APF_ITSWILD)))
    {
      global->ap.ap_Flags |= APF_DODIR;
      matchrc = MatchNext(&global->ap);
    }
  }

  /*===================================================================*/
  /* To facilitate maintaining a lock on the current parent directory, */
  /* we copy the parent lock.  Match...() generates this and we need   */
  /* to save this lock for the interactive TYPE and DELETE commands.   */
  /* We are certain to have a lock by this time or this code will have */
  /* never executed to this point.				       */
  /* *** It became apparent that there is no need to execute this code */
  /* if matchrc is indicating anything other that complete success.    */
  /* The problem that arose is one of hitting ^C at just the right     */
  /* time, I was trying to duplock something that didn't exist!!       */
  /*===================================================================*/
  if((global->ap.ap_FoundBreak) & BREAKBITS)
  {
    matchrc = ERROR_BREAK;
    global->optflags = OPT_QUIT_FLAG;
  }

  if ( matchrc == NULL )        /* added/restored by bmd on 01/20/91 */
  {
    dirlock = DupLock(global->ap.ap_Current->an_Lock);

    /*====================================================================*/
    /* We need to check our list of directory locks to see if we have	  */
    /* already seen this directory entry.  If we have, we have a circular */
    /* hard/soft link (or a corrupt directory!).                          */
    /*====================================================================*/
    previous.lock = dirlock;
    previous.next = lookback;
    global->lookwalk = lookback;
    while (global->lookwalk)
    {
      if ( (SameLock(global->lookwalk->lock, dirlock) == LOCK_SAME) &&
	   !(global->ap.ap_Flags & APF_DIDDIR) )
      {
	PutStr(MSG_CIRCULAR_DIR);
	goto TimeToGo;
      }
      global->lookwalk = global->lookwalk->next;
    }
  }

  /*================================================================*/
  /* Everything should be ready to go at this point.  This while    */
  /* loop starts the section of the code which increments through   */
  /* a directory one file at a time.  There are many things to	    */
  /* check for as we go, but as long as matchrc == NULL, things are */
  /* A-OK.							    */
  /*================================================================*/
  while(matchrc == NULL) {

    /*==============================================================*/
    /* MatchNext() may return the flag APF_DIDDIR.  This shows that */
    /* it is done with this directory level.   This would seem	    */
    /* redundant with the flag ERROR_NO_MORE_ENTRIES, but we were   */
    /* getting returned to us an entry for the parent directory!    */
    /* We have to trap this.					    */
    /*==============================================================*/

    if (global->ap.ap_Flags & APF_DIDDIR)
    {
      global->ap.ap_Flags &= ~APF_DIDDIR;
      matchrc = ERROR_NO_MORE_ENTRIES;
      break;
    }

    if((global->ap.ap_FoundBreak) & BREAKBITS) {
	matchrc = ERROR_BREAK;
	global->optflags = OPT_QUIT_FLAG;
	break;
    }

    /*=============================================================*/
    /* When we find a directory, print it unless printing is	   */
    /* suppressed. fib_DirEntryType > NULL indicates a directory. */
    /*=============================================================*/
    if ((global->optflags & DIRS_FLAG) &&
	(global->ap.ap_Info.fib_DirEntryType > 0)) {

      /*===================================================================*/
      /* When the user uses wildcards, we are to print the fully qualified */
      /* path with our dirname.  This way the user sees the context of the */
      /* directory files as the program sees it.			   */
      /*===================================================================*/
      if ( (global->ap.ap_Flags & APF_ITSWILD) && (reclvl == 0) ) /* 020491 */
      {
	NameFromLock( global->ap.ap_Current->an_Lock,
		      global->filename, FILEPATH_SIZE );
	AddPart(global->filename, global->ap.ap_Info.fib_FileName, FILEPATH_SIZE);
      }
      else
      {
	strncpy(global->filename, global->ap.ap_Info.fib_FileName, FILEPATH_SIZE);
      }

      /*======================================================*/
      /* set up indentation (brute force!) and print the name */
      /*======================================================*/
      tempvector[0] = (reclvl + 1) * 5;
      tempvector[1] = (LONG)global->filename;
#ifdef SEE_LINKS
      if (global->ap.ap_Info.fib_DirEntryType == ST_LINKDIR)
	tempvector[2] = (LONG)"(dir) <hl>";
      else
#endif
	tempvector[2] = (LONG)"(dir)";
      vsprintf(global->buffer, "%%%lds%%s %%s", tempvector);
      tempvector[0] = (LONG)" ";

      /*=========================================================*/
      /* If we are in interactive mode, call dointeract(), else  */
      /* we tack on a newline to our freshly printed dirname.	 */
      /*=========================================================*/
      if (global->optflags & INTER_FLAG) {

	/*========================================================*/
	/* Basically, dointeract() prompts after the filename and */
	/* performs actions based on its template.  Return codes  */
	/* appear in global->optflags and global->returncode. The */
	/* REPEAT_ENTRY_FLAG is to support the dointeract() call  */
	/* and redisplay the filename depending on which command  */
	/* the user typed.					  */
	/*========================================================*/
	global->optflags |= REPEAT_ENTRY_FLAG;
	while(global->optflags & REPEAT_ENTRY_FLAG)
	{
	  /*======================================================*/
	  /* Print the filename and perform interactive functions */
	  /*======================================================*/
	  VPrintf(global->buffer, tempvector);
	  VPrintf(" ? ", NULL);
	  dointeract(global, NULL, TYPE_DIR);
	}
	/*=========================================================*/
	/* OPT_BACK_FLAG signals that the user wishes to come back */
	/* one level shallower in the directory tree structure.    */
	/*=========================================================*/
	if (global->optflags & OPT_BACK_FLAG)
	{
	  /*=====================================*/
	  /* Reset the flag and exit this level. */
	  /*=====================================*/
	  global->optflags &= ~OPT_BACK_FLAG;
	  break;
	}

	/*===========================================================*/
	/* OPT_QUIT_FLAG signals the user's desire to hit the road.  */
	/* We have to preserve this flag so we can quit cleanly from */
	/* any level. We also quit out if we have a major error.     */
	/*===========================================================*/
	if ((global->optflags & OPT_QUIT_FLAG) ||
	    (global->returncode == RETURN_FAIL))
	  goto TimeToGo;
      }
      else
      {
	/*====================================*/
	/* Just print and terminate the line. */
	/*====================================*/
	VPrintf(global->buffer, tempvector);
	PutStr("\n");
      }
    }

    /*====================================================================*/
    /*	 Under 1.3, if the user types a command such as "Dir RAM:#?", the */
    /* user will see the contents of the top 2 levels of RAM:.	I need	  */
    /* to check for all the conditions to emulate this behaviour.	  */
    /*====================================================================*/

    /*====================================================================*/
    /* 26APR91 - Put this code in comment. 1.3 behavior was wrong.        */
    /*           "Dir RAM:#?" now only shows files and dirs in the root   */
    /*           of RAM: and doesn't enter unless ALL is specified        */
    /*====================================================================*/
/*
    if ( !(global->optflags & ALL_FLAG) &&
	  (reclvl < 1) &&
	  (global->ap.ap_Flags & APF_ITSWILD) &&
	  (global->ap.ap_Info.fib_DirEntryType >= 0))
    {
      global->optflags |= OPT_ENTER_FLAG;
    }
*/

    /*===============================================================*/
    /* If the user specified the ALL keyword (or OPT A) then we need */
    /* to take the just printed dirname and use it to send to the    */
    /* do_dir() function recursively. It is possible to enter sub-   */
    /* directories even if we specified not to display them!	     */
    /*===============================================================*/
    if ((global->optflags & (ALL_FLAG | OPT_ENTER_FLAG)) &&
	(global->ap.ap_Info.fib_DirEntryType >= 0))
    {
      /*===========================================================*/
      /* Make the call to get the subdirectory. Add 1 to reclvl to */
      /* keep track of the recursion level for proper indentation. */
      /*===========================================================*/
      do_dir(global, reclvl + 1, &previous);

      /*==================================================================*/
      /* MatchNext() returns the flag APF_DIDDIR if it finished examining */
      /* a directory.  We need to reset it at this point because our code */
      /* will exit prematurely if we don't!                               */
      /*==================================================================*/
      global->ap.ap_Flags &= ~APF_DIDDIR;

      /*=================================================================*/
      /* The program may have had an error, or the user may want to quit */
      /* the program.  Either way, we exit the same.			 */
      /*=================================================================*/
      if ((global->optflags & OPT_QUIT_FLAG) ||
	  (global->returncode == RETURN_FAIL))
	goto TimeToGo;
    }

    /*==========================================================*/
    /* If we have a file and we want to print files, we need to */
    /* store the filenames in sorted order for later printing.	*/
    /*==========================================================*/
    if((global->optflags & FILES_FLAG) &&
       (global->ap.ap_Info.fib_DirEntryType < 0)) {

      /*==========================================*/
      /* First, we need to allocate the new node. */
      /*==========================================*/
      fnametemp = AllocVec(sizeof(struct FNameList),
			   MEMF_CLEAR | MEMF_PUBLIC);
      if (fnametemp == NULL)
      {
	/*=======================================*/
	/* If we have very little memory left... */
	/*=======================================*/
	global->errorcode = IoErr();
	PutStr(MSG_OUT_OF_MEM_INC);
	break;
      }

      /*=========================================*/
      /* Initialize the new node filename field. */
      /*=========================================*/
      strncpy(fnametemp->fnl_name, global->ap.ap_Info.fib_FileName, FILENAME_SIZE);
      fnametemp->fnl_filetype = global->ap.ap_Info.fib_DirEntryType;

      /* Insert the new node in alphabetical order within the list of names */
      fnameprev = &fnamehead;
      fnamewalk = fnamehead;
      while (fnamewalk)
      {
          if (stricmp(fnamewalk->fnl_name,fnametemp->fnl_name) > 0)
          {
              fnametemp->fnl_next = fnameprev->fnl_next;
              fnameprev->fnl_next = fnametemp;
              break;
          }
          fnameprev = fnamewalk;
          fnamewalk = fnamewalk->fnl_next;
      }

      if (!fnamewalk)
           fnameprev->fnl_next = fnametemp;

    } /*===== End if(isn't a dir). =====*/

    /*========================================*/
    /* We are ready to examine the next item. */
    /*========================================*/
    matchrc = MatchNext(&global->ap);

    /*============================================================*/
    /* The reason we delete our directory here is because of the  */
    /* need to work around an anomoly in the Match...() routines. */
    /* It seems that if we delete a file or directory out from	  */
    /* under the Match...() functions, we invite a visit from the */
    /* GURU.  NOTE: This workaround was unnecessary with ARP!	  */
    /* We have to have done our next MatchNext() to preserve the  */
    /* integrity of the AnchorPath state variables.		  */
    /*============================================================*/
    if (global->optflags & DELETE_DIR_FLAG)
    {
      lock = CurrentDir(dirlock);
      if (!DeleteFile(global->filename))
      {
	PrintFault(IoErr(), NULL);      /* fixed 11OCT90, err msg bug */
      }
      else
      {
	PutStr(MSG_DELETED);
      }
      CurrentDir(lock);
      global->optflags &= ~DELETE_DIR_FLAG;
    }

  } /*===== End while(!matchrc), the main examination loop. =====*/

  /*========================================*/
  /* Check return codes for control breaks. */
  /*========================================*/
  if (matchrc == ERROR_BREAK)
  {
    global->errorcode = ERROR_BREAK;
    global->returncode = RETURN_WARN;
    global->optflags = OPT_QUIT_FLAG;
    goto TimeToGo;
  }

  /*===============================================================*/
  /* When the user has specified QUIT in the interactive mode,	   */
  /* we need to skirt around code which is unnecessary to execute. */
  /*===============================================================*/
  if (global->optflags & OPT_QUIT_FLAG)
  {
    goto TimeToGo;
  }

  /*===============================================*/
  /* A real error occurred if this check succeeds! */
  /*===============================================*/
  if (matchrc != ERROR_NO_MORE_ENTRIES)
  {
    global->errorcode = IoErr();
  }

  global->returncode = RETURN_OK;	/* (probable) successful execution */


  /*============================================================*/
  /* This hack attempts to fix one of the wildard problems with */
  /* the Match...() calls. I am looking for a real fix, as this */
  /* is broken for certain patterns.				*/
  /*============================================================*/

  /*=======================================================================*/
  /* 26APR91 - put this code in comment, we want behavior consistent with  */
  /*	       the List command                                            */
  /*=======================================================================*/
/*
  if ( !((reclvl == 0) && strchr( (char *)global->opts[OPT_DIR], '/') &&
	 global->ap.ap_Flags & APF_ITSWILD) )
  {
*/
    /*=============================================================*/
    /* Printout routine. This takes a list and prints it unless we */
    /* have suppressed the printing of the filenames.		   */
    /*=============================================================*/
    fnametemp = fnamehead;
    tempvector[2] = NULL;
    column = 1;
    while (fnametemp && (!global->returncode))
    {
      /*======================================================*/
      /* Check for the INTER keyword..	We indent differently */
      /* when in the interactive mode.			      */
      /*======================================================*/
      if (global->optflags & INTER_FLAG)
      {
	/*====================*/
	/* Set up indentation */
	/*====================*/
	tempvector[0] = (reclvl * 5) + 2;
	tempvector[1] = (LONG)fnametemp->fnl_name;
	tempvector[2] = (LONG)"";
#ifdef SEE_LINKS
	if (fnametemp->fnl_filetype == ST_LINKFILE)
	  tempvector[2] = (LONG)" <hl>";
	if (fnametemp->fnl_filetype == ST_SOFTLINK)
	  tempvector[2] = (LONG)" <sl>";
#else
	tempvector[2] = (LONG)"";
#endif
	vsprintf(global->buffer, "%%%lds%%s%%s", tempvector);
	tempvector[0] = (LONG)" ";

	/*======================================================*/
	/* Support for repeating the printing of the filename	*/
	/* after certain commands in the dointeract() function. */
	/*======================================================*/
	global->optflags |= REPEAT_ENTRY_FLAG;
	strncpy(global->filename, fnametemp->fnl_name, FILEPATH_SIZE);
	while(global->optflags & REPEAT_ENTRY_FLAG)
	{
	  VPrintf(global->buffer, tempvector);
	  VPrintf(" ? ", NULL);
	  dointeract(global, dirlock, TYPE_FILE);
	}

	/*=========================================*/
	/* Check on flags returned by dointeract() */
	/*=========================================*/
	if (global->optflags & OPT_BACK_FLAG)
	{
	  global->optflags &= ~OPT_BACK_FLAG;
	  goto TimeToGo;
	}
	if ((global->optflags & OPT_QUIT_FLAG))
	{
	  goto TimeToGo;
	}
      }
      else
      {
	/*===================*/
	/* Give user an out! */
	/*===================*/
	if (CheckSignal(BREAKBITS))
	{
	  global->optflags = OPT_QUIT_FLAG;
	  global->returncode = RETURN_WARN;
	  global->errorcode = ERROR_BREAK;
	  break;
	}

	/*=======================================================*/
	/* We are not interactive, so set up the indentation and */
	/* print the filenames in a two column format (for now). */
	/*=======================================================*/
	strncpy(global->fntemp[column&1], fnametemp->fnl_name, FILENAME_SIZE);
#ifdef SEE_LINKS
	if (fnametemp->fnl_filetype == ST_LINKFILE)
	  strcat(global->fntemp[column&1], " <hl>");
	if (fnametemp->fnl_filetype == ST_SOFTLINK)
	  strcat(global->fntemp[column&1], " <sl>");
#endif
	tempvector[column] = (LONG)global->fntemp[column&1];
	column = 3 - column;
	if ((column == 1) || (fnametemp->fnl_next == NULL))
	{
	  tempvector[0] = (reclvl * 5) + 2;
	  vsprintf(global->buffer, "%%%lds%%-31s  %%s\n", tempvector);
	  tempvector[0] = (LONG)" ";
	  VPrintf(global->buffer, tempvector);
	  tempvector[2] = NULL;
	}
      }

      /*================*/
      /* Walk the list. */
      /*================*/
      fnametemp = fnametemp->fnl_next;
/*
    }
*/
  }  /*===== End of if(matchrc). End of file printing routine =====*/

  /*=============================================================*/
  /* TimeToGo is a place that the code can jump to IF an exit	 */
  /* is in order.  All cleanup functions are assured to execute. */
  /*=============================================================*/

TimeToGo:
  /*==============================*/
  /* File Name List cleanup code. */
  /*==============================*/
  while (fnamehead != NULL)
  {
    fnametemp = fnamehead->fnl_next;
    FreeVec(fnamehead);
    fnamehead = fnametemp;
  }
  if (dirlock) UnLock(dirlock);
}


/*======================================================================*/
/*  Function:  dointeract()                                             */
/*   Purpose:  This function does our interactive functions for us.	*/
/*	       Since the context is single file oriented, we just need	*/
/*	       to pass the filename.					*/
/*======================================================================*/

void dointeract(struct Global *global, BPTR parent, int filetype)
{
  struct RDArgs      *argsptr;
  UBYTE 	     *execbuff;
  LONG		      opts[INTER_COUNT];
  char		      chr[1];
  BPTR		      fileptr,
		      lock;

  /*===========================*/
  /* Initialize main variables */
  /*===========================*/
  global->returncode = RETURN_OK;
  global->optflags |= REPEAT_ENTRY_FLAG;
  memset(&opts, 0, sizeof(opts));

  Flush(Output());

  /*===================================*/
  /* Allocate memory for general usage */
  /*===================================*/
  if (
      !(execbuff = AllocVec(EXEC_BUFF_SIZE, MEMF_CLEAR | MEMF_PUBLIC))
     )
  {
    /*=============================*/
    /* If our allocation failed... */
    /*=============================*/
    global->errorcode = IoErr();
    PutStr(MSG_OUT_OF_MEM_INC);
    global->returncode = RETURN_FAIL;
  }
  else
  {
    /*=====================================================*/
    /* If our memory is allocated, get our args from StdIn */
    /*=====================================================*/
    if (filetype == TYPE_DIR)
    {
      /*===============================================*/
      /* If the name is a directory, use this template */
      /*===============================================*/
      argsptr = ReadArgs(INTER_DIR_TEMPLATE, opts, 0);
    }
    else
    {
      /*==================================================*/
      /* If the name is a file, use this template instead */
      /*==================================================*/
      argsptr = ReadArgs(INTER_FILE_TEMPLATE, opts, 0);
    }

    /*===================================*/
    /* Check to see if ReadArgs() failed */
    /*===================================*/
    if (argsptr == NULL)
    {
      global->errorcode = IoErr();
      global->returncode = RETURN_FAIL;
    }
    else
    {
      /*====================*/
      /*  Type file routine */
      /*====================*/
      if (opts[OPT_INTER_ENTER_TYPE] && (filetype == TYPE_FILE))
      {
	/*===================*/
	/* Open file to type */
	/*===================*/
	lock = CurrentDir(parent);
	if (!(fileptr = Open(global->filename, MODE_OLDFILE)))
	{
	  global->errorcode = IoErr();
	  global->returncode = RETURN_FAIL;
	}
	else
	{
	  /*=========================================================*/
	  /* Loop through file, reading and then writing a character */
	  /*=========================================================*/
	  while ((*chr = FGetC(fileptr)) != -1)
	  {
	    /*===================*/
	    /* Give user an out! */
	    /*===================*/
	    if (CheckSignal(BREAKBITS))
	    {
	      PrintFault(ERROR_BREAK, NULL);
	      break;
	    }

	    /*================================*/
	    /* Output the character to stdout */
	    /*================================*/
	    WriteChars(chr, 1);
	  }

	  /*==================================================*/
	  /* Replace previous input filehandle and close file */
	  /*==================================================*/
	  Close(fileptr);
	}
	CurrentDir(lock);
	goto RepeatOut;
      }

      /*===================*/
      /* Command execution */
      /*===================*/
      if (opts[OPT_INTER_COM])  /*= user MUST specify C or COM =*/
      {
	if (opts[OPT_INTER_COMMAND])
	{
	  /*=====================================*/
	  /* Everything is ready, do the command */
	  /*=====================================*/
	  Execute((char *)opts[OPT_INTER_COMMAND], NULL, NULL);
	  PutStr("\n");
	  goto RepeatOut;
	}
	else
	{
	  /*=============================================================*/
	  /* Provide a command prompt for those who apparently need one! */
	  /*=============================================================*/
	  PutStr(MSG_COMMAND_PROMPT);
	  Flush(Output());

	  if(FGets(Input(), execbuff, BUFFER_SIZE))
	  {
		Execute(execbuff, NULL, NULL);
		PutStr("\n");
	  }
	  goto RepeatOut;
	}
      }
      else
      {
	/*===========================================================*/
	/* If the user typed gobbledygook, this traps it as an error */
	/* because any unknown syntax in the rest of the template    */
	/* will end up here!					     */
	/*===========================================================*/
	if (opts[OPT_INTER_COMMAND])
	{
	  PutStr(MSG_INVALID_RESPONSE);
	  goto RepeatOut;
	}
      }

      /*==============*/
      /* Quit routine */
      /*==============*/
      if (opts[OPT_INTER_QUIT])
      {
	global->optflags = OPT_QUIT_FLAG;
	goto NoRepeatOut;
      }

      /*========================*/
      /* Backup one dir routine */
      /*========================*/
      if (opts[OPT_INTER_BACK])
      {
	global->ap.ap_Flags |= APF_DIDDIR;
	global->optflags |= OPT_BACK_FLAG;
	goto NoRepeatOut;
      }

      /*=======================*/
      /* Enter one dir routine */
      /*=======================*/
      if (opts[OPT_INTER_ENTER_TYPE] && (filetype == TYPE_DIR))
      {
	global->optflags |= OPT_ENTER_FLAG;
	goto NoRepeatOut;
      }

      /*=====================*/
      /* Delete file routine */
      /*=====================*/
      if (opts[OPT_INTER_DELETE])
      {
	if (filetype == TYPE_FILE)
	{
	  /*============================================================*/
	  /* Set up to delete the file by setting the current directory */
	  /*============================================================*/
	  lock = CurrentDir(parent);
	  if(!DeleteFile(global->filename))
	  {
	    global->errorcode = IoErr();
	    global->returncode = RETURN_ERROR;
	  }
	  else
	  {
	    PutStr(MSG_DELETED);
	  }
	  CurrentDir(lock);
	}
	else  /*= filetype == TYPE_DIR =*/
	{
	  global->optflags |= DELETE_DIR_FLAG;
	}
	goto NoRepeatOut;
      }

      /*==================================================================*/
      /* Control reaches here if the user only pressed the return key or  */
      /* was "goto"d here.  We need to reset the repeat flag so we can go */
      /* to another file!						  */
      /*==================================================================*/
NoRepeatOut:

      /*==================================================================*/
      /* Reset the repeat flag because we want to go to the next filename */
      /*==================================================================*/
      global->optflags &= ~REPEAT_ENTRY_FLAG;

RepeatOut:

      /*====================================*/
      /* Clean up after the ReadArgs() call */
      /*====================================*/
      FreeArgs(argsptr);
    }

    /*===============================*/
    /* Give back the memory we used. */
    /*===============================*/
    FreeVec(execbuff);
  }
}


/*==============================================================*/
/* Function: prbuf						*/
/*  Purpose: The following stub routine is called from RawDoFmt */
/*	     for each character in the string.	At invocation,	*/
/*	     we have:						*/
/*		D0 - next character to be formatted		*/
/*		A3 - pointer to data buffer			*/
/*==============================================================*/
static void __regargs prbuf(char c)
{
  __builtin_emit(0x16c0);   /* move.b D0,(A3)+ */
}


/*====================================================================*/
/* Function: vsprintf						      */
/*  Purpose: This is a tiny implementation of sprintf.	Note that it  */
/*	     assumes that the buffer is large enough to hold any      */
/*	     formatted string you ask it to generate.		      */
/*====================================================================*/
int vsprintf(char *buf, char *ctl, long args[])
{
  RawDoFmt(ctl, args, prbuf, buf);
  return(strlen(buf));
}
