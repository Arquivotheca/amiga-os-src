; /*
lc -d -j73 -O -o/obj/remrad.o -i/include -v -csf remrad
blink /obj/remrad.o to /bin/remrad sc sd nd
protect /bin/remrad +p
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

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: remrad                                                      */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 01FEB90  John Toebes   Initial Creation                              */
/* 26AUG90  John Toebes   B9137 - reworded no ram drive message         */
/* 10DEC90  John Toebes   B10206 - Template is wrong                    */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "dos/filehandler.h"
#include "remrad_rev.h"

#define MSG_RAMDRIVE "ramdrive.device"
#define MSG_BADDEV   "Invalid Device name %s\n"
#define MSG_IDUNNO   "Unable to access Device list\n"
#define MSG_NOTFOUND "%s: is not a mounted device\n"
#define MSG_NODEVICE "Missing Device Node for %s:\n"
#define MSG_NOTRAD   "%s: is not a Recoverable Ram Drive\n"
#define MSG_NORAD0   "No Recoverable Ram Drive Mounted on Unit 0\n"
#define MSG_NOLOCK   "Unable to access %s:\n"
#define MSG_INUSE    "%s is currently still in use - Use FORCE to override\n"

#define TEMPLATE "DEVICE,FORCE/S" CMDREV
#define OPT_DEVICE 0
#define OPT_FORCE  1
#define OPT_COUNT 2

#pragma libcall RamdriveBase KillRAD 30 1
void KillRAD( ULONG );

long cmd_remrad(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   long opts[OPT_COUNT];
   int rc;
   char *msg, *name, *p;
   BPTR lock;
   char buf[5];
   struct RDargs *rdargs;
   struct MsgPort *task;
   struct DosList *DosList, *RootDosList;
   struct FileSysStartupMsg *fssm;
   struct FileLock *flock;
   struct Library *RamdriveBase;

   rc = RETURN_FAIL;

   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         if(!(RootDosList = LockDosList(LDF_ALL|LDF_READ)))
         {
            msg = MSG_IDUNNO;
         }
         else
         {
            rc = RETURN_ERROR;
            name = (char *)opts[OPT_DEVICE];
            if (name != NULL)
            {
               p = name;
               while ((*p) && (*p != ':')) p++;
               if (p[0] != ':' || p[1] != '\0' || p == name)
               {
                  DosList = NULL;
                  msg = MSG_BADDEV;
               }
               else
               {
                  *p = '\0';

                  /* If they gave us a name then we need to search the device */
                  /* list for their requested item                            */
                  msg = MSG_NOTFOUND;
                  DosList = FindDosEntry(RootDosList, name, LDF_ALL);
                  if ((DosList != NULL) &&
                      (DosList->dol_Type != DLT_DEVICE))
                  {
                     task = DosList->dol_Task;
                  
                     msg = MSG_NODEVICE;
                     DosList = RootDosList;
                     while(DosList=NextDosEntry(DosList, LDF_DEVICES))
                        if (DosList->dol_Task == task) break;
                  }
                  if (DosList != NULL)
                  {
                     /* We found a device entry.  Make sure it is ramdrive    */
                     /* that they are asking us for.                          */
                     fssm = (struct FileSysStartupMsg *)
                            BADDR(DosList->dol_misc.dol_handler.dol_Startup);
                     if (((long)fssm <= 0x100) ||
                         (BADDR(fssm->fssm_Device) == NULL) ||
                         (strcmp(((char *)BADDR(fssm->fssm_Device))+1,
                                 MSG_RAMDRIVE)))
                     {
                        /* Not Ramdrive.device, let them know                 */
                        msg = MSG_NOTRAD;
                        DosList = NULL;
                     }
                  }
               }
            }
            else
            {
               /* They didn't give us one, we need to search the device    */
               /* list looking one with a FSSM that indicates a device of  */
               /* ramdrive.device with a unit number of zero               */
               msg = MSG_NORAD0;
               DosList = RootDosList;
               while(DosList=NextDosEntry(DosList, LDF_DEVICES))
               {
                  fssm = (struct FileSysStartupMsg *)
                         BADDR(DosList->dol_misc.dol_handler.dol_Startup);
                  if (((long)fssm > 0x100) &&
                      (fssm->fssm_Unit == 0))
                  {
                     name = (char *)BADDR(fssm->fssm_Device);
                     if (name != NULL && !strcmp(name+1, MSG_RAMDRIVE))
                     {
                        opts[OPT_DEVICE] = (long)
                                  ((char *)BADDR(DosList->dol_Name))+1;
                        break;
                     }
                  }
               }
            }
            /* At this point, DosList will either be NULL in the event  */
            /* we could not locate the entry they requested, OR it      */
            /* will be a pointer to the entry.                          */
            /* fssm will point to the startup message                   */
            if (DosList != NULL)
            {
               msg = MSG_NOLOCK;
               if (DosList->dol_Task != NULL)
               {
                  /* Now see if we have any locks outstanding.  To do this,*/
                  /* We need to get a lock on the device (the root will be */
                  /* fine) and then inhibit the device.  If we are the only*/
                  /* lock on the list then all is fine.                    */
                  name = BADDR(MKBADDR(buf+3));
                  name[0] = 1;
                  name[1] = ':';
                  lock = (BPTR)DoPkt(DosList->dol_Task,
                                     ACTION_LOCATE_OBJECT,
                                     NULL, MKBADDR(name), SHARED_LOCK, 0);
                  if (lock != NULL)
                  {
                     DoPkt(DosList->dol_Task, ACTION_INHIBIT,
                                              DOSTRUE, 0, 0, 0);
                     flock = BADDR(lock);
                      if (!opts[OPT_FORCE] &&
                        (flock->fl_Volume == NULL ||
                         ((struct DeviceList *)
                           BADDR(flock->fl_Volume))->dl_LockList != lock  ||
                         flock->fl_Link != NULL))
                     {
                        /* Device in use, let them know */
                        msg = MSG_INUSE;
                        DoPkt(DosList->dol_Task, ACTION_INHIBIT,
                                                 DOSFALSE, 0, 0, 0);
                     }
                     else
                     {
                        /* It looks good, it is already inhibited so remove */
                        /* the ramdrive.                                    */
                        RamdriveBase = (struct Library *)
                                       FindName(&EXECBASE->DeviceList,
                                                MSG_RAMDRIVE);
                        if (RamdriveBase != NULL)
                        {
                           KillRAD(fssm->fssm_Unit);
                        }
                        rc = RETURN_OK;
                        msg = NULL;
                     }
                  }
               }
            }
            UnLockDosList(LDF_ALL|LDF_READ);
         }
         if (msg != NULL) VPrintf(msg, opts);
         /*-------------------------------------------------------------*/
         /* Finally we clean up and exit                                */
         /*-------------------------------------------------------------*/
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
