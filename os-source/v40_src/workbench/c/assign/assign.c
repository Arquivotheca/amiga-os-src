/*
lc -O -d -j73 -o/obj/Assign.o -i/include -rr -v -csf Assign
blink /obj/Assign.o lib lib:lcr.lib to /bin/Assign sc sd nd batch
protect /bin/Assign +p
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
/* Command: Assign                                                      */
/* Author:  Doug Walker                                                 */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 28MAR89  Doug Walker   First implementation                          */
/* 06NOV89  Doug Walker   Revved for dos.library 36.38                  */
/* 06DEC89  John Toebes   Final cleanup for checkin                     */
/* 07JAN90  Doug Walker   Corrected multi-args bug                      */
/* 04APR91  Doug Walker   Fixed uninitialized variable 'msg'            */
/* 22APR91  Doug Walker   Fixed truncation of long assign names w/LIST  */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "assign_rev.h"

#define TEMPLATE    "NAME,TARGET/M,LIST/S,EXISTS/S,DISMOUNT/S,DEFER/S,PATH/S,ADD/S,"\
                    "REMOVE/S,VOLS/S,DIRS/S,DEVICES/S" CMDREV
#define OPT_NAME    0
#define OPT_TARGET  1
#define OPT_LIST    2
#define OPT_EXIST   3
#define OPT_DISMOUNT  4
#define OPT_DEFER   5
#define OPT_PATH    6
#define OPT_ADD     7
#define OPT_REMOVE     8
#define OPT_VOL     9
#define OPT_DIR    10
#define OPT_DEV    11
#define OPT_COUNT  12

#define MAXASNLEN 30  /* Max length for ASSIGN names */

/* Messages - replace for foreign-language versions */
#define MSG_BADDEV      "Invalid device name %s\n"
#define MSG_NODIR       "Can't find %s\n"
#define MSG_INUSE       "Can't cancel %s\n"
#define MSG_IDUNNO      "Can't assign %s\n"
#define MSG_VOLUMES     "Volumes:\n"
#define MSG_ASSIGNS     "\nDirectories:\n"
#define MSG_DEVICES     "\nDevices:\n"
#define MSG_BADLOCK     "Bad lock\n"
#define MSG_NOTASSIGNED "%s: not assigned\n"
#define MSG_MOUNTED     " [Mounted]\n"
#define MSG_CONFLICT    "Only one of ADD, SUB, PATH, or DEFER allowed\n"
#define MSG_NOSUB       "Can't subtract %s from %s\n"
#define MSG_NOADD       "Can't add %s to %s\n"
#define MSG_LOOP        "Assign would refer to itself\n"


struct BUFINFO
{
   struct BUFINFO *next;
   char *buf;
   int used;
   int max;
};

void PutName(struct DosLibrary *DOSBase, BPTR lock, struct BUFINFO **buf);

#define MyPutStr(s, b) MyWriteChars(s, strlen(s), b)
int MyWriteChars(char *s, int len, struct BUFINFO **buf);
int MyFlush(struct DosLibrary *DOSBase, struct BUFINFO *buf);
char *CopyAsnName(UBYTE *name, UBYTE *path);
char *CheckPath(struct DosLibrary *DOSBase, UBYTE *asnname,
              UBYTE *asnto, void **work, long nwork);

int assign_main(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
struct DosLibrary *DOSBase;
struct DosList *DosList;
ULONG flags;
int cnt, rc, rc2, tlen, nodetype, didone;
char *msg, *tmpchar;
long opts[OPT_COUNT];
struct AssignList *alist;
struct RDargs *rdargs;
char **argptr;
char *name, *target;
BPTR lock;
struct BUFINFO obuf;
struct BUFINFO *buf;
char firstbuf[1500];  /* NOTE that this is a large stack allocation.   */
                      /*      It can safely be reduced to any number   */
                      /*      over 255, but will result in more memory */
                      /*      being allocated.  Everything else uses   */
                      /*      less than 500 bytes of stack, including  */
                      /*      the 256-byte auto array in PutName, so   */
                      /*      our stack usage should be less than 2k.  */

/* Set up the output buffer in case we need it */
obuf.next = NULL;
obuf.buf = firstbuf;
obuf.used = 0;
obuf.max = sizeof(firstbuf);
msg = NULL;

rc = RETURN_FAIL;
if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))
{
   memset((char *)opts, 0, sizeof(opts));
   rdargs = ReadArgs(TEMPLATE, opts, NULL);

   if(rdargs == NULL)
   {
      PrintFault(IoErr(), NULL);
   }
   else
   {
      rc = rc2 = 0;

      name = (char *)opts[OPT_NAME];

      if(name)
      {
         for(tmpchar=name; tmpchar[0] != ':' && tmpchar[1] != '\0'; tmpchar++);

         if(tmpchar[0] != ':' || tmpchar[1] != '\0' || tmpchar == name ||
             ((unsigned long)tmpchar - (unsigned long)name) > MAXASNLEN)
         {
            msg = MSG_BADDEV;
            rc = RETURN_FAIL;
            goto ERRORCASE;
         }
         /* Strip the ':' from the name */
         *tmpchar = '\0';
      }

      argptr=(char **)opts[OPT_TARGET];

      target = (argptr ? *argptr : NULL);

      /* Do sanity checks on the arguments */
      if(opts[OPT_EXIST])
      {
         /* EXISTS overrides LIST silently */
         opts[OPT_LIST] = 0;
      }
      else if(opts[OPT_VOL] || opts[OPT_DIR] || opts[OPT_DEV])
      {
         /* Any one of the VOL, DIR or DEV guys implies LIST */
         opts[OPT_LIST] = DOSTRUE;
      }
      else if(opts[OPT_LIST] || !name)
      {
         /* LIST without VOL, DIR or DEV implies all three */
         opts[OPT_LIST] = opts[OPT_VOL] =
         opts[OPT_DIR]  = opts[OPT_DEV] = DOSTRUE;
      }

      if( !target &&
         (opts[OPT_DEFER] || opts[OPT_PATH] || opts[OPT_ADD] || opts[OPT_REMOVE]))
      {
         /* If no target is specified, DEFER, PATH, ADD and SUB are meaningless */
         opts[OPT_DEFER] = opts[OPT_PATH] = opts[OPT_ADD] = opts[OPT_REMOVE] = 0;
      }

      if( (opts[OPT_ADD] != 0)  + (opts[OPT_DEFER] != 0) + 
          (opts[OPT_PATH] != 0) + (opts[OPT_REMOVE] != 0) > 1)
      {
         /* Conflicting options */
         msg = MSG_CONFLICT;
         rc = RETURN_FAIL;
         goto ERRORCASE;
      }

      if(name && !opts[OPT_EXIST] && !opts[OPT_DISMOUNT])
      {
         do
         {
            if(CheckSignal(SIGBREAKF_CTRL_C))
            {
               PrintFault(ERROR_BREAK, NULL);
               rc = RETURN_FAIL;
               rc2 = ERROR_BREAK;
               goto ERRORCASE;
            }

            /* We now should assign <name> to <target> */
            if(opts[OPT_DEFER])
            {
               /* We know we have a target or DEFER would have been reset above */
               if(msg = CheckPath(DOSBase, name, target, 
                                  (void **)firstbuf, sizeof(firstbuf)))
               {
                  rc = RETURN_FAIL;
                  rc2 = (rc2 == -1 ? IoErr() : 0);
               }
               else if(!AssignLate(name, target))
               {
                  msg = MSG_IDUNNO;
                  rc2 = IoErr();
               }
            }
            else
            {
               lock = NULL;
               if(target && !opts[OPT_PATH] && !(lock=Lock(target, SHARED_LOCK)))
               {
                  /* Specified directory does not exist */
                  msg = MSG_NODIR;
                  rc2 = IoErr();
                  opts[0] = (LONG)target;
                  goto ERRORCASE;
               }
               if(opts[OPT_ADD] || opts[OPT_REMOVE])
               {
                  DosList = LockDosList(LDF_ASSIGNS|LDF_WRITE);

                  /* Find the entry on the DosList;  if found,       */
                  /* make sure it is a DIRECTORY type entry, which   */
                  /* is the only kind that currently supports multi- */
                  /* directory assigns.  Then do the add or sub as   */
                  /* appropriate.                                    */
                  if( (DosList = FindDosEntry(DosList, name, LDF_ASSIGNS) )
                      &&
                      (
                        DosList->dol_Type != DLT_DIRECTORY 
                        ||
                        (opts[OPT_ADD] && !AssignAdd(name, lock) )
                        ||
                        (opts[OPT_REMOVE] && !RemAssignList(name, lock) )
                      )
                    )
                  {
                     msg = (opts[OPT_REMOVE] ? MSG_NOSUB : MSG_NOADD);
                     opts[0] = (LONG)target;
                     opts[1] = (LONG)name;
                     rc2 = IoErr();
                  }
                  UnLockDosList(LDF_ASSIGNS|LDF_WRITE);

                  if(opts[OPT_REMOVE]) UnLock(lock);

                  if(!DosList && !opts[OPT_REMOVE] && 
                     !AssignLock((UBYTE *)(name), lock))
                  {
                     msg = MSG_IDUNNO;
                     rc2 = IoErr();
                     UnLock(lock);
                  }
               }
               else if(opts[OPT_PATH])
               {
                  UnLock(lock);

                  if(msg = CheckPath(DOSBase, name, target, 
                                     (void **)firstbuf, sizeof(firstbuf)))
                  {
                     rc = RETURN_FAIL;
                     rc2 = (rc2 == -1 ? IoErr() : 0);
                  }
                  else if(!AssignPath(name, target))
                  {
                     msg = MSG_IDUNNO;
                     rc2 = IoErr();
                  }
               }
               else
               {
                  /* A simple assign or reassign of a name to a dir                 */
                  /* If no OPT_TARGET was specified, this just cancels the old one  */
                  if(AssignLock(name, lock) != DOSTRUE)
                  {
                     msg = MSG_INUSE;
                     rc2 = IoErr();
                  }

                  /* If we come through the loop again, we add to this one */
                  /* instead of recreating the old one...                  */
                  opts[OPT_ADD] = DOSTRUE;
               }
            }
            if(msg) goto ERRORCASE;
         }
         while(target && (target = *(++argptr)));  /* Assignment! */
      }

      if(opts[OPT_DISMOUNT] && name)
      {
         DosList = LockDosList(LDF_ALL|LDF_WRITE);
         if(DosList = FindDosEntry(DosList, name, LDF_ALL))
         {
            RemDosEntry(DosList);

#if 0
            /* In my opinion, we should NOT free the memory */
            /* associated with the node; it will increase   */
            /* the odds of a crash.                         */
            /* However, if you insist:                      */
            FreeVec(DosList);
#endif
         }
         
         UnLockDosList(LDF_ALL|LDF_WRITE);
      }

      if(opts[OPT_LIST] || opts[OPT_EXIST])
      {
         /* Whether we are examining volumes, devices or directories, we */
         /* want them sorted by type.  Make three passes over the list,  */
         /* only examine nodes of the appropriate type on each pass      */

         for(didone=cnt=nodetype=0; nodetype<3; nodetype++)
         {
            if(nodetype == 0)
            {
               tmpchar = MSG_VOLUMES;
               flags   = LDF_VOLUMES;
            }
            else if(nodetype == 1)
            {
               tmpchar = MSG_ASSIGNS;
               flags   = LDF_ASSIGNS;
            }
            else
            {
               tmpchar = MSG_DEVICES;
               flags   = LDF_DEVICES;
            }
               
            if(opts[OPT_LIST]) 
            {
               /* The following depends on OPT_VOL, OPT_DIR and OPT_DEV   */
               /* being consecutive and in that order in the 'opts' array */

               if(!opts[OPT_VOL + nodetype]) continue;

               PutStr(tmpchar);
            }

            buf = &obuf;
            if(!(DosList = LockDosList(flags|LDF_READ)))
            {
               msg = MSG_IDUNNO;
               rc2 = IoErr();
               goto ERRORCASE;
            }

            while(DosList=NextDosEntry(DosList, flags))
            {
               if(opts[OPT_EXIST])
               {
                  if(!(DosList=FindDosEntry(DosList, name, flags)))
                     break;
                  didone = 1;
               }

               tmpchar = (char *)BADDR(DosList->dol_Name);
               tlen = tmpchar[0];
               /* 22Apr91/djw */
               /* if(nodetype == 1) tlen = (tlen > 14 ? 14 : tlen); */
               MyWriteChars(tmpchar+1, tlen, &buf);

               switch(nodetype)
               {
                  case 0:
                     MyPutStr(DosList->dol_Task ? MSG_MOUNTED : "\n", &buf);
                     break;

                  case 1:
                     tmpchar = (char *)DosList->dol_misc.dol_assign.dol_AssignName;
                     MyWriteChars("               ", 
                                  (tlen < 15 ? 15-tlen : 1), &buf);
                     switch(DosList->dol_Type)
                     {
                        case DLT_DIRECTORY:
                           PutName(DOSBase, DosList->dol_Lock, &buf);

                           if(alist=DosList->dol_misc.dol_assign.dol_List)
                           {
                              for(; alist; alist = alist->al_Next)
                              {
                                 MyPutStr("             + ", &buf);
                                 PutName(DOSBase, alist->al_Lock, &buf);
                              }
                           }
                           break;


                        case DLT_LATE:
                           /* Late-binding assign */
                           MyWriteChars("<", 1, &buf);
                           MyPutStr(tmpchar, &buf);
                           MyWriteChars(">\n", 2, &buf);
                           break;

                        case DLT_NONBINDING:
                           MyWriteChars("[", 1, &buf);
                           MyPutStr(tmpchar, &buf);
                           MyWriteChars("]\n", 2, &buf);
                           break;

                     }
                     break;

                  case 2:
                     if(opts[OPT_EXIST] || ++cnt == 5)
                     {
                        cnt = 0;
                        MyPutStr("\n", &buf);
                     }
                     else
                     {
                        MyPutStr(" ", &buf);
                     }
                     break;

               }
            }
            UnLockDosList(flags|LDF_READ);
            if(rc2=MyFlush(DOSBase, &obuf)) 
            {
               rc = RETURN_FAIL;
               PrintFault(rc2, NULL);
               goto ERRORCASE;
            }
         }
         if(cnt > 0) PutStr("\n");  /* Might need a last LF for list */
         if(opts[OPT_EXIST] && !didone)
         {
            msg = MSG_NOTASSIGNED;
            rc = RETURN_WARN;
         }
      }
   }

ERRORCASE:
   /* Free any memory used for outputting the lists */
   for(buf=obuf.next; buf; buf=obuf.next)
   {
      obuf.next = buf->next;
      FreeMem((char *)buf, buf->max+sizeof(struct BUFINFO));
   }
   if(msg) VPrintf(msg, opts);
   FreeArgs(rdargs);
   SetIoErr(rc2);
   CloseLibrary((struct Library *)DOSBase);
}
else
{
   OPENFAIL;
   rc = RETURN_FAIL;
}

if(!rc && rc2) rc = RETURN_FAIL;

return(rc);
}

void PutName(struct DosLibrary *DOSBase, BPTR lock, struct BUFINFO **buf)
{
   struct DeviceList *dlist;
   char *tmpchar;
   char work[256];

   if(lock)
   {
      dlist = (struct DeviceList *)
         BADDR(((struct FileLock *)BADDR(lock))->fl_Volume);
      if(!dlist || !dlist->dl_Task)
      {
         /* Volume this lock is associated with is not mounted */
         MyPutStr("Volume: ", buf);
         if(dlist) 
         {
            tmpchar = (char *)BADDR(dlist->dl_Name);
            MyWriteChars(tmpchar+1, tmpchar[0], buf);
         }
         else
            MyPutStr("???", buf);
      }
      else
      {
         /* Volume is mounted, get the full name */
         work[0] = '\0';
         NameFromLock(lock, work, 255);
         MyPutStr(work, buf);
      }
   }
   MyPutStr("\n", buf);
}

#define BUFINCR 1024
int MyWriteChars(char *str, int len, struct BUFINFO **buf)
{
   struct BUFINFO *cur;
   struct Library *SysBase = (*((struct Library **)4));

   cur = *buf;
   if(cur->used < 0) return(1);  /* Out of memory earlier */

   if(cur->used + len >= cur->max)
   {
      if(cur->next) 
         cur = cur->next;  /* Already had one from previous time */
      else
      {
         if(!(cur->next = (struct BUFINFO *)
            AllocMem(sizeof(struct BUFINFO)+BUFINCR, 0)))
         {
            cur->used = -1;  /* Signal out of memory */
            return(1);
         }
         cur->next->max = BUFINCR;
         cur = cur->next;
         cur->next = NULL;
      }
      cur->used = 0;
      cur->buf = (char *)(cur+1);
      *buf = cur;
   }

   memcpy(cur->buf+cur->used, str, len);
   cur->used += len;

   return(0);
}

#define OUTINCR 100  /* Number of bytes to output at once (allows for ctrl-c) */
int MyFlush(struct DosLibrary *DOSBase, struct BUFINFO *buf)
{
   char *tmp;
   int cur;
   while(buf)
   {
      if(buf->used > 0)
      {
         for(tmp=buf->buf, cur=buf->used; cur>0; cur-=OUTINCR, tmp+=OUTINCR)
         {
            if(CheckSignal(SIGBREAKF_CTRL_C))
               return(ERROR_BREAK);

            WriteChars(tmp, (cur > OUTINCR ? OUTINCR : cur));
         }
      }
      else if(buf->used == 0)
         return(0);
      else
         return(ERROR_NO_FREE_STORE);
      buf->used = 0;
      buf=buf->next;
   }
}

/* Make sure a PATH assign isn't an endless loop. */
/* return                                         */
/*    0 == OK                                     */
/*    1 == loop                                   */
/*   -1 == DOS error (IoErr for more info)        */
/*   -2 == internal error                         */

char *CheckPath(struct DosLibrary *DOSBase, UBYTE *asnname,
              UBYTE *asnto, void **work, long nwork)
{
   struct DosList *DosListRoot, *DosList;
   int nused = 0;
   int n;
   char *msg;
   UBYTE name[MAXASNLEN+1];

   DosListRoot = LockDosList(LDF_ASSIGNS|LDF_READ);

   if(DosListRoot == NULL) return(MSG_IDUNNO);

   msg = NULL;

   nwork /= sizeof(struct DosList *);

   while(1)
   {
      /* Get the name before the ':' */
      /* if no name before the ':', 1 is returned */
      if((msg=CopyAsnName(name, asnto)) || name[0] == 0)
         goto GETOUT;

      /* Special case - if this guy refers to the name we are */
      /* attempting to assign, we have a loop.                */
      if(!stricmp(asnname, name))
      { 
         msg = MSG_LOOP;
         goto GETOUT;
      }

      DosList = FindDosEntry(DosListRoot, name, LDF_ASSIGNS);

      if(DosList == NULL || 
          (
           DosList->dol_Type != DLT_NONBINDING 
            &&
           DosList->dol_Type != DLT_LATE
          )
        )
      {
         /* Can't be a loop if the thing it's assigned to does not exist */
         /* or is a volume, device or resolved assign.                   */
         goto GETOUT;
      }

      for(n=0; n<nused; n++)
      {
         if(work[n] == (void *)DosList)
         {
            /* This is an entry we've seen already */
            msg = MSG_LOOP;
            goto GETOUT;
         }
      }

      if(nused >= nwork)
      {
         msg = MSG_IDUNNO;
         goto GETOUT;
      }

      /* Remember this entry.  If we ever come back to it, */
      /* we know we are in a loop.                         */
      work[nused++] = (void *)DosList;

      asnto = (char *)DosList->dol_misc.dol_assign.dol_AssignName;
   }

GETOUT:

   UnLockDosList(LDF_ASSIGNS|LDF_READ);

   return(msg);
}

char *CopyAsnName(UBYTE *name, UBYTE *path)
{
   int n;

   for(n=0; path[n] && path[n] != ':'; n++);

   if(path[n] == 0) 
   {
      name[0] = 0;
      return(NULL);    /* No colon seen at all */
   }

   if(n >= MAXASNLEN) return(MSG_IDUNNO); /* Invalid name */

   memcpy(name, path, n);            /* Copy the name over    */
   name[n] = 0;                      /* and null-terminate it */

   return(NULL);
}
