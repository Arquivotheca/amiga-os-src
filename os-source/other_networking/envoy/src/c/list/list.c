; /*
lc -d -j73 -rr -O -o/obj/list.o -i/include -v -cisf list
blink /obj/list.o to /bin/list sc sd nd
protect /bin/list +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 460-7430    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: List                                                             */
/* Author:  James E. Cooper Jr.                                              */
/* Change History:                                                           */
/*  Date     Person        Action                                            */
/* -------  ------------- -----------------                                  */
/* 24MAY89  Jim Cooper    Initial Creation                                   */
/* 13DEC89  Jim Cooper    Time for some serious work; deadline TODAY!        */
/* 19DEC89  Jim Cooper    FINALLY! (Even LFORMAT!!!)                         */
/* 30DEC89  Jim Cooper    Bug Fixes (NULL on 'TO' option, Header and Footer  */
/*                        suppression on 'LFORMAT' option, etc.)             */
/* 19FEB90  Jim Cooper    You mean nobody noticed that I wasn't printing     */
/*                        FileNotes?  Shows how often it is used.            */
/* 05MAR90  Jim Cooper    Finished code for 'ALL' option.                    */
/* 08MAR90  Jim Cooper    ??BUG reported when 'Global' not initialized.      */
/*                        Also, made "...is empty" work as supposed to.      */
/* 21MAY90  Jim Cooper    Small change to logic for 'ALL' option, added      */
/*                        "No match found..." message.                       */
/* 22MAY90  Jim Cooper    Small cleanups to output.                          */
/* 23MAY90  Jim Cooper    Added new LFORMAT options!                         */
/* 24MAY90  Jim Cooper    Finished adding new LFORMAT options.  Fixed bug    */
/*                        with occasional (-1) returns.                      */
/* 31AUG90  Jim Cooper    Final bug, I hope.  Fixed suppression of header    */
/*                        when using QUICK option, hopefully in ALL cases!   */
/* 10OCT90  Jim Cooper    Cleaned up, fixed 'list since', compiled w/5.10.   */
/* 17OCT90  Jim Cooper    Fixed problem with extremely long lformat strings. */
/* 20NOV90  Jim Cooper    Updated for change in DateStamp() prototype.  Also */
/*                        changed BBS number in header, and re-compiled with */
/*                        most recent headers.                               */
/* 10DEC90  Jim Cooper    Fixed extra lines problems with options of NOHEAD  */
/*                        ALL FILES together.  Also, misalignment on large   */
/*                        files.                                             */
/* 20DEC90  Jim Cooper    Rearranged code slightly.  Fixed problem of wrong  */
/*                        message being printed.                             */
/* 02JAN91  Jim Cooper    Made SUB switch work again.  Change necessary due  */
/*                        to change in behaviour of DOS AddPart() call.      */
/* 11JAN91  Jim Cooper    O.K.  Fixed so '#?/#?' style parameters work.      */
/* 22JAN91  Jim Cooper    Oops!  Minor inconsistency with last change, where */
/*                        two slashes were printed when only one was needed. */
/*                        Only showed up with LFORMAT option.                */
/* 03FEB91  Jim Cooper    Yeah!  Finally got 'APF_DirChanged'!  Took nearly  */
/*                        five whole minutes to make use of it!              */
/* 25FEB91  Jim Cooper    Changed to use ap_Buf to enable printing RELATIVE  */
/*                        path (just like 1.3!).  Added '%f' flag to LFORMAT */
/*                        in case you really DO need the FULL path.          */
/* 08MAY91  Jim Cooper    Forgot to screen extras recognised by RawDoFmt()   */
/*                        from output string when using LFORMAT option.  Now */
/*                        unrecognised options simply print with '%' in      */
/*                        front of them.                                     */
/* 18OCT91  Jim Cooper    Fixed SUB and PAT options to work as they should   */
/*                        have all along.  Now "list all pat #?.c" will show */
/*                        all .c files on drive.  Also, worked on strings    */
/*                        a little to ease internationalization process.     */
/* 04NOV91  Jim Cooper    #?!@&$ forgot to change the position of a linefeed */
/*                        print when I moved strings around in the previous  */
/*                        version.  Fixed.                                   */
/* 08NOV91  Jim Cooper    Added two new LFORMAT options: %E - file Extension */
/*                        and %M - name Minus extension.                     */
/*                                                                           */
/* Notes:                                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Comment out the following line if you do NOT want any new LFormat options */
/*---------------------------------------------------------------------------*/
#define WANT_NEW_LFORMAT

#include "internal/commands.h"
#include "list_rev.h"
#include "//fs/fs.h" /* temporary for the ACTION's -- will go away when randell assigns me packet #'s */
#include <envoy/accounts.h>

/* defines for ReadArgs() */


#define MSG_ARGS_EXCL   "'DATES' and 'NODATES' are mutually exclusive\n"
#define MSG_CANT_OPEN   "*** Can't open %s - "
#define MSG_INVALID_SINCE_UPTO \
"*** Invalid 'UPTO' or 'SINCE' parameter - ignored\n"
#define MSG_DIR         "Directory \"%s\" "
#define MSG_DIR_EMPTY   "is empty\n"
#define MSG_DIR_ON      "on %s %s\n"
#define MSG_DIR_TYPE    "Dir"
#define MSG_NO_INFO     "No information for \"%s\""
#define MSG_EMPTY       "empty"
#define MSG_TOTAL       "\nTOTAL: "
#define MSG_COUNT       "%ld %s - "
#define MSG_BLOCK       "%ld %s used\n"
#define MSG_MANY_FILES  "files"
#define MSG_ONE_FILE    "file"
#define MSG_MANY_DIRS   "directories"
#define MSG_ONE_DIR     "directory"
#define MSG_MANY_BLOCKS "blocks"
#define MSG_ONE_BLOCK   "block"
#define MSG_PROTBITS    "hsparwed"
#define MSG_USEROTHERBITS " rwed rwed"
#define MSG_NO_MATCH    "No match found\n"
#define MSG_CANT_LIST   "\" cannot be listed: not a FileSystem device\n"
#define MSG_INVALID_DS  "<invalid>"

#define TEMPLATE "DIR/M,P=PAT/K,KEYS/S,DATES/S,NODATES/S,TO/K,SUB/K,SINCE/K,\
UPTO/K,QUICK/S,BLOCK/S,NOHEAD/S,FILES/S,DIRS/S,LFORMAT/K,USERS/S,GROUPS/S,ALL/S" CMDREV

#define OPT_DIR      0
#define OPT_PAT      1
#define OPT_KEYS     2
#define OPT_DATES    3
#define OPT_NODATES  4
#define OPT_TO       5
#define OPT_SUB      6
#define OPT_SINCE    7
#define OPT_UPTO     8
#define OPT_QUICK    9
#define OPT_BLOCK   10
#define OPT_NOHEAD  11
#define OPT_FILES   12
#define OPT_DIRS    13
#define OPT_LFORMAT 14
#define OPT_USERS   15
#define OPT_GROUPS  16
#define OPT_ALL     17
#define OPT_COUNT   18

/*---------------------------------------------------------------------------*/
/* Defines added for the Envoy Users/Groups/more protection bits stuff       */
/*---------------------------------------------------------------------------*/
#define USERLIMIT   10      /* Number of characters printed for the "user"   */
#define GROUPLIMIT  10      /* Number of characters printed for the "group"  */


/*---------------------------------------------------------------------------*/
/* Define the signals we wish the pattern matcher to stop on.                */
/*---------------------------------------------------------------------------*/
#define BREAK_SIGS SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D

/*---------------------------------------------------------------------------*/
/* Define the minimum buffer size necessary to format output.                */
/*---------------------------------------------------------------------------*/
#define MIN_BUFFER 32

/*---------------------------------------------------------------------------*/
/* DirList structure is used to store directory names for ALL option.        */
/*---------------------------------------------------------------------------*/
struct DirList {
  struct DirList *next;
  char dname[ENVMAX];
};

/*---------------------------------------------------------------------------*/
/* Global structure is used to pass info between the two routines.           */
/*---------------------------------------------------------------------------*/
struct Global {
  struct DosLibrary *DOSBase;
  long opts[OPT_COUNT];
  long dircount;
  long filecount;
  long numblocks;
  struct DirList *dhead;
  long totaldirs;
  long totalfiles;
  long totalblocks;
  char *lformat;                        /* print format specifier string     */
  char *lorder;                         /* order of values for above         */
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
};

int list_dir(struct Global *, char *, struct AnchorPath *,
             struct DateStamp *, struct DateStamp *);
void PrintDirStats(struct Global *);
void vsprintf(char *, char *, long *);
void prbuf(char c);
BOOL UIDToName(UWORD ID, STRPTR target, STRPTR dir, struct Global *global);
BOOL GIDToName(UWORD ID, STRPTR target, STRPTR dir, struct Global *global);
void kprintf(STRPTR, ...);

/*---------------------------------------------------------------------------*/
/* Main program                                                              */
/*---------------------------------------------------------------------------*/
int cmd_list(void)
{
  struct Library *SysBase = (*((struct Library **) 4));
  struct DosLibrary *DOSBase;
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

  rc = RETURN_FAIL;

  /*-------------------------------------------------------------------------*/
  /* 'dummy' is used to initialize the 'argptr' array, in case the user      */
  /* didn't type any dir names.                                              */
  /*-------------------------------------------------------------------------*/
  dummy[0] = "";
  dummy[1] = NULL;

  /*-------------------------------------------------------------------------*/
  /* Try to open the 2.0 dos.library.                                        */
  /*-------------------------------------------------------------------------*/
  if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {

    /*-----------------------------------------------------------------------*/
    /* Create our global data area.                                          */
    /*-----------------------------------------------------------------------*/
    if ((global = (struct Global *)
            AllocVec(sizeof(struct Global), MEMF_PUBLIC|MEMF_CLEAR)) == NULL) {
      PrintFault(IoErr(),NULL);
      goto panic;
    }

    /*-----------------------------------------------------------------------*/
    /* Save the DOSBase pointer for later use.                               */
    /*-----------------------------------------------------------------------*/
    global->DOSBase = DOSBase;

    /*-----------------------------------------------------------------------*/
    /* Now try to parse the user's input.                                    */
    /*-----------------------------------------------------------------------*/
    rdargs = ReadArgs(TEMPLATE, global->opts, NULL);

    /*-----------------------------------------------------------------------*/
    /* If we had an error parsing, rdargs should be null.                    */
    /*-----------------------------------------------------------------------*/
    if (rdargs == NULL) {
      PrintFault(IoErr(),NULL);
    } else {

      /*---------------------------------------------------------------------*/
      /* If they gave an LFORMAT option, try to parse it the same way the    */
      /* BCPL version of list does.                                          */
      /*---------------------------------------------------------------------*/
      temp = (char *)global->opts[OPT_LFORMAT];

      /*---------------------------------------------------------------------*/
      /* Grab some memory for workspace.                                     */
      /*---------------------------------------------------------------------*/
      i = (temp != NULL) ? strlen(temp) : MIN_BUFFER;
      if ((global->lformat = AllocVec(((i<<1)+i+1),
                                     MEMF_PUBLIC|MEMF_CLEAR))==NULL) {
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
#ifdef WANT_NEW_LFORMAT
              case 'E':
#endif
              case 'F':
              case 'K':
              case 'L':
#ifdef WANT_NEW_LFORMAT
              case 'M':
#endif
              case 'N':
              case 'P':
              case 'T':
              case 'G':
              case 'U':
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
        i = 0;                          /* use as index to 'lorder' string.  */

        /*-------------------------------------------------------------------*/
        /* If not using the LFORMAT option, we ALWAYS! print the name.       */
        /*-------------------------------------------------------------------*/
        global->lorder[i++] = 'n';

        /*-------------------------------------------------------------------*/
        /* QUICK option handling.                                            */
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
//            strcat(global->lformat," %3ld");
            strcat(global->lformat," %10s");
            global->lorder[i++] = 'u';
          }

          if (global->opts[OPT_GROUPS] != NULL) {
//            strcat(global->lformat," %3ld");
            strcat(global->lformat," %10s");
            global->lorder[i++] = 'g';
          }

          strcat(global->lformat, " %s");
          global->lorder[i++] = 'a';

          if (global->opts[OPT_GROUPS] != NULL) {
            strcat(global->lformat,"%s");
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
      /* create an array to hold at least that many variables.  We'll make   */
      /* sure we have at least the minimum necessary to handle any default   */
      /* printing as well.                                                   */
      /*---------------------------------------------------------------------*/
      if ((global->listfields = AllocVec(((strlen(global->lorder) > 4) ?
                                         strlen(global->lorder) : 4)
                                         * sizeof(long),
                                         MEMF_PUBLIC|MEMF_CHIP))==NULL) {
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
        /* at least gets started in that direction.                          */
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
                           MEMF_PUBLIC|MEMF_CLEAR)) == NULL) {
          PrintFault(IoErr(), NULL);
        } else {

          /*-----------------------------------------------------------------*/
          /* Grab the current DateStamp, for future use.                     */
          /*-----------------------------------------------------------------*/
          DateStamp((struct DateStamp *)&global->now);

          /*-----------------------------------------------------------------*/
          /* If they gave a SINCE option, try to set up for its use.         */
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
          /* Check for an UPTO option, and set it up if present.             */
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
          /* Start parsing the Multi-Args!                                   */
          /*-----------------------------------------------------------------*/
          while ((*(argptr) != NULL) || (global->dhead != NULL)) {

            /*---------------------------------------------------------------*/
            /* If there is anything in 'global->dhead' (which can ONLY occur */
            /* if the ALL option was used!) we need to take care of any sub- */
            /* directories before we move on.                                */
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
            /* Handle PAT and SUB keywords here.                             */
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
            /* Call the match loop.                                          */
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
            /* Check to see if we HAVE any stats to print...                 */
            /*---------------------------------------------------------------*/
            if ((global->totalfiles+global->totaldirs) != NULL) {

              PutStr(MSG_TOTAL);

              /*-------------------------------------------------------------*/
              /* Print total statistics, then exit.                          */
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
      if (global->listfields != NULL) {
        FreeVec(global->listfields);
      }
      if (global->lformat != NULL) {
        FreeVec(global->lformat);
      }
    }

    /*-----------------------------------------------------------------------*/
    /* If there is anything in the DirList, we have to clean it up before we */
    /* exit!                                                                 */
    /*-----------------------------------------------------------------------*/
    while (global->dhead) {
      dtemp = global->dhead->next;
      FreeVec(global->dhead);
      global->dhead = dtemp;
    }

    rc = global->rc;
    FreeVec(global);

panic:
    CloseLibrary((struct Library *)DOSBase);
  } else {
    OPENFAIL;
  }
  return(rc);
}

#define DOSBase global->DOSBase

/*---------------------------------------------------------------------------*/
/* This is the routine which parses the directories, deciding what to print  */
/* based on the user input.                                                  */
/*---------------------------------------------------------------------------*/
int list_dir(global, dirname, ua, since, upto)
struct Global *global;
char *dirname;
struct AnchorPath *ua;
struct DateStamp *since, *upto;
{
  struct Library *SysBase = (*((struct Library **) 4));
  long temprc=0;
  char namebuf[ENVMAX];
  char dirbuf[ENVMAX];
  char fulldir[ENVMAX];
  char combuf[ENVMAX];
#ifdef WANT_NEW_LFORMAT
  char minusbuf[32];
  char extbuf[32];
#endif
  char blkbuf[16];
  char keybuf[16];
  char lenbuf[16];
  char protbits[9];
  char userotherbits[11];
  char *tptr;
  long i, entrytype, lindex;
  long rc;
  long didhdr;
  struct DirList *dhead, *dtemp, *dtail;
  char username[32], groupname[32];

  dhead = dtail = NULL;
  didhdr = 0L;

  /*-------------------------------------------------------------------------*/
  /* Set the default return code.                                            */
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
    /* file.                                                                 */
    /*-----------------------------------------------------------------------*/
    ua->ap_Flags &= ~APF_DIDDIR;
    do {

      /*---------------------------------------------------------------------*/
      /* First check to see if MatchNext() did an internal dir change.       */
      /*---------------------------------------------------------------------*/
      if (ua->ap_Flags & APF_DirChanged) {

        /*-------------------------------------------------------------------*/
        /* The dir has changed... print stats for the last directory, and    */
        /* reset so we print everything correctly!                           */
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
        /* LFORMAT option.                                                   */
        /*-------------------------------------------------------------------*/
        /* For regular printing, we need a relative path...                  */
        /*-------------------------------------------------------------------*/
        strcpy(dirbuf, ua->ap_Buf);
        *(PathPart(dirbuf)) = 0;
        strcpy(namebuf, dirbuf);

        /*-------------------------------------------------------------------*/
        /* ...but, if you use %f in LFORMAT, we need the Full path.          */
        /*-------------------------------------------------------------------*/
        NameFromLock(ua->ap_Current->an_Lock, fulldir, ENVMAX);
        AddPart(fulldir, "", ENVMAX);

        /*-------------------------------------------------------------------*/
        /* Add the trailing ':' or '/' to the directory name, for possible   */
        /* printing use.                                                     */
        /*-------------------------------------------------------------------*/
        AddPart(dirbuf, "", ENVMAX);

        /*-------------------------------------------------------------------*/
        /* Print a header if we aren't specifically told not to, and we are  */
        /* NOT using the LFORMAT option, and they didn't give us a non-      */
        /* ambiguous fully qualified filespec.                               */
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
        /* Make sure we print a blank line, if we have more to do.           */
        /*-------------------------------------------------------------------*/
        global->notfirst = 1;

        /*-------------------------------------------------------------------*/
        /* Also, flag that we have printed the header for this directory.    */
        /*-------------------------------------------------------------------*/
        didhdr = 1L;
      }

      entrytype = ua->ap_Info.fib_DirEntryType;

      /*---------------------------------------------------------------------*/
      /* If they requested the ALL option, we must keep track of the         */
      /* directory names we encounter along the way.  FIFO buffering is used */
      /* for simplicity's sake.                                              */
      /*---------------------------------------------------------------------*/
      if ((global->opts[OPT_ALL] != NULL) && (entrytype >= 0)) {
        if ((dtemp = AllocVec(sizeof(struct DirList),
                              MEMF_PUBLIC|MEMF_CLEAR)) != NULL) {
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
      /* requested, of course!                                               */
      /*---------------------------------------------------------------------*/
      if ((global->opts[OPT_SINCE] != NULL) &&
          (CompareDates(since,
                        (struct DateStamp *)&ua->ap_Info.fib_Date) <= 0)) {
        continue;
      }

      /*---------------------------------------------------------------------*/
      /* Check UPTO here.                                                    */
      /*---------------------------------------------------------------------*/
      if ((global->opts[OPT_UPTO] != NULL) &&
          (CompareDates(upto,
                        (struct DateStamp *)&ua->ap_Info.fib_Date) >= 0)) {
        continue;
      }

      /*---------------------------------------------------------------------*/
      /* Last, check if it matches the PAT or SUB patterns, if any.          */
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

#ifdef WANT_NEW_LFORMAT
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
#endif

      vsprintf(blkbuf,"%ld",&ua->ap_Info.fib_NumBlocks);
      vsprintf(keybuf,"%ld",&ua->ap_Info.fib_DiskKey);
      vsprintf(lenbuf,"%ld",&ua->ap_Info.fib_Size);

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
        if ((ua->ap_Info.fib_Protection & (1 << i))) {
          int ioffset;
          if (i < 12)
            ioffset = 12-i;
          else
            ioffset = 21-i;
          userotherbits[ioffset]= '-';
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

#ifdef WANT_NEW_LFORMAT
          /* file extension only */
          case 'e':
            global->listfields[lindex] = (long)extbuf;
            break;
#endif

          /* full path */
          case 'f':
            global->listfields[lindex] = (long)fulldir;
            break;

          /* Owner GID */
          case 'g':
//            global->listfields[lindex] = (long) ua->ap_Info.fib_OwnerGID;
            groupname[0]=0;
            global->listfields[lindex] = (long) &groupname[0];
            GIDToName(ua->ap_Info.fib_OwnerGID, (STRPTR) &groupname, dirname, global);
            groupname[GROUPLIMIT]=0;
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

#ifdef WANT_NEW_LFORMAT
          /* filename minus extension */
          case 'm':
            global->listfields[lindex] = (long)minusbuf;
            break;
#endif

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
//            global->listfields[lindex] = (long) ua->ap_Info.fib_OwnerUID;
            global->listfields[lindex] = (long) &username;
            username[0]=0;
            UIDToName(ua->ap_Info.fib_OwnerUID,(STRPTR) &username,dirname, global);
            username[USERLIMIT]=0; /* No more than limit characters */
            break;

          case 'v':
            global->listfields[lindex] = (long) userotherbits;
            break;

        }
      }

      /*---------------------------------------------------------------------*/
      /* Finally!  Print everything we have collected.                       */
      /*---------------------------------------------------------------------*/
      VPrintf(global->lformat, global->listfields);
      PutStr("\n");

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
      /* We must add one to account for the extra block used by the          */
      /* directory entry itself.                                             */
      /*---------------------------------------------------------------------*/
      global->numblocks += ua->ap_Info.fib_NumBlocks+1;
      global->totalblocks += ua->ap_Info.fib_NumBlocks+1;

      if (CheckSignal(SIGBREAKF_CTRL_C) != 0L) {
        Result2(ERROR_BREAK);
        rc = 1;
        break;
      }

    } while ((rc=MatchNext(ua)) == NULL);

    if(rc)temprc = IoErr();
    if((temprc == ERROR_NO_MORE_ENTRIES) || (temprc == 0)) {
      global->rc = RETURN_OK;
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
  /* to add them to the global list for processing.                          */
  /*-------------------------------------------------------------------------*/
  if (dhead) {
    dtail->next = global->dhead;
    global->dhead = dhead;
  }

  /*-------------------------------------------------------------------------*/
  /* Print statistics for the final directory we worked on.                  */
  /*-------------------------------------------------------------------------*/
  PrintDirStats(global);

  return(global->rc);
}

/*---------------------------------------------------------------------------*/
/* This routine prints the statistics for a single directory.  The TOTAL is  */
/* printed in main() (IF the ALL option was used.).                          */
/*---------------------------------------------------------------------------*/
void PrintDirStats(struct Global *global)
{
  struct Library *SysBase = (*((struct Library **) 4));

  /*-------------------------------------------------------------------------*/
  /* If we DIDN'T use the LFORMAT option...                                  */
  /*-------------------------------------------------------------------------*/
  if ((global->opts[OPT_LFORMAT] == NULL) &&
      (global->opts[OPT_NOHEAD] == NULL)) {

    /*-----------------------------------------------------------------------*/
    /* Check to see if we HAVE any stats to print...                         */
    /*-----------------------------------------------------------------------*/
    if ((global->filecount+global->dircount) != NULL) {

      /*---------------------------------------------------------------------*/
      /* Print statistics for this set, then exit.                           */
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
  /* Clear these for next time around.                                       */
  /*-------------------------------------------------------------------------*/
  global->dircount = global->filecount = global->numblocks = 0L;
}

/***
*
* This is a tiny implementation of sprintf for use within List
* It should elminate most needs to format numbers
* Note that it assumes that the buffer is large enough to hold any
* formatted string
*
***/

//#pragma syscall RawDoFmt 20a ba9804
extern void __builtin_emit(unsigned short);

void vsprintf(char *buffer, char *ctlstr, long args[])
{
   struct Library *SysBase = (*((struct Library **) 4));

   RawDoFmt(ctlstr, args, prbuf, buffer);
}

/***
*
* The following stub routine is called from RawDoFmt for each
* character in the string.  At invocation, we have:
*    D0 - next character to be formatted
*    A3 - pointer to data buffer
*
***/
void prbuf(char c)
{
__builtin_emit(0x16c0);   /* move.b D0,(A3)+ */
}



BOOL UIDToName(UWORD ID, STRPTR target, STRPTR dir, struct Global *global)
{

    struct MsgPort *m;
    ULONG t=ID;

    if (!ID)
    {
        strcpy(target,"No Owner");
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
            vsprintf(target,"<%ld>",&t);
        }
    }
    return(FALSE);
}



BOOL GIDToName(UWORD ID, STRPTR target, STRPTR dir, struct Global *global)
{

    struct MsgPort *m;
    ULONG t=ID;

    if (!ID)
    {
        strcpy(target,"No Group");
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
            vsprintf(target,"<%ld>",&t);

    }
    return(FALSE);
}


