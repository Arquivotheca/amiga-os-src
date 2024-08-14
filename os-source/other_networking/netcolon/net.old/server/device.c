/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987 The Software Distillery. All Rights Reserved*/
/* |. o.| || This program may not be distributed without the permission of */
/* | .	| || the author.					   BBS:    */
/* | o	| ||   John Toebes    Dave Baker		     (919)-471-6436*/
/* |  . |//								   */
/* ======								   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Device.c - Device support routines */
/*
**      $Filename: device.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.7 $
**      $Date: 90/12/29 13:07:00 $
**      $Log:	device.c,v $
 * Revision 1.7  90/12/29  13:07:00  J_Toebes
 * Change name of config file to a #define to make it easier to build the testing
 * and production server.
 * 
 * Revision 1.6  90/11/29  01:57:19  J_Toebes
 * Eliminate DosFreeMem calls.
 * 
 * Revision 1.5  90/11/28  03:28:13  J_Toebes
 * Change config file name, correct perm pack initialization.
 * 
 * Revision 1.4  90/11/18  22:55:27  J_Toebes
 * Corrected location of config file, corrected library opening code.
 * 
 * Revision 1.3  90/11/15  08:17:32  J_Toebes
 * Correct multiple bugs associated with new version.  Sessions still are not supported.
 * 
 * Revision 1.2  90/11/05  06:56:17  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
**
**/

#include "server.h"

void ParseConfig(global)
GLOBAL global;
{
   BPTR fh;
   char libname[24];
   char *s, *t;
   char buf[101];
   int len, i;
   DRIVER *driver;
   struct Library *DriverBase;
   SPACKET *sp;

   fh = Open(CONFIGFILE, MODE_OLDFILE);
   if (fh == NULL)
   {
      return;
   }

   len = 0;
   for(;;)
   {
      i = Read(fh, buf+len, 100-len);
      if (i < 0) break;
      len += i;
      if (len == 0) break;
      for (i = 0; i < 100; i++)
         if (buf[i] == '\n')
            break;
      /* Null terminate the buffer */
      buf[i] = 0;
      if (buf[0] != '#')
      {
         /* We have one to configure */
         /* Parse out the device name and open the library for it */
         /* Then we call the device dependent */
         driver = (DRIVER *)AllocMem(sizeof(DRIVER), MEMF_CLEAR);
         if (driver == NULL) break;         
         memcpy(libname, "NET-", 4);
         t = libname+4;
         s = buf;
         while(*s && *s != ' ')
            *t++ = *s++;
         strcpy(t, ".library");
         BUG(("Config to open library %s\n", libname));
         if (!global->sigbits)
         {
            /* We should check to see that we actually get the signal bit and   */
            /* do something sensible, but for now let it slide                  */
            global->sigbits = 1L << AllocSignal(-1);
         }
         driver->sigmask = global->sigbits;
         DriverBase = driver->libbase = OpenLibrary(libname, 0);
         if (driver->libbase == NULL ||
             SDNSerInit(&driver->drglobal, &driver->sigmask, s) != SDN_OK)
         {
            BUGR("Cannot init requested library");
            FreeMem((char *)driver, sizeof(DRIVER));
         }
         else
         {
            BUG(("Library inited\n"));
            /* Now eliminate their signal bits from what we gave them   */
            global->sigbits &= ~driver->sigmask;
            global->waitbits |= driver->sigmask;
            
            /* Add them to the end of the chain because we are using this list  */
            /* as a priority order of drivers to be evaluated.                  */
            driver->next = NULL;
            if (global->drivers == NULL)
               global->drivers = driver;
            else
            {
               DRIVER *dp;
               for (dp = global->drivers; dp->next != NULL; dp = dp->next);
               dp->next = driver;
            }
         }
      }
      memcpy(buf, buf+i+1, 101-i);
      len -= i+1;
   }
   global->pspacket = (SPACKET *)AllocMem(PERMSPACK*sizeof(SPACKET),
                                          MEMF_CLEAR|MEMF_PUBLIC);
   if (global->pspacket != NULL)
   {
      for(sp = global->pspacket, i = 0; i < PERMSPACK; i++, sp++)
      {
         sp->sp_Next = SPACKET_PERM;
         sp->sp_Msg.mn_Node.ln_Name = (char *)&(sp->sp_Pkt);
         sp->sp_Pkt.dp_Link         = &(sp->sp_Msg);
      }
   }
   Close(fh);
}

SPACKET *GetSPacket(global)
GLOBAL global;
{
   SPACKET *sp;
   int i;
   for(sp = global->pspacket, i = 0; i < PERMSPACK; i++, sp++)
   {
      if (sp->sp_State == 0)
      {
         /* We found an empty one to use */
         sp->sp_State = 1;      /* Any non-zero value will do.                  */
         return(sp);
      }
   }
   /* Ooops, we need to allocate some memory to hold the packet                 */
   sp = AllocMem(sizeof(SPACKET), MEMF_CLEAR|MEMF_PUBLIC);
   if (sp != NULL)
   {
      sp->sp_Next = global->tempspkt;
      global->tempspkt = sp;
      sp->sp_State = 1;
      sp->sp_Msg.mn_Node.ln_Name = (char *)&(sp->sp_Pkt);
      sp->sp_Pkt.dp_Link         = &(sp->sp_Msg);
   }
   return(sp);
}

void FreeSPacket(global, smsg)
GLOBAL global;
SPACKET * smsg;
{
   SPACKET *sp;
   if (smsg == NULL) return;
   smsg->sp_State = 0;
   if (smsg->sp_Next != SPACKET_PERM)
   {
      /* It is a temporary one.  Return it to the system                        */
      if (global->tempspkt == smsg)
      {
         global->tempspkt = smsg->sp_Next;
      }
      else
      {
         for(sp = global->tempspkt; sp != NULL && sp->sp_Next != smsg; sp=sp->sp_Next);
         if (sp != NULL)
         {
            sp->sp_Next = smsg->sp_Next;
         }
      }
      /* We should probably see if we have to free a FIB or whatever here       */
      FreeMem(smsg, sizeof(SPACKET));
   }
}

ACTFN(RmtDie    )
{
    if (--global->n.run == 1)
	global->n.run = 0;
}

