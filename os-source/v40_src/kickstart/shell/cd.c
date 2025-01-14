;/*
lc -O -M -d -j73 -ocd.o -iV:include -iV:inc.lattice -v -csf cd
blink cd.o to cd.ld LIB LIB:amiga.lib lib:debug.lib sc sd nd
protect cd.ld +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: CD                                                          */
/* Author:  Doug Walker                                                 */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 05APR89  Doug Walker   Filled in body                                */
/* 06NOV89  Doug Walker   Revved for dos.library 36.38                  */
/* 07JAN90  Doug Walker   Added support for wildcards                   */
/* 23MAY90  Doug Walker   Cleaned up wildcard problems                  */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "global.h"

/* #include "cd_rev.h" */

#define TEMPLATE    "DIR"
#define OPT_DIR     0
#define OPT_COUNT   1

#define SMALL 1

/* Macro to get longword-aligned stack space for a structure */
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately    */
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.                                        */
#define D_S(name, type) char a_##name[sizeof(type)+3]; \
                        type *name = (type *)((LONG)(a_##name+3) & ~3)
struct uAnchorPath
{
   struct AnchorPath ap;
   char name[255];
};

int cmd_cd(void)
{
   struct FileInfoBlock __aligned fib;
   struct uAnchorPath __aligned ap;
   struct DosLibrary *DOSBase;
   int rc, rc2;
   LONG opts[OPT_COUNT];
   char data[256];
   struct Process *process;
   BPTR lock;
   struct RDargs *rdargs;

   rc = RETURN_FAIL;
   rc2 = 0;
#ifndef SMALL
#if ram
   if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)) {
#else
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
#endif
      memset((char *)opts, 0, sizeof(opts));
      if(!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) {
         PrintFault(IoErr(), NULL);
      }
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
         rc = RETURN_OK;
         process = THISPROC;
         if(opts[OPT_DIR]) {
            /* First try for a direct match */
            if(lock=Lock((char *)opts[OPT_DIR], SHARED_LOCK)) {
               if(!Examine(lock, &fib) || fib.fib_DirEntryType < 0) {
                  rc= RETURN_FAIL;
                  if(!(rc2 = IoErr()))rc2=ERROR_OBJECT_WRONG_TYPE;
		  /* unlocking a null is ok */
                  UnLock(lock);
                  goto ERRORSPOT;
               }
               /* We now have a lock on the directory to which we want to CD */
               /* Fall through and call CurrentDir on it after this else     */
            }
	    else {
		rc2=IoErr();
		if((rc2 == ERROR_NO_DISK) || (rc2==ERROR_DEVICE_NOT_MOUNTED)) {
		    rc = RETURN_FAIL;
                    goto ERRORSPOT;
		}
		/* check if we want to do a wildcard match */
                else if (ParsePattern((char *)opts[OPT_DIR],data,255)> 0 ) {
                   memset(&ap, 0, sizeof(struct uAnchorPath));
                   ap.ap.ap_Flags = APF_DOWILD;
                   ap.ap.ap_Strlen = 255;
                   ap.ap.ap_BreakBits = SIGBREAKF_CTRL_C;

                   /* If they specified a wildcard, and it matches      */
                   /* exactly one DIRECTORY, assume that's our argument */
                   if(MatchFirst((char *)opts[OPT_DIR], &ap.ap)) {
                      /* No matches at all, forget it */
                      rc = RETURN_FAIL;
                      rc2 = ERROR_DIR_NOT_FOUND;
                      MatchEnd(&ap.ap);
                      goto ERRORSPOT;
		   }
               /*
                * OK, HERE'S THE PLAN:
                *
                * If the wildcard-matching stuff comes up with EXACTLY ONE directory
                * match, we will use that as our argument.  We have initialized
                * 'lock' to NULL.  We will obtain a lock on the first directory
                * match.  If we get another directory matching the specified wildcard
                * and 'lock' is not NULL, we have a conflict and we must fail.
                * If it is an error case, the extra lock will be freed after we
                * kick out of the loop.
                *
                *         --Doug
                */
               lock = NULL;
               do { /* while more matches */
                  if(ap.ap.ap_Info.fib_DirEntryType >= 0) { /* If it's a directory...*/
                     if(lock == NULL) { /* If we haven't hit a directory yet... */
                        BPTR tmplock;

                        /* We have matched a directory for the first time   */
                        /* CD to the directory the wildcard is relative to, */
                        /* try to lock the directory, and restore ourselves */
                        /* to the old current directory                     */

                        tmplock = CurrentDir(ap.ap.ap_Current->an_Lock);
                        lock = Lock(ap.ap.ap_Info.fib_FileName, SHARED_LOCK);
                        CurrentDir(tmplock);
                        if(lock == NULL)  { /* Failed to lock it - In use? */

                           rc = RETURN_FAIL;
                           rc2 = IoErr();
                        }
                     }
                     else {
                        /* Whoops - second directory match.  Give up.     */
                        /* Don't mess with 'lock' here - it will be freed */
                        /* in the error case below.                       */
			rc2= STR_TOO_MANY_MATCHES;
                        rc = RETURN_FAIL;
                     }
                  }
               }
               while(!rc && !MatchNext(&ap.ap));
               MatchEnd(&ap.ap);
	      	}
               if(rc || !lock)
               {
                  /* Some kind of failure.  If rc is not set, it's a simple  */
                  /* 'no such directory' error.  If lock is set, it's a case */
                  /* of too many matches, so free the lock.                  */

                  rc = RETURN_FAIL;
		  /* unlocking a null is ok */
                  UnLock(lock);
                  goto ERRORSPOT;
               }
            }

            if( (lock = CurrentDir(lock)) != NULL)   /* Assignment! */
               UnLock(lock);
         }

         /* Now we need to get the full name of the current directory */
         if(!NameFromLock(process->pr_CurrentDir, data, 255))
         {
            if((rc2=IoErr()) == ERROR_LINE_TOO_LONG)
            {
               /* No matter, we can only use 63 characters anyway */
               rc2 = 0;
            }
            else
            {
               rc = RETURN_ERROR;
               goto ERRORSPOT;
            }
         }

         if(opts[OPT_DIR])
         {
            if(!SetCurrentDirName(data))
            {
               /* We may have failed due to line too long    */
               /* If so, these return codes will get cleaned */
               /* up since that is really not an error       */
               rc2 = IoErr();
               rc = RETURN_FAIL;
            }
         }
         else
         {
            opts[0] = (LONG)data;
	    VPrintf("%s\n",opts);
         }
ERRORSPOT:
         FreeArgs(rdargs);
         if(rc && rc2) {
            PrintFault(rc2, NULL);
            /* Not really an error if dir name is too long */
            if(rc2 == ERROR_LINE_TOO_LONG)rc = rc2 = 0;
         }
         SetIoErr(rc2);
      }

      CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else
   {
      OPENFAIL;
   }
#endif
   return(rc);
}
