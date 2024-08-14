; /*
lc -j73 -d -rr -O -o/obj/MakeLink.o -i/include -v -csf MakeLink
blink /obj/MakeLink.o to /bin/MakeLink sc sd nd batch
protect /bin/MakeLink +p
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

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: MakeLink                                                    */
/* Author:  John A. Toebes                                              */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 06DEC89  John Toebes   Initial Creation                              */
/* 14DEC89  John Toebes   Changed to use LINK type #defines             */
/* 07NOV90  John Toebes   Corrected -1 returncode problem               */
/* 24MAR91  John Toebes   Added code to check for uplevel links         */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "makelink_rev.h"

/* There is no \n on this message to leave it for printfault    */
#define MSG_CANTFIND "Can't find %s "
#define MSG_UPLEVEL  "Link loop from %s to %s not allowed\n"
#define MSG_DIRLINK  "Links to directories require use of the FORCE keyword\n"

#define TEMPLATE    "FROM/A,TO/A,HARD/S,FORCE/S" CMDREV
#define OPT_FROM    0
#define OPT_TO      1
#define OPT_HARD    2
#define OPT_FORCE   3
#define OPT_COUNT   4

int isuplevel(struct DosLibrary *DOSBase,BPTR lock,char *name);

int cmd_makelink(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   struct RDargs *rdargs;
   int rc;
   int rc2;
   BPTR lock;
   LONG opts[OPT_COUNT];

   rc = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
      memset((char *)opts, 0, sizeof(opts));
      if ((rdargs = ReadArgs(TEMPLATE, opts, NULL)) == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         /*-------------------------------------------------------------*/
         /* Determine the type of link to create.  If they do not       */
         /* tell us otherwise, we will assume a hard link               */
         /*-------------------------------------------------------------*/
#if 0
         if (opts[OPT_SOFT])
         {
            rc = !MakeLink((char *)opts[OPT_FROM], opts[OPT_TO], LINK_SOFT);
            rc2 = IoErr();
         }

         /*-------------------------------------------------------------*/
         /* For a hard link, we must lock the object to pass to the     */
         /* MakeLink routine.                                           */
         /*-------------------------------------------------------------*/
	else
#endif
         {
            lock = Lock((char *)opts[OPT_TO], SHARED_LOCK);
            if (lock == NULL)
            {
               /*-------------------------------------------------------*/
               /* We couldn't find the object, so let them know.        */
               /*-------------------------------------------------------*/
	       rc = 1;
               rc2 = IoErr();
               VPrintf(MSG_CANTFIND, opts+OPT_TO);
            }
            else
            {
               /*-------------------------------------------------------*/
               /* Got a lock on the object, ask him to make a link      */
               /*-------------------------------------------------------*/
               if (rc2 = isuplevel(DOSBase, lock, (char *)opts[OPT_FROM]))
	       {
		  if (rc2 == -1)
		  {
		     VPrintf(MSG_UPLEVEL, opts);
		     rc2 = 0;
		  }
		  else if (rc2 == 1)
		  {
		     if (opts[OPT_FORCE]) goto dolink;
		     VPrintf(MSG_DIRLINK, opts+1);
		     rc2 = 0;
		  }
		  rc = 1;
	       }
               /*-------------------------------------------------------*/
               /* Before we do this, we need to make sure that it is    */
               /* not a link to an uplevel directory.                   */
               /*-------------------------------------------------------*/
	       else
	       {
dolink:
                  rc = !MakeLink((char *)opts[OPT_FROM], (long)lock, LINK_HARD);
                  rc2 = IoErr();
                  UnLock(lock);
               }
            }
         }
         if (rc) {
            PrintFault(rc2, NULL);
            rc = RETURN_FAIL;
         }
         FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      OPENFAIL;
   }
   return(rc);
}

int isuplevel(DOSBase, lock, name)
struct DosLibrary *DOSBase;
BPTR lock;
char *name;
{
   char *p, c;
   struct FileInfoBlock __aligned fib;
   BPTR testlock, parent;

   /* If we are unable to examine the lock, something is wrong, so just quit */
   if (!Examine(lock, &fib)) return(IoErr());
   /* If the target object is only a file, just let it ride, it can't cause a problem */
   if (fib.fib_EntryType < 0) return(0);
   
   /* We know that the target object is a directory.  Now go locate where we would */
   /* put the new link and see if any parent of the target link would be the       */
   /* directory.  								   */
   p = PathPart(name);
   c = *p;
   *p = '\0';
   testlock = Lock(name, ACCESS_READ);
   *p = c;
   /* If we can't lock the directory it doesn't exist or something else is wrong */
   if (testlock == NULL) return(IoErr());
   
   /* Now bruteforce your way through the parent chain checking to see if there */
   /* is anyone who is the same as the target.                                  */
   while(testlock != NULL)
   {
      if (SameLock(lock, testlock) == LOCK_SAME)
      {
	 UnLock(testlock);
         return(-1);
      }
      /* This level didn't match, go to the parent and try again */
      parent = ParentDir(testlock);
      UnLock(testlock);
      testlock = parent;
   }
   return(1);
}
