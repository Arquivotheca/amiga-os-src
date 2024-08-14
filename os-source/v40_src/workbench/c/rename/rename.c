; /*
lc -d -j73 -O -o/obj/Rename.o -i/include -v -csf Rename
blink /obj/Rename.o to /bin/Rename sc sd nd
protect /bin/Rename +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-----------------------------------------------------------------------*/
/* Command: Rename                                                       */
/* Author:  Gordon Keener                                                */
/* Change History:                                                       */
/*  Date    Person        Action                                         */
/* -------  ------------- -----------------                              */
/* 19MAR89  John Toebes   Initial Creation                               */
/* 25Apr89  Gordon Keener Actual Implementation                          */
/* 20Oct89  Gordon Keener ..and again                                    */
/* 12Dec89  Jay Denebeim  Added wildcards                                */
/* 07Jan90  Gordon Keener (thanks, Jay!) Bug fixes                       */
/* 09Jan90  Gordon Keener Bug fixes, stole more of Jay's code from copy  */
/* Notes:                                                                */
/*-----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "rename_rev.h"

#define MSG_CATCHALL    "Can't rename %s as %s because "
#define MSG_NODIR       "Destination \"%s\" is not a directory.\n"
#define MSG_RENAME      "Renaming %s as %s\n"

#define TEMPLATE    "FROM/A/M,TO=AS/A,QUIET/S" CMDREV
#define OPT_FROM    0
#define OPT_TO      1
#define OPT_QUIET   2
#define OPT_COUNT   3

/*---------------------------------------------------------------------------*/
/* Structure used by the pattern matching routines.                          */
/*---------------------------------------------------------------------------*/
struct uAnchor {
  struct AnchorPath uA_AP;
  BYTE uA_Path[256];
};
#define  MAX_PATH 256

int cmd_rename(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   int               rc;
   int               res2=0;
   int		     loopc;
   long              opts[OPT_COUNT];
   struct RDargs     *rdargs = NULL;
   struct uAnchor    *ua = NULL;
   char              *msgbuf = NULL;
   char              *destpath = NULL;
   char              *srcname = NULL;
   BPTR              destlock = NULL;
   char              **argptr, *cur;
   char              *destname;
   char              fibstr[sizeof(struct FileInfoBlock) + 3];
   struct FileInfoBlock *fib;
   int               msgblock[2], destisdir,sourcelock;

   rc = RETURN_FAIL;

   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
      if( CheckSignal( SIGBREAKF_CTRL_C ) ){
         PrintFault(ERROR_BREAK,NULL);
         goto Done;
      }

      /*-------------------------------------------------------------*/
      /* Allocate memory to use with the pattern match stuff.        */
      /*-------------------------------------------------------------*/
      if ((msgbuf = AllocVec(MSGBUFSIZE, NULL))              == NULL ||
          (ua = AllocVec(sizeof(struct uAnchor),MEMF_CLEAR)) == NULL ||
          (srcname  = AllocVec(MAX_PATH, NULL      ))        == NULL ||
          (destpath = AllocVec(MAX_PATH, MEMF_CLEAR))        == NULL ) {
         PrintFault(ERROR_NO_FREE_STORE, NULL);
         goto Done;
      }

      /*-------------------------------------------------------------*/
      /* It's time to parse our arguments!!!                         */
      /*-------------------------------------------------------------*/
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL) {
         res2 = IoErr();
         goto ErrFail;
      }

      rc = RETURN_OK;

      /*----------------------------------------------------------*/
      /* Now we have to set up the pattern match structure.       */
      /*                                                          */
      /* Tell the pattern routines that we want them to return up */
      /* to 255 chars of the full path to any matches.            */
      /*----------------------------------------------------------*/
      ua->uA_AP.ap_Strlen = 255;
      /*----------------------------------------------------------*/
      /* Set the flag which tells the pattern matcher to decode   */
      /* wild cards.                                              */
      /*----------------------------------------------------------*/
      ua->uA_AP.ap_Flags = APF_DOWILD;
      /*----------------------------------------------------------*/
      /* Tell the matcher to stop if the user presses Ctrl-C.     */
      /*----------------------------------------------------------*/
      ua->uA_AP.ap_BreakBits = SIGBREAKF_CTRL_C;

      /*----------------------------------------------------------*/
      /* We need to check for multiple arguments/wildcards. If we */
      /* get more than one source, we need to require a directory */
      /* for the destination.                                     */
      /*----------------------------------------------------------*/
      argptr = (char **)opts[OPT_FROM];
      if (MatchFirst(*argptr, (struct AnchorPath *)ua)) {
         res2 = IoErr();
         if(res2 == ERROR_OBJECT_NOT_FOUND) {
	    opts[OPT_FROM] = (long)*argptr;
            VPrintf(MSG_CATCHALL, opts);
	 }
         goto ErrFail;
      }

      /* We need to determine up front whether or not our     */
      /* destination is a directory. If it is, We need to go  */
      /* thru the multi/wildcard path, even if there is only  */
      /* one source file. Granted, I could add code to the    */
      /* simple case, but why duplicate the code for so little*/
      /* gain?                                                */
      /* and the answer is, so directories can be renamed ! */
      destisdir = FALSE;
      destlock = Lock((char *)opts[OPT_TO], SHARED_LOCK);
      if (destlock) {
         /* We need this to get a word-aligned fib pointer */
         fib = (struct FileInfoBlock *)(((long)fibstr + 3) & ~3);
         if (Examine(destlock, fib) && (fib->fib_DirEntryType > 0))
	     destisdir = TRUE;
      }
      /* Sanity check. If we're going to use multiargs and/or */
      /* wildcards, we'd better be renaming to a directory.   */
      if ((!destisdir)                                 &&
          (((ua->uA_AP.ap_Flags & APF_ITSWILD) != 0)   ||
           (argptr[1] != NULL)))
      {
         VPrintf(MSG_NODIR,&opts[OPT_TO]);
         goto Done;
      }

     if(destisdir && !argptr[1]) {
         sourcelock=Lock((char *)argptr[0], SHARED_LOCK);
         if(sourcelock) { /* we're just renaming a directory */
	    if( (SameLock(sourcelock,destlock))==LOCK_SAME)destisdir=FALSE;
            UnLock(sourcelock);
	 }
     }
      /* Done with our up-front test */
      MatchEnd((struct AnchorPath *)ua);

      /* We now have two cases to handle. If the destination  */
      /* isn't a directory, we use a simple Rename() call to  */
      /* act just like the 1.3 Rename.                        */
      if (!destisdir) {
	 ParsePattern((char *)*argptr,srcname,MAX_PATH);
/*         if (! Rename((char *)*argptr, (char *)opts[OPT_TO])) { */
         if (! Rename(srcname, (char *)opts[OPT_TO])) {
            /* If rename fails for any reason, no matter */
            /* what, print this stupid message           */
            res2 = IoErr();
            opts[OPT_FROM] = (long)*argptr;
            VPrintf(MSG_CATCHALL, opts);
            SetIoErr(res2);
         }
      }
      else {
         /* Get the full pathname for the destination. Here,  */
         /* we know destlock is OK.                           */
         NameFromLock(destlock, destpath, MAX_PATH);

         destname = &destpath[strlen(destpath)];

         /* If the path is not a device name, it's a directory, */
         /* and we need to add a trailing '/' so later cats     */
         /* work.                                               */
         if (destname[-1] != ':') {
            *destname++ = '/';
         }

         while (cur = *argptr++) {  /* assign */
            if (MatchFirst(cur, (struct AnchorPath *)ua)) {
               res2 = IoErr();
               goto ErrFail;
            }
            /* Loop thru each match for this arg. */
            do {
               /* Build our full pathname for the destination  */
               strcpy(destname, ua->uA_AP.ap_Info.fib_FileName);

               /* Copy out the srcname, because the MatchNext  */
               /* is going to clobber ua                       */
               strcpy(srcname,  ua->uA_AP.ap_Buf);

               /* Go to the next match here. MatchNext() gets  */
               /* confused if you rename its file out from     */
               /* under it.                                    */
               loopc = MatchNext((struct AnchorPath *)ua);

               /* Build msgblock for a VPrintf, for verbosity  */
               /* or for error.                                */
               msgblock[0] = (int)srcname;
               msgblock[1] = (int)destpath;

               /* Information message, unless we are shut up */
               if (! opts[OPT_QUIET])VPrintf(MSG_RENAME, msgblock);

               if (! Rename( (char *)srcname, destpath)) {
                  /* If rename fails for any reason, no matter */
                  /* what, print this message                  */
                  res2 = IoErr();
                  VPrintf(MSG_CATCHALL, msgblock);
                  SetIoErr(res2);
                  goto ErrFail;
               }
            } while (loopc == NULL);
            MatchEnd((struct AnchorPath *)ua);
         }
      }
ErrFail:
   if(res2) {
        MatchEnd((struct AnchorPath *)ua);
/*	GetProgramName( msgbuf, MSGBUFSIZE ); */
   	PrintFault(res2, NULL);
        rc = RETURN_FAIL;
  }
Done:
   if (rdargs)     FreeArgs(rdargs);
   UnLock(destlock);
   FreeVec(ua);
   FreeVec(msgbuf);
   FreeVec(destpath);
   FreeVec(srcname);

   CloseLibrary((struct Library *)DOSBase);
   }
   else {OPENFAIL;}
   return(rc);
}

