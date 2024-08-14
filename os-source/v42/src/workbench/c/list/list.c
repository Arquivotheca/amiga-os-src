
#include <internal/commands.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <envoy/accounts.h>
#include "list_rev.h"
/* defines for ReadArgs() */

#define MSG_ARGS_EXCL	  "'DATES' and 'NODATES' are mutually exclusive\n"
#define MSG_CANT_OPEN	  "*** Can't open %s - "
#define MSG_INVALID_SINCE_UPTO \
"*** Invalid 'UPTO' or 'SINCE' parameter - ignored\n"
#define MSG_DIR 	  "Directory \"%s\" "
#define MSG_DIR_EMPTY	  "is empty\n"
#define MSG_DIR_ON	  "on %s %s\n"
#define MSG_DIR_TYPE	  "Dir"
#define MSG_NO_INFO	  "No information for \"%s\""
#define MSG_EMPTY	  "empty"
#define MSG_TOTAL	  "\nTOTAL: "
#define MSG_COUNT	  "%ld %s - "
#define MSG_BLOCK	  "%ld %s used\n"
#define MSG_MANY_FILES 	  "files"
#define MSG_ONE_FILE	  "file"
#define MSG_MANY_DIRS	  "directories"
#define MSG_ONE_DIR	  "directory"
#define MSG_MANY_BLOCKS   "blocks"
#define MSG_ONE_BLOCK	  "block"
#define MSG_PROTBITS	  "hsparwed"
#define MSG_USEROTHERBITS "rwed rwed"
#define MSG_NO_MATCH  	  "No match found\n"
#define MSG_CANT_LIST	  "\" cannot be listed: not a FileSystem device\n"
#define MSG_INVALID_DS	  "<invalid>"
#define MSG_BAD_SORT_TYPE "'SORT' keyword requires any of N=NAME,S=SIZE,D=DATE,RN=REVERSENAME,RS=REVERSESIZE,RD=REVERSEDATE\n"
#define MSG_NO_OWNER      "<No Owner>"
#define MSG_NO_GROUP      "<No Group>"
#define TEMPLATE "DIR/M,P=PAT/K,KEYS/S,DATES/S,NODATES/S,TO/K,SUB/K,SINCE/K,\
UPTO/K,QUICK/S,BLOCK/S,NOHEAD/S,FILES/S,DIRS/S,LFORMAT/K,SORT/K,USERS/S,GROUPS/S,ALL/S" CMDREV

#define OPT_DIR      0
#define OPT_PAT      1
#define OPT_KEYS     2
#define OPT_DATES    3
#define OPT_NODATES  4
#define OPT_TO	     5
#define OPT_SUB      6
#define OPT_SINCE    7
#define OPT_UPTO     8
#define OPT_QUICK    9
#define OPT_BLOCK   10
#define OPT_NOHEAD  11
#define OPT_FILES   12
#define OPT_DIRS    13
#define OPT_LFORMAT 14
#define OPT_SORT    15
#define OPT_USERS   16
#define OPT_GROUPS  17
#define OPT_ALL     18
#define OPT_COUNT   19

/*---------------------------------------------------------------------------*/
/* Define the signals we wish the pattern matcher to stop on.		     */
/*---------------------------------------------------------------------------*/
#define BREAK_SIGS SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D

/*---------------------------------------------------------------------------*/
/* Define the minimum buffer size necessary to format output.		     */
/*---------------------------------------------------------------------------*/
#define MIN_BUFFER 32

/*---------------------------------------------------------------------------*/
/* DirList structure is used to store directory names for ALL option.	     */
/* and to store entry info for the SORT option                               */
/*---------------------------------------------------------------------------*/
struct DirList {
  struct DirList *next;
  char dname[ENVMAX];
};

struct DirEntry
{
    struct Node      de_Link;
    UWORD            de_Pad;
    LONG             de_FileSize;
    struct DateStamp de_FileDate;
    STRPTR           de_FileName;
};


/* order of this template must match that of the definitions for SortTypes below */
#define SORT_TEMPLATE "N=NAME,D=DATE,S=SIZE,RN=REVERSENAME,RD=REVERSEDATE,RS=REVERSESIZE"

enum SortTypes
{
    ST_NAME,
    ST_DATE,
    ST_SIZE,
    ST_REVERSENAME,
    ST_REVERSEDATE,
    ST_REVERSESIZE
};

struct SortOrder
{
    struct MinNode so_Link;
    enum SortTypes so_Type;
};


/*---------------------------------------------------------------------------*/
/* Global structure is used to pass info between the two routines.	     */
/*---------------------------------------------------------------------------*/
struct Global {
  struct Library *SysBase;
  struct Library *DOSBase;
  struct Library *UtilityBase;
  long opts[OPT_COUNT];
  long dircount;
  long filecount;
  long numblocks;
  struct DirList *dhead;
  long totaldirs;
  long totalfiles;
  long totalblocks;
  char *lformat;			/* print format specifier string     */
  char *lorder; 			/* order of values for above	     */
  long *listfields;
  long notfirst;
  long scount;
  long rc;
  struct DateStamp now;
  struct DateTime datetime;
  char comdef[4];
  char dow[LEN_DATSTRING];
  char date[LEN_DATSTRING];
  char time[LEN_DATSTRING];
  UBYTE patbuf[ENVMAX*2];
  UBYTE subbuf[ENVMAX*2];
  struct MinList SortOrder;
};

static int list_dir(struct Global *, char *, struct AnchorPath *,
	     struct DateStamp *, struct DateStamp *);
static void PrintDirStats(struct Global *);
static BOOL UIDToName(struct Global *, UWORD ID, STRPTR target, STRPTR dir);
static BOOL GIDToName(struct Global *, UWORD ID, STRPTR target, STRPTR dir);

/*---------------------------------------------------------------------------*/
/* Main program 							     */
/*---------------------------------------------------------------------------*/
int cmd_list(void)
{
  struct Library *SysBase;
  struct Library *DOSBase;
  struct RDArgs *rdargs;
  struct AnchorPath *ua;
  struct DateStamp since, upto;
  struct Global *global;
  struct DirList *dtemp;
  char curbuf[ENVMAX];
  char subtemp[ENVMAX];
  char **argptr;
  char *dummy[2];
  BPTR tofile, oldoutput;
  long i, j, lfindex, lindex, scount, rc;
  char *temp;
  char c;
  struct SortOrder *so;
  STRPTR ptr, start;
  LONG type;

  rc = RETURN_FAIL;

  /*-------------------------------------------------------------------------*/
  /* 'dummy' is used to initialize the 'argptr' array, in case the user      */
  /* didn't type any dir names.                                              */
  /*-------------------------------------------------------------------------*/
  dummy[0] = "";
  dummy[1] = NULL;

  /*-------------------------------------------------------------------------*/
  /* Try to open the 2.0 dos.library.					     */
  /*-------------------------------------------------------------------------*/
  SysBase = (*((struct Library **) 4));

  if ((DOSBase = OpenLibrary(DOSLIB, DOSVER))) {

    /*-----------------------------------------------------------------------*/
    /* Create our global data area.					     */
    /*-----------------------------------------------------------------------*/
    if ((global = (struct Global *)
	    AllocVec(sizeof(struct Global), MEMF_CLEAR)) == NULL) {
      PrintFault(IoErr(),NULL);
      goto panic;
    }

    /*-----------------------------------------------------------------------*/
    /* Save the DOSBase pointer for later use.				     */
    /*-----------------------------------------------------------------------*/
    global->DOSBase = DOSBase;
    global->SysBase = SysBase;
    global->UtilityBase = OpenLibrary("utility.library",0);

    NewList((struct List *)&global->SortOrder);

    /*-----------------------------------------------------------------------*/
    /* Now try to parse the user's input.                                    */
    /*-----------------------------------------------------------------------*/
    rdargs = ReadArgs(TEMPLATE, global->opts, NULL);

    /*-----------------------------------------------------------------------*/
    /* If we had an error parsing, rdargs should be null.		     */
    /*-----------------------------------------------------------------------*/
    if (rdargs == NULL) {
      PrintFault(IoErr(),NULL);
      global->rc = RETURN_FAIL;
    } else {

       if (ptr = (STRPTR)global->opts[OPT_SORT])
       {
           while (*ptr)
           {
               start = ptr;
               while (*ptr && (*ptr != ','))
                   ptr++;

               c    = *ptr;
               *ptr = 0;
               type = FindArg(SORT_TEMPLATE,start);
               *ptr = c;
               if (c)
                   ptr++;

               if (type < 0)
               {
                   PutStr(MSG_BAD_SORT_TYPE);
                   goto sortexit;
               }
               else
               {
                   so = AllocVec(sizeof(struct SortOrder),MEMF_ANY);
                   if (!so)
                   {
                       PrintFault(ERROR_NO_FREE_STORE,NULL);
                   }

                   so->so_Type = type;
                   AddTail((struct List *)&global->SortOrder,(struct Node *)so);
               }
           }
       }

      /*---------------------------------------------------------------------*/
      /* If they gave an LFORMAT option, try to parse it the same way the    */
      /* BCPL version of list does.					     */
      /*---------------------------------------------------------------------*/
      temp = (char *)global->opts[OPT_LFORMAT];

      /*---------------------------------------------------------------------*/
      /* Grab some memory for workspace.				     */
      /*---------------------------------------------------------------------*/
      i = (temp != NULL) ? strlen(temp) : MIN_BUFFER;
      if ((global->lformat = AllocVec(((i<<1)+i+1),
				     MEMF_CLEAR))==NULL) {
	PrintFault(IoErr(), NULL);
	goto listexit;
      }
      global->lorder = global->lformat + (i<<1) + 1;

      if (temp != NULL) {
	for (global->scount=0, i=0; i<strlen(temp); i++) {
	  if ((temp[i] == '%') && ((temp[i+1] & 0x5f) == 'S')) {
	    temp[i+1] = 's';
	    global->scount++;
	  }
	}
	for (i=0,lfindex=0,lindex=0,scount=0; i<strlen(temp); i++) {
	  subtemp[0]=0;
	  global->lformat[lfindex++] = temp[i];
	  if (temp[i] == '%') {
	    j=0;
	    while ((temp[i+1] == '-') ||
		   ((temp[i+1] >= '0') && (temp[i+1] <= '9')) ||
		   (temp[i+1] == '.')) {
	      subtemp[j++] = temp[i+1];
	      i++;
	    }
	    subtemp[j]=0;
	    c = (temp[i+1] & 0x5f);
	    switch (c) {
	      case 'A':
	      case 'B':
	      case 'C':
	      case 'D':
	      case 'E':
	      case 'F':
	      case 'K':
	      case 'L':
	      case 'M':
	      case 'N':
	      case 'P':
	      case 'T':
	      case 'G':
	      case 'U':
	      case 'V':
		temp[i+1] = 's';
		global->lorder[lindex++] = (c+0x20);
		break;
	      case 'S':
		scount++;
		if ((global->scount > 3) && ((scount==1) || (scount == 3))) {
		  global->lorder[lindex++] = 'p';
		} else {
		  if ((global->scount > 1) && (scount==1)) {
		    global->lorder[lindex++] = 'p';
		  } else {
		    global->lorder[lindex++] = 'n';
		  }
		}
		break;
	      default:
		global->lformat[lfindex++] = '%';
		break;
	    }
	    if (subtemp[0]) {
	      j=0;
	      while (subtemp[j]) {
		global->lformat[lfindex++] = subtemp[j++];
	      }
	      subtemp[0]=0;
	    }
	  }
	}
      } else {

	/*-------------------------------------------------------------------*/
	/* They DIDN'T give an LFORMAT option; let's build our output string */
	/* anyway.  Makes life simpler (and faster!) in the other routine.   */
	/*-------------------------------------------------------------------*/
	i = 0;				/* use as index to 'lorder' string.  */

	/*-------------------------------------------------------------------*/
	/* If not using the LFORMAT option, we ALWAYS! print the name.	     */
	/*-------------------------------------------------------------------*/
	global->lorder[i++] = 'n';

	/*-------------------------------------------------------------------*/
	/* QUICK option handling.					     */
	/*-------------------------------------------------------------------*/
	if (global->opts[OPT_QUICK] == NULL) {
	  if (global->opts[OPT_KEYS] != NULL) {
	    strcpy(global->lformat, "%-17s [%5s]");
	    global->lorder[i++] = 'k';
	  } else {
	    strcpy(global->lformat, "%-24s");
	  }
	  strcat(global->lformat, " %7s");
	  if (global->opts[OPT_BLOCK] != NULL) {
	    global->lorder[i++] = 'b';
	  } else {
	    global->lorder[i++] = 'l';
	  }

          if (global->opts[OPT_USERS] != NULL) {
            strcat(global->lformat," %-10.10s");
            global->lorder[i++] = 'u';
          }

          if (global->opts[OPT_GROUPS] != NULL) {
            strcat(global->lformat," %-10.10s");
            global->lorder[i++] = 'g';
          }

	  strcat(global->lformat, " %s");
	  global->lorder[i++] = 'a';

          if (global->opts[OPT_GROUPS] != NULL) {
            strcat(global->lformat," %s");
            global->lorder[i++] = 'v';
          }

	  if (global->opts[OPT_NODATES] == NULL) {
	    strcat(global->lformat, " %-9s %s");
	    global->lorder[i++] = 'd';
	    global->lorder[i++] = 't';
	  }
	  strcpy(global->comdef,"\n: ");
	  strcat(global->lformat, "%s");
	  global->lorder[i] = 'c';
	} else {
	  if (global->opts[OPT_DATES] != NULL) {
	    strcpy(global->lformat, "%-25s %-9s %s");
	    global->lorder[i++] = 'd';
	    global->lorder[i] = 't';
	  } else {
	    strcpy(global->lformat, "%s");
	  }
	}
      }

      /*---------------------------------------------------------------------*/
      /* Now that we know how many variables we need to print, we need to    */
      /* create an array to hold at least that many variables.	We'll make   */
      /* sure we have at least the minimum necessary to handle any default   */
      /* printing as well.						     */
      /*---------------------------------------------------------------------*/
      if ((global->listfields = AllocVec(((strlen(global->lorder) > 4) ?
					 strlen(global->lorder) : 4)
					 * sizeof(long),
					 MEMF_ANY))==NULL) {
	PrintFault(IoErr(),NULL);
	goto listexit;
      }

      /*---------------------------------------------------------------------*/
      /* 'DATES' and 'NODATES' are the only mutually-exclusive options.      */
      /*---------------------------------------------------------------------*/
      if ((global->opts[OPT_DATES] != NULL) &&
	  (global->opts[OPT_NODATES] != NULL)) {
	PutStr(MSG_ARGS_EXCL);
      } else {
	/*-------------------------------------------------------------------*/
	/* If the user told us where to send the output, try to make sure it */
	/* at least gets started in that direction.			     */
	/*-------------------------------------------------------------------*/
	if (global->opts[OPT_TO] != NULL) {
	  if ((tofile = Open((char *)global->opts[OPT_TO],
			     MODE_NEWFILE)) == NULL) {
	    VPrintf(MSG_CANT_OPEN, &global->opts[OPT_TO]);
	    PrintFault(IoErr(),NULL);
	    goto listexit;
	  } else {
	    oldoutput = SelectOutput(tofile);
	  }
	}
	if ((ua = AllocVec(sizeof(struct AnchorPath)+ENVMAX,
			   MEMF_CLEAR)) == NULL) {
	  PrintFault(IoErr(), NULL);
	} else {

	  /*-----------------------------------------------------------------*/
	  /* Grab the current DateStamp, for future use.		     */
	  /*-----------------------------------------------------------------*/
	  DateStamp((struct DateStamp *)&global->now);

	  /*-----------------------------------------------------------------*/
	  /* If they gave a SINCE option, try to set up for its use.	     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_SINCE] != NULL) {
	    memcpy((char *)&global->datetime,(char *)&global->now,
		   sizeof(struct DateStamp));
	    global->datetime.dat_Format = FORMAT_DOS;
	    global->datetime.dat_Flags = DTF_SUBST;
	    global->datetime.dat_StrDate = (char *)global->opts[OPT_SINCE];
	    global->datetime.dat_StrTime = "00:00:00";
	    if (!StrToDate(&global->datetime)) {
	      global->datetime.dat_StrDate = NULL;
	      global->datetime.dat_StrTime = (char *)global->opts[OPT_SINCE];
	      if (!StrToDate(&global->datetime)) {
		PutStr(MSG_INVALID_SINCE_UPTO);
	      } else {
		memcpy((char *)&since,(char *)&global->datetime,
		       sizeof(struct DateStamp));
	      }
	    } else {
	      memcpy((char *)&since,(char *)&global->datetime,
		     sizeof(struct DateStamp));
	    }
	  }

	  /*-----------------------------------------------------------------*/
	  /* Check for an UPTO option, and set it up if present.	     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_UPTO] != NULL) {
	    memcpy((char *)&global->datetime,(char *)&global->now,
		   sizeof(struct DateStamp));
	    global->datetime.dat_Format = FORMAT_DOS;
	    global->datetime.dat_Flags = DTF_SUBST;
	    global->datetime.dat_StrDate = (char *)global->opts[OPT_UPTO];
	    global->datetime.dat_StrTime = "23:59:59";
	    if (!StrToDate(&global->datetime)) {
	      global->datetime.dat_StrDate = NULL;
	      global->datetime.dat_StrTime = (char *)global->opts[OPT_UPTO];
	      if (!StrToDate(&global->datetime)) {
		PutStr(MSG_INVALID_SINCE_UPTO);
	      } else {
		memcpy((char *)&upto,(char *)&global->datetime,
		       sizeof(struct DateStamp));
	      }
	    } else {
	      memcpy((char *)&upto,(char *)&global->datetime,
		     sizeof(struct DateStamp));
	    }
	  }

	  /*-----------------------------------------------------------------*/
	  /* Remember 'dummy', from above?  Use it if necessary.             */
	  /*-----------------------------------------------------------------*/
	  if ((argptr = (char **)global->opts[OPT_DIR]) == NULL) {
	    argptr = dummy;
	  }

	  /*-----------------------------------------------------------------*/
	  /* Start parsing the Multi-Args!				     */
	  /*-----------------------------------------------------------------*/
	  while ((*(argptr) != NULL) || (global->dhead != NULL)) {

	    /*---------------------------------------------------------------*/
	    /* If there is anything in 'global->dhead' (which can ONLY occur */
	    /* if the ALL option was used!) we need to take care of any sub- */
	    /* directories before we move on.				     */
	    /*---------------------------------------------------------------*/
	    if (global->dhead != NULL) {
	      strcpy(curbuf, global->dhead->dname);
	      dtemp = global->dhead->next;
	      FreeVec(global->dhead);
	      global->dhead = dtemp;
	    } else {
	      curbuf[0] = 0;
	      AddPart(curbuf, *(argptr++), ENVMAX);
	    }

	    /*---------------------------------------------------------------*/
	    /* Handle PAT and SUB keywords here.			     */
	    /*---------------------------------------------------------------*/
	    if (global->opts[OPT_PAT] != NULL) {
	      ParsePatternNoCase((UBYTE *)global->opts[OPT_PAT],
				 global->patbuf, ENVMAX << 1);
	    } else {
	      if (global->opts[OPT_SUB] != NULL) {
		strcpy(subtemp, "#?");
		strcat(subtemp, (char *)global->opts[OPT_SUB]);
		strcat(subtemp, "#?");
		ParsePatternNoCase(subtemp, global->subbuf, ENVMAX << 1);
	      }
	    }

	    /*---------------------------------------------------------------*/
	    /* Call the match loop.					     */
	    /*---------------------------------------------------------------*/
	    if (list_dir(global,curbuf,ua,&since,&upto) > RETURN_WARN) {
	      break;
	    }

	    if (global->rc != RETURN_OK) {
	      break;
	    }
	  }

	  /*-----------------------------------------------------------------*/
	  /* If we are NOT using the LFORMAT option and we DID use ALL...    */
	  /* and they didn't request not to see it                           */
	  /*-----------------------------------------------------------------*/
	  if ((global->opts[OPT_LFORMAT] == NULL) &&
	      (global->opts[OPT_NOHEAD] == NULL) &&
	      (global->opts[OPT_ALL] != NULL)) {

	    /*---------------------------------------------------------------*/
	    /* Check to see if we HAVE any stats to print...		     */
	    /*---------------------------------------------------------------*/
	    if ((global->totalfiles+global->totaldirs) != NULL) {

	      PutStr(MSG_TOTAL);

	      /*-------------------------------------------------------------*/
	      /* Print total statistics, then exit.			     */
	      /*-------------------------------------------------------------*/
	      if ((global->opts[OPT_DIRS] == NULL) &&
		  (global->totalfiles != NULL)) {
		global->listfields[0] = global->totalfiles;
		if (global->totalfiles > 1L) {
		  global->listfields[1] = (long)MSG_MANY_FILES;
		} else {
		  global->listfields[1] = (long)MSG_ONE_FILE;
		}
		VPrintf(MSG_COUNT, global->listfields);
	      }
	      if ((global->opts[OPT_FILES] == NULL) &&
		  (global->totaldirs != NULL)) {
		global->listfields[0] = global->totaldirs;
		if (global->totaldirs > 1L) {
		  global->listfields[1] = (long)MSG_MANY_DIRS;
		} else {
		  global->listfields[1] = (long)MSG_ONE_DIR;
		}
		VPrintf(MSG_COUNT, global->listfields);
	      }
	      global->listfields[0] = global->totalblocks;
	      if (global->totalblocks > 1L) {
		global->listfields[1] = (long)MSG_MANY_BLOCKS;
	      } else {
		global->listfields[1] = (long)MSG_ONE_BLOCK;
	      }
	      VPrintf(MSG_BLOCK, global->listfields);
	    }
	  }
	}
	FreeVec(ua);

	/*-------------------------------------------------------------------*/
	/* If we have re-directed the output (at the user's request), we     */
	/* have to set the output back to where it was when we started.      */
	/*-------------------------------------------------------------------*/
	if (global->opts[OPT_TO] != NULL) {
	  SelectOutput(oldoutput);
	  Close(tofile);
	}
      }
listexit:
      FreeArgs(rdargs);
      FreeVec(global->listfields);
      FreeVec(global->lformat);
    }

sortexit:
    while (so = (struct SortOrder *)RemHead((struct List *)&global->SortOrder))
        FreeVec(so);

    /*-----------------------------------------------------------------------*/
    /* If there is anything in the DirList, we have to clean it up before we */
    /* exit!								     */
    /*-----------------------------------------------------------------------*/
    while (global->dhead) {
      dtemp = global->dhead->next;
      FreeVec(global->dhead);
      global->dhead = dtemp;
    }

    CloseLibrary(global->UtilityBase);

    rc = global->rc;
    FreeVec(global);

panic:
    CloseLibrary(DOSBase);
  } else {
    OPENFAIL;
  }
  return(rc);
}

#define DOSBase     global->DOSBase
#define SysBase     global->SysBase
#define UtilityBase global->UtilityBase

extern void __builtin_emit(unsigned short);


void SpitChar(char c)
{
   __builtin_emit(0x16c0);   /* move.b D0,(A3)+ */
}

void CountChar(char c)
{
   __builtin_emit(0x5293);   /* addq.l #1,(a3) */
}

/*---------------------------------------------------------------------------*/
/* This is the routine which parses the directories, deciding what to print  */
/* based on the user input.						     */
/*---------------------------------------------------------------------------*/
static int list_dir(global, dirname, ua, since, upto)
struct Global *global;
char *dirname;
struct AnchorPath *ua;
struct DateStamp *since, *upto;
{
  long temprc=0;
  char namebuf[ENVMAX];
  char dirbuf[ENVMAX];
  char fulldir[ENVMAX];
  long entrytype;
  long rc;
  long didhdr;
  struct DirList *dhead, *dtemp, *dtail;
  struct DirEntry *de;
  struct DirEntry *node;
  struct List fileList;
  char combuf[ENVMAX];
  char minusbuf[32];
  char extbuf[32];
  char blkbuf[16];
  char keybuf[16];
  char lenbuf[16];
  char protbits[9];
  char userotherbits[11];
  char *tptr;
  WORD i;
  long lindex;
  long count;
  struct SortOrder *sortOrder;
  enum SortTypes    sortType;
  LONG              cmp;
  char username[32], groupname[32];

  dhead = dtail = NULL;
  didhdr = 0L;

  NewList(&fileList);

  /*-------------------------------------------------------------------------*/
  /* Set the default return code.					     */
  /*-------------------------------------------------------------------------*/
  global->rc = RETURN_FAIL;
  memset((char *)ua, 0, sizeof(struct AnchorPath));

  /*-------------------------------------------------------------------------*/
  /* Set up the AnchorPath structure for use in the pattern match routines.  */
  /*-------------------------------------------------------------------------*/
  ua->ap_BreakBits = SIGBREAKF_CTRL_C;
#if THEY_EVER_IMPLEMENT_THE_FLAG
  ua->ap_Flags |= APF_DOWILD|APF_FollowHLinks;
#else
  ua->ap_Flags |= APF_DOWILD;
#endif
  ua->ap_Strlen = ENVMAX;

  if ((rc=MatchFirst(dirname,ua))==NULL) {
    if (!IsFileSystem(dirname)) {
      PutStr("\"");
      PutStr(dirname);
      PutStr(MSG_CANT_LIST);
      goto ldexit;
    }

    if ((ua->ap_Info.fib_DirEntryType >= 0) && !(ua->ap_Flags&APF_ITSWILD)) {

      ua->ap_Flags |= APF_DODIR;
      if (((rc=MatchNext(ua)) == NULL) && (ua->ap_Flags & APF_DIDDIR)) {

	ua->ap_Flags &= ~APF_DIDDIR;
	if ((global->opts[OPT_LFORMAT] == NULL) &&
	    (global->opts[OPT_NOHEAD] == NULL) &&
	    (global->notfirst)) {
	  PutStr("\n");
	}
	global->notfirst = 1;
	temprc = (long)dirname;
	if ((global->opts[OPT_LFORMAT]==NULL)&&(global->opts[OPT_NOHEAD]==NULL)) {
	  VPrintf(MSG_DIR, &temprc);
	  PutStr(MSG_DIR_EMPTY);
	}
	global->rc = RETURN_OK;
	goto ldexit;
      }
    }
  }

  if(rc)temprc = IoErr();
  if (temprc == ERROR_NO_MORE_ENTRIES) {
    global->rc = RETURN_OK;
    goto ldexit;
  }
  if (temprc == ERROR_BREAK) {
    PrintFault(IoErr(),NULL);
    global->rc = RETURN_WARN;
    goto ldexit;
  }

  if (temprc != NULL) {
    temprc = (long)dirname;
    if ((global->opts[OPT_LFORMAT]==NULL)&&(global->opts[OPT_NOHEAD]==NULL))
	VPrintf(MSG_NO_INFO, &temprc);
    PrintFault(IoErr(), "");
    goto ldexit;
  }

  else {

    /*-----------------------------------------------------------------------*/
    /* Finally, the meat of this routine --- the loop which processes each   */
    /* file.								     */
    /*-----------------------------------------------------------------------*/
    ua->ap_Flags &= ~APF_DIDDIR;
    do {

      /*---------------------------------------------------------------------*/
      /* First check to see if MatchNext() did an internal dir change.       */
      /*---------------------------------------------------------------------*/
      if (ua->ap_Flags & APF_DirChanged) {

        while (de = (struct DirEntry *)RemHead(&fileList))
        {
            PutStr(de->de_Link.ln_Name);
            PutStr("\n");

            if (CheckSignal(SIGBREAKF_CTRL_C) != 0L)
            {
                Result2(ERROR_BREAK);
                rc = 1;
                goto breakloop;
            }

            FreeVec(de);
        }

	/*-------------------------------------------------------------------*/
	/* The dir has changed... print stats for the last directory, and    */
	/* reset so we print everything correctly!			     */
	/*-------------------------------------------------------------------*/
	PrintDirStats(global);
	didhdr = 0L;
      }

      if (ua->ap_Flags & APF_DIDDIR) {
	ua->ap_Flags &= ~APF_DIDDIR;
/*
	rc=0;
	temprc = ERROR_NO_MORE_ENTRIES;
	break;
*/
	continue;
      }

      /*---------------------------------------------------------------------*/
      /* If we haven't printed a header for this directory yet...            */
      /*---------------------------------------------------------------------*/
      if (didhdr == 0L) {

	/*-------------------------------------------------------------------*/
	/* Get the name of the PARENT of the current file; we need to print  */
	/* it if we print a header, or if the user specified the appropriate */
	/* LFORMAT option.						     */
	/*-------------------------------------------------------------------*/
	/* For regular printing, we need a relative path...		     */
	/*-------------------------------------------------------------------*/
	strcpy(dirbuf, ua->ap_Buf);
	*(PathPart(dirbuf)) = 0;
	strcpy(namebuf, dirbuf);

	/*-------------------------------------------------------------------*/
	/* ...but, if you use %f in LFORMAT, we need the Full path.	     */
	/*-------------------------------------------------------------------*/
	NameFromLock(ua->ap_Current->an_Lock, fulldir, ENVMAX);
	AddPart(fulldir, "", ENVMAX);

	/*-------------------------------------------------------------------*/
	/* Add the trailing ':' or '/' to the directory name, for possible   */
	/* printing use.						     */
	/*-------------------------------------------------------------------*/
	AddPart(dirbuf, "", ENVMAX);

	/*-------------------------------------------------------------------*/
	/* Print a header if we aren't specifically told not to, and we are  */
	/* NOT using the LFORMAT option, and they didn't give us a non-      */
	/* ambiguous fully qualified filespec.				     */
	/*-------------------------------------------------------------------*/
	if ((namebuf[0] != 0) &&
	    (global->opts[OPT_NOHEAD] == NULL) &&
	    (global->opts[OPT_QUICK] == NULL) &&
	    (global->opts[OPT_LFORMAT] == NULL)) {
	  if (global->notfirst) {
	    PutStr("\n");
	  }
	  global->listfields[0] = (long)namebuf;
	  VPrintf(MSG_DIR, global->listfields);
	  memcpy((char *)&global->datetime,(char *)&global->now,
		 sizeof(struct DateStamp));
	  global->datetime.dat_Format  = FORMAT_DOS;
	  global->datetime.dat_Flags   = NULL;
	  global->datetime.dat_StrDay  = global->dow;
	  global->datetime.dat_StrDate = global->date;
	  global->datetime.dat_StrTime = global->time;
	  DateToStr(&global->datetime);
	  global->listfields[0] = (long)global->dow;
	  global->listfields[1] = (long)global->date;
	  VPrintf(MSG_DIR_ON, global->listfields);
	}

	/*-------------------------------------------------------------------*/
	/* Make sure we print a blank line, if we have more to do.	     */
	/*-------------------------------------------------------------------*/
	global->notfirst = 1;

	/*-------------------------------------------------------------------*/
	/* Also, flag that we have printed the header for this directory.    */
	/*-------------------------------------------------------------------*/
	didhdr = 1L;
      }

      entrytype = ua->ap_Info.fib_DirEntryType;

      /*---------------------------------------------------------------------*/
      /* If they requested the ALL option, we must keep track of the	     */
      /* directory names we encounter along the way.  FIFO buffering is used */
      /* for simplicity's sake.                                              */
      /*---------------------------------------------------------------------*/
      if (global->opts[OPT_ALL] && (entrytype >= 0)) {
	if ((dtemp = AllocVec(sizeof(struct DirList),
			      MEMF_CLEAR)) != NULL) {
	  strcpy(dtemp->dname,namebuf);
	  AddPart(dtemp->dname,ua->ap_Info.fib_FileName,ENVMAX);
	  if (dhead == NULL) {
	    dhead = dtail = dtemp;
	  } else {
	    dtail->next = dtemp;
	    dtail = dtemp;
	  }
	}
      }

      /*---------------------------------------------------------------------*/
      /* Check the entry's type, and fit it with the FILES and DIRS options. */
      /*---------------------------------------------------------------------*/
      if (((entrytype < 0) && (global->opts[OPT_DIRS] != NULL)) ||
	  ((entrytype > 0) && (global->opts[OPT_FILES] != NULL))) {
	if ((global->opts[OPT_DIRS] == NULL) ||
	    (global->opts[OPT_FILES] == NULL)) {
	  continue;
	}
      }

      /*---------------------------------------------------------------------*/
      /* Check the current file's DateStamp against the SINCE option, if     */
      /* requested, of course!						     */
      /*---------------------------------------------------------------------*/
      if ((global->opts[OPT_SINCE] != NULL) &&
	  (CompareDates(since,
			(struct DateStamp *)&ua->ap_Info.fib_Date) <= 0)) {
	continue;
      }

      /*---------------------------------------------------------------------*/
      /* Check UPTO here.						     */
      /*---------------------------------------------------------------------*/
      if ((global->opts[OPT_UPTO] != NULL) &&
	  (CompareDates(upto,
			(struct DateStamp *)&ua->ap_Info.fib_Date) >= 0)) {
	continue;
      }

      /*---------------------------------------------------------------------*/
      /* Last, check if it matches the PAT or SUB patterns, if any.	     */
      /*---------------------------------------------------------------------*/
      if (global->patbuf[0]) {
	if (MatchPatternNoCase(global->patbuf,
			       (char *)&ua->ap_Info.fib_FileName) == NULL) {
	  continue;
	}
      } else {
	if (global->subbuf[0]) {
	  if (MatchPatternNoCase(global->subbuf,
				 (char *)&ua->ap_Info.fib_FileName) == NULL) {
	    continue;
	  }
	}
      }

      /*---------------------------------------------------------------------*/
      /* If we made it here, we are good to go on printing this file.  Of    */
      /* course, we have to collect and format all the information first.    */
      /*---------------------------------------------------------------------*/
      memcpy((char *)&global->datetime,(char *)&ua->ap_Info.fib_Date,
             sizeof(struct DateStamp));
      global->datetime.dat_Format  = FORMAT_DOS;
      global->datetime.dat_Flags =(!global->opts[OPT_DATES]) ?  DTF_SUBST : 0;
                                  global->datetime.dat_StrDay  = global->dow;
      global->datetime.dat_StrDate = global->date;
      global->datetime.dat_StrTime = global->time;

      /*---------------------------------------------------------------------*/
      /* If DateToStr() set the fields properly, as in setting "<unset>",    */
      /* "<invalid>", etc. on error, I wouldn't have to do the following     */
      /* check.  I could use the fields directly, instead of setting them    */
      /* myself.                                                             */
      /*---------------------------------------------------------------------*/
      if (DateToStr(&global->datetime) == NULL) {
        strcpy(global->date, MSG_INVALID_DS);
        strcpy(global->time, MSG_INVALID_DS);
      }

      strcpy(minusbuf, ua->ap_Info.fib_FileName);
      tptr = strrchr(minusbuf, '.');
      if (tptr)
      {
         strcpy(extbuf, &tptr[1]);
         *tptr = 0;
      }
      else
      {
         extbuf[0] = 0;
      }

      RawDoFmt("%ld",&ua->ap_Info.fib_NumBlocks, SpitChar, blkbuf);
      RawDoFmt("%ld",&ua->ap_Info.fib_DiskKey,   SpitChar, keybuf);
      RawDoFmt("%ld",&ua->ap_Info.fib_Size,      SpitChar, lenbuf);

      /*---------------------------------------------------------------------*/
      /* For speed issues, I have moved this loop here. In previous versions */
      /* I had this loop INSIDE the next, which would have slowed the loop   */
      /* considerably if anyone requested multiple instances of attributes.  */
      /*---------------------------------------------------------------------*/
      strcpy(protbits, MSG_PROTBITS);
      for (i=0; i<8; i++) {
        if ((ua->ap_Info.fib_Protection & (1 << (7-i))) ==
            ((i<4) ? NULL : (1 << (7-i)))) {
          protbits[i] = '-';
        }
      }

      strcpy(userotherbits, MSG_USEROTHERBITS);
      for (i= 8; i < 16; i++) {
        if ((ua->ap_Info.fib_Protection & (1 << i)) == 0) {
          int ioffset;
          if (i < 12)
            ioffset = 12-i;
          else
            ioffset = 21-i;
          userotherbits[ioffset-1]= '-';
        }
      }

      for (lindex = 0; (lindex < strlen(global->lorder)); lindex++) {
        switch (global->lorder[lindex]) {

          /* file protection attributes */
          case 'a':
            global->listfields[lindex] = (long)protbits;
            break;

          /* file length (in blocks) */
          case 'b':
            if (entrytype >= 0) {
              global->listfields[lindex] = (long)MSG_DIR_TYPE;
            } else {
              if (ua->ap_Info.fib_Size == NULL) {
                global->listfields[lindex] = (long)MSG_EMPTY;
              } else {
                global->listfields[lindex] = (long)blkbuf;
              }
            }
            break;

          /* file comment */
          case 'c':
            global->listfields[lindex] = (long)combuf;
            if (ua->ap_Info.fib_Comment[0] != NULL) {
              strcpy(combuf,global->comdef);
              strcat(combuf,ua->ap_Info.fib_Comment);
            } else {
              combuf[0] = 0;
            }
            break;

          /* date field */
          case 'd':
            global->listfields[lindex] = (long)global->date;
            break;

          /* file extension only */
          case 'e':
            global->listfields[lindex] = (long)extbuf;
            break;

          /* full path */
          case 'f':
            global->listfields[lindex] = (long)fulldir;
            break;

          /* Owner GID */
          case 'g':
            groupname[0]=0;
            global->listfields[lindex] = (long) groupname;
            GIDToName(global,ua->ap_Info.fib_OwnerGID, (STRPTR) &groupname, dirname);
            break;

          /* file's key block */
          case 'k':
            global->listfields[lindex] = (long)keybuf;
            break;

          /* file length (in bytes) */
          case 'l':
            if (entrytype >= 0) {
              global->listfields[lindex] = (long)MSG_DIR_TYPE;
            } else {
              if (ua->ap_Info.fib_Size == NULL) {
                global->listfields[lindex] = (long)MSG_EMPTY;
              } else {
                global->listfields[lindex] = (long)lenbuf;
              }
            }
            break;

          /* filename minus extension */
          case 'm':
            global->listfields[lindex] = (long)minusbuf;
            break;

          /* filename */
          case 'n':
            global->listfields[lindex] = (long)&ua->ap_Info.fib_FileName;
            break;

          /* path */
          case 'p':
            global->listfields[lindex] = (long)dirbuf;
            break;

          /* time field */
          case 't':
            global->listfields[lindex] = (long)global->time;
            break;

          /* Owner UID */
          case 'u':
            username[0] = 0;
            global->listfields[lindex] = (long)username;
            UIDToName(global,ua->ap_Info.fib_OwnerUID,(STRPTR) &username,dirname);
            break;

          case 'v':
            global->listfields[lindex] = (long) userotherbits;
            break;

        }
      }

      if (global->opts[OPT_SORT])
      {
          count = 0;
          RawDoFmt(global->lformat, global->listfields, CountChar, &count);

          if (de = AllocVec(sizeof(struct DirEntry) + count + strlen(ua->ap_Info.fib_FileName) + 1,MEMF_ANY))
          {
              de->de_Link.ln_Name = (STRPTR)&de[1];
              RawDoFmt(global->lformat,global->listfields,SpitChar,de->de_Link.ln_Name);

              de->de_FileSize = ua->ap_Info.fib_Size;
              de->de_FileDate = ua->ap_Info.fib_Date;
              de->de_FileName = (STRPTR)((ULONG)de->de_Link.ln_Name + count);
              strcpy(de->de_FileName,ua->ap_Info.fib_FileName);

              if (entrytype > 0)
                  de->de_FileSize = 0;

              node = (struct DirEntry *)fileList.lh_Head;
              while (node->de_Link.ln_Succ)
              {
                  sortOrder = (struct SortOrder *)global->SortOrder.mlh_Head;
                  sortType  = sortOrder->so_Type;

                  do
                  {
                      switch (sortType)
                      {
                          case ST_NAME        : cmp = Stricmp(node->de_FileName,de->de_FileName);
                                                break;

                          case ST_DATE        : cmp = CompareDates(&node->de_FileDate,&de->de_FileDate);
                                                break;

                          case ST_SIZE        : if (node->de_FileSize > de->de_FileSize)
                                                    cmp = 1;
                                                else if (node->de_FileSize < de->de_FileSize)
                                                    cmp = -1;
                                                else
                                                    cmp = 0;

                                                break;

                          case ST_REVERSENAME : cmp = Stricmp(de->de_FileName,node->de_FileName);
                                                break;

                          case ST_REVERSEDATE : cmp = CompareDates(&de->de_FileDate,&node->de_FileDate);
                                                break;

                          case ST_REVERSESIZE : if (de->de_FileSize > node->de_FileSize)
                                                    cmp = 1;
                                                else if (de->de_FileSize < node->de_FileSize)
                                                    cmp = -1;
                                                else
                                                    cmp = 0;

                                                break;
                      }

                      if (cmp == 0)
                      {
                          sortOrder = (struct SortOrder *)sortOrder->so_Link.mln_Succ;
                          if (!sortOrder->so_Link.mln_Succ)
                              break;
                          sortType = sortOrder->so_Type;
                      }
                  }
                  while (cmp == 0);

                  if (cmp > 0)
                      break;

                  node = (struct DirEntry *)node->de_Link.ln_Succ;
              }
              Insert(&fileList,de,node->de_Link.ln_Pred);
          }
          else
          {
              PrintFault(ERROR_NO_FREE_STORE,NULL);
              global->rc = RETURN_FAIL;
              goto ldexit;
          }
      }
      else
      {
          /*---------------------------------------------------------------------*/
          /* Finally!  Print everything we have collected.                       */
          /*---------------------------------------------------------------------*/
          VPrintf(global->lformat, global->listfields);
          PutStr("\n");
      }

      if (entrytype >= 0) {
	if (global->opts[OPT_FILES] == NULL) {
	  global->dircount++;
	  global->totaldirs++;
	}
      } else {
	if (global->opts[OPT_DIRS] == NULL) {
	  global->filecount++;
	  global->totalfiles++;
	}
      }

      /*---------------------------------------------------------------------*/
      /* We must add one to account for the extra block used by the	     */
      /* directory entry itself.					     */
      /*---------------------------------------------------------------------*/
      global->numblocks += ua->ap_Info.fib_NumBlocks+1;
      global->totalblocks += ua->ap_Info.fib_NumBlocks+1;

      if (CheckSignal(SIGBREAKF_CTRL_C) != 0L) {
	Result2(ERROR_BREAK);
	rc = 1;
	break;
      }

    } while ((rc=MatchNext(ua)) == NULL);

breakloop:
    if(rc)temprc = IoErr();
    if((temprc == ERROR_NO_MORE_ENTRIES) || (temprc == 0)) {
      global->rc = RETURN_OK;

      while (de = (struct DirEntry *)RemHead(&fileList))
      {
          PutStr(de->de_Link.ln_Name);
          PutStr("\n");

          if (CheckSignal(SIGBREAKF_CTRL_C) != 0L)
          {
              temprc = ERROR_BREAK;
              break;
          }

          FreeVec(de);
      }
    }
    if (temprc == ERROR_BREAK) {
      PrintFault(ERROR_BREAK, NULL);
      global->rc = RETURN_WARN;
    }
ldexit:
    MatchEnd(ua);
  }

  /*-------------------------------------------------------------------------*/
  /* If we have cached any directories, for use with the ALL option, we need */
  /* to add them to the global list for processing.			     */
  /*-------------------------------------------------------------------------*/
  if (dhead) {
    dtail->next = global->dhead;
    global->dhead = dhead;
  }

  while (de = (struct DirEntry *)RemHead(&fileList))
      FreeVec(de);

  /*-------------------------------------------------------------------------*/
  /* Print statistics for the final directory we worked on.		     */
  /*-------------------------------------------------------------------------*/
  PrintDirStats(global);

  return(global->rc);
}

/*---------------------------------------------------------------------------*/
/* This routine prints the statistics for a single directory.  The TOTAL is  */
/* printed in main() (IF the ALL option was used.).                          */
/*---------------------------------------------------------------------------*/
static void PrintDirStats(struct Global *global)
{
  /*-------------------------------------------------------------------------*/
  /* If we DIDN'T use the LFORMAT option...                                  */
  /*-------------------------------------------------------------------------*/
  if ((global->opts[OPT_LFORMAT] == NULL) &&
      (global->opts[OPT_NOHEAD] == NULL)) {

    /*-----------------------------------------------------------------------*/
    /* Check to see if we HAVE any stats to print...			     */
    /*-----------------------------------------------------------------------*/
    if ((global->filecount+global->dircount) != NULL) {

      /*---------------------------------------------------------------------*/
      /* Print statistics for this set, then exit.			     */
      /*---------------------------------------------------------------------*/
      if ((global->opts[OPT_DIRS] == NULL) &&
	  (global->filecount != NULL)) {
	global->listfields[0] = global->filecount;
	if (global->filecount > 1L) {
	  global->listfields[1] = (long)MSG_MANY_FILES;
	} else {
	  global->listfields[1] = (long)MSG_ONE_FILE;
	}
	VPrintf(MSG_COUNT, global->listfields);
      }
      if ((global->opts[OPT_FILES] == NULL) &&
	  (global->dircount != NULL)) {
	global->listfields[0] = global->dircount;
	if (global->dircount > 1L) {
	  global->listfields[1] = (long)MSG_MANY_DIRS;
	} else {
	  global->listfields[1] = (long)MSG_ONE_DIR;
	}
	VPrintf(MSG_COUNT, global->listfields);
      }
      global->listfields[0] = global->numblocks;
      if (global->numblocks > 1L) {
	global->listfields[1] = (long)MSG_MANY_BLOCKS;
      } else {
	global->listfields[1] = (long)MSG_ONE_BLOCK;
      }
      VPrintf(MSG_BLOCK, global->listfields);
    }
/*  else PutStr(MSG_NO_MATCH); */
  }

  /*-------------------------------------------------------------------------*/
  /* Clear these for next time around.					     */
  /*-------------------------------------------------------------------------*/
  global->dircount = global->filecount = global->numblocks = 0L;
}


#define ACTION_Dummy                20000
#define ACTION_USERNAME_TO_UID      (ACTION_Dummy+0)
#define ACTION_GROUPNAME_TO_GID     (ACTION_Dummy+1)
#define ACTION_UID_TO_USERINFO      (ACTION_Dummy+2)
#define ACTION_GID_TO_GROUPINFO     (ACTION_Dummy+3)


static BOOL UIDToName(struct Global *global, UWORD ID, STRPTR target, STRPTR dir)
{

    struct MsgPort *m;
    ULONG t=ID;

    if (!ID)
    {
        strcpy(target,MSG_NO_OWNER);
        return(TRUE);
    }

    m = DeviceProc(dir);
    if (m)
    {
        ULONG res1;
        struct UserInfo x;                                                  /* Not filled in by security.library -- copied in by fs */
        target[0]=0;
        res1 = DoPkt(m, ACTION_UID_TO_USERINFO, ID, (ULONG) &x,0,0,0);      /* Call the FS to get the info */
        if (res1)                                                           /* If FS reports successful, copy the string and exit */
        {
            strncpy(target, x.ui_UserName, 32);
            return(TRUE);
        }
        else                                                                /* If not successful, return a string formed of <#>, where # is the ID */
        {
            RawDoFmt("<%ld>",&t,SpitChar,target);
        }
    }
    return(FALSE);
}



static BOOL GIDToName(struct Global *global, UWORD ID, STRPTR target, STRPTR dir)
{

    struct MsgPort *m;
    ULONG t=ID;

    if (!ID)
    {
        strcpy(target,MSG_NO_GROUP);
        return(TRUE);
    }

    m = DeviceProc(dir);
    if (m)
    {
        ULONG res1;
        struct GroupInfo x;          /* Not filled in by security.library -- copied in by fs */
        target[0]=0;
        res1 = DoPkt(m, ACTION_GID_TO_GROUPINFO, ID, (ULONG) &x,0,0,0);
        if (res1)
        {
            strncpy(target, x.gi_GroupName, 32);
            return(TRUE);
        }
        else
        {
            RawDoFmt("<%ld>",&t,SpitChar,target);
        }
    }
    return(FALSE);
}
