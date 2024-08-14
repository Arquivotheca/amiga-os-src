; /*
lc -d -j73 -O -oJoin.o -i//include -v -csf join
blink join.o to join.ld sc sd nd batch
protect /bin/join +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 471-6436    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*---------------------------------------------------------------------------*/
/* Command: Join                                                             */
/* Author:  Andrew N. Mercier, Jr.                                           */
/* Change History:                                                           */
/*  Date    Person        Action                                             */
/* -------  ------------- -----------------                                  */
/* 19MAR89  John Toebes   Initial Creation                                   */
/* 04APR89  Andy Mercier  Created buffering.                                 */
/* 29OCT89  Andy Mercier  Updated to DOS V1.4                                */
/* 05DEC89  Andy Mercier  Tested and touched up code                         */
/* 13DEC89  Andy Mercier  Removed last traces of bad code.                   */
/* 17DEC89  Andy Mercier  Worked on pattern matching and multiargs           */
/* 23MAR90  Andy Mercier  Cleared up return codes, avoided joining           */
/*                        directories                                        */
/* 24SEP90  Andy Mercier  Changed template to make more sense                */
/* 06NOV90  John Toebes   Corrected directory checking code to only apply    */
/*                        when doing a wildcard.                             */
/*                                                                           */
/* Notes:                                                                    */
/*       Join can act as a file copier if you specify one file to join TO/AS */
/*       another file, preferably TO to match the copy command, it's much,   */
/*       much smaller.                                                       */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "join_rev.h"
#include <stdlib.h>

#define min __builtin_min
extern int min(int, int);

#define MSG_CANT_OPEN    "Can't open %s\n"
#define MSG_DEST_REMOVED "Destination file \"%s\" removed\n"

#define TEMPLATE  "FILE/M/A,AS=TO/K/A" CMDREV
#define OPT_FILE  0
#define OPT_AS    1
#define OPT_COUNT OPT_AS+1

#define BUFSAFE    100
#define BUFSIZE    102400     /* 100K buffer */

#define DELETE_FILE_FLAG 30   /* Using unique rc for flag telling us
                                 to delete the output file */

/* Prototype for join subroutine */
int joinfile(BPTR infh, BPTR outfh, void *buff, long buffsize, struct DosLibrary *DOSBase);


int cmd_join(void)
{
struct Library *SysBase = (*((struct Library **) 4));
struct DosLibrary *DOSBase;
int rc,rc2=0;
char *msg;
long opts[OPT_COUNT], buffsize=BUFSIZE, scratch[1];
BPTR infile, outfile;
BPTR lock;
void *buff;
char *curarg, **argptr;
struct RDArgs *rdargs;
struct AnchorPath *ua;

infile = NULL;
outfile = NULL;
msg = NULL;
rc = RETURN_FAIL;

if ((DOSBase = (struct DosLibrary *) OpenLibrary(DOSLIB, DOSVER))) {
   memset((char *) opts, 0, sizeof(opts));

   if ((rdargs = ReadArgs(TEMPLATE, opts, NULL)) == NULL) {
      PrintFault(IoErr(), NULL);
   }
   else {
      /*-------------------------------------------------------------*/
      /* Allocate memory to use with the pattern match stuff.        */
      /*-------------------------------------------------------------*/
      if ((ua = AllocVec(sizeof(struct AnchorPath)+255,
                         MEMF_CLEAR)) == NULL) {
            PrintFault(IoErr(), NULL);
      }
      else {
         /*----------------------------------------------------------*/
         /* Now we have to set up the pattern match structure.       */
         /*                                                          */
         /* Tell the pattern routines that we want them to return up */
         /* to 255 chars of the full path to any matches.            */
         /*----------------------------------------------------------*/
         ua->ap_Strlen = 255;
         /*----------------------------------------------------------*/
         /* Set the flag which tells the pattern matcher to decode   */
         /* wild cards.                                              */
         /*----------------------------------------------------------*/
         ua->ap_Flags = 0;
         /*----------------------------------------------------------*/
         /* Tell the matcher to stop if the user presses Ctrl-C.     */
         /*----------------------------------------------------------*/
         ua->ap_BreakBits = SIGBREAKF_CTRL_C;

         /* This opens the output file                               */
         if (!(outfile = Open((char *) opts[OPT_AS], MODE_NEWFILE))) {
            PrintFault(IoErr(), (char *) opts[OPT_AS]);
            rc2=0;
            goto done;
         }

         /*-------------------------------------------------------------*/
         /* Try to get our standard buffer size, if we can't, get the   */
         /* biggest that we can, safely                                 */
         /*-------------------------------------------------------------*/
         if ((buff = AllocVec(BUFSIZE, MEMF_PUBLIC)) == NULL) {
            Forbid();
            buffsize = AvailMem(MEMF_LARGEST) - BUFSAFE;
            buff = AllocVec(buffsize, MEMF_PUBLIC);
            Permit();
            if (buff == NULL) {
               rc2=IoErr();
               goto done;
            }
         }

         argptr = (char **) opts[OPT_FILE];
      if (opts[OPT_FILE] != NULL) {
         msg = MSG_CANT_OPEN;
         rc2 = 0;

         /*-------------------------------------------------------------*/
         /* The following while loop handles the MultiArgs spec.        */
         /*-------------------------------------------------------------*/
         while (curarg = *argptr++) {
            msg = MSG_CANT_OPEN;
            /*----------------------------------------------------------*/
            /* Next we process the wild cards                           */
            /*----------------------------------------------------------*/
            if (rc = MatchFirst(curarg, ua)) {
               rc2 = IoErr();
            }
            scratch[0] = (long) curarg;
            while (!rc) {
               /*-------------------------------------------------*/
               /* Set up a default message for when we can't find */
               /* the input file                                  */
               /*-------------------------------------------------*/
               msg = MSG_CANT_OPEN;

               /*-------------------------------------------------*/
               /* Let's see if this is a directory, if so skip it */
               /*-------------------------------------------------*/
               if ((ua->ap_Info.fib_DirEntryType > 0) &&
	            (ua->ap_Flags & APF_ITSWILD))
               {
                 if (rc = MatchNext(ua)) {
            	     rc2 = IoErr();
                 }
                 msg = NULL;
                 continue;
               }
               /*-------------------------------------------------*/
               /* Locate and open the input source file.          */
               /*-------------------------------------------------*/
               lock = CurrentDir(ua->ap_Current->an_Lock);
               if ((infile = Open((char *)ua->ap_Info.fib_FileName,
                                  MODE_OLDFILE)) == NULL) {
                  rc2 = IoErr();
               }
               CurrentDir(lock);
               /*-------------------------------------------------*/
               /* Check to make sure that we could open the input */
               /* file.                                           */
               /*-------------------------------------------------*/
               if (infile == NULL) {
                  /*----------------------------------------------*/
                  /* No, give them a message and fall through     */
                  /*----------------------------------------------*/
                  scratch[0] = (long)ua->ap_Info.fib_FileName;
                  VPrintf(msg, scratch);
                  rc = RETURN_FAIL;    /* 29-Dec-90 */
                  msg = NULL;
                  /*----------------------------------------------*/
                  /* Note:  RC2 is set above                      */
                  /*----------------------------------------------*/
                  break;
               }
               /*-------------------------------------------------*/
               /* By some miracle we have succeeded, start joins  */
               /*-------------------------------------------------*/
               rc = joinfile(infile, outfile, buff, buffsize, DOSBase);
               Close(infile);
               infile = NULL;
               if (rc) {
                  if (rc == DELETE_FILE_FLAG) {
                     Close(outfile);
                     DeleteFile((char *) opts[OPT_AS]);
                     outfile = NULL;
                     VPrintf(MSG_DEST_REMOVED, &opts[OPT_AS]);  /* 29-Dec-90 */
                  }
                  rc = RETURN_FAIL;
                  scratch[0] = (long) ua->ap_Info.fib_FileName;
                  break;
               }

               /*-------------------------------------------------*/
               /* Advance to the next file that might match       */
               /*-------------------------------------------------*/
               if (rc = MatchNext(ua))
                  rc2 = IoErr();
               msg = NULL;
            }
            scratch[0] = (long) curarg;
            MatchEnd(ua);
            if (rc2 != ERROR_NO_MORE_ENTRIES) break;
         }      /* End of while for multi-args stuff */
      } /* End of if to keep from advancing NULL pointer */
      FreeVec(buff);
      FreeVec(ua);
      }         /* End of else for pattern match stuff */
   }    /* End of else for handling valid options */

done:
   if (infile) Close(infile);
   if (outfile) Close(outfile);

   if (msg || rc) {
      /*-------------------------------------------------------*/
      /* Not a ^C, how about a normal exit of no more entries  */
      /*-------------------------------------------------------*/
      if (rc2 == ERROR_NO_MORE_ENTRIES) {
         rc = RETURN_OK;
      }

      else if (msg || (rc2 == ERROR_OBJECT_IN_USE)) {
         if (rc2 == ERROR_OBJECT_IN_USE) {
             msg = MSG_CANT_OPEN;
         }
         rc = RETURN_FAIL;    /* 29-Dec-90 */
         VPrintf(msg, scratch);
      }

      else {
         if (rc2) PrintFault(rc2, NULL);
         if (rc2 == ERROR_BREAK)rc = RETURN_WARN;  /* 29-Dec-90 */
         else rc = RETURN_FAIL;
         }
   }
   FreeArgs(rdargs);
   CloseLibrary((struct Library *) DOSBase);
} 
else {OPENFAIL;}
return(rc);
}


int joinfile(BPTR infh,BPTR outfh,void *buff,long buffsize,struct DosLibrary *DOSBase)
{
LONG pos, len;
int rc;

rc = RETURN_OK;

if (IsInteractive(infh)) {
   pos = buffsize;
}
else {
   if (Seek(infh, 0, OFFSET_END) < 0) {
      return(IoErr());
   }
   if ((pos = Seek(infh, 0, OFFSET_BEGINNING)) < 0) {
      return(IoErr());
   }
}

do {
   len = min(pos, buffsize);
   if ((len = Read(infh, buff, len)) <= 0) {
      /*----------------------------------------------------*/
      /* This will occur on EOF in Interactive mode         */
      /*----------------------------------------------------*/
      PrintFault(IoErr(), NULL);
      if (!IsInteractive(infh))
         rc = RETURN_FAIL;
      break;          /* Read failure when not Interactive mode */
   }
   /*--------------------------------------------------------*/
   /* Check to see if all the bytes read were written        */
   /*--------------------------------------------------------*/
   if (Write(outfh, buff, len) != len) {
      PrintFault(IoErr(), NULL);
      /*---------------------------------------------------*/
      /* Using rc as a flag, there is no return code 30    */
      /* This flag is used to separate other failures from */
      /* this one, since this one requires that the file   */
      /* be removed.                                       */
      /*---------------------------------------------------*/
      rc = DELETE_FILE_FLAG;
      break;
   }
   if (CheckSignal(SIGBREAKF_CTRL_C)) {
      PrintFault(ERROR_BREAK, NULL);
      rc = RETURN_WARN;
      break;
   }
} while ( (pos-= len) > 0);

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
